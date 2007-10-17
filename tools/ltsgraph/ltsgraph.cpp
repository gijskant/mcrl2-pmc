//  Copyright 2007 Didier Le Lann, Carst Tankink, Muck van Weerdenburg and Jeroen van der Wulp. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ./ltsgraph.cpp

// Graphical shell
#include "graph_frame.h"

#include "ltsgraph_version.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <mcrl2/lts/liblts.h>

std::string lts_file_argument;

#ifdef ENABLE_SQUADT_CONNECTIVITY
bool command_line = false;

// SQuADT protocol interface
# include <mcrl2/utilities/squadt_interface.h>

class squadt_interactor: public mcrl2::utilities::squadt::mcrl2_tool_interface {
  
  private:

    static const char*  lts_file_for_input;  ///< file containing an LTS that can be imported
 
    // Wrapper for wxEntry invocation
    mcrl2::utilities::squadt::entry_wrapper& starter;

  public:
    // Constructor
    squadt_interactor(mcrl2::utilities::squadt::entry_wrapper&);

    // Configures tool capabilities.
    void set_capabilities(tipi::tool::capabilities&) const;

    // Queries the user via SQuADt if needed to obtain configuration information
    void user_interactive_configuration(tipi::configuration&);

    // Check an existing configuration object to see if it is usable
    bool check_configuration(tipi::configuration const&) const;

    // Performs the task specified by a configuration
    bool perform_task(tipi::configuration&);
};

const char* squadt_interactor::lts_file_for_input  = "lts_in";

squadt_interactor::squadt_interactor(mcrl2::utilities::squadt::entry_wrapper& w): starter(w) {
}

void squadt_interactor::set_capabilities(tipi::tool::capabilities& c) const {
  c.add_input_combination(lts_file_for_input, tipi::mime_type("aut", tipi::mime_type::text), tipi::tool::category::visualisation);
  c.add_input_combination(lts_file_for_input, tipi::mime_type("svc", tipi::mime_type::application), tipi::tool::category::visualisation);
  c.add_input_combination(lts_file_for_input, tipi::mime_type("svc+mcrl", tipi::mime_type::application), tipi::tool::category::visualisation);
  c.add_input_combination(lts_file_for_input, tipi::mime_type("svc+mcrl2", tipi::mime_type::application), tipi::tool::category::visualisation);
#ifdef MCRL2_BCG
  c.add_input_combination(lts_file_for_input, tipi::mime_type("bcg", tipi::mime_type::application), tipi::tool::category::visualisation);
#endif  
}

void squadt_interactor::user_interactive_configuration(tipi::configuration& c) {
  //skip
}

bool squadt_interactor::check_configuration(tipi::configuration const& c) const {
  if (c.input_exists(lts_file_for_input)) {
    /* The input object is present, verify whether the specified format is supported */
    tipi::object input_object(c.get_input(lts_file_for_input));

    lts_file_argument = input_object.get_location();

    if (lts::parse_format(input_object.get_mime_type().get_sub_type().c_str()) == lts_none) {
      send_error(boost::str(boost::format("Invalid configuration: unsupported type `%s' for main input") % input_object.get_mime_type().get_sub_type().c_str()));
      return false;
    }
  }
  else {
    return false;
  }
  
  return true;
}

bool squadt_interactor::perform_task(tipi::configuration&) {
  return starter.perform_entry();
}
#endif

void print_help() {
  cout << "Usage: ltsgraph [INFILE]\n"
       << "Draw graphs and optimize their layout in a graphical environment. If INFILE (LTS file : *.aut or *.svc) is supplied \n"
       << "the tool will use this file as input for drawing.\n"
       << "\n"
       << "Use left click to drag the nodes and right click to fix the nodes. \n"
       << "\n"
       << "Mandatory arguments to long options are mandatory for short options too.\n"
       << "  -h, --help            display this help message\n"
       << "  --version             displays version information and exits \n";
}

void print_version() {
  cout << "ltsgraph version " << LTSG_VERSION << "\n"
       << "Part of mCRL2 toolset revision " << REVISION << "\n";
}

class GraphApp : public wxApp
{
  public:
    bool OnInit() {

      gsEnableConstructorFunctions();
#ifdef ENABLE_SQUADT_CONNECTIVITY
      if(command_line) {
#endif
        // Not SQuADT-connected, parse commandline
        wxCmdLineParser cmdln(argc,argv);
	   
        cmdln.AddSwitch(wxT("h"),wxT("help"),wxT("displays this message"));
        cmdln.AddSwitch(wxEmptyString,wxT("version"), wxT("displays version information and exits"));
        cmdln.AddParam(wxT("INFILE"),wxCMD_LINE_VAL_STRING,wxCMD_LINE_PARAM_OPTIONAL);
        cmdln.SetLogo(wxT("Graphical tool for visualizing graph."));

        if ( cmdln.Parse()) {
          return false;
        }

        if ( cmdln.Found(wxT("h")) ) {
          print_help();
          return false;
        }

        if (cmdln.Found(wxT("version")) ) {
          print_version();
          return false;
        }

        if ( cmdln.GetParamCount() > 0 ) {
          lts_file_argument = std::string(cmdln.GetParam(0).fn_str());
        }
#ifdef ENABLE_SQUADT_CONNECTIVITY
      }    
#endif

      init_frame(lts_file_argument);
      return true;
    }

    int OnExit() {
      return (wxApp::OnExit());
    }

  private:	
    void init_frame(std::string lts_file_argument) {
      GraphFrame *frame;
	
      frame = new GraphFrame(wxT("ltsgraph"), wxPoint(150, 150), wxSize(INITIAL_WIN_WIDTH, INITIAL_WIN_HEIGHT));
      frame->Show(true);
      frame->GetSizer()->RecalcSizes();
      if (!lts_file_argument.empty()) {
        frame->Init(wxString(lts_file_argument.c_str(), wxConvLocal));
      }
    }
};


IMPLEMENT_APP_NO_MAIN(GraphApp)
IMPLEMENT_WX_THEME_SUPPORT

#ifdef __WINDOWS__
extern "C" int WINAPI WinMain(HINSTANCE hInstance,                    
                                  HINSTANCE hPrevInstance,                
                                  wxCmdLineArgType lpCmdLine,             
                                  int nCmdShow) {
    ATerm bot;

    ATinit(0,0,&bot); // XXX args?
# ifdef ENABLE_SQUADT_CONNECTIVITY
    mcrl2::utilities::squadt::entry_wrapper starter(hInstance, hPrevInstance, lpCmdLine, nCmdShow);

    squadt_interactor c(starter);

    if (!c.try_interaction(lpCmdLine)) {
      return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);    
    }

    return 0;
# else
    return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);    
# endif
}
#else
int main(int argc, char **argv)
{
  ATerm bot;
  ATinit(argc,argv,&bot);
# ifdef ENABLE_SQUADT_CONNECTIVITY
  mcrl2::utilities::squadt::entry_wrapper starter(argc, argv);

  squadt_interactor c(starter);

  if(!c.try_interaction(argc, argv)) {
    command_line = true;

    return wxEntry(argc, argv);
  }

  return 0;
# else
  return wxEntry(argc, argv);
# endif
}
#endif
