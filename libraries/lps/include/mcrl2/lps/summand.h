// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/summand.h
/// \brief The class summand.

#ifndef MCRL2_LPS_SUMMAND_H
#define MCRL2_LPS_SUMMAND_H

#include <string>
#include <cassert>
#include <algorithm>
#include <iterator>
#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_access.h"
#include "mcrl2/atermpp/aterm_list.h"
#include "mcrl2/atermpp/algorithm.h"
#include "mcrl2/atermpp/utility.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/data/data_expression.h"
#include "mcrl2/data/print.h"
#include "mcrl2/data/real.h"
#include "mcrl2/data/bool.h"
#include "mcrl2/data/substitute.h"
#include "mcrl2/data/detail/assignment_functional.h"
#include "mcrl2/data/detail/sequence_algorithm.h"
#include "mcrl2/lps/action.h"
#include "mcrl2/lps/deadlock.h"
#include "mcrl2/lps/multi_action.h"

namespace mcrl2 {

namespace lps {

/// \brief LPS summand.
// <LinearProcessSummand>   ::= LinearProcessSummand(<DataVarId>*, <DataExpr>, <MultActOrDelta>,
//                    <DataExprOrNil>, <Assignment>*)
//<MultActOrDelta>
//               ::= <MultAct>
//                 | Delta
// <MultAct>      ::= MultAct(<ParamId>*)                                   (- tc)
//                  | MultAct(<Action>*)                                    (+ tc)
class summand: public atermpp::aterm_appl
{
  protected:
    /// \brief The summation variables of the summand
    data::variable_list m_summation_variables;

    /// \brief The condition of the summand
    data::data_expression m_condition;

    /// \brief If m_delta is true the summand is a delta summand
    bool m_delta;

    /// \brief The actions of the summand
    action_list m_actions;

    /// \brief The time of the summand. If <tt>m_time == data::data_expression()</tt>
    /// the summand has no time.
    data::data_expression m_time;

    /// \brief The assignments of the summand. These assignments are an encoding of
    /// the 'next states' of the summand.
    data::assignment_list m_assignments;

  public:
    /// \brief Returns the multi-action of this summand.
    /// \pre The summand is no deadlock summand.
    lps::multi_action multi_action() const
    {
      return lps::multi_action(m_actions, m_time);
    }

    /// \brief Returns the deadlock of this summand.
    /// \pre The summand is a deadlock summand.
    lps::deadlock deadlock() const
    {
      return lps::deadlock(m_time);
    }

    /// \brief Constructor.
    summand()
      : atermpp::aterm_appl(core::detail::constructLinearProcessSummand())
    {}

    /// \brief Constructor.
    /// \param t A term
    summand(atermpp::aterm_appl t)
     : atermpp::aterm_appl(t)
    {
      assert(core::detail::check_rule_LinearProcessSummand(m_term));
      atermpp::aterm_appl::iterator i = t.begin();

      m_summation_variables = *i++;
      m_condition           = data::data_expression(*i++);
      atermpp::aterm_appl x          = *i++;
      m_delta = core::detail::gsIsDelta(x);
      if (!m_delta)
      {
        assert(core::detail::gsIsMultAct(x));
        m_actions = action_list(x.argument(0));
      }
      m_time                = data::data_expression(*i++);
      m_assignments = *i;
    }

    /// \brief Constructor.
    /// Constructs an untimed summand.
    /// \deprecated
    summand(data::variable_list   summation_variables,
            data::data_expression      condition,
            bool                 delta,
            action_list          actions,
            data::assignment_list assignments
           )
      : atermpp::aterm_appl(core::detail::gsMakeLinearProcessSummand(
               summation_variables,
               condition,
               (delta ? core::detail::gsMakeDelta() : core::detail::gsMakeMultAct(actions)),
               core::detail::gsMakeNil(),
               assignments)
        ),
        m_summation_variables(summation_variables),
        m_condition          (condition),
        m_delta              (delta),
        m_actions            (actions),
        m_time               (data::data_expression(core::detail::gsMakeNil())),
        m_assignments        (assignments)
    {}

