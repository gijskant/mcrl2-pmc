// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/mucalculus.h
/// \brief Add your file description here.

#ifndef MCRL2_MODAL_ACTION_FORMULA_H
#define MCRL2_MODAL_ACTION_FORMULA_H

#include <iostream> // for debugging

#include <string>
#include <cassert>
#include "mcrl2/atermpp/aterm_traits.h"
#include "mcrl2/atermpp/atermpp.h"
#include "mcrl2/lps/action.h"
#include "mcrl2/lps/detail/action_utility.h"

namespace mcrl2 {

namespace modal {

using atermpp::aterm_appl;
using atermpp::aterm_list;
using atermpp::aterm;
using atermpp::term_list;

///////////////////////////////////////////////////////////////////////////////
// action_formula
/// \brief action formula expression.
///
//<ActFrm>       ::= <MultAct>
//                 | <DataExpr>
//                 | ActTrue
//                 | ActFalse
//                 | ActNot(<ActFrm>)
//                 | ActAnd(<ActFrm>, <ActFrm>)
//                 | ActOr(<ActFrm>, <ActFrm>)
//                 | ActImp(<ActFrm>, <ActFrm>)
//                 | ActForall(<DataVarId>+, <ActFrm>)
//                 | ActExists(<DataVarId>+, <ActFrm>)
//                 | ActAt(<ActFrm>, <DataExpr>)
class action_formula: public aterm_appl
{
  public:
    action_formula()
      : aterm_appl(mcrl2::core::detail::constructActFrm())
    {}

    action_formula(ATermAppl t)
      : aterm_appl(aterm_appl(t))
    {
      assert(mcrl2::core::detail::check_rule_ActFrm(m_term));
    }

    action_formula(aterm_appl t)
      : aterm_appl(t)
    {
      assert(mcrl2::core::detail::check_rule_ActFrm(m_term));
    }

    /// \brief Applies a substitution to this action_formula and returns the result
    /// The Substitution object must supply the method aterm operator()(aterm).
    ///
    template <typename Substitution>
    action_formula substitute(Substitution f) const
    {
      return action_formula(f(aterm(*this)));
    }     
};

///////////////////////////////////////////////////////////////////////////////
// action_formula_list
/// \brief singly linked list of data expressions
///
typedef term_list<action_formula> action_formula_list;

/// Accessor functions and predicates for action formulas.
namespace act_frm
{ 

using atermpp::arg1;
using atermpp::arg2;
using atermpp::arg3;
using atermpp::list_arg1;
using atermpp::list_arg2;
using namespace mcrl2::data;
using namespace mcrl2::core::detail;

  /// \brief Returns the expression true
  inline
  action_formula true_()
  {
    return action_formula(core::detail::gsMakeActTrue());
  }
  
  /// \brief Returns the expression false
  inline
  action_formula false_()
  {
    return action_formula(core::detail::gsMakeActFalse());
  }
  
  /// \brief Returns not applied to p
  inline
  action_formula not_(action_formula p)
  {
    return action_formula(core::detail::gsMakeActNot(p));
  }
  
  /// \brief Returns and applied to p and q
  inline
  action_formula and_(action_formula p, action_formula q)
  {
    return action_formula(core::detail::gsMakeActAnd(p,q));
  }
  
  /// \brief Returns or applied to p and q
  inline
  action_formula or_(action_formula p, action_formula q)
  {
    return action_formula(core::detail::gsMakeActOr(p,q));
  }
  
  /// \brief Returns imp applied to p and q
  inline
  action_formula imp(action_formula p, action_formula q)
  {
    return action_formula(core::detail::gsMakeActImp(p,q));
  }
  
  /// \brief Returns the universal quantification of the formula p over the variables in l
  /// \pre l may not be empty
  inline
  action_formula forall(data::data_variable_list l, action_formula p)
  {
    assert(!l.empty());
    return action_formula(core::detail::gsMakeActForall(l, p));
  }

  /// \brief Returns the existential quantification of the formula p over the variables in l
  /// \pre l may not be empty
  inline
  action_formula exists(data::data_variable_list l, action_formula p)
  {
    assert(!l.empty());
    return action_formula(core::detail::gsMakeActExists(l, p));
  }

