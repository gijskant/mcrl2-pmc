#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
    #pragma implementation "xsimtracedll.h"
#endif

#include <wx/wxprec.h>

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include <sstream>
#include <aterm2.h>
#include "xsimtracedll.h"
#include "liblowlevel.h"
#include "libstruct.h"
#include "libprint_types.h"
#include "libprint.h"
#include "libnextstate.h"

using namespace std;

//------------------------------------------------------------------------------
// XSimMain
//------------------------------------------------------------------------------

const int ID_LISTVIEW = 10101;

BEGIN_EVENT_TABLE(XSimTraceDLL,wxFrame)
    EVT_CLOSE(XSimTraceDLL::OnCloseWindow)
    EVT_LIST_ITEM_ACTIVATED(ID_LISTVIEW,XSimTraceDLL::OnListItemActivated)
END_EVENT_TABLE()

static void PrintState(stringstream &ss, ATerm state, NextState *ns)
{
        for (int i=0; i<ns->getStateLength(); i++)
        {
                if ( i > 0 )
                {
			ss << ", ";
		}

                ATermAppl a = ns->getStateArgument(state,i);
                if ( gsIsDataVarId(a) )
                {
			ss << "_";
                } else {
                        PrintPart_CXX(ss, (ATerm) a, ppDefault);
                }
        }
}

XSimTraceDLL::XSimTraceDLL( wxWindow *parent ) :
    wxFrame( parent, -1, wxT("XSim Trace (DLL)"), wxDefaultPosition, wxSize(300,400), wxDEFAULT_FRAME_STYLE )
{
    wxPanel *panel = new wxPanel(this,-1);
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    wxStaticBox *box = new wxStaticBox(panel,-1,wxT("Transitions"));
    wxStaticBoxSizer *boxsizer = new wxStaticBoxSizer(box,wxVERTICAL);
    traceview = new wxListView(panel,ID_LISTVIEW,wxDefaultPosition,wxSize(0,0),wxLC_REPORT|wxSUNKEN_BORDER|wxLC_HRULES|wxLC_VRULES|wxLC_SINGLE_SEL);
    traceview->InsertColumn(0,wxT("#"));
    traceview->InsertColumn(1,wxT("Action"));
    traceview->InsertColumn(2,wxT("State"));
    traceview->SetColumnWidth(0,30);
    traceview->SetColumnWidth(1,120);
    boxsizer->Add(traceview,1,wxGROW|wxALIGN_CENTER|wxALL,5);
    sizer->Add(boxsizer,1,wxGROW|wxALIGN_CENTER|wxALL,5);
    panel->SetSizer(sizer);

    simulator = NULL;
}

void XSimTraceDLL::Registered(SimulatorInterface *Simulator)
{
	SimulatorViewDLLInterface::Registered(Simulator);

	simulator = Simulator;
}

void XSimTraceDLL::Unregistered()
{
	SimulatorViewDLLInterface::Unregistered();

	simulator = NULL;
	traceview->DeleteAllItems();
}

void XSimTraceDLL::Initialise(ATermList /* Pars */)
{
}

void XSimTraceDLL::AddState(ATermAppl Transition, ATerm State, bool enabled)
{
	if ( Transition != NULL )
	{
		stringstream ss;
		unsigned int l = traceview->GetItemCount();

		traceview->InsertItem(l,wxString::Format(wxT("%u"),l));
		traceview->SetItem(l,1,wxConvLocal.cMB2WX(PrintPart_CXX((ATerm) Transition, ppDefault).c_str()));
		PrintState(ss,State,simulator->GetNextState());
		traceview->SetItem(l,2,wxConvLocal.cMB2WX(ss.str().c_str()));
		traceview->SetColumnWidth(2,wxLIST_AUTOSIZE);
		if ( enabled )
		{
			wxColor col(255,255,255);
			traceview->SetItemBackgroundColour(l,col);
		} else {
			wxColor col(245,245,245);
			traceview->SetItemBackgroundColour(l,col);
		}
	}
}

void XSimTraceDLL::StateChanged(ATermAppl Transition, ATerm State, ATermList /* NextStates */)
{
	if ( Transition != NULL )
	{
		unsigned int l = traceview->GetItemCount()-1;

		while ( l > current_pos )
		{
			traceview->DeleteItem(l);
			l--;
		}
		AddState(Transition,State,true);
		current_pos++;
	}
}

void XSimTraceDLL::Reset(ATerm State)
{
	stringstream ss;

	traceview->DeleteAllItems();
	traceview->InsertItem(0,wxT("0"));
	traceview->SetItem(0,1,wxT(""));
	PrintState(ss,State,simulator->GetNextState());
	traceview->SetItem(0,2,wxConvLocal.cMB2WX(ss.str().c_str()));
	traceview->SetColumnWidth(2,wxLIST_AUTOSIZE);
	current_pos = 0;
}

void XSimTraceDLL::Undo(unsigned int Count)
{
	while ( Count > 0 )
	{
		wxColor col(245,245,245);
		traceview->SetItemBackgroundColour(current_pos,col);
		current_pos--;
		Count--;
	}
}

void XSimTraceDLL::Redo(unsigned int Count)
{
	while ( Count > 0 )
	{
		wxColor col(255,255,255);
		current_pos++;
		traceview->SetItemBackgroundColour(current_pos,col);
		Count--;
	}
}

void XSimTraceDLL::TraceChanged(ATermList Trace, unsigned int From)
{
	unsigned int l = traceview->GetItemCount();
	
	while ( l > From )
	{
		l--;
		traceview->DeleteItem(l);
	}

	for (; !ATisEmpty(Trace); Trace=ATgetNext(Trace))
	{
		if ( From == 0 )
		{
			Reset(ATgetFirst(ATgetNext(ATLgetFirst(Trace))));
		} else {
			AddState(ATAgetFirst(ATLgetFirst(Trace)),ATgetFirst(ATgetNext(ATLgetFirst(Trace))),current_pos >= From);
		}
		From++;
	}
}

void XSimTraceDLL::TracePosChanged(ATermAppl /* Transition */, ATerm /* State */, unsigned int Index)
{
	while ( current_pos > Index )
	{
		Undo(1);
	}
	while ( current_pos < Index )
	{
		Redo(1);
	}
}


void XSimTraceDLL::OnCloseWindow( wxCloseEvent& /* event */)
{
	if ( xsimdll != NULL )
	{
		xsimdll->Remove(this,true);
	}
	Destroy();
}

void XSimTraceDLL::OnListItemActivated( wxListEvent &event )
{
	if ( simulator != NULL )
	{
		simulator->SetTracePos(event.GetIndex());
	}
}


static XSimViewsDLL *xsimdll;

extern "C" void SimulatorViewDLLAddView(SimulatorInterface *Simulator)
{
	XSimTraceDLL *v;
	v = new XSimTraceDLL(Simulator->MainWindow());
	v->Show();
	v->SetXSimViewsDLL(xsimdll);
	xsimdll->Add(v,Simulator);
}

extern "C" __attribute__((constructor)) void SimulatorViewDLLInit()
{
	gsEnableConstructorFunctions();
	xsimdll = new XSimViewsDLL;
}

extern "C" __attribute__((destructor)) void SimulatorViewDLLCleanUp()
{
	delete xsimdll;
}
