// Author(s): Jeroen van der Wulp
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file source/squadt_interface.cpp
/// \brief Add your file description here.

#define NO_MCRL2_TOOL_FACILITIES
#include "mcrl2/utilities/squadt_interface.h"
#include "tipi/utility/logger.hpp"

using namespace mcrl2::utilities;

namespace mcrl2 {
  namespace utilities {
    namespace squadt {
      tipi::tool::communicator* mcrl2_tool_interface::communicator;

      bool tool_interface::try_run() {
        if (active) {
          bool valid_configuration_present = false;
          bool termination_requested       = false;
  
          initialise();
  
          try {
            while (!termination_requested) {
              switch (m_communicator.await_message(tipi::message_any)->get_type()) {
                case tipi::message_configuration: {
                    tipi::configuration& configuration = m_communicator.get_configuration();
           
                    /* Insert configuration in tool communicator object */
                    valid_configuration_present = check_configuration(configuration);
           
                    if (configuration.is_fresh()) {
                      do {
                        user_interactive_configuration(configuration);
           
                        /* Insert configuration in tool communicator object */
                        valid_configuration_present = check_configuration(configuration);
                   
                      } while (!valid_configuration_present);
                    }
           
                    /* Signal that the configuration is acceptable */
                    m_communicator.send_configuration(configuration);
                  }
                  break;
                case tipi::message_task_start:
                  if (valid_configuration_present) {
                    tipi::configuration& configuration = m_communicator.get_configuration();

                    /* Signal that the job is finished */
                    bool result = perform_task(configuration);
  
                    if (!result) {
                      send_error("Operation failed; tool may have crashed!");
                    }
                    else {
                      m_communicator.send_configuration(configuration);
                    }
  
                    m_communicator.send_signal_done(result);
                  }
                  else {
                    send_error("Start signal received without valid configuration!");
                  }
                  break;
                case tipi::message_termination:
           
                  termination_requested = true;
           
                  break;
                default:
                  /* Messages with a type that do not need to be handled */
                  break;
              }
            }
          }
          catch (std::exception& e) {
            /* Handle standard exceptions */
            send_error(std::string("Caught exception: ") + e.what());
          }
  
          finalise();
  
          m_communicator.send_signal_termination();

          m_communicator.disconnect();
  
          active = false;
  
          return (true);
        }
  
        return (false);
      }
  
      bool tool_interface::try_interaction(char* av) {
        set_capabilities(m_communicator.get_tool_capabilities());
  
        active = m_communicator.activate(av);
  
        return (try_run());
      }
  
      /**
       * The connection is built using information such as a socket identifier when
       * found among the command line arguments. The command line arguments that
       * are SQuADT specific are filtered out.
       *
       * \param[in] ac the number of command line arguments
       * \param[in] av a pointer to an array of the actual command line arguments
       *
       * \return whether or not SQuADT interaction was successful
       **/
      bool tool_interface::try_interaction(int& ac, char** const av) {
  
        set_capabilities(m_communicator.get_tool_capabilities());
  
        active = m_communicator.activate(ac,av);
  
        return (try_run());
      }
  
      bool tool_interface::is_active() const {
        return (active);
      }
  
      void tool_interface::send_notification(std::string const& m) const {
        m_communicator.send_status_report(tipi::report::notice, m);
      }
  
      void tool_interface::send_warning(std::string const& m) const {
        m_communicator.send_status_report(tipi::report::warning, m);
      }
  
      void tool_interface::send_error(std::string const& m) const {
        m_communicator.send_status_report(tipi::report::error, m);
      }
  
      void tool_interface::send_display_layout(std::auto_ptr < tipi::layout::manager >& p) {
        m_communicator.send_display_layout(tipi::layout::tool_display::create(p));
      }
  
      void tool_interface::send_clear_display() {
        m_communicator.send_clear_display();
      }
  
      void tool_interface::send_hide_display() {
        boost::shared_ptr < tipi::layout::tool_display > p(new tipi::layout::tool_display());
  
        p->show(false);
  
        m_communicator.send_display_layout(p);
      }
  
      boost::shared_ptr < tipi::datatype::enumeration > rewrite_strategy_enumeration;
  
      static bool initialise () {
        rewrite_strategy_enumeration.reset(new tipi::datatype::enumeration("inner"));
        *rewrite_strategy_enumeration % "innerc" % "jitty" % "jittyc";
  
        return true;
      }
  
      bool initialised = initialise();
    }
  }
}
