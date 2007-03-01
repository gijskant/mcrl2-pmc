///////////////////////////////////////////////////////////////////////////////
/// \file action.h
/// Contains action data structures for the LPS Library.

#ifndef LPS_ACTION_H
#define LPS_ACTION_H

#include <cassert>
#include "atermpp/atermpp.h"
#include "lps/identifier_string.h"
#include "lps/data.h"
#include "lps/pretty_print.h"
#include "lps/detail/soundness_checks.h"

namespace lps {

using atermpp::aterm_appl;
using atermpp::term_list;

///////////////////////////////////////////////////////////////////////////////
// action_label
/// \brief Represents a label of an action.
///
// <ActId>        ::= ActId(<String>, <SortExpr>*)
class action_label: public aterm_appl
{
  protected:
    identifier_string m_name;
    sort_list m_sorts;

  public:
    action_label()
      : aterm_appl(detail::constructActId())
    {}

    action_label(aterm_appl t)
     : aterm_appl(t)
    {
      assert(detail::check_rule_ActId(m_term));
      aterm_appl::iterator i = t.begin();
      m_name  = *i++;
      m_sorts = *i;
    }

    action_label(const identifier_string& name, const sort_list &sorts)
     : aterm_appl(gsMakeActId(name, sorts)),
       m_name(name),
       m_sorts(sorts)
    {}

    /// Returns the name of the action label.
    ///
    identifier_string name() const
    {
      return m_name;
    }

    /// Returns the sorts of the action label.
    ///
    sort_list sorts() const
    {
      return m_sorts;
    }

    /// Applies a substitution to this action label and returns the result.
    /// The Substitution object must supply the method aterm operator()(aterm).
    ///
    template <typename Substitution>
    action_label substitute(Substitution f)
    {
      return action_label(f(*this));
    }     
};

///////////////////////////////////////////////////////////////////////////////
// action_label_list
/// \brief singly linked list of actions
///
typedef term_list<action_label> action_label_list;

inline
bool is_action_label(aterm_appl t)
{
  return gsIsActId(t);
};

///////////////////////////////////////////////////////////////////////////////
// action
/// \brief Represents an action.
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

    /// DEPRECATED (This constructor will disappear).
    action(const identifier_string& name, const data_expression_list& arguments)
     : aterm_appl(gsMakeAction(gsMakeActId(name, apply(arguments, gsGetSort)), arguments)),
       m_arguments(arguments)
    {
      m_label = action_label(*begin());
    }

    /// Returns the label of the action.
    ///
    action_label label() const
    {
      return m_label;
    }

    /// Returns the sequence of arguments.
    ///
    data_expression_list arguments() const
    {
      return m_arguments;
    }

    /// DEPRECATED (This method will disappear).
    /// Returns the name of the action.
    ///
    identifier_string name() const
    {
      return m_label.name();
    }

    /// DEPRECATED (This method will disappear).
    /// Returns the sorts of the action.
    ///
    sort_list sorts() const
    {
      return m_label.sorts();
    }

    /// Applies a substitution to this action and returns the result.
    /// The Substitution object must supply the method aterm operator()(aterm).
    ///
    template <typename Substitution>
    action substitute(Substitution f)
    {
      return action(f(aterm_appl(*this)));
    }     
};

///////////////////////////////////////////////////////////////////////////////
// action_list
/// \brief singly linked list of actions
///
typedef term_list<action> action_list;

inline
bool is_action(aterm_appl t)
{
  return gsIsAction(t);
}

///////////////////////////////////////////////////////////////////////////////
// timed_action
/// \brief multi action with time
///
class timed_action
{
  protected:
    action_list     m_actions;
    data_expression m_time;

  public:
    timed_action(action_list actions, data_expression time)
      : m_actions(actions), m_time(time)
    {}

    /// Returns true if time is available.
    ///
    bool has_time() const
    {
      return !m_time.is_nil();
    }

    /// Returns the sequence of actions. Returns an empty list if is_delta() holds.
    ///
    action_list actions() const
    {
      return m_actions;
    }

    /// Returns the time expression.
    ///
    data_expression time() const
    {
      return m_time;
    }
    
    /// Returns a term representing the name of the first action.
    ///
    identifier_string name() const
    {
      return front(m_actions).name();
    }
    
    /// Returns the argument of the multi action.
    data_expression_list arguments() const
    {
      return front(m_actions).arguments();
    }

    /// Applies a substitution to this action and returns the result.
    /// The Substitution object must supply the method aterm operator()(aterm).
    ///
    template <typename Substitution>
    timed_action substitute(Substitution f)
    {
      return timed_action(m_actions.substitute(f), m_time.substitute(f));
    }     
};

} // namespace lps

/// INTERNAL ONLY
namespace atermpp
{
using lps::action_label;
using lps::action;

template<>
struct aterm_traits<action_label>
{
  typedef ATermAppl aterm_type;
  static void protect(action_label t)   { t.protect(); }
  static void unprotect(action_label t) { t.unprotect(); }
  static void mark(action_label t)      { t.mark(); }
  static ATerm term(action_label t)     { return t.term(); }
  static ATerm* ptr(action_label& t)    { return &t.term(); }
};

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

#endif // LPS_ACTION_H
