// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/monotonicity.h
/// \brief Functions for computing monotonicity of pbes data types.

#ifndef MCRL2_PBES_MONOTONICITY_H
#define MCRL2_PBES_MONOTONICITY_H

#include "mcrl2/pbes/detail/pbes_monotonicity_visitor.h"
#include "mcrl2/pbes/pbes.h"

namespace mcrl2 {

namespace pbes_system {

  /// \brief Returns true if the pbes expression is monotonous.
  inline
  bool is_monotonous(const pbes_expression& x)
  {
    detail::pbes_monotonicity_visitor<pbes_expression> visitor;
    visitor.visit(x);
    return visitor.result();
  }

  /// \brief Returns true if the pbes equation is monotonous.
  inline
  bool is_monotonous(const pbes_equation& e)
  {
    return is_monotonous(e.formula());
  }

  /// \brief Returns true if the pbes is monotonous.
  template <typename Container>
  bool is_monotonous(const pbes<Container>& p)
  {
    for (typename Container::const_iterator i = p.equations().begin(); i != p.equations().end(); ++i)
    {
      if (!is_monotonous(*i))
      {
        return false;
      }
    }
    return true;
  }

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_MONOTONICITY_H
