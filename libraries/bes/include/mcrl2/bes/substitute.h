// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/bes/substitute.h
/// \brief add your file description here.

#ifndef MCRL2_BES_SUBSTITUTE_H
#define MCRL2_BES_SUBSTITUTE_H

#include <functional>
#include "mcrl2/bes/boolean_expression.h"
#include "mcrl2/bes/builder.h"

namespace mcrl2 {

namespace bes {

struct boolean_variable_substitution: public std::unary_function<boolean_variable, boolean_expression>
{
  const boolean_variable& v;
  const boolean_expression& phi;

  boolean_variable_substitution(const boolean_variable& v_, const boolean_expression& phi_)
    : v(v_), phi(phi_)
  {}

  boolean_expression operator()(const boolean_variable& x) const
  {
    if (x != v)
    {
      return x;
    }
    return phi;
  }
};

/// \brief Applies the substitution \p X := \p phi to the boolean expression \p t.
/// \param t A boolean expression
/// \param X A boolean variable
/// \param phi A boolean expression
/// \return The result of the substitution.
template <typename Substitution>
boolean_expression substitute_boolean_variables(const boolean_expression& x, Substitution sigma)
{
  return core::make_update_apply_builder<boolean_expression_builder>(sigma)(x);
}

} // namespace bes

} // namespace mcrl2

#endif // MCRL2_BES_SUBSTITUTE_H
