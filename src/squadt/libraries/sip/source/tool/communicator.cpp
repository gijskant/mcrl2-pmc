#include <boost/shared_array.hpp>

#include <sip/tool/category.h>
#include <sip/controller/capabilities.h>
#include <sip/detail/tool.tcc>
#include <sip/detail/event_handlers.h>

namespace sip {
  namespace tool {

    communicator::communicator(communicator_impl* c) : sip::messenger(c) {
    }

    communicator::communicator() : sip::messenger(new communicator_impl) {
    }

    /**
     * \param[in,out] argc the number of command line arguments
     * \param[in,out] argv a pointer to the list of command line arguments
     * \attention the specific command line options are removed, so and argc and argv are modified
     * \return whether options were found and whether a connection is being opened with a controller
     **/
    bool communicator::activate(int& argc, char** const argv) {
      return (boost::dynamic_pointer_cast < communicator_impl > (impl)->activate(argc, argv));
    }

    bool communicator::activate(char*& argv) {
      return (boost::dynamic_pointer_cast < communicator_impl > (impl)->activate(argv));
    }

    bool communicator::activate(std::vector < std::string > const& arguments) {
      int                        argc = arguments.size();
      boost::shared_array<char*> argv(new char*[argc]);

      for (int i = 0; i != argc; ++i) {
        argv[i] = const_cast < char* > (arguments[i].c_str());
      }
      
      return (activate(argc, argv.get()));
    }

    /**
     * \return a pointer to the tool capabilities object that is sent to the controller on request
     **/
    tool::capabilities& communicator::get_tool_capabilities() {
      return (boost::dynamic_pointer_cast < communicator_impl > (impl)->current_tool_capabilities);
    }
 
    /**
     * This object can be stored by the controller and subsequently be used to
     * restore this exact configuration state at the side of the tool.
     *
     * \return a reference to the current tool configuration object
     **/
    configuration& communicator::get_configuration() {
      return (*boost::dynamic_pointer_cast < communicator_impl > (impl)->current_configuration);
    }
 
    void communicator::set_configuration(boost::shared_ptr < configuration > c) {
      boost::dynamic_pointer_cast < communicator_impl > (impl)->current_configuration = c;
    }
 
    /**
     * \return p which is a pointer to the most recently retrieved controller capabilities object or 0
     **/
    const boost::shared_ptr < controller::capabilities > communicator::get_controller_capabilities() const {
      if (boost::dynamic_pointer_cast < communicator_impl > (impl)->current_controller_capabilities.get() == 0) {
        throw std::runtime_error("Controller capabilities are unknown");
      }
 
      return (boost::dynamic_pointer_cast < communicator_impl > (impl)->current_controller_capabilities);
    }
 
    /* Request details about the amount of space that the controller currently has reserved for this tool */
    void communicator::request_controller_capabilities() {
      message m(sip::message_request_controller_capabilities);
 
      impl->send_message(m);
 
      /* Await the reply */
      do {
        message_ptr p = await_message(sip::message_response_controller_capabilities);
 
        if (p.get() != 0) {
          boost::shared_ptr < controller::capabilities > n(new controller::capabilities);

          sip::visitors::restore(*n, p->to_string());
          
          boost::dynamic_pointer_cast < communicator_impl > (impl)->current_controller_capabilities = n;

          break;
        }
      } while (1);
    }
 
    /* Send a specification of the current configuration (it may change during tool execution) */
    void communicator::send_accept_configuration() {
      boost::shared_ptr < configuration > c(boost::dynamic_pointer_cast < communicator_impl > (impl)->current_configuration);

      c->set_fresh(false);

      message m(sip::visitors::store(*c), sip::message_accept_configuration);
 
      impl->send_message(m);
    }
 
    /**
     * \param[in] c the configuration object that specifies the accepted configuration
     **/
    void communicator::send_accept_configuration(sip::configuration& c) {
      c.set_fresh(false);

      message m(sip::visitors::store(c), sip::message_accept_configuration);
 
      impl->send_message(m);
    }
 
    /**
     * Send a layout specification for the display space reserved for this
     * tool. Also sets up an event handler to update the tool display with
     * data from change events, once those are are received.
     *
     * The last communicated display layout is stored internally and is updated
     * accordingly when data is received.
     **/
    void communicator::send_display_layout(layout::tool_display::sptr d) {
      struct trampoline {
        inline static void send_display_data(boost::shared_ptr< communicator_impl > impl, void const* e) {
          std::string c;

          {
            sip::store_visitor v(c);

            v.visit(*reinterpret_cast < sip::layout::element const* > (e), reinterpret_cast < sip::layout::element_identifier > (e));
          }

          impl->send_message(sip::message(c, sip::message_display_update));
        }
      };

      if (d->get_manager()) {
        d->get_manager()->get_event_handler()->connect(boost::bind(trampoline::send_display_data, boost::dynamic_pointer_cast < communicator_impl > (impl), _1));
      }

      boost::dynamic_pointer_cast < communicator_impl > (impl)->send_display_layout(d);
    }

    void communicator::send_clear_display() {
      boost::dynamic_pointer_cast < communicator_impl > (impl)->send_clear_display();
    }

    /* Send a signal that the tool is about to terminate */
    void communicator::send_signal_done(bool b) {
      message m((b) ? "success" : "", sip::message_signal_done);
 
      impl->send_message(m);
    }

    /* Send a signal that the tool is about to terminate */
    void communicator::send_signal_termination() {
      message m(sip::message_signal_termination);
 
      impl->send_message(m);
    }
 
    /* Send a status report to the controller */
    void communicator::send_report(sip::report const& r) const {
      message m(sip::visitors::store(r), sip::message_report);
 
      impl->send_message(m);
    }

    /**
     * \param[in] t the report type
     * \param[in] e a description
     **/
    void communicator::send_status_report(sip::report::type t, std::string const& e) const {
      sip::report report(t, e);

      send_report(report);
    }

    /**
     * \param[in] e pointer to a sip layout element of which the data is to be sent
     **/
    void communicator::send_display_data(layout::element const* e) {
      std::string        c;

      {
        sip::store_visitor v(c);

        v.visit(*e,reinterpret_cast < sip::layout::element_identifier> (e));
      }

      impl->send_message(sip::message(c, sip::message_display_update));
    }

    void communicator::await_configuration() const {
      impl->await_message(sip::message_offer_configuration);
    }
  }
}

