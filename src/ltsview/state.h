#ifndef STATE_H
#define STATE_H

#include <vector>
#include <set>
#include "aterm2.h"
#include "liblowlevel.h"
#include "utils.h"

#ifndef TRANSITION_H
  #include "transition.h"
#else
  class Transition;
#endif

#ifndef CLUSTER_H
  #include "cluster.h"
#else
  class Cluster;
#endif

//using namespace std;

class State
{
  public:
    State( ATermList sv );
    ~State();
    void      addComrade( State* s );
    void      addInTransition( Transition* trans );
    void      addLoop( Transition* trans );
    void      addOutTransition( Transition* trans );
    void      addSubordinate( State* s );
    void      addSuperior( State* s );
    void      clearHierarchyInfo();
    //void      deselect();
    void      DFSfinish();
    void      DFSclear();
    void      DFSvisit();
    
    Utils::DFSState  getVisitState() const;
    Cluster*  getCluster() const;
    void      getComrades( std::set< State* > &ss ) const;
    void      getInTransitions( std::vector< Transition* > &ts ) const;
    void      getOutTransitions( std::vector< Transition* > &ts ) const;
    Transition* getOutTransitioni( int i) const;
    int       getNumberOfOutTransitions() const;
    float     getPosition() const;
    int	      getRank() const;
    void      getSubordinates( std::set< State* > &ss ) const;
    void      getSuperiors( std::set< State* > &ss ) const;
    int	      getValueIndexOfParam( int paramIndex );
    bool      isDeadlock() const;
    bool      isMarked() const;
    //bool      isSelected() const;
    void      mark();
    //void      select();
    void      setCluster( Cluster* c );
    void      setPosition( float p );
    void      setRank( int r );
    void      unmark();
   

  private:
    Cluster*		  cluster;
    std::set< State* >	  comrades;
    //float		  currentProbability;
    std::vector< Transition* > inTransitions;
    std::vector< Transition* > loops;
    bool		  marked;
    //float		  probability;
    std::vector< Transition* > outTransitions;
    float		  position;
    int			  rank;
    //bool		  selected;
    std::vector< ATerm >  stateVector;
    ATermList		  stateVectorAT;
    std::set< State* >	  subordinates;
    std::set< State* >	  superiors;
    Utils::DFSState       visitState;
};

#endif //STATE_H
