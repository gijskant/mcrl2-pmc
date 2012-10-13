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
#include "mcrl2/atermpp/utility.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/data/data_expression.h"
#include "mcrl2/data/print.h"
#include "mcrl2/data/real.h"
#include "mcrl2/data/bool.h"
#include "mcrl2/data/replace.h"
#include "mcrl2/data/detail/assignment_functional.h"
#include "mcrl2/data/detail/sequence_algorithm.h"
#include "mcrl2/lps/action.h"
#include "mcrl2/lps/deadlock.h"
#include "mcrl2/lps/multi_action.h"

namespace mcrl2
{

namespace lps
{

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

    /// \brief Mark the term for not being garbage collected.
    void mark() const
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
    
    /// \brief Protects the term from being freed during garbage collection.
    void protect() const
    {
      m_summation_variables.protect();
      m_condition.protect();
    }

    /// \brief Unprotect the term.
    /// Releases protection of the term which has previously been protected through a
    /// call to protect.
    void unprotect() const
    {
      m_summation_variables.unprotect();
      m_condition.unprotect();
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
    void protect() const
    {
      super::protect();
      m_deadlock.protect();
    }

    /// \brief Unprotect the term.
    /// Releases protection of the term which has previously been protected through a
    /// call to protect.
    void unprotect() const
    {
      super::unprotect();
      m_deadlock.unprotect();
    }

    /// \brief Mark the term for not being garbage collected.
    void mark() const
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
atermpp::aterm_appl deadlock_summand_to_aterm(const deadlock_summand& s)
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


    /// \brief Mark the term for not being garbage collected.
    void mark() const
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
      return data::replace_variables(atermpp::convert<data::data_expression_list>(process_parameters), data::assignment_sequence_substitution(assignments()));
    }

    /// \brief Protects the term from being freed during garbage collection.
    void protect() const
    {
      super::protect();
      m_multi_action.protect();
      m_assignments.protect();
    }

    /// \brief Unprotect the term.
    /// Releases protection of the term which has previously been protected through a
    /// call to protect.
    void unprotect() const
    {
      super::unprotect();
      m_multi_action.unprotect();
      m_assignments.unprotect();
    }
};

/// \brief Comparison operator for action summands.
inline
bool operator<(const action_summand& x, const action_summand& y)
{
  if (x.summation_variables() != y.summation_variables())
  {
    return x.summation_variables() < y.summation_variables();
  }
  if (x.condition() != y.condition())
  {
    return x.condition() < y.condition();
  }
  if (x.assignments() != y.assignments())
  {
    return x.assignments() < y.assignments();
  }
  return x.multi_action() < y.multi_action();
}

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
atermpp::aterm_appl action_summand_to_aterm(const action_summand& s)
{
  ATermAppl result = core::detail::gsMakeLinearProcessSummand(
                       s.summation_variables(),
                       s.condition(),
                       lps::detail::multi_action_to_aterm(s.multi_action()),
                       s.multi_action().time(),
                       s.assignments()
                     );
  return atermpp::aterm_appl(result);
}

} // namespace lps

} // namespace mcrl2

/// \cond INTERNAL_DOCS
namespace atermpp
{

template<>
struct aterm_traits<mcrl2::lps::summand_base>
{
  static void protect(const mcrl2::lps::summand_base& t)
  {
    t.protect();
  }
  static void unprotect(const mcrl2::lps::summand_base& t)
  {
    t.unprotect();
  }
  static void mark(const mcrl2::lps::summand_base& t)
  {
    t.mark();
  }
};

template<>
struct aterm_traits<mcrl2::lps::deadlock_summand>
{
  static void protect(const mcrl2::lps::deadlock_summand& t)
  {
    t.protect();
  }
  static void unprotect(const mcrl2::lps::deadlock_summand& t)
  {
    t.unprotect();
  }
  static void mark(const mcrl2::lps::deadlock_summand& t)
  {
    t.mark();
  }
};

template<>
struct aterm_traits<mcrl2::lps::action_summand>
{
  static void protect(const mcrl2::lps::action_summand& t)
  {
    t.protect();
  }
  static void unprotect(const mcrl2::lps::action_summand& t)
  {
    t.unprotect();
  }
  static void mark(const mcrl2::lps::action_summand& t)
  {
    t.mark();
  }
};

} // namespace atermpp
/// \endcond

#endif // MCRL2_LPS_SUMMAND_H
