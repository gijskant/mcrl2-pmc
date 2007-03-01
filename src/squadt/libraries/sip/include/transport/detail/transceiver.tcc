#ifndef TRANSPORT_TRANSCEIVER_TCC
#define TRANSPORT_TRANSCEIVER_TCC

#include <transport/transporter.h>

namespace transport {
  namespace transceiver {

    inline transporter const* basic_transceiver::get_owner() const {
      return (owner);
    }

    /**
     * @param o a pointer to the owner
     **/
    inline basic_transceiver::basic_transceiver(transporter* o) : owner(o) {
      assert(o != 0);
    }

    /**
     * @param d an string that represents the data to be delivered
     **/
    inline void basic_transceiver::deliver(const std::string& d) {
      if (owner != 0) {
        owner->deliver(d, this);
      }
    }

    /**
     * @param d an input stream with the data that is to be delivered
     **/
    inline void basic_transceiver::deliver(std::istream& d) {
      if (owner != 0) {
        owner->deliver(d, this);
      }
    }

    /**
     * @param t a pointer to the owner
     **/
    inline void basic_transceiver::handle_disconnect(basic_transceiver* t) {
      /* Remove instance from the list of connections */
      if (owner != 0) {
        owner->disassociate(t);
      }
    }

    inline basic_transceiver::~basic_transceiver() {
    }
  }
}

#endif

