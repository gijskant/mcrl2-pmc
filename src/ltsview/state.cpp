#include "state.h"
#include "liblowlevel.h"
using namespace std;
using namespace Utils;

State::State( ATermList sv )
{
  stateVectorAT = sv;
  ATprotectList( &stateVectorAT );
  while ( !ATisEmpty( sv ) )
  {
    stateVector.push_back( ATgetFirst( sv ) );
    sv = ATgetNext( sv );
  }
  cluster = NULL;
  rank = 0;
  position = -1.0f;
  marked = false;
  outTransitions = vector<Transition*> ();
  visitState = DFS_WHITE;

  // Simulation initialisation
  selected = false;
  simulationState = UNSEEN;
  visitedAt = 0;
}

State::~State()
{
  ATunprotectList( &stateVectorAT );
}

void State::addSuperior( State* s )
{
  superiors.insert( s );
}

void State::addSubordinate( State* s )
{
  subordinates.insert( s );
}

void State::addComrade( State* s )
{
  comrades.insert( s );
}

void State::addInTransition( Transition* trans )
{
  inTransitions.push_back( trans );
}

void State::addOutTransition( Transition* trans )
{
  outTransitions.push_back( trans );
}

void State::addLoop( Transition* trans )
{
  loops.push_back( trans );
}

bool State::isDeadlock() const
{
  return ( outTransitions.size() + loops.size() == 0 );
}

bool State::isMarked() const
{
  return marked;
}

void State::mark()
{
  marked = true;
}

void State::unmark()
{
  marked = false;
}
/*
bool State::isSelected() const
{
  return selected;
}

void State::select()
{
  selected = true;
}

void State::deselect()
{
  selected = false;
}
*/
int State::getRank() const
{
  return rank;
}

void State::setRank( int r )
{
  rank = r;
}

int State::getSlot() const
{
  return slot;
}

void State::setSlot( int s )
{
  slot = s;
}


float State::getPosition() const
{
  return position;
}

Point3D State::getPositionAbs() const
{
  return positionAbs;
}

Point3D State::getOutgoingControl() const 
{
  return outgoingControl;
}

Point3D State::getIncomingControl() const 
{
  return incomingControl;
}

void State::setPosition( float p )
{
  position = p;
}

void State::setPositionAbs(Point3D p)
{
  positionAbs = p;
}

void State::setOutgoingControl(Point3D p) 
{
  outgoingControl = p;
}

void State::setIncomingControl(Point3D p) 
{
  incomingControl = p;
}

Cluster* State::getCluster() const
{
  return cluster;
}

void State::setCluster( Cluster* c )
{
  cluster = c;
}

void State::getSubordinates( set< State* > &ss ) const
{
  ss = subordinates;
}

void State::getSuperiors( set< State* > &ss ) const
{
  ss = superiors;
}

void State::getComrades( set< State* > &ss ) const
{
  ss = comrades;
}

void State::getInTransitions( vector< Transition* > &ts ) const
{
  ts = inTransitions;
}

void State::getOutTransitions( vector< Transition* > &ts ) const
{
  ts = outTransitions;
}

Transition* State::getOutTransitioni( int i ) const
{
  return outTransitions[i];
}

int State::getNumberOfOutTransitions( ) const 
{
  return outTransitions.size();
}

void State::clearHierarchyInfo()
{
  superiors.clear();
  subordinates.clear();
  comrades.clear();
}
void State::DFSfinish() {
  visitState = DFS_BLACK;
}

void State::DFSclear() {
  visitState = DFS_WHITE;
}

void State::DFSvisit() {
  visitState = DFS_GREY;
}


DFSState State::getVisitState() const {
  return visitState;
}

int State::getValueIndexOfParam( int paramIndex )
{
  return ATgetInt((ATermInt)ATgetArgument((ATermAppl)stateVector[paramIndex],1));
}

// Methods for simulation, implementation
bool State::isSelected() const {
  return selected;
}

SimState State::getSimulationState() const {
  return simulationState;
}

int State::getVisitedAt() const {
  return visitedAt;
}


void State::simVisit(int va) {
  // Set state's state
  simulationState = NOW;
  visitedAt = va;
  
  // Trigger all outgoing transitions to be future ones:
  for(unsigned int i = 0; i < outTransitions.size(); ++i) {
    Transition* toFuture = outTransitions[i];
    toFuture->setSimulationState(FUTURE);
  }
  
  for(set<State*>::iterator it = subordinates.begin();
      it != subordinates.end(); ++it) {
    (*it)->setFuture(va + 1);
  }
  // TODO: How to handle with states on backpointers?
}

void State::simHistory() {
  // Set state's state
  simulationState = HISTORY;
  
  // Unset all subordinates
  for(set< State* >::iterator i = subordinates.begin(); 
      i != subordinates.end(); ++i) {
    State* toUnset = (*i);
    toUnset->simUnset();
  }

  // Unset all transitions
  for(unsigned int i = 0; i < outTransitions.size(); ++i) {
    Transition* toUnset = outTransitions[i];
    toUnset->setSimulationState(UNSEEN);
  }

  // TODO: How to handle with states on backpointers?
}

void State::simUnset() {
  // Sets the state's state
  simulationState = UNSEEN;

  // Unset the descendants
  for(set< State* >::iterator it = subordinates.begin(); 
      it != subordinates.end(); ++it) {
    (*it)->simUnset();
  }

  // Unset the transitions
  for(unsigned int i = 0; i < outTransitions.size(); ++i) {
    Transition* toUnset = outTransitions[i];
    toUnset->setSimulationState(UNSEEN);
  }
}

void State::setFuture(int va) {
  // Set state
  visitedAt = va;
  simulationState = FUTURE;

  for(set<State*>::iterator it = subordinates.begin();
      it != subordinates.end(); ++it) {
    (*it)->setFuture(va + 1);
  }
}


