// Author(s): Bas Ploeger and Carst Tankink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file transition.cpp
/// \brief Add your file description here.

#include "transition.h"
using namespace Utils;

// Constructor and Destructor

Transition::Transition(State* bs,State* es,int lbl)
: beginState(bs), endState(es), label(lbl), marked(NULL) {
}

Transition::~Transition() {
}

State* Transition::getBeginState() const
{
  return beginState;
}

State* Transition::getEndState() const
{
  return endState;
}

int Transition::getLabel() const
{
  return label;
}

bool Transition::isBackpointer() const
{
  return (beginState->getRank() > endState->getRank());
}

bool Transition::isSelfLoop() const
{
  return (beginState == endState);
}

bool Transition::isMarked() const
{
  return *marked;
}

void Transition::setMarkedPointer( bool* bp )
{
  marked = bp;
}

