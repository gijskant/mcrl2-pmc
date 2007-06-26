#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "xsimmain.h"
#endif

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include <wx/dynlib.h>
#include <wx/config.h>
#include <wx/textfile.h>
#include <wx/dynarray.h>
#include <sstream>
#include <cstdlib>
#include <aterm2.h>
#include "xsimbase.h"
#include "xsimmain.h"
#include "libstruct.h"
#include "libnextstate.h"
#include "librewrite.h"
#include "libprint_types.h"
#include "libprint.h"
#include "libtrace.h"
#include "mcrl2/utilities/aterm_ext.h"

// For PLUGIN_DIRECTORY
#include <setup.h>

// For compatibility with older wxWidgets versions (pre 2.8)
#if (wxMINOR_VERSION < 8)
# define wxFD_SAVE wxSAVE
# define wxFD_CHANGE_DIR wxCHANGE_DIR
#endif

using namespace std;
using namespace ::mcrl2::utilities;

//------------------------------------------------------------------------------
// XSimMain
//------------------------------------------------------------------------------

BEGIN_EVENT_TABLE(XSimMain,wxFrame)
    EVT_MENU(wxID_OPEN, XSimMain::OnOpen)
    EVT_MENU(wxID_EXIT, XSimMain::OnQuit)
    EVT_MENU(ID_UNDO, XSimMain::OnUndo)
    EVT_MENU(ID_REDO, XSimMain::OnRedo)
    EVT_MENU(ID_RESET, XSimMain::OnReset)
    EVT_MENU(ID_LOADTRACE, XSimMain::OnLoadTrace)
    EVT_MENU(ID_SAVETRACE, XSimMain::OnSaveTrace)
    EVT_MENU(ID_FITCS, XSimMain::OnFitCurrentState)
    EVT_MENU(ID_TRACE, XSimMain::OnTrace)
    EVT_MENU(ID_LOADVIEW, XSimMain::OnLoadView)
    EVT_MENU(ID_SHOWDC, XSimMain::OnShowDCChanged)
    EVT_MENU(ID_DELAY, XSimMain::OnSetDelay)
    EVT_MENU(ID_PLAYI, XSimMain::OnResetAndPlay)
    EVT_MENU(ID_PLAYC, XSimMain::OnPlay)
    EVT_MENU(ID_PLAYRI, XSimMain::OnResetAndPlayRandom)
    EVT_MENU(ID_PLAYRC, XSimMain::OnPlayRandom)
    EVT_MENU(ID_STOP, XSimMain::OnStop)
    EVT_MENU(wxID_ABOUT, XSimMain::OnAbout)
    EVT_TIMER(-1, XSimMain::OnTimer)
    EVT_CLOSE(XSimMain::OnCloseWindow)
    EVT_LIST_ITEM_SELECTED(ID_LISTCTRL1, XSimMain::stateOnListItemSelected)
    EVT_LIST_ITEM_ACTIVATED(ID_LISTCTRL2, XSimMain::transOnListItemActivated)
END_EVENT_TABLE()

XSimMain::XSimMain( wxWindow *parent, wxWindowID id, const wxString &title,
    const wxPoint &position, const wxSize& size, long style ) :
    wxFrame( parent, id, title, position, size, style ) ,
    timer( this )
{
    use_dummies = false;
    rewr_strat = GS_REWR_INNER; // XXX add to constructor?

    base_title = title;

    CreateMenu();
    CreateStatus();
    CreateContent();

    /* Attach resize event handler */
    Connect(id, wxEVT_SIZE, wxCommandEventHandler(XSimMain::UpdateSizes), NULL, this);
    Connect(id, wxEVT_MAXIMIZE, wxCommandEventHandler(XSimMain::UpdateSizes), NULL, this);

    state_vars = ATmakeList0();
    ATprotectList(&state_vars);
    state_varnames = ATmakeList0();
    ATprotectList(&state_varnames);
    initial_state = NULL;;
    ATprotect(&initial_state);
    current_state = NULL;
    ATprotect(&current_state);
    next_states = ATmakeList0();
    ATprotectList(&next_states);
    trace = ATmakeList0();
    ATprotectList(&trace);
    ecart = ATmakeList0();
    ATprotectList(&ecart);
    
    tracewin = new XSimTrace(this);
    Register(tracewin);
    //tracewin->Show(FALSE); // default, so not needed
    
    wxConfig config(wxT("xsimrc"));
    if ( config.HasGroup(wxT("LoadLibrary")) )
    {
	    config.SetPath(wxT("/LoadLibrary"));

	    wxString s;
	    long i;
	    bool b = config.GetFirstEntry(s,i);
	    while ( b )
	    {
		    LoadDLL(config.Read(s,wxT("")));
		    b = config.GetNextEntry(s,i);
	    }
    }

    interactive = true;
    stopper_cnt = 0;
    timer_interval = 1000;

    seen_states = ATindexedSetCreate(100,80);

    nextstate = NULL;
    nextstategen = NULL;
}

XSimMain::~XSimMain()
{
	for (viewlist::iterator i = views.begin(); i != views.end(); i++)
	{
		(*i)->Unregistered();
	}
	
	delete tracewin;

	if ( initial_state != NULL )
	{
		delete nextstategen;
		delete nextstate;
	}

	ATunprotectList(&state_vars);
	ATunprotectList(&state_varnames);
	ATunprotect(&initial_state);
	ATunprotect(&current_state);
	ATunprotectList(&next_states);
	ATunprotectList(&trace);
	ATunprotectList(&ecart);
}

