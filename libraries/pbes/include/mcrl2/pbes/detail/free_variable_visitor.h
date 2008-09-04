// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file free_variable_visitor.h
/// \brief Add your file description here.

#ifndef MCRL2_PBES_DETAIL_FREE_VARIABLE_VISITOR_H
#define MCRL2_PBES_DETAIL_FREE_VARIABLE_VISITOR_H

#include "mcrl2/data/find.h"
#include "mcrl2/data/utility.h"
#include "mcrl2/pbes/pbes_expression_visitor.h"

namespace mcrl2 {

namespace pbes_system {

namespace detail {

template <typename Term>
struct free_variable_visitor: public pbes_expression_visitor<Term>
{
  typedef pbes_expression_visitor<Term> super;
  typedef typename super::term_type term_type;
  typedef typename super::data_term_type data_term_type;
  
  data::data_variable_list bound_variables;
  std::vector<data::data_variable_list> quantifier_stack;
  std::set<data::data_variable> result;
  bool search_propositional_variables;

  free_variable_visitor(bool search_propositional_variables_ = true)
    : search_propositional_variables(search_propositional_variables_)
  {}

  free_variable_visitor(data::data_variable_list bound_variables_, bool search_propositional_variables_ = true)
    : bound_variables(bound_variables_), search_propositional_variables(search_propositional_variables_)
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

  bool visit_forall(const term_type& e, const data::data_variable_list& v, const term_type&)
  {
    push(v);
    return true;
  }

  void leave_forall()
  {
    pop();
  }

  bool visit_exists(const term_type& e, const data::data_variable_list& v, const term_type&)
  {
    push(v);
    return true;
  }

  void leave_exists()
  {
    pop();
  }

  bool visit_propositional_variable(const term_type& e, const propositional_variable_instantiation& v)
  {
    if (search_propositional_variables)
    {
      std::set<data::data_variable> variables = data::find_all_data_variables(v.parameters());
      for (std::set<data::data_variable>::iterator i = variables.begin(); i != variables.end(); ++i)
      {
        if (!is_bound(*i))
        {
          result.insert(*i);
        }
      }
    }
    return true;
  }

  bool visit_data_expression(const term_type& e, const data_term_type& d)
  {
    std::set<data::data_variable> variables = data::find_all_data_variables(d);
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

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_DETAIL_FREE_VARIABLE_VISITOR_H

