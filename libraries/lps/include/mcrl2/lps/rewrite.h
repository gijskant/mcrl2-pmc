// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lps/include/mcrl2/lps/rewrite.h
/// \brief add your file description here.

#ifndef LPS_INCLUDE_MCRL2_LPS_REWRITE_H
#define LPS_INCLUDE_MCRL2_LPS_REWRITE_H

#include "mcrl2/data/rewrite.h"
#include "mcrl2/lps/builder.h"

namespace mcrl2 {

namespace lps {

//--- start generated lps rewrite code ---//
/// \brief Rewrites all embedded expressions in an object x
/// \param x an object containing expressions
/// \param R a rewriter
template <typename T, typename Rewriter>
void rewrite(T& x,
             Rewriter R,
             typename boost::disable_if<typename boost::is_base_of<atermpp::aterm, T>::type>::type* = 0
            )
{
  data::detail::make_rewrite_data_expressions_builder<lps::data_expression_builder>(R)(x);
}

/// \brief Rewrites all embedded expressions in an object x
/// \param x an object containing expressions
/// \param R a rewriter
/// \return the rewrite result
template <typename T, typename Rewriter>
T rewrite(const T& x,
          Rewriter R,
          typename boost::enable_if<typename boost::is_base_of<atermpp::aterm, T>::type>::type* = 0
         )
{
  return data::detail::make_rewrite_data_expressions_builder<lps::data_expression_builder>(R)(x);
}

/// \brief Rewrites all embedded expressions in an object x, and applies a substitution to variables on the fly
/// \param x an object containing expressions
/// \param R a rewriter
/// \param sigma a substitution
template <typename T, typename Rewriter, typename Substitution>
void rewrite(T& x,
             Rewriter R,
             Substitution sigma,
             typename boost::disable_if<typename boost::is_base_of<atermpp::aterm, T>::type>::type* = 0
            )
{
  data::detail::make_rewrite_data_expressions_with_substitution_builder<lps::data_expression_builder>(R, sigma)(x);
}

/// \brief Rewrites all embedded expressions in an object x, and applies a substitution to variables on the fly
/// \param x an object containing expressions
/// \param R a rewriter
/// \param sigma a substitution
/// \return the rewrite result
template <typename T, typename Rewriter, typename Substitution>
T rewrite(const T& x,
          Rewriter R,
          Substitution sigma,
          typename boost::enable_if<typename boost::is_base_of<atermpp::aterm, T>::type>::type* = 0
         )
{
  return data::detail::make_rewrite_data_expressions_with_substitution_builder<lps::data_expression_builder>(R, sigma)(x);
}
//--- end generated lps rewrite code ---//

} // namespace lps

} // namespace mcrl2

#endif // LPS_INCLUDE_MCRL2_LPS_REWRITE_H
