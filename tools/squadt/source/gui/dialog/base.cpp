// Author(s): Jeroen van der Wulp
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file gui/dialog/base.cpp
/// \brief Add your file description here.

#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/statline.h>

#include "gui/dialog/base.h"

namespace squadt {
  namespace GUI {
    namespace dialog {

      /**
       * @param[in] p the parent window
       * @param[in] t the title of the window
       * @param[in] d the dimensions as (width, height) in pixels of the window
       **/
      basic::basic(wxWindow* p, wxString const& t, wxSize const& d) : wxDialog(p, wxID_ANY, t, wxDefaultPosition, d) {
        build();

        Connect(wxEVT_CLOSE_WINDOW, wxCloseEventHandler(dialog::basic::on_window_close));
        Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(dialog::basic::on_button_clicked));
      }

      /**
       * @param[in] e the event object passed by wxWidgets
       **/
      void basic::on_button_clicked(wxCommandEvent& e) {
        EndModal((e.GetId() == wxID_CANCEL) ? 0 : 1);
      }

      void basic::on_window_close(wxCloseEvent&) {
        EndModal(0);
      }

      void basic::build() {
        wxBoxSizer* s  = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* ss = new wxBoxSizer(wxHORIZONTAL);

        main_panel    = new wxPanel(this, wxID_ANY);
        button_accept = new wxButton(this, wxID_OK);
        button_cancel = new wxButton(this, wxID_CANCEL);

        ss->Add(button_cancel, 0, wxRIGHT, 5);
        ss->Add(button_accept, 0, wxLEFT|wxRIGHT, 5);
        s->Add(main_panel, 1, wxEXPAND|wxALL, 4);
        s->Add(new wxStaticLine(this, wxID_ANY), 0, wxEXPAND|wxALL, 2);
        s->Add(ss, 0, wxALIGN_RIGHT|wxALL, 5);

        SetSizer(s);

        button_accept->SetDefault();

        CentreOnParent();
      }

      basic::~basic() {
      }
    }
  }
}
