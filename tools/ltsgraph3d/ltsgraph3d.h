// Author(s): Carst Tankink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ltsgraph3d.h
/// \brief Main application class.

#ifndef LTSGRAPH3D_H
#define LTSGRAPH3D_H

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
#include "mcrl2/utilities/squadt_tool.h"

#ifndef __glu_h__
#ifdef __APPLE__
  #include <OpenGL/glu.h>
#else
  #include <GL/glu.h>
#endif
#endif

class LTSGraph3d : public mcrl2::utilities::wx::tool< LTSGraph3d,
   mcrl2::utilities::tools::squadt_tool< mcrl2::utilities::tools::input_tool > >
{
    typedef mcrl2::utilities::wx::tool< LTSGraph3d,
       mcrl2::utilities::tools::squadt_tool< mcrl2::utilities::tools::input_tool > > super;

  private:
    Graph *graph; // The labeled transition system (graph) that we work on

    GLCanvas *glCanvas;  // GLcanvas for visualisation
    Visualizer *visualizer; // Visualizer of objects
    MainFrame *mainFrame;  // Mainframe/central GUI.
    std::vector<LayoutAlgorithm*> algorithms; // The layout algorithms loaded.

    State* selectedState;
    Transition* selectedTransition;
    Transition* selectedLabel; // Label of transition selected should be moved

    std::string fileName;
    bool colouring;
    wxColour brushColour;

  public:
    LTSGraph3d();

    bool run();

    void openFile(std::string const &path);
    void display();

#ifdef ENABLE_SQUADT_CONNECTIVITY
    void set_capabilities(tipi::tool::capabilities&) const;
    void user_interactive_configuration(tipi::configuration&);
    bool check_configuration(tipi::configuration const&) const;
    bool perform_task(tipi::configuration&);
#endif

	void moveObject(double diffX, double diffY, double diffZ);
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
	void getCanvasMdlvwMtrx(float * mtrx);
	void getCanvasCamPos(double & x, double & y, double & z);
};


#endif //LTSGRAPH3D_H
