#include "springlayout.h"
#include <wx/sizer.h>
#include <wx/slider.h>
#include "ids.h"

using namespace IDS;

BEGIN_EVENT_TABLE(SpringLayout, wxEvtHandler)
  EVT_BUTTON(myID_START_OPTI, SpringLayout::onStart)
  EVT_BUTTON(myID_STOP_OPTI, SpringLayout::onStop)

  EVT_COMMAND_SCROLL(myID_NS_SLIDER, SpringLayout::onNodeStrength)
  EVT_COMMAND_SCROLL(myID_ES_SLIDER, SpringLayout::onEdgeStrength)
  EVT_COMMAND_SCROLL(myID_NL_SLIDER, SpringLayout::onTransLength)

END_EVENT_TABLE()


SpringLayout::SpringLayout(GLTSGraph* owner)
{
  app = owner;
  edgeStiffness = 1;
  nodeStrength = 100000;
  naturalLength = 20;
  stopOpti = true;
  stopped = true;
}

void SpringLayout::setupPane(wxPanel* pane)
{ 
  int lflags = wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL;
  int rflags = wxALIGN_RIGHT | wxALIGN_CENTER_VERTICAL | wxEXPAND | wxALL;

  wxFlexGridSizer* sizer = new wxFlexGridSizer(0, 1, 0, 0);

  wxSlider* sliderNodeStrength = new wxSlider(
                                        pane, myID_NS_SLIDER, 
                                        nodeStrength,10000, 1000000, 
                                        wxDefaultPosition, wxDefaultSize, 
                                        wxSL_HORIZONTAL | wxSL_LABELS);
  wxSlider* sliderEdgeStiffness = new wxSlider(
                                        pane, myID_ES_SLIDER,
                                        edgeStiffness, 0, 15, 
                                        wxDefaultPosition, wxDefaultSize,
                                        wxSL_HORIZONTAL | wxSL_LABELS);
  wxSlider* sliderNaturalLength = new wxSlider(
                                        pane, myID_NL_SLIDER,
                                        naturalLength, 1, 500, 
                                        wxDefaultPosition, wxDefaultSize,
                                        wxSL_HORIZONTAL|wxSL_LABELS);

  wxButton* optimizeBtn = new wxButton(pane, myID_START_OPTI, wxT("Start"));
  wxButton* stopBtn = new wxButton(pane, myID_STOP_OPTI, wxT("Stop"));
  
  sizer->Add( 
    new wxStaticText(pane, wxID_ANY, wxT("State repulsion")), 
    0, lflags, 4);
  sizer->Add(sliderNodeStrength, 0, rflags, 3);

  sizer->Add(
    new wxStaticText(pane, wxID_ANY, wxT("Transition attracting force")),
    0, lflags, 4);
  sizer->Add(sliderEdgeStiffness, 0, rflags, 3);

  sizer->Add(
    new wxStaticText(pane, wxID_ANY, wxT("Natural transition length")),
    0, lflags, 4);
  sizer->Add(sliderNaturalLength, 0, rflags, 3);
  
  wxFlexGridSizer* btnSizer = new wxFlexGridSizer(0, 2, 0, 0);
  btnSizer->Add(optimizeBtn);
  btnSizer->Add(stopBtn);


  sizer->Add(btnSizer);

  pane->SetSizer(sizer);
  pane->Fit();
  pane->Layout();

}




void SpringLayout::onStart(wxCommandEvent& /* event */)
{
  Graph* g = app->getGraph();
  
  stopOpti = false;
  stopped = false;

  while(!stopOpti)
  {
    layoutGraph(g);

    if(app)
    {
      app->display(); 
    }

    wxYield();
  }
  
  stopped = true;
}


void SpringLayout::onStop(wxCommandEvent& /* event */)
{
  stopOpti = true;
}

void SpringLayout::layoutGraph(Graph* graph)
{
  size_t nrStates = graph->getNumberOfStates();
  std::vector<float> sumFX;
  std::vector<float> sumFY;

  double windowWidth = 2000;
  double windowHeight = 2000;

  for(size_t i = 0; i < nrStates; ++i)
  {
    sumFX.push_back(0.0f);
    sumFY.push_back(0.0f);

    State* s1 = graph->getState(i);
    
    // Calculate forces
    double x1 = s1->getX();
    double y1 = s1->getY();

    for (size_t j = 0; j < nrStates; ++j)
    {
      if (i != j)
      {
        State* s2 = graph->getState(j);

        double x2 = s2->getX();
        double y2 = s2->getY();
        double xDiff = x1 - x2;
        double yDiff = y1 - y2;

        // Euclidean distance
        double distance = sqrt (xDiff * xDiff + yDiff * yDiff);

        if (distance > 1)
        {
          double s = nodeStrength / (distance * distance * distance);

          sumFX[i] += s * xDiff;
          sumFY[i] += s * yDiff;
        }
        else
        {
          if (i > j)
          {
            sumFX[i] += 5;
            sumFY[i] += 5;
          }
          else
          {
            sumFX[i] += -5;
            sumFX[i] += -5;
          }
        }
      }
    }
    
    sumFX[i] += -2 * x1 / windowWidth;
    sumFY[i] += -2 * y1 / windowHeight;  
  
    for(size_t j = 0; j < s1->getNumberOfTransitions(); ++j)
    {
      State* s2 = s1->getTransition(j)->getTo();
      double x1 = s1->getX();
      double x2 = s2->getX();
      double y1 = s1->getY();
      double y2 = s2->getY();

      double xDiff = x2 - x1;
      double yDiff = y2 - y1;
      double distance = sqrt(xDiff * xDiff + yDiff * yDiff);

      if (distance > 0.1)
      {
        double s = (edgeStiffness * log (distance / naturalLength)) / distance;

        sumFX[i] += s * xDiff;
        sumFY[i] += s * yDiff;
      }
    }

    for(size_t j = 0; j < s1->getNumberOfInTransitions(); ++j)
    {
      State* s2 = s1->getInTransition(j)->getFrom();
      double x1 = s2->getX();
      double x2 = s1->getX();
      double y1 = s2->getY();
      double y2 = s1->getY();

      double xDiff = x2 - x1;
      double yDiff = y2 - y1;
      double distance = sqrt(xDiff * xDiff + yDiff * yDiff);

      if (distance > 0.1)
      {
        double s = (edgeStiffness * log(distance / naturalLength)) / distance;
        sumFX[i] -= s * xDiff;
        sumFY[i] -= s * yDiff;
      }
    }
  }

  for(size_t i = 0; i < graph->getNumberOfStates(); ++i)
  {
    double newX = 0;
    double newY = 0;
    State* s = graph->getState(i);
    if (!s->isLocked())
    {
      newX = s->getX() + sumFX[i];
      newY = s->getY() + sumFY[i];
      
      if(newX > 1000)
      {
        newX = 1000;
      }

      if (newX < -1000)
      {
        newX = -1000;
      }
      
      if (newY > 1000)
      {
        newY = 1000;
      }

      if (newY < -1000)
      {
        newY = -1000;
      }
     
      s->setX(newX);
      s->setY(newY);
    }
  }
  
}

void SpringLayout::onNodeStrength(wxScrollEvent& evt)
{
  nodeStrength = evt.GetPosition();
}

void SpringLayout::onEdgeStrength(wxScrollEvent& evt)
{
  edgeStiffness = evt.GetPosition();
}

void SpringLayout::onTransLength(wxScrollEvent& evt)
{
  naturalLength = evt.GetPosition();
}

void SpringLayout::stop()
{
  stopOpti = true;
}
