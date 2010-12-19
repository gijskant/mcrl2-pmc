// Author(s): Carst Tankink and Ali Deniz Aladagli
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mainframe.h
/// \brief Main window declaration

#ifndef MAINFRAME_H
#define MAINFRAME_H

#include <wx/wx.h>
#include <wx/frame.h>

#ifndef LTSGRAPH_H
  #include "ltsgraph.h"
#else
  class LTSGraph; // Forward declaration
#endif

#ifndef GLCANVAS_H
  #include "glcanvas.h"
#else
  class GLCanvas;
#endif

#ifndef ALGO_DIALOG_H
  #include "algodialog.h"
#else
  class AlgoDialog;
#endif

#include "infodialog.h"

class MainFrame : public wxFrame
{
  public:
    MainFrame(LTSGraph *owner);
		~MainFrame();

    GLCanvas* getGLCanvas();

    void setLTSInfo(size_t is, size_t ns, size_t nt, size_t nl);

  private:
    LTSGraph *app;
    GLCanvas *glCanvas;
    AlgoDialog *algoDlg;
    InfoDialog *infoDlg;

    void setupMenuBar();
    void setupMainArea();


    void onOpen(wxCommandEvent& event);
    void onQuit(wxCommandEvent& event);
    void onClose(wxCloseEvent& event);
    void onAlgo(wxCommandEvent& event);
    void onInfo(wxCommandEvent& event);
    void onTogglePositioning(wxCommandEvent& event);
    void onToggleVector(wxCommandEvent& event);
	void onToggle3D(wxCommandEvent& event);
    void onExport(wxCommandEvent& event);
    void onImport(wxCommandEvent& event);
    void onSelect(wxCommandEvent& event);
    void onColour(wxCommandEvent& event);
	void onResetAll(wxCommandEvent& event);
	void onResetRot(wxCommandEvent& event);
	void onResetPan(wxCommandEvent& event);
	void onMode(wxCommandEvent& event);
	void onShowSystem(wxCommandEvent& event);

  DECLARE_EVENT_TABLE()
};

#endif // MAINFRAME_H
