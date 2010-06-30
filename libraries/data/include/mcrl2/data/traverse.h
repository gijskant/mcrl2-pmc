// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/traverse.h
/// \brief add your file description here.

#ifndef MCRL2_DATA_TRAVERSE_H
#define MCRL2_DATA_TRAVERSE_H

#include "mcrl2/data/detail/find.h"
#include "mcrl2/data/detail/traverser.h"
#include "mcrl2/data/detail/sort_traverser.h"

namespace mcrl2 {

namespace data {

  /// \brief Traverses all sort expressions in the data type o, and writes them
  /// to the output iterator dest.
  template <typename Object, typename OutIter>
  void traverse_sort_expressions(const Object& o, OutIter dest)
  {
    data::detail::make_find_helper< data::sort_expression, data::detail::sort_traverser >(dest)(o);
  }

  /// \brief Traverses all variables in the data type o, and writes them
  /// to the output iterator dest.
  template <typename Object, typename OutIter>
  void traverse_variables(const Object& o, OutIter dest)
  {
    data::detail::make_find_helper< data::variable, data::detail::traverser >(dest)(o);
  }

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_TRAVERSE_H
