#include "glcanvas.h"
#include <wx/dcclient.h>
#include <iostream>

#include "ids.h"

#ifdef __APPLE__
  #include <OpenGL/glu.h>
#else
  #include <GL/glu.h>
#endif

BEGIN_EVENT_TABLE(GLCanvas, wxGLCanvas)
  EVT_PAINT(GLCanvas::onPaint)
  EVT_SIZE(GLCanvas::onSize)
  EVT_ERASE_BACKGROUND(GLCanvas::onEraseBackground)

  EVT_ENTER_WINDOW(GLCanvas::onMouseEnter)
  EVT_LEFT_DOWN(GLCanvas::onMouseLftDown)
  EVT_RIGHT_DOWN(GLCanvas::onMouseRgtDown)
  EVT_LEFT_UP(GLCanvas::onMouseLftUp)
  EVT_RIGHT_UP(GLCanvas::onMouseRgtUp)
  EVT_MOTION(GLCanvas::onMouseMove)
END_EVENT_TABLE()

GLCanvas::GLCanvas(GLTSGraph* app, wxWindow* parent,
                   const wxSize &size, int* attribList)
  : wxGLCanvas(parent, wxID_ANY, wxDefaultPosition, size, wxSUNKEN_BORDER,
               wxEmptyString, attribList)
{
  owner = app;
  displayAllowed = false;

  scaleFactor = 1.0;
}

GLCanvas::~GLCanvas()
{
  owner->getAlgorithm(0)->stop();

}

void GLCanvas::initialize()
{
  SetCurrent();
  
  glLoadIdentity();

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  SwapBuffers();
  displayAllowed = true;
  visualizer->initFontRenderer();
}

void GLCanvas::setVisualizer(Visualizer *vis)
{
  visualizer = vis;
}

void GLCanvas::display()
{
  if (GetContext())
  {
    int width, height;
    
    GetClientSize( &width, &height);
    // This is current context
    SetCurrent();
   
    // Set up viewing volume
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    // Cast to GLdouble for smooth transitions
    GLdouble aspect = (GLdouble)width / (GLdouble)height;

    if (aspect > 1)
    {
      // width > height
      gluOrtho2D(aspect*(-1), aspect, -1.0, 1.0);
    }
    else
    {
      // height >= width
      gluOrtho2D(-1.0, 1.0, (1/aspect)*(-1), (1/aspect));
    }
    
    glMatrixMode( GL_MODELVIEW);
    glLoadIdentity();
    
    glViewport(0, 0, width, height);
    
    double wwidth, wheight;

    getSize(wwidth, wheight);
    double pS = getPixelSize();
  
    if (visualizer)
    {
      // Draw 
      visualizer->visualize(wwidth, wheight, pS, false);
    }
    
    glFinish();
    SwapBuffers();

    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
      std::cerr << "OpenGL error: " << gluErrorString(error) << std::endl;
    }
  }
}

void GLCanvas::onPaint(wxPaintEvent& /*event*/)
{
  wxPaintDC dc(this);
  display();
}

void GLCanvas::onSize(wxSizeEvent& /*event*/)
{
  reshape();
}

void GLCanvas::reshape()
{
  if(GetContext())
  {
    int width, height;
    SetCurrent();
    
    GetClientSize(&width, &height);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0,0, width, height);
  }
}

void GLCanvas::onEraseBackground(wxEraseEvent& /*event*/)
{
}


void GLCanvas::getSize(
  double &width,
  double &height)
/* (Based on method of the same name in Diagraphica's GLcanvas, the following
 * description derrives from that tool)
 * Returns viewport width and height in world coordinates.
 *
 * Before scaling, the viewport is set up such that the shortest side has length
 * 2 in world coordinates. Let ratio = width/height be the aspect ration of the 
 * window and keep in mind that:
 *   
 *  Size(viewport)   = Size(world)*scaleFactor
 *  So, Size(world)  = Size(viewport)/ScaleFactor
 *  So, Size(world)  = 2/ScaleFactor
 * There are 2 cases:
 * (1) If aspect > 1, the viewport is wider than tall
 *     So, the starting height was 2: 
 *     world width      = ( aspect*2 ) / scaleFactor
 *     world height     = 2 / scaleFactor
 * (2) If aspect <= 1, the viewport is taller than wide
 *     So, the starting width was 2:
 *     world width      = 2 / scaleFactor;
 *     world height     = ( aspect*2 ) / scaleFactor 
 */
{
  int widthViewPort;
  int heightViewPort;
  double aspect;

  // get pixel (device) sizes
  GetClientSize(
    &widthViewPort,
    &heightViewPort);

  aspect = (double) widthViewPort /  (double) heightViewPort;

  if (aspect > 1)
  {
    // width > height, so starting height was 2
    width = (aspect * 2.0) / (double) scaleFactor;
    height = 2.0 / (double) scaleFactor;
  }
  else
  {
    // height >= width, so starting width was 2
    width = 2.0 / (double) scaleFactor;
    height = ((1/aspect) * 2.0) / (double) scaleFactor;
  }
}

