//  Copyright 2007 Jeroen van der Wulp. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file tipi/detail/basic_messenger.hpp

#ifndef BASIC_MESSENGER_H
#define BASIC_MESSENGER_H

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include "tipi/utility/print_logger.hpp"

#include "tipi/detail/transport/transporter.hpp"
#include "tipi/detail/message.hpp"

namespace transport {
  class transporter;
}

namespace tipi {

  namespace messaging {
    template < class M > class basic_messenger;

    template < class M >
    class basic_messenger_impl;

    /**
     * \brief Abstract communicator class that divides an incoming data stream in messages
     *
     * M is the type of a messenger::message or derived type
     */
    template < class M >
    class basic_messenger : public transport::transporter {

      protected:
 
        /** \brief Alternate constructor */
        basic_messenger(boost::shared_ptr < basic_messenger_impl< M > > const&);

      public:

        /** \brief Convenience type for messages of type M */
        typedef M                                                         message;

        /** \brief Convenience type for shared pointers */
        typedef boost::shared_ptr < M >                                   message_ptr;

        /** \brief Convenience type for handlers */
        typedef boost::function < void (boost::shared_ptr < M > const&) > handler_type;
 
      public:

        /** \brief Default constructor */
        basic_messenger();

        /** \brief Default constructor */
        basic_messenger(boost::shared_ptr < utility::logger >);

        /** \brief Destroys all connections */
        void disconnect();
 
        /** \brief Wait until the next message of a certain type arrives */
        const boost::shared_ptr < M > await_message(typename M::type_identifier_t);
 
        /** \brief Wait until the next message of a certain type arrives */
        const boost::shared_ptr < M > await_message(typename M::type_identifier_t, long const&);
 
        /** \brief Send a message */
        void send_message(message const&);
 
        /** \brief Set the handler for a type */
        void add_handler(const typename M::type_identifier_t, handler_type);

        /** \brief Clears the handlers for a message type */
        void clear_handlers(const typename M::type_identifier_t);

        /** \brief Remove a specific handlers for a message type */
        void remove_handler(const typename M::type_identifier_t, handler_type);

        /** \brief Gets the associated logger object */
        utility::logger* get_logger();

        /** \brief Gets the associated logger object */
        static utility::logger* get_standard_logger();

        /** \brief Sets the standard logger object */
        static void set_standard_logger(boost::shared_ptr < utility::logger >);
    };
  }
}

#endif
