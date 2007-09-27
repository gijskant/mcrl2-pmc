// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/pbes_expression_builder.h
/// \brief Add your file description here.

#ifndef MCRL2_PBES_PBES_EXPRESSION_BUILDER_H
#define MCRL2_PBES_PBES_EXPRESSION_BUILDER_H

#include <stdexcept>
#include "mcrl2/pbes/pbes_expression.h"

namespace lps {

/// Visitor class for visiting the nodes of a pbes expression. During traversal
/// of the nodes, the expression is rebuilt from scratch.
// TODO: rebuilding expressions with ATerms is very expensive. So it is probably
// more efficient to  check if the children of a node have actually changed,
// before rebuilding it.
struct pbes_expression_builder
{
  virtual ~pbes_expression_builder()
  { }
  
  virtual pbes_expression visit_data_expression(const pbes_expression& /* e */, const data_expression& d)
  {
    return pbes_expression();
  }

  virtual pbes_expression visit_true(const pbes_expression& /* e */)
  {
    return pbes_expression();
  }

  virtual pbes_expression visit_false(const pbes_expression& /* e */)
  {
    return pbes_expression();
  }

  virtual pbes_expression visit_and(const pbes_expression& /* e */, const pbes_expression& /* left */, const pbes_expression& /* right */)
  {
    return pbes_expression();
  }

  virtual pbes_expression visit_or(const pbes_expression& /* e */, const pbes_expression& /* left */, const pbes_expression& /* right */)
  {
    return pbes_expression();
  }    

  virtual pbes_expression visit_forall(const pbes_expression& /* e */, const data_variable_list& /* variables */, const pbes_expression& /* expression */)
  {
    return pbes_expression();
  }

  virtual pbes_expression visit_exists(const pbes_expression& /* e */, const data_variable_list& /* variables */, const pbes_expression& /* expression */)
  {
    return pbes_expression();
  }

  virtual pbes_expression visit_propositional_variable(const pbes_expression& /* e */, const propositional_variable_instantiation& /* v */)
  {
    return pbes_expression();
  }
  
  /// Visits the nodes of the pbes expression, and calls the corresponding visit_<node>
  /// member functions. If the return value of a visit function equals pbes_expression(),
  /// the recursion in this node is continued automatically, otherwise the returned
  /// value is used for rebuilding the expression.
  pbes_expression visit(const pbes_expression& e)
  {
    using namespace pbes_expr;

    if (is_data(e)) {
      pbes_expression result = visit_data_expression(e, val(e));
      return (result == pbes_expression()) ? e : result;
    } else if (is_true(e)) {
      pbes_expression result = visit_true(e);
      return (result == pbes_expression()) ? e : result;
    } else if (is_false(e)) {
      pbes_expression result = visit_false(e);
      return (result == pbes_expression()) ? e : result;
    } else if (is_and(e)) {
      const pbes_expression& left  = lhs(e);
      const pbes_expression& right = rhs(e);
      pbes_expression result = visit_and(e, left, right);
      return (result == pbes_expression()) ? and_(visit(left), visit(right)) : result;
    } else if (is_or(e)) {
      const pbes_expression& left  = lhs(e);
      const pbes_expression& right = rhs(e);
      pbes_expression result = visit_or(e, left, right);
      return (result == pbes_expression()) ? or_(visit(left), visit(right)) : result;
    } else if (is_forall(e)) {
      const data_variable_list& qvars = quant_vars(e);
      const pbes_expression& qexpr = quant_expr(e);
      pbes_expression result = visit_forall(e, qvars, qexpr);
      return (result == pbes_expression()) ? forall(qvars, visit(qexpr)) : result;
    } else if (is_exists(e)) {
      const data_variable_list& qvars = quant_vars(e);
      const pbes_expression& qexpr = quant_expr(e);
      pbes_expression result = visit_exists(e, qvars, qexpr);
      return (result == pbes_expression()) ? exists(qvars, visit(qexpr)) : result;
    }
    else if(is_propositional_variable_instantiation(e)) {
      pbes_expression result = visit_propositional_variable(e, propositional_variable_instantiation(e));
      return (result == pbes_expression()) ? e : result;
    }
    else {
      throw std::runtime_error(std::string("error in pbes_expression_builder::visit() : unknown pbes expression ") + e.to_string());
      return pbes_expression();
    }
  }
};

} // namespace lps

#endif // MCRL2_PBES_PBES_EXPRESSION_BUILDER_H
