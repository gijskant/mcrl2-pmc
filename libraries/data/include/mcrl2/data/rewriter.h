// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/rewriter.h
/// \brief The class rewriter.

#ifndef MCRL2_DATA_REWRITER_H
#define MCRL2_DATA_REWRITER_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <sstream>

#include "boost/shared_ptr.hpp"
#include "boost/type_traits/add_reference.hpp"

#include "mcrl2/utilities/exception.h"
#include "mcrl2/data/expression_traits.h"
#include "mcrl2/data/detail/rewrite.h"
#include "mcrl2/data/detail/data_expression_with_variables.h"
#include "mcrl2/data/detail/data_expression_with_variables_traits.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/replace.h"
#include "mcrl2/data/substitutions/mutable_indexed_substitution.h"

namespace mcrl2
{

namespace data
{

/// \brief Rewriter class for the mCRL2 Library. It only works for terms of type data_expression
/// and data_expression_with_variables.
template < typename Term >
class basic_rewriter
{
  public:
    /// \brief The type for the substitution that is used internally.
    typedef data::mutable_indexed_substitution<> substitution_type;

  protected:
    /// \brief The wrapped Rewriter.
    boost::shared_ptr<detail::Rewriter> m_rewriter;

  public:

    /// \brief The term type of the rewriter.
    typedef Term term_type;

    /// \brief The type for expressions manipulated by the rewriter.
    typedef Term expression_type;

    /// \brief The rewrite strategies of the rewriter.
    typedef rewrite_strategy strategy;

    /// \brief The rewriter needs to find fresh names for
    //         variables. This name generator can be used
    //         for other renaming purposes as well.
    data::set_identifier_generator& identifier_generator()
    {
      return m_rewriter->identifier_generator();
    }


  protected:

    /// \brief Constructor.
    /// \param[in] r A rewriter
    basic_rewriter(const boost::shared_ptr<detail::Rewriter> & r) :
      m_rewriter(r)
    {}

    /// \brief Copy Constructor
    basic_rewriter(const basic_rewriter &other) :
      m_rewriter(other.m_rewriter)
    {}

    /// \brief Constructor.
    basic_rewriter(const data_specification & d, const used_data_equation_selector &equation_selector, const strategy s = jitty) :
      m_rewriter(detail::createRewriter(d, equation_selector, static_cast< rewrite_strategy >(s)))
    {}

};

/// \brief Rewriter class for the mCRL2 Library. It only works for terms of type data_expression
/// and data_expression_with_variables.
template <>
class basic_rewriter< data_expression > : public basic_rewriter< atermpp::aterm >
{
    template < typename CompatibleExpression >
    friend class basic_rewriter;
    friend class enumerator;

  public:
    /// \brief The term type of the rewriter.
    typedef data_expression                                     expression_type;
    /// \brief The variable type of the rewriter.
    typedef core::term_traits< expression_type >::variable_type variable_type;


    typedef basic_rewriter< data_expression >::substitution_type substitution_type;

  protected:

    /// \brief Copy constructor for conversion between derived types
    template < typename CompatibleExpression >
    basic_rewriter(const basic_rewriter< CompatibleExpression > & other) :
      basic_rewriter< data_expression >(other)
    { }

  public:

    /// \brief Constructor.
    /// \param[in] other A rewriter
    basic_rewriter(const basic_rewriter & other) :
      basic_rewriter< atermpp::aterm >(other)
    { }

    /// \brief Constructor.
    /// \param[in] d A data specification
    /// \param[in] s A rewriter strategy.
    basic_rewriter(const data_specification& d, const strategy s = jitty) :
      basic_rewriter< atermpp::aterm >(d,used_data_equation_selector(d),s)
    { }

