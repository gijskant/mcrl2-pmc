//  Author(s): A.j. (Hannes) pretorius
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  LICENSE_1_0.txt)
//
/// \file ./diagram.cpp

// --- diagram.cpp --------------------------------------------------
// (c) 2007  -  A.J. Pretorius  -  Eindhoven University of Technology
// ---------------------------  *  ----------------------------------


#include "diagram.h"


// -- constructors and destructor -----------------------------------


// -----------------
Diagram::Diagram( 
    Mediator* m )
    : Colleague( m )
// -----------------
{
    showGrid = true;
    snapGrid = true;

    GRID_NUM_INTERV_HINT = 30;
    ANGL_NUM_INTERV_HINT = 24;
    SIZE_BORDER          = 15;
}


// ----------------
Diagram::~Diagram()
// ----------------
{
    // composition
    for ( size_t i = 0; i < shapes.size(); ++i )
    {
        delete shapes[i];
        shapes[i] = NULL;
    }
    shapes.clear();
}
	

// -- set functions -------------------------------------------------


// -------------------------------
void Diagram::addShape( Shape *s )
// -------------------------------
{
    shapes.push_back( s );
}


// ---------------------------------------------
void Diagram::moveShapeToFront( const size_t &idx )
// ---------------------------------------------
{
    if ( 0 <= idx && idx < shapes.size() )
    {
        Shape* tmp = shapes[idx];
        for ( size_t i = idx; i > 0; --i )
        {
            shapes[i] = shapes[i-1];
            shapes[i]->setIndex(i);
        }
        shapes[0] = tmp;
        shapes[0]->setIndex(0);
        tmp = NULL;
    }
}


// --------------------------------------------
void Diagram::moveShapeToBack( const size_t &idx )
// --------------------------------------------
{
    if ( 0 <= idx && idx < shapes.size() )
    {
        Shape* tmp = shapes[idx];
        for ( size_t i = idx; i < shapes.size()-1; ++i )
        {
            shapes[i] = shapes[i+1];
            shapes[i]->setIndex(i);
        }
        shapes[shapes.size()-1] = tmp;
        shapes[shapes.size()-1]->setIndex(shapes.size()-1);
        tmp = NULL;
    }
}


// ---------------------------------------------
void Diagram::moveShapeForward( const size_t &idx )
// ---------------------------------------------
{
    if ( 0 < idx && idx < shapes.size() )
    {
        Shape* tmp = shapes[idx];
        
        shapes[idx] = shapes[idx-1];
        shapes[idx]->setIndex(idx);
        
        shapes[idx-1] = tmp;
        shapes[idx-1]->setIndex(idx-1);
        
        tmp = NULL;
    }
}


// ----------------------------------------------
void Diagram::moveShapeBackward( const size_t &idx )
// ----------------------------------------------
{
    if ( 0 <= idx && idx < shapes.size()-1 )
    {
        Shape* tmp = shapes[idx];

        shapes[idx] = shapes[idx+1];
        shapes[idx]->setIndex(idx);
        
        shapes[idx+1] = tmp;
        shapes[idx+1]->setIndex(idx+1);
        
        tmp = NULL;
    }
}


// ------------------------------------------
void Diagram::setShowGrid( const bool &flag )
// ------------------------------------------
{
    showGrid = flag;
}


// ------------------------------------------
void Diagram::setSnapGrid( const bool &flag )
// ------------------------------------------
{
    snapGrid = flag;
}

    
// -- get functions -------------------------------------------------


// -------------------------
int Diagram::getSizeShapes()
// -------------------------
{
    return shapes.size();
}


// ---------------------------------------
Shape* Diagram::getShape( const size_t &idx )
// ---------------------------------------
{
    Shape* result = NULL;
    if ( 0 <= idx && idx < shapes.size() )
	result = shapes[idx];
    return result;
}


// ------------------------
bool Diagram::getSnapGrid()
// ------------------------
{
    return snapGrid;
}


// ------------------------------------------------
double Diagram::getGridInterval( GLCanvas* canvas )
// ------------------------------------------------
{
    double numIntervals = GRID_NUM_INTERV_HINT;
    double sizeInterval;
    double pix;
    
    // get pixel size
    pix = canvas->getPixelSize();
    
    sizeInterval = (2.0-(2.0*pix*SIZE_BORDER))/(double)numIntervals;
        
    return sizeInterval;
}


// -------------------------------
double Diagram::getAngleInterval()
// -------------------------------
{
    double numIntervals = ANGL_NUM_INTERV_HINT;
    double sizeInterval = 360.0/(double)numIntervals;
    
    return sizeInterval;
}


