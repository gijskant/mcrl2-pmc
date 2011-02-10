// Author(s): Diana Koenraadt, Remco Blewanus, Bram Schoenmakers, Thorstin Crijns, Hans Poppelaars, Bas Luksenburg, Jonathan Nelisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file decl.cpp
//
// Implements the decl datatype.

#include "wx.hpp" // precompiled headers
#include "wx/wx.h"
#include <sstream>
#include "mcrl2gen/mcrl2gen_validate.h"
#include "mcrl2/core/print.h"

#include "decl.h"

using namespace mcrl2::core;
using namespace grape::libgrape;
using namespace grape::mcrl2gen;
using namespace std;

decl::decl(void)
{
  m_name = wxEmptyString;
}

decl::decl(const decl& p_decl)
{
  m_name = p_decl.m_name;
  m_type = p_decl.m_type;
}

decl::~decl(void)
{
}

wxString decl::get_decl(void) const
{
  return get_name()+_T(":")+get_type();
}

bool decl::set_decl(const wxString& p_decl)
{
  int pos = p_decl.Find(_T(":"));
  wxString name = p_decl.Mid(0, pos);
  wxString type = p_decl.Mid(pos+1);
  name.Trim(true);
  name.Trim(false);
  type.Trim(true);
  type.Trim(false);
  if (name.IsEmpty() || type.IsEmpty() || pos == wxNOT_FOUND)
  {
    return false;
  }

  ATermAppl a_parsed_identifier = parse_identifier(name);

// TODO: use other line
  if (is_identifier(name))
//  if ( a_parsed_identifier )
  {
    string a_name = identifier_string(a_parsed_identifier);
    set_name(wxString(a_name.c_str(), wxConvLocal));

    ATermAppl a_parsed_sort_expr = parse_sort_expr(type);
    if (a_parsed_sort_expr)
    {
      string a_type = pp(a_parsed_sort_expr);
      set_type(wxString(a_type.c_str(), wxConvLocal));
      return true;
    }
  }
  return false;
}

wxString decl::get_name(void) const
{
  return m_name;
}

void decl::set_name(const wxString& p_name)
{
  m_name = p_name;
  m_name.Trim(true);
  m_name.Trim(false);
}

wxString decl::get_type(void) const
{
  return m_type.get_expression();
}

void decl::set_type(const wxString& p_type)
{
  m_type.set_expression(p_type);
}

// WxWidgets dynamic array implementation.
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(list_of_decl)

decl_init::decl_init(void) : decl()
{
}

decl_init::decl_init(const decl_init& p_decl_init) : decl(p_decl_init)
{
  m_value = p_decl_init.m_value;
}

decl_init::~decl_init(void)
{
}

wxString decl_init::get_decl_init(void) const
{
  return get_decl()+_T("=")+get_value();
}

bool decl_init::set_decl_init(const wxString& p_decl_init)
{
  int pos = p_decl_init.Find(_T(":"));
  wxString name = p_decl_init.Mid(0, pos);
  wxString type_value = p_decl_init.Mid(pos+1);
  int pos1 = type_value.Find(_T("="));
  wxString type = type_value.Mid(0, pos1);
  wxString value = type_value.Mid(pos1+1);
  name.Trim(true);
  name.Trim(false);
  type.Trim(true);
  type.Trim(false);
  value.Trim(true);
  value.Trim(false);
  if (name.IsEmpty() || type.IsEmpty() || value.IsEmpty() || pos == wxNOT_FOUND || pos1 == wxNOT_FOUND)
  {
    return false;
  }

  ATermAppl a_parsed_identifier = parse_identifier(name);
// TODO: use other line
  if (is_identifier(name))
//  if ( a_parsed_identifier )
  {
    string a_name = identifier_string(a_parsed_identifier);
    set_name(wxString(a_name.c_str(), wxConvLocal));

    ATermAppl a_parsed_sort_expr = parse_sort_expr(type);
    if (a_parsed_sort_expr)
    {
      string a_type = pp(a_parsed_sort_expr);
      set_type(wxString(a_type.c_str(), wxConvLocal));

      ATermAppl a_parsed_data_expr = parse_data_expr(value);
      if (a_parsed_data_expr)
      {
        string a_value = pp(a_parsed_data_expr);
        set_value(wxString(a_value.c_str(), wxConvLocal));
        return true;
      }
    }
  }
  return false;
}

wxString decl_init::get_value(void) const
{
  return m_value.get_expression();
}

void decl_init::set_value(const wxString& p_value)
{
  m_value.set_expression(p_value);
}

// WxWidgets dynamic array implementation.
#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(list_of_decl_init)

