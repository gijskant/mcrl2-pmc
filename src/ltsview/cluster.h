#ifndef CLUSTER_H
#define CLUSTER_H
#include <vector>
#include <algorithm>
#include <math.h>
#include "utils.h"

#ifndef STATE_H
  #include "state.h"
#else
  class State;
#endif

using namespace Utils;
using namespace std;

// forward declaration
class Cluster;

// class for cluster comparison based on cluster sizes
class Comp_ClusterSize
{
  public:
    bool operator()( const Cluster*, const Cluster* ) const;
};

class Cluster
{
  
  private:
    Cluster*		ancestor;
    float		baseRadius;
    vector< Cluster* >	descendants;
    float		position;
    float		size;
    vector< State* >	states;
    float		topRadius;
  
  public:
    Cluster();
    ~Cluster();
    void      addDescendant( Cluster* c );
    void      addState( State* s );
    void      computeSizeAndDescendantPositions();
    Cluster*  getAncestor() const;
    float     getBaseRadius() const;
    void      getDescendants( vector< Cluster* > &cs ) const;
    float     getPosition() const;
    float     getTopRadius() const;
    float     getSize() const;
    void      getStates( vector< State* > &ss ) const;
    bool      hasDescendants() const;
    void      setAncestor( Cluster* c );
    void      setPosition( float p );
};

#endif
