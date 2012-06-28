// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/normalize.h
/// \brief Normalization of pbes expressions.

#ifndef MCRL2_PBES_NORMALIZE_H
#define MCRL2_PBES_NORMALIZE_H

#include "mcrl2/utilities/exception.h"
#include "mcrl2/pbes/builder.h"
#include "mcrl2/pbes/traverser.h"
#include "mcrl2/pbes/pbes_equation.h"
#include "mcrl2/data/bool.h"

namespace mcrl2
{

namespace pbes_system
{

/// \cond INTERNAL_DOCS
// \brief Visitor for checking if a pbes expression is normalized.
struct is_normalized_traverser: public pbes_expression_traverser<is_normalized_traverser>
{
  typedef pbes_expression_traverser<is_normalized_traverser> super;
  using super::enter;
  using super::leave;
  using super::operator();

#if BOOST_MSVC
#include "mcrl2/core/detail/traverser_msvc.inc.h"
#endif

  bool result;

  is_normalized_traverser()
    : result(true)
  {}

  /// \brief Visit not node
  void enter(const not_& /* x */)
  {
    result = false;
  }

  /// \brief Visit imp node
  void enter(const imp& /* x */)
  {
    result = false;
  }
};
/// \endcond

/// \cond INTERNAL_DOCS
// \brief Visitor for checking if a pbes expression is normalized.
struct normalize_builder: public pbes_expression_builder<normalize_builder>
{
  typedef pbes_expression_builder<normalize_builder> super;
  using super::enter;
  using super::leave;
  using super::operator();

  typedef core::term_traits<pbes_expression> tr;

#if BOOST_MSVC
#include "mcrl2/core/detail/builder_msvc.inc.h"
#endif

  bool negated;

  normalize_builder()
    : negated(false)
  {}

  pbes_expression operator()(const data::data_expression& x)
  {
    return negated ? data::sort_bool::not_(x) : x;
  }

  pbes_expression operator()(const true_&)
  {
    return negated ? tr::false_() : tr::true_();
  }

  pbes_expression operator()(const false_&)
  {
    return negated ? tr::true_() : tr::false_();
  }

  pbes_expression operator()(const not_& x)
  {
    negated = !negated;
    pbes_expression result = super::operator()(x.operand());
    negated = !negated;
    return result;
  }

  pbes_expression operator()(const and_& x)
  {
    pbes_expression left = super::operator()(x.left());
    pbes_expression right = super::operator()(x.right());
    return negated ? tr::or_(left, right) : tr::and_(left, right);
  }

  pbes_expression operator()(const or_& x)
  {
    pbes_expression left = super::operator()(x.left());
    pbes_expression right = super::operator()(x.right());
    return negated ? tr::and_(left, right) : tr::or_(left, right);
  }

  pbes_expression operator()(const imp& x)
  {
    negated = !negated;
    pbes_expression left = super::operator()(x.left());
    negated = !negated;
    pbes_expression right = super::operator()(x.right());
    return negated ? tr::and_(left, right) : tr::or_(left, right);
  }

  pbes_expression operator()(const forall& x)
  {
    pbes_expression body = super::operator()(x.body());
    return negated ? tr::exists(x.variables(), body) : tr::forall(x.variables(), body);
  }

  pbes_expression operator()(const exists& x)
  {
    pbes_expression body = super::operator()(x.body());
    return negated ? tr::forall(x.variables(), body) : tr::exists(x.variables(), body);
  }

  pbes_expression operator()(const propositional_variable_instantiation& x)
  {
    if (negated)
    {
      throw mcrl2::runtime_error(std::string("normalize error: illegal argument ") + x.to_string());
    }
    return x;
  }
};
/// \endcond

/// \brief Checks if a pbes expression is normalized
/// \param t A PBES expression
/// \return True if the pbes expression is normalized
template <typename T>
bool is_normalized(const T& x)
{
  is_normalized_traverser f;
  f(x);
  return f.result;
}

/// \brief The function normalize brings (embedded) pbes expressions into positive normal form,
/// i.e. a formula without any occurrences of ! or =>.
/// \param x an object containing pbes expressions
template <typename T>
void normalize(T& x,
               typename boost::disable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
              )
{
  normalize_builder f;
  f(x);
}

/// \brief The function normalize brings (embedded) pbes expressions into positive normal form,
/// i.e. a formula without any occurrences of ! or =>.
/// \param x an object containing pbes expressions
template <typename T>
T normalize(const T& x,
            typename boost::enable_if<typename boost::is_base_of<atermpp::aterm_base, T>::type>::type* = 0
           )
{
  normalize_builder f;
  return f(x);
}

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_NORMALIZE_H
