//  Author(s): A.j. (Hannes) pretorius
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  LICENSE_1_0.txt)
//
/// \file ./popupframe.cpp

// --- popupframe.cpp -----------------------------------------------
// (c) 2007  -  A.J. Pretorius  -  Eindhoven University of Technology
// ---------------------------  *  ----------------------------------


#include "popupframe.h"


// -- constructors and desctructor ----------------------------------


// ------------------------------
PopupFrame::PopupFrame(
    Mediator* m,
    wxWindow* parent,
    wxWindowID id,
    wxString title,
    wxPoint position,
    wxSize size )
    : Colleague( m ),
      wxFrame( 
        parent,
        id,
        wxString( title ),
        position,
        size,
        wxDEFAULT_FRAME_STYLE |
        wxFRAME_FLOAT_ON_PARENT |
		wxFRAME_TOOL_WINDOW |
        wxFRAME_NO_TASKBAR )
// ------------------------------
{}


// ----------------------
PopupFrame::~PopupFrame()
// ----------------------
{}


// -- event handlers ------------------------------------------------


// --------------------------------------------
void PopupFrame::OnClose( wxCloseEvent& event )
// --------------------------------------------
{
    mediator->handleCloseFrame( this );
    this->Destroy();
}


// -- implement event table -----------------------------------------


BEGIN_EVENT_TABLE( PopupFrame, wxFrame )
    // close event
    EVT_CLOSE( PopupFrame::OnClose )
END_EVENT_TABLE()


// -- end -----------------------------------------------------------