    /// \brief Constructor.
    /// Constructs a timed summand.
    /// \deprecated
    summand(data::variable_list   summation_variables,
            data::data_expression condition,
            bool                      delta,
            action_list               actions,
            data::data_expression time,
            data::assignment_list assignments
           )
      : atermpp::aterm_appl(core::detail::gsMakeLinearProcessSummand(
               summation_variables,
               condition,
               (delta ? core::detail::gsMakeDelta() : core::detail::gsMakeMultAct(actions)),
               time,
               assignments)
        ),
        m_summation_variables(summation_variables),
        m_condition          (condition),
        m_delta              (delta),
        m_actions            (actions),
        m_time               (time),
        m_assignments        (assignments)
    {}

    /// \brief Constructor.
    /// Constructs a multi action summand.
    summand(data::variable_list   summation_variables,
            data::data_expression      condition,
            const lps::multi_action&   a,
            data::assignment_list assignments
           )
      : atermpp::aterm_appl(core::detail::gsMakeLinearProcessSummand(
               summation_variables,
               condition,
               core::detail::gsMakeMultAct(a.actions()),
               a.time(),
               assignments)
        ),
        m_summation_variables(summation_variables),
        m_condition          (condition),
        m_delta              (false),
        m_actions            (a.actions()),
        m_time               (a.time()),
        m_assignments        (assignments)
    {}

    /// \brief Constructor.
    /// Constructs a deadlock summand.
    summand(data::variable_list   summation_variables,
            data::data_expression      condition,
            const lps::deadlock&       d
           )
      : atermpp::aterm_appl(core::detail::gsMakeLinearProcessSummand(
               summation_variables,
               condition,
               core::detail::gsMakeDelta(),
               d.time(),
               atermpp::term_list< data::assignment >())
        ),
        m_summation_variables(summation_variables),
        m_condition          (condition),
        m_delta              (true),
        m_time               (d.time())
    {}

    /// \brief Returns the sequence of summation variables.
    /// \return The sequence of summation variables.
    data::variable_list summation_variables() const
    {
      return m_summation_variables;
    }

    /// \brief Returns true if the multi-action corresponding to this summand is equal to delta.
    /// \return True if the multi-action corresponding to this summand is equal to delta.
    bool is_delta() const
    {
      return m_delta;
    }

    /// \brief Returns true if the multi-action corresponding to this summand is equal to tau.
    /// \return True if the multi-action corresponding to this summand is equal to tau.
    bool is_tau() const
    {
      // return !is_delta() && actions().size() == 1 && core::detail::gsIsTau(actions().front());

      /*
      // Written by Luc Engelen; no idea what it does.
      ATermAppl v_multi_action_or_delta = ATAgetArgument(*this, 2);
      if (gsIsMultAct(v_multi_action_or_delta)) {
        return ATisEmpty(ATLgetArgument(v_multi_action_or_delta, 0));
      } else {
        return false;
      }
      */
      return !is_delta() && actions().empty();
    }

    /// \brief Returns true if time is available.
    /// \return True if time is available.
    bool has_time() const
    {
      return !data::is_nil(m_time);
    }

    /// \brief Returns the condition expression.
    /// \return The condition expression.
    data::data_expression const& condition() const
    {
      return m_condition;
    }

    /// \brief Returns the sequence of actions. Returns an empty list if is_delta() holds.
    /// \return The sequence of actions. Returns an empty list if is_delta() holds.
    /// \deprecated
    action_list actions() const
    {
      return m_actions;
    }

    /// \brief Returns the time.
    /// \return The time.
    /// \deprecated
    data::data_expression const& time() const
    {
      return m_time;
    }

    /// \brief Returns the sequence of assignments.
    /// \return The sequence of assignments.
    data::assignment_list const& assignments() const
    {
      return m_assignments;
    }

