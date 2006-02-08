#include "lts.h"

LTS::LTS( Mediator* owner)
{
  mediator = owner;
  initialState = NULL;
}

LTS::~LTS()
{
  ATunprotectList( &transitionLabels );
  ATunprotectList( &stateVectorSpec );
  
  for ( unsigned int i = 0 ; i < states.size() ; ++i )
  {
    delete states[i];
  }
  states.clear();
  initialState = NULL;
  
  for ( unsigned int i = 0 ; i < transitions.size() ; ++i )
  {
    delete transitions[i];
  }
  transitions.clear();
}

void LTS::setStateVectorSpec( ATermList spec )
{
  stateVectorSpec = spec;
  ATprotectList( &stateVectorSpec );
}

ATermList LTS::getStateVectorSpec()
{
  return stateVectorSpec;
}
  
void LTS::setInitialState( State* s )
{
  initialState = s;
}

/*
void LTS::addDataType( DataType* dt )
{
  dataTypes.push_back( dt );
}
*/

void LTS::addState( State* s )
{
  states.push_back( s );
}

void LTS::addTransition( Transition* t )
{
  transitions.push_back( t );
}

void LTS::addTransitionLabels( ATermList labels )
{
  transitionLabels = labels;
  ATprotectList( &transitionLabels );
}

void LTS::getClustersAtRank( unsigned int r, vector< Cluster* > &cs ) const
{
  if ( r < clustersInRank.size() )
    cs = clustersInRank[ r ];
}

State* LTS::getInitialState() const
{
  return initialState;
}

int LTS::getNumberOfRanks() const
{
  return clustersInRank.size();
}

int LTS::getNumberOfClusters() const
{
  int result = 0;
  for ( unsigned int i = 0 ; i < clustersInRank.size() ; ++i )
    result += clustersInRank[i].size();
  return result;
}

int LTS::getNumberOfStates() const
{
  return states.size();
}

int LTS::getNumberOfTransitions() const
{
  return transitions.size();
}

// Ranks the states iteratively and clusters comrades
void LTS::applyIterativeRanking()
{
  clearRanksAndClusters();
  
  int currRank = 0;
  initialState->setRank( currRank );

  vector< State* > nextRank;
  nextRank.push_back( initialState );
  statesInRank.push_back( nextRank );
  
  while ( statesInRank[ currRank ].size() > 0 )
  {
    nextRank.clear();
    
    // iterate over the states in this rank
    vector< State* >::iterator it;
    for ( it  = statesInRank[ currRank ].begin() ; 
	  it != statesInRank[ currRank ].end()	 ; ++it )
    {
      State* begState = *it;
      
      // iterate over all out-transitions of begState
      vector< Transition* > outTrans;
      begState->getOutTransitions( outTrans );
      vector< Transition* >::iterator tit;
      for ( tit = outTrans.begin() ; tit != outTrans.end() ; ++tit )
      {
	(**tit).setBackpointer( false );
	State* endState = (**tit).getEndState();
	
	if ( endState->getRank() == -1 )
	{
	  endState->setRank( currRank+1 );
	  begState->addSubordinate( endState );
	  endState->addSuperior( begState );
	  nextRank.push_back( endState );
	}
	else if ( endState->getRank() == currRank+1 )
	{
	  begState->addSubordinate( endState );
	  endState->addSuperior( begState );
	}
	else if ( endState->getRank() == currRank )
	{
	  begState->addComrade( endState );
	  endState->addComrade( begState );
	}
	else // 0 <= endState->getRank() < currRank
	{
	  (**tit).setBackpointer( true );
	}
      }
    }
    statesInRank.push_back( nextRank );
      
    ++currRank;
  }
  // last element of statesInRank is an empty vector
  statesInRank.erase( statesInRank.end()-1 );
}

