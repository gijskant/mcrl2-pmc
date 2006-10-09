#ifndef GLCANVAS_H
#define GLCANVAS_H

#include <wx/wx.h>
#include <wx/glcanvas.h>
#include <wx/image.h>
#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif
#include <cmath>
#include "mediator.h"
#include "utils.h"
#include "ids.h"

//using namespace IDs;
//using namespace Utils;

class GLCanvas: public wxGLCanvas
{
  public:
    GLCanvas( Mediator* owner, wxWindow* parent, const wxSize &size =
	wxDefaultSize, int* attribList = NULL );
    ~GLCanvas();

    void      display();
    void      enableDisplay();
    void      disableDisplay();
    Utils::RGB_Color getBackgroundColor() const;
    Utils::RGB_Color getDefaultBackgroundColor() const { return defaultBGColor; }
    void      getMaxViewportDims(int *w,int *h);
    unsigned char* getPictureData(int res_x,int res_y);
    void      initialize();
    void      onMouseDown( wxMouseEvent& event );
    void      onMouseEnter( wxMouseEvent& event );
    void      onMouseMove( wxMouseEvent& event);
    void      onMouseUp( wxMouseEvent& event );
    void      onMouseWheel( wxMouseEvent& event );
    void      onPaint( wxPaintEvent& event );
    void      onSize( wxSizeEvent& event );
    void      OnEraseBackground( wxEraseEvent& event );
    void      resetView();
    void      reshape();
    void      setActiveTool( int t );
    void      setBackgroundColor( Utils::RGB_Color c ) { glClearColor(c.r,c.g,c.b,1); }
    void      setDefaultPosition( float structWidth, float structHeight );

  private:
    int	      activeTool;
    float     angleX;
    float     angleY;
    int	      currentTool;
    Utils::RGB_Color defaultBGColor;
    float     startPosZ;
    float     startPosZDefault;
    bool      displayAllowed;
    float     farPlane;
    float     farPlaneDefault;
    Mediator* mediator;
    Utils::Point3D   moveVector;
    float     nearPlane;
    int	      oldMouseX;
    int	      oldMouseY;
    
    void      determineCurrentTool( wxMouseEvent& event );
    void      setMouseCursor();

    DECLARE_EVENT_TABLE()
};

#endif
