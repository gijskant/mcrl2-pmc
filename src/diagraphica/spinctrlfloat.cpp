// --- spinctrlfloat.cpp ----------------------------------------------
// (c) 2006 -  S.W.C. Ploeger, D. Reniers  - Eindhoven Univ. Technology
// ---------------------------  *  ------------------------------------

#include "spinctrlfloat.h"

wxSpinCtrlFloat::wxSpinCtrlFloat(
    wxWindow* parent, 
    wxWindowID id, 
    double p_Min, 
    double p_Max, 
    double p_Rate, 
    double p_Init, 
    const wxPoint& pos, 
    const wxSize&  size)
    : wxPanel(
        parent, 
        -1, 
        pos, 
        size, 
        wxNO_BORDER ), 
      m_Rate(p_Rate), 
      m_ID(id)
{
  wxBoxSizer* Sizer = new wxBoxSizer( wxHORIZONTAL );
  m_TextCtrl = new wxTextCtrl( this, 0, wxEmptyString, wxDefaultPosition,
      wxDefaultSize, wxTE_PROCESS_ENTER );
  m_SpinButton = new wxSpinButton( this, 1, wxDefaultPosition, wxDefaultSize,
      wxSP_VERTICAL );
  
  // spin button should be at most as high as the text control
  int w; int h;
  m_TextCtrl->GetSize( &w, &h );
  m_SpinButton->SetSizeHints( -1, h, -1, h );
  
  Sizer->Add( m_TextCtrl, 1, wxEXPAND, 0 );
  Sizer->Add( m_SpinButton , 0, wxEXPAND, 0 );
  
  m_SpinButton->SetRange( Utils::rndToInt( p_Min/p_Rate ), Utils::rndToInt( p_Max/p_Rate ) );
  SetValue( p_Init );

  Sizer->SetMinSize( size );
  this->SetSizer( Sizer );
  Sizer->SetSizeHints( this );
}

BEGIN_EVENT_TABLE(wxSpinCtrlFloat, wxPanel)
  EVT_TEXT_ENTER( 0, wxSpinCtrlFloat::OnEnter )
  EVT_SPIN( 1, wxSpinCtrlFloat::OnSpin )
END_EVENT_TABLE()

void wxSpinCtrlFloat::OnEnter( wxCommandEvent& event )
{
  string str( event.GetString().fn_str() );
  
  double f;
  sscanf( str.c_str(), "%f", &f );

  f = Utils::minn( m_SpinButton->GetMax() * m_Rate, Utils::maxx( f, m_SpinButton->GetMin() * m_Rate ) );
  SetValue( f );

  // Create new event with the user specified event id m_ID
  wxCommandEvent NewEvent( wxEVT_COMMAND_SPINCTRL_UPDATED, m_ID );
  NewEvent.SetEventObject( this );
  GetParent()->ProcessEvent( NewEvent );
}

void wxSpinCtrlFloat::OnSpin( wxSpinEvent& event )
{
  int i = event.GetInt();
  m_TextCtrl->SetValue( wxString::Format( wxT("%2.1f"), i * m_Rate ) );

  // Create new event with the user specified event id m_ID
  wxCommandEvent NewEvent( wxEVT_COMMAND_SPINCTRL_UPDATED, m_ID );
  NewEvent.SetEventObject( this );
  GetParent()->ProcessEvent( NewEvent );
}

double wxSpinCtrlFloat::GetValue()
{
  return m_Rate * m_SpinButton->GetValue();
}

void wxSpinCtrlFloat::SetValue( float v )
{
  m_TextCtrl->SetValue( wxString::Format( wxT("%2.1f"), v ) );
  m_SpinButton->SetValue( Utils::rndToInt( v/m_Rate ) );
}

// -- end -----------------------------------------------------------
