// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/core/print_format.h
/// \brief add your file description here.

#ifndef MCRL2_CORE_PRINT_FORMAT_H
#define MCRL2_CORE_PRINT_FORMAT_H

#include <string>
#include "mcrl2/exception.h"

namespace mcrl2 {

namespace core {

/// \brief print_format_type represents the available pretty print formats
typedef enum { print_default, print_internal } print_format_type;

/// \brief Print string representation of pretty print format
/// \param pp_format a pretty print format
/// \return string representation of the pretty print format
/// \throws mcrl2::runtime error if an unknown pretty print format
///         is passed into the function.
inline
std::string pp_format_to_string(const print_format_type pp_format)
{
  switch (pp_format)
  {
    case print_default:
      return "default";
    case print_internal:
      return "internal";
    default:
      throw mcrl2::runtime_error("Unknown pretty print format");
  }
}

inline
std::ostream& operator<<(std::ostream& os, const print_format_type& pp_format)
{
  os << pp_format_to_string(pp_format);
  return os;
}

inline
print_format_type parse_pp_format(const std::string& s)
{
  if(s == "default")
  {
    return print_default;
  }
  else if(s == "internal")
  {
    return print_internal;
  }
  else
  {
    throw mcrl2::runtime_error("Unknown pretty print format " + s);
  }
}

inline
std::istream& operator>>(std::istream& is, print_format_type& f)
{
  try
  {
    std::string s;
    is >> s;
    f = parse_pp_format(s);
  }
  catch(mcrl2::runtime_error &)
  {
    is.setstate(std::ios_base::failbit);
  }
  return is;
}

} // namespace core

} // namespace mcrl2

#endif // MCRL2_CORE_PRINT_FORMAT_H
