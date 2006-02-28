#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "mediator.h"
#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
    #include <GLUT/glut.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
#endif
#include "utils.h"
#include "glutils.h"
#include "lts.h"

using namespace Utils;
using namespace GLUtils;

class Visualizer
{
  private:
    float		  cos_obt;
    static VisSettings	  defaultVisSettings;
    LTS*		  lts;
    MarkStyle		  markStyle;
    Mediator*		  mediator;
    vector< Primitive* >  primitives;
    RankStyle		  rankStyle;
    bool		  refreshPrimitives;
    float		  sin_obt;
    float		  structHeight;
    float		  structWidth;
    VisSettings		  visSettings;

    void drawSubtree( Cluster* root, bool topClosed, float &boundWidth, float
	&boundHeight, HSV_Color col, HSV_Color delta_col );
    void drawSubtreeMarkStates( Cluster* root, bool topClosed, float
	&boundWidth, float &boundHeight );
    void drawSubtreeMarkDeadlocks( Cluster* root, bool topClosed, float
	&boundWidth, float &boundHeight );
  
  public:
    Visualizer( Mediator* owner );
    ~Visualizer();
    
    void	drawLTS();
    float	getStructureHeight() const;
    float	getStructureWidth() const;
    RankStyle	getRankStyle() const;
    VisSettings getDefaultVisSettings() const;
    VisSettings getVisSettings() const;
    void	positionClusters();
    void	setLTS( LTS* l );
    void	setMarkStyle( MarkStyle ms );
    void	setRankStyle( RankStyle rs );
    void	setVisSettings( VisSettings vs );
};
#endif