void LTS::applyCyclicRanking()
{
  clearRanksAndClusters();
  
  int currRank = 0;
  initialState->setRank( currRank );

  vector< State* > nextRank;
  nextRank.push_back( initialState );
  statesInRank.push_back( nextRank );
  
  while ( statesInRank[ currRank ].size() > 0 )
  {
    nextRank.clear();
    
    // iterate over the states in this rank
    vector< State* >::iterator it;
    for ( it  = statesInRank[ currRank ].begin() ; 
	  it != statesInRank[ currRank ].end()	 ; ++it )
    {
      State* curState = *it;
      
      // iterate over all in-transitions of curState
      vector< Transition* > trans;
      curState->getInTransitions( trans );
      vector< Transition* >::iterator transit;
      
      for ( transit = trans.begin() ; transit != trans.end() ; ++transit )
      {
	(**transit).setBackpointer( false );
	State* begState = (**transit).getBeginState();
	if ( begState->getRank() == -1 )
	{
	  begState->setRank( currRank+1 );
	  nextRank.push_back( begState );
	  begState->addSuperior( curState );
	  curState->addSubordinate( begState );
	}
	else if ( begState->getRank() == currRank+1 )
	{
	  begState->addSuperior( curState );
	  curState->addSubordinate( begState );
	}
	else if ( begState->getRank() == currRank )
	{
	  begState->addComrade( curState );
	  curState->addComrade( begState );
	}
      }

      // iterate over all out-transitions of curState
      
      curState->getOutTransitions( trans );
      for ( transit = trans.begin() ; transit != trans.end() ; ++transit )
      {
	State* endState = (**transit).getEndState();
	if ( endState->getRank() == -1 )
	{
	  endState->setRank( currRank+1 );
	  nextRank.push_back( endState );
	  endState->addSuperior( curState );
	  curState->addSubordinate( endState );
	}
	else if ( endState->getRank() == currRank+1 )
	{
	  endState->addSuperior( curState );
	  curState->addSubordinate( endState );
	}
	else if ( endState->getRank() == currRank )
	{
	  endState->addComrade( curState );
	  curState->addComrade( endState );
	}
      }
    }
    statesInRank.push_back( nextRank );
    ++currRank;
  }
  // last element of statesInRank is an empty vector
  statesInRank.erase( statesInRank.end()-1 );
}

void LTS::clearRanksAndClusters()
{
  vector< State* >::iterator it;
  for ( it = states.begin() ; it != states.end() ; ++it )
  {
    State* state = *it;
    state->setRank( -1 );
    state->setCluster( NULL );
    state->clearHierarchyInfo();
  }

  for ( unsigned int r = 0 ; r < clustersInRank.size() ; ++r )
    for ( unsigned int c = 0 ; c < clustersInRank[r].size() ; ++c )
      delete clustersInRank[r][c];
  
  vector< vector< State* > > temp1;
  vector< vector< Cluster* > > temp2;
  statesInRank.swap( temp1 );
  clustersInRank.swap( temp2 );
}

void LTS::clusterComrades()
{
  vector< vector< State* > >::iterator rankit;
  for ( rankit  = statesInRank.begin() ;
	rankit != statesInRank.end()   ; ++rankit )
  {
    vector< Cluster* > cluslist;
    vector< State* >::iterator stateit;
    for ( stateit = (*rankit).begin() ; stateit != (*rankit).end() ; ++stateit )
    {
      if ( (**stateit).getCluster() == NULL )
      {
	Cluster* c = new Cluster();
	cluslist.push_back( c );
	addComradesToCluster( c, *stateit );
      }
    }
    clustersInRank.push_back( cluslist );
  }
}

void LTS::addComradesToCluster( Cluster* c, State* s )
{
  if ( s->getCluster() == NULL )
  {
    c->addState( s );
    s->setCluster( c );
    set< State* > comrades;
    s->getComrades( comrades );
    set< State* >::iterator comit;
    for ( comit = comrades.begin() ; comit != comrades.end() ; ++comit )
      addComradesToCluster( c, *comit );
  }
}

