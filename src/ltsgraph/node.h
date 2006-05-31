#ifndef node_h
#define node_h

#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "wx/wxprec.h"
#include "wx/dataobj.h"
#include <string>

#define POS_NODE_LBL_X 3.0
#define POS_NODE_LBL_Y 5.0
#define FONT_SIZE 9
#define CIRCLE_RADIUS 10


using namespace std;

class Node
{
public:
  Node(unsigned int _num, double _posX, double _posY, wxString _lbl, bool _initState);
  void OnPaint(wxPaintDC * ptrDC);
  unsigned int Get_num();
  double GetX(); 
  double GetY();
  void SetXY(double, double);
	void forceSetXY(double, double);//set the x and y even if node is locked
  bool IsLocked();
  void Lock();
  void Unlock();
	void ShowLabels();
	void HideLabels();
	void SetRadius(int);

private:
	int radius;
  bool locked;
	bool labelsVisible;
  double posX;
  double posY;
  wxString lbl;
  unsigned int num;
	bool initState;
    

};



#endif //node_h
