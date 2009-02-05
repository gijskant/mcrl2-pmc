// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file xsimtrace.h

#ifndef __xsimtrace_H__
#define __xsimtrace_H__

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <aterm2.h>
#include "simbase.h"

//----------------------------------------------------------------------------
// XSimTrace
//----------------------------------------------------------------------------

class XSimTrace: public wxFrame, public SimulatorViewInterface
{
public:
    // constructors and destructors
    XSimTrace( wxWindow *parent );
/*    XSimTrace( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE ) : XSimTrace(parent);*/

    // SimulatorViewInterface
    virtual void Registered(SimulatorInterface *Simulator);
    virtual void Unregistered();
    virtual void Initialise(ATermList Pars);
    virtual void StateChanged(ATermAppl Transition, ATerm State, ATermList NextStates);
    virtual void Reset(ATerm State);
    virtual void Undo(unsigned int Count);
    virtual void Redo(unsigned int Count);
    virtual void TraceChanged(ATermList Trace, unsigned int From);
    virtual void TracePosChanged(ATermAppl Transition, ATerm State, unsigned int Index);

private:
    // WDR: method declarations for XSimMain
    void AddState(ATermAppl Transition, ATerm State, bool enabled);
    void _add_state(ATermAppl Transition, ATerm State, bool enabled);
    void _reset(ATerm State);
    void _update();

private:
    // WDR: member variable declarations for XSimMain
    wxListView *traceview;
    SimulatorInterface *simulator;
    unsigned int current_pos;

private:
    // WDR: handler declarations for XSimMain
    void OnCloseWindow( wxCloseEvent &event );
    void OnListItemActivated( wxListEvent &event );

private:

private:
    DECLARE_EVENT_TABLE()
};

#endif
