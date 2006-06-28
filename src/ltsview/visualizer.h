#ifndef VISUALIZER_H
#define VISUALIZER_H

#include <vector>
#include <math.h>
#include "mediator.h"
#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif
#include "utils.h"
#include "lts.h"

using namespace std;
using namespace Utils;

struct Primitive
{
  GLuint  displayList;
  float   distance;
  Point3D worldCoordinate;
};

// class for primitive comparison based on distance
class Distance_desc
{
  public:
    bool operator()( const Primitive*, const Primitive* ) const;
};

class Visualizer
{
  private:
    float		  boundingCylH;
    float		  boundingCylW;
    float		  clusterHeight;
    float		  cos_ibt;
    float		  cos_obt;
    vector< float >	  cos_tab;
    static VisSettings	  defaultVisSettings;
    HSV_Color		  delta_col;
    bool		  displayStates;
    bool		  displayWireframe;
    LTS*		  lts;
    MarkStyle		  markStyle;
    Mediator*		  mediator;
    vector< Primitive* >  primitives;
    RankStyle		  rankStyle;
    bool		  refreshPrimitives;
    bool		  refreshStates;
    float		  sin_ibt;
    float		  sin_obt;
    vector< float >	  sin_tab;
    GLuint		  statesDisplayList;
    Point3D		  viewpoint;
    VisSettings		  visSettings;

    void computeSubtreeBounds( Cluster* root, float &boundWidth, float
	&boundHeight );
    void drawCylinder( float baserad, float toprad, float height, RGB_Color
	basecol, RGB_Color topcol, bool baseclosed, bool topclosed );
    void drawCylinderInterpolate( float baserad, float toprad, float height,
	RGB_Color basecol, RGB_Color topcol );
    void drawCylinderSplit( float baserad, float toprad, float height, RGB_Color
	basecol, RGB_Color topcol );
    void drawHemisphere( float rad );
    void drawSphere( float rad, int n = 0 );
    void drawStates( Cluster* root, int rot );
    void drawStatesMarkStates( Cluster* root, int rot );
    void drawStatesMarkDeadlocks( Cluster* root, int rot );
    void drawSubtree( Cluster* root, HSV_Color col, int rot );
    void drawSubtreeMarkDeadlocks( Cluster* root, int rot );
    void drawSubtreeMarkStates( Cluster* root, int rot );
    void drawSubtreeMarkTransitions( Cluster* root, int rot );
    void drawSubtreeOld( Cluster* root, bool topClosed, HSV_Color col, int rot );
    void drawSubtreeOldMarkDeadlocks( Cluster* root, bool topClosed, int rot );
    void drawSubtreeOldMarkStates( Cluster* root, bool topClosed, int rot );
    void drawSubtreeOldMarkTransitions( Cluster* root, bool topClosed, int rot );
    void drawTubeInterpolate( float baserad, float toprad, RGB_Color basecol,
	RGB_Color topcol, Point3D b1, Point3D b2, Point3D b3, Point3D &center );
    void drawTubeSplit( float baserad, float toprad, RGB_Color basecol,
	RGB_Color topcol, Point3D b1, Point3D b2, Point3D b3, Point3D &center );
    void myRotatef( float theta, float ax, float ay, float az, float M[] );
    void myTranslatef( float tx, float ty, float tz, float M[] );
    void updateGeometryTables();
  
  public:
    Visualizer( Mediator* owner );
    ~Visualizer();
    
    void	computeBoundsInfo();
    void	computeClusterHeight();
    void	drawLTS( Point3D viewpoint );
    float	getBoundingCylinderHeight() const;
    float	getBoundingCylinderWidth() const;
    float	getHalfStructureHeight() const;
    RankStyle	getRankStyle() const;
    VisSettings getDefaultVisSettings() const;
    VisSettings getVisSettings() const;
    void	setLTS( LTS* l );
    void	setMarkStyle( MarkStyle ms );
    void	setRankStyle( RankStyle rs );
    bool	setVisSettings( VisSettings vs );
    void	toggleDisplayStates();
    void	toggleDisplayWireframe();
};
#endif
