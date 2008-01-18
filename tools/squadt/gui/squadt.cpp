// Author(s): Jeroen van der Wulp
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file gui/squadt.cpp
/// \brief Add your file description here.

#define NAME "squadt"
#define AUTHOR "Jeroen van der Wulp"

#include <boost/bind.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/thread/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "mcrl2/utilities/version_info.h"

#include "settings_manager.hpp"
#include "tool_manager.hpp"
#include "build_system.hpp"
#include "executor.hpp"

#include "gui/splash.hpp"
#include "gui/main.hpp"

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/cmdline.h>
#include <wx/msgdlg.h>
#include <wx/sysopt.h>

using namespace squadt::GUI;

bool parse_command_line(int argc, wxChar** argv, boost::function < void (squadt::GUI::main*) >& action) {
  bool c = 0 < argc;

  if (c) {
    wxCmdLineParser parser(argc, argv); 
 
    parser.AddSwitch(wxT("c"),wxT("create"),wxT(""));
    parser.AddSwitch(wxT("d"),wxT("debug"),wxT(""));
    parser.AddSwitch(wxT("h"),wxT("help"),wxT(""));
    parser.AddSwitch(wxT("q"),wxT("quiet"),wxT(""));
    parser.AddSwitch(wxT("v"),wxT("verbose"),wxT(""));
    parser.AddSwitch(wxT(""),wxT("version"),wxT(""));
    parser.AddParam(wxEmptyString,wxCMD_LINE_VAL_STRING,wxCMD_LINE_PARAM_OPTIONAL);
 
    c = parser.Parse(false) == 0;
 
    if (c) {
      tipi::utility::logger::log_level default_log_level = 1;

      if (0 < parser.GetParamCount()) {
        boost::filesystem::path target(std::string(parser.GetParam(0).fn_str()));

        if (!target.has_root_path()) {
          target = boost::filesystem::initial_path() / target;
        }

        if (parser.Found(wxT("c"))) {
          action = boost::bind(&squadt::GUI::main::project_new, _1, target.string(), std::string());
        }
        else {
          action = boost::bind(&squadt::GUI::main::project_open, _1, target.string());
        }
      }
      if (parser.Found(wxT("c"))) {
        std::cerr << "Fatal: found -c, or --create option so expected path argument\n" << std::string(parser.GetParam(0).fn_str());
       
        return (false);
      }
      if (parser.Found(wxT("d"))) {
        default_log_level = 3;
      }
      if (parser.Found(wxT("v"))) {
        default_log_level = 2;
      }
      if (parser.Found(wxT("h"))) {
        std::cout <<
        "Usage: " << std::string(wxString(argv[0]).fn_str()) << " [OPTION]... [PATH]\n"
        "Graphical environment that provides a uniform interface for using all kinds of\n"
        "other connected tools. If PATH is provided, it provides an existing project in\n"
        "PATH.\n"
        "\n"
        "Options:\n"
        "  -c, --create          create new project in PATH\n"
        "  -h, --help            display this help message\n"
        "      --version         display version information\n"
        "  -q, --quiet           represses unnecessary output\n"
        "  -v, --verbose         display additional information during operation\n"
        "  -d, --debug           produce lots of debug output\n"
        "\n"
        "Report bugs at <http://www.mcrl2.org/issuetracker>.\n"
        ;
 
        return (false);
      }
      if (parser.Found(wxT("q"))) {
        default_log_level = 0;
      }
      if (parser.Found(wxT("version"))) {
        print_version_information(NAME, AUTHOR);
        return (false);
      }

      tipi::controller::communicator::get_default_logger().set_default_filter_level(default_log_level);
      tipi::controller::communicator::get_default_logger().set_filter_level(default_log_level);
    }
  }

  return (c);
}

/* Squadt class declaration */
class Squadt : public wxApp {
  public:

    bool OnInit();
    int  OnExit();
};

IMPLEMENT_APP(Squadt)

/*
 * Squadt class implementation
 *
 * Must return true because static initialisation might not have completed
 */
bool Squadt::OnInit() {
  using namespace squadt;
  using namespace squadt::GUI;

  boost::function < void (squadt::GUI::main*) > action;

  bool c = parse_command_line(argc, argv, action);

  if (c) {
    wxInitAllImageHandlers();

    wxSystemOptions::SetOption(wxT("msw.remap"), 0);
    
    splash* splash_window = new splash(1);

    try {
      struct local {
        static void initialise_tools(splash& splash_window, bool& finished, bool& too_many_tools_failed) {
          try {
            squadt::global_build_system.get_tool_manager()->query_tools(
              boost::bind(&splash::set_operation, &splash_window, "", _1));
          }
          catch (...) {
            too_many_tools_failed = true;
          }

          finished = true;
        }
      };

      try {
        std::auto_ptr < settings_manager > global_settings_manager(new settings_manager(std::string(wxFileName::GetHomeDir().fn_str())));

        // Open log file
        tipi::controller::communicator::get_default_logger().set_filter_level(3);
        tipi::controller::communicator::get_default_logger().redirect(global_settings_manager->path_to_user_settings().append("/log"));

        global_build_system.initialise(
          global_settings_manager,
          std::auto_ptr < tool_manager > (new tool_manager()),
          std::auto_ptr < executor > (new executor()),
          std::auto_ptr < type_registry > (new type_registry()));
      }
      catch (std::exception& e) {
        wxMessageDialog(0, wxT("Initialisation error!\n\n") + wxString(e.what(), wxConvLocal), wxT("Fatal"), wxOK|wxICON_ERROR).ShowModal();

        return false;
      }
     
      splash_window->set_category("Querying tools", global_build_system.get_tool_manager()->number_of_tools());
     
      /* Perform initialisation */
      bool finished              = false;
      bool too_many_tools_failed = false;

      boost::thread initialisation_thread(boost::bind(&local::initialise_tools,
                        boost::ref(*splash_window), boost::ref(finished), boost::ref(too_many_tools_failed)));

      /* Cannot just wait because the splash would not be updated */
      do {
        wxApp::Yield();

        splash_window->update();
      }
      while (!finished);

      if (too_many_tools_failed) {
        wxMessageDialog retry(0, wxT("Do you want to replace the current list of known tools with the default set and retry?"),
                wxT("Initialisation of multiple tools failed!"), wxYES_NO|wxICON_WARNING);

        if (retry.ShowModal() == wxID_YES) {
          // Reset list of known tools
          global_build_system.get_tool_manager()->factory_configuration();

          // Perform initialisation
          boost::thread reinitialisation_thread(boost::bind(&local::initialise_tools,
                                boost::ref(*splash_window), boost::ref(finished), boost::ref(too_many_tools_failed)));

          finished = false;
       
          do {
            wxApp::Yield();

            splash_window->update();
          }
          while (!finished);
        }
      }

      splash_window->set_category("Initialising components");

      global_build_system.get_type_registry()->rebuild_indices();

      /* Disable splash */
      splash_window->set_done();

      /* Initialise main application window */
      SetTopWindow(new squadt::GUI::main());
     
      if (action) {
        action(static_cast < squadt::GUI::main* > (GetTopWindow()));
      }

      SetUseBestVisual(true);
    }
    catch (...) {
      splash_window->set_done();

      wxMessageDialog(0, wxT("Initialisation failed! Another instance, or tool related to a previous instance, is probably still active and blocking the initialisation."), wxT("Fatal"), wxOK|wxICON_ERROR).ShowModal();

      return (false);
    }
  }

  return (c);
}

int Squadt::OnExit() {
  return wxApp::OnExit();
}
