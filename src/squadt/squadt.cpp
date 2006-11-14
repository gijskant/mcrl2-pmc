#include <fstream>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

#define SQUADT_IMPORT_STATIC_DEFINITIONS

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "exception.h"
#include "tool_manager.h"
#include "settings_manager.tcc"
#include "build_system.h"

#include "gui_splash.h"
#include "gui_main.h"

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/thread.h>
#include <wx/cmdline.h>
#include <wx/msgdlg.h>

const char* program_name    = "squadt";
const char* program_version = "0.2.1";

/**
 * \namespace squadt
 *
 * The global namespace for all squadt components.
 **/
namespace squadt {

  /** \brief Global Settings Manager component */
  boost::shared_ptr < settings_manager > global_settings_manager;

  /** \brief Global Tool Manager component */
  boost::shared_ptr < tool_manager >     global_tool_manager;
}

using namespace squadt::GUI;

class initialisation : public wxThread {

  private:

    splash* splash_window;

  public:

    initialisation(splash* s) : wxThread(wxTHREAD_JOINABLE), splash_window(s) {
      Create();
      Run();
    }

    void* Entry() {
      squadt::global_tool_manager->query_tools(
                    boost::bind(&splash::set_operation, splash_window, std::string("processing"), _1));

      return (0);
    }
};

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
      if (parser.Found(wxT("c"))) {
        if (0 < parser.GetParamCount()) {
          action = boost::bind(&squadt::GUI::main::project_new, _1, std::string(parser.GetParam(0).fn_str()), std::string());
        }
        else {
          std::cerr << "Fatal: found -c, or --create option so expected path argument\n" << parser.GetParam(0).fn_str();

          return (false);
        }
      }
      else if (0 < parser.GetParamCount()) {
        action = boost::bind(&squadt::GUI::main::project_open, _1, std::string(parser.GetParam(0).fn_str()));
      }
      if (parser.Found(wxT("d"))) {
        sip::controller::communicator::get_standard_error_logger()->set_filter_level(3);
      }
      if (parser.Found(wxT("v"))) {
        sip::controller::communicator::get_standard_error_logger()->set_filter_level(2);
      }
      if (parser.Found(wxT("h"))) {
        std::cout << "Usage: " << program_name << " [OPTION] [PATH]\n"
                  << "Graphical environment that provides a uniform interface for using all kinds\n"
                  << "of other connected tools.\n"
                  << "\n"
                  << "Mandatory arguments to long options are mandatory for short options too.\n"
                  << "  -c, --create          create new project in PATH\n"
                  << "  -d, --debug           produce lots of debug output\n"
                  << "  -h, --help            display this help message\n"
                  << "  -q, --quiet           represses unnecessary output\n"
                  << "  -v, --verbose         display additional information during operation\n"
                  << "      --version         display version information\n"
                  << "\n";
 
        return (false);
      }
      if (parser.Found(wxT("q"))) {
        sip::controller::communicator::get_standard_error_logger()->set_filter_level(1);
      }
      if (parser.Found(wxT("version"))) {
        std::cerr << program_name << " " << program_version << " (revision " << REVISION << ")" << std::endl;
 
        return (false);
      }
    }
  }

  return (c);
}

/* Squadt class declaration */
class Squadt : public wxApp {
  public:

    virtual bool OnInit();
    virtual int  OnExit();
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
    global_settings_manager = settings_manager::ptr(new settings_manager(wxFileName::GetHomeDir().fn_str()));
 
    wxInitAllImageHandlers();
 
    #include "pixmaps/logo.xpm"

    wxImage logo(logo_xpm);
 
    splash* splash_window = new splash(&logo, 1);
 
    try {
      global_tool_manager = tool_manager::read();

      splash_window->set_category("Querying tools", global_tool_manager->number_of_tools());
     
      /* Perform initialisation */
      initialisation ti(splash_window);
     
      /* Cannot just wait because the splash would not be updated */
      while (ti.IsAlive()) {
        splash_window->update();
     
        wxApp::Yield();
      }

      splash_window->set_category("Initialising components");

      /* Initialise main application window */
      SetTopWindow(new squadt::GUI::main());
     
      if (action) {
        action(static_cast < squadt::GUI::main* > (GetTopWindow()));
      }

      SetUseBestVisual(true);

      /* Disable splash */
      splash_window->set_done();
    }
    catch (sip::listening_exception& e) {
      /* Disable splash */
      splash_window->set_done();

      wxMessageDialog error_dialog(0, wxString(e.what(), wxConvLocal), wxT("Fatal"), wxOK|wxICON_ERROR);
      
      error_dialog.ShowModal();

      return (false);
    }
  }

  return (c);
}

int Squadt::OnExit() {
  return (wxApp::OnExit());
}
