// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/atermpp/make_list.h
/// \brief Convenience functions for creating an aterm_list.

#ifndef MCRL2_ATERMPP_MAKE_LIST_H
#define MCRL2_ATERMPP_MAKE_LIST_H

#include "mcrl2/atermpp/aterm_list.h"

namespace atermpp
{
/// \brief Makes a list with a fixed number of arguments.
/// \param t0 A list element.
/// \return The generated list.
template <typename T>
term_list<T> make_list(const T &t0)
{
  term_list<T> result;
  result.push_front(t0);
  return result;
}

/// \brief Makes a list with a fixed number of arguments.
/// \param t0 A list element.
/// \param t1 A list element.
/// \return The generated list.
template <typename T>
term_list<T> make_list(const T &t0, const T &t1)
{
  term_list<T> result;
  result.push_front(t1);
  result.push_front(t0);
  return result;
}

/// \brief Makes a list with a fixed number of arguments.
/// \param t0 A list element.
/// \param t1 A list element.
/// \param t2 A list element.
/// \return The generated list.
template <typename T>
term_list<T> make_list(const T &t0, const T &t1, const T &t2)
{
  term_list<T> result;
  result.push_front(t2);
  result.push_front(t1);
  result.push_front(t0);
  return result;
}

/// \brief Makes a list with a fixed number of arguments.
/// \param t0 A list element.
/// \param t1 A list element.
/// \param t2 A list element.
/// \param t3 A list element.
/// \return The generated list.
template <typename T>
term_list<T> make_list(const T &t0, const T &t1, const T &t2, const T &t3)
{
  term_list<T> result;
  result.push_front(t3);
  result.push_front(t2);
  result.push_front(t1);
  result.push_front(t0);
  return result;
}

} // namespace atermpp

#endif // MCRL2_ATERMPP_MAKE_LIST_H
