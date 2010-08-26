
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file
/// \brief Contains the LTSView application class.

#ifndef LTSVIEW_H
#define LTSVIEW_H

#include <wx/wx.h>
#include <wx/app.h>
#include <string>

#include "mcrl2/utilities/input_tool.h"
#include "mcrl2/utilities/wx_tool.h"
#include "mcrl2/utilities/mcrl2_gui_tool.h"

#include "enums.h"
#include "mediator.h"

class GLCanvas;
class LTS;
class MainFrame;
class MarkManager;
class RGB_Color;
class Settings;
class Visualizer;

class LTSView :
  public mcrl2::utilities::wx::tool< LTSView, mcrl2::utilities::tools::input_tool >,
  public Mediator
{
    typedef mcrl2::utilities::wx::tool< LTSView,
       mcrl2::utilities::tools::input_tool > super;

  private:
    LTS *lts;
    unsigned int colourCounter;
    GLCanvas *glCanvas;
    MainFrame *mainFrame;
    Settings *settings;
    Visualizer *visualizer;
    MarkManager *markManager;
    RankStyle rankStyle;
    bool fsmStyle;
    void applyMarkStyle();

  public:

    LTSView();
    ~LTSView();
    virtual bool run();
    bool parse_command_line(int argc, wxChar** argv);
    void activateMarkRule(int index,bool activate);
    void addMarkRule();
    void editMarkRule(int index);
    void exportToText(std::string filename);
    MarkStyle getMarkStyle();
    MatchStyle getMatchStyle();
    bool isMarked(State *s);
    bool isMarked(Cluster *s);
    bool isMarked(Transition *t);
    RGB_Color getMarkRuleColor(int mr);
    RGB_Color getNewRuleColour();
    std::string getVersionString();
    void notifyRenderingFinished();
    void notifyRenderingStarted();
    void openFile(std::string fileName);
    void removeMarkRule(int mr);
    void setActionMark(int l,bool b);
    void setMarkStyle(MarkStyle ms);
    void setMatchStyle(MatchStyle ms);
    void setMatchStyleClusters(MatchStyle ms);
    void setRankStyle(RankStyle rs);
    void setVisStyle(VisStyle vs);
    void setFSMStyle(bool b);
    void startSim();
    int getNumberOfParams() const;
    std::string getActionLabel(const int i) const;
    std::string getParName(const int i) const;
    std::string getParValue(State *s, const int j) const;
    void selectStateByID(const int id);
    void selectCluster(const int rank, const int pos);
    void deselect();
    int getNumberOfObjects();

    void zoomInBelow();
    void zoomInAbove();
    void zoomOutTillTop();
    void zoomOut();

    void reportError(std::string const& error);

    void loadTrace(std::string const& path);
    void generateBackTrace();

    void printHelp(std::string const& name);
    void printVersion();

    DECLARE_EVENT_TABLE()
};

  class LTSView_gui_tool: public mcrl2::utilities::mcrl2_gui_tool<LTSView>
  {
    public:
  	LTSView_gui_tool()
      {
        //m_gui_options["no-state"] = create_checkbox_widget();
      }
  };

DECLARE_APP(LTSView_gui_tool)

#endif
