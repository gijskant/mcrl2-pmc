#include "node.h"

Node::Node(unsigned int _num, double _posX, double _posY, wxString _lbl, bool _initState) : 
             posX(_posX), posY(_posY), lbl(_lbl), num(_num), initState(_initState)
{
  locked = false;
}

void Node::OnPaint(wxPaintDC * ptrDC) 
{
  wxColour color;
  if (!locked)
    color = wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE);
  else 
    color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);

    //Circle
		if (initState) {
			wxPen pen = wxPen(wxT("RED"), 2, wxSOLID);
			ptrDC->SetPen(pen);
		}
		else {
			wxPen pen = wxPen(wxT("BLACK"), 1, wxSOLID);
			ptrDC->SetPen(pen);
		}
    wxBrush myBrush(color,wxSOLID );
    ptrDC->SetBrush(myBrush);
    ptrDC->DrawCircle((wxCoord)posX,(wxCoord)posY,radius);

    //Label
		if (labelsVisible) {
			wxFont MyFont(FONT_SIZE, wxSWISS , wxNORMAL, wxNORMAL, false, wxT("Arial"));
			ptrDC->SetFont(MyFont);
	
			double x= posX-POS_NODE_LBL_X;;
			//Adjust label x position according to the length
			switch (lbl.Length()) {
				case 1:  x = posX-POS_NODE_LBL_X;  break;
				case 2:  x = posX-POS_NODE_LBL_X-3;break;
				case 3:  x = posX-POS_NODE_LBL_X-6;break;
				default: x = posX-POS_NODE_LBL_X;  break;
			}
	
			double y=posY-POS_NODE_LBL_Y;
	
			ptrDC->DrawRotatedText(lbl,(int) round(x),(int) round(y),0.0);
		}
}

unsigned int Node::Get_num() {
    return num;
}

double Node::GetX() {
    return posX;
}

double Node::GetY() {
    return posY;
}

bool Node::IsLocked(){
  return locked;
}

void Node::Lock() {
  locked = true;
}

void Node::Unlock() {
  locked = false;
}

void Node::ShowLabels() {
		labelsVisible = true;
} 

void Node::HideLabels() {
		labelsVisible = false;
} 

void Node::SetXY(double _x, double _y) {
  if (!locked) {
    posX = _x;
    posY = _y;
  }
  else 
    wxBell();
}

void Node::forceSetXY(double _x, double _y) {
    posX = _x;
    posY = _y;
}

void Node::SetRadius(int newRadius) {
	radius = newRadius;
}

