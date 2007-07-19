#ifndef TIPI_TRANSPORTER_TCC__
#define TIPI_TRANSPORTER_TCC__

#include <exception>
#include <list>
#include <istream>

#include <boost/thread/recursive_mutex.hpp>

#include "transport/transporter.hpp"
#include "transport/detail/listener.hpp"

namespace transport {
  using namespace transceiver;
  using namespace listener;

  class transporter_impl : private boost::noncopyable {
    friend class transceiver::basic_transceiver;
    friend class listener::basic_listener;
    friend class transport::transporter;

    public:

      /** \brief Convenience type to hide the shared pointer */
      typedef std::list < basic_transceiver::ptr >    connection_list;

      /** \brief Convenience type to hide the shared pointer */
      typedef std::list < basic_listener::ptr >       listener_list;

    private:

      /** \brief To obtain mutual exclusion for operations on the list of connections */
      mutable boost::recursive_mutex lock;

      /** \brief Listeners (for socket communication etc) */
      listener_list                  listeners;

      /** \brief The list with connections */
      connection_list                connections;

    protected:

      /** \brief Abstract function for the delivery of streamed data to the client program */
      virtual void deliver(std::istream&, basic_transceiver const*) = 0;

      /** \brief Abstract function for the delivery of streamed data to the client program */
      virtual void deliver(std::string const&, basic_transceiver const*) = 0;

    protected:

      /** \brief Associate a connection with this transporter */
      void associate(boost::shared_ptr < transporter_impl > const&, const basic_transceiver::ptr&);

      /** \brief Associate a connection with this transporter */
      void associate(boost::shared_ptr < transporter_impl > const&, basic_transceiver*);

      /** \brief Disassociate a connection from this transporter */
      basic_transceiver::ptr disassociate(basic_transceiver*);

      /** \brief Creates direct connection to another transporter object */
      void connect(boost::shared_ptr < basic_transceiver > const&, boost::shared_ptr < transporter_impl >&);

      /** \brief Creates socket connection to another transporter object (using a loopback connection by default) */
      void connect(boost::shared_ptr < basic_transceiver > const&, ip_address_t const& = ip_loopback, port_t const& = 0);

      /** \brief Creates socket connection to another transporter object (using a loopback connection by default) */
      void connect(boost::shared_ptr < basic_transceiver > const&, host_name_t const&, port_t const& = 0);

      /** \brief Disconnect all */
      void disconnect();

      /** \brief Disconnect connection number <|number|> */
      void disconnect(size_t number);

      /** \brief Disconnect from directly connected peer */
      void disconnect(transporter_impl const*);

      /** \brief Pass a connection through to another transporter */
      void relay_connection(transporter*, basic_transceiver*);

      /** \brief Activate a socket listener */
      void add_listener(boost::shared_ptr < transporter_impl > const&, ip_address_t const& = ip_any, port_t const& port = 0);

      /** \brief Activate a socket listener */
      void add_listener(boost::shared_ptr < transporter_impl > const&, host_name_t const&, port_t const& port = 0);

      /** \brief Activate a socket listener by its number */
      void remove_listener(size_t number = 0);
  
      /** \brief Communicate a string with all peers */
      void send(const std::string&);
 
      /** \brief Communicate data from a stream with all peers */
      void send(std::istream&);

      /** \brief Destructor */
      virtual ~transporter_impl();
  };
}
#endif

