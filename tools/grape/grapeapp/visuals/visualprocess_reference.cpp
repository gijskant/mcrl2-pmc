// Author(s): VitaminB100
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file visualprocess_reference.cpp
//
// Implements the visualprocess_reference class.

#include <string>

#include "grape_glcanvas.h"
#include "processreference.h"
#include "visualprocess_reference.h"
#include "geometric.h"
#include "mcrl2/utilities/font_renderer.h"

using namespace grape::grapeapp;
using namespace mcrl2::utilities;

visualprocess_reference::visualprocess_reference( process_reference* p_process_reference )
{
  m_object = p_process_reference;
}

visualprocess_reference::visualprocess_reference( const visualprocess_reference &p_process_reference )
: visual_object( p_process_reference )
{
}

visualprocess_reference::~visualprocess_reference( void )
{
}

void visualprocess_reference::draw( void )
{
  process_reference *procref = static_cast<process_reference *>(m_object);

  float width = m_object->get_width();
  float height = m_object->get_height();
  float x = m_object->get_coordinate().m_x-width/2;
  float y = m_object->get_coordinate().m_y-height/2;

  // draw process reference
  draw_process_reference( m_object->get_coordinate(), m_object->get_width(), m_object->get_height(), m_object->get_selected() );

  // draw process reference name 
//  render_text(procref->get_name(), x + width*0.5, y + height - g_text_space, width, height*0.25 + g_text_space, true);
  grape_glcanvas::get_font_renderer()->draw_wrapped_text( std::string(procref->get_name().fn_str()), x, x+width, y + height, y + height * 0.75, 0.0015f, al_center, al_top );

  // draw process reference text 
//  render_text(procref->get_text(), x, y + height*0.75 - g_text_space, width, height*0.75 + g_text_space);
  grape_glcanvas::get_font_renderer()->draw_wrapped_text( std::string(procref->get_text().fn_str()), x, x+width, y + height * 0.75, y, 0.0015f, al_left, al_top );

  // draw bounding box; only drawn if the object is selected
  draw_bounding_box( m_object->get_coordinate(), m_object->get_width(), m_object->get_height(), m_object->get_selected() );
}

bool visualprocess_reference::is_inside( libgrape::coordinate &p_coord )
{
  return is_inside_rectangle( m_object->get_coordinate(), m_object->get_width(), m_object->get_height(), p_coord ) || ( grab_bounding_box( m_object->get_coordinate(), m_object->get_width(), m_object->get_height(), p_coord, m_object->get_selected() ) != GRAPE_DIR_NONE );
}

grape_direction visualprocess_reference::is_on_border( libgrape::coordinate &p_coord )
{
  return grab_bounding_box( m_object->get_coordinate(), m_object->get_width(), m_object->get_height(), p_coord, m_object->get_selected() );
}

coordinate visualprocess_reference::move_to_border( libgrape::coordinate &p_coord )
{
  return move_to_border_rectangle( m_object->get_coordinate(), m_object->get_width(), m_object->get_height(), p_coord );
}