void LTS::mergeSuperiorClusters()
{
  // iterate over the ranks in reverse order (bottom-up)
  vector< vector< Cluster* > >::reverse_iterator rankit;
  for ( rankit  = clustersInRank.rbegin() ; 
	rankit != clustersInRank.rend()-1 ; ++rankit )
  {
    vector< Cluster* > *prevRank = &(*(rankit+1));
    // iterate over the clusters in this rank
    vector< Cluster* >::iterator clusit;
    for ( clusit  = (*rankit).begin() ; clusit != (*rankit).end() ; ++clusit )
    {
      set< Cluster* > mergeSet;
      
      // iterate over the states in this cluster
      vector< State* > clusstates;
      (**clusit).getStates( clusstates );
      
      vector< State* >::iterator stateit;
      for ( stateit  = clusstates.begin() ; 
	    stateit != clusstates.end()   ; ++stateit )
      {
	// iterate over the superiors of this state
	set< State* > superiors;
	(**stateit).getSuperiors( superiors );
	
	set< State* >::iterator superit;
	for ( superit  = superiors.begin() ; 
	      superit != superiors.end()   ; ++superit )
	{
	  // add the superior's cluster to the merge set
	  mergeSet.insert( (**superit).getCluster() );
	}
      }
      
      if ( mergeSet.size() > 1 )
      {
	Cluster* c = new Cluster();
	
	// iterate over the clusters in the mergeSet
	set< Cluster* >::iterator clusit1;
	for ( clusit1  = mergeSet.begin() ;
	      clusit1 != mergeSet.end()   ; ++clusit1 )
	{
	  // add the cluster's states to c
	  (**clusit1).getStates( clusstates );
	  for ( stateit  = clusstates.begin() ;
		stateit != clusstates.end()   ; ++stateit )
	  {
	    c->addState( *stateit );
	    (**stateit).setCluster( c );
	  }
	  
	  // delete the cluster
	  prevRank->erase( find( prevRank->begin(), prevRank->end(), *clusit1 ) );
	  delete *clusit1;
	}
	prevRank->push_back( c );
      }
    }
    
    // clusters on previous rank have been merged; compute hierarchy info
    for ( clusit  = (*rankit).begin() ; clusit != (*rankit).end() ; ++clusit )
    {
      vector< State* > clusstates;
      set< State* > superiors;
      
      (**clusit).getStates( clusstates );
      clusstates[0]->getSuperiors( superiors );
      Cluster* ancestor = (**(superiors.begin())).getCluster();

      (**clusit).setAncestor( ancestor );
      ancestor->addDescendant( *clusit );
    }
  }
}

// function for test purposes
void LTS::printStructure()
{
  // give every state an id
  map< State*, int > stateId;
  for ( unsigned int i = 0 ; i < states.size() ; ++i )
  {
    stateId[ states[i] ] = i;
  }
  
  // give every cluster an id
  map< Cluster*, int > clusterId;
  int n = 0;
  for ( unsigned int i = 0 ; i < clustersInRank.size() ; ++i )
  {
    vector< Cluster* >::iterator j;
    for ( j = clustersInRank[i].begin() ; j != clustersInRank[i].end() ; ++j )
    {
      clusterId[ *j ] = n++;
    }
  }

  for ( unsigned int s = 0 ; s < states.size() ; ++s )
  {
    State* state = states[s];
    cout << stateId[state] << "\t";
    set< State* > ss;
    set< State* >::iterator sit;
    
    state->getSuperiors( ss );
    cout << "{";
    for ( sit = ss.begin() ; sit != ss.end() ; ++sit )
    {
      cout << stateId[*sit] << ",";
    }
    cout << "}\t";
    
    state->getSubordinates( ss );
    cout << "{";
    for ( sit = ss.begin() ; sit != ss.end() ; ++sit )
    {
      cout << stateId[&(**sit)] << ",";
    }
    cout << "}\t";
    
    state->getComrades( ss );
    cout << "{";
    for ( sit = ss.begin() ; sit != ss.end() ; ++sit )
    {
      cout << stateId[&(**sit)] << ",";
    }
    cout << "}\t";
    
    cout << state->getRank() << endl;
  }

  for ( unsigned int r = 0 ; r < clustersInRank.size() ; ++r )
  {
    vector< Cluster* >::iterator clusit;
    for ( clusit = clustersInRank[r].begin() ; clusit != clustersInRank[r].end() ; ++clusit )
    {
      cout << clusterId[ *clusit ] << "\t";
      vector< State* > ss;
      (**clusit).getStates( ss );
      vector< State* >::iterator sit;
      cout << "{";
      for ( sit = ss.begin() ; sit!= ss.end() ; ++sit )
      {
	cout << stateId[ *sit ] << ",";
      }
      cout << "}\t";
      if ( (**clusit).getAncestor() != NULL )
	cout << clusterId[ (**clusit).getAncestor() ];
      cout << "\t";
      
      vector< Cluster* > cs;
      vector< Cluster* >::iterator cit;
      (**clusit).getDescendants( cs );
      cout << "{";
      for ( cit = cs.begin() ; cit != cs.end() ; ++cit )
      {
	cout << clusterId[ *cit ] << ",";
      }
      cout << "}\t" << r << endl;
    }
  }
}

void LTS::printClusterSizesPositions()
{
  for ( unsigned int i = 0 ; i < clustersInRank.size() ; ++i )
  {
    cout << "Clusters in rank " << i << ": ";
    vector< Cluster* >::iterator j;
    for ( j = clustersInRank[i].begin() ; j != clustersInRank[i].end() ; ++j )
    {
      cout << "(" << (**j).getTopRadius() << "," << (**j).getSize() << "," << (**j).getPosition() << ") ";
    }
    cout << endl;
  }
}
