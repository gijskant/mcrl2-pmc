// Author(s): VitaminB100
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file processreference.cpp
//
// Implements the process_reference class.

#include <wx/tokenzr.h>

#include "processreference.h"

using namespace grape::libgrape;

process_reference::process_reference( void )
: compound_reference( PROCESS_REFERENCE )
{
  m_parameter_assignments.Clear();
  m_refers_to_process = 0;
}

process_reference::process_reference( const process_reference &p_process_ref )
: compound_reference( p_process_ref )
{
  m_parameter_assignments = p_process_ref.m_parameter_assignments;
  m_refers_to_process = p_process_ref.m_refers_to_process;
  m_text = p_process_ref.m_text;
}

process_reference::~process_reference( void )
{

  // Remove all references to this object.
  for ( uint i = 0; i < m_has_channel.GetCount(); ++i )
  {
    channel* channel_ptr = m_has_channel.Item(i);
    channel_ptr->detach_reference();
  }

  // Free all used resources.
  m_has_channel.Clear();
  m_parameter_assignments.Clear();
}

process_diagram* process_reference::get_relationship_refers_to( void )
{
  return m_refers_to_process;
}

void process_reference::set_relationship_refers_to( process_diagram* p_proc_diagram )
{
  m_refers_to_process = p_proc_diagram;
}

list_of_varupdate process_reference::get_parameter_updates( void ) const
{
  return m_parameter_assignments;
}

void process_reference::set_parameter_updates( const list_of_varupdate& p_parameter_assignments )
{
  m_parameter_assignments = p_parameter_assignments;
}

bool process_reference::set_text( const wxString &p_text )
{
  m_text = p_text;
  // Do processing, i.e. make parameter declarations

  m_parameter_assignments.Empty();
  wxStringTokenizer tkw(m_text, _T(";"));
  varupdate p_upd;
  while(tkw.HasMoreTokens())
  {
    wxString token = tkw.GetNextToken();
    p_upd.set_varupdate(token);
    m_parameter_assignments.Add(p_upd);
  }

  return true;
}

wxString process_reference::get_text() const
{
  wxString result;
  for ( uint i = 0; i < m_parameter_assignments.GetCount(); ++i )
  {
    varupdate parameter_assignment = m_parameter_assignments.Item( i );
    result += parameter_assignment.get_lhs() + _T( ":=" ) + parameter_assignment.get_rhs() + _T( ";" );
  }
  return result;
}

// WxWidgets dynamic array implementation.
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( arr_process_reference );
