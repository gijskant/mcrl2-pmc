#include <algorithm>
#include <functional>

#include <boost/asio/ip/address.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>
#include <boost/foreach.hpp>

#define IMPORT_STATIC_DEFINITIONS
#include <transport/detail/basics.h>
#include <transport/detail/transceiver.tcc>
#include <transport/detail/socket_listener.h>
#include <transport/detail/direct_transceiver.h>
#include <transport/detail/socket_transceiver.h>

namespace transport {

  using namespace transceiver;
  using namespace listener;

  transporter::~transporter() {
    using namespace boost;

    boost::recursive_mutex::scoped_lock l(lock);

    /* Disconnect all peers */
    disconnect();
  
    /* Clean up listeners */
    for (listener_list::iterator i = listeners.begin(); i != listeners.end(); ++i) {
      (*i)->shutdown();
    }
  }

  /**
   * \param p the peer to connect to
   **/
  void transporter::connect(transporter& p) {
    basic_transceiver::ptr t(new direct_transceiver(this));

    boost::recursive_mutex::scoped_lock pl(p.lock);
    p.connections.push_back(t);

    boost::recursive_mutex::scoped_lock l(lock);
    connections.push_back(basic_transceiver::ptr(new direct_transceiver(&p, reinterpret_cast < direct_transceiver* > (t.get()))));
  }

  /**
   * \param a an address
   * \param p a port
   **/
  void transporter::connect(const ip_address_t& a, port_t const& p) {
    basic_transceiver::ptr c = socket_transceiver::create(this);

    boost::recursive_mutex::scoped_lock l(lock);

    reinterpret_cast < socket_transceiver* > (c.get())->connect(a, p);

    connections.push_back(c);
  }

  /**
   * \param h a hostname
   * \param p a port
   **/
  void transporter::connect(const std::string& h, port_t const& p) {
    basic_transceiver::ptr c = socket_transceiver::create(this);

    boost::recursive_mutex::scoped_lock l(lock);

    reinterpret_cast < socket_transceiver* > (c.get())->connect(h, p);

    connections.push_back(c);
  }

  /**
   * \param t the connection to associate with this transporter
   **/
  void transporter::associate(const basic_transceiver::ptr& t) {
    boost::recursive_mutex::scoped_lock l(lock);

    const basic_transceiver* p = t.get();

    connection_list::iterator i = std::find_if(connections.begin(), connections.end(),
                      boost::bind(std::equal_to< const basic_transceiver* >(), p,
                              boost::bind(&basic_transceiver::ptr::get, _1)));

    if (i == connections.end()) {
      connections.push_back(t);

      t->owner = this;
    }
  }

  /**
   * \param t the transceiver that identifies the connection to be severed
   *
   * \return a shared pointer to the transceiver that is removed
   **/
  basic_transceiver::ptr transporter::disassociate(basic_transceiver* t) {
    basic_transceiver::ptr p;

    assert(t->owner == this);

    boost::recursive_mutex::scoped_lock l(lock);

    connection_list::iterator i = std::find_if(connections.begin(), connections.end(),
                    boost::bind(std::equal_to< const basic_transceiver* >(), t,
                            boost::bind(&basic_transceiver::ptr::get, _1)));

    if (i != connections.end()) {
      p = *i;

      connections.erase(i);

      t->owner = 0;
    }

    return (p);
  }

  /**
   * \param t the transceiver that identifies the connection to be associated
   * \pre t->owner != 0 && t->owner != this
   **/
  void transporter::associate(basic_transceiver* t) {
    assert(t->owner != 0);

    if (t->owner != this) {
      boost::recursive_mutex::scoped_lock l(lock);
      boost::recursive_mutex::scoped_lock ll(t->owner->lock);

      for (connection_list::iterator i = t->owner->connections.begin(); i != t->owner->connections.end(); ++i) {
        if (t == (*i).get()) {
          connections.push_back(*i);
          t->owner->connections.erase(i);

          break;
        }
      }

      t->owner = this;
    }
  }

  void transporter::disconnect() {
    boost::recursive_mutex::scoped_lock l(lock);

    for (connection_list::const_iterator i = connections.begin(); i != connections.end(); i = connections.begin()) {
      (*i)->disconnect(*i);
    }
  }

  /**
   * \param n the number of the connection that is to be closed
   **/
  void transporter::disconnect(size_t n) {
    assert(n < connections.size());

    boost::recursive_mutex::scoped_lock l(lock);

    connection_list::iterator i = connections.begin();
  
    while (0 < n) {
      assert(i != connections.end());

      --n;

      ++i;
    }

    (*i)->disconnect(*i);
  }

  /**
   * \param m the directly connected peer
   **/
  void transporter::disconnect(transporter& m) {
    using namespace boost;

    boost::recursive_mutex::scoped_lock l(lock);

    const transporter* p = &m;

    connection_list::iterator i = std::find_if(connections.begin(), connections.end(),
                      bind(std::equal_to< const transporter* >(), p,
                                      bind(&basic_transceiver::get_owner,
                                                      bind(&basic_transceiver::ptr::get, _1))));

    if (i != connections.end()) {
      (*i)->disconnect(*i);
    }
  }

  /**
   * \param a an address
   * \param p a port
   **/
  void transporter::add_listener(const ip_address_t& a, port_t const& p) {
    basic_listener::ptr new_listener(new socket_listener(*this, a, p));

    listeners.push_back(new_listener);

    reinterpret_cast < socket_listener* > (new_listener.get())->activate(new_listener);
  }

  /**
   * \param a an address
   * \param p a port
   **/
  void transporter::add_listener(const host_name_t& a, port_t const& p) {
    basic_listener::ptr new_listener(new socket_listener(*this, boost::asio::ip::address_v4::from_string(a), p));

    listeners.push_back(new_listener);

    reinterpret_cast < socket_listener* > (new_listener.get())->activate(new_listener);
  }

  /**
   * \param n the number of the listener that is to be removed
   **/
  void transporter::remove_listener(size_t n) {
    assert(n < listeners.size());

    listener_list::iterator i = listeners.begin();

    while (0 < n) {
      --n;

      ++i;
    }

    if (i != listeners.end()) {
      (*i)->shutdown();

      listeners.erase(i);
    }
  }

  host_name_t transporter::get_local_host() {
    return (socket_transceiver::get_local_host());
  }
}

