// Author(s): Bas Ploeger and Carst Tankink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file
/// \brief Contains implementation of the LTSView application.

#define NAME "ltsview"
#define AUTHOR "Bas Ploeger and Carst Tankink"

#include <string>
#include <wx/wx.h>
#include "mcrl2/utilities/command_line_interface.h"

std::string get_about_message() {
  static const std::string version_information =
        mcrl2::utilities::interface_description("", NAME, AUTHOR, "", "").
                                                        version_information() +
    std::string("\n"
     "Tool for interactive visualisation of state transition systems.\n"
     "Developed by Bas Ploeger and Carst Tankink.\n"
     "\n"
     "LTSView is based on visualisation techniques by Frank van Ham and Jack van Wijk. "
     "See: F. van Ham, H. van de Wetering and J.J. van Wijk, "
     "\"Visualization of State Transition Graphs\". "
     "Proceedings of the IEEE Symposium on Information Visualization 2001. IEEE CS Press, pp. 59-66, 2001.\n"
     "\n"
     "The default colour scheme for state marking was obtained through http://www.colorbrewer.org\n"
     "\n"
     "This tool is part of the mCRL2 toolset.\n"
     "For information see http://www.mcrl2.org\n"
     "\n"
     "For feature requests or bug reports,\n"
     "please visit http://www.mcrl2.org/issuetracker");

  return version_information;
}

std::string lts_file_argument;

#ifdef ENABLE_SQUADT_CONNECTIVITY
//SQuADT protocol interface
#include <mcrl2/utilities/mcrl2_squadt_interface.h>

using namespace mcrl2::utilities::squadt;

const char* fsm_file_for_input = "fsm_in";

class squadt_interactor: public mcrl2::utilities::squadt::mcrl2_wx_tool_interface {

  public:

    // Configures tool capabilities.
    void set_capabilities(tipi::tool::capabilities& c) const {
      c.add_input_configuration(fsm_file_for_input,tipi::mime_type("fsm",
            tipi::mime_type::text),tipi::tool::category::visualisation);
    }

    // Queries the user via SQuADt if needed to obtain configuration information
    void user_interactive_configuration(tipi::configuration&) {
    }

    bool check_configuration(tipi::configuration const& c) const {
      bool valid = c.input_exists(fsm_file_for_input);

      if (!valid) {
        send_error("Invalid input combination!");
      }

      return valid;
    }

    bool perform_task(tipi::configuration& c) {
      lts_file_argument = c.get_input(fsm_file_for_input).get_location();

      return mcrl2_wx_tool_interface::perform_task(c);
    }
};
#endif

#include <wx/filename.h>
#include <wx/image.h>
#include "mcrl2/utilities/aterm_ext.h"
#include "ltsview.h"
#include "markstateruledialog.h"
#include "fileloader.h"
#include "settings.h"

using namespace std;
using namespace Utils;
IMPLEMENT_APP_NO_MAIN(LTSView)

BEGIN_EVENT_TABLE(LTSView, wxApp)
END_EVENT_TABLE()

void parse_command_line(int argc, wxChar** argv) {

  using namespace mcrl2::utilities;

  interface_description clinterface(
        std::string(wxString(argv[0], wxConvLocal).fn_str()),
        NAME, AUTHOR, "[OPTION]... [INFILE]\n",
    "Start the LTSView application and open INFILE. If INFILE is not "
    "supplied then LTSView is started without opening an LTS.\n"
    "\n"
    "INFILE should be in the FSM format.");

  command_line_parser parser(clinterface, argc, argv);

  if (parser.arguments.size() > 1)
  {
    parser.error("too many file arguments");
  }
  else if (parser.arguments.size() == 1)
  {
    lts_file_argument = parser.arguments[0];
  }
}

std::string parse_error;

