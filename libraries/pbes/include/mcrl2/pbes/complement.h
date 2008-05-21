// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file complement.h
/// \brief The complement function for pbes expressions.
//
// Comp ( val(b) ) = val (! b)
// 
// Comp ( X(d) ) = "ABORT, should not happen"
// 
// Comp ( PbesAnd (f, g) ) = PbesOr (Comp (f), Comp (g) )
// 
// Comp ( PbesOr (f, g) ) = PbesAnd (Comp (f), Comp (g) )
// 
// Comp (PbesForAll (f) ) = PbesExists (Comp (f) )
// 
// Comp (PbesExists (f) ) = PbesForall (Comp (f) )
// 
// Comp (Comp (f) ) = f

#ifndef MCRL2_PBES_COMPLEMENT_H
#define MCRL2_PBES_COMPLEMENT_H

#include <stdexcept>
#include "mcrl2/pbes/pbes_expression_builder.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/iterator/transform_iterator.hpp>
#include "mcrl2/data/detail/data_functional.h"
#include "mcrl2/data/detail/sequence_substitution.h"
#include "mcrl2/data/set_identifier_generator.h"

namespace mcrl2 {

namespace pbes_system {

/// Visitor that pushes a negation in a PBES expression as far as possible
/// inwards towards a data expression.
struct complement_builder: public pbes_expression_builder
{
  /// \overload
  ///
  pbes_expression visit_data_expression(const pbes_expression& /* e */, const data::data_expression& d)
  {
    return data::data_expr::not_(d);
  }

  /// \overload
  ///
  pbes_expression visit_true(const pbes_expression& /* e */)
  {
    using namespace pbes_expr_optimized;
    return false_();
  }

  /// \overload
  ///
  pbes_expression visit_false(const pbes_expression& /* e */)
  {
    using namespace pbes_expr_optimized;
    return true_();
  }

  /// \overload
  ///
  pbes_expression visit_and(const pbes_expression& /* e */, const pbes_expression& left, const pbes_expression& right)
  {
    using namespace pbes_expr_optimized;
    return or_(visit(left), visit(right));
  }

  /// \overload
  ///
  pbes_expression visit_or(const pbes_expression& /* e */, const pbes_expression& left, const pbes_expression& right)
  {
    using namespace pbes_expr_optimized;
    return and_(visit(left), visit(right));
  }    

  /// \overload
  ///
  pbes_expression visit_forall(const pbes_expression& /* e */, const data::data_variable_list& variables, const pbes_expression& expression)
  {
    using namespace pbes_expr_optimized;
    return exists(variables, visit(expression));
  }

  /// \overload
  ///
  pbes_expression visit_exists(const pbes_expression& /* e */, const data::data_variable_list& variables, const pbes_expression& expression)
  {
    using namespace pbes_expr_optimized;
    return forall(variables, visit(expression));
  }

  /// \overload
  ///
  pbes_expression visit_propositional_variable(const pbes_expression& /* e */, const propositional_variable_instantiation& v)
  {
    throw std::runtime_error(std::string("complement_builder error: unexpected propositional variable encountered ") + pp(v));
    return pbes_expression();
  }
};

/// Returns the expression obtained by pushing the negations in the pbes
/// expression as far as possible inwards towards a data expression.
inline
pbes_expression complement(const pbes_expression p)
{
  return complement_builder().visit(p);
}

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_COMPLEMENT_H
