// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/enumerator.h
/// \brief The class enumerator.

#ifndef MCRL2_DATA_ENUMERATOR_H
#define MCRL2_DATA_ENUMERATOR_H

#include <functional>
#include <utility>
#include <boost/shared_ptr.hpp>
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/atermpp/set.h"
#include "mcrl2/atermpp/aterm_access.h"
#include "mcrl2/core/sequence.h"
#include "mcrl2/data/detail/data_expression_with_variables.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/substitute.h"
#include "mcrl2/data/identifier_generator.h"
#include "mcrl2/exception.h"

namespace mcrl2
{

namespace data
{

/// \cond INTERNAL_DOCS
namespace detail
{

struct data_enumerator_helper
{
  const data_expression_with_variables& e_;
  const atermpp::vector<data_expression_with_variables>& values_;
  atermpp::vector<data_expression_with_variables>& result_;

  data_enumerator_helper(const data_expression_with_variables& e,
                         const atermpp::vector<data_expression_with_variables>& values,
                         atermpp::vector<data_expression_with_variables>& result
                        )
    : e_(e), values_(values), result_(result)
  {}

  /// \brief Function call operator
  void operator()()
  {
    data_expression d = data::substitute_variables(static_cast<const data_expression&>(e_), data::make_sequence_sequence_substitution(e_.variables(), values_));

    // 9/8/2009. Changed line below from std::vector<variable> to atermpp::vector<variable> because it appears that
    // at times variables can occur only in this vector of variables, causing problems when garbage collected.
    // Jan Friso Groote.
    atermpp::vector<variable> v;
    for (atermpp::vector<data_expression_with_variables>::const_iterator i = values_.begin(); i != values_.end(); ++i)
    {
      v.insert(v.end(), i->variables().begin(), i->variables().end());
    }
    result_.push_back(data_expression_with_variables(d, variable_list(v.begin(), v.end())));
  }
};

} // namespace detail
/// \endcond

/// \brief Class for enumerating data expressions.
template <typename IdentifierGenerator = number_postfix_generator>
class data_enumerator
{
  protected:

    /// \brief A map that caches the constructors corresponding to sort expressions.
    typedef std::map<sort_expression, std::vector<function_symbol> > constructor_map;

    /// \brief A data specification.
    const data_specification* m_data;

    /// \brief A rewriter.
    const data::rewriter* m_rewriter;

    /// \brief An identifier generator.
    IdentifierGenerator* m_generator;

    /// \brief A mapping with constructors.
    mutable constructor_map m_constructors;

    /// \brief Returns the constructors with target s.
    /// \param s A sort expression
    /// \return The constructors corresponding to the sort expression.
    const std::vector<function_symbol>& constructors(sort_expression s) const
    {
      constructor_map::const_iterator i = m_constructors.find(s);
      if (i != m_constructors.end())
      {
        return i->second;
      }
      m_constructors[s] = boost::copy_range< std::vector<function_symbol> >(m_data->constructors(s));
      return m_constructors[s];
    }

  public:

    /// \brief The variable type of the enumerator.
    typedef variable variable_type;

    /// \brief The term type of the enumerator.
    typedef data_expression_with_variables term_type;

    /// \brief Constructor.
    /// \param data_spec A data specification.
    /// \param rewriter A rewriter.
    /// \param generator An identifier generator.
    data_enumerator(const data_specification& data_spec,
                    const data::rewriter& rewriter,
                    IdentifierGenerator& generator)
      : m_data(&data_spec), m_rewriter(&rewriter), m_generator(&generator)
    {}

    /// \brief The data specification.
    /// \return The data specification.
    const data_specification& data() const
    {
      return *m_data;
    }

    /// \brief Enumerates a data variable.
    /// \param v A data variable
    /// \return A sequence of expressions that is the result of applying the enumerator to the variable once.
    atermpp::vector<data_expression_with_variables> enumerate(const variable& v) const
    {
      // std::cerr << "Enumerate " << v << "\n";
      atermpp::vector<data_expression_with_variables> result;
      const std::vector<function_symbol>& c = constructors(v.sort());

      if (c.empty())
      {
        throw mcrl2::runtime_error("Could not enumerate variable " + core::pp(v) + " of sort " + core::pp(v.sort()) + " as there are no constructors.");
      }
      for (std::vector<function_symbol>::const_iterator i = c.begin(); i != c.end(); ++i)
      {
        if (is_function_sort(i->sort()))
        {
          atermpp::vector<variable> variables;

          for (boost::iterator_range< sort_expression_list::iterator > j(function_sort(i->sort()).domain()); !j.empty(); j.advance_begin(1))
          {
            variables.push_back(variable((*m_generator)(), j.front()));
          }

          variable_list w(atermpp::convert< variable_list >(variables));

          result.push_back(data_expression_with_variables(application(*i, atermpp::convert< data_expression_list >(w)), w));
        }
        else
        {
          result.push_back(data_expression_with_variables(data_expression(*i), variable_list()));
        }
      }
      /* for(atermpp::vector<data_expression_with_variables>::const_iterator i=result.begin();
               i!=result.end(); ++i)
      { std::cerr << "Enumerate result " << core::pp(*i) << "\n";
      } */
      return result;
    }

    /// \brief Enumerates a data expression. Only the variables of the enumerator
    /// expression are expanded. Fresh variables are created using the
    /// identifier generator that was passed in the constructor.
    /// \param e A data expression.
    /// \return A sequence of expressions that is the result of applying the enumerator to the expression once.
    atermpp::vector<data_expression_with_variables> enumerate(const data_expression_with_variables& e) const
    {
      atermpp::vector<data_expression_with_variables> result;

      // Compute the instantiations for each variable of e.
      std::vector<atermpp::vector<data_expression_with_variables> > enumerated_values;
      variable_list variables(e.variables());
      for (variable_list::const_iterator i = variables.begin(); i != variables.end(); ++i)
      {
        enumerated_values.push_back(enumerate(*i));
      }

      atermpp::vector<data_expression_with_variables> values(enumerated_values.size());

      core::foreach_sequence(enumerated_values, values.begin(), detail::data_enumerator_helper(e, values, result));
      return result;
    }
};

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_ENUMERATOR_H
