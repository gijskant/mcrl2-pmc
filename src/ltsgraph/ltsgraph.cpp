#include <mcrl2_revision.h>
#include "ltsgraph_version.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include "lts/liblts.h"

// Graphical shell
#include "graph_frame.h"

std::string lts_file_argument;
bool command_line = false;

#ifdef ENABLE_SQUADT_CONNECTIVITY
// SQuADT protocol interface
# include <squadt_utility.h>

class squadt_interactor: public squadt_tool_interface {
  
  private:
    /* Constants for identifiers of options and objects */
    enum identifiers {
      lts_file_for_input // Main input file that contains an lts
    };
 
    boost::function<int()> startup_function;

  public:
    // Constructor
    squadt_interactor(boost::function<bool()>);

    // Configures tool capabilities.
    void set_capabilities(sip::tool::capabilities&) const;

    // Queries the user via SQuADt if needed to obtain configuration information
    void user_interactive_configuration(sip::configuration&);

    // Check an existing configuration object to see if it is usable
    bool check_configuration(sip::configuration const&) const;

    // Performs the task specified by a configuration
    bool perform_task(sip::configuration&);
};

squadt_interactor::squadt_interactor(boost::function<bool()> startup): startup_function(startup) {
  // skip 
}

void squadt_interactor::set_capabilities(sip::tool::capabilities& c) const {
  c.add_input_combination(lts_file_for_input, "Visualisation", "aut");
  c.add_input_combination(lts_file_for_input, "Visualisation", "svc");
#ifdef MCRL2_BCG
  c.add_input_combination(lts_file_for_input, "Visualisation", "bcg");
  //Untestable
#endif  
}

void squadt_interactor::user_interactive_configuration(sip::configuration& c) {
  //skip
}


bool squadt_interactor::check_configuration(sip::configuration const& c) const {
  if (c.object_exists(lts_file_for_input)) {
    /* The input object is present, verify whether the specified format is supported */
    sip::object::sptr input_object = c.get_object(lts_file_for_input);
    lts_file_argument = input_object->get_location();


    lts_type t = lts::parse_format(input_object->get_format().c_str());

    if (t == lts_none) {
      send_error(boost::str(boost::format("Invalid configuration: unsupported type `%s' for main input") % lts::string_for_type(t)));
      return false;
    }
  }

  else {
    return false;
  }

  
  return true;
  
}

bool squadt_interactor::perform_task(sip::configuration&) {

  return startup_function() == 0;
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
      if(command_line) {
        // Not SQuADT-connected, parse commandline
        wxCmdLineParser cmdln(argc,argv);
	   
        cmdln.AddSwitch(wxT("h"),wxT("help"),wxT("displays this message"));
        cmdln.AddSwitch(wxEmptyString,wxT("version"), wxT("displays version information and exits"));
        cmdln.AddParam(wxT("INFILE"),wxCMD_LINE_VAL_STRING,wxCMD_LINE_PARAM_OPTIONAL);
        cmdln.SetLogo(wxT("Graphical tool for visualizing graph."));

        if ( cmdln.Parse() ) {
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
      }    

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
int wx_entry_proxy(HINSTANCE hInstance, 
                   HINSTACE hPrevInstance, 
                   wxCmdLineArgType lpCmdLine,
                   int nCmdShow) {

  return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);    
}

extern "C" int WINAPI WinMain(HINSTANCE hInstance,                    
                                  HINSTANCE hPrevInstance,                
                                  wxCmdLineArgType lpCmdLine,             
                                  int nCmdShow)                           
    {                                                                     
        ATerm bot;

        ATinit(0,NULL,&bot); // XXX args?
#ifdef ENABLE_SQUADT_CONNECTIVITY
        squadt_interactor c(boost::bind (wx_entry_proxy, hInstance, hPrevInstance, lpCmdLine, nCmdShow));
        if (!c.try_interaction(0, NULL)) {
#endif
          return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);    

#ifdef ENABLE_SQUADT_CONNECTIVITY
        }
        return 0;
#endif

    }
#else

int wx_entry_proxy(int argc, char** argv) {
 return wxEntry(argc, argv);
}

int main(int argc, char **argv)
{
  ATerm bot;
  ATinit(argc,argv,&bot);
#ifdef ENABLE_SQUADT_CONNECTIVITY
  squadt_interactor c(boost::bind(wx_entry_proxy, argc, argv));
  if(!c.try_interaction(argc, argv)) {
    command_line = true;
#endif
    /* On purpose we do not catch exceptions */

    return wxEntry(argc, argv);
#ifdef ENABLE_SQUADT_CONNECTIVITY
  }
  return 0;
#endif

}
#endif





