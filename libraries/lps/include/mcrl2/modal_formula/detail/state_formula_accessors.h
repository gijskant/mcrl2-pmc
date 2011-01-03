// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/detail/state_formula_accessors.h
/// \brief add your file description here.

#ifndef MCRL2_MODAL_FORMULA_DETAIL_STATE_FORMULA_ACCESSORS_H
#define MCRL2_MODAL_FORMULA_DETAIL_STATE_FORMULA_ACCESSORS_H

#include "mcrl2/modal_formula/state_formula.h"

namespace mcrl2 {

namespace state_formulas {

namespace detail {

namespace accessors {

  /// \brief Returns the argument of a data expression
  /// \param t A modal formula
  /// \return The argument of a data expression
  inline
  data::data_expression val(const state_formula& t)
  {
    assert(core::detail::gsIsDataExpr(t));
    return t;
  }

  /// \brief Returns the state formula argument of an expression of type
  /// not, mu, nu, exists, forall, must or may.
  /// \param t A modal formula
  /// \return The state formula argument of an expression of type
  inline
  state_formula arg(const state_formula& t)
  {
    if (core::detail::gsIsStateNot(t))
    {
      return atermpp::arg1(t);
    }
    if (core::detail::gsIsStateMu(t) || core::detail::gsIsStateNu(t))
    {
      return atermpp::arg3(t);
    }
    assert(core::detail::gsIsStateExists(t) ||
           core::detail::gsIsStateForall(t) ||
           core::detail::gsIsStateMust(t)   ||
           core::detail::gsIsStateMay(t)
          );
    return atermpp::arg2(t);
  }

  /// \brief Returns the left hand side of an expression of type and/or/imp
  /// \param t A modal formula
  /// \return The left hand side of an expression of type and/or/imp
  inline
  state_formula left(const state_formula& t)
  {
    assert(core::detail::gsIsStateAnd(t) || core::detail::gsIsStateOr(t) || core::detail::gsIsStateImp(t));
    return atermpp::arg1(t);
  }

  /// \brief Returns the right hand side of an expression of type and/or/imp.
  /// \param t A modal formula
  /// \return The right hand side of an expression of type and/or/imp.
  inline
  state_formula right(const state_formula& t)
  {
    assert(core::detail::gsIsStateAnd(t) || core::detail::gsIsStateOr(t) || core::detail::gsIsStateImp(t));
    return atermpp::arg2(t);
  }

  /// \brief Returns the variables of a quantification expression
  /// \param t A modal formula
  /// \return The variables of a quantification expression
  inline
  data::variable_list var(const state_formula& t)
  {
    assert(core::detail::gsIsStateExists(t) || core::detail::gsIsStateForall(t));
    return data::variable_list(
      atermpp::term_list_iterator< data::variable >(atermpp::list_arg1(t)),
      atermpp::term_list_iterator< data::variable >());
  }

  /// \brief Returns the time of a delay or yaled expression
  /// \param t A modal formula
  /// \return The time of a delay or yaled expression
  inline
  data::data_expression time(const state_formula& t)
  {
    assert(core::detail::gsIsStateDelayTimed(t) || core::detail::gsIsStateYaledTimed(t));
    return atermpp::arg1(t);
  }

  /// \brief Returns the name of a variable expression
  /// \param t A modal formula
  /// \return The name of a variable expression
  inline
  core::identifier_string name(const state_formula& t)
  {
    assert(core::detail::gsIsStateVar(t) ||
           core::detail::gsIsStateMu(t)  ||
           core::detail::gsIsStateNu(t)
          );
    return atermpp::arg1(t);
  }

  /// \brief Returns the parameters of a variable expression
  /// \param t A modal formula
  /// \return The parameters of a variable expression
  inline
  data::data_expression_list param(const state_formula& t)
  {
    assert(core::detail::gsIsStateVar(t));
    return data::data_expression_list(
      atermpp::term_list_iterator< data::data_expression >(atermpp::list_arg2(t)),
      atermpp::term_list_iterator< data::data_expression >());
  }

  /// \brief Returns the parameters of a mu or nu expression
  /// \param t A modal formula
  /// \return The parameters of a mu or nu expression
  inline
  data::assignment_list ass(const state_formula& t)
  {
    assert(core::detail::gsIsStateMu(t) || core::detail::gsIsStateNu(t));
    return data::assignment_list(
      atermpp::term_list_iterator< data::assignment >(atermpp::list_arg2(t)),
      atermpp::term_list_iterator< data::assignment >());
  }

  /// \brief Returns the regular formula of a must or may expression
  /// \param t A modal formula
  /// \return The regular formula of a must or may expression
  inline
  regular_formulas::regular_formula act(const state_formula& t)
  {
    assert(core::detail::gsIsStateMust(t) || core::detail::gsIsStateMay(t));
    return atermpp::arg1(t);
  }

} // namespace accessors

} // namespace detail

} // namespace state_formulas

} // namespace mcrl2

#endif // MCRL2_MODAL_FORMULA_DETAIL_STATE_FORMULA_ACCESSORS_H
