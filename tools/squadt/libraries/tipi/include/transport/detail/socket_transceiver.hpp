//  Copyright 2007 Jeroen van der Wulp. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file include/transport/detail/socket_transceiver.h

#ifndef SOCKET_TRANSCEIVER_H__
#define SOCKET_TRANSCEIVER_H__

#include <memory>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/system/system_error.hpp>

#include "transport/transporter.hpp"
#include "transport/detail/basics.hpp"
#include <transport/detail/transceiver.tcc>
#include "transport/detail/socket_scheduler.hpp"

namespace transport {
  namespace transceiver {

    /* Class that is used internally for direct transmitting/receiving */
    class socket_transceiver : public basic_transceiver {
      friend class transport::listener::socket_listener;
  
      public:

        /** \brief Convenience type to hide the boost shared pointer implementation */
        typedef boost::shared_ptr < socket_transceiver >  ptr;

        /** \brief Convenience type to hide the boost shared pointer implementation */
        typedef boost::shared_ptr < socket_transceiver >  sptr;

      private:

        /** \brief Host name resolver */
        static boost::asio::ip::tcp::resolver   resolver;

        /** \brief Scheduler for asynchronous socket communication */
        static socket_scheduler                 scheduler;

        /** \brief Default port for socket connections */
        static port_t                           default_port;

        /** \brief Size of the input buffer */
        static unsigned int                     input_buffer_size;

      private:

        /** \brief The input buffer */
        boost::shared_array < char >           buffer;

        /** \brief The local endpoint of a connection */
        boost::asio::ip::tcp::socket           socket;
 
        /** \brief Used to make operations: read, write and disconnect mutually exclusive */
        boost::mutex                           operation_lock;

        /** \brief Used to make sure that an object is not deleted when its methods are still executing */
        boost::weak_ptr < socket_transceiver > this_ptr;

        /** \brief Used to make it possible to wait for send operations to complete */
        boost::mutex                           send_lock;

        /** \brief Used to make it possible to wait for send operations to complete */
        boost::condition                       send_monitor;

        /** \brief The amount of send operations that are currently in progress */
        unsigned int                           send_count;

      private:

        /** \brief Constructor that connects to a port on an address */
        inline socket_transceiver(boost::shared_ptr < transporter_impl > const&);

        /** \brief Wrapper for connect() that ensures establishes that the object is not freed yet */
        void connect(boost::weak_ptr < socket_transceiver >, const std::string&, port_t const&);

        /** \brief Wrapper for connect() that ensures establishes that the object is not freed yet */
        void connect(boost::weak_ptr < socket_transceiver >, const ip_address_t&, port_t const&);

        /** \brief Send a string input stream to the peer */
        void send(boost::weak_ptr < socket_transceiver >, const std::string&);
  
        /** \brief Send the contents of an input stream to the peer */
        void send(boost::weak_ptr < socket_transceiver >, std::istream&);

        /** \brief Terminate the connection with the peer */
        void disconnect(boost::weak_ptr < socket_transceiver >&, basic_transceiver::ptr const&);

        /** \brief Start listening for new data */
        void activate(boost::weak_ptr < socket_transceiver >);

        /** \brief Read from the socket */
        void handle_receive(boost::weak_ptr < socket_transceiver >, const boost::system::error_code&);

        /** \brief Process results from a write operation on the socket */
        void handle_write(boost::weak_ptr < socket_transceiver >, boost::shared_array < char >, const boost::system::error_code&);

        /** \brief Actual disconnection operation */
        void handle_disconnect();

      public:

        /** \brief Factory function */
        static inline socket_transceiver::ptr create(boost::shared_ptr < transporter_impl > const&);

        /** \brief Returns an object with the local hosts name and addresses */
        static host_name_t get_local_host();

        /** \brief Send a string input stream to the peer */
        void send(const std::string&);
  
        /** \brief Send the contents of an input stream to the peer */
        void send(std::istream&);

        /** \brief Wrapper for connect() that ensures establishes that the object is not freed yet */
        inline void connect(const std::string&, port_t const& = default_port);

        /** \brief Wrapper for connect() that ensures establishes that the object is not freed yet */
        inline void connect(const ip_address_t& = transport::ip_any, port_t const& = default_port);

        /** \brief Terminate connection with peer */
        void disconnect(boost::shared_ptr < basic_transceiver > const&);

        /** \brief Start listening for new data */
        void activate();

        /** \brief Destructor */
        ~socket_transceiver();
    };

    /**
     * @param o a transporter to which to deliver data
     **/
    inline socket_transceiver::socket_transceiver(boost::shared_ptr < transporter_impl > const& o) : basic_transceiver(o),
                buffer(new char[input_buffer_size + 1]), socket(scheduler.io_service), send_count(0) {

      buffer[input_buffer_size] = '\0';
    }

    /**
     * @param o a transporter to which to deliver data
     **/
    boost::shared_ptr < socket_transceiver > socket_transceiver::create(boost::shared_ptr < transporter_impl > const& o) {
      socket_transceiver::ptr t(new socket_transceiver(o));

      t->this_ptr = boost::weak_ptr < socket_transceiver >(t);

      return (t);
    }

    /**
     * \attention Wrapper for the similar looking private function
     **/
    inline void socket_transceiver::connect(const ip_address_t& a, port_t const& p) {
      connect(this_ptr.lock(), a, p);
    }

    /**
     * \attention Wrapper for the similar looking private function
     **/
    inline void socket_transceiver::connect(const std::string& a, port_t const& p) {
      connect(this_ptr.lock(), a, p);
    }

    /**
     * \attention Wrapper for the similar looking private function
     **/
    inline void socket_transceiver::send(const std::string& d) {
      send(this_ptr.lock(), d);
    }

    /**
     * \attention Wrapper for the similar looking private function
     **/
    inline void socket_transceiver::send(std::istream& d) {
      send(this_ptr.lock(), d);
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
    inline void socket_transceiver::disconnect(boost::shared_ptr < basic_transceiver > const& p) {
      disconnect(this_ptr, p);
    }
  }
}

#endif