double GLCanvas::getPixelSize()
// TODO: Comment
{
  double result = 0.0;

  int widthPixels;
  int heightPixels;
  GetClientSize(
    &widthPixels,
    &heightPixels);

  double widthWorld;
  double heightWorld;

  getSize(widthWorld, heightWorld);

  result = widthWorld * (1 / static_cast<double>(widthPixels));
  return result;
}

void GLCanvas::onMouseEnter(wxMouseEvent& /* event */)
{
  this->SetFocus();
}


void GLCanvas::onMouseLftDown(wxMouseEvent& event)
{
  oldX = event.GetX();
  oldY = event.GetY();
  pickObjects(oldX, oldY);
  owner->dragObject();
  display();
}

void GLCanvas::onMouseLftUp(wxMouseEvent& /* evt */)
{
  owner->stopDrag();
  display();
}

void GLCanvas::onMouseRgtDown(wxMouseEvent& event)
{
  oldX = event.GetX();
  oldY = event.GetY();

  pickObjects(oldX, oldY);
  owner->lockObject(); 
  display();
}

void GLCanvas::onMouseRgtUp(wxMouseEvent& /*evt */)
{
}

void GLCanvas::onMouseMove(wxMouseEvent& event)
{
  if(event.Dragging() && event.LeftIsDown())
  {
    int width, height;
    GetClientSize(&width, &height);

    int newX = static_cast<int>(event.GetX());
    int newY = static_cast<int>(event.GetY());

    double diffX = static_cast<double>(newX - oldX) / static_cast<double>(width) * 2000;
    double diffY = static_cast<double>(oldY - newY) / static_cast<double>(height) * 2000;
    
    oldX = newX;
    oldY = newY;

    owner->moveObject(diffX, diffY);
    display();
  }
}


void GLCanvas::pickObjects(int x, int y)
{
  owner->deselect(); 


  if(GetContext()) 
  {
    GLuint selectBuf[512];
    GLint  hits = 0;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glSelectBuffer(512, selectBuf);
    // Swith to selection mode
    (void) glRenderMode(GL_SELECT);

    glInitNames();

    // Create new projection transformation
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    // Create picking region near cursor location
    gluPickMatrix((GLdouble) x, 
                  (GLdouble)  viewport[3] - y, 
                  5.0,
                  5.0, 
                  viewport);
    
    // Get current size of canvas
    int width,height;
    GetClientSize(&width,&height);

    GLdouble aspect = (GLdouble)width / (GLdouble)height;

    if (aspect > 1)
    {
      // width > height
      gluOrtho2D(aspect * (-1), aspect, -1, 1);
    }
    else
    {
      // height >= width
      gluOrtho2D(-1, 1, -1/aspect, (1/aspect));
                               // calculate rotations etc.
    }

    glMatrixMode( GL_MODELVIEW);
    double wwidth, wheight;
    getSize(wwidth, wheight);
    double pS = getPixelSize();
    
    visualizer->visualize(wwidth, wheight, pS, true);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glFlush();

    hits = glRenderMode(GL_RENDER);
    
    processHits(hits, selectBuf);
    reshape();
    display();
  }
}

void GLCanvas::processHits(const GLint hits, GLuint *buffer)
{
  // This method selects the object clicked.
  //
  // The buffer content per hit is encoded as follows:
  //  buffer[0]: The number of names on the name stack at the moment of the hit
  //  buffer[1]: The minimal depth of the object hit
  //  buffer[2]: The maximal depth of the object. We are certainly not 
  //             interested in this.
  //  buffer[3]: The first identifier of the object picked.

  GLuint *ptr;

  int number = 0;
  int selectedType = -1;
  size_t selectedObject;

  ptr = (GLuint*) buffer;

  if (hits > 0)
  {
    while(number == 0)
    {

      number = *ptr;
      ++ptr; // number;
      ++ptr; // z1
      ++ptr; // z2
      // There are no names on the stack, so skip over them.
    }
    // Number != 0 => *ptr is the first name on the stack.
    selectedType = *ptr;

    ++ptr; // ID;
    selectedObject = *ptr;

    
    switch(selectedType)
    {
      case IDS::TRANSITION: 
      { 
        ++ptr; // Second ID, for transitions
        size_t selectedTrans = *ptr;
        owner->selectTransition(selectedObject, selectedTrans); 
        break;
      }
      case IDS::SELF_LOOP:
      {
        ++ptr; // Second ID, for transitions
        size_t selectedTrans = *ptr;
        owner->selectSelfLoop(selectedObject, selectedTrans);
        break;
      }
      case IDS::STATE:
      {
        owner->selectState(selectedObject);
        break;
      }
      default: break;
    }
  }
  else
  {
  }

  ptr = NULL; 
}
