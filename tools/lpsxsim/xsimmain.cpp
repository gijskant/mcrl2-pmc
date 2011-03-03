// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file xsimmain.cpp

#include "wx.hpp" // precompiled headers

#if defined(__GNUG__) && !defined(NO_GCC_PRAGMA)
#pragma implementation "xsimmain.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <wx/dynlib.h>
#include <wx/config.h>
#include <wx/textfile.h>
#include <wx/dynarray.h>
#include <wx/textfile.h>
#include <sstream>
#include <cstdlib>
#include <aterm2.h>
#include "simbase.h"
#include "xsimmain.h"
#include "mcrl2/lps/nextstate.h"
#include "mcrl2/core/print.h"
#include "mcrl2/core/aterm_ext.h"

// For compatibility with older wxWidgets versions (pre 2.8)
#if (wxMINOR_VERSION < 8)
# define wxFD_SAVE wxSAVE
# define wxFD_CHANGE_DIR wxCHANGE_DIR
#endif

using namespace std;
using namespace mcrl2::core;
using namespace mcrl2::core::detail;

int wxCALLBACK MyCompareFunction(long item1, long item2, long WXUNUSED(sortData))
{
    // inverse the order
    if (item1 < item2)
        return -1;
    if (item1 > item2)
        return 1;

    return 0;
}


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
  EVT_MENU(ID_TAU, XSimMain::OnTauPrioritisation)
  EVT_MENU(ID_SHOWDC, XSimMain::OnShowDCChanged)
  EVT_MENU(ID_DELAY, XSimMain::OnSetDelay)
  EVT_MENU(ID_PLAYI, XSimMain::OnResetAndPlay)
  EVT_MENU(ID_PLAYC, XSimMain::OnPlay)
  EVT_MENU(ID_PLAYRI, XSimMain::OnResetAndPlayRandom)
  EVT_MENU(ID_PLAYRC, XSimMain::OnPlayRandom)
  EVT_MENU(ID_STOP, XSimMain::OnStop)
  EVT_TIMER(-1, XSimMain::OnTimer)
  EVT_CLOSE(XSimMain::OnCloseWindow)
  EVT_LIST_ITEM_SELECTED(ID_LISTCTRL1, XSimMain::stateOnListItemSelected)
  EVT_LIST_ITEM_SELECTED(ID_LISTCTRL2, XSimMain::transOnListItemSelected)
  EVT_LIST_ITEM_DESELECTED(ID_LISTCTRL2, XSimMain::transOnListItemDeSelected)
  EVT_LIST_ITEM_ACTIVATED(ID_LISTCTRL2, XSimMain::transOnListItemActivated)
END_EVENT_TABLE()

XSimMain::XSimMain(wxWindow* parent, wxWindowID id, const wxString& title,
                   const wxPoint& position, const wxSize& size, long style) :
  wxFrame(parent, id, title, position, size, style) ,
  timer(this)
{
  base_title = title;

  CreateMenu();
  CreateStatus();
  CreateContent();

  /* Attach resize event handler */
  Connect(id, wxEVT_SIZE, wxSizeEventHandler(XSimMain::OnResize), 0, this);
  Connect(id, wxEVT_MAXIMIZE, wxMaximizeEventHandler(XSimMain::OnMaximize), 0, this);

  state_varnames = ATmakeList0();
  ATprotectList(&state_varnames);
  current_state = NULL;
  ATprotect(&current_state);

  simulator = new StandardSimulatorGUI(this);
  simulator->Register(this);

  tracewin = new XSimTrace(this);
  simulator->Register(tracewin);
  //tracewin->Show(FALSE); // default, so not needed

  wxConfig config(wxT("xsimrc"));
  if (config.HasGroup(wxT("LoadLibrary")))
  {
    config.SetPath(wxT("/LoadLibrary"));

    wxString s;
    long i;
    bool b = config.GetFirstEntry(s,i);
    while (b)
    {
      string t(config.Read(s,wxT("")).mb_str(wxConvUTF8));
      simulator->LoadView(t);
      b = config.GetNextEntry(s,i);
    }
  }

  interactive = true;
  stopper_cnt = 0;
  timer_interval = 1000;
}

