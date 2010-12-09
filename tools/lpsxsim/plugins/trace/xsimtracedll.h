// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file xsimtracedll.h

#ifndef __xsimtracedll_H__
#define __xsimtracedll_H__

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma interface "xsimtracedll.h"
#endif

// Include wxWindows' headers

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <aterm2.h>
#include "simbase.h"

//----------------------------------------------------------------------------
// XSimTraceDLL
//----------------------------------------------------------------------------

class XSimTraceDLL: public wxFrame, public SimulatorViewDLLInterface
{
public:
    // constructors and destructors
    XSimTraceDLL( wxWindow *parent );
/*    XSimTraceDLL( wxWindow *parent, wxWindowID id, const wxString &title,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_FRAME_STYLE ) : XSimTraceDLL(parent);*/

    // SimulatorViewInterface
    virtual void Registered(SimulatorInterface *Simulator);
    virtual void Unregistered();
    virtual void Initialise(ATermList Pars);
    virtual void StateChanged(ATermAppl Transition, ATerm State, ATermList NextStates);
    virtual void Reset(ATerm State);
    virtual void Undo(size_t Count);
    virtual void Redo(size_t Count);
    virtual void TraceChanged(ATermList Trace, size_t From);
    virtual void TracePosChanged(ATermAppl Transition, ATerm State, size_t Index);

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
    size_t current_pos;

private:
    // WDR: handler declarations for XSimMain
    void OnCloseWindow( wxCloseEvent &event );
    void OnListItemActivated( wxListEvent &event );

private:

private:
    DECLARE_EVENT_TABLE()
};

#endif