  /// \brief Returns the 'p at d'
  inline
  action_formula at(action_formula p, data::data_expression d)
  {
    return action_formula(core::detail::gsMakeActAt(p,d));
  }

  /// \brief Returns true if the term t is a multi action
  inline bool is_mult_act(aterm_appl t) { return core::detail::gsIsMultAct(t); }

  /// \brief Returns true if the term t is a data expression
  inline bool is_data(aterm_appl t) { return core::detail::gsIsDataExpr(t); }

  /// \brief Returns true if the term t is equal to true
  inline bool is_true(aterm_appl t) { return core::detail::gsIsActTrue(t); }

  /// \brief Returns true if the term t is equal to false
  inline bool is_false(aterm_appl t) { return core::detail::gsIsActFalse(t); }

  /// \brief Returns true if the term t is a not expression
  inline bool is_not(aterm_appl t) { return core::detail::gsIsActNot(t); }

  /// \brief Returns true if the term t is an and expression
  inline bool is_and(aterm_appl t) { return core::detail::gsIsActAnd(t); }

  /// \brief Returns true if the term t is an or expression
  inline bool is_or(aterm_appl t) { return core::detail::gsIsActOr(t); }

  /// \brief Returns true if the term t is an implication expression
  inline bool is_imp(aterm_appl t) { return core::detail::gsIsActImp(t); }

  /// \brief Returns true if the term t is a universal quantification
  inline bool is_forall(aterm_appl t) { return core::detail::gsIsActForall(t); }

  /// \brief Returns true if the term t is an existential quantification
  inline bool is_exists(aterm_appl t) { return core::detail::gsIsActExists(t); }

  /// \brief Returns true if the term t is an at expression
  inline bool is_at(aterm_appl t) { return core::detail::gsIsActAt(t); }

} // namespace act_frm

namespace accessors
{
  /// \brief Returns the parameters of an action formula
  inline
  lps::action_list mult_params(action_formula t)
  {
    assert(core::detail::gsIsMultAct(t));
    return list_arg1(t);
  }
  
  /// \brief Returns the action formula argument of an expression of
  /// type not, at, exists or forall.
  inline
  action_formula arg(action_formula t)
  {
    if (core::detail::gsIsActNot(t) || core::detail::gsIsActAt(t))
    {
      return arg1(t);
    }
    assert(core::detail::gsIsActExists(t) || core::detail::gsIsActForall(t));
    return arg2(t);
  }
  
  /// \brief Returns the left hand side of an expression of type and/or/imp
  inline
  action_formula left(action_formula t)
  {
    assert(core::detail::gsIsActAnd(t) || core::detail::gsIsActOr(t) || core::detail::gsIsActImp(t));
    return arg1(t);
  }
  
  /// \brief Returns the right hand side of an expression of type and/or/imp.
  inline
  action_formula right(action_formula t)
  {
    assert(core::detail::gsIsActAnd(t) || core::detail::gsIsActOr(t) || core::detail::gsIsActImp(t));
    return arg2(t);
  }
  
  /// \brief Returns the variables of a quantification expression
  inline
  data::data_variable_list var(action_formula t)
  {
    assert(core::detail::gsIsActExists(t) || core::detail::gsIsActForall(t));
    return list_arg1(t);
  }
  
  /// \brief Returns the time of an at expression
  inline
  data::data_expression time(action_formula t)
  {
    assert(core::detail::gsIsActAt(t));
    return arg2(t);
  }

} // namespace accessors

} // namespace modal

} // namespace mcrl2

/// \internal
namespace atermpp
{
using mcrl2::modal::action_formula;

template<>
struct aterm_traits<action_formula>
{
  typedef ATermAppl aterm_type;
  static void protect(action_formula t)   { t.protect(); }
  static void unprotect(action_formula t) { t.unprotect(); }
  static void mark(action_formula t)      { t.mark(); }
  static ATerm term(action_formula t)     { return t.term(); }
  static ATerm* ptr(action_formula& t)    { return &t.term(); }
};

} // namespace atermpp

#endif // MCRL2_MODAL_ACTION_FORMULA_H
