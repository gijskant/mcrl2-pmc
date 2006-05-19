#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include <wx/wxprec.h>
#include <wx/cmdline.h>

#include <vector>
#include <iostream>
#include <string>
#include "stdlib.h"
//random
#include <cstdlib>
#include <ctime> 
//LTS
#include "liblts.h"
#include "libstruct.h"

#include "node.h"
#include "edge.h"

#define INITIAL_WIN_HEIGHT 780
#define INITIAL_WIN_WIDTH  1024


using namespace std;
using namespace mcrl2::lts;

const int ID_OPTIMIZE      = 10000;
const int ID_STOP_OPTIMIZE = 10001;
const int ID_SET_EDGE_STIF = 10002;
const int ID_SET_NODE_STRE = 10003;
const int ID_SET_NATU_LENG = 10004;

class GraphFrame : public wxFrame
{
public:
  GraphFrame(const wxString& title, const wxPoint& pos, const wxSize& size, long style = wxDEFAULT_FRAME_STYLE);
  void Init(wxString LTSfile);
  void OnPaint(wxPaintEvent& evt);
  void OnOpen(wxCommandEvent& event);
  void OnQuit(wxCommandEvent& event);
  void OnSetEdgeStiffness( wxCommandEvent &event );
  void OnSetNodeStrength( wxCommandEvent &event );
  void OnSetNaturalLength( wxCommandEvent &event );
  void OnOptimize( wxCommandEvent &event );
  void OnStopOptimize( wxCommandEvent &event );
  void OnResize(wxSizeEvent& event);
  bool OptimizeDrawing(double precision);
  int  FindNode(wxPoint);
  void PressLeft(wxMouseEvent& event);
  void Drag(wxMouseEvent& event);
  void ReleaseLeft(wxMouseEvent& event);
  void PressRight(wxMouseEvent& event);//to fix a node
  void CreateMenu();


private:
  wxSize sz;
  bool StopOpti;
  
  double EdgeStiffness;   //stiffness of the edge
  double NodeStrength;   //Strength of the electrical repulsion
  double NaturalLength;

  wxMenuBar *menu;
  wxMenu *file;
  wxMenu *draw;
  wxMenu *options;
  wxMenuItem *openItem;
  wxMenuItem *quitItem;
  wxMenuItem *optimizeGraph;
  wxMenuItem *stopOptimize;
  wxMenuItem *setEdgeStiffnessItem;
  wxMenuItem *setNodeStrengthItem;
  wxMenuItem *setNaturalLengthItem;

DECLARE_EVENT_TABLE()
};