    /// \brief Constructor.
    /// \param[in] d A data specification
    /// \param[in] s A rewriter strategy.
    /// \param[in] selector A component that selects the equations that are converted to rewrite rules
    template < typename EquationSelector >
    basic_rewriter(const data_specification& d, const EquationSelector& selector, const strategy s = jitty) :
      basic_rewriter< atermpp::aterm >(d,selector,s)
    { }
};

/// \brief Rewriter that operates on data expressions.
//
/// \attention As long as normalisation of sorts remains necessary, the data
/// specification object used for construction *must* exist during the
/// lifetime of the rewriter object.

class rewriter: public basic_rewriter<data_expression>
{
  public:
    typedef basic_rewriter<data_expression>::substitution_type substitution_type;

    /// \brief Constructor.
    /// \param[in] r a rewriter.
    rewriter(const rewriter& r) :
      basic_rewriter<data_expression>(r)
    { }

    /// \brief Constructor.
    /// \param[in] d A data specification
    /// \param[in] s A rewriter strategy.
    rewriter(const data_specification& d = rewriter::default_specification(), const strategy s = jitty) :
      basic_rewriter<data_expression>(d, s)
    { }

    /// \brief Constructor.
    /// \param[in] d A data specification
    /// \param[in] selector A component that selects the equations that are converted to rewrite rules
    /// \param[in] s A rewriter strategy.
    template < typename EquationSelector >
    rewriter(const data_specification& d, const EquationSelector& selector, const strategy s = jitty) :
      basic_rewriter<data_expression>(d, selector, s)
    {
    }

    /// \brief Default specification used if no specification is specified at construction
    static data_specification& default_specification()
    {
      static data_specification specification;
      return specification;
    }

    /// \brief Rewrites a data expression.
    /// \param[in] d A data expression
    /// \return The normal form of d.
    data_expression operator()(const data_expression& d) const
    {
      substitution_type sigma;
#ifdef MCRL2_PRINT_REWRITE_STEPS
      mCRL2log(log::debug) << "REWRITE: " << d;
#endif
      data_expression result(m_rewriter->rewrite(d,sigma));

#ifdef MCRL2_PRINT_REWRITE_STEPS
      mCRL2log(log::debug) << " ------------> " << result << std::endl;
#endif
      return result;
    }

    /// \brief Rewrites the data expression d, and on the fly applies a substitution function
    /// to data variables.
    /// \param[in] d A data expression
    /// \param[in] sigma A substitution function
    /// \return The normal form of the term.
    template <typename SubstitutionFunction>
    data_expression operator()(const data_expression& d, const SubstitutionFunction& sigma) const
    {
# ifdef MCRL2_PRINT_REWRITE_STEPS
      mCRL2log(log::debug) << "REWRITE " << d << "\n";
#endif
      substitution_type sigma_with_iterator;
      std::set < variable > free_variables=data::find_free_variables(d);
      for(std::set < variable >::const_iterator it=free_variables.begin(); it!=free_variables.end(); ++it)
      {
        sigma_with_iterator[*it]=sigma(*it);
      }

      data_expression result(m_rewriter->rewrite(d,sigma_with_iterator));

# ifdef MCRL2_PRINT_REWRITE_STEPS
      mCRL2log(log::debug) << " ------------> " << result << std::endl;
#endif
      return result;
    }

    /// \brief Rewrites the data expression d, and on the fly applies a substitution function
    /// to data variables.
    /// \param[in] d A data expression
    /// \param[in] sigma A substitution function
    /// \return The normal form of the term.
    //  Added bij JFG, to avoid the use of find_free_variables in the function operator() with
    //  an arbitrary SubstitionFunction, as this is prohibitively costly.

    data_expression operator()(const data_expression& d, substitution_type& sigma) const
    {
# ifdef MCRL2_PRINT_REWRITE_STEPS
      mCRL2log(log::debug) << "REWRITE " << d << "\n";
      data_expression result(m_rewriter->rewrite(d,sigma));
      mCRL2log(log::debug) << " ------------> " << result << std::endl;
      return result;
#else
      return m_rewriter->rewrite(d,sigma);
#endif
    }
    /// \brief Rewrites the data expression d, and on the fly applies a substitution function
    /// to data variables.
    /// \param[in] d A data expression
    /// \param[in] sigma A substitution function
    /// \return The normal form of the term.
    //  Added bij JFG, to avoid the use of find_free_variables in the function operator() with
    //  an arbitrary SubstitionFunction, as this is prohibitively costly.

