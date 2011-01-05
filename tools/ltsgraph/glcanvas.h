// Author(s): Carst Tankink and Ali Deniz Aladagli
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file glcanvas.h
/// \brief OpenGL rendering canvas

#ifndef GLCANVAS_H
#define GLCANVAS_H

#include <wx/glcanvas.h>
#include "glcontext.h"

#ifndef LTSGRAPH3D_H
  #include "ltsgraph.h"
#else
  class LTSGraph;
#endif

#ifndef VISUALIZER_H
  #include "visualizer.h"
#else
  class Visualizer;
#endif

class GLCanvas : public wxGLCanvas
{
  public:
    GLCanvas(
          LTSGraph* app,
          wxWindow* parent,
          const wxSize &size = wxDefaultSize,
          int* attribList = NULL);

    ~GLCanvas();

    void display();
    void setVisualizer(Visualizer *vis);

    void onPaint(wxPaintEvent& event);
    void onSize(wxSizeEvent& event);

    // Mouse event handlers
    void onMouseEnter(wxMouseEvent& event);
    void onMouseLeave(wxFocusEvent& event);
    void onMouseLftDown(wxMouseEvent& event);
    void onMouseRgtDown(wxMouseEvent& event);
    void onMouseMove(wxMouseEvent& event);
    void onMouseLftUp(wxMouseEvent& event);
    void onMouseRgtUp(wxMouseEvent& event);
    void onMouseWhl(wxMouseEvent& event);
    void onMouseDblClck(wxMouseEvent& event);
	void onMouseMidUp(wxMouseEvent& event);
	void onMouseMidDown(wxMouseEvent& event);

    void getSize(double & width, double & height, double & depth);
	void ResetAll();
	void ResetPan();
	void ResetRot();
	void setMode(int tool);
	void changeDrawMode();
	void showSystem();
	void recalcPixelSize();
	void recalcAspectRatio();
	double getPixelSize();
    double getAspectRatio();
	double getMaxDepth() const;
	void getMdlvwMtrx(double * mtrx);
	void getCamPos(double & x, double & y, double & z);
	bool get3D();
	void render2D();
	void render3D();

  private:
    LTSGraph* owner;
    Visualizer* visualizer;
    bool displayAllowed;
	bool panning;
	bool dispSystem;
	bool usingTool;
    double scaleFactor, maxDepth;
    int oldX, oldY;
	float lookX, lookY, lookZ, rotX, rotY;
	double currentModelviewMatrix[16];
	int currentTool;
	bool calcRot;
	bool drawIn3D;
	double pixelSize;
	double aspectRatio;

	void normalizeMatrix();
    bool pickObjects3d(int x, int y, wxMouseEvent const&);
	void pickObjects(int x, int y, wxMouseEvent const&);
    void processHits(const GLint hits, GLuint * buffer, wxMouseEvent const&);
	void setMouseCursor(int theTool);

	GLContext* getGLContext( wxGLCanvas *canvas );
	GLContext* glContext;


  DECLARE_EVENT_TABLE()
};

#endif //GLCANVAS_H
