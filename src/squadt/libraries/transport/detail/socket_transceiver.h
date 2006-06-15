#ifndef SOCKET_TRANSCEIVER_H
#define SOCKET_TRANSCEIVER_H

#include <memory>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread/mutex.hpp>

#include <transport/transporter.h>
#include <transport/detail/transceiver.tcc>
#include <transport/detail/socket_scheduler.h>

namespace transport {
  namespace transceiver {

    /* Class that is used internally for direct transmitting/receiving */
    class socket_transceiver : public basic_transceiver {
      friend class transport::listener::socket_listener;
  
      public:
        /** \brief IP version 4 address verifier (refer to the asio documentation) */
        typedef asio::ipv4::address                      address;

        /** \brief IP version 4 host class (refer to the asio documentation) */
        typedef asio::ipv4::host                         host;

        /** \brief Convenience type to hide the boost shared pointer implementation */
        typedef boost::shared_ptr < socket_transceiver > ptr;

        /** \brief Convenience type to hide the boost shared pointer implementation */
        typedef boost::weak_ptr < socket_transceiver >   wptr;

      private:

        /** \brief Host name resolver */
        static asio::ipv4::host_resolver resolver;

        /** \brief Scheduler for asynchronous socket communication */
        static socket_scheduler          scheduler;

        /** \brief Default port for socket connections */
        static long                      default_port;

        /** \brief Size of the input buffer */
        static unsigned int              input_buffer_size;

      private:

        /** \brief The input buffer */
        boost::shared_array < char >     buffer;

        /** \brief The local endpoint of a connection */
        asio::ipv4::tcp::socket          socket;
 
        /** \brief Used to make operations: read, write and disconnect mutually exclusive */
        boost::mutex                     operation_lock;

        /** \brief Used to make sure that an object is not deleted when its methods are still executing */
        socket_transceiver::wptr         this_ptr;

        /** \brief Used to make it possible to wait for send operations to complete */
        boost::mutex                     send_lock;

        /** \brief Used to make it possible to wait for send operations to complete */
        boost::condition                 send_monitor;

        /** \brief The amount of send operations that are currently in progress */
        unsigned int                     send_count;

      private:

        /** \brief Constructor that connects to a port on an address */
        inline socket_transceiver(transporter* o);

        /** \brief Wrapper for connect() that ensures establishes that the object is not freed yet */
        void connect(const std::string&, const long, ptr);

        /** \brief Wrapper for connect() that ensures establishes that the object is not freed yet */
        void connect(const address&, const long, ptr);

        /** \brief Send a string input stream to the peer */
        void send(const std::string&, ptr);
  
        /** \brief Send the contents of an input stream to the peer */
        void send(std::istream&, ptr);

        /** \brief Terminate the connection with the peer */
        void disconnect(basic_transceiver::ptr, ptr);

        /** \brief Start listening for new data */
        void activate(ptr);

        /** \brief Read from the socket */
        void handle_receive(wptr, const asio::error&);

        /** \brief Process results from a write operation on the socket */
        void handle_write(wptr, boost::shared_array < char >, const asio::error&);

      public:

        /** \brief Factory function */
        static inline socket_transceiver::ptr create(transporter*);

        /** \brief Returns an object with the local hosts name and addresses */
        static host get_local_host();

        /** \brief Send a string input stream to the peer */
        void send(const std::string&);
  
        /** \brief Send the contents of an input stream to the peer */
        void send(std::istream&);

        /** \brief Wrapper for connect() that ensures establishes that the object is not freed yet */
        inline void connect(const std::string&, const long = default_port);

        /** \brief Wrapper for connect() that ensures establishes that the object is not freed yet */
        inline void connect(const address& = address::any(), const long = default_port);

        /** \brief Terminate the connection with the peer */
        inline void disconnect(basic_transceiver::ptr);

        /** \brief Start listening for new data */
        void activate();

        /** \brief Destructor */
        inline ~socket_transceiver();
    };

    /**
     * @param o a transporter to deliver data to
     **/
    inline socket_transceiver::socket_transceiver(transporter* o) : basic_transceiver(o),
                buffer(new char[input_buffer_size + 1]), socket(scheduler.io_service), send_count(0) {

      buffer[input_buffer_size] = '\0';
    }

    /**
     * @param o a transporter to deliver data to
     **/
    boost::shared_ptr < socket_transceiver > socket_transceiver::create(transporter* o) {
      socket_transceiver::ptr t(new socket_transceiver(o));

      t->this_ptr = socket_transceiver::wptr(t);

      return (t);
    }

    /**
     * \attention Wrapper for the similar looking private function
     **/
    inline void socket_transceiver::connect(const address& a, const long p) {
      connect(a, p, this_ptr.lock());
    }

    /**
     * \attention Wrapper for the similar looking private function
     **/
    inline void socket_transceiver::connect(const std::string& a, const long p) {
      connect(a, p, this_ptr.lock());
    }

    /**
     * \attention Wrapper for the similar looking private function
     **/
    inline void socket_transceiver::send(const std::string& d) {
      send(d, this_ptr.lock());
    }

    /**
     * \attention Wrapper for the similar looking private function
     **/
    inline void socket_transceiver::send(std::istream& d) {
      send(d, this_ptr.lock());
    }

    /**
     * \attention Wrapper for the similar looking private function
     **/
    inline void socket_transceiver::activate() {
      activate(this_ptr.lock());
    }

    /**
     * \attention Wrapper for the similar looking private function
     **/
    inline void socket_transceiver::disconnect(basic_transceiver::ptr p) {
      disconnect(p, this_ptr.lock());
    }

    inline socket_transceiver::~socket_transceiver() {
      boost::mutex::scoped_lock s(send_lock);

      /* Wait until send operations complete */
      if (0 < send_count) {
        send_monitor.wait(s);
      }
    }
  }
}

#endif

