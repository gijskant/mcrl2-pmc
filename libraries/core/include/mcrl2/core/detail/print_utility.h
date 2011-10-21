// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/core/detail/print_utility.h
/// \brief add your file description here.

#ifndef MCRL2_CORE_DETAIL_PRINT_UTILITY_H
#define MCRL2_CORE_DETAIL_PRINT_UTILITY_H

#include <iostream>
#include <string>
#include <sstream>

#include "mcrl2/core/print.h"

namespace mcrl2
{

namespace core
{

namespace detail
{

struct default_printer
{
  template <typename T>
  std::ostream& operator()(const T& x, std::ostream& out) const
  {
    return out << x;
    return out;
  }
};

/// \brief Creates a string representation of a container using the pp pretty print function.
/// \param v A container
/// \param message A string
/// \param print_index If true, an index is written in front of each term
template <typename Container, typename Printer>
std::string print_container(const Container& v, Printer print = default_printer(), std::string begin_marker = "(", std::string end_marker = ")", std::string message = "", bool print_index = false)
{
  std::ostringstream out;
  if (!message.empty())
  {
    out << "--- " << message << "---" << std::endl;
  }
  out << begin_marker << " ";
  int index = 0;
  for (typename Container::const_iterator i = v.begin(); i != v.end(); ++i)
  {
    if (print_index)
    {
      out << index++ << " ";
      print(*i, out);
      out << std::endl;
    }
    else
    {
      if (i != v.begin())
      {
        out << ", ";
      }
      print(*i, out);
    }
  }
  out << " " << end_marker;
  return out.str();
}

/// \brief Creates a string representation of a container.
/// \param v A container
/// \param message A string
/// \param print_index If true, an index is written in front of each term
template <typename Container, typename Printer>
std::string print_list(const Container& v, Printer print = default_printer(), std::string message = "", bool print_index = false)
{
  return print_container(v, print, "[", "]", message, print_index);
}

/// \brief Creates a string representation of a container.
/// \param v A container
/// \param message A string
/// \param print_index If true, an index is written in front of each term
template <typename Container, typename Printer>
std::string print_set(const Container& v, Printer print = default_printer(), std::string message = "", bool print_index = false)
{
  return print_container(v, print, "{", "}", message, print_index);
}

} // namespace detail

} // namespace core

} // namespace mcrl2

#endif // MCRL2_CORE_DETAIL_PRINT_UTILITY_H
