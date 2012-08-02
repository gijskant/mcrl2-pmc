// Author(s): A.J. (Hannes) Pretorius
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ./glcanvas.h

#ifndef GLCANVAS_H
#define GLCANVAS_H

#include <QtCore>
#include <QtGui>

#include <cstddef>
#include <cmath>
#include <string>

#ifdef __APPLE__
# include <GLUT/glut.h>
#else
# ifdef WIN32
#  define NOMINMAX
#  include <windows.h>
# endif
# include <GL/glu.h>
#endif
#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/tooltip.h>
#include "colleague.h"
#include "mediator.h"
#include "visutils.h"

class GLCanvas : public wxGLCanvas, Colleague
{
  public:
    // -- constructors and desctructor ------------------------------
    GLCanvas(
      Mediator* m,
      wxWindow* parent,
      wxWindowID id);
    virtual ~GLCanvas();

    // -- set functions ---------------------------------------------
    void setScaleFactor(const double& f);
    void setXTranslation(const double& x);
    void setYTranslation(const double& y);
    void setTranslation(
      const double& x,
      const double& y);

    void enableMouseMotion();

    // -- get functions ---------------------------------------------
    double getScaleFactor();
    double getXTranslation();
    double getYTranslation();
    void getTranslation(
      double& x,
      double& y);

    void disableMouseMotion();

    double getWidth();
    double getHeight();
    void getSize(
      double& width,
      double& height);

    double getPixelSize();
    void getWorldCoords(
      const double& deviceX,
      const double& deviceY,
      double& worldX,
      double& worldY);

    // -- drawing and interaction functions -------------------------
    void clear();
    void display();

    void showToolTip(const std::string& msg);
    void clearToolTip();

    void onEvtPaint(wxPaintEvent& event);
    void onEvtSize(wxSizeEvent& event);
    void OnEraseBackground(wxEraseEvent& event);

    void onMouseEvent(wxMouseEvent& event);
    void onWheelEvent(wxMouseEvent& event);

    void onEnterMouse(wxMouseEvent& /*event*/);
    void onLeaveMouse(wxMouseEvent& /*event*/);

    void onKeyEvent(wxKeyEvent& event, bool down);

    void onKeyDown(wxKeyEvent& event);
    void onKeyUp(wxKeyEvent& event);

  private:
    // -- data members ----------------------------------------------
    double scaleFactor;
    double xTranslation;
    double yTranslation;

    bool handleMouseMotion;

    wxString tooltip;

    // -- declare event table ---------------------------------------
    DECLARE_EVENT_TABLE()
};

#endif

// -- end -----------------------------------------------------------
