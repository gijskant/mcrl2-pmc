#ifndef TRANSPORTER_H
#define TRANSPORTER_H

#include <exception>
#include <string>
#include <list>
#include <istream>

#include <boost/asio/ipv4/address.hpp>
#include <boost/asio/ipv4/host.hpp>
#include <boost/noncopyable.hpp>

#include <transport/detail/exception.h>
#include <transport/detail/transceiver.h>
#include <transport/detail/listener.h>

/*
 * Socket/Direct communication abstraction
 *
 * for transparent communication, via sockets or direct, between senders and
 * receivers
 *
 *  Direct:
 *   - the sender directly calls the deliver routine of a connected transporter object
 *
 *  Sockets:
 *   - the sender puts the data on a socket, when the data arrives, it is
 *   passed in the very same way to the receiving context.
 *
 *  Currently, unless exceptions occur, a single sender can be connected to
 *  many receivers that will all receive everything that is sent.
 */

namespace transport {

  using transceiver::basic_transceiver;
  using listener::basic_listener;

  class transporter : boost::noncopyable {
    friend class basic_transceiver;
    friend class basic_listener;

    public:

      /** Convenience type to hide the shared pointer */
      typedef std::list < basic_transceiver::ptr >    connection_list;

      /** Convenience type to hide the shared pointer */
      typedef std::list < basic_listener::ptr >       listener_list;

      /** IP version 4 address verifier (refer to the asio documentation) */
      typedef asio::ipv4::address                     address;

      /** IP version 4 address verifier (refer to the asio documentation) */
      typedef asio::ipv4::host                        host;

    private:

      /** \brief listeners (for socket communication etc) */
      listener_list   listeners;

      /** \brief The list with connections */
      connection_list connections;

    private:

      /** \brief Abstract function for the delivery of streamed data to the client program */
      virtual void deliver(std::istream&, basic_transceiver*) = 0;

      /** \brief Abstract function for the delivery of streamed data to the client program */
      virtual void deliver(std::string&, basic_transceiver*) = 0;

      /** \brief Creates direct connection to another transporter object */
      void connect(basic_transceiver*);

      /** \brief Associate a connection with this transporter */
      void associate(const basic_transceiver::ptr&);

      /** \brief Disassociate a connection from this transporter */
      basic_transceiver::ptr disassociate(const basic_transceiver*);

    public:
  
      /** \brief Default constructor with no initial connections */
      inline transporter();
 
      /** \brief Destructor */
      virtual ~transporter();
  
      /** \brief Creates direct connection to another transporter object */
      void connect(transporter&);

      /** \brief Creates socket connection to another transporter object (using a loopback connection by default) */
      void connect(const address& = address::loopback(), const long port = 0);

      /** \brief Creates socket connection to another transporter object (using a loopback connection by default) */
      void connect(const std::string& host_name, const long port = 0);

      /** \brief Disconnect connection number <|number|> */
      void disconnect(size_t number = 0);

      /** \brief Disconnect from directly connected peer */
      void disconnect(transporter&);

      /** \brief Pass a connection through to another transporter */
      inline void relay_connection(transporter*, basic_transceiver*);

      /** \brief Activate a socket listener (using a loopback connection by default) */
      void add_listener(const address& = address::loopback(), const long port = 0);

      /** \brief Activate a socket listener by its number */
      void remove_listener(size_t number = 0);
  
      /** \brief Communicate a string with all peers */
      inline void send(const std::string&);
 
      /** \brief Communicate data from a stream with all peers */
      inline void send(std::istream&);

      /** \brief The number of active listeners */
      inline size_t number_of_listeners() const;

      /** \brief Returns an object with the local hosts name and addresses */
      static host get_local_host();

      /** \brief The number of active connections */
      inline size_t number_of_connections() const;
  };

  inline transporter::transporter() {
  }
 
  inline size_t transporter::number_of_listeners() const {
    return (listeners.size());
  }

  inline size_t transporter::number_of_connections() const {
    return (connections.size());
  }

  /**
   * @param d the data to be sent
   **/
  inline void transporter::send(const std::string& d) {
    for (connection_list::const_iterator i = connections.begin(); i != connections.end(); ++i) {
      (*i)->send(d);
    }
  }

  /**
   * @param s stream that contains the data to be sent
   **/
  inline void transporter::send(std::istream& s) {
    for (connection_list::const_iterator i = connections.begin(); i != connections.end(); ++i) {
      (*i)->send(s);
    }
  }

  /**
   * @param[in,out] t the transporter to relay the connection to
   * @param[in] c the transceiver that represents the local end point of the connection
   **/
  inline void transporter::relay_connection(transporter* t, basic_transceiver* c) {
    assert(t != 0);

    t->associate(c->owner->disassociate(c));

    c->owner = t;
  }
}

#endif