bool LTSView::OnInit()
{
  lts = NULL;
  rankStyle = ITERATIVE;
  fsmStyle = false;
  colourCounter = 0;
  settings = new Settings();
  mainFrame = new MainFrame(this,settings);
  visualizer = new Visualizer(this,settings);
  markManager = new MarkManager();
  glCanvas = mainFrame->getGLCanvas();
  glCanvas->setVisualizer(visualizer);
  
  SetTopWindow(mainFrame);
  mainFrame->Show(true);
  glCanvas->initialize();
  mainFrame->Layout();

  wxInitAllImageHandlers();

  if ( !parse_error.empty() )
  {
    wxMessageDialog msg_dlg(mainFrame, wxString(parse_error.c_str(), wxConvLocal),
        wxT("Command line error"), wxOK | wxICON_ERROR);
    msg_dlg.ShowModal();
  }
  else if (!lts_file_argument.empty())
  {
    wxFileName fileName(wxString(lts_file_argument.c_str(), wxConvLocal));
    fileName.Normalize();
    mainFrame->setFileInfo(fileName);
    openFile(static_cast< string >(fileName.GetFullPath().fn_str()));
  }
  return true;
}

bool LTSView::Initialize(int& argc, wxChar** argv) {
  try {
    parse_command_line(argc, argv);
  }
  catch (std::exception& e) {
    if (wxApp::Initialize(argc, argv)) {
      parse_error = std::string(e.what()).
        append("\n\nNote that other command line options may have been ignored because of this error.");
    }
    else {
      std::cerr << e.what() << std::endl;

      return false;
    }

    return true;
  }

  return wxApp::Initialize(argc, argv);
}


