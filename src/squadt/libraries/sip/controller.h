#ifndef SIP_CONTROLLER_H
#define SIP_CONTROLLER_H

#include <sip/detail/common.h>
#include <sip/detail/controller_capabilities.h>
#include <sip/detail/tool_capabilities.h>
#include <sip/detail/display_layout.h>

/* Interface classes for the tool side of the Squadt Interaction Protocol */
namespace sip {
  namespace controller {

    /** \brief The main interface to the protocol (controller-side) */
    class communicator : public sip::messenger {
      protected:
        /** Type for keeping protocol phase status */
        enum status {
          status_initialising, ///< \brief No connection with tool yet
          status_clean,        ///< \brief Connection with tool: Phase 0
          status_configured,   ///< \brief Tool has accepted a configuration: Phase 1
          status_started,      ///< \brief Tool is running: Phase 2
          status_reported,     ///< \brief Tool is finished and has send a report: Phase 3
          status_error         ///< \brief An error occurred
        };
 
        /** \brief The current protocol status */
        status                      current_status;
 
        /** \brief The currently active display layout */
        sip::layout::display_layout current_layout;
 
        /** \brief The current configuration of a tool (may be limited to a main input configuration) */
        configuration::ptr          current_configuration;
 
      protected:

        /** \brief The capabilities object of the controller as it is send, when requested */
        static controller::capabilities current_controller_capabilities;

      protected:

        /** \brief convenience function for use with event handlers */
        inline void set_status(status);

      private:
 
        /** \brief Handler function to replace the current display layout with a new one */
        void accept_layout_handler(messenger::message_ptr&);
 
        /** \brief Handler function to map data to the display */
        void accept_data_handler(messenger::message_ptr&);
 
      public:

        /** \brief Default constructor */
        communicator();
 
        /** \brief Default destructor */
        ~communicator();
 
        /** \brief Send details about the amount of space that the controller currently has reserved for this tool */
        void reply_controller_capabilities();
 
        /** \brief Request the list of basic input configurations */
        void request_tool_capabilities();
 
        /** \brief Send a specification of a (perhaps partial) configuration */
        void send_configuration();
 
        /** \brief Send a layout specification for the display space reserved for this tool */
        void send_interaction_data();
 
        /** \brief Send a layout specification for the display space reserved for this tool */
        void send_start_signal();
 
        /** \brief Request the tool to terminate itself */
        void request_termination();
 
        /** \brief Get the controller_capabilities object that is send to tools */
        const controller::capabilities& get_controller_capabilities();
 
        /** \brief Set the current configuration (only effective before status_configured) */
        void set_configuration(const configuration&);
 
        /** \brief Get the current (perhaps partial) configuration */
        configuration::ptr get_configuration() const;
    };
 
    inline const controller::capabilities& communicator::get_controller_capabilities() {
      return (current_controller_capabilities);
    }
 
    /** \attention use get_configuration().swap() to set the configuration */
    inline configuration::ptr communicator::get_configuration() const {
      return (current_configuration);
    }
 
    inline void communicator::set_status(status s) {
      current_status = s;
    }
  }
}

#endif