// -- clear functions -----------------------------------------------


// ----------------------------------------
void Diagram::deleteShape( const size_t &idx )
// ----------------------------------------
{
    if ( 0 <= idx && idx < shapes.size() )
    {
        // delete shape
        Shape* s = shapes[idx];
        shapes.erase( shapes.begin()+idx );
        delete s;
        s = NULL;
        // update indices
        for ( size_t i = idx; i < shapes.size(); ++i )
            shapes[i]->setIndex( i );
    }
}


// -- vis functions -------------------------------------------------


// ------------------------------------------------
void Diagram::visualize( 
    const bool &inSelectMode,
    GLCanvas* canvas )
// ------------------------------------------------
// ------------------------------------------------------------------
// Used by diagram editor.
// ------------------------------------------------------------------
{
    drawBorder( inSelectMode, canvas );
    if ( showGrid == true )
        drawGrid( inSelectMode, canvas );
    drawShapes( inSelectMode, canvas );
}


// ----------------------------------
void Diagram::visualize(
    const bool &inSelectMode,
    GLCanvas* canvas,
    const vector< Attribute* > attrs,
    const vector< double > attrValIdcs )
// ----------------------------------
// ------------------------------------------------------------------
// Used by visualizers.
// ------------------------------------------------------------------
{
    if ( inSelectMode == true )
    {
        drawBorderFlush( inSelectMode, canvas );
    }
    else
    {
        drawBorderFlush( inSelectMode, canvas );
        for ( size_t i = 0; i < shapes.size(); ++i )
            shapes[i]->visualize( canvas, attrs, attrValIdcs );
    }
}


// ----------------------------------
void Diagram::visualize(
    const bool &inSelectMode,
    GLCanvas* canvas,
    const vector< Attribute* > attrs,
    const vector< double > attrValIdcs,
    const double &pix )
// ----------------------------------
// ------------------------------------------------------------------
// Used by visualizers.
// ------------------------------------------------------------------
{
    if ( inSelectMode == true )
    {
        drawBorderFlush( inSelectMode, canvas );
    }
    else
    {
        drawBorderFlush( inSelectMode, canvas );
        for ( size_t i = 0; i < shapes.size(); ++i )
            shapes[i]->visualize( canvas, attrs, attrValIdcs, pix );
    }
}


// ----------------------------------
void Diagram::visualize(
    const bool &inSelectMode,
    GLCanvas* canvas,
    const double &opacity,
    const vector< Attribute* > attrs,
    const vector< double > attrValIdcs )
// ----------------------------------
// ------------------------------------------------------------------
// Used by visualizers.
// ------------------------------------------------------------------
{
    if ( inSelectMode == true )
    {
        drawBorderFlush( inSelectMode, canvas );
    }
    else
    {
        drawBorderFlush( inSelectMode, canvas, opacity );
        for ( size_t i = 0; i < shapes.size(); ++i )
            shapes[i]->visualize( canvas, opacity, attrs, attrValIdcs );
    }
}


// -- private utility functions -------------------------------------


// -----------------------------
void Diagram::initGridSettings()
// -----------------------------
{
    showGrid   = true;
    snapGrid   = false;
}


// --------------------------
void Diagram::drawAxes( 
    const bool &inSelectMode,
    GLCanvas* canvas )
// --------------------------
{
    if ( inSelectMode != true )
    {
        double pix = canvas->getPixelSize();
        
        VisUtils::setColorMdGray();
        VisUtils::drawLine(  
            0.0,                 0.0,
            1.0-pix*SIZE_BORDER, -1+pix*SIZE_BORDER );
        VisUtils::drawLine( 
            -1.0+pix*SIZE_BORDER, 1-pix*SIZE_BORDER, 
            0.0,                  0.0 );    
    }
}


// --------------------------
void Diagram::drawBorder( 
    const bool &inSelectMode,
    GLCanvas* canvas )