XSimMain::~XSimMain()
{
  simulator->Unregister(tracewin);
  simulator->Unregister(this);

  delete tracewin;

  ATunprotectList(&state_varnames);
  ATunprotect(&current_state);

#if wxCHECK_VERSION(2, 9, 0)
  this->PopEventHandler(true);
#endif

  delete simulator;
}

void XSimMain::CreateMenu()
{
  menu = new wxMenuBar;

  wxMenu* file = new wxMenu;
  openitem = file->Append(wxID_OPEN, wxT("&Open...	CTRL-o"), wxT(""));
  file->AppendSeparator();
  ldtrcitem = file->Append(ID_LOADTRACE, wxT("&Load trace...	CTRL-l"), wxT(""));
  ldtrcitem->Enable(false);
  svtrcitem = file->Append(ID_SAVETRACE, wxT("&Save trace...	CTRL-s"), wxT(""));
  svtrcitem->Enable(false);
  file->AppendSeparator();
  file->Append(wxID_EXIT, wxT("&Quit	CTRL-q"), wxT(""));
  menu->Append(file, wxT("&File"));

  editmenu = new wxMenu;
  undo = editmenu->Append(ID_UNDO, wxT("&Undo	CTRL-LEFT"), wxT(""));
  undo->Enable(false);
  redo = editmenu->Append(ID_REDO, wxT("Re&do	CTRL-RIGHT"), wxT(""));
  redo->Enable(false);
  editmenu->Append(ID_RESET, wxT("&Reset	CTRL-r"), wxT(""));
  menu->Append(editmenu, wxT("&Edit"));

  wxMenu* sim = new wxMenu;
  playiitem = sim->Append(ID_PLAYI, wxT("Play Trace from Initial State"), wxT(""));
  playcitem = sim->Append(ID_PLAYC, wxT("Play Trace from Current State"), wxT(""));
  playriitem = sim->Append(ID_PLAYRI, wxT("Random Play from Initial State"), wxT(""));
  playrcitem = sim->Append(ID_PLAYRC, wxT("Random Play from Current State"), wxT(""));
  stopitem = sim->Append(ID_STOP, wxT("Stop	DEL"), wxT(""));
  stopitem->Enable(false);
  menu->Append(sim, wxT("&Automation"));

  wxMenu* opts = new wxMenu;
  tau_prior = opts->Append(ID_TAU, wxT("Enable Tau Prioritisation"), wxT(""), wxITEM_CHECK);
  showdc = opts->Append(ID_SHOWDC, wxT("Show Don't Cares in State Changes"), wxT(""), wxITEM_CHECK);
  opts->Append(ID_DELAY, wxT("Set Play Delay"), wxT(""));
  opts->Append(ID_FITCS, wxT("F&it to Current State	CTRL-f"), wxT(""));
  menu->Append(opts, wxT("&Options"));

  wxMenu* views = new wxMenu;
  views->Append(ID_TRACE, wxT("&Trace	CTRL-t"), wxT(""));
  views->Append(ID_MENU, wxT("&Graph"), wxT(""))->Enable(false);
  views->Append(ID_LOADVIEW, wxT("&Load Plugin..."), wxT(""));
  menu->Append(views, wxT("&Views"));

  wxMenu* help = new wxMenu;
  help->Append(wxID_HELP, wxT("&Contents"), wxT("Show help contents"));
  help->AppendSeparator();
  help->Append(wxID_ABOUT, wxT("&About"));
  menu->Append(help, wxT("&Help"));

  SetMenuBar(menu);
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

void XSimMain::UpdateSizes()
{
  int s  = stateview->GetClientSize().GetWidth() - stateview->GetColumnWidth(0);

  if (s <= 80)
  {
    s = wxLIST_AUTOSIZE;
  }

  /* Set column width of stateview, if necessary */
  if (stateview->GetColumnWidth(1) != s)
  {
    stateview->SetColumnWidth(1, s);
  }

  s  = transview->GetClientSize().GetWidth() - transview->GetColumnWidth(0);

  if (s <= 80)
  {
    s = wxLIST_AUTOSIZE;
  }

  /* Set column width of transview, if necessary */
  if (transview->GetColumnWidth(1) != s)
  {
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

void XSimMain::OnMaximize(wxMaximizeEvent& event)
{
  event.Skip();

  UpdateSizes();
}

void XSimMain::OnResize(wxSizeEvent& event)
{
  event.Skip();

  UpdateSizes();
}

void XSimMain::SetInteractiveness(bool interactive)
{
  size_t s = editmenu->GetMenuItemCount();
  wxMenuItemList edits = editmenu->GetMenuItems();

  if (interactive)
  {
    openitem->Enable(true);
    ldtrcitem->Enable(true);
    svtrcitem->Enable(true);
    for (size_t i=0; i<s; i++)
    {
      edits[i]->Enable(true);
    }
    playiitem->Enable(true);
    playcitem->Enable(true);
    playriitem->Enable(true);
    playrcitem->Enable(true);
    if (simulator->GetTracePos() == 0)
    {
      undo->Enable(false);
    }
    if (simulator->GetTracePos() == simulator->GetTraceLength()-1)
    {
      redo->Enable(false);
    }
  }
  else
  {
    openitem->Enable(false);
    ldtrcitem->Enable(false);
    svtrcitem->Enable(false);
    for (size_t i=0; i<s; i++)
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

void XSimMain::LoadFile(const wxString& filename)
{
  try
  {
    m_specification.load(std::string(filename.mb_str(wxConvUTF8)));
  }
  catch (std::exception const& e)
  {
    wxMessageDialog(this, wxString(e.what(), wxConvLocal), wxT("Error"), wxOK|wxICON_ERROR).ShowModal();
    return;
  }

  SetTitle(base_title+wxT(" - ")+filename);

  ldtrcitem->Enable(true);
  svtrcitem->Enable(true);

  try
  {
    simulator->LoadSpec(m_specification);
  }
  catch (std::exception const& e)
  {
    wxMessageDialog(this, wxString(e.what(), wxConvLocal), wxT("Error"),
                    wxOK | wxICON_ERROR).ShowModal();
    return;
  }

}

void XSimMain::LoadDLL(const wxString& filename)
{
  string s(filename.mb_str(wxConvUTF8));
  simulator->LoadView(s);
}


void XSimMain::Registered(SimulatorInterface* /*Simulator*/)
{
}

void XSimMain::Unregistered()
{
}

void XSimMain::Initialise(ATermList Pars)
{
  state_varnames = ATmakeList0();
  stateview->DeleteAllItems();
  for (long i=0; !ATisEmpty(Pars); Pars=ATgetNext(Pars), i++)
  {
    wxString s(ATgetName(ATgetAFun(ATAgetArgument(ATAgetFirst(Pars),0)))
#ifdef wxUSE_UNICODE
               ,wxConvLocal
#endif
              );
    stateview->InsertItem(i,s);
    state_varnames = ATinsert(state_varnames,ATgetArgument(ATAgetFirst(Pars),0));
  }
  state_varnames = ATreverse(state_varnames);
  current_state = NULL;
}

void XSimMain::StateChanged(ATermAppl /*Transition*/, ATerm State, ATermList NextStates)
{
  try
  {
    SetCurrentState(State);
    UpdateTransitions(NextStates);
    // if ( simulator->ErrorOccurred() )
  }
  catch (mcrl2::runtime_error e)
  {
    // TODO: print error message.
    wxString mystring(e.what(), wxConvUTF8);
    wxMessageDialog msg(this,
                        mystring,
                        wxT("Error while calculating transitions"),
                        wxOK|wxICON_ERROR);
    msg.ShowModal();
    StopAutomation();
  }
  if (interactive)
  {
    undo->Enable(simulator->GetTracePos()>0);
    redo->Enable(simulator->GetTracePos() != simulator->GetTraceLength()-1);
  }
}

void XSimMain::Reset(ATerm /*State*/)
{
}

void XSimMain::Undo(size_t /* Count */)
{
}

void XSimMain::Redo(size_t /* Count */)
{
}

void XSimMain::TracePosChanged(ATermAppl /*Transition*/, ATerm /*State*/, size_t /* Index */)
{
}

void XSimMain::TraceChanged(ATermList /* Trace */, size_t /* From */)
{
}



void XSimMain::OnOpen(wxCommandEvent& /* event */)
{
  wxFileDialog dialog(this, wxT("Select a LPS file..."), wxT(""), wxT(""), wxT("LPSs (*.lps)|*.lps|All files|*"),wxFD_CHANGE_DIR);
  if (dialog.ShowModal() == wxID_OK)
  {
    LoadFile(dialog.GetPath());
  }
}

void XSimMain::OnQuit(wxCommandEvent& /* event */)
{
  Close(TRUE);
}

void XSimMain::OnUndo(wxCommandEvent& /* event */)
{
  simulator->Undo();
}

void XSimMain::OnRedo(wxCommandEvent& /* event */)
{
  simulator->Redo();
}

void XSimMain::OnReset(wxCommandEvent& /* event */)
{
  simulator->Reset();
}

void XSimMain::OnLoadTrace(wxCommandEvent& /* event */)
{
  try
  {
    wxFileDialog dialog(this, wxT("Load trace..."), wxT(""), wxT(""), wxT("Traces (*.trc)|*.trc|All Files|*.*"),wxFD_CHANGE_DIR);
    if (dialog.ShowModal() == wxID_OK)
    {
      string fn(dialog.GetPath().mb_str(wxConvUTF8));
      Stopper_Enter();
      try
      {
        simulator->LoadTrace(fn);
      }
      catch (string err)
      {
        wxMessageDialog dialog(this,wxConvLocal.cMB2WX(err.c_str()),wxT("Error loading trace"),wxOK|wxICON_ERROR);
        dialog.ShowModal();
      }
      Stopper_Exit();
    }
  }
  catch (mcrl2::runtime_error e)
  {
    wxMessageDialog dialog(this, wxString(e.what(), wxConvUTF8),wxT("Error loading trace"),wxOK|wxICON_ERROR);
    dialog.ShowModal();
  }
}

void XSimMain::OnSaveTrace(wxCommandEvent& /* event */)
{
  wxFileDialog dialog(this, wxT("Save trace..."), wxT(""), wxT(""), wxT("Traces (*.trc)|*.trc|All Files|*.*"),wxFD_SAVE|wxFD_CHANGE_DIR);
  if (dialog.ShowModal() == wxID_OK)
  {
    string fn(dialog.GetPath().mb_str(wxConvUTF8));
    try
    {
      simulator->SaveTrace(fn);
    }
    catch (string err)
    {
      wxMessageDialog dialog(this,wxConvLocal.cMB2WX(err.c_str()),wxT("Error saving trace"),wxOK|wxICON_ERROR);
      dialog.ShowModal();
    }
  }
}

void XSimMain::OnFitCurrentState(wxCommandEvent& /* event */)
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
  if (newpos <= 0)
  {
    newpos = 1;
  }
  /* reposition splitter */
  split->SetSashPosition(newpos);
#if defined(__WXGTK__)
  /* hack to avoid unnecessary scrollbars */
  stateview->SetClientSize(0,n);
  stateview->SetClientSize(w,n);
#endif
}

void XSimMain::OnTrace(wxCommandEvent& /* event */)
{
  tracewin->Show(!tracewin->IsShown());
}

void XSimMain::OnLoadView(wxCommandEvent& /* event */)
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

  wxFileDialog dialog(this, wxT("Select a View Plugin..."), wxT(""), wxT(""), filter);

  dialog.SetDirectory(wxString(PLUGINDIR, wxConvLocal));

  if (dialog.ShowModal() == wxID_OK)
  {
    LoadDLL(dialog.GetPath());
  }
}

void XSimMain::OnTauPrioritisation(wxCommandEvent& /* event */)
{
  simulator->SetTauPrioritisation(tau_prior->IsChecked());
}

void XSimMain::OnShowDCChanged(wxCommandEvent& /* event */)
{
  UpdateTransitions(simulator->GetNextStates());
}

void XSimMain::OnSetDelay(wxCommandEvent& /* event */)
{
  wxTextEntryDialog dialog(this,wxT("Enter the delay in milliseconds."),wxT("Set Delay"),wxString::Format(wxT("%d"),timer_interval));

  if (dialog.ShowModal() == wxID_OK)
  {
    long new_value;
    bool conv = dialog.GetValue().ToLong(&new_value);

    if (conv && (((int) new_value) >= 0))
    {
      timer_interval = new_value;
      if (timer.IsRunning())
      {
        wxMessageDialog dialog2(this,wxT("New delay will be applied after current run."),wxT("Information"),wxOK|wxICON_INFORMATION);
        dialog2.ShowModal();
      }
    }
    else
    {
      wxMessageDialog dialog2(this,wxT("Invalid value supplied."),wxT("Invalid value"),wxOK|wxICON_ERROR);
      dialog2.ShowModal();
    }
  }
}

void XSimMain::OnResetAndPlay(wxCommandEvent& event)
{
  if (!ATisEmpty(simulator->GetTrace()))
  {
    simulator->SetTracePos(0);
    OnPlay(event);
  }
}

void XSimMain::OnPlay(wxCommandEvent& /* event */)
{
  if (simulator->IsActive())
  {
    //SetInteractiveness(false);
    Stopper_Enter();
    timer_func = FUNC_PLAY;
    timer.Start(timer_interval);
  }
}

void XSimMain::OnResetAndPlayRandom(wxCommandEvent& event)
{
  if (simulator->IsActive())
  {
    simulator->Reset();
    OnPlayRandom(event);
  }
}

void XSimMain::OnPlayRandom(wxCommandEvent& /* event */)
{
  if (simulator->IsActive())
  {
    //SetInteractiveness(false);
    Stopper_Enter();
    timer_func = FUNC_RANDOM;
    timer.Start(timer_interval);
  }
}

static bool IsTau(ATermAppl Transition)
{
  return (Transition != NULL) && ATisEmpty(ATLgetArgument(Transition,0));
}
void XSimMain::OnTimer(wxTimerEvent& /* event */)
{
  switch (timer_func)
  {
    case FUNC_PLAY:
      if ((simulator->GetTracePos() < simulator->GetTraceLength()-1) && !stopped)
      {
        simulator->Redo();
        while (tau_prior->IsChecked() && IsTau(simulator->GetNextTransitionFromTrace()))
        {
          simulator->Redo();
        }
        Update();
        wxYield();
      }
      else
      {
        timer.Stop();
        timer_func = FUNC_NONE;
        Stopper_Exit();
        //SetInteractiveness(true);
      }
      break;
    case FUNC_RANDOM:
      if (!ATisEmpty(simulator->GetNextStates()) && !stopped)
      {
        try
        {
          simulator->ChooseTransition(rand() % ATgetLength(simulator->GetNextStates()));
        }
        catch (std::exception const& e)
        {
          wxMessageDialog(this, wxString(e.what(), wxConvLocal), wxT("Error"),
                          wxOK | wxICON_ERROR).ShowModal();
          return;
        }
        Update();
        wxYield();
      }
      else
      {
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
  if (stopper_cnt == 0)
  {
    stopped = false;
    stopitem->Enable(true);
    SetInteractiveness(false);
  }
  stopper_cnt++;
}

void XSimMain::Stopper_Exit()
{
  if (stopper_cnt > 0)
  {
    stopper_cnt--;
  }
  if (stopper_cnt == 0)
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

void XSimMain::OnStop(wxCommandEvent& /* event */)
{
  StopAutomation();
}

void XSimMain::OnCloseWindow(wxCloseEvent& /* event */)
{
  Destroy();
}

void XSimMain::stateOnListItemSelected(wxListEvent& event)
{
  stateview->Select(event.GetIndex(),FALSE);
}

void XSimMain::transOnListItemDeSelected(wxListEvent& event)
{
  transview->SetToolTip( wxT("No transition selected.") );
}

void XSimMain::transOnListItemSelected(wxListEvent& event)
{
  wxString TransTooltip = wxT("Selected transition:\n");
  TransTooltip.Append( wxT("- Action:\n") );

  wxListItem     info;

  //Prepare Cell: (x,0)
  info.m_itemId = event.GetIndex();
  info.m_col = 0;
  info.m_mask = wxLIST_MASK_TEXT;
  transview->GetItem( info );

  // Extract the text from cell
  TransTooltip.Append( wxT("\t") + info.m_text + wxT("\n") );

  TransTooltip.Append( wxT("- State Change:\n") );

  //Prepare Cell: (x,1)
  info.m_col = 1;
  transview->GetItem( info );
  wxString tmp = info.m_text;
  if( !tmp.IsEmpty() )
  {
    tmp.Replace(wxT(","), wxT("\n\t"));
    TransTooltip.Append( wxT("\t ") + tmp ) ;
  } else {
    TransTooltip.Append( wxT("\t -")) ;
  }
  transview->SetToolTip( TransTooltip );
}


void XSimMain::transOnListItemActivated(wxListEvent& event)
{
  try
  {
    simulator->ChooseTransition(event.GetData());
  }
  catch (std::exception const& e)
  {
    wxMessageDialog(this, wxString(e.what(), wxConvLocal), wxT("Error"),
                    wxOK | wxICON_ERROR).ShowModal();
    return;
  }
}

void XSimMain::SetCurrentState(ATerm state, bool showchange)
{
  ATerm old;

  if (current_state == NULL)
  {
    old = state;
  }
  else
  {
    old = current_state;
  }
  current_state = state;


  wxString tooltip_string;
  NextState* nextstate = simulator->GetNextState();
  for (long i=0; static_cast<size_t>(i)<ATgetLength(state_varnames); i++)
  {
    ATermAppl oldval = nextstate->getStateArgument(old,i);
    ATermAppl newval = nextstate->getStateArgument(state,i);

    if (mcrl2::data::is_variable(newval))
    {
      stateview->SetItem(i,1,wxT("_"));
    }
    else
    {
      stateview->SetItem(i,1,wxConvLocal.cMB2WX(PrintPart_CXX((ATerm) newval, ppDefault).c_str()));
    }

    tooltip_string.Append(wxT(" ")+stateview->GetItemText(i) + wxT(" = ")+ wxConvLocal.cMB2WX(PrintPart_CXX((ATerm) newval, ppDefault).c_str()) + wxTextFile::GetEOL());

    if (showchange && !(ATisEqual(oldval,newval) || (mcrl2::data::is_variable(oldval) && mcrl2::data::is_variable(newval))))
    {
      wxColour col(255,255,210);
      stateview->SetItemBackgroundColour(i,col);
    }
    else
    {
      wxColour col(255,255,255); // XXX is this the correct colour?
      stateview->SetItemBackgroundColour(i,col);
    }
  }

  stateview->SetColumnWidth(1,stateview->GetClientSize().GetWidth() - stateview->GetColumnWidth(0));

  tooltip_string.Prepend(wxT("Current state:\n"));
  stateview->SetToolTip(tooltip_string);
}

static void sort_transitions(wxArrayString& actions, wxArrayString& statechanges, wxArrayInt& indices)
{
  size_t len = indices.GetCount();
  size_t end = len;

  for (size_t i=0; i<end; i++)
  {
    if (actions[i] == wxT("tau"))
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
    }
    else
    {
      size_t j = i;
      while ((j > 0) && ((actions[j] < actions[j-1]) || ((actions[j] == actions[j-1]) && (statechanges[j] < statechanges[j-1]))))
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
  for (size_t i=end+1; i<len; i++)
  {
    size_t j = i;
    while ((j > end) && ((actions[j] < actions[j-1]) || ((actions[j] == actions[j-1]) && (statechanges[j] < statechanges[j-1]))))
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

static ATermAppl ToStateVector(ATerm state, NextState* nstate)
{
  return (state != NULL)?nstate->makeStateVector(state):NULL;
}
void XSimMain::UpdateTransitions(ATermList nextstates)
{
  wxArrayString actions;
  wxArrayString statechanges;
  wxArrayInt indices;

  NextState* nextstate = simulator->GetNextState();

  wxArrayInt trace_next;
  ATermAppl trace_next_transition = simulator->GetNextTransitionFromTrace();
  ATermAppl trace_next_state = ToStateVector(simulator->GetNextStateFromTrace(),nextstate);

  transview->DeleteAllItems();
  long i = 0;
  for (ATermList l=nextstates; !ATisEmpty(l); l=ATgetNext(l), i++)
  {
    actions.Add(wxConvLocal.cMB2WX(PrintPart_CXX(ATgetFirst(ATLgetFirst(l)), ppDefault).c_str()));
    indices.Add(i);
    if ((trace_next_state != NULL) &&
        ATisEqual(ATgetFirst(ATLgetFirst(l)),trace_next_transition) &&
        (nextstate->parseStateVector(trace_next_state,ATgetFirst(ATgetNext(ATLgetFirst(l)))) != NULL))
    {
      trace_next.Add(1);
    }
    else
    {
      trace_next.Add(0);
    }
//    transview->SetItemData(i,i);
    stringstream ss;
    ATerm m = current_state;
    ATerm n = ATgetFirst(ATgetNext(ATLgetFirst(l)));
    ATermList o = state_varnames;
    bool comma = false;
    for (size_t i=0; i<ATgetLength(state_varnames); i++)
    {
      ATermAppl oldval = nextstate->getStateArgument(m,i);
      ATermAppl newval = nextstate->getStateArgument(n,i);

      if (!ATisEqual(oldval,newval) && (!mcrl2::data::is_variable(newval) || showdc->IsChecked()))
      {
        if (comma)
        {
          ss << ", ";
        }
        else
        {
          comma = true;
        }
        PrintPart_CXX(ss, ATgetFirst(o), ppDefault);
        ss << " := ";
        if (mcrl2::data::is_variable(newval))
        {
          ss << "_";
        }
        else
        {
          PrintPart_CXX(ss, (ATerm) newval, ppDefault);
        }
      }

      o = ATgetNext(o);
    }
//    transview->SetItem(i,1,s);
    statechanges.Add(wxConvLocal.cMB2WX(ss.str().c_str()));
  }

  sort_transitions(actions,statechanges,indices);
  ssize_t next = -1;
  for (long i=0; static_cast<size_t>(i)<indices.GetCount(); i++)
  {
    transview->InsertItem(i,actions[i]);
    transview->SetItem(i,1,statechanges[i]);
    transview->SetItemData(i,indices[i]);
    if ((next < 0) && (trace_next[indices[i]] == 1))
    {
      next = i;
    }
  }

  if (!ATisEmpty(nextstates))
  {
    if (next < 0)
    {
      transview->Select(0);
    }
    else
    {
      transview->Select(next);
    }
  }

  /* Adapt column width */
  transview->SetColumnWidth(1,wxLIST_AUTOSIZE);
  transview->SetColumnWidth(1,transview->GetClientSize().GetWidth() - transview->GetColumnWidth(0));
}
