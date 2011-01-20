// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/replace.h
/// \brief add your file description here.

#ifndef MCRL2_LPS_REPLACE_H
#define MCRL2_LPS_REPLACE_H

#include "mcrl2/data/replace.h"
#include "mcrl2/lps/add_binding.h"
#include "mcrl2/lps/builder.h"

namespace mcrl2 {

namespace lps {

//--- start generated lps replace code ---//
#ifdef MCRL2_NEW_REPLACE_VARIABLES
  template <typename T, typename Substitution>
  void replace_variables(T& x,
                         Substitution sigma,
                         typename boost::disable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                        )
  {
    core::make_update_apply_builder<lps::data_expression_builder>(sigma)(x);
  }

  template <typename T, typename Substitution>
  T replace_variables(const T& x,
                      Substitution sigma,
                      typename boost::enable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                     )
  {   
    return core::make_update_apply_builder<lps::data_expression_builder>(sigma)(x);
  }

  template <typename T, typename Substitution>
  void replace_free_variables(T& x,
                              Substitution sigma,
                              typename boost::disable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                             )
  {
    data::detail::make_replace_free_variables_builder<lps::data_expression_builder, lps::add_data_variable_binding>(sigma)(x);
  }

  template <typename T, typename Substitution>
  T replace_free_variables(const T& x,
                           Substitution sigma,
                           typename boost::enable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                          )
  {
    return data::detail::make_replace_free_variables_builder<lps::data_expression_builder, lps::add_data_variable_binding>(sigma)(x);
  }

  template <typename T, typename Substitution, typename VariableContainer>
  void replace_free_variables(T& x,
                              Substitution sigma,
                              const VariableContainer& bound_variables,
                              typename boost::disable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                             )
  {
    data::detail::make_replace_free_variables_builder<lps::data_expression_builder, lps::add_data_variable_binding>(sigma)(x, bound_variables);
  }

  template <typename T, typename Substitution, typename VariableContainer>
  T replace_free_variables(const T& x,
                           Substitution sigma,
                           const VariableContainer& bound_variables,
                           typename boost::enable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
                          )
  {
    return data::detail::make_replace_free_variables_builder<lps::data_expression_builder, lps::add_data_variable_binding>(sigma)(x, bound_variables);
  }
#else
  template < typename Container, typename Substitution >
  Container replace_variables(Container const& container, Substitution substitution)
  {
    return data::replace_variables(container, substitution);
  }
  
  template <typename Container, typename Substitution >
  Container replace_free_variables(Container const& container, Substitution substitution)
  {
    return data::replace_free_variables(container, substitution);
  }

  template <typename Container, typename Substitution , typename VariableSequence >
  Container replace_free_variables(Container const& container, Substitution substitution, VariableSequence const& bound)
  {
    return data::replace_free_variables(container, substitution, bound);
  }
#endif // MCRL2_NEW_REPLACE_VARIABLES
//--- end generated lps replace code ---//

#ifndef MCRL2_NEW_REPLACE_VARIABLES
  template <typename Substitution>
  void replace_free_variables(multi_action& m, Substitution substitution)
  {
    m.actions() = lps::replace_free_variables(m.actions(), substitution);
    if (m.has_time())
    {
      m.time() = data::replace_free_variables(m.time(), substitution);
    }
  }
#endif // MCRL2_NEW_REPLACE_VARIABLES

} // namespace lps

} // namespace mcrl2

#endif // MCRL2_LPS_REPLACE_H