void XSimMain::CreateMenu()
{
    menu = new wxMenuBar;

    wxMenu *file = new wxMenu;
    openitem = file->Append( wxID_OPEN, wxT("&Open...	CTRL-o"), wxT("") );
    file->AppendSeparator();
    ldtrcitem = file->Append( ID_LOADTRACE, wxT("&Load trace...	CTRL-l"), wxT("") );
    ldtrcitem->Enable(false);
    svtrcitem = file->Append( ID_SAVETRACE, wxT("&Save trace...	CTRL-s"), wxT("") );
    svtrcitem->Enable(false);
    file->AppendSeparator();
    file->Append( wxID_EXIT, wxT("&Quit	CTRL-q"), wxT("") );
    menu->Append( file, wxT("&File") );

    editmenu = new wxMenu;
    undo = editmenu->Append( ID_UNDO, wxT("&Undo	CTRL-LEFT"), wxT("") );
    undo->Enable(false);
    redo = editmenu->Append( ID_REDO, wxT("Re&do	CTRL-RIGHT"), wxT("") );
    redo->Enable(false);
    editmenu->Append( ID_RESET, wxT("&Reset	CTRL-r"), wxT("") );
    menu->Append( editmenu, wxT("&Edit") );
    
    wxMenu *sim = new wxMenu;
    playiitem = sim->Append( ID_PLAYI, wxT("Play Trace from Initial State"), wxT(""));
    playcitem = sim->Append( ID_PLAYC, wxT("Play Trace from Current State"), wxT(""));
    playriitem = sim->Append( ID_PLAYRI, wxT("Random Play from Initial State"), wxT(""));
    playrcitem = sim->Append( ID_PLAYRC, wxT("Random Play from Current State"), wxT(""));
    stopitem = sim->Append( ID_STOP, wxT("Stop	DEL"), wxT("") );
    stopitem->Enable(false);
    menu->Append( sim, wxT("&Automation") );
    
    wxMenu *opts = new wxMenu;
    tau_prior = opts->Append( ID_TAU, wxT("Enable Tau Prioritisation"), wxT(""), wxITEM_CHECK );
    showdc = opts->Append( ID_SHOWDC, wxT("Show Don't Cares in State Changes"), wxT(""), wxITEM_CHECK );
    opts->Append( ID_DELAY, wxT("Set Play Delay"), wxT("") );
    opts->Append( ID_FITCS, wxT("F&it to Current State	CTRL-f"), wxT("") );
    menu->Append( opts, wxT("&Options") );

    wxMenu *views = new wxMenu;
    views->Append( ID_TRACE, wxT("&Trace	CTRL-t"), wxT("") );
    views->Append( ID_MENU, wxT("&Graph"), wxT("") )->Enable(false);
    views->Append( ID_LOADVIEW, wxT("&Load Plugin..."), wxT("") );
    menu->Append( views, wxT("&Views") );
    
    wxMenu *help = new wxMenu;
    help->Append( wxID_ABOUT, wxT("&About"), wxT("") );
    menu->Append( help, wxT("&Help") );

    SetMenuBar( menu );
}

void XSimMain::CreateStatus()
{
    CreateStatusBar(1);
    SetStatusText(wxT(""));
}

void XSimMain::CreateContent()
{
    mainsizer = new wxBoxSizer(wxVERTICAL);
    split     = new wxSplitterWindow(this,ID_SPLITTER,wxDefaultPosition,wxDefaultSize,wxCLIP_CHILDREN);

    toppanel    = new wxPanel(split,-1);
    topsizer    = new wxBoxSizer(wxVERTICAL);
    topboxsizer = new wxStaticBoxSizer(wxVERTICAL,toppanel,wxT("Current State"));
    stateview   = new wxListView(toppanel,ID_LISTCTRL1,wxDefaultPosition,wxDefaultSize,wxLC_REPORT|wxSUNKEN_BORDER|wxLC_HRULES|wxLC_VRULES|wxLC_SINGLE_SEL);

    topboxsizer->Add(stateview,1,wxEXPAND|wxALIGN_CENTER|wxALL,5);
    topsizer->Add(topboxsizer,1,wxEXPAND|wxALIGN_CENTER|wxALL,5);
    toppanel->SetSizer(topsizer);

    bottompanel    = new wxPanel(split,-1);
    bottomsizer    = new wxBoxSizer(wxVERTICAL);
    bottomboxsizer = new wxStaticBoxSizer(wxVERTICAL,bottompanel,wxT("Transitions"));
    transview      = new wxListView(bottompanel,ID_LISTCTRL2,wxDefaultPosition,wxDefaultSize,wxLC_REPORT|wxSUNKEN_BORDER|wxLC_HRULES|wxLC_VRULES|wxLC_SINGLE_SEL);

    bottomboxsizer->Add(transview,1,wxEXPAND|wxALIGN_CENTER|wxALL,5);
    bottomsizer->Add(bottomboxsizer,1,wxEXPAND|wxALIGN_CENTER|wxALL,5);
    bottompanel->SetSizer(bottomsizer);

    split->SplitHorizontally(bottompanel,toppanel);
    split->SetMinimumPaneSize(27);
    split->SetSashGravity(1.0);
    mainsizer->Add(split, 1, wxEXPAND|wxALIGN_CENTER|wxALL, 5);
    
    SetMinSize(wxSize(240,160));

    stateview->InsertColumn(0, wxT("Parameter"), wxLIST_FORMAT_LEFT, 120);
    stateview->InsertColumn(1, wxT("Value"), wxLIST_FORMAT_LEFT);
    stateview->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER|wxLIST_AUTOSIZE);

    transview->InsertColumn(0, wxT("Action"), wxLIST_FORMAT_LEFT, 120);
    transview->InsertColumn(1, wxT("State Change"), wxLIST_FORMAT_LEFT);
    transview->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER|wxLIST_AUTOSIZE);
    transview->SetFocus();

    /* Show in order to be able to query client width */
    Show();

    stateview->SetColumnWidth(1,stateview->GetClientSize().GetWidth() - stateview->GetColumnWidth(0));
    transview->SetColumnWidth(1,transview->GetClientSize().GetWidth() - transview->GetColumnWidth(0));
    
    /* Obtain height information of stateview; needed for FitCurrentState() */
    stateview->InsertItem(0,wxT("tmp"));
    wxRect r;
    stateview->GetItemRect(0,r);
    stateview_header_height = r.y;
    stateview_item_height = r.GetHeight();
    stateview->DeleteAllItems();
}

