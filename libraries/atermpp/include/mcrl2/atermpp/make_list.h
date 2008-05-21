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

#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_list.h"

namespace atermpp
{
  /// Makes a list with a fixed number of arguments.
  ///
  template <typename T0>
  aterm_list make_list(T0 t0)
  {
    return aterm_list(ATmakeList1(aterm_traits<T0>::term(t0)));
  }
  
  /// Makes a list with a fixed number of arguments.
  ///
  template <typename T0, typename T1>
  aterm_list make_list(T0 t0, T1 t1)
  {
    return aterm_list(ATmakeList2(aterm_traits<T0>::term(t0), aterm_traits<T1>::term(t1)));
  }
  
  /// Makes a list with a fixed number of arguments.
  ///
  template <typename T0, typename T1, typename T2>
  aterm_list make_list(T0 t0, T1 t1, T2 t2)
  {
    return aterm_list(ATmakeList3(aterm_traits<T0>::term(t0), aterm_traits<T1>::term(t1), aterm_traits<T2>::term(t2)));
  }

  /// Makes a list with a fixed number of arguments.
  ///
  template <typename T0, typename T1, typename T2, typename T3>
  aterm_list make_list(T0 t0, T1 t1, T2 t2, T3 t3)
  {
    return aterm_list(ATmakeList4(aterm_traits<T0>::term(t0), aterm_traits<T1>::term(t1), aterm_traits<T2>::term(t2), aterm_traits<T3>::term(t3)));
  }

} // namespace atermpp

#endif // MCRL2_ATERMPP_MAKE_LIST_H
