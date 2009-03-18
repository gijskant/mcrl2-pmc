// Author(s): Diana Koenraadt, Remco Blewanus, Bram Schoenmakers, Thorstin Crijns, Hans Poppelaars, Bas Luksenburg, Jonathan Nelisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file channeldialog.h
//
// Defines a channel dialog.

#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include "channelcommunicationdialog.h"

using namespace grape::grapeapp;

grape_channel_communication_dlg::grape_channel_communication_dlg( channel_communication &p_channel_communication )
: wxDialog( 0, wxID_ANY, _T("Edit channel communication"), wxDefaultPosition, wxDefaultSize )
{  
  wxBoxSizer *wnd_sizer = new wxBoxSizer(wxVERTICAL);

  wxStaticText *text_rename = new wxStaticText( this, wxID_ANY, _T("rename channel communication") );
  wnd_sizer->Add(text_rename, 0, wxALIGN_TOP, 0 );
  wnd_sizer->AddSpacer( 5 );
  
  m_rename_input = new wxTextCtrl(this, wxID_ANY, p_channel_communication.get_rename_to() );            
  wnd_sizer->Add(m_rename_input, 0, wxEXPAND, 0);
  wnd_sizer->AddSpacer( 5 );

  wxStaticText *text_property = new wxStaticText( this, wxID_ANY, _T("channel communication property") );
  wnd_sizer->Add(text_property, 0, wxALIGN_TOP, 0 );
  wnd_sizer->AddSpacer( 5 );
  
  // select the correct property of the channel communication
  int index = p_channel_communication.get_channel_communication_type();
  wxString combobox_list[3] = {_T("visible"), _T("hidden"), _T("blocked")};
  m_combobox = new wxComboBox(this, wxID_ANY, combobox_list[index], wxDefaultPosition, wxDefaultSize, 3, combobox_list, wxCB_READONLY);
  
  wnd_sizer->Add(m_combobox, 0, wxEXPAND, 0);
  wnd_sizer->AddSpacer( 5 );

  // create buttons
  wxSizer *sizer = CreateButtonSizer(wxOK | wxCANCEL);
  sizer->Layout();
  wnd_sizer->Add(sizer, 0, wxALIGN_BOTTOM, 0);

  // realize sizers
  SetSizer(wnd_sizer);
  wnd_sizer->Fit(this);
  wnd_sizer->SetSizeHints(this);

  m_rename_input->SetFocus();
}

grape_channel_communication_dlg::grape_channel_communication_dlg()
{
  // shouldn't be called
}

grape_channel_communication_dlg::~grape_channel_communication_dlg()
{
}

bool grape_channel_communication_dlg::show_modal( channel_communication &p_channel_communication )
{
  if (ShowModal() != wxID_CANCEL)
  {      
    p_channel_communication.set_rename_to(m_rename_input->GetValue());     
    if (m_combobox->GetValue() == _T("visible")) p_channel_communication.set_channel_communication_type(VISIBLE_CHANNEL_COMMUNICATION);
    if (m_combobox->GetValue() == _T("hidden")) p_channel_communication.set_channel_communication_type(HIDDEN_CHANNEL_COMMUNICATION);
    if (m_combobox->GetValue() == _T("blocked")) p_channel_communication.set_channel_communication_type(BLOCKED_CHANNEL_COMMUNICATION);
  
    return true;
  }

  return false;
}
