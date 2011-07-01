// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/process/detail/linear_process_expression_traverser.h
/// \brief add your file description here.

#ifndef MCRL2_PROCESS_DETAIL_LINEAR_PROCESS_EXPRESSION_TRAVERSER_H
#define MCRL2_PROCESS_DETAIL_LINEAR_PROCESS_EXPRESSION_TRAVERSER_H

#include "mcrl2/process/process_specification.h"
#include "mcrl2/process/traverser.h"
#include "mcrl2/process/print.h"
#include "mcrl2/process/detail/is_linear.h"

namespace mcrl2
{

namespace process
{

namespace detail
{

/// \brief Checks if a process equation is linear.
/// Use the is_linear() member function for this.
struct linear_process_expression_traverser: public process_expression_traverser<linear_process_expression_traverser>
{
  typedef process_expression_traverser<linear_process_expression_traverser> super;
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

  /// \brief The process equation that is checked.
  process_equation eqn;

  /// \brief Exception that is thrown by linear_process_expression_traverser.
  struct non_linear_process
  {
    std::string msg;

    non_linear_process(const std::string& s)
      : msg(s)
    {}
  };

  /// \brief Returns true if the argument is a process instance
  /// \param x A process expression
  /// \return True if the argument is a process instance
  bool is_process(const process_expression& x)
  {
    return is_process_instance(x)
           || is_process_instance_assignment(x)
           ;
  }

  /// \brief Returns true if the argument is a deadlock
  /// \param x A process expression
  /// \return True if the argument is a deadlock
  bool is_timed_deadlock(const process_expression& x)
  {
    return is_delta(x)
           || is_at(x)
           ;
  }

  /// \brief Returns true if the argument is a multi-action
  /// \param x A process expression
  /// \return True if the argument is a multi-action
  bool is_multiaction(const process_expression& x)
  {
    return is_tau(x)
           || is_sync(x)
           || lps::is_action(x)
           ;
  }

  /// \brief Returns true if the argument is a multi-action
  /// \param x A process expression
  /// \return True if the argument is a multi-action
  bool is_timed_multiaction(const process_expression& x)
  {
    return is_at(x)
           || is_multiaction(x);
  }

  /// \brief Returns true if the argument is an action prefix
  /// \param x A process expression
  /// \return True if the argument is an action prefix
  bool is_action_prefix(const process_expression& x)
  {
    return is_seq(x)
           || is_timed_multiaction(x);
  }

  /// \brief Returns true if the argument is a conditional deadlock
  /// \param x A process expression
  /// \return True if the argument is a conditional deadlock
  bool is_conditional_deadlock(const process_expression& x)
  {
    return is_if_then(x)
           || is_timed_deadlock(x);
  }

  /// \brief Returns true if the argument is a conditional action prefix.
  /// \param x A process expression
  /// \return True if the argument is a conditional action prefix.
  bool is_conditional_action_prefix(const process_expression& x)
  {
    return is_if_then(x)
           || is_action_prefix(x);
  }

  /// \brief Returns true if the argument is an alternative composition
  /// \param x A process expression
  /// \return True if the argument is an alternative composition
  bool is_alternative(const process_expression& x)
  {
    return is_sum(x)
           || is_conditional_action_prefix(x)
           || is_conditional_deadlock(x)
           ;
  }

  /// \brief Returns true if the argument is a linear process
  /// \param x A process expression
  /// \return True if the argument is a linear process
  bool is_linear_process_term(const process_expression& x)
  {
    return is_choice(x)
           || is_alternative(x)
           ;
  }

  // TODO: check if this function is needed
  /// \brief Visit process_instance node
  /// \return The result of visiting the node
  /// \param x A process instance
  void enter(const process_instance& x)
  {
    if (!detail::check_process_instance(eqn, x))
    {
      throw non_linear_process(process::pp(x) + " is not a valid process instance");
    }
  }

  /// \brief Visit process_instance_assignment node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param pi A process identifier
  /// \param v A sequence of assignments to data variables
  void enter(const process_instance_assignment& x)
  {
    if (!detail::check_process_instance_assignment(eqn, x))
    {
      throw non_linear_process(process::pp(x) + " is not a valid process instance assignment");
    }
  }

  /// \brief Visit sum node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param v A sequence of data variables
  /// \param right A process expression
  void enter(const sum& x)
  {
    if (!is_alternative(x.operand()))
    {
      throw non_linear_process(process::pp(x.operand()) + " is not an alternative expression");
    }
  }

