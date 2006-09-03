#ifndef SIP_TOOL_H
#define SIP_TOOL_H

#include <set>

#include <sip/detail/tool_capabilities.h>
#include <sip/detail/controller_capabilities.h>
#include <sip/detail/layout_tool_display.h>
#include <sip/detail/layout_elements.h>
#include <sip/detail/report.h>
#include <utility/logger.h>

/* Interface classes for the tool side of the SQuADt Interaction Protocol */
namespace sip {
  namespace tool {

    class communicator_impl;

    /** \brief The main interface to the protocol implementation (tool-side) */
    class communicator {
      friend class layout::element;
      friend class communicator_impl;

      protected:
 
        /** \brief Implementation object, that contains the real functionality */
        boost::shared_ptr < communicator_impl > impl;
 
      private:

        /** \brief Send data to update the state of the last communicated display layout */
        void send_display_data(layout::element const*);
 
        /** \brief Signal that the current configuration is complete enough for the tool to start processing */
        void send_accept_configuration(sip::configuration&);
 
        /** \brief Set the current tool configuration object */
        inline void set_configuration(configuration::sptr);

      protected:
 
        /** \brief Alternate constructor */
        communicator(communicator_impl*);

      public:
 
        /** \brief Default constructor */
        communicator();
 
        /** \brief Constructor that takes controller connection arguments from the command line */
        bool activate(int&, char**);
 
        /** \brief Request details about the amount of space that the controller currently has reserved for this tool */
        void request_controller_capabilities();
 
        /** \brief Signal that the current configuration is complete enough for the tool to start processing */
        void send_accept_configuration();

        /** \brief Send a layout specification for the display space reserved for this tool */
        void send_display_layout(layout::tool_display::sptr);
 
        /** \brief Sends the empty layout specification for the display space */
        void send_clear_display();
 
        /** \brief Send a signal that the tool has finished its last operation */
        void send_signal_done();
 
        /** \brief Send a signal that the tool is about to terminate */
        void send_signal_termination();
 
        /** \brief Send a status report to the controller */
        void send_report(sip::report const&);

        /** \brief Sends an error report to the controller */
        void send_status_report(sip::report::type, std::string const&);
 
        /** \brief Get the tool capabilities object that will be sent when a request is received */
        tool::capabilities& get_tool_capabilities();
 
        /** \brief Get the current tool configuration object be sent when a request is received */
        configuration& get_configuration();
 
        /** \brief Get the last communicated set of controller capabilities */
        const controller::capabilities::ptr get_controller_capabilities() const;

        /** \brief Waits until a configuration is delivered and returns a pointer to it */
        void await_configuration() const;
 
        /** \brief Waits for the first message with a specific type to arrive */
        const sip::message_ptr await_message(sip::message::type_identifier_t);

        /** \brief Set the handler for a type */
        void add_handler(const sip::message::type_identifier_t, sip::message_handler_type);

        /** \brief Remove a specific handlers for a message type */
        void remove_handler(const sip::message::type_identifier_t, sip::message_handler_type);

        /** \brief Get the logger instance used for this communicator */
        utility::logger* get_logger() const;

        /** \brief Gets the associated logger object */
        static utility::logger* get_standard_error_logger();
    };
  }
}

#endif