void XSimMain::UpdateSizes(wxCommandEvent& event) {
  int s  = stateview->GetClientSize().GetWidth() - stateview->GetColumnWidth(0);

  event.Skip();

  if (s <= 80) {
    s = wxLIST_AUTOSIZE;
  }

  /* Set column width of stateview, if necessary */
  if (stateview->GetColumnWidth(1) != s) { 
    stateview->SetColumnWidth(1, s);
  }

  s  = transview->GetClientSize().GetWidth() - transview->GetColumnWidth(0);

  if (s <= 80) {
    s = wxLIST_AUTOSIZE;
  }

  /* Set column width of transview, if necessary */
  if (transview->GetColumnWidth(1) != s) { 
    transview->SetColumnWidth(1, s);
  }

  /* hack to avoid unnecessary scrollbars */
#if defined(__WXGTK__)
  int w,h;
  transview->GetClientSize(&w,&h);
  transview->SetClientSize(0,0);
  transview->SetClientSize(w,h);
  stateview->GetClientSize(&w,&h);
  stateview->SetClientSize(0,0);
  stateview->SetClientSize(w,h);
#endif
}

void XSimMain::SetInteractiveness(bool interactive)
{
	int s = editmenu->GetMenuItemCount();
	wxMenuItemList edits = editmenu->GetMenuItems();

	if ( interactive )
	{
		openitem->Enable(true);
		ldtrcitem->Enable(true);
		svtrcitem->Enable(true);
		for (int i=0; i<s; i++)
		{
			edits[i]->Enable(true);
		}
		playiitem->Enable(true);
		playcitem->Enable(true);
		playriitem->Enable(true);
		playrcitem->Enable(true);
		if ( ATisEmpty(trace) || ATisEmpty(ATgetNext(trace)) )
		{
			undo->Enable(false);
		}
		if ( ATisEmpty(ecart) )
		{
			redo->Enable(false);
		}
	} else {
		openitem->Enable(false);
    		ldtrcitem->Enable(false);
    		svtrcitem->Enable(false);
		for (int i=0; i<s; i++)
		{
			edits[i]->Enable(false);
		}
		playiitem->Enable(false);
		playcitem->Enable(false);
		playriitem->Enable(false);
		playrcitem->Enable(false);
	}

	this->interactive = interactive;
}

void XSimMain::LoadFile(const wxString &filename)
{
    FILE *f;
    
    if ( (f = fopen(filename.fn_str(),"rb")) == NULL )
    {
	    wxMessageDialog msg(this, wxT("Failed to open file."),
		wxT("Error"), wxOK|wxICON_ERROR);
	    msg.ShowModal();
	    return;
    }

    ATermAppl Spec = (ATermAppl) ATreadFromFile(f);
    fclose(f);

    if ( (Spec == NULL) || !gsIsSpecV1(Spec) )
    {
	    wxMessageDialog msg(this, wxT("Invalid file."),
		wxT("Error"), wxOK|wxICON_ERROR);
	    msg.ShowModal();
	    return;
    }

    SetTitle(base_title+wxT(" - ")+filename);    

    ATermList l = ATLgetArgument(ATAgetArgument(Spec,2),1);
    ATermList m = ATmakeList0();
    ATermList n = ATmakeList0();
    stateview->DeleteAllItems();
    for (int i=0; !ATisEmpty(l); l=ATgetNext(l), i++)
    {
	    wxString s(ATgetName(ATgetAFun(ATAgetArgument(ATAgetFirst(l),0)))
#ifdef wxUSE_UNICODE
			    ,wxConvLocal
#endif
			    );
	    stateview->InsertItem(i,s);
	    m = ATinsert(m,ATgetArgument(ATAgetFirst(l),0));
	    n = ATinsert(n,ATgetFirst(l));
    }
    state_varnames = ATreverse(m);
    state_vars = ATreverse(n);
    
    delete nextstategen;
    delete nextstate;
    nextstate = createNextState(Spec,!use_dummies,GS_STATE_VECTOR,rewr_strat);
    nextstategen = NULL;
    initial_state = nextstate->getInitialState();

    current_state = NULL;
    InitialiseViews();
    Reset(initial_state);
    
    ldtrcitem->Enable(true);
    svtrcitem->Enable(true);
}

