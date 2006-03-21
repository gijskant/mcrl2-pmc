#include <transport/transporter.h>
#include <transport/detail/socket_listener.h>
#include <transport/detail/socket_transceiver.h>

namespace transport {
  namespace listener {

    using namespace transceiver;
    using namespace listener;

    /**
     * @param m the owner of the listener
     * @param a the address to listen on
     * @param p the port to listen on
     **/
    socket_listener::socket_listener(transporter& m, const address& a, const long p) :
      basic_listener(m), acceptor(socket_transceiver::scheduler.demuxer), dispatcher(socket_transceiver::scheduler.demuxer) {
      using namespace asio;
      using namespace boost;

      ipv4::tcp::endpoint endpoint((p == 0) ? socket_transceiver::default_port : p, ipv4::address(a));

      acceptor.open(endpoint.protocol());
      acceptor.set_option(socket_acceptor::reuse_address(true));
      acceptor.bind(endpoint);
      acceptor.listen();
    }

    /**
     * @param l a shared pointer to the listerner
     **/
    void socket_listener::activate(basic_listener::ptr l) {
      /* Create a new socket transceiver that is not yet connected to the transporter */
      boost::shared_ptr < socket_transceiver > t(new socket_transceiver(&owner));

      acceptor.async_accept(t->socket, dispatcher.wrap(bind(&socket_listener::handle_accept, this, asio::placeholders::error, t, l)));

      /* Make sure the scheduler is running */
      socket_transceiver::scheduler.run();
    }

    /**
     * @param e asio error object
     * @param t the transceiver that will be the end point of this side of a new connection
     * @param l a shared pointer to the listerner
     **/
    void socket_listener::handle_accept(const asio::error& e, transceiver::socket_transceiver::ptr t, basic_listener::ptr l) {
      if (!e) {
        socket_listener::associate(t);

        t->activate();

        t = boost::shared_ptr < socket_transceiver > (new socket_transceiver(&owner));

        /* Listen for new connections */
        acceptor.async_accept(t->socket, dispatcher.wrap(bind(&socket_listener::handle_accept, this, asio::placeholders::error, t, l)));
      }
      else if (e == asio::error::connection_aborted) {
        /* Listen for new connections */
        acceptor.async_accept(t->socket, dispatcher.wrap(bind(&socket_listener::handle_accept, this, asio::placeholders::error, t, l)));
      }
      else if (e == asio::error::operation_aborted) {
        return;
      }
      else {
        /* Some other error occurred, abort... */
        throw (exception(exception_identifier::listener_failure));
      }

      /* Make sure the scheduler is running */
      socket_transceiver::scheduler.run();
    }
  }
}

