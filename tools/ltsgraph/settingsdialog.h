// Author(s): Carst Tankink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file settingsdialog.h
/// \brief Settings dialog declaration.

#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/clrpicker.h>
#include "ltsgraph.h"

class SettingsDialog: public wxDialog {
  public:
    SettingsDialog(LTSGraph* app, wxWindow* parent);
    

  private:
    LTSGraph* app;
    void onRadius(wxSpinEvent& evt);
    void onCurves(wxCommandEvent& evt);
    void onColour(wxColourPickerEvent& evt);
  
  DECLARE_EVENT_TABLE()
};



#endif // SETTINGS_DIALOG_H
