// --- colorchooser.h -----------------------------------------------
// (c) 2006  -  A.J. Pretorius  -  Eindhoven University of Technology
// ---------------------------  *  ----------------------------------

#ifndef COLORCHOOSER_H
#define COLORCHOOSER_H

#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <vector>
using namespace std;
#include "graph.h"
#include "shape.h"
#include "utils.h"
#include "visualizer.h"

class ColorChooser : public Visualizer
{
public:
    // -- constructors and destructor -------------------------------
    ColorChooser(
        Mediator* m,
        Graph* g,
        GLCanvas* c );
    virtual ~ColorChooser();

    // -- set functions ---------------------------------------------
    void setActive( const bool &flag );
    void setPoints(
        const vector< double > &hue,
        const vector< double > &y );
    
    // -- visualization functions  ----------------------------------
    void visualize( const bool &inSelectMode );
    
    // -- event handlers --------------------------------------------
    void handleMouseLftDownEvent( 
        const int &x, 
        const int &y );
    void handleMouseLftUpEvent( 
        const int &x, 
        const int &y );
    void handleMouseRgtDownEvent( 
        const int &x, 
        const int &y );
    void handleMouseRgtUpEvent( 
        const int &x, 
        const int &y );
	void handleMouseMotionEvent( 
        const int &x, 
        const int &y );

    // -- public constants ------------------------------------------
    static double hdlSzeHnt;

protected:
    // -- utility drawing functions ---------------------------------
    // ***
    //void clear();
    void drawColorSpectrum();
    void drawPath( const bool &inSelectMode );
    void drawPoints( const bool &inSelectMode );
    
    // -- utility event handlers ------------------------------------
    void handleHits( const vector< int > &ids );
    void handleDrag();
   
    // -- hit detection ---------------------------------------------
    void processHits( 
        GLint hits, 
        GLuint buffer[] );
    
    // -- data members ----------------------------------------------
    bool active;
    
    vector< double > positionsX;
    vector< double > positionsY;
    int dragIdx;

    // vis settings
    double hdlSize;
    /*
    vector< Position2D > positions;
    */
};

#endif

// -- end -----------------------------------------------------------
