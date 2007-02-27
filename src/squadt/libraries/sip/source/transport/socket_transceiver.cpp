#include <iostream>
#include <sstream>

#include <transport/detail/socket_transceiver.h>

namespace transport {
  namespace transceiver {

    unsigned int                   socket_transceiver::input_buffer_size = 8192;

    socket_scheduler               socket_transceiver::scheduler;

    boost::asio::ip::tcp::resolver socket_transceiver::resolver(scheduler.io_service);

    transport::port_t socket_transceiver::default_port = 10947;

    boost::asio::socket_base::keep_alive  option_keep_alive(true);
    boost::asio::socket_base::linger      option_linger(false, 0);

    /* Start listening */
    void socket_transceiver::activate(socket_transceiver::wptr w) {
      socket_transceiver::sptr l(w.lock());

      if (l.get() != 0) {
        using namespace boost;
        using namespace boost::asio;

        socket.set_option(option_keep_alive);
        socket.set_option(option_linger);

        socket.async_receive(asio::buffer(buffer.get(), input_buffer_size), 0, 
                        boost::bind(&socket_transceiver::handle_receive, this, w, _1));
      }
    }

    /**
     * \param[in] a the address to connect to
     * \param[in] p the port to use
     * \param[in] w a shared pointer for this object
     *
     * \pre w.lock.get() must be `this'
     **/
    void socket_transceiver::connect(socket_transceiver::wptr w, ip_address_t const& a, port_t const& p) {
      socket_transceiver::sptr l(w.lock());

      if (l.get() != 0) {
        using namespace boost;
        using namespace boost::asio;

        boost::system::error_code e;

        boost::mutex::scoped_lock l(operation_lock);

        /* Build socket connection */
        ip::tcp::endpoint endpoint(a, (p == 0) ? default_port : p);

        socket.connect(endpoint, e);

        /* Set socket options */
        socket.set_option(socket_base::keep_alive(true));
        socket.set_option(socket_base::linger(false, 0));

        if (!e) {
          /* Clear buffer */
          for (unsigned int i = 0; i < input_buffer_size; ++i) {
            buffer[i] = 0;
          }

          socket.async_receive(asio::buffer(buffer.get(), input_buffer_size), 0,
                          boost::bind(&socket_transceiver::handle_receive, this, w, _1));

          /* Make sure the scheduler is running */
          scheduler.run();
        }
        else {
          if (e == asio::error::eof) {
            handle_disconnect(this);
          }
          else if (e != asio::error::operation_aborted) {
            /* The safe default error handling */
            throw (boost::system::system_error(e));
          }
        }
      }
    }

    /**
     * \param[in] w a reference to a weak pointer for this object
     * \param[in] h the host name to use
     * \param[in] p the port to use
     **/
    void socket_transceiver::connect(socket_transceiver::wptr w, const std::string& h, port_t const& p) {
      using namespace boost::asio;

      socket_transceiver::sptr l(w.lock());

      if (l.get() != 0) {
        connect(w, (*resolver.resolve(ip::tcp::resolver::query(h, "",
                        ip::resolver_query_base::numeric_service|
                        ip::resolver_query_base::address_configured))).endpoint().address(), p);
      }
    }

    /**
     * @param w a reference to a weak pointer for this object (w.lock().get() == this (or 0)
     **/
    void socket_transceiver::disconnect(socket_transceiver::wptr w, basic_transceiver::ptr) {
      socket_transceiver::sptr l(w.lock());

      if (l.get() != 0) {
        boost::mutex::scoped_lock s(send_lock);

        /* Wait until send operations complete */
        if (0 < send_count) {
          send_monitor.wait(s);
        }
      
        boost::mutex::scoped_lock l(operation_lock);

        socket.close();

        basic_transceiver::handle_disconnect(this);
      }
    }

    transport::host_name_t socket_transceiver::get_local_host() {
      using namespace boost::asio;

      std::string current_host_name(ip::host_name());

      ip::tcp::resolver::iterator i(resolver.resolve(ip::tcp::resolver::query(current_host_name, "",
                        ip::resolver_query_base::numeric_service|ip::resolver_query_base::address_configured)));

      if (i == ip::tcp::resolver::iterator()) {
        return (boost::asio::ip::address_v4::loopback().to_string());
      }

      return ((*i).endpoint().address().to_string());
    }

