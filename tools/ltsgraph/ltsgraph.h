// Author(s): Carst Tankink and Ali Deniz Aladagli
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ltsgraph.h
/// \brief Main application class.

#ifndef LTSGRAPH_H
#define LTSGRAPH_H

#ifndef MAINFRAME_H
#include "mainframe.h"
#else
//Forward declaration
class MainFrame;
#endif

#include "graph.h"
#include "glcanvas.h"
#include "visualizer.h"
#include "layoutalgo.h"

#include <wx/app.h>

#include "mcrl2/utilities/wx_tool.h"
#include "mcrl2/utilities/input_tool.h"
#include "mcrl2/utilities/mcrl2_gui_tool.h"

#ifndef __glu_h__
#ifdef __APPLE__
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif
#endif

class LTSGraph : public mcrl2::utilities::wx::tool< LTSGraph,
  mcrl2::utilities::tools::input_tool >
{
    typedef mcrl2::utilities::wx::tool< LTSGraph,
            mcrl2::utilities::tools::input_tool > super;

  private:
    Graph* graph; // The labeled transition system (graph) that we work on

    GLCanvas* glCanvas;  // GLcanvas for visualisation
    Visualizer* visualizer; // Visualizer of objects
    MainFrame* mainFrame;  // Mainframe/central GUI.
    std::vector<LayoutAlgorithm*> algorithms; // The layout algorithms loaded.

    State* selectedState;
    Transition* selectedTransition;
    Transition* selectedLabel; // Label of transition selected should be moved

    std::string fileName;
    bool colouring;
    wxColour brushColour;

  public:
    LTSGraph();

    bool run();

    void openFile(std::string const& path);
    void display();

    void moveObject(double invect[4]);
    void moveObject(double x, double y);
    void toggleVectorSelected();
    void lockObject();
    void dragObject();
    void stopDrag();
    void deselect();
    void selectState(size_t state);
    void colourState(size_t state);
    void uncolourState(size_t state);
    void selectTransition(size_t state, size_t trans);
    void selectSelfLoop(size_t state, size_t trans);
    void selectLabel(size_t state, size_t trans);
    void selectSelfLabel(size_t state, size_t trans);

    // Setters
    void setLTSInfo(int is, int ns, int nt, int nl);
    void setRadius(int radius);
    void setTransLabels(bool value);
    void setStateLabels(bool value);
    void setCurves(bool value);
    void setBrushColour(wxColour colour);
    void setTool(bool isColour);

    // Getters
    Graph* getGraph();
    size_t getNumberOfAlgorithms() const;
    LayoutAlgorithm* getAlgorithm(size_t i) const;
    size_t getNumberOfObjects();
    std::string getFileName() const;
    int getRadius() const;
    double getAspectRatio() const;
    void getCanvasMdlvwMtrx(double* mtrx);
    void getCanvasCamPos(double& x, double& y, double& z);
    void forceWalls();
    bool get3dMode();
};

class LTSGraph_gui_tool: public mcrl2::utilities::mcrl2_gui_tool<LTSGraph>
{
  public:
    LTSGraph_gui_tool()
    {
      //m_gui_options["no-state"] = create_checkbox_widget();
    }
};

#endif //LTSGRAPH_H
