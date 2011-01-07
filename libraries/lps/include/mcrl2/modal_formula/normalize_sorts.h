// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/normalize_sorts.h
/// \brief add your file description here.

#ifndef MCRL2_MODAL_FORMULA_NORMALIZE_SORTS_H
#define MCRL2_MODAL_FORMULA_NORMALIZE_SORTS_H

#include "mcrl2/data/normalize_sorts.h"
#include "mcrl2/modal_formula/action_formula.h"
#include "mcrl2/modal_formula/regular_formula.h"
#include "mcrl2/modal_formula/state_formula.h"
#include "mcrl2/modal_formula/builder.h"

namespace mcrl2 {

namespace action_formulas {

  template <typename T>
  void normalize_sorts(T& x,
                       const data::data_specification& data_spec,
                       typename boost::disable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                      )
  {
    core::make_update_apply_builder<action_formulas::sort_expression_builder>(data::detail::normalize_sorts_function(data_spec))(x);
  }

  template <typename T>
  T normalize_sorts(const T& x,
                    const data::data_specification& data_spec,
                    typename boost::enable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                   )
  {
    return core::make_update_apply_builder<action_formulas::sort_expression_builder>(data::detail::normalize_sorts_function(data_spec))(x);
  }

} // namespace action_formulas

namespace regular_formulas {

  template <typename T>
  void normalize_sorts(T& x,
                       const data::data_specification& data_spec,
                       typename boost::disable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                      )
  {
    core::make_update_apply_builder<regular_formulas::sort_expression_builder>(data::detail::normalize_sorts_function(data_spec))(x);
  }

  template <typename T>
  T normalize_sorts(const T& x,
                    const data::data_specification& data_spec,
                    typename boost::enable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                   )
  {
    return core::make_update_apply_builder<regular_formulas::sort_expression_builder>(data::detail::normalize_sorts_function(data_spec))(x);
  }

} // namespace regular_formulas

namespace state_formulas {

  template <typename T>
  void normalize_sorts(T& x,
                       const data::data_specification& data_spec,
                       typename boost::disable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                      )
  {
    core::make_update_apply_builder<state_formulas::sort_expression_builder>(data::detail::normalize_sorts_function(data_spec))(x);
  }

  template <typename T>
  T normalize_sorts(const T& x,
                    const data::data_specification& data_spec,
                    typename boost::enable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                   )
  {
    return core::make_update_apply_builder<state_formulas::sort_expression_builder>(data::detail::normalize_sorts_function(data_spec))(x);
  }

} // namespace state_formulas

} // namespace mcrl2

#endif // MCRL2_MODAL_FORMULA_NORMALIZE_SORTS_H
