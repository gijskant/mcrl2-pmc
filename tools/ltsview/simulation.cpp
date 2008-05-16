// Author(s): Bas Ploeger and Carst Tankink
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file simulation.cpp
/// \brief Add your file description here.

#include "simulation.h"

using namespace std;

Simulation::Simulation()
{
  //currState = NULL;
  started=false;
  chosenTrans = -1;
}



void Simulation::setInitialState(State* init)
{
  initialState = init;
}

void Simulation::start() {
  stateHis.push_back(initialState);
  initialState->setSimulated(true);
  currState = initialState;

  posTrans.clear();
  for (int i = 0; i < currState->getNumOutTransitions(); ++i) {
    posTrans.push_back(currState->getOutTransition(i));
    currState->getOutTransition(i)->getEndState()->setSimulated(true);
  }
  for (int i = 0; i < currState->getNumLoops(); ++i) {
    posTrans.push_back(currState->getLoop(i));
  }
  if (posTrans.size() > 0)
  {
    chosenTrans = 0;
  }
  else 
  {
    chosenTrans = -1;
  }
  
  started = true;
  //Fire signal
  signal();
}
void Simulation::stop()
{
  // Set started to false
  started = false;
  
  for (size_t i = 0; i < stateHis.size(); ++i)
  {
    stateHis[i]->setSimulated(false);
  }

  for(size_t i = 0; i < posTrans.size(); ++i)
  {
    posTrans[i]->getEndState()->setSimulated(false);
  }

  chosenTrans = -1;
  // Clear history
  stateHis.clear();
  transHis.clear();
  posTrans.clear();
  currState = NULL;
  // Fire signal
  signal();
  
}
  
Simulation::~Simulation() {
  //Stop simulation
  stop();

  transHis.clear();
  
  for(size_t i = 0; i < stateHis.size(); ++i)
  {
    stateHis[i]->setSimulated(false);
  }
  
  stateHis.clear();

  for(size_t i = 0; i < posTrans.size(); ++i)
  {
    posTrans[i]->getEndState()->setSimulated(false);
  }
  
  posTrans.clear();
}

vector< Transition* > const& Simulation::getTransHis() const {
  return transHis;
}

vector< State* > const& Simulation::getStateHis() const {
  return stateHis;
}

State* Simulation::getCurrState() const {
  return currState;
}

vector< Transition* > const& Simulation::getPosTrans() const {
  return posTrans;
}

Transition* Simulation::getChosenTrans() const {
  if (chosenTrans >= 0)
  {
    return posTrans[chosenTrans];
  }
  else
  {
    return NULL;
  }
}

int Simulation::getChosenTransi() const {
  return chosenTrans;
}
bool Simulation::getStarted() const {
  return started;
}

void Simulation::traceBack(State* initState)
{
  // First, reverse the vectors, so we can push each new transition at the back
  reverse(stateHis.begin(), stateHis.end());
  reverse(transHis.begin(), transHis.end());

  State* currPos = stateHis.back();

  while(currPos != initState)
  {
    transHis.push_back(currPos->getInTransition(0));
    currPos = currPos->getInTransition(0)->getBeginState();
    currPos->setSimulated(true);
    stateHis.push_back(currPos);
  }

  // Undo reversion
  reverse(transHis.begin(), transHis.end());
  reverse(stateHis.begin(), stateHis.end());

  signal();
}

void Simulation::followTrans() {
  if (chosenTrans != -1)
  {
    Transition* toFollow = posTrans[chosenTrans];
    State* nextState = toFollow->getEndState();
  
    transHis.push_back(posTrans[chosenTrans]);
   
    for(size_t i = 0; i < posTrans.size(); ++i)
    {
      posTrans[i]->getEndState()->setSimulated(false);
    }

    for(size_t i = 0; i < stateHis.size(); ++i)
    {
      stateHis[i]->setSimulated(true);
    }

    nextState->setSimulated(true);
  
    stateHis.push_back(nextState);
    currState = nextState;

    posTrans.clear();
    for (int i = 0; i < nextState->getNumOutTransitions(); ++i) {
      posTrans.push_back(nextState->getOutTransition(i));
      nextState->getOutTransition(i)->getEndState()->setSimulated(true);
    }
    for (int i = 0; i < nextState->getNumLoops(); ++i) {
      posTrans.push_back(nextState->getLoop(i));
    }

    if (posTrans.size() > 0)
    {
      chosenTrans = 0;
    }
    else
    {
      chosenTrans = -1;
    }

    //Fire signal
    signal();
  }
}
void Simulation::chooseTrans(int i) {
  chosenTrans = i;

  // Fire signal
  selChangeSignal();
}

void Simulation::undoStep() {
  State* lastState;
  
  // Remove last transition, state from history
  transHis.pop_back();

  stateHis.back()->setSimulated(false);
  stateHis.pop_back();
  
  for(size_t i = 0; i < posTrans.size(); ++i)
  {
    posTrans[i]->getEndState()->setSimulated(false);
    posTrans[i]->getEndState()->deselect();
  }
  
  // Set new states
  lastState = stateHis.back();
  currState = lastState;
  currState->setSimulated(true);

  posTrans.clear();
  for (int i = 0; i < currState->getNumOutTransitions(); ++i) {
    posTrans.push_back(currState->getOutTransition(i));
    currState->getOutTransition(i)->getEndState()->setSimulated(true);
  }
  for (int i = 0; i < currState->getNumLoops(); ++i) {
    posTrans.push_back(currState->getLoop(i));
  }
  
  if (posTrans.size() > 0)
  {
    chosenTrans = 0;
  }
  else
  {
    chosenTrans = -1;
  }

  // Fire signal
  signal();
}


void Simulation::resetSim() {
  State* firstState = stateHis.front();
  
  transHis.clear();
  for (size_t i = 0; i < stateHis.size(); ++i)
  {
    stateHis[i]->setSimulated(false);
  }
  
  stateHis.clear();

  firstState->setSimulated(true);
  
  stateHis.push_back(firstState);
  
  currState = firstState;

  posTrans.clear();
  for (int i = 0; i < currState->getNumOutTransitions(); ++i) {
    posTrans.push_back(currState->getOutTransition(i));
    currState->getOutTransition(i)->getEndState()->setSimulated(true);
  }
  for (int i = 0; i < currState->getNumLoops(); ++i) {
    posTrans.push_back(currState->getLoop(i));
  }
  
  if (posTrans.size() > 0)
  {
    chosenTrans = 0;
  }
  else 
  {
    chosenTrans = -1;
  }

  // Fire signal
  signal();
}

Simulation::simConnection Simulation::connectSel(
              simulationSignal::slot_function_type subscriber
            )
{
  simConnection result = selChangeSignal.connect(subscriber);

  // Send acknowledgement to subscriber;
  selChangeSignal();

  return result;
}

Simulation::simConnection Simulation::connect( 
              simulationSignal::slot_function_type subscriber
            )
{
  simConnection result = signal.connect(subscriber);
  // Send acknowledge to subscriber
  signal();
  // return value
  return result;
}

void Simulation::disconnect(Simulation::simConnection subscriber)
{
  subscriber.disconnect();
}

