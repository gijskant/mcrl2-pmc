#ifndef LISTENER_H
#define LISTENER_H

#include <boost/shared_ptr.hpp>

#include <transport/transporter.h>

namespace transport {
  namespace transceiver {
    class basic_transceiver;
  }

  namespace listener {

    /* Base class for listeners */
    class basic_listener {
      public:
        /** \brief Convenience type alias to hide the shared pointer implementation */
        typedef boost::shared_ptr < basic_listener > ptr;

      protected:
        /** \brief The communicator that will take over the accepted connections */
        transport::transporter& owner;

        /** \brief Associates the owner with the connection */
        inline void associate(boost::shared_ptr < transceiver::basic_transceiver >);

      public:

        /** \brief Constructor */
        inline basic_listener(transport::transporter& m);

        /** \brief Shutdown the listener */
        virtual void shutdown() = 0;

        /** \brief Destructor */
        virtual inline ~basic_listener();
    };

    inline void basic_listener::associate(boost::shared_ptr < transceiver::basic_transceiver > t) {
      owner.connections.push_back(t);
    }

    inline basic_listener::basic_listener(transporter& m) : owner(m) {
    }

    inline basic_listener::~basic_listener() {
    }
  }
}

#endif