  /// \brief Visit block node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param s A sequence of identifiers
  /// \param right A process expression
  void enter(const block& x)
  {
    throw non_linear_process("block expression " + process::pp(x) + " encountered");
  }

  /// \brief Visit hide node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param s A sequence of identifiers
  /// \param right A process expression
  void enter(const hide& x)
  {
    throw non_linear_process("hide expression " + process::pp(x) + " encountered");
  }

  /// \brief Visit rename node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param r A sequence of rename expressions
  /// \param right A process expression
  void enter(const rename& x)
  {
    throw non_linear_process("rename expression " + process::pp(x) + " encountered");
  }

  /// \brief Visit comm node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param c A sequence of communication expressions
  /// \param right A process expression
  void enter(const comm& x)
  {
    throw non_linear_process("comm expression " + process::pp(x) + " encountered");
  }

  /// \brief Visit allow node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param s A sequence of multi-action names
  /// \param right A process expression
  void enter(const allow& x)
  {
    throw non_linear_process("allow expression " + process::pp(x) + " encountered");
  }

  /// \brief Visit sync node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void enter(const sync& x)
  {
    if (!is_multiaction(x.left()) || !is_multiaction(x.right()))
    {
      if (!is_multiaction(x.left()))
      {
        throw non_linear_process(process::pp(x.left()) + " is not a multi action");
      }
      else
      {
        throw non_linear_process(process::pp(x.right()) + " is not a multi action");
      }
    }
  }

  /// \brief Visit at node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param d A data expression
  void enter(const at& x)
  {
    if (!is_multiaction(x.operand()) && !is_delta(x.operand()))
    {
      throw non_linear_process(process::pp(x.operand()) + " is not a multi action and not a deadlock");
    }
  }

  /// \brief Visit seq node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void enter(const seq& x)
  {
    if (!is_timed_multiaction(x.left()) || !is_process(x.right()))
    {
      throw non_linear_process(process::pp(x.left()) + " is not a timed multi action and not a process");
    }
    if (is_process_instance(x.right()))
    {
      process_instance q = x.right();
      if (q.identifier() != eqn.identifier())
      {
        throw non_linear_process(process::pp(q) + " has an unexpected identifier");
      }
    }
    else if (is_process_instance_assignment(x.right()))
    {
      process_instance_assignment q = x.right();
      if (q.identifier() != eqn.identifier())
      {
        throw non_linear_process(process::pp(q) + " has an unexpected identifier");
      }
    }
    else
    {
      std::cerr << "seq right hand side: " << process::pp(x.right()) << std::endl;
      throw std::runtime_error("unexpected error in visit_seq");
    }
  }

  /// \brief Visit if_then node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param d A data expression
  /// \param right A process expression
  void enter(const if_then& x)
  {
    if (!is_action_prefix(x.then_case()) && !is_timed_deadlock(x.then_case()))
    {
      throw non_linear_process(process::pp(x) + " is not an action prefix and not a timed deadlock");
    }
  }

  /// \brief Visit if_then_else node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param d A data expression
  /// \param left A process expression
  /// \param right A process expression
  void enter(const if_then_else& x)
  {
    throw non_linear_process("if then else expression " + process::pp(x) + " encountered");
  }

  /// \brief Visit bounded_init node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void enter(const bounded_init& x)
  {
    throw non_linear_process("bounded init expression " + process::pp(x) + " encountered");
  }

  /// \brief Visit merge node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void enter(const merge& x)
  {
    throw non_linear_process("merge expression " + process::pp(x) + " encountered");
  }

  /// \brief Visit left_merge node
  /// \return The result of visiting the node
  /// \param x A process expression
  /// \param left A process expression
  /// \param right A process expression
  void enter(const left_merge& x)
  {
    throw non_linear_process("left merge expression " + process::pp(x) + " encountered");
  }

  /// \brief Returns true if the process equation e is linear.
  /// \param e A process equation
  /// \return True if the process equation e is linear.
  bool is_linear(const process_equation& e, bool verbose = false)
  {
    eqn = e;
    try
    {
      (*this)(e.expression());
    }
    catch (non_linear_process& p)
    {
      if (verbose)
      {
        std::clog << p.msg << std::endl;
      }
      return false;
    }
    return true;
  }
};

} // namespace detail

} // namespace process

} // namespace mcrl2

#endif // MCRL2_PROCESS_DETAIL_LINEAR_PROCESS_EXPRESSION_TRAVERSER_H
