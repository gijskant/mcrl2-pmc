// Author(s): VitaminB100
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file nonterminatingtransition.cpp
//
// Implements the nonterminating_transition class.

#include "nonterminatingtransition.h"
#include "compoundstate.h"

using namespace grape::libgrape;

nonterminating_transition::nonterminating_transition( compound_state* p_beginstate, compound_state* p_endstate )
: transition( NONTERMINATING_TRANSITION )
{
  m_linetype = straight;
  m_breakpoints.Empty();
  m_beginstate = p_beginstate;
  m_endstate = p_endstate;
}

nonterminating_transition::nonterminating_transition( const nonterminating_transition &p_ntt )
: transition( p_ntt )
{
  m_endstate = p_ntt.m_endstate;
}

nonterminating_transition::~nonterminating_transition( void )
{
  if ( m_beginstate != 0 )
  {
    // Remove the reference from the state that is the beginstate of this transition
    m_beginstate->detach_transition_beginstate( this );
  }

  if ( m_endstate != 0 )
  {
    // Remove the reference from the state that is the endstate of this transition
    m_endstate->detach_transition_endstate( this );
  }

  // Free all space used by this object
  m_breakpoints.Clear();
}

void nonterminating_transition::attach_endstate( compound_state* p_endstate )
{
  m_endstate = p_endstate;
}

compound_state * nonterminating_transition::get_endstate( void )
{
  return m_endstate;
}

void nonterminating_transition::detach_endstate( void )
{
  m_endstate = 0;
}

// WxWidgets dynamic array implementation.
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( arr_nonterminating_transition );
