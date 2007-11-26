// Author(s): Jeroen van der Wulp
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file include/mcrl2/utilities/squadt_interface.h

#ifndef MCRL2_SQUADT_INTERFACE_H_
#define MCRL2_SQUADT_INTERFACE_H_

#include <boost/cstdint.hpp>

#include <tipi/report.hpp>
#include <tipi/tool_display.hpp>
#include <tipi/configuration.hpp>
#include <tipi/tool/capabilities.hpp>

#include "mcrl2/utilities/tipi_ext.h"

#ifndef NO_MCRL2_TOOL_FACILITIES
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "mcrl2/print/messaging.h"
#endif

/// \cond INTERNAL_DOCS
namespace tipi {
  class configuration;

  namespace tool {

    class communicator;
    class capabilities;
  }
}
/// \endcond

namespace mcrl2 {
  namespace utilities {

    /// SQuADT specific utility functionality
    namespace squadt {
  
      /**
       * \brief A convenient interface for tool developers to make use of the
       * functionality provided by the tipi tool-side library.
       *
       * \note this interface is focussed on tools in the mCRL2 toolset most
       * notably the gs message layer relay mechanism presented above is activated.
       **/
      class tool_interface {
        friend class mcrl2_tool_interface;
  
        private:
  
          /** \brief whether or not the communicator is active */
          bool active;
  
        protected:
  
          /** \brief communicator object through which all communication with SQuADT will take place */
          tipi::tool::communicator* m_communicator;
  
        protected:
  
          /** \brief send notification message */
          void send_notification(std::string const&) const;
  
          /** \brief send status message */
          void send_report(const tipi::report::type, std::string const&);

          /** \brief send warning message */
          void send_warning(std::string const&) const;
  
          /** \brief send error message */
          void send_error(std::string const&) const;
  
          /** \brief sends a layout for the tool display */
          void send_display_layout(::tipi::layout::tool_display&);
  
          /** \brief sends a clear display layout signal */
          void send_clear_display();
  
          /** \brief sends a hide display layout signal */
          void send_hide_display();
  
        protected:
  
          /** \brief initialisation after connection to SQuADt */
          virtual void initialise();
  
          /** \brief configures tool capabilities */
          virtual void set_capabilities(tipi::tool::capabilities&) const = 0;
  
          /** \brief queries the user via SQuADT if needed to obtain configuration information */
          virtual void user_interactive_configuration(tipi::configuration&) = 0;
  
          /** \brief check an existing configuration object to see if it is usable */
          virtual bool check_configuration(tipi::configuration const&) const = 0;
  
          /** \brief performs the task specified by a configuration */
          virtual bool perform_task(tipi::configuration&) = 0;
  
          /** \brief finalisation after termination signal has been received */
          virtual void finalise();
  
          /** \brief virtual destructor */
          virtual ~tool_interface();
  
        private:
  
          /** \brief checks for a connection and if so starts the event loop */
          bool try_run();
  
        public: 
  
          /** \brief default constructor */
          tool_interface();
  
          /** \brief builds a connection with SQuADT */
          bool try_interaction(int&, char** const);
  
          /** \brief builds a connection with SQuADT */
          bool try_interaction(char*);
  
          /**  \brief whether or not the communicator is active (connected to SQuADT) */
          bool is_active() const;
      };

#ifdef __WXWINDOWS__
      /** 
       * \brief Convenience class for connecting wxWidgets applications to the environment (platform dependent wrapper around wxEntry)
       **/
      class entry_wrapper {
#ifdef __WINDOWS__
      private:
 
        HINSTANCE        hInstance;
        HINSTANCE        hPrevInstance;
        wxCmdLineArgType lpCmdLine;
        int              nCmdShow;
 
      public:
 
        entry_wrapper(HINSTANCE hc, HINSTANCE hp, wxCmdLineArgType lp, int ns) {
          hInstance     = hc;
          hPrevInstance = hp;
          lpCmdLine     = lp;
          nCmdShow      = ns;
        }
 
        bool perform_entry() {
          return (wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow) == 0);
        }
#else
      private:
 
        int&    argc;
        char**& argv;
 
