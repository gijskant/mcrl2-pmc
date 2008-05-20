// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING).
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file free_variables.h
/// \brief Add your file description here.

#ifndef MCRL2_MODAL_FREE_VARIABLES_H
#define MCRL2_MODAL_FREE_VARIABLES_H

#include <set>
#include <vector>
#include "mcrl2/modal_formula/state_formula_visitor.h"
#include "mcrl2/data/data.h"
#include "mcrl2/data/find.h"

namespace mcrl2 {

namespace modal {

namespace state_frm {

struct free_variable_visitor: public state_formula_visitor
{
  data::data_variable_list bound_variables;
  std::vector<data::data_variable_list> quantifier_stack;
  std::set<data::data_variable> result;

  free_variable_visitor()
  {}

  free_variable_visitor(data::data_variable_list bound_variables_)
    : bound_variables(bound_variables_)
  {}

  // returns true if v is an element of bound_variables or quantifier_stack
  bool is_bound(const data::data_variable& v) const
  {
    if (std::find(bound_variables.begin(), bound_variables.end(), v) != bound_variables.end())
    {
      return true;
    }
    for (std::vector<data::data_variable_list>::const_iterator i = quantifier_stack.begin(); i != quantifier_stack.end(); ++i)
    {
      if (std::find(i->begin(), i->end(), v) != i->end())
      {
        return true;
      }
    }
    return false;
  }

  void push(const data::data_variable_list& v)
  {
    quantifier_stack.push_back(v);
  }

  void pop()
  {
    quantifier_stack.pop_back();
  }

  bool visit_forall(const state_formula& e, const data::data_variable_list& v, const state_formula&)
  {
    push(v);
    return true;
  }

  void leave_forall()
  {
    pop();
  }

  bool visit_exists(const state_formula& e, const data::data_variable_list& v, const state_formula&)
  {
    push(v);
    return true;
  }

  void leave_exists()
  {
    pop();
  }

  bool visit_var(const state_formula& /* e */, const core::identifier_string& /* n */, const data::data_expression_list& l)
  {
    std::set<data::data_variable> variables = find_all_data_variables(l);
    for (std::set<data::data_variable>::iterator i = variables.begin(); i != variables.end(); ++i)
    {
      if (!is_bound(*i))
      {
        result.insert(*i);
      }
    }
    return true;
  }

  bool visit_data_expression(const state_formula& /* e */, const data::data_expression& d)
  {
    std::set<data::data_variable> variables = find_all_data_variables(d);
    for (std::set<data::data_variable>::iterator i = variables.begin(); i != variables.end(); ++i)
    {
      if (!is_bound(*i))
      {
        result.insert(*i);
      }
    }
    return true;
  }
};

} // namespace state_frm

/// Computes the free variables that occur in the state formula f.
inline
std::set<data::data_variable> compute_free_state_formula_variables(const state_formula& f)
{
  state_frm::free_variable_visitor visitor;
  visitor.visit(f);
  return visitor.result;
}

} // namespace modal

} // namespace mcrl2

#endif // MCRL2_MODAL_FREE_VARIABLES_H
