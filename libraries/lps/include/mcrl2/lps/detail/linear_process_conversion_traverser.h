// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/detail/linear_process_conversion_traverser.h
/// \brief add your file description here.

#ifndef MCRL2_LPS_DETAIL_LINEAR_PROCESS_CONVERSION_TRAVERSER_H
#define MCRL2_LPS_DETAIL_LINEAR_PROCESS_CONVERSION_TRAVERSER_H

#include <stdexcept>
#include "mcrl2/exception.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/process/process_specification.h"
#include "mcrl2/process/traverser.h"
#include "mcrl2/process/detail/is_linear.h"

namespace mcrl2
{

namespace process
{

namespace detail
{

/// \brief Converts a process expression into linear process format.
/// Use the \p convert member functions for this.
struct linear_process_conversion_traverser: public process_expression_traverser<linear_process_conversion_traverser>
{
  typedef process_expression_traverser<linear_process_conversion_traverser> super;
  using super::enter;
  using super::leave;
  using super::operator();

#if BOOST_MSVC
  template <typename Container >
  void operator()(Container const& a)
  {
    super::operator()(a);
  }
#endif

  /// \brief The result of the conversion.
  lps::action_summand_vector m_action_summands;

  /// \brief The result of the conversion.
  lps::deadlock_summand_vector m_deadlock_summands;

  /// \brief The process equation that is checked.
  process_equation m_equation;

  /// \brief Contains intermediary results.
  data::variable_list m_sum_variables;

  /// \brief Contains intermediary results.
  data::assignment_list m_next_state;

  /// \brief Contains intermediary results.
  lps::multi_action m_multi_action;

  /// \brief Contains intermediary results.
  lps::deadlock m_deadlock;

  /// \brief True if m_deadlock was changed.
  bool m_deadlock_changed;

  /// \brief True if m_multi_action was changed.
  bool m_multi_action_changed;

  /// \brief True if m_next_state was changed.
  bool m_next_state_changed;

  /// \brief Contains intermediary results.
  data::data_expression m_condition;

  /// \brief Contains intermediary results.
  lps::action_summand m_action_summand;

  /// \brief Contains intermediary results.
  lps::deadlock_summand m_deadlock_summand;

  /// \brief Exception that is thrown to denote that the process is not linear.
  struct non_linear_process
  {
    process_expression expr;

    non_linear_process(const process_expression& p)
      : expr(p)
    {}
  };

  /// \brief Clears the current summand
  void clear_summand()
  {
    m_sum_variables = data::variable_list();
    m_deadlock = lps::deadlock();
    m_deadlock_changed = false;
    m_multi_action = lps::multi_action();
    m_multi_action_changed = false;
    m_condition = data::sort_bool::true_();
    m_next_state = data::assignment_list();
    m_next_state_changed = false;
  }

  /// \brief Adds a summand to the result
  void add_summand()
  {
    if (m_multi_action_changed)
    {
      if (m_next_state_changed)
      {
        m_action_summands.push_back(lps::action_summand(m_sum_variables, m_condition, m_multi_action, m_next_state));
        clear_summand();
      }
      else
      {
        throw mcrl2::runtime_error("Error in linear_process_conversion_traverser::convert: encountered a multi action without process reference");
      }
    }
    else if (m_deadlock_changed)
    {
      m_deadlock_summands.push_back(lps::deadlock_summand(m_sum_variables, m_condition, m_deadlock));
      clear_summand();
    }
// std::cout << "adding summand" << m_multi_action_changed << m_deadlock_changed << "\n" << core::pp(m_summand) << std::endl;
  }

  /// \brief Visit delta node
  /// \return The result of visiting the node
  /// \param x A process expression
  void leave(const delta& /* x */)
  {
    m_deadlock = lps::deadlock();
    m_deadlock_changed = true;
// std::cout << "adding deadlock\n" << m_deadlock.to_string() << std::endl;
  }

  /// \brief Visit tau node
  /// \return The result of visiting the node
  /// \param x A process expression
  void leave(const tau& /* x */)
  {
    m_multi_action = lps::multi_action();
    m_multi_action_changed = true;
// std::cout << "adding multi action tau\n" << m_multi_action.to_string() << std::endl;
  }

  /// \brief Visit action node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param l An action label
  /// \param v A sequence of data expressions
  void leave(const lps::action& x)
  {
    action a(x.label(), x.arguments());
    m_multi_action = lps::multi_action(a);
    m_multi_action_changed = true;
// std::cout << "adding multi action\n" << m_multi_action.to_string() << std::endl;
  }

  /// \brief Visit sum node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param v A sequence of data variables
  /// \param right A process expression
  void leave(const sum& x)
  {
    m_sum_variables = m_sum_variables + x.bound_variables();
// std::cout << "adding sum variables\n" << core::pp(v) << std::endl;
  }

  /// \brief Visit block node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param s A sequence of identifiers
  /// \param right A process expression
  void leave(const block& x)
  {
    throw non_linear_process(x);
  }

  /// \brief Visit hide node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param s A sequence of identifiers
  /// \param right A process expression
  void leave(const hide& x)
  {
    throw non_linear_process(x);
  }

  /// \brief Visit rename node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param r A sequence of rename expressions
  /// \param right A process expression
  void leave(const rename& x)
  {
    throw non_linear_process(x);
  }

  /// \brief Visit comm node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param c A sequence of communication expressions
  /// \param right A process expression
  void leave(const comm& x)
  {
    throw non_linear_process(x);
  }

  /// \brief Visit allow node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param s A sequence of multi-action names
  /// \param right A process expression
  void leave(const allow& x)
  {
    throw non_linear_process(x);
  }