    /// \brief Returns the next state corresponding to this summand.
    /// \details The next state is constructed out of the assignments in
    /// this summand, by substituting the assignments to the list of
    /// variables that are an argument of this function. In general this
    /// argument is the list with the process parameters of this process.
    /// \param process_parameters A sequence of data variables
    /// \return A symbolic representation of the next states
    data::data_expression_list next_state(const data::variable_list& process_parameters) const
    {
      return data::substitute_variables(atermpp::convert<data::data_expression_list>(process_parameters), data::assignment_sequence_substitution(assignments()));
    }
};

/// \brief Sets the summation variables of s and returns the result
/// \param s A linear process summand
/// \param summation_variables A sequence of data variables
/// \return The updated summand
inline
summand set_summation_variables(summand s, data::variable_list summation_variables)
{
  return summand(summation_variables,
                     s.condition          (),
                     s.is_delta           (),
                     s.actions            (),
                     s.time               (),
                     s.assignments        ()
                    );
}

/// \brief Sets the condition of s and returns the result
/// \param s A linear process summand
/// \param condition A data expression
/// \return The updated summand
inline
summand set_condition(summand s, data::data_expression condition)
{
  return summand(s.summation_variables(),
                 condition,
                 s.is_delta           (),
                 s.actions            (),
                 s.time               (),
                 s.assignments        ()
                );
}

/// \brief Sets the actions of s to delta and returns the result
/// \param s A linear process summand
/// \return The updated summand
inline
summand set_delta(summand s)
{
  return summand(s.summation_variables(),
                 s.condition          (),
                 false,
                 s.actions            (),
                 s.time               (),
                 s.assignments        ()
                );
}

/// \brief Sets the actions of s and returns the result
/// \param s A linear process summand
/// \param actions A sequence of actions
/// \return The updated summand
inline
summand set_actions(summand s, action_list actions)
{
  return summand(s.summation_variables(),
                 s.condition          (),
                 s.is_delta           (),
                 actions,
                 s.time               (),
                 s.assignments        ()
                );
}

/// \brief Sets the time of s and returns the result
/// \param s A linear process summand
/// \param time A data expression
/// \return The updated summand
inline
summand set_time(summand s, data::data_expression time)
{
  return summand(s.summation_variables(),
                     s.condition          (),
                     s.is_delta           (),
                     s.actions            (),
                     time,
                     s.assignments        ()
                    );
}

/// \brief Sets the assignments of s and returns the result
/// \param s A linear process summand
/// \param assignments A sequence of assignments to data variables
/// \return The updated summand
inline
summand set_assignments(summand s, data::assignment_list assignments)
{
  return summand(s.summation_variables(),
                 s.condition          (),
                 s.is_delta           (),
                 s.actions            (),
                 s.time               (),
                 assignments
                );
}

/// \brief Read-only singly linked list of summands
typedef atermpp::term_list<summand> summand_list;

/// \brief Base class for LPS summands.
// <LinearProcessSummand>   ::= LinearProcessSummand(<DataVarId>*, <DataExpr>, <MultActOrDelta>,
//                    <DataExprOrNil>, <Assignment>*)
//<MultActOrDelta>
//               ::= <MultAct>
//                 | Delta
// <MultAct>      ::= MultAct(<ParamId>*)                                   (- tc)
//                  | MultAct(<Action>*)                                    (+ tc)
class summand_base
{
  friend struct atermpp::aterm_traits<summand_base>;

  protected:
    /// \brief The summation variables of the summand
    data::variable_list m_summation_variables;

    /// \brief The condition of the summand
    data::data_expression m_condition;

    /// \brief Protects the term from being freed during garbage collection.
    void protect()
    {
      m_summation_variables.protect();
      m_condition.protect();
    }

    /// \brief Unprotect the term.
    /// Releases protection of the term which has previously been protected through a
    /// call to protect.
    void unprotect()
    {
      m_summation_variables.unprotect();
      m_condition.unprotect();
    }

    /// \brief Mark the term for not being garbage collected.
    void mark()
    {
      m_summation_variables.mark();
      m_condition.mark();
    }

  public:
    /// \brief Constructor.
    summand_base()
    {}

    /// \brief Constructor.
    summand_base(const data::variable_list& summation_variables, const data::data_expression& condition)
      : m_summation_variables(summation_variables),
        m_condition(condition)
    {}

    /// \brief Returns the sequence of summation variables.
    /// \return The sequence of summation variables.
    data::variable_list& summation_variables()
    {
      return m_summation_variables;
    }

    /// \brief Returns the sequence of summation variables.
    /// \return The sequence of summation variables.
    const data::variable_list& summation_variables() const
    {
      return m_summation_variables;
    }

    /// \brief Returns the condition expression.
    /// \return The condition expression.
    const data::data_expression& condition() const
    {
      return m_condition;
    }