#ifdef __WINDOWS__
extern "C" int WINAPI WinMain(HINSTANCE hInstance,                    
                                  HINSTANCE hPrevInstance,                
                                  wxCmdLineArgType lpCmdLine,             
                                  int nCmdShow) {                                                                     

  MCRL2_ATERM_INIT(0, lpCmdLine) 

#ifdef ENABLE_SQUADT_CONNECTIVITY
  if(interactor< squadt_interactor >::free_activation(hInstance, hPrevInstance, lpCmdLine, nCmdShow)) {
    return EXIT_SUCCESS;
  }
#endif

  return wxEntry(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
#else
int main(int argc, char **argv) {

  MCRL2_ATERM_INIT(argc, argv)

#ifdef ENABLE_SQUADT_CONNECTIVITY
  if(interactor< squadt_interactor >::free_activation(argc, argv)) {
    return EXIT_SUCCESS;
  }
#endif

  return wxEntry(argc, argv);
}
#endif

int LTSView::OnExit() {
  if (lts != NULL) delete lts;
  delete settings;
  delete visualizer;
  delete markManager;
  return 0;
}

std::string LTSView::getVersionString() {
  return mcrl2::utilities::interface_description("", NAME, AUTHOR, "", "").
                                                        version_information();
}

void LTSView::openFile(string fileName) {
  glCanvas->disableDisplay();

  mainFrame->createProgressDialog("Opening file","Parsing file");
  mainFrame->updateProgressDialog(0,"Parsing file");
  LTS* newlts = new LTS(this);
  try {
    ltsview::FileLoader floader = ltsview::FileLoader(newlts);
    if (!floader.parse_file(fileName))
    {
      throw "Parsing failed.";
    }
  }
  catch (string msg) {
    delete newlts;
    mainFrame->updateProgressDialog(100,"Error loading file");
    mainFrame->showMessage("Error loading file",msg);
    return;
  }
  if (lts != NULL) delete lts;
  lts = newlts;
  // first remove all unreachable states
  lts->trim();
  
  mainFrame->updateProgressDialog(17,"Ranking states");
  lts->rankStates(rankStyle);
  
  mainFrame->updateProgressDialog(33,"Clustering states");
  lts->clusterStates(rankStyle);

  mainFrame->updateProgressDialog(50,"Setting cluster info");
  lts->computeClusterInfo();
  
  mainFrame->updateProgressDialog(67,"Positioning clusters");
  lts->positionClusters(fsmStyle);

  markManager->setLTS(lts,true);
  visualizer->setLTS(lts,true);
  
  mainFrame->updateProgressDialog(83,"Positioning states");
  lts->positionStates();
  
  mainFrame->updateProgressDialog(100,"Done");
  visualizer->notifyMarkStyleChanged();

  glCanvas->enableDisplay();
  glCanvas->resetView();
  
  mainFrame->loadTitle();
  mainFrame->setNumberInfo(lts->getNumStates(),
      lts->getNumTransitions(),lts->getNumClusters(),
      lts->getNumRanks());
  mainFrame->resetParameters();
  for (unsigned int i = 0; i < lts->getNumParameters(); ++i) {
    mainFrame->addParameter(i,lts->getParameterName(i));
  }
  mainFrame->resetMarkRules();
  
  vector< string > ls;
  lts->getActionLabels(ls);

  mainFrame->setSim(lts->getSimulation());
  mainFrame->setActionLabels(ls);

  glCanvas->setSim(lts->getSimulation());

  mainFrame->setMarkedStatesInfo(0);
  mainFrame->setMarkedTransitionsInfo(0);
}

void LTSView::setRankStyle(RankStyle rs) {
  if (rankStyle != rs) {
    rankStyle = rs;
    if (lts != NULL) {
      glCanvas->disableDisplay();

      mainFrame->createProgressDialog("Structuring LTS","Applying ranking");

      mainFrame->updateProgressDialog(17,"Ranking states");
      lts->rankStates(rankStyle);
      
      mainFrame->updateProgressDialog(33,"Clustering states");
      lts->clusterStates(rankStyle);

      mainFrame->updateProgressDialog(40,"Setting cluster info");
      lts->computeClusterInfo();
      markManager->markClusters();
      
      mainFrame->updateProgressDialog(60,"Positioning clusters");
      lts->positionClusters(fsmStyle);

      visualizer->setLTS(lts,true);

      mainFrame->updateProgressDialog(80,"Positioning states");
      lts->positionStates();

      mainFrame->updateProgressDialog(100,"Done");

      glCanvas->enableDisplay();
      glCanvas->resetView();

      mainFrame->setNumberInfo(lts->getNumStates(),
        lts->getNumTransitions(),lts->getNumClusters(),
        lts->getNumRanks());
    }
  }
}

void LTSView::setVisStyle(VisStyle vs) {
  if (visualizer->getVisStyle() != vs) {
    visualizer->setVisStyle(vs);
    glCanvas->display();
  }
}

void LTSView::setFSMStyle(bool b) {
  if (b != fsmStyle) {
    fsmStyle = b;
    if (lts != NULL) {
      glCanvas->disableDisplay();
      lts->positionClusters(fsmStyle);
      visualizer->setLTS(lts,true);
      glCanvas->enableDisplay();
      glCanvas->resetView();
    }
  }
}

void LTSView::addMarkRule() {
  if (lts != NULL) {
    MarkStateRuleDialog* msrdlg = new MarkStateRuleDialog(mainFrame,this,lts);
    msrdlg->CentreOnParent();
    if (msrdlg->ShowModal() == wxID_OK) {
      if (msrdlg->getParamIndex() != -1) {
        vector<bool> vals;
        msrdlg->getValues(vals);
        int mr = markManager->createMarkRule(msrdlg->getParamIndex(),
            msrdlg->getNegated(),msrdlg->getColor(),vals);
        mainFrame->addMarkRule(msrdlg->getMarkRuleString(),mr);

        if (markManager->getMarkStyle() != MARK_STATES) {
          setMarkStyle(MARK_STATES);
        } else {
          applyMarkStyle();
        }
      }
    }
    msrdlg->Close();
    msrdlg->Destroy();
  }
}

void LTSView::removeMarkRule(int mr) {
  markManager->removeMarkRule(mr);
  if (markManager->getMarkStyle() != MARK_STATES) {
    setMarkStyle(MARK_STATES);
  } else {
    applyMarkStyle();
  }
}

void LTSView::editMarkRule(int mr) {
  if (lts != NULL) {
    MarkStateRuleDialog* msrdlg = new MarkStateRuleDialog(mainFrame,this,lts);
    vector< bool > vals;
    markManager->getMarkRuleValues(mr,vals);
    msrdlg->setData(markManager->getMarkRuleParam(mr),
        markManager->getMarkRuleColor(mr),markManager->getMarkRuleNegated(mr),
        vals);
    msrdlg->CentreOnParent();
    if (msrdlg->ShowModal() == wxID_OK) {
      if (msrdlg->getParamIndex() != -1) {
        msrdlg->getValues(vals);
        markManager->setMarkRuleData(mr,msrdlg->getParamIndex(),
            msrdlg->getNegated(),msrdlg->getColor(),vals);
        mainFrame->replaceMarkRule(msrdlg->getMarkRuleString(),mr);
        if (markManager->getMarkStyle() != MARK_STATES) {
          setMarkStyle(MARK_STATES);
        } else {
          applyMarkStyle();
        }
      }
    }
    msrdlg->Close();
    msrdlg->Destroy();
  }
}

Utils::MarkStyle LTSView::getMarkStyle() {
  return markManager->getMarkStyle();
}

Utils::MatchStyle LTSView::getMatchStyle() {
  return markManager->getMatchStyle();
}

bool LTSView::isMarked(State* s) {
  return markManager->isMarked(s);
}

bool LTSView::isMarked(Cluster* c) {
  return markManager->isMarked(c);
}

bool LTSView::isMarked(Transition* t) {
  return markManager->isMarked(t);
}

Utils::RGB_Color LTSView::getMarkRuleColor(int mr) {
  return markManager->getMarkRuleColor(mr);
}

Utils::RGB_Color LTSView::getNewRuleColour()
{
  // TODO implement
  colourCounter = (colourCounter + 1) % 9;

  RGB_Color result;

  switch(colourCounter)
  {
    case 0: 
      { result.r = 228;
        result.g = 26;
        result.b = 28;
        break;
      }
    case 1:
      { result.r = 55;
        result.g = 126;
        result.b = 184;
        break;
      }
    case 2:
      { result.r = 77;
        result.g = 175;
        result.b = 74;
        break;
      }
    case 3:
      { result.r = 152;
        result.g = 78;
        result.b = 163;
        break;
      }
    case 4:
      { result.r = 255;
        result.g = 127;
        result.b = 0;
        break;
      }
    case 5:
      { result.r = 255;
        result.g = 255;
        result.b = 51;
        break;
      }
    case 6:
      { result.r = 166;
        result.g = 86;
        result.b = 40;
        break;
      }
    case 7:
     { result.r = 247;
       result.g = 129;
       result.b = 191;
       break;
     }
    case 8:
     { result.r = 153;
       result.g = 153;
       result.b = 153;
       break;
     }
    default: //does not occur for modulo, but keep compiler happy
    {  result.r = 0;
       result.g = 0;
       result.b = 0;
    }
  }
  return result;
}

void LTSView::activateMarkRule(int mr,bool activate) {
  if (lts != NULL) {
    markManager->setMarkRuleActivated(mr,activate);
    if (markManager->getMarkStyle() != MARK_STATES) {
      setMarkStyle(MARK_STATES);
    } else {
      applyMarkStyle();
    }
  }
}

void LTSView::setMatchStyle(Utils::MatchStyle ms) {
  if (lts == NULL) return;
  markManager->setMatchStyle(ms);
  if (markManager->getMarkStyle() != MARK_STATES) {
    setMarkStyle(MARK_STATES);
  } else {
    applyMarkStyle();
  }
}

void LTSView::setActionMark(string label,bool b) {
  markManager->setActionMark(label,b);
  setMarkStyle(MARK_TRANSITIONS);
}

void LTSView::setMarkStyle(Utils::MarkStyle ms) {
  markManager->setMarkStyle(ms);
  applyMarkStyle();
}

void LTSView::applyMarkStyle() {
  if (lts == NULL) return;

  switch (markManager->getMarkStyle()) {
    case MARK_DEADLOCKS:
      mainFrame->setMarkedStatesInfo(lts->getNumDeadlocks());
      mainFrame->setMarkedTransitionsInfo(0);
      break;
    case MARK_STATES:
      mainFrame->setMarkedStatesInfo(markManager->getNumMarkedStates());
      mainFrame->setMarkedTransitionsInfo(0);
      break;
    case MARK_TRANSITIONS:
      mainFrame->setMarkedStatesInfo(0);
      mainFrame->setMarkedTransitionsInfo(
          markManager->getNumMarkedTransitions());
      break;
    case NO_MARKS:
    default:
      mainFrame->setMarkedStatesInfo(0);
      mainFrame->setMarkedTransitionsInfo(0);
      break;
  }
  visualizer->notifyMarkStyleChanged();
  glCanvas->display();
}

void LTSView::notifyRenderingStarted() {
  mainFrame->startRendering();
}

void LTSView::notifyRenderingFinished() {
  mainFrame->stopRendering();
}

void LTSView::startSim() {
  Simulation* sim = lts->getSimulation();

  sim->start();
  
  // Select/deselect initial state of simulation, to initialize state info pane.
  selectStateByID(sim->getCurrState()->getID());
}

int LTSView::getNumberOfParams() const {
  if (lts != NULL)
  {
    return lts->getNumParameters();
  }
  else
  {
    return 0;
  }
}
string LTSView::getActionLabel(const int i) const {
  if (lts != NULL)
  {
    return lts->getLabel(i);
  }
  else 
  {
    return "";
  }
}

string LTSView::getParName(const int i) const {
  if (lts != NULL)
  {
    return lts->getParameterName(i);
  }
  else 
  {
    return "";
  }
}

string LTSView::getParValue(const int i, const int j) const {
  if (lts != NULL)
  {
    return lts->getParameterValue(i, j);
  }
  else
  {
    return "";
  }
}


void LTSView::selectStateByID(const int id) {
  if (lts != NULL) {
    State* s = lts->selectStateByID(id);
    lts->getSimulation()->setInitialState(s);
    for (unsigned int i = 0; i < lts->getNumParameters(); ++i) {
      mainFrame->setParameterValue(i,lts->getParameterValue(i,
            s->getParameterValue(i)));
    }
  }
}

void LTSView::selectCluster(const int rank, const int pos) 
{
  if (lts != NULL) 
  {
    Cluster* c = lts->selectCluster(rank, pos);
    mainFrame->setClusterStateNr(c->getNumStates());
    for (unsigned int i = 0; i < lts->getNumParameters(); ++i)
    {
      std::vector<int> vs; 
      std::vector<std::string> val;

      c->getParameterValues(i, vs);

      for(size_t j = 0; j < vs.size(); ++j)
      {
        val.push_back(lts->getParameterValue(i, vs[j]));
      }
      mainFrame->setParameterValues(i, val);
    }

  }
}

void LTSView::deselect() {
  if (lts != NULL)
  {
    lts->deselect();

    mainFrame->resetParameterValues();
  }
}

int LTSView::getNumberOfObjects() {
  int result = 0;

  if (lts != NULL) {
    result += lts->getNumClusters();
    result += lts->getNumStates();
  }

  return result;
}

void LTSView::zoomInBelow()
{
  LTS* newLTS = lts->zoomIntoBelow();
  deselect();
  lts = newLTS;
  markManager->setLTS(lts,false);
  visualizer->setLTS(lts,false);
  mainFrame->setNumberInfo(lts->getNumStates(),
      lts->getNumTransitions(),lts->getNumClusters(),
      lts->getNumRanks());
  applyMarkStyle();
  glCanvas->setSim(lts->getSimulation());
  mainFrame->setSim(lts->getSimulation());
}

void LTSView::zoomInAbove()
{
  LTS* newLTS = lts->zoomIntoAbove();
  deselect();
  lts = newLTS;
  markManager->setLTS(lts,false);
  visualizer->setLTS(lts,false);
  mainFrame->setNumberInfo(lts->getNumStates(),
     lts->getNumTransitions(),lts->getNumClusters(),
     lts->getNumRanks()); 
  applyMarkStyle();
  glCanvas->setSim(lts->getSimulation());
  mainFrame->setSim(lts->getSimulation());
}

void LTSView::zoomOutTillTop()
{
  LTS* oldLTS = lts;
  do
  {
    zoomOut();
    oldLTS = lts;
  }  while (lts != oldLTS);
}

void LTSView::zoomOut()
{
  LTS* oldLTS = lts;
  lts = oldLTS->zoomOut();
  oldLTS->deselect();
  markManager->setLTS(lts,false);
  visualizer->setLTS(lts,false);
  mainFrame->setNumberInfo(lts->getNumStates(),
    lts->getNumTransitions(),lts->getNumClusters(),
    lts->getNumRanks()); 
  applyMarkStyle();
  glCanvas->setSim(lts->getSimulation());
  mainFrame->setSim(lts->getSimulation());

  if (oldLTS != lts) 
  {
    delete oldLTS;
  }
}

void LTSView::loadTrace(std::string const& path)
{
  lts->loadTrace(path);
  glCanvas->setSim(lts->getSimulation());
  mainFrame->setSim(lts->getSimulation());
}

void LTSView::reportError(std::string const& error) 
{
  mainFrame->reportError(error);
}


void LTSView::generateBackTrace()
{
  lts->generateBackTrace();
}

void LTSView::exportToText(std::string filename)
{
  visualizer->exportToText(filename);
}
