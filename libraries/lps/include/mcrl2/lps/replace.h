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

#include "mcrl2/data/substitutions/mutable_map_substitution.h"
#include "mcrl2/process/replace.h"
#include "mcrl2/lps/find.h"
#include "mcrl2/process/replace.h"
#include "mcrl2/lps/add_binding.h"
#include "mcrl2/lps/builder.h"

namespace mcrl2
{

namespace lps
{

namespace detail {

/// \cond INTERNAL_DOCS
template <template <class> class Builder, class Derived, class Substitution>
struct add_capture_avoiding_replacement: public process::detail::add_capture_avoiding_replacement<Builder, Derived, Substitution>
{
  typedef process::detail::add_capture_avoiding_replacement<Builder, Derived, Substitution> super;
  using super::enter;
  using super::leave;
  using super::operator();
  using super::sigma;
  using super::update_sigma;

  add_capture_avoiding_replacement(Substitution& sigma, std::multiset<data::variable>& V)
    : super(sigma, V)
  { }

  template <typename ActionSummand>
  void do_action_summand(ActionSummand& x, const data::variable_list& v)
  {
    x.summation_variables() = v;
    x.condition() = (*this)(x.condition());
    (*this)(x.multi_action());
    x.assignments() = (*this)(x.assignments());
  }

  void operator()(action_summand& x)
  {
    data::variable_list v = update_sigma.push(x.summation_variables());
    do_action_summand(x, v);
    update_sigma.pop(v);
  }

  void operator()(stochastic_action_summand& x)
  {
    data::variable_list v = update_sigma.push(x.summation_variables());
    do_action_summand(x, v);
    x.distribution() = (*this)(x.distribution());
    update_sigma.pop(v);
  }

  void operator()(deadlock_summand& x)
  {
    data::variable_list v = update_sigma.push(x.summation_variables());
    x.summation_variables() = v;
    x.condition() = (*this)(x.condition());
    (*this)(x.deadlock());
    update_sigma.pop(v);
  }

  template <typename LinearProcess>
  void do_linear_process(const LinearProcess& x)
  {
    data::variable_list v = update_sigma.push(x.process_parameters());
    x.process_parameters() = v;
    (*this)(x.action_summands());
    (*this)(x.deadlock_summands());
    update_sigma.pop(v);
  }

  void operator()(linear_process& x)
  {
    do_linear_process(x);
  }

  void operator()(stochastic_linear_process& x)
  {
    do_linear_process(x);
  }

  template <typename Specification>
  void do_specification(Specification& x)
  {
    std::set<data::variable> v = update_sigma(x.global_variables());
    x.global_variables() = v;
    (*this)(x.process());
    x.action_labels() = (*this)(x.action_labels());
    x.initial_process() = (*this)(x.initial_process());
    update_sigma.pop(v);
  }

  void operator()(specification& x)
  {
    do_specification(x);
  }

  void operator()(stochastic_specification& x)
  {
    do_specification(x);
  }

