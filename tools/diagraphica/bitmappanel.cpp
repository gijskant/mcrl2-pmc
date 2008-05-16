// Author(s): A.j. (Hannes) pretorius
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// LICENSE_1_0.txt)
//
/// \file ./bitmappanel.cpp


#include "bitmappanel.h"


// -- constructors and desctructor ----------------------------------


// ---------------------------------------
BitmapPanel::BitmapPanel( 
    wxWindow* parent,
    wxSize size,
    const char* const* image )
    : wxPanel(
        parent,
        wxID_ANY )
// --------------------------------------
{
    // no resize
    this->SetClientSize( size );
    this->SetMinSize( this->GetSize() );
    this->SetMaxSize( this->GetSize() );

    /*
    // load image
    bitmap = new wxBitmap( 
        path, 
        wxBITMAP_TYPE_BMP );
    if ( bitmap->Ok() != true )
    {
        wxString msg;
        msg.Append( "Error opening file: " );
        msg.Append( path );
        wxLogError( msg );
    }
    */
    // load image, cast for wxWidgets 2.6 compatibility
    bitmap = new wxBitmap( const_cast< const char** >( image ) );
}


// ------------------------
BitmapPanel::~BitmapPanel()
// ------------------------
{
    delete bitmap;
    bitmap = NULL;
}


// -- event handlers ------------------------------------------------


// ------------------------------------------------
void BitmapPanel::onEvtPaint( wxPaintEvent& event )
// ------------------------------------------------
{
    if ( bitmap->Ok() == true )
    {
        wxClientDC dc( this );

        // aparently BeginDrawing() and EndDrawing() are depricated and do nothing
        //dc.BeginDrawing();
        dc.DrawBitmap( *bitmap, 0, 0, false );
        //dc.EndDrawing();
    }
}


// -- event table ---------------------------------------------------


BEGIN_EVENT_TABLE( BitmapPanel, wxPanel )
    EVT_PAINT( BitmapPanel::onEvtPaint )
END_EVENT_TABLE()


// -- end -----------------------------------------------------------