    /**
     * @param w a reference to a weak pointer for this object (w.lock().get() == this (or 0)
     * @param e reference to an asio error object
     **/
    void socket_transceiver::handle_receive(socket_transceiver::wptr w, const boost::system::error_code& e) {
      /* Object still exists, so do the receiving and delivery */
      using namespace boost;
      using namespace boost::asio;

      socket_transceiver::ptr s = w.lock();

      if (!w.expired()) {
        mutex::scoped_lock l(s->operation_lock);

        if (!e) {
          basic_transceiver::deliver(std::string(buffer.get()));

          /* Clear buffer */
          for (unsigned int i = 0; i < input_buffer_size; ++i) {
            buffer[i] = 0;
          }

          socket.async_receive(asio::buffer(buffer.get(), input_buffer_size), 0,
                                  boost::bind(&socket_transceiver::handle_receive, this, w, _1));
       
          /* Make sure the scheduler is running */
          scheduler.run();
        }
        else {
          if (e == asio::error::eof) {
            /* The safe default error handling */
            handle_disconnect(this);
          }
          else if (e != asio::error::operation_aborted) {
            throw (boost::system::system_error(e));
          }
        }
      }
    }

    /**
     * @param w a reference to a weak pointer for this object (w.lock().get() == this (or 0)
     * @param e reference to an asio error object
     **/
    void socket_transceiver::handle_write(socket_transceiver::wptr w, boost::shared_array < char >, const boost::system::error_code& e) {
      socket_transceiver::ptr s = w.lock();

      if (!w.expired()) {
        boost::mutex::scoped_lock k(send_lock);

        if (--send_count == 0) {
          send_monitor.notify_all();
        }

        /* Object still exists, so continue processing the write operation */
        if (e) {
          if (e == boost::asio::error::eof) {
            /* Connection was closed by peer */
            handle_disconnect(this);
          }
          else if (e != boost::asio::error::operation_aborted) {
            throw (boost::system::system_error(e));
          }
        }
      }
    }

    /**
     * @param w a reference to a weak pointer for this object (w.lock().get() == this (or 0)
     * @param d the data that is to be sent
     **/
    void socket_transceiver::send(socket_transceiver::wptr w ,const std::string& d) {
      socket_transceiver::sptr l(w.lock());

      if (l.get() != 0) {
        boost::mutex::scoped_lock k(send_lock);

        ++send_count;

        boost::mutex::scoped_lock l(operation_lock);

        /* The null character is added so that the buffer on the receiving end does not have to be cleared every time */
        boost::shared_array < char > buffer(new char[d.size() + 1]);

        d.copy(buffer.get(), d.size(), 0);

        boost::asio::async_write(socket, boost::asio::buffer(buffer.get(), d.size()), 
               boost::asio::transfer_all(),
               boost::bind(&socket_transceiver::handle_write, this, w, buffer, _1));
      }
    }

    /**
     * @param w a reference to a weak pointer for this object (w.lock().get() == this (or 0)
     * @param d the stream that contains the data that is to be sent
     **/
    void socket_transceiver::send(socket_transceiver::wptr w, std::istream& d) {
      using namespace boost;
      using namespace boost::asio;

      socket_transceiver::sptr l(w.lock());

      if (l.get() != 0) {
        mutex::scoped_lock k(send_lock);

        ++send_count;

        std::ostringstream s;

        mutex::scoped_lock l(operation_lock);

        s << d.rdbuf();

        /* The null character is added so that the buffer on the receiving end does not have to be cleared every time */
        shared_array < char > buffer(new char[s.str().size() + 1]);

        s.str().copy(buffer.get(), s.str().size(), 0);

        async_write(socket, asio::buffer(buffer.get(), s.str().size()), 
               boost::asio::transfer_all(),
               bind(&socket_transceiver::handle_write, this, w, buffer, _1));
      }
    }
  }
}