      public:
 
        entry_wrapper(int& ac, char**& av) : argc(ac), argv(av) {
        }
 
        bool perform_entry() {
          return (wxEntry(argc, argv) == 0);
        }
#endif
      };
#endif

      /**
       * \brief mCRL2 tool specific squadt interface
       *
       * In addition to the interface above, the message system used by most
       * tools in the mCRL2 toolset is initialised such that messages are
       * relayed.
       *
       * \note only works for one object at a time
       **/
      class mcrl2_tool_interface : public tool_interface {
        template < typename T >
        friend void relay_message(const T, const char* data);

        private:

          /** \brief send status message, used only to relay messages */
          static boost::function< void (const tipi::report::type, std::string const&) > do_send_report;

          using tool_interface::send_report;

        protected:

          /** \brief initialisation after connection to SQuADt */
          void initialise();
  
          /** \brief finalisation after termination signal has been received */
          void finalise();
      };
  
#ifndef NO_MCRL2_TOOL_FACILITIES
      using ::mcrl2::utilities::messageType;
  
      /** \internal \brief Used to relay messages generated using core::print */
      template < >
      inline void relay_message(const messageType t, const char* data) {
        tipi::report::type report_type;
  
        switch (t) {
          case gs_notice:
            report_type = tipi::report::notice;
            break;
          case gs_warning:
            report_type = tipi::report::warning;
            break;
          case gs_error:
          default:
            report_type = tipi::report::error;
            break;
        }
      
        mcrl2_tool_interface::do_send_report(report_type, std::string(data));
      }  

      inline void mcrl2_tool_interface::initialise() {
        gsSetCustomMessageHandler(relay_message< messageType >);
  
        tipi::utility::logger::log_level l = tipi::utility::logger::get_default_filter_level();
  
        gsSetNormalMsg();
  
        if (2 < l) {
          gsSetVerboseMsg();
  
          if (3 < l) {
            gsSetDebugMsg();
          }
        }

        mcrl2_tool_interface::do_send_report = boost::bind(&tool_interface::send_report, this, _1, _2);
      }
  
      inline void mcrl2_tool_interface::finalise() {
        /* Unregister message relay */
        gsSetCustomMessageHandler(0);
      }
#endif

      inline void tool_interface::initialise() {
      }
  
      inline void tool_interface::finalise() {
      }

      /**
       * \brief Component that simplifies a connection attempt with squadt to a single call
       **/
      template < typename T >
      class interactor {

        public:
  
          /**
           * \brief builds a connection with SQuADT
           **/
          inline static bool free_activation(int& ac, char** const av) {
#ifdef __WXWINDOWS__
            int    dummy_ac = 0;
            char** dummy_av = 0;

            entry_wrapper starter(dummy_ac, dummy_av);

            T c(starter);
#else
            T c;
#endif
           
            return (c.try_interaction(ac, av));
          }

#ifdef __WINDOWS__
          inline static bool free_activation(HINSTANCE hInstance, HINSTANCE hPrevInstance, wxCmdLineArgType lpCmdLine, int nCmdShow) {
# ifdef __WXWINDOWS__
            wxCmdLineArgType dummy_lpCmdLine = 0;

            entry_wrapper starter(hInstance, hPrevInstance, dummy_lpCmdLine, nCmdShow);

            T c(starter);
# else
            T c;
# endif
            return (c.try_interaction(lpCmdLine));
          }
#endif
      };

      /** \brief Standard type for communication of rewrite strategy */
      extern boost::shared_ptr < tipi::datatype::enumeration > rewrite_strategy_enumeration;
  
      /// \internal Helper function for unsigned long to string conversion
      inline std::ostream& operator<<(std::ostream& o, unsigned long const& t) {
        char buf[21];
   
        sprintf(buf,"%lu",t);
   
        o << buf;
   
        return o;
      }
  
      /// \internal Helper function for unsigned long long to string conversion
      inline std::ostream& operator<<(std::ostream& o, unsigned long long const& t) {
        char buf[21];
   
        sprintf(buf,"%llu",t);
   
        o << buf;
   
        return o;
      }
    }
  }
}

#endif
