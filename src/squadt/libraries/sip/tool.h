#ifndef SIP_TOOL_H
#define SIP_TOOL_H

#include <set>

#include <sip/detail/common.h>
#include <sip/detail/tool_capabilities.h>
#include <sip/detail/controller_capabilities.h>
#include <sip/detail/layout_tool_display.h>
#include <sip/detail/layout_elements.h>
#include <sip/detail/schemes.h>
#include <sip/detail/report.h>

/* Interface classes for the tool side of the Squadt Interaction Protocol */
namespace sip {
  namespace tool {

    class communicator;
 
    /** \brief The main interface to the protocol implementation (tool-side) */
    class communicator : public sip::messenger {
      friend class messaging::scheme< sip::message >;
      friend class layout::element;
 
      private:

        /** Type for keeping protocol phase status */
        typedef enum {
          status_inactive,     ///< \brief Inactive
          status_initialising, ///< \brief No connection with controller yet
          status_clean,        ///< \brief Connection with controller: Phase 0
          status_configured,   ///< \brief Tool has accepted a configuration: Phase 1
          status_started,      ///< \brief Tool is running: Phase 2
          status_reported,     ///< \brief Tool is finished and has send a report: Phase 3
          status_error         ///< \brief An error occurred
        } status;
 
        /** \brief The current protocol status */
        status                        current_status;
 
        /** \brief The last received set of controller capabilities */
        controller::capabilities::ptr current_controller_capabilities;
 
        /** \brief The object that descibed the capabilities of the current tool */
        tool::capabilities            current_tool_capabilities;
 
        /** \brief This object reflects the current configuration */
        configuration::sptr           current_configuration;
 
        /** \brief Unique identifier for the running tool, obtained via the command line */
        long                          instance_identifier;
 
      private:

        /** \brief Handler for incoming data resulting from user interaction with the display relayed by the controller */
        void accept_display_data(const sip::messenger::message_ptr&, layout::tool_display::sptr);

        /** \brief Send data to update the state of the last communicated display layout */
        void send_display_data(layout::element const*);
 
        /** \brief Signal that the current configuration is complete enough for the tool to start processing */
        void send_accept_configuration(sip::configuration&);
 
        /** \brief Set the current tool configuration object */
        inline void set_configuration(configuration::sptr);
 
      public:
 
        /** \brief Default constructor */
        communicator();
 
        /** \brief Constructor that takes controller connection arguments from the command line */
        bool activate(int&, char**);
 
        /** \brief Default destructor */
        ~communicator();
 
        /** \brief Request details about the amount of space that the controller currently has reserved for this tool */
        void request_controller_capabilities();
 
        /** \brief Request the list of basic input configurations */
        void reply_tool_capabilities();
 
        /** \brief Signal that the current configuration is complete enough for the tool to start processing */
        void send_accept_configuration();

        /** \brief Send a layout specification for the display space reserved for this tool */
        void send_display_layout(layout::tool_display::sptr);
 
        /** \brief Sends the empty layout specification for the display space */
        void clear_display();
 
        /** \brief Send a signal that the tool has finished its last operation */
        void send_signal_done();
 
        /** \brief Send a signal that the tool is about to terminate */
        void send_signal_termination();
 
        /** \brief Send a status report to the controller */
        void send_report(sip::report const&);

        /** \brief Sends an error report to the controller */
        void send_status_report(sip::report::type, std::string const&);
 
        /** \brief Get the tool capabilities object that will be sent when a request is received */
        inline tool::capabilities& get_tool_capabilities();
 
        /** \brief Get the current tool configuration object be sent when a request is received */
        inline configuration& get_configuration();
 
        /** \brief Get the last communicated set of controller capabilities */
        inline const controller::capabilities::ptr get_controller_capabilities() const;

        /** \brief Waits until a configuration is delivered and returns a pointer to it */
        const configuration::sptr await_configuration();
 
        /** \brief Returns whether the current status is equal to status_inactive */
        inline bool is_active() const;

        /** \brief Extract a configuration from an offer_configuration message */
        inline configuration::sptr operator<< (const sip::messenger::message_ptr& m);
    };

    /**
     * @param[in] m shared pointer reference to an offer_configuration message
     * @param[in] c reference to a configuration object
     **/
    inline configuration::sptr communicator::operator<< (const sip::messenger::message_ptr& m) {
      assert(m->get_type() == sip::message_offer_configuration);

      if (m.get() != 0) {
        current_configuration = sip::configuration::read(m->to_string());
      }

      return (current_configuration);
    }
 
    /**
     * \return a pointer to the tool capabilities object that is sent to the controller on request
     **/
    inline tool::capabilities& communicator::get_tool_capabilities() {
      return (current_tool_capabilities);
    }
 
    /**
     * This object can be stored by the controller and subsequently be used to
     * restore this exact configuration state at the side of the tool.
     *
     * \return a reference to the current tool configuration object
     **/
    inline configuration& communicator::get_configuration() {
      return (*current_configuration);
    }
 
    inline void communicator::set_configuration(configuration::sptr c) {
      current_configuration = c;
    }
 
    /**
     * \pre status is not status_initialising
     * \return p which is a pointer to the most recently retrieved controller capabilities object or 0
     **/
    inline const controller::capabilities::ptr communicator::get_controller_capabilities() const {
      if (current_controller_capabilities.get() == 0) {
        throw (sip::exception(sip::controller_capabilities_unknown));
      }
 
      return (current_controller_capabilities);
    }
 
    inline bool communicator::is_active() const {
      return (current_status != status_inactive);
    }
  }
}

#include <sip/detail/basic_messenger.tcc>

#endif
