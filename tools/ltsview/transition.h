// Author(s): Bas Ploeger and Carst Tankink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file transition.h
/// \brief Header file for Transition class

#ifndef TRANSITION_H
#define TRANSITION_H

class State;

class Transition
{
  public:
    Transition(State* bs,State* es,int label);
    ~Transition();
    State*	getBeginState() const;
    State*	getEndState() const;
    int			getLabel() const;
    bool		isBackpointer() const;
    bool                isSelfLoop() const;
    bool		isMarked() const;
    void		setMarkedPointer(bool* bp);

  private:
    State*  beginState;
    State*  endState;
    int     label;
    bool*   marked;
};

#endif