void XSimMain::LoadDLL(const wxString &filename)
{
	wxDynamicLibrary lib(filename);

	if ( lib.IsLoaded() )
	{
		void (*f)(SimulatorInterface *);

		f = (void (*)(SimulatorInterface *)) lib.GetSymbol(wxT("SimulatorViewDLLAddView"));
		if ( f != NULL )
		{
			f(this);
			lib.Detach(); //XXX
		} else {
			wxMessageDialog msg(this, wxT("DLL does not appear to contain a View."), wxT("Error"), wxOK|wxICON_ERROR);
			msg.ShowModal();
		}
	} else {
		/*wxMessageDialog msg(this, wxT("Failed to open DLL."), wxT("Error"), wxOK|wxICON_ERROR);
		msg.ShowModal();*/
       }
 }
 

void XSimMain::Register(SimulatorViewInterface *View)
{
	views.push_back(View);
	View->Registered(this);
	if ( !ATisEmpty(trace) )
	{
	        View->Initialise(state_vars);
                View->StateChanged(NULL, current_state, next_states);
		View->TraceChanged(GetTrace(),0);
		View->TracePosChanged(ATAgetFirst(ATLgetFirst(trace)),current_state,ATgetLength(trace)-1);
	}
}

void XSimMain::Unregister(SimulatorViewInterface *View)
{
	views.remove(View);
	View->Unregistered();
}

wxWindow *XSimMain::MainWindow()
{
	return this;
}

ATermList XSimMain::GetParameters()
{
	return state_vars;
}

void XSimMain::Reset()
{
        Reset(nextstate->getInitialState());
}

void XSimMain::Reset(ATerm State)
{
	initial_state = State;
	if ( initial_state != NULL )
	{
		traceReset(initial_state);
		SetCurrentState(initial_state);
		UpdateTransitions();

		for (viewlist::iterator i = views.begin(); i != views.end(); i++)
		{
			(*i)->Reset(initial_state);
			(*i)->StateChanged(NULL,initial_state,next_states);
		}

		undo->Enable(false);
		redo->Enable(false);
	}
}

bool XSimMain::Undo()
{
	if ( ATgetLength(trace) > 1 )
	{
		ATermList l = traceUndo();
		ATerm state = ATgetFirst(ATgetNext(l));

		SetCurrentState(state);
		UpdateTransitions();
		
		for (viewlist::iterator i = views.begin(); i != views.end(); i++)
		{
			(*i)->Undo(1);
			(*i)->StateChanged(NULL,state,next_states);
		}

		if ( interactive )
		{
			if ( ATisEmpty(ATgetNext(trace)) )
			{
				undo->Enable(false);
			}
			redo->Enable();
		}

		return true;
	} else {
		return false;
	}
}

bool XSimMain::Redo()
{
	if ( !ATisEmpty(ecart) )
	{
		ATermList trans = traceRedo();
		ATerm state = ATgetFirst(ATgetNext(trans));

		SetCurrentState(state);
		UpdateTransitions();

		for (viewlist::iterator i = views.begin(); i != views.end(); i++)
		{
			(*i)->Redo(1);
			(*i)->StateChanged(NULL,state,next_states);
		}

		if ( interactive )
		{
			if ( ATisEmpty(ecart) )
			{
				redo->Enable(false);
			}
			undo->Enable();
		}

		return true;
	} else {
		return false;
	}
}

ATerm XSimMain::GetState()
{
	return current_state;
}

ATermList XSimMain::GetNextStates()
{
	return next_states;
}

NextState *XSimMain::GetNextState()
{
	return nextstate;
}

bool XSimMain::ChooseTransition(unsigned int index)
{
	if ( !ATisEmpty(next_states) && (index < ATgetLength(next_states)) )
	{
		Stopper_Enter();

		ATermList l = ATLelementAt(next_states,index);
		ATermAppl trans = ATAgetFirst(l);
		ATerm state = ATgetFirst(ATgetNext(l));

		SetCurrentState(state,true);
		UpdateTransitions();

		traceSetNext(l);

		for (viewlist::iterator i = views.begin(); i != views.end(); i++)
		{
			(*i)->StateChanged(trans,state,next_states);
		}

		long i;
		bool last = true;
		if ( tau_prior->IsChecked() && ((i = transview->FindItem(-1,wxT("tau"))) >= 0) )
		{
			ATbool b;
			ATindexedSetPut(seen_states,current_state,&b);

			bool found = false;
			i=0;
			for (ATermList l=next_states; !ATisEmpty(l); l=ATgetNext(l),i++)
			{
				ATermList trans = ATLgetFirst(l);
				if ( ATisEmpty(ATLgetArgument(ATAgetFirst(trans),0)) )
				{
					if ( ATindexedSetGetIndex(seen_states,ATgetFirst(ATgetNext(trans))) < 0 )
					{
						found = true;
						break;
					}
				}
			}
			if ( found )
			{
				last = false;
				Update();
				wxYield();
				if ( !stopped )
				{
					ChooseTransition(i);
//					ChooseTransition(transview->GetItemData(i));
				}
			}
		}
		if ( last )
		{
			if ( tau_prior->IsChecked() )
			{
				ATindexedSetReset(seen_states);
			}
			/* Should be done after (last) Stopper_Exit(), so not needed here
 			if ( interactive ) {
				undo->Enable();
				redo->Enable(false);
			}*/
		}
		
		Stopper_Exit();

		return true;
	} else {
		return false;
	}
}

int XSimMain::GetTraceLength()
{
	return ATgetLength(trace)+ATgetLength(ecart);
}

int XSimMain::GetTracePos()
{
	return ATgetLength(trace)-1;
}

