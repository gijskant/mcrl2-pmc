// Author(s): VitaminB100
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file visualnonterminating_transition.cpp
//
// Implements the visualnonterminating_transition class.

#include "nonterminatingtransition.h"
#include "transition.h"
#include "visualnonterminating_transition.h"
#include "geometric.h"
#include "state.h"
#include "font_renderer.h"
#include "label.h"

using namespace grape::grapeapp;

visualnonterminating_transition::visualnonterminating_transition( nonterminating_transition* p_nonterminating_transition )
{
  m_object = p_nonterminating_transition;
  m_same_begin_end = false;
  compound_state *beginstate = p_nonterminating_transition->get_beginstate();
  compound_state *endstate = p_nonterminating_transition->get_endstate();

  // if the transition has a beginstate and an endstate
  if ( ( beginstate != 0 ) && ( endstate != 0 ) )
  {
    if ( beginstate != endstate )
    {
      m_arrow_base = get_coordinate_on_edge( m_object->get_coordinate(), beginstate ) - m_object->get_coordinate();
      m_arrow_head = get_coordinate_on_edge( m_object->get_coordinate(), endstate ) - m_object->get_coordinate();
    }
    else
    {
      m_same_begin_end = true;
      coordinate base_corner = { beginstate->get_coordinate().m_x - beginstate->get_height(), beginstate->get_coordinate().m_y };
      m_arrow_base = get_coordinate_on_edge( base_corner, beginstate ) - m_object->get_coordinate();
      coordinate head_corner = { base_corner.m_x, base_corner.m_y - beginstate->get_width() };
      m_arrow_head = get_coordinate_on_edge( head_corner, beginstate ) - m_object->get_coordinate();\
    }
  }

  // if the transition only has a endstate.
  if ( ( endstate != 0 ) && ( beginstate == 0 ) )
  {
    m_arrow_base.m_x = 0;
    m_arrow_base.m_y = 0;
    m_arrow_head = get_coordinate_on_edge( m_arrow_base, endstate) - m_object->get_coordinate();
  }

  // if the transition only has an beginstate.
  if ( ( beginstate != 0 ) && ( endstate == 0 ) )
  {
    m_arrow_head.m_x = m_object->get_width();
    m_arrow_head.m_y = m_object->get_height();
    m_arrow_base = get_coordinate_on_edge( m_arrow_head, beginstate ) - m_object->get_coordinate();
  }

  if ( ( beginstate == 0 ) && ( endstate == 0 ) )
  {
    m_arrow_base.m_x = 0;
    m_arrow_base.m_y = 0;
    m_arrow_head.m_x = m_object->get_width();
    m_arrow_head.m_y = m_object->get_height();
  }
}

visualnonterminating_transition::visualnonterminating_transition( const visualnonterminating_transition &p_nonterminating_transition )
: visual_object( p_nonterminating_transition )
{
  m_object = p_nonterminating_transition.m_object;
  m_arrow_base = p_nonterminating_transition.m_arrow_base;
  m_arrow_head = p_nonterminating_transition.m_arrow_head;
}

visualnonterminating_transition::~visualnonterminating_transition( void )
{
}

void visualnonterminating_transition::draw( void )
{
  nonterminating_transition *transition = static_cast<nonterminating_transition *>(m_object);
  wxString text = transition->get_label()->get_text();
  bool selected = m_object->get_selected();

  coordinate startpoint = {m_arrow_base.m_x + m_object->get_coordinate().m_x, m_arrow_base.m_y + m_object->get_coordinate().m_y};
  coordinate controlpoint = m_object->get_coordinate();
  coordinate endpoint = {m_arrow_head.m_x + m_object->get_coordinate().m_x, m_arrow_head.m_y + m_object->get_coordinate().m_y};

  draw_nonterminating_transition( startpoint, controlpoint, endpoint, selected, text);
}

bool visualnonterminating_transition::is_inside( libgrape::coordinate &p_coord )
{
  return is_inside_nonterminating_transition_same_state( m_object->get_coordinate(), m_arrow_base, m_arrow_head, p_coord );
}

grape_direction visualnonterminating_transition::is_on_border( libgrape::coordinate &p_coord )
{
  // is_on_border for a transition has not been defined
  return grab_bounding_box( m_object->get_coordinate(), m_object->get_width(), m_object->get_height(), p_coord, m_object->get_selected() );
}

bool visualnonterminating_transition::is_nearest_head( libgrape::coordinate &p_coord )
{
  bool result = false;
  // the line is drawn differently (because it starts and ends in the same state)
  if ( m_same_begin_end )
  {
    // retrieve only the first point
    coordinate first = { m_arrow_base.m_x + m_object->get_coordinate().m_x - 0.5 * m_object->get_width(), m_arrow_base.m_y };
    result = ( is_inside_line( m_arrow_base + m_object->get_coordinate(), first, p_coord ) );
  }
  else
  {
    // draw transition line
    result = is_nearest_beginpoint( m_arrow_head + m_object->get_coordinate(), m_arrow_base + m_object->get_coordinate(), p_coord );
  }
  return result;
}