    data_expression operator()(const data_expression& d, const substitution_type& sigma) const
    {
      substitution_type sigma1(sigma);
# ifdef MCRL2_PRINT_REWRITE_STEPS
      mCRL2log(log::debug) << "REWRITE " << d << "\n";
      data_expression result(m_rewriter->rewrite(d,sigma1));
      mCRL2log(log::debug) << " ------------> " << result << std::endl;
      return result;
#else
      return m_rewriter->rewrite(d,sigma1);
#endif
    }
};

/// \brief Rewriter that operates on data expressions.
class rewriter_with_variables: public basic_rewriter<data_expression>
{
    /// \brief The term type of the rewriter.
    typedef data_expression term_type;

    /// \brief The variable type of the rewriter.
    typedef core::term_traits< data_expression_with_variables >::variable_type variable_type;

  public:

    /// \brief Constructor. The Rewriter object that is used internally will be shared with \p r.
    /// \param[in] r A data rewriter
    rewriter_with_variables(const basic_rewriter< data_expression >& r) :
      basic_rewriter<data_expression>(r)
    {}

    /// \brief Constructor.
    /// \param[in] d A data specification
    /// \param[in] s A rewriter strategy.
    rewriter_with_variables(const data_specification& d = rewriter::default_specification(), const strategy s = jitty) :
      basic_rewriter<data_expression>(d, s)
    { }

    /// \brief Constructor.
    /// \param[in] d A data specification
    /// \param[in] selector A component that selects the equations that are converted to rewrite rules
    /// \param[in] s A rewriter strategy.
    template < typename EquationSelector >
    rewriter_with_variables(const data_specification& d, const EquationSelector & selector, const strategy s = jitty) :
      basic_rewriter<data_expression>(d, selector, s)
    { }


    /// \brief Rewrites a data expression.
    /// \param[in] d The term to be rewritten.
    /// \return The normal form of d.
    data_expression_with_variables operator()(const data_expression_with_variables& d) const
    {
      substitution_type sigma;
      data_expression t = m_rewriter->rewrite(d,sigma);
      data_expression_with_variables result(t, find_free_variables(t));
#ifdef MCRL2_PRINT_REWRITE_STEPS
      mCRL2log(log::debug) << "REWRITE " << d << " ------------> " << result << std::endl;
#endif
      return result;
    }

    /// \brief Rewrites the data expression d, and on the fly applies a substitution function
    /// to data variables.
    /// \param[in] d A term.
    /// \param[in] sigma A substitution function
    /// \return The normal form of the term.
    template <typename SubstitutionFunction>
    data_expression_with_variables operator()(const data_expression_with_variables& d, const SubstitutionFunction& sigma) const
    {
      substitution_type sigma_with_iterator;
      std::set < variable > free_variables=data::find_free_variables(d);
      for(std::set < variable >::const_iterator it=free_variables.begin(); it!=free_variables.end(); ++it)
      {
        sigma_with_iterator[*it]=sigma(*it);
      }

      data_expression t(m_rewriter->rewrite(static_cast< const data_expression& >(d),sigma_with_iterator));
      data_expression_with_variables result(t, find_free_variables(t));
#ifdef MCRL2_PRINT_REWRITE_STEPS
      mCRL2log(log::debug) << "REWRITE " << d << " ------------> " << result << std::endl;
#endif
      return result;
    }
};


/// \brief Function object for converting a data rewriter into a rewriter that
/// applies a substitution.
template <typename DataRewriter, typename Substitution>
struct rewriter_adapter
{
  const DataRewriter& R_;
  const Substitution& sigma_;

  rewriter_adapter(const DataRewriter& R, const Substitution& sigma)
    : R_(R), sigma_(sigma)
  {}

  data::data_expression operator()(const data::data_expression& t) const
  {
    return R_(t, sigma_);
  }
};

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_REWRITER_H