bool XSimMain::SetTracePos(unsigned int pos)
{
	if ( ATgetLength(trace) == 0 )
	{
		return false;
	}

	unsigned int l = ATgetLength(trace)-1;
	ATermAppl trans;
	ATerm state;

	if ( pos <= l+ATgetLength(ecart) )
	{
		while ( l < pos )
		{
			trace = ATinsert(trace,ATgetFirst(ecart));
			ecart = ATgetNext(ecart);
			l++;
		}
		while ( l > pos )
		{
			ecart = ATinsert(ecart,ATgetFirst(trace));
			trace = ATgetNext(trace);
			l--;
		}

		trans = ATAgetFirst(ATLgetFirst(trace));
		state = ATgetFirst(ATgetNext(ATLgetFirst(trace)));

		SetCurrentState(state);
		UpdateTransitions();

		for (viewlist::iterator i = views.begin(); i != views.end(); i++)
		{
			(*i)->TracePosChanged(trans,state,pos);
			(*i)->StateChanged(NULL,state,next_states);
		}

		undo->Enable(!ATisEmpty(ATgetNext(trace)));
		redo->Enable(!ATisEmpty(ecart));

		return true;
	} else {
		return false;
	}
}

ATermList XSimMain::GetTrace()
{
	ATermList l = ecart;
	ATermList m = trace;

	for (; !ATisEmpty(m); m=ATgetNext(m))
	{
		l = ATinsert(l,ATgetFirst(m));
	}

	return l;
}

bool XSimMain::SetTrace(ATermList /* Trace */, unsigned int /* From */)
{
	// XXX
	return false;
}


void XSimMain::InitialiseViews()
{
	for (viewlist::iterator i = views.begin(); i != views.end(); i++)
	{
		(*i)->Initialise(state_vars);
	}
}


void XSimMain::traceReset(ATerm state)
{
	trace = ATmakeList1((ATerm) ATmakeList2((ATerm) gsMakeNil(), state));
	ecart = ATmakeList0();
}

void XSimMain::traceSetNext(ATermList transition)
{
	trace = ATinsert(trace,(ATerm) transition);
	ecart = ATmakeList0();
}

ATermList XSimMain::traceUndo()
{
	ecart = ATinsert(ecart,ATgetFirst(trace));
	trace = ATgetNext(trace);

	return ATLgetFirst(trace);
}

ATermList XSimMain::traceRedo()
{
	trace = ATinsert(trace,ATgetFirst(ecart));
	ecart = ATgetNext(ecart);

	return ATLgetFirst(trace);
}

void XSimMain::OnOpen( wxCommandEvent& /* event */ )
{
  wxFileDialog dialog( this, wxT("Select a LPS file..."), wxT(""), wxT(""), wxT("LPSs (*.lps)|*.lps|All files|*"),wxFD_CHANGE_DIR);
  if ( dialog.ShowModal() == wxID_OK )
  {
    LoadFile(dialog.GetPath());
  }
}

void XSimMain::OnQuit( wxCommandEvent& /* event */ )
{
     Close( TRUE );
}

void XSimMain::OnUndo( wxCommandEvent& /* event */ )
{
	Undo();
}

void XSimMain::OnRedo( wxCommandEvent& /* event */ )
{
	Redo();
}

void XSimMain::OnReset( wxCommandEvent& /* event */ )
{
	Reset();
}

void XSimMain::OnLoadTrace( wxCommandEvent& /* event */ )
{
    wxFileDialog dialog( this, wxT("Load trace..."), wxT(""), wxT(""), wxT("Traces (*.trc)|*.trc|All Files|*.*"),wxFD_CHANGE_DIR);
    if ( dialog.ShowModal() == wxID_OK )
    {
	    string fn(dialog.GetPath().fn_str());
	    Trace tr;
	    if ( !tr.load(fn) )
	    {
		    wxMessageDialog dialog(this,wxT("Cannot read trace from file.\n"),wxT("Error reading file"),wxOK|wxICON_ERROR);
		    dialog.ShowModal();
		    return;
	    }

	    //SetInteractiveness(false);
	    Stopper_Enter();

	    ATerm state = (ATerm) tr.getState();
	    ATermList newtrace = ATmakeList0();

	    if ( (state != NULL) && ((state = nextstate->parseStateVector((ATermAppl) state)) == NULL) )
	    {
		    wxMessageDialog dialog(this,wxT("Initial state of trace is not a valid state for this specification.\n"),wxT("Error in trace"),wxOK|wxICON_ERROR);
		    dialog.ShowModal();
	    } else {
		    if ( state == NULL )
		    {
			    Reset();
                            state = current_state;
		    } else {
			    Reset(state);
		    }

		    ATermAppl act;
		    while ( (act = tr.getAction()) != NULL )
		    {
			    nextstategen = nextstate->getNextStates(state,nextstategen);
			    if ( gsIsMultAct(act) )
			    {
				    ATermAppl Transition;
				    ATerm NewState;
				    bool found = false;
				    while ( nextstategen->next(&Transition,&NewState) )
				    {
					    if ( ATisEqual(Transition,act) )
					    {
						    if ( (tr.getState() == NULL) || ((NewState = nextstate->parseStateVector(tr.getState(),NewState)) != NULL) )
						    {
							    newtrace = ATinsert(newtrace,(ATerm) ATmakeList2((ATerm) Transition,NewState));
							    state = NewState;
							    found = true;
							    break;
						    }
					    }
				    }
				    if ( !found )
				    {
					    wxString s = wxConvLocal.cMB2WX(PrintPart_CXX((ATerm) act, ppDefault).c_str());
					    wxMessageDialog dialog(this,wxString::Format(wxT("Cannot append transition '%s' to trace.\n"),s.c_str()),wxT("Error in trace"),wxOK|wxICON_ERROR);
					    dialog.ShowModal();
					    break;
				    }
			    } else {
				    // Perhaps trace was in plain text format; try pp-ing actions
				    // XXX Only because libtrace cannot parse text (yet)
				    ATermAppl Transition;
				    ATerm NewState;
				    wxString s(ATgetName(ATgetAFun(act)),wxConvLocal);
				    bool found = false;
				    while ( nextstategen->next(&Transition,&NewState) )
				    {
					    wxString t = wxConvLocal.cMB2WX(PrintPart_CXX((ATerm) Transition, ppDefault).c_str());
					    if ( s == t )
					    {
						    if ( (tr.getState() == NULL) || ((NewState = nextstate->parseStateVector(tr.getState(),NewState)) != NULL) )
						    {
							    newtrace = ATinsert(newtrace,(ATerm) ATmakeList2((ATerm) Transition,NewState));
							    state = NewState;
							    found = true;
							    break;
						    }
					    }
				    }
				    if ( !found )
				    {
					    wxMessageDialog dialog(this,wxString::Format(wxT("Cannot append transition '%s' to trace.\n"),s.c_str()),wxT("Error in trace"),wxOK|wxICON_ERROR);
					    dialog.ShowModal();
					    break;
				    }
			    }	    
		    }
	    }

	    for (ATermList l=newtrace; !ATisEmpty(l); l=ATgetNext(l) )
	    {
		    ecart = ATinsert(ecart,ATgetFirst(l));
	    }
	    newtrace = ATinsert(ecart,ATgetFirst(trace));
	    for (viewlist::iterator i = views.begin(); i != views.end(); i++)
	    {
		    (*i)->TraceChanged(newtrace,0);
	    }
            UpdateTransitions(false);

	    Stopper_Exit();
	    //SetInteractiveness(true);
    }
}