// --------------------------
{
    if ( inSelectMode == true )
    {
        double pix;
        double xLft, xRgt, yTop, yBot;

        // get pixel size
        pix = canvas->getPixelSize();
    
        // calc margins
        xLft = -1.0 + pix*SIZE_BORDER;
        xRgt =  1.0 - pix*SIZE_BORDER;
        yTop =  1.0 - pix*SIZE_BORDER;
        yBot = -1.0 + pix*SIZE_BORDER;

        // draw
        VisUtils::fillRect( xLft, xRgt, yTop, yBot );
    }
    else
    {
        double pix;
        double xLft, xRgt, yTop, yBot;

        // get pixel size
        pix = canvas->getPixelSize();
    
        // calc margins
        xLft = -1.0 + pix*SIZE_BORDER;
        xRgt =  1.0 - pix*SIZE_BORDER;
        yTop =  1.0 - pix*SIZE_BORDER;
        yBot = -1.0 + pix*SIZE_BORDER;

        // draw
        VisUtils::setColorWhite();
        VisUtils::fillRect( xLft, xRgt, yTop, yBot );

        VisUtils::setColorMdGray();
        VisUtils::drawRect( xLft, xRgt, yTop, yBot );
    }
}


// ---------------------------
void Diagram::drawBorderFlush( 
    const bool &inSelectMode,
    GLCanvas* canvas )
// ---------------------------
{
    if ( inSelectMode == true )
        VisUtils::fillRect( -1.0, 1.0, 1.0, -1.0 );
    else
    {
        VisUtils::setColorWhite();
        VisUtils::fillRect( -1.0, 1.0, 1.0, -1.0 );

        VisUtils::setColorMdGray();
        VisUtils::drawRect( -1.0, 1.0, 1.0, -1.0 );
    }

}


// ---------------------------
void Diagram::drawBorderFlush( 
    const bool &inSelectMode,
    GLCanvas* canvas,
    const double &opacity )
// ---------------------------
{
    if ( inSelectMode == true )
        VisUtils::fillRect( -1.0, 1.0, 1.0, -1.0 );
    else
    {
        ColorRGB col;

        VisUtils::enableBlending();

        VisUtils::mapColorWhite( col );
        col.a = opacity;
        VisUtils::setColor( col );
//        VisUtils::fillRect( -1.0, 1.0, 1.0, -1.0 );

        VisUtils::mapColorMdGray( col );
        col.a = opacity;
        VisUtils::setColor( col );
        VisUtils::drawRect( -1.0, 1.0, 1.0, -1.0 );

        VisUtils::disableBlending();
    }

}


// --------------------------
void Diagram::drawGrid( 
    const bool &inSelectMode,
    GLCanvas* canvas )
// --------------------------
{
    if ( inSelectMode != true )
    {
        double numIntervals = GRID_NUM_INTERV_HINT;
        double sizeInterval;
        double pix;
        double xLft, xRgt, yTop, yBot;

        // get pixel size
        pix = canvas->getPixelSize();
    
        // calc margins
        xLft = -1.0 + pix*SIZE_BORDER;
        xRgt =  1.0 - pix*SIZE_BORDER;
        yTop =  1.0 - pix*SIZE_BORDER;
        yBot = -1.0 + pix*SIZE_BORDER;
        
        sizeInterval = (2.0-(2.0*pix*SIZE_BORDER)-2.0*pix)/(double)numIntervals;
        
        VisUtils::setColorLtGray();
        // draw inside out
        for ( int i = 0; i < numIntervals/2; ++i )
        {
            // vertical
            VisUtils::drawLine( 
                -i*sizeInterval,          -i*sizeInterval, 
                 1.0-pix*SIZE_BORDER-pix, -1.0+pix*SIZE_BORDER+pix );
            VisUtils::drawLine(  
                 i*sizeInterval,           i*sizeInterval, 
                 1.0-pix*SIZE_BORDER-pix, -1.0+pix*SIZE_BORDER+pix );
            // horizontal
            VisUtils::drawLine( 
                -1.0+pix*SIZE_BORDER+pix,  1.0-pix*SIZE_BORDER-pix,  
                 i*sizeInterval,           i*sizeInterval );
            VisUtils::drawLine( 
                -1.0+pix*SIZE_BORDER+pix,  1.0-pix*SIZE_BORDER-pix, 
                -i*sizeInterval,          -i*sizeInterval );
        }
    }
}


// --------------------------
void Diagram::drawShapes( 
    const bool &inSelectMode,
    GLCanvas* canvas )
// --------------------------
{
    
    if ( inSelectMode == true )
    {
        for ( size_t i = 0; i < shapes.size(); ++i )
        {
            glPushName( i );	
            shapes[i]->visualize( inSelectMode, canvas );
            glPopName();
        }
    }
    else
    {
        for ( size_t i = 0; i < shapes.size(); ++i )
        {	
            shapes[i]->visualize( inSelectMode, canvas );
        }
    }
   
}


// -- end -----------------------------------------------------------
