// Author(s): Bas Ploeger and Carst Tankink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file cluster.h
/// \brief Add your file description here.

#ifndef CLUSTER_H
#define CLUSTER_H
#include <vector>
#include <map>
#include "utils.h"

#ifndef STATE_H
  #include "state.h"
#else
  class State;
#endif

// forward declaration
class Cluster;

// class for cluster comparison based on cluster radii
class Comp_BCRadius {
  public:
    bool operator()(const Cluster*,const Cluster*) const;
};

// class for cluster comparison based on cluster volumes
class Comp_BCVolume {
  public:
    bool operator()(const Cluster*,const Cluster*) const;
};

class Cluster {

  public:
    // Constructor & destructor.
    Cluster(int r);
    ~Cluster();

    // Methods on descendants and ancestors.
    void addDescendant(Cluster* c);
    Cluster* getDescendant(int i) const;
    int getNumDescendants() const;
    bool hasDescendants() const;
    
    // Lets the cluster pretend that it does not have descendant i (for
    // zooming)
    void severDescendant(int i);

    // Restores the severance
    void healSeverance( int i);
    bool hasSeveredDescendants();
    void setAncestor(Cluster* c);
    Cluster* getAncestor() const;

    // Methods on states
    void addState(State* s);
    void addUndecidedState(State* s);
    State* getState(int i) const;
    int getNumStates() const;
    bool hasDeadlock() const;
    void setDeadlock( bool b );

    void addMatchedRule(int mr);
    void removeMatchedRule(int mr);
    //void getMatchedRules(std::vector< int > &mrs);
    int getNumMarkedStatesAll();
    int getNumMarkedStatesAny();
    void setNumMarkedStatesAll(int n);
    void setNumMarkedStatesAny(int n);
    int getNumMarkedTransitions();
    /*
    bool getMarking() const;
    void setMarking(bool b);
    void setMarkAllEmpty(bool b);
    bool hasMarkedState() const;
    int getMarkedTransitionCount() const;

    // Calls s->mark(rule) for all s in states that match rule, and adapts
    // markedState to the number of states marked.
    void markState(Utils::MarkRule* rule);

    // Calls s->unmark(rule) for all s in states that match rule, and adapts 
    // markedState to the number of states marked.
    void unmarkState(Utils::MarkRule* rule);
    */

    unsigned int getTotalNumSlots() const;
    unsigned int getNumSlots(unsigned int ring) const;
    void occupySlot(unsigned int ring,float pos,State* s);
    void occupyCenterSlot(State* s);
    void resolveSlots();
    void clearSlots();
    void positionStatesSpiral();

    // Methods on transitions
    void addActionLabel(int l);
    bool hasMarkedTransition() const;
    int setActionMark(int l,bool b);
    
    // General cluster information
    void center();
    void computeSizeAndPositions();
    void computeSizeAndPositions_FSM();
    float getBaseRadius() const;
    float getPosition() const;
    int getRank() const;
    int getPositionInRank() const;
    void setPositionInRank(int p);
    float getTopRadius() const;
    float getBCHeight() const;
    float getBCRadius() const;
    float getBCVolume() const;
    bool isCentered() const;
    void setPosition(float p);

    // Methods for visualization
    int getVisObject() const;
    void setVisObject(int vo);
    int getBranchVisObject(int i) const;
    int getNumBranchVisObjects() const;
    void addBranchVisObject(int vo);
    void clearBranchVisObjects();
    
    // Methods for selection
    void select();
    void deselect();
    bool isSelected() const;

  private:
    std::map< int, int > actionLabelCounts;
    Cluster* ancestor;
    float baseRadius;
    bool deadlock;
    std::vector< Cluster* > descendants;
    std::vector<std::vector<bool> > severedDescendants;
    unsigned int severedDescendantsC;
    int numMarkedStatesAll;
    int numMarkedStatesAny;
    /*
    bool marking;
    bool markAllEmpty;*/
    int numMarkedTransitions;
    float position;
    int rank;
    int positionInRank;
    std::vector< State* > states;
    float topRadius;
    float bc_radius; // radius of the bounding cylinder that contains this cluster's subtree
    float bc_height; // height of the bounding cylinder that contains this cluster's subtree
    std::vector< State* > undecidedStates;
    std::vector< std::vector< std::vector< State* > > > slots;
    int visObject;
    std::vector< int > branchVisObjects;
    bool selected;
    std::set< int > matchedRules;

    void slotUndecided(unsigned int ring,unsigned int from,unsigned int to);
    void spreadSlots(unsigned int ring);
};

#endif
