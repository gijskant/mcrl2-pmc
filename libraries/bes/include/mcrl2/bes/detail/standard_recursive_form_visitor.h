// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/bes/detail/standard_recursive_form_visitor.h
/// \brief add your file description here.

#ifndef MCRL2_BES_DETAIL_STANDARD_RECURSIVE_FORM_VISITOR_H
#define MCRL2_BES_DETAIL_STANDARD_RECURSIVE_FORM_VISITOR_H

#include "mcrl2/bes/boolean_expression_visitor.h"
#include "mcrl2/bes/boolean_equation.h"
#include "mcrl2/exception.h"
#include "mcrl2/atermpp/map.h"

namespace mcrl2 {

namespace bes {

namespace detail {

  enum standard_recursive_form_type
  {
    standard_recursive_form_both,
    standard_recursive_form_and,
    standard_recursive_form_or
  };

  typedef std::pair<boolean_expression, standard_recursive_form_type> standard_recursive_form_pair;

  struct standard_recursive_form_visitor: public boolean_expression_visitor<void>
  {
    typedef boolean_expression_visitor<void> super;
    typedef core::term_traits<boolean_expression> tr;

    /// \brief The fixpoint symbol of the current equation.
    fixpoint_symbol m_symbol;

    /// \brief A stack containing sub-terms.
    std::vector<standard_recursive_form_pair> m_expression_stack;

    /// \brief A vector containing generated equations.
    atermpp::vector<boolean_equation> m_equations;

    /// \brief Maps right hand sides of equations to their corresponding left hand side.
    atermpp::map<boolean_expression, boolean_variable> m_table;

    /// \brief The variable corresponding to true.
    boolean_variable m_true;

    /// \brief The variable corresponding to false.
    boolean_variable m_false;

    /// \brief Pops the stack and returns the popped element
    standard_recursive_form_pair pop()
    {
      standard_recursive_form_pair result = m_expression_stack.back();
      m_expression_stack.pop_back();
      return result;
    }

    /// \brief Pushes (first, second) on the stack.
    void push(const boolean_expression& first, standard_recursive_form_type second)
    {     
      m_expression_stack.push_back(standard_recursive_form_pair(first, second));
    }

    /// \brief Generates a fresh boolean variable.
    boolean_variable fresh_variable() const
    {
      // TODO: implement a decent solution for this
      static int index = 0;
      std::ostringstream out;
      out << "FVAR" << index++;
      return boolean_variable(out.str());
    }

    /// \brief Generates an equation var=expr for the expression expr (if it does not exist).
    /// \return The variable var.
    boolean_variable create_variable(const boolean_expression& expr, standard_recursive_form_type type)
    {
      atermpp::map<boolean_expression, boolean_variable>::iterator i = m_table.find(expr);
      if (i != m_table.end())
      {
        return i->second;
      }
      boolean_variable var = fresh_variable();
      m_table[expr] = var;
      if (type == standard_recursive_form_and)
      {
        m_equations.push_back(boolean_equation(m_symbol, var, expr));
      }
      else
      {
        m_equations.push_back(boolean_equation(m_symbol, var, expr));
      }
      return var;
    }

    /// \brief Constructor.
    /// Adds equations for true and false.
    standard_recursive_form_visitor()
    {
      m_true = fresh_variable();
      m_false = fresh_variable();
      m_equations.push_back(boolean_equation(fixpoint_symbol::nu(), m_true, tr::and_(m_true, m_true)));
      m_equations.push_back(boolean_equation(fixpoint_symbol::mu(), m_false, tr::and_(m_false, m_false)));
    }

    /// \brief Returns the top element of the expression stack, which is the result of the normalization.
    boolean_expression result() const
    {
      return m_expression_stack.back().first;
    }

    /// \brief Returns the generated equations.
    const atermpp::vector<boolean_equation>& equations() const
    {
      return m_equations;
    }

    /// \brief Visit true node
    /// \param e A term
    /// \return The result of visiting the node
    bool visit_true(const boolean_expression& /* e */)
    {
      push(m_true, standard_recursive_form_both);
      return super::continue_recursion;
    }

    /// \brief Visit false node
    /// \param e A term
    /// \return The result of visiting the node
    bool visit_false(const boolean_expression& /* e */)
    {
      push(m_false, standard_recursive_form_both);
      return super::continue_recursion;
    }

    /// \brief Visit propositional_variable node
    /// \param e A term
    /// \param X A propositional variable
    /// \return The result of visiting the node
    bool visit_var(const boolean_expression& /* e */, const boolean_variable& X)
    {
      push(X, standard_recursive_form_both);
      return super::continue_recursion;
    }

    /// \brief Leave not node
    void leave_not()
    {
      throw mcrl2::runtime_error("negation is not supported in standard recursive form algorithm");
    }

    /// \brief Leave and node
    void leave_and()
    {
      standard_recursive_form_pair right = pop();
      standard_recursive_form_pair left = pop();
      if (left.second == standard_recursive_form_or)
      {
        left.first = create_variable(left.first, standard_recursive_form_or);
      }
      if (right.second == standard_recursive_form_or)
      {
        right.first = create_variable(right.first, standard_recursive_form_or);
      }
      push(tr::and_(left.first, right.first), standard_recursive_form_and);
    }

    /// \brief Leave or node
    void leave_or()
    {
      standard_recursive_form_pair right = pop();
      standard_recursive_form_pair left = pop();
      if (left.second == standard_recursive_form_and)
      {
        left.first = create_variable(left.first, standard_recursive_form_and);
      }
      if (right.second == standard_recursive_form_and)
      {
        right.first = create_variable(right.first, standard_recursive_form_and);
      }
      push(tr::or_(left.first, right.first), standard_recursive_form_or);
    }

    /// \brief Leave imp node
    void leave_imp()
    {
      throw mcrl2::runtime_error("implication is not supported in standard recursive form algorithm");
    }
    
    /// \brief Visit an equation
    void visit_equation(const boolean_equation& eq)
    {
      m_symbol = eq.symbol();
      super::visit(eq.formula());
      standard_recursive_form_pair p = pop();
      m_equations.push_back(boolean_equation(eq.symbol(), eq.variable(), p.first));
      m_table[p.first] = eq.variable();
    }
  };

} // namespace detail

} // namespace bes

} // namespace mcrl2

#endif // MCRL2_BES_DETAIL_STANDARD_RECURSIVE_FORM_VISITOR_H