void XSimMain::OnSaveTrace( wxCommandEvent& /* event */ )
{
    wxFileDialog dialog( this, wxT("Save trace..."), wxT(""), wxT(""), wxT("Traces (*.trc)|*.trc|All Files|*.*"),wxFD_SAVE|wxFD_CHANGE_DIR);
    if ( dialog.ShowModal() == wxID_OK )
    {
	    Trace tr;
	    if ( !ATisEmpty(trace) )
	    {
		    ATermList m = ATreverse(trace);
		    tr.setState(nextstate->makeStateVector(ATgetFirst(ATgetNext(ATLgetFirst(m)))));
		    for (ATermList l=ATconcat(ATgetNext(m),ecart); !ATisEmpty(l); l=ATgetNext(l))
		    {
			    tr.addAction(ATAgetFirst(ATLgetFirst(l)));
			    tr.setState(nextstate->makeStateVector(ATgetFirst(ATgetNext(ATLgetFirst(l)))));
/*			PrintPart_CXX(f, ATgetFirst(ATLgetFirst(l)), ppDefault);
                        f << endl;*/
		    }
	    }

	    string fn(dialog.GetPath().fn_str());
	    if ( !tr.save(fn) )
            {
              wxMessageDialog dialog(this,wxT("Cannot save trace to file"),wxT("File error"),wxOK|wxICON_ERROR);
              dialog.ShowModal();
              return;
            }
    }
}

void XSimMain::OnFitCurrentState( wxCommandEvent& /* event */ )
{
    int w,h,n,newpos;

    /* calculate desired height */
    n = stateview_header_height+stateview->GetItemCount()*stateview_item_height;
    /* get current size of stateview */
    stateview->GetClientSize(&w,&h);
    /* position of splitter should be moved by the difference in the
     * current height and the desired height */
    newpos = split->GetSashPosition()-(n-h);
    /* SetSashPosition doesn't seem to like non-positive numbers */
    if ( newpos <= 0 )
      newpos = 1;
    /* reposition splitter */
    split->SetSashPosition(newpos);
#if defined(__WXGTK__)
    /* hack to avoid unnecessary scrollbars */
    stateview->SetClientSize(0,n);
    stateview->SetClientSize(w,n);
#endif
}

void XSimMain::OnTrace( wxCommandEvent& /* event */ )
{
    tracewin->Show(!tracewin->IsShown());
}

void XSimMain::OnLoadView( wxCommandEvent& /* event */ )
{
#if defined(__WINDOWS__)
    static wxString filter("Dynamic Libraries (*.dll)|*.so;*.dll;|All Files|*.*", wxConvLocal);
#else
# if defined(__WXMAC__)
    static wxString filter("Dynamic Libraries (*.dylib)|*.dylib|All Files|*.*", wxConvLocal);
# else
    static wxString filter("Dynamic Libraries (*.so)|*.so|All Files|*.*", wxConvLocal);
# endif
#endif

    wxFileDialog dialog( this, wxT("Select a View Plugin..."), wxT(""), wxT(""), filter);

    dialog.SetDirectory(wxString(wxT(PLUGIN_DIRECTORY), wxConvLocal));

    if ( dialog.ShowModal() == wxID_OK )
    {
	    LoadDLL(dialog.GetPath());
    }
}

void XSimMain::OnShowDCChanged( wxCommandEvent& /* event */ )
{
	UpdateTransitions(false);
}

