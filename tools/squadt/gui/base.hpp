// Author(s): Jeroen van der Wulp
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file gui/dialog/base.h
/// \brief Add your file description here.

#ifndef GUI_DIALOG_BASE_H
#define GUI_DIALOG_BASE_H

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/dirctrl.h>

namespace squadt {
  namespace GUI {
    namespace dialog {

      /**
       * \brief Base class for dialogs
       **/
      class basic : public wxDialog {

        protected:

          /** The panel that holds the custom controls for this dialog */
          wxPanel*        main_panel;

          /** \brief Button that, when pressed, accepts the current settings */
          wxButton*       button_accept;

          /** \brief Button that, when pressed, cancels and closes the window */
          wxButton*       button_cancel;

        private:

          /** \brief Handler for when one of the buttons is clicked */
          void on_button_clicked(wxCommandEvent&);

          /** \brief Handler for the window close event */
          void on_window_close(wxCloseEvent&);

          /** \brief Helper function that places the widgets */
          void build();

        public:

          /** \brief Constructor */
          basic(wxWindow*, wxString const&, wxSize const&);

          /** \brief Destructor */
          virtual ~basic() = 0;
      };
    }
  }
}

#endif