  stochastic_distribution operator()(stochastic_distribution& x)
  {
    data::variable_list v = update_sigma.push(x.variables());
    stochastic_distribution result(x.variables(), (*this)(x.distribution()));
    update_sigma.pop(v);
    return result;
  }

#ifdef BOOST_MSVC
#include "mcrl2/core/detail/builder_msvc.inc.h"
#endif
};

} // namespace detail

//--- start generated lps replace code ---//
template <typename T, typename Substitution>
void replace_sort_expressions(T& x,
                              const Substitution& sigma,
                              bool innermost,
                              typename std::enable_if<!std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                             )
{
  data::detail::make_replace_sort_expressions_builder<lps::sort_expression_builder>(sigma, innermost)(x);
}

template <typename T, typename Substitution>
T replace_sort_expressions(const T& x,
                           const Substitution& sigma,
                           bool innermost,
                           typename std::enable_if<std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                          )
{
  return data::detail::make_replace_sort_expressions_builder<lps::sort_expression_builder>(sigma, innermost)(x);
}

template <typename T, typename Substitution>
void replace_data_expressions(T& x,
                              const Substitution& sigma,
                              bool innermost,
                              typename std::enable_if<!std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                             )
{
  data::detail::make_replace_data_expressions_builder<lps::data_expression_builder>(sigma, innermost)(x);
}

template <typename T, typename Substitution>
T replace_data_expressions(const T& x,
                           const Substitution& sigma,
                           bool innermost,
                           typename std::enable_if<std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                          )
{
  return data::detail::make_replace_data_expressions_builder<lps::data_expression_builder>(sigma, innermost)(x);
}

template <typename T, typename Substitution>
void replace_variables(T& x,
                       const Substitution& sigma,
                       typename std::enable_if<!std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                      )
{
  core::make_update_apply_builder<lps::data_expression_builder>(sigma)(x);
}

template <typename T, typename Substitution>
T replace_variables(const T& x,
                    const Substitution& sigma,
                    typename std::enable_if<std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                   )
{
  return core::make_update_apply_builder<lps::data_expression_builder>(sigma)(x);
}

template <typename T, typename Substitution>
void replace_all_variables(T& x,
                           const Substitution& sigma,
                           typename std::enable_if<!std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                          )
{
  core::make_update_apply_builder<lps::variable_builder>(sigma)(x);
}

template <typename T, typename Substitution>
T replace_all_variables(const T& x,
                        const Substitution& sigma,
                        typename std::enable_if<std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                       )
{
  return core::make_update_apply_builder<lps::variable_builder>(sigma)(x);
}

/// \brief Applies the substitution sigma to x.
/// \pre { The substitution sigma must have the property that FV(sigma(x)) is included in {x} for all variables x. }
template <typename T, typename Substitution>
void replace_free_variables(T& x,
                            const Substitution& sigma,
                            typename std::enable_if<!std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                           )
{
  assert(data::is_simple_substitution(sigma));
  data::detail::make_replace_free_variables_builder<lps::data_expression_builder, lps::add_data_variable_binding>(sigma)(x);
}

/// \brief Applies the substitution sigma to x.
/// \pre { The substitution sigma must have the property that FV(sigma(x)) is included in {x} for all variables x. }
template <typename T, typename Substitution>
T replace_free_variables(const T& x,
                         const Substitution& sigma,
                         typename std::enable_if<std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                        )
{
  assert(data::is_simple_substitution(sigma));
  return data::detail::make_replace_free_variables_builder<lps::data_expression_builder, lps::add_data_variable_binding>(sigma)(x);
}

/// \brief Applies the substitution sigma to x, where the elements of bound_variables are treated as bound variables.
/// \pre { The substitution sigma must have the property that FV(sigma(x)) is included in {x} for all variables x. }
template <typename T, typename Substitution, typename VariableContainer>
void replace_free_variables(T& x,
                            const Substitution& sigma,
                            const VariableContainer& bound_variables,
                            typename std::enable_if<!std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                           )
{
  assert(data::is_simple_substitution(sigma));
  data::detail::make_replace_free_variables_builder<lps::data_expression_builder, lps::add_data_variable_binding>(sigma)(x, bound_variables);
}

/// \brief Applies the substitution sigma to x, where the elements of bound_variables are treated as bound variables.
/// \pre { The substitution sigma must have the property that FV(sigma(x)) is included in {x} for all variables x. }
template <typename T, typename Substitution, typename VariableContainer>
T replace_free_variables(const T& x,
                         const Substitution& sigma,
                         const VariableContainer& bound_variables,
                         typename std::enable_if<std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                        )
{
  assert(data::is_simple_substitution(sigma));
  return data::detail::make_replace_free_variables_builder<lps::data_expression_builder, lps::add_data_variable_binding>(sigma)(x, bound_variables);
}
//--- end generated lps replace code ---//

//--- start generated lps replace_capture_avoiding code ---//
/// \brief Applies sigma as a capture avoiding substitution to x
/// \param sigma A mutable substitution
/// \param sigma_variables a container of variables
/// \pre { sigma_variables must contain the free variables appearing in the right hand side of sigma }
template <typename T, typename Substitution, typename VariableContainer>
void replace_variables_capture_avoiding(T& x,
                       Substitution& sigma,
                       const VariableContainer& sigma_variables,
                       typename std::enable_if<!std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                      )
{
  std::multiset<data::variable> V;
  lps::find_free_variables(x, std::inserter(V, V.end()));
  V.insert(sigma_variables.begin(), sigma_variables.end());
  data::detail::apply_replace_capture_avoiding_variables_builder<lps::data_expression_builder, lps::detail::add_capture_avoiding_replacement>(sigma, V)(x);
}

/// \brief Applies sigma as a capture avoiding substitution to x
/// \param sigma A mutable substitution
/// \param sigma_variables a container of variables
/// \pre { sigma_variables must contain the free variables appearing in the right hand side of sigma }
template <typename T, typename Substitution, typename VariableContainer>
T replace_variables_capture_avoiding(const T& x,
                    Substitution& sigma,
                    const VariableContainer& sigma_variables,
                    typename std::enable_if<std::is_base_of<atermpp::aterm, T>::value>::type* = 0
                   )
{
  std::multiset<data::variable> V;
  lps::find_free_variables(x, std::inserter(V, V.end()));
  V.insert(sigma_variables.begin(), sigma_variables.end());
  return data::detail::apply_replace_capture_avoiding_variables_builder<lps::data_expression_builder, lps::detail::add_capture_avoiding_replacement>(sigma, V)(x);
}
//--- end generated lps replace_capture_avoiding code ---//

namespace detail {

/// \cond INTERNAL_DOCS
template <template <class> class Builder, template <template <class> class, class> class Binder, class Substitution>
struct replace_process_parameter_builder: public Binder<Builder, replace_process_parameter_builder<Builder, Binder, Substitution> >
{
  typedef Binder<Builder, replace_process_parameter_builder<Builder, Binder, Substitution> > super;
  using super::enter;
  using super::leave;
  using super::operator();
  using super::is_bound;
  using super::bound_variables;
  using super::increase_bind_count;