void XSimMain::OnSetDelay( wxCommandEvent& /* event */ )
{
	wxTextEntryDialog dialog(this,wxT("Enter the delay in milliseconds."),wxT("Set Delay"),wxString::Format(wxT("%d"),timer_interval));

	if ( dialog.ShowModal() == wxID_OK )
	{
		long new_value;
		bool conv = dialog.GetValue().ToLong(&new_value);

		if ( conv && (((int) new_value) >= 0) )
		{
			timer_interval = new_value;
			if ( timer.IsRunning() )
			{
				wxMessageDialog dialog2(this,wxT("New delay will be applied after current run."),wxT("Information"),wxOK|wxICON_INFORMATION);
				dialog2.ShowModal();
			}
		} else {
			wxMessageDialog dialog2(this,wxT("Invalid value supplied."),wxT("Invalid value"),wxOK|wxICON_ERROR);
			dialog2.ShowModal();
		}
	}
}

void XSimMain::OnResetAndPlay( wxCommandEvent& event )
{
	if ( !ATisEmpty(trace) )
	{
		SetTracePos(0);
		OnPlay(event);
	}
}

void XSimMain::OnPlay( wxCommandEvent& /* event */ )
{
	if ( !ATisEmpty(trace) )
	{
		//SetInteractiveness(false);
		Stopper_Enter();
		timer_func = FUNC_PLAY;
		timer.Start(timer_interval);
	}
}

void XSimMain::OnResetAndPlayRandom( wxCommandEvent& event )
{
	if ( !ATisEmpty(trace) )
	{
		Reset();
		OnPlayRandom(event);
	}
}

void XSimMain::OnPlayRandom( wxCommandEvent& /* event */ )
{
	if ( !ATisEmpty(trace) )
	{
		//SetInteractiveness(false);
		Stopper_Enter();
		timer_func = FUNC_RANDOM;
		timer.Start(timer_interval);
	}
}

void XSimMain::OnTimer( wxTimerEvent& /* event */ )
{
	switch ( timer_func )
	{
		case FUNC_PLAY:
			if ( !ATisEmpty(ecart) && !stopped )
			{
				Redo();
				while ( tau_prior->IsChecked() && !ATisEmpty(ecart) && ATisEmpty(ATgetArgument(ATAgetFirst(ATLgetFirst(ecart)),0)))
				{
					Redo();
				}
				Update();
				wxYield();
			} else {
				timer.Stop();
				timer_func = FUNC_NONE;
				Stopper_Exit();
				//SetInteractiveness(true);
			}
			break;
		case FUNC_RANDOM:
			if ( !ATisEmpty(next_states) && !stopped )
			{
				ChooseTransition(rand() % ATgetLength(next_states));
				Update();
				wxYield();
			} else {
				timer.Stop();
				timer_func = FUNC_NONE;
				Stopper_Exit();
				//SetInteractiveness(true);
			}
			break;
		default:
			break;
	}
}

void XSimMain::Stopper_Enter()
{
	if ( stopper_cnt == 0 )
	{
		stopped = false;
		stopitem->Enable(true);
		SetInteractiveness(false);
	}
	stopper_cnt++;
}

void XSimMain::Stopper_Exit()
{
	if ( stopper_cnt > 0 )
	{
		stopper_cnt--;
	}
	if ( stopper_cnt == 0 )
	{
		stopped = true;
		stopitem->Enable(false);
		SetInteractiveness(true);
	}
}

void XSimMain::StopAutomation()
{
	stopper_cnt = 0;
	Stopper_Exit();
}

void XSimMain::OnStop( wxCommandEvent& /* event */ )
{
	StopAutomation();
}

void XSimMain::OnAbout( wxCommandEvent& /* event */ )
{
    wxMessageDialog dialog( this, wxT("mCRL2 Simulator GUI"),
        wxT("About XSim"), wxOK|wxICON_INFORMATION );
    dialog.ShowModal();
}

void XSimMain::OnCloseWindow( wxCloseEvent& /* event */ )
{
    Destroy();
}

void XSimMain::stateOnListItemSelected( wxListEvent& event )
{
	stateview->Select(event.GetIndex(),FALSE);
}

void XSimMain::transOnListItemActivated( wxListEvent& event )
{
	ChooseTransition(event.GetData());
}

void XSimMain::SetCurrentState(ATerm state, bool showchange)
{
	ATerm old;

	if ( current_state == NULL )
	{
		old = state;
	} else {
		old = current_state;
	}
	current_state = state;

	for (unsigned int i=0; i<ATgetLength(state_vars); i++)
	{
		ATermAppl oldval = nextstate->getStateArgument(old,i);
		ATermAppl newval = nextstate->getStateArgument(state,i);

		if ( gsIsDataVarId(newval) )
		{
			stateview->SetItem(i,1,wxT("_"));
		} else {
			stateview->SetItem(i,1,wxConvLocal.cMB2WX(PrintPart_CXX((ATerm) newval, ppDefault).c_str()));
		}
		if ( showchange && !(ATisEqual(oldval,newval) || (gsIsDataVarId(oldval) && gsIsDataVarId(newval)) ) )
		{
		        wxColour col(255,255,210);
		        stateview->SetItemBackgroundColour(i,col);
		} else {
		        wxColour col(255,255,255); // XXX is this the correct colour?
		        stateview->SetItemBackgroundColour(i,col);
		}
	}

        stateview->SetColumnWidth(1,stateview->GetClientSize().GetWidth() - stateview->GetColumnWidth(0));
}

