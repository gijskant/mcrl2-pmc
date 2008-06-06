// Author(s): VitaminB100
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file dataexpression.cpp
//
// Implements the dataexpression datatype.

#include "dataexpression.h"

using namespace grape::libgrape;

dataexpression::dataexpression( void )
{
  m_expression = wxEmptyString;
}

dataexpression::dataexpression( const dataexpression &p_dataexpression )
{
  m_expression = p_dataexpression.m_expression;
  m_sortexpression = p_dataexpression.m_sortexpression;
}

dataexpression::~dataexpression( void )
{
}

void dataexpression::set_expression( const wxString &p_expression )
{
  m_expression = p_expression;
}

wxString dataexpression::get_expression( void ) const
{
  return m_expression;
}

// WxWidgets dynamic array implementation.
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY( list_of_dataexpression );
