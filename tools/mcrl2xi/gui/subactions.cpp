// Author(s): Frank Stappers
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file subactions.cpp

#include "subactions.h"
#include "options.h"

BEGIN_EVENT_TABLE(evalDataExpr, wxPanel)
EVT_BUTTON(OPTION_EVAL, evalDataExpr::OnEval)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(typeCheckSpec, wxPanel)
EVT_BUTTON(OPTION_TC, typeCheckSpec::OnTypeCheck)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(solveDataExpr, wxPanel)
EVT_BUTTON(OPTION_SOLVE, solveDataExpr::OnSolve)
END_EVENT_TABLE()

evalDataExpr::evalDataExpr(wxWindow *parent, wxWindowID id):
wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER)
{
  p_parent = parent;
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Rewrite Data expression:")), 0, wxLEFT|wxALL, 5);
  EvalExpr = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, -1));
  EvalExpr->SetToolTip(wxT("Rewrite EXPRESSION and print result \n \n Example: 5 < 6 \n \n NOTE: Rewriting an EXPRESSION \n\t only be performed for valid\n\t specifications"));
  sizer->Add(EvalExpr, 0, wxEXPAND| wxALL, 5);
  sizer->Add(new wxButton(this, OPTION_EVAL, wxT("Rewrite")), 0, wxLEFT | wxALL, 5);
  sizer->SetSizeHints(this);
  SetSizer(sizer);
}

wxString evalDataExpr::getDataExprVal(){
  return EvalExpr->GetValue();
}

void evalDataExpr::OnEval(wxCommandEvent& e) {
  ((Options*) p_parent)-> OnEval(e) ;
}

solveDataExpr::solveDataExpr(wxWindow *parent, wxWindowID id):
wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER)
{
  p_parent = parent;
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Solve Data expression:")), 0, wxLEFT|wxALL, 5);
  SolveExpr = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, -1));
  sizer->Add(SolveExpr, 0, wxEXPAND| wxALL, 5);
  SolveExpr->SetToolTip(wxT("Give all valuations of the variables \n in VARLIST that satisfy EXPRESSION. \n\n Example: s n:Nat;b:Bool. b==false && n < 5"));
  sizer->Add(new wxButton(this, OPTION_SOLVE, wxT("Solve")), 0, wxLEFT | wxALL, 5);
  sizer->Add( new wxStaticText(this, wxID_ANY, wxT("Solutions:")), 0, wxLEFT|wxALL, 5);
  p_solutions = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(-1, 150), wxTE_READONLY|wxTE_MULTILINE|wxTE_DONTWRAP);
  sizer->Add(p_solutions, 0, wxEXPAND|wxALL, 5);
  sizer->SetSizeHints(this);
  SetSizer(sizer);
}

wxTextCtrl* solveDataExpr::getSolutionWindow(){
  return p_solutions;
}

wxString solveDataExpr::getDataExprSolve(){
  return SolveExpr->GetValue();;
}

void solveDataExpr::OnSolve(wxCommandEvent& e) {
  //e.SetClientData( p_solutions );
  ((Options*) p_parent)-> SolveExpr(e) ;
}

typeCheckSpec::typeCheckSpec(wxWindow *parent, wxWindowID id):
wxPanel(parent, id, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER)
{
  p_parent = parent;
  wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Parse and type check specification")), 0, wxLEFT|wxALL, 5);
  sizer->Add(new wxButton(this, OPTION_TC, wxT("Parse and type check")), 0, wxLEFT | wxALL, 5);
  sizer->SetSizeHints(this);
  SetSizer(sizer);
}

void typeCheckSpec::OnTypeCheck(wxCommandEvent& e){
  ((Options*) p_parent)-> OnTypeCheck(e) ;
}