    /// \brief Returns the condition expression.
    /// \return The condition expression.
    data::data_expression& condition()
    {
      return m_condition;
    }
};

/// \brief LPS summand containing a deadlock.
class deadlock_summand: public summand_base
{
  friend struct atermpp::aterm_traits<deadlock_summand>;

  protected:
    /// \brief The super class
    typedef summand_base super;

    /// \brief The deadlock of the summand
    lps::deadlock m_deadlock;

    /// \brief Protects the term from being freed during garbage collection.
    void protect()
    {
      super::protect();
      m_deadlock.protect();
    }

    /// \brief Unprotect the term.
    /// Releases protection of the term which has previously been protected through a
    /// call to protect.
    void unprotect()
    {
      super::unprotect();
      m_deadlock.unprotect();
    }

    /// \brief Mark the term for not being garbage collected.
    void mark()
    {
      super::mark();
      m_deadlock.mark();
    }

    /// \brief Returns true if time is available.
    /// \return True if time is available.
    bool has_time() const
    {
      return m_deadlock.has_time();
    }

  public:
    /// \brief Constructor.
    // TODO: check if the default constructor results in a deadlock summand
    deadlock_summand()
    {}

    /// \brief Constructor.
    deadlock_summand(const data::variable_list& summation_variables, const data::data_expression& condition, const lps::deadlock& delta)
      : summand_base(summation_variables, condition),
        m_deadlock(delta)
    {}

    /// \brief Returns the deadlock of this summand.
    const lps::deadlock& deadlock() const
    {
      return m_deadlock;
    }

    /// \brief Returns the deadlock of this summand.
    lps::deadlock& deadlock()
    {
      return m_deadlock;
    }
};

/// \brief Vector of deadlock summands
typedef atermpp::vector<deadlock_summand> deadlock_summand_vector;

/// \brief Conversion to ATermAppl.
/// \return The deadlock summand converted to ATerm format.
inline
summand deadlock_summand_to_aterm(const deadlock_summand& s)
{
  ATermAppl result = core::detail::gsMakeLinearProcessSummand(
         s.summation_variables(),
         s.condition(),
         core::detail::gsMakeDelta(),
         s.deadlock().time(),
         data::assignment_list()
        );
  return atermpp::aterm_appl(result);
}

/// \brief Conversion to action summand.
/// \deprecated
inline
deadlock_summand summand_to_deadlock_summand(const summand& s)
{
  assert(s.is_delta());
  return deadlock_summand(s.summation_variables(), s.condition(), s.deadlock());
}

/// \brief LPS summand containing a multi-action.
class action_summand: public summand_base
{
  friend struct atermpp::aterm_traits<action_summand>;

  protected:
    /// \brief The super class
    typedef summand_base super;

    /// \brief The summation variables of the summand
    lps::multi_action m_multi_action;

    /// \brief The assignments of the next state
    data::assignment_list m_assignments;

    /// \brief Protects the term from being freed during garbage collection.
    void protect()
    {
      super::protect();
      m_multi_action.protect();
      m_assignments.protect();
    }

    /// \brief Unprotect the term.
    /// Releases protection of the term which has previously been protected through a
    /// call to protect.
    void unprotect()
    {
      super::unprotect();
      m_multi_action.unprotect();
      m_assignments.unprotect();
    }

    /// \brief Mark the term for not being garbage collected.
    void mark()
    {
      super::mark();
      m_multi_action.mark();
      m_assignments.mark();
    }

  public:
    /// \brief Constructor.
    // TODO: check if the default constructor results in a deadlock summand
    action_summand()
    {}

    /// \brief Constructor.
    action_summand(const data::variable_list& summation_variables, const data::data_expression& condition, const lps::multi_action& action, const data::assignment_list& assignments)
      : summand_base(summation_variables, condition),
        m_multi_action(action),
        m_assignments(assignments)
    {}

    /// \brief Returns the multi-action of this summand.
    const lps::multi_action& multi_action() const
    {
      return m_multi_action;
    }

    /// \brief Returns the multi-action of this summand.
    lps::multi_action& multi_action()
    {
      return m_multi_action;
    }