static void sort_transitions(wxArrayString &actions, wxArrayString &statechanges, wxArrayInt &indices)
{
	int len = indices.GetCount();
	int end = len;

	for (int i=0; i<end; i++)
	{
		if ( actions[i] == wxT("tau") )
		{
			wxString s;
			int h;

			s = actions[i];
			actions[i] = actions[end-1];
			actions[end-1] = s;

			s = statechanges[i];
			statechanges[i] = statechanges[end-1];
			statechanges[end-1] = s;

			h = indices[i];
			indices[i] = indices[end-1];
			indices[end-1] = h;

			end--;
			i--;
		} else {
			int j = i;
			while ( (j > 0) && ( (actions[j] < actions[j-1]) || ((actions[j] == actions[j-1]) && (statechanges[j] < statechanges[j-1])) ) )
			{
				wxString s;
				int h;
			
				s = actions[j];
				actions[j] = actions[j-1];
				actions[j-1] = s;

				s = statechanges[j];
				statechanges[j] = statechanges[j-1];
				statechanges[j-1] = s;

				h = indices[j];
				indices[j] = indices[j-1];
				indices[j-1] = h;

				j--;
			}
		}
	}
	for (int i=end+1; i<len; i++)
	{
		int j = i;
		while ( (j > end) && ( (actions[j] < actions[j-1]) || ((actions[j] == actions[j-1]) && (statechanges[j] < statechanges[j-1])) ) )
		{
			wxString s;
			int h;
		
			s = actions[j];
			actions[j] = actions[j-1];
			actions[j-1] = s;

			s = statechanges[j];
			statechanges[j] = statechanges[j-1];
			statechanges[j-1] = s;

			h = indices[j];
			indices[j] = indices[j-1];
			indices[j-1] = h;

			j--;
		}
	}
}

void XSimMain::UpdateTransitions(bool update_next_states)
{
	wxArrayString actions;
	wxArrayString statechanges;
	wxArrayInt indices;
	
	ATermAppl trace_next_transition;
	ATermAppl trace_next_state;
	wxArrayInt trace_next;
	if ( ATisEmpty(ecart) )
	{
		trace_next_transition = NULL;
		trace_next_state = NULL;
	} else {
		ATermList hd = ATLgetFirst(ecart);
		trace_next_transition = ATAgetFirst(hd);
		trace_next_state = nextstate->makeStateVector(ATgetFirst(ATgetNext(hd)));
	}

	if ( update_next_states )
	{
		nextstategen = nextstate->getNextStates(current_state,nextstategen);
		next_states = ATmakeList0();
		ATermAppl transition;
		ATerm newstate;
		while ( nextstategen->next(&transition,&newstate) )
		{
			next_states = ATinsert(next_states,(ATerm) ATmakeList2((ATerm) transition,newstate));
		}
		if ( nextstategen->errorOccurred() )
		{
			wxMessageDialog msg(this,wxT("An error occurred while calculating the transitions from this state. This likely means that not all possible transitions are shown."),wxT("Error while calculating transitions"),wxOK|wxICON_ERROR);
			msg.ShowModal();
			StopAutomation();
		}
	}

	transview->DeleteAllItems();
	int i = 0;
	for (ATermList l=next_states; !ATisEmpty(l); l=ATgetNext(l), i++)
	{
		actions.Add(wxConvLocal.cMB2WX(PrintPart_CXX(ATgetFirst(ATLgetFirst(l)), ppDefault).c_str()));
		indices.Add(i);
		if ( (trace_next_state != NULL) &&
			ATisEqual(ATgetFirst(ATLgetFirst(l)),trace_next_transition) &&
			(nextstate->parseStateVector(trace_next_state,ATgetFirst(ATgetNext(ATLgetFirst(l)))) != NULL) )
		{
			trace_next.Add(1);
		} else {
			trace_next.Add(0);
		}
//		transview->SetItemData(i,i);
		stringstream ss;
		ATerm m = current_state;
		ATerm n = ATgetFirst(ATgetNext(ATLgetFirst(l)));
		ATermList o = state_varnames;
		bool comma = false;
		for (unsigned int i=0; i<ATgetLength(state_vars); i++)
		{
			ATermAppl oldval = nextstate->getStateArgument(m,i);
			ATermAppl newval = nextstate->getStateArgument(n,i);

			if ( !ATisEqual(oldval,newval) && (!gsIsDataVarId(newval) || showdc->IsChecked()) )
			{
				if ( comma )
				{
					ss << ", ";
				} else {
					comma = true;
				}
				PrintPart_CXX(ss, ATgetFirst(o), ppDefault);
				ss << " := ";
				if ( gsIsDataVarId(newval) )
				{
					ss << "_";
				} else {
					PrintPart_CXX(ss, (ATerm) newval, ppDefault);
				}
			}

			o = ATgetNext(o);
		}
//		transview->SetItem(i,1,s);
		statechanges.Add(wxConvLocal.cMB2WX(ss.str().c_str()));
	}

	sort_transitions(actions,statechanges,indices);
	int next = -1;
	for (unsigned int i=0; i<indices.GetCount(); i++)
	{
		transview->InsertItem(i,actions[i]);
		transview->SetItem(i,1,statechanges[i]);
		transview->SetItemData(i,indices[i]);
		if ( (next < 0) && (trace_next[indices[i]] == 1) )
		{
			next = i;
		}
	}

	if ( !ATisEmpty(next_states) )
	{
		if ( next < 0 )
		{
			transview->Select(0);
		} else {
			transview->Select(next);
		}
	}

        /* Adapt column width */
	transview->SetColumnWidth(1,wxLIST_AUTOSIZE);
        transview->SetColumnWidth(1,transview->GetClientSize().GetWidth() - transview->GetColumnWidth(0));
}


