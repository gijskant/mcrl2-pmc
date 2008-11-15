// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file xsim.cpp

#include "wx.hpp" // precompiled headers

#define NAME "xsim"
#define AUTHOR "Muck van Weerdenburg"

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "xsim.h"
#endif

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include <cstring>
#include <iostream>
#include <wx/msgdlg.h>
#include <wx/string.h>
#include <aterm2.h>
#include "xsimmain.h"
#include "mcrl2/core/detail/struct.h"
#include "mcrl2/data/rewrite.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/core/aterm_ext.h"
#include "mcrl2/utilities/command_line_interface.h"

std::string get_about_message() {
  static const std::string version_information =
        mcrl2::utilities::interface_description("", NAME, AUTHOR, "", "").
                                                        version_information() +
           std::string("\n"
           "This tool is part of the mCRL2 toolset.\n"
           "For information see http://www.mcrl2.org\n"
           "\n"
           "For feature requests or bug reports,\n"
           "please visit http://www.mcrl2.org/issuetracker");

  return version_information;
}

/* The optional input file that should contain an LPS */
std::string lps_file_argument;

void xsim_message_handler(mcrl2::core::messageType msg_type, const char *msg);

// Squadt protocol interface
#ifdef ENABLE_SQUADT_CONNECTIVITY
#include <mcrl2/utilities/mcrl2_squadt_interface.h>

using namespace mcrl2::utilities::squadt;

const char* lps_file_for_input = "lps_in";

class squadt_interactor : public mcrl2::utilities::squadt::mcrl2_wx_tool_interface {

  public:

    // Special initialisation
    void initialise() {
      gsSetCustomMessageHandler(xsim_message_handler);
    }

    // Configures tool capabilities.
    void set_capabilities(tipi::tool::capabilities& c) const {
      /* The tool has only one main input combination it takes an LPS and then behaves as a reporter */
      c.add_input_configuration(lps_file_for_input,
           tipi::mime_type("lps", tipi::mime_type::application), tipi::tool::category::simulation);
    }

    // Queries the user via SQuADt if needed to obtain configuration information
    void user_interactive_configuration(tipi::configuration&) { }

    // Check an existing configuration object to see if it is usable
    bool check_configuration(tipi::configuration const& c) const {
      bool valid = c.input_exists(lps_file_for_input);

      if (!valid) {
        send_error("Invalid input combination!");
      }

     return valid;
   }

    bool perform_task(tipi::configuration& c) {
      lps_file_argument = c.get_input(lps_file_for_input).location();

      return mcrl2_wx_tool_interface::perform_task(c);
    }
};
#endif

//------------------------------------------------------------------------------
// XSim
//------------------------------------------------------------------------------
class XSim: public wxApp
{
private:
    RewriteStrategy rewrite_strategy;
    bool            dummies;
    std::string     parse_error;

public:
    virtual bool OnInit();

    void parse_command_line(int argc, wxChar** argv);

    bool Initialize(int& argc, wxChar** argv) {
      try {
        parse_command_line(argc, argv);
      }
      catch (std::exception& e) {
        if (wxApp::Initialize(argc, argv)) {
          parse_error = std::string(e.what()).
            append("\n\nNote that other command line options may have been ignored because of this error.");
        }
        else {
          std::cerr << e.what() << std::endl;

          return false;
        }

        return true;
      }

      return wxApp::Initialize(argc, argv);
    }

    virtual int OnExit();
};

void XSim::parse_command_line(int argc, wxChar** argv) {

  using namespace ::mcrl2::utilities;

  interface_description clinterface(std::string(wxString(argv[0], wxConvLocal).fn_str()),
      NAME, AUTHOR, "[OPTION]... [INFILE]\n",
    "Simulate LPSs in a graphical environment. If INFILE is supplied it will be "
    "loaded into the simulator.");

  clinterface.add_rewriting_options();

  clinterface.
    add_option("dummy", "replace free variables in the LPS with dummy values", 'y');

  command_line_parser parser(clinterface, argc, argv);

  dummies = 0 < parser.options.count("dummy");

  rewrite_strategy = parser.option_argument_as< RewriteStrategy >("rewriter");

  if (0 < parser.arguments.size()) {
    lps_file_argument = parser.arguments[0];
  }
  if (1 < parser.arguments.size()) {
    parser.error("too many file arguments");
  }
}

static XSim *this_xsim = NULL;
void xsim_message_handler(mcrl2::core::messageType msg_type, const char *msg)
{
  using namespace ::mcrl2::utilities;
  using namespace mcrl2::core;

  if ( this_xsim == NULL )
  {
    fprintf(stderr,"%s",msg);
    fprintf(stderr,"this message was brought to you by XSim (all rights reserved)\n");
  } else {
    const char *msg_end = msg+std::strlen(msg)-1;
    while ( (msg <= msg_end) && ((*msg == '\r') || (*msg == '\n')) )
    {
      --msg_end;
    }
    wxString wx_msg(msg,wxConvLocal, msg_end - msg);
    switch (msg_type)
    {
      case gs_warning:
        {
        wxMessageDialog(NULL,wx_msg,wxT("mCRL2 warning"),wxOK|wxICON_EXCLAMATION).ShowModal();
        }
        break;
      case gs_error:
        {
        wxMessageDialog(NULL,wx_msg,wxT("mCRL2 error"),wxOK|wxICON_ERROR).ShowModal();
        }
        break;
      case gs_notice:
      default:
        {
        wxMessageDialog(NULL,wx_msg,wxT("mCRL2 notice"),wxOK|wxICON_INFORMATION).ShowModal();
        }
        break;
    }
  }
}


bool XSim::OnInit()
{

  this_xsim = this;

  /* Whether to replace free variables in the LPS with dummies */
  dummies = false;
 
  /* The rewrite strategy that will be used */
  rewrite_strategy = GS_REWR_JITTY;
 
  XSimMain *frame = new XSimMain( 0, -1, wxT("XSim"), wxPoint(-1,-1), wxSize(500,400) );
  frame->simulator->use_dummies = dummies;
  frame->simulator->rewr_strat  = rewrite_strategy;
  frame->Show(true);

  if (!parse_error.empty()) {
    wxMessageDialog(frame, wxString(parse_error.c_str(), wxConvLocal),
         wxT("Command line parsing error"), wxOK|wxICON_ERROR).ShowModal();
  }

  if (!lps_file_argument.empty()) {
    frame->LoadFile(wxString(lps_file_argument.c_str(), wxConvLocal));
  }
 
  return true;
}

int XSim::OnExit()
{
    return 0;
}

IMPLEMENT_APP_NO_MAIN(XSim)
IMPLEMENT_WX_THEME_SUPPORT

#ifdef __WINDOWS__
extern "C" int WINAPI WinMain(HINSTANCE hInstance,                    
                                  HINSTANCE hPrevInstance,                
                                  wxCmdLineArgType lpCmdLine,             
                                  int nCmdShow) {                                                                     

  MCRL2_ATERM_INIT(0, lpCmdLine)

#ifdef ENABLE_SQUADT_CONNECTIVITY
  if(interactor< squadt_interactor >::free_activation(hInstance, hPrevInstance, lpCmdLine, nCmdShow)) {
    return EXIT_SUCCESS;
  }
#endif

  return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#else
int main(int argc, char **argv)
{
  MCRL2_ATERM_INIT(argc, argv)

#ifdef ENABLE_SQUADT_CONNECTIVITY
  if(interactor< squadt_interactor >::free_activation(argc, argv)) {
    return EXIT_SUCCESS;
  }
#endif

  return wxEntry(argc, argv);
}
#endif