    /// \brief Returns the sequence of assignments.
    /// \return The sequence of assignments.
    const data::assignment_list& assignments() const
    {
      return m_assignments;
    }

    /// \brief Returns the sequence of assignments.
    /// \return The sequence of assignments.
    data::assignment_list& assignments()
    {
      return m_assignments;
    }

    // TODO: check if this is correct (where is the documentation of the internal format?)
    /// \brief Returns true if the multi-action corresponding to this summand is equal to tau.
    /// \return True if the multi-action corresponding to this summand is equal to tau.
    bool is_tau() const
    {
      return multi_action().actions().empty();
    }

    /// \brief Returns true if time is available.
    /// \return True if time is available.
    bool has_time() const
    {
      return m_multi_action.has_time();
    }

    /// \brief Returns the next state corresponding to this summand.
    /// \details The next state is constructed out of the assignments in
    /// this summand, by substituting the assignments to the list of
    /// variables that are an argument of this function. In general this
    /// argument is the list with the process parameters of this process.
    /// \param process_parameters A sequence of data variables
    /// \return A symbolic representation of the next states
    data::data_expression_list next_state(const data::variable_list& process_parameters) const
    {
      return data::substitute_variables(atermpp::convert<data::data_expression_list>(process_parameters), data::assignment_sequence_substitution(assignments()));
    }
};

/// \brief Vector of action summands
typedef atermpp::vector<action_summand> action_summand_vector;

/// \brief Equality operator of action summands
inline
bool operator==(const action_summand& x, const action_summand& y)
{
  return x.condition() == y.condition() && x.multi_action() == y.multi_action() && x.assignments() == y.assignments();
}

/// \brief Conversion to ATermAppl.
/// \return The action summand converted to ATerm format.
inline
summand action_summand_to_aterm(const action_summand& s)
{
  ATermAppl result = core::detail::gsMakeLinearProcessSummand(
         s.summation_variables(),
         s.condition(),
         core::detail::gsMakeMultAct(s.multi_action().actions()),
         s.multi_action().time(),
         s.assignments()
        );
  return atermpp::aterm_appl(result);
}

/// \brief Conversion to action summand.
/// \deprecated
inline
action_summand summand_to_action_summand(const summand& s)
{
  assert(!s.is_delta());
  return action_summand(s.summation_variables(), s.condition(), s.multi_action(), s.assignments());
}

} // namespace lps

} // namespace mcrl2

/// \cond INTERNAL_DOCS
namespace atermpp {

template<>
struct aterm_traits<mcrl2::lps::summand_base>
{
  typedef ATermAppl aterm_type;
  static void protect(mcrl2::lps::summand_base t)   { t.protect(); }
  static void unprotect(mcrl2::lps::summand_base t) { t.unprotect(); }
  static void mark(mcrl2::lps::summand_base t)      { t.mark(); }
  //static ATerm term(mcrl2::lps::summand_base t)     { return t.term(); }
  //static ATerm* ptr(mcrl2::lps::summand_base& t)    { return &t.term(); }
};

template<>
struct aterm_traits<mcrl2::lps::deadlock_summand>
{
  typedef ATermAppl aterm_type;
  static void protect(mcrl2::lps::deadlock_summand t)   { t.protect(); }
  static void unprotect(mcrl2::lps::deadlock_summand t) { t.unprotect(); }
  static void mark(mcrl2::lps::deadlock_summand t)      { t.mark(); }
  //static ATerm term(mcrl2::lps::deadlock_summand t)     { return t.term(); }
  //static ATerm* ptr(mcrl2::lps::deadlock_summand& t)    { return &t.term(); }
};

template<>
struct aterm_traits<mcrl2::lps::action_summand>
{
  typedef ATermAppl aterm_type;
  static void protect(mcrl2::lps::action_summand t)   { t.protect(); }
  static void unprotect(mcrl2::lps::action_summand t) { t.unprotect(); }
  static void mark(mcrl2::lps::action_summand t)      { t.mark(); }
  //static ATerm term(mcrl2::lps::action_summand t)     { return t.term(); }
  //static ATerm* ptr(mcrl2::lps::action_summand& t)    { return &t.term(); }
};

} // namespace atermpp
/// \endcond

#endif // MCRL2_LPS_SUMMAND_H
