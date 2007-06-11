// Copyright (c) 2007 Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/action.h
/// \brief Add your file description here.

#ifndef MCRL2_LPS_ACTION_H
#define MCRL2_LPS_ACTION_H

#include <cassert>
#include "mcrl2/lps/action_label.h"
#include "mcrl2/data/data.h"

namespace lps {

using atermpp::aterm_appl;
using atermpp::term_list;

///////////////////////////////////////////////////////////////////////////////
// action
/// \brief Represents an action
///
// <Action>       ::= Action(<ActId>, <DataExpr>*)
class action: public aterm_appl
{
  protected:
    action_label m_label;
    data_expression_list m_arguments;

  public:
    action()
      : aterm_appl(detail::constructAction())
    {}

    action(aterm_appl t)
     : aterm_appl(t)
    {
      assert(detail::check_rule_Action(m_term));
      aterm_appl::iterator i = t.begin();
      m_label = action_label(*i++);
      m_arguments = data_expression_list(*i);
    }

    action(const action_label& label, const data_expression_list& arguments)
     : aterm_appl(gsMakeAction(label, arguments)),
       m_label(label),
       m_arguments(arguments)
    {}

    /// Returns the label of the action.
    ///
    action_label label() const
    {
      return m_label;
    }

    /// Returns the arguments of the action.
    ///
    data_expression_list arguments() const
    {
      return m_arguments;
    }

    /// Applies a substitution to this action and returns the result.
    /// The Substitution object must supply the method aterm operator()(aterm).
    template <typename Substitution>
    action substitute(Substitution f)
    {
      return action(f(aterm_appl(*this)));
    }     
};

///////////////////////////////////////////////////////////////////////////////
// action_list
/// \brief singly linked list of actions
typedef term_list<action> action_list;

/// Returns true if the term t is an action
inline
bool is_action(aterm_appl t)
{
  return gsIsAction(t);
}

} // namespace lps

/// INTERNAL ONLY
namespace atermpp
{
using lps::action;

template<>
struct aterm_traits<action>
{
  typedef ATermAppl aterm_type;
  static void protect(action t)   { t.protect(); }
  static void unprotect(action t) { t.unprotect(); }
  static void mark(action t)      { t.mark(); }
  static ATerm term(action t)     { return t.term(); }
  static ATerm* ptr(action& t)    { return &t.term(); }
};

} // namespace atermpp

#endif // MCRL2_LPS_ACTION_H