  Substitution sigma;
  std::size_t count; // the bind count

  replace_process_parameter_builder(Substitution sigma_)
    : sigma(sigma_), count(1)
  {}

  template <typename VariableContainer>
  replace_process_parameter_builder(Substitution sigma_, const VariableContainer& bound_variables)
    : sigma(sigma_), count(1)
  {
    increase_bind_count(bound_variables);
  }

  data::data_expression operator()(const data::variable& x)
  {
    if (bound_variables().count(x) == count)
    {
      return sigma(x);
    }
    return x;
  }

  data::assignment_expression operator()(const data::assignment& x)
  {
    data::variable lhs = atermpp::down_cast<data::variable>((*this)(x.lhs()));
    data::data_expression rhs = (*this)(x.rhs());
    return data::assignment(lhs, rhs);
  }

  void operator()(lps::deadlock_summand& x)
  {
    count = 1;
    super::operator()(x);
  }

  void operator()(lps::action_summand& x)
  {
    count = 1;
    super::operator()(x);
    x.assignments() = super::operator()(x.assignments());
  }

  lps::process_initializer operator()(const lps::process_initializer& x)
  {
    count = 0;
    lps::process_initializer result = super::operator()(x);
    return lps::process_initializer(super::operator()(result.assignments()));
  }

  void operator()(lps::linear_process& x)
  {
    super::operator()(x);
    count = 0;
    x.process_parameters() = super::operator()(x.process_parameters());
  }

#ifdef BOOST_MSVC
#include "mcrl2/core/detail/builder_msvc.inc.h"
#endif
};

template <template <class> class Builder, template <template <class> class, class> class Binder, class Substitution>
replace_process_parameter_builder<Builder, Binder, Substitution>
make_replace_process_parameters_builder(Substitution sigma)
{
  return replace_process_parameter_builder<Builder, Binder, Substitution>(sigma);
}
/// \endcond

} // namespace detail

/// \brief Applies a substitution to the process parameters of the specification spec.
template <typename Substitution>
void replace_process_parameters(specification& spec, Substitution sigma)
{
  lps::detail::make_replace_process_parameters_builder<lps::data_expression_builder, lps::add_data_variable_binding>(sigma)(spec);
}

/// \brief Applies a substitution to the process parameters of the specification spec.
inline
void replace_summand_variables(specification& spec, data::mutable_map_substitution<>& sigma)
{
  std::set<data::variable> sigma_variables = data::substitution_variables(sigma);
  action_summand_vector& a = spec.process().action_summands();
  for (action_summand_vector::iterator i = a.begin(); i != a.end(); ++i)
  {
    i->summation_variables() = data::replace_variables(i->summation_variables(), sigma);
    i->condition() = data::replace_variables_capture_avoiding(i->condition(), sigma, sigma_variables);
    lps::replace_variables_capture_avoiding(i->multi_action(), sigma, sigma_variables);
    i->assignments() = data::replace_variables_capture_avoiding(i->assignments(), sigma, sigma_variables);
  }
  deadlock_summand_vector& d = spec.process().deadlock_summands();
  for (deadlock_summand_vector::iterator i = d.begin(); i != d.end(); ++i)
  {
    i->summation_variables() = data::replace_variables(i->summation_variables(), sigma);
    i->condition() = data::replace_variables_capture_avoiding(i->condition(), sigma, sigma_variables);
    lps::replace_variables_capture_avoiding(i->deadlock(), sigma, sigma_variables);
  }
}

} // namespace lps

} // namespace mcrl2

#endif // MCRL2_LPS_REPLACE_H