  /// \brief Visit sync node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void operator()(const sync& x)
  {
    (*this)(x.left());
    lps::multi_action l = m_multi_action;
    (*this)(x.right());
    lps::multi_action r = m_multi_action;
    m_multi_action = l + r;
    m_multi_action_changed = true;
// std::cout << "adding multi action\n" << m_multi_action.to_string() << std::endl;
  }

  /// \brief Visit at node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param d A data expression
  void leave(const at& x)
  {
    if (is_delta(x))
    {
      m_deadlock.time() = x.time_stamp();
// std::cout << "adding deadlock\n" << m_deadlock.to_string() << std::endl;
    }
    else
    {
      m_multi_action.time() = x.time_stamp();
// std::cout << "adding multi action\n" << m_multi_action.to_string() << std::endl;
    }
  }

  /// \brief Visit seq node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void operator()(const seq& x)
  {
    (*this)(x.left());

    // Check 1) The expression right must be a process instance or a process assignment
    if (is_process_instance(x.right()))
    {
      process_instance p = x.right();
      // Check 2) The process equation and and the process instance must match
      if (!detail::check_process_instance(m_equation, p))
      {
        std::clog << "seq right hand side: " << core::pp(x.right()) << std::endl;
        throw mcrl2::runtime_error("Error in linear_process_conversion_traverser::convert: seq expression encountered that does not match the process equation");
      }
      m_next_state = data::make_assignment_list(m_equation.formal_parameters(), p.actual_parameters());
      m_next_state_changed = true;
    }
    else if (is_process_instance_assignment(x.right()))
    {
      process_instance_assignment p = x.right();
      // Check 2) The process equation and and the process instance assignment must match
      if (!detail::check_process_instance_assignment(m_equation, p))
      {
        std::clog << "seq right hand side: " << core::pp(x.right()) << std::endl;
        throw mcrl2::runtime_error("Error in linear_process_conversion_traverser::convert: seq expression encountered that does not match the process equation");
      }
      m_next_state = p.assignments(); // TODO: check if this is correct
      m_next_state_changed = true;
    }
    else
    {
      std::clog << "seq right hand side: " << core::pp(x.right()) << std::endl;
      throw mcrl2::runtime_error("Error in linear_process_conversion_traverser::convert: seq expression encountered with an unexpected right hand side");
    }

// std::cout << "adding next state\n" << core::pp(m_next_state) << std::endl;
  }

  /// \brief Visit if_then node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param d A data expression
  /// \param right A process expression
  void leave(const if_then& x)
  {
    m_condition = x.condition();
// std::cout << "adding condition\n" << core::pp(m_condition) << std::endl;
  }

  /// \brief Visit if_then_else node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param d A data expression
  /// \param left A process expression
  /// \param right A process expression
  void leave(const if_then_else& x)
  {
    throw non_linear_process(x);
  }

  /// \brief Visit bounded_init node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void leave(const bounded_init& x)
  {
    throw non_linear_process(x);
  }

  /// \brief Visit merge node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void leave(const merge& x)
  {
    throw non_linear_process(x);
  }

  /// \brief Visit left_merge node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void leave(const left_merge& x)
  {
    throw non_linear_process(x);
  }

  /// \brief Visit choice node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void operator()(const choice& x)
  {
    (*this)(x.left());
    if (!is_choice(x.left()))
    {
      add_summand();
    }
    (*this)(x.right());
    if (!is_choice(x.right()))
    {
      add_summand();
    }
  }

  /// \brief Returns true if the process equation e is linear.
  /// \param e A process equation
  void convert(const process_equation& /* e */)
  {
    clear_summand();
    (*this)(m_equation.expression());
    add_summand(); // needed if it is not a choice
  }

  /// \brief Converts a process_specification into a specification.
  /// Throws \p non_linear_process if a non-linear sub-expression is encountered.
  /// Throws \p mcrl2::runtime_error in the following cases:
  /// \li The number of equations is not equal to one
  /// \li The initial process is not a process instance, or it does not match with the equation
  /// \li A sequential process is found with a right hand side that is not a process instance,
  /// or it doesn't match the equation
  /// \param p A process specification
  /// \return The converted specification
  lps::specification convert(const process_specification& p)
  {
    m_action_summands.clear();
    m_deadlock_summands.clear();

    // Check 1) The number of equations must be one
    if (p.equations().size() != 1)
    {
      throw mcrl2::runtime_error("Error in linear_process_conversion_traverser::convert: the number of process equations is not equal to 1!");
    }
    m_equation = p.equations().front();

    lps::process_initializer proc_init;

    if (is_process_instance(p.init()))
    {
      process_instance init = p.init();
      if (!check_process_instance(m_equation, init))
      {
        throw mcrl2::runtime_error("Error in linear_process_conversion_traverser::convert: the initial process does not match the process equation");
      }
      proc_init = lps::process_initializer(data::make_assignment_list(m_equation.formal_parameters(), init.actual_parameters()));
    }
    else if (is_process_instance_assignment(p.init()))
    {
      process_instance_assignment init = p.init();
      if (!check_process_instance_assignment(m_equation, init))
      {
        throw mcrl2::runtime_error("Error in linear_process_conversion_traverser::convert: the initial process does not match the process equation");
      }
      proc_init = lps::process_initializer(init.assignments());
    }
    else
    {
      throw mcrl2::runtime_error("Error in linear_process_conversion_traverser::convert: the initial process has an unexpected value");
    }

    // Do the conversion
    convert(m_equation);

    lps::linear_process proc(m_equation.formal_parameters(), m_deadlock_summands, m_action_summands);
    return lps::specification(p.data(), p.action_labels(), p.global_variables(), proc, proc_init);
  }
};

} // namespace detail

} // namespace process

} // namespace mcrl2

#endif // MCRL2_LPS_DETAIL_LINEAR_PROCESS_CONVERSION_TRAVERSER_H
