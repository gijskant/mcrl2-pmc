// Author(s): Jeroen van der Wulp
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/selection.h
/// \brief Provides selection utility functionality

#ifndef MCRL2_DATA_SELECTION_H
#define MCRL2_DATA_SELECTION_H

#include <algorithm>

#include "mcrl2/data/data_equation.h"
#include "mcrl2/data/standard_utility.h"
#include "mcrl2/data/find.h"

namespace mcrl2
{

namespace data
{

/** \brief Component for selecting a subset of equations that are actually used in an encompassing specification
 *
 * This component can be used with the constructor of data::basic_rewriter
 * derived classes to select a smaller set of equations that are used as
 * rewrite rules. This limited set of rewrite rules should be enough for the
 * purpose of rewriting objects that occur in the encompassing
 * specification (the context).
 *
 * \note Use of this component can have a dramatic effect of rewriter
 * initialisation time and overall performance.
 **/
class used_data_equation_selector
{
  private:
    std::set< function_symbol > m_used_symbols;

    template < typename Range >
    void add_symbols(Range const& r)
    {
      m_used_symbols.insert(r.begin(), r.end());
    }

  protected:
    void add_data_specification_symbols(const data_specification& specification)
    {
      // Add all constructors of all sorts as they may be used when enumerating over these sorts
      atermpp::set< sort_expression > sorts(boost::copy_range< atermpp::set< sort_expression > >(specification.sorts()));
      for (atermpp::set< sort_expression>::const_iterator j = sorts.begin(); j != sorts.end(); ++j)
      {
        add_symbols(specification.constructors(*j));
      }

      std::set< data_equation > equations(boost::copy_range< std::set< data_equation > >(specification.equations()));

      std::map< data_equation, std::set< function_symbol > > symbols_for_equation;

      for (std::set< data_equation >::const_iterator i = equations.begin(); i != equations.end(); ++i)
      {
        std::set< function_symbol > used_symbols;

        data::detail::make_find_function_symbols_traverser<data::data_expression_traverser>(std::inserter(used_symbols, used_symbols.end()))(i->lhs());

        symbols_for_equation[*i].swap(used_symbols);
      }

      for (std::set< data_equation >::size_type n = 0, m = equations.size(); n != m; n = m, m = equations.size())
      {
        for (std::set< data_equation >::iterator i = equations.begin(); i != equations.end();)
        {
          if (std::includes(m_used_symbols.begin(), m_used_symbols.end(), symbols_for_equation[*i].begin(), symbols_for_equation[*i].end()))
          {
            data::detail::make_find_function_symbols_traverser<data::data_expression_traverser>(std::inserter(m_used_symbols, m_used_symbols.end()))(i->rhs());
            data::detail::make_find_function_symbols_traverser<data::data_expression_traverser>(std::inserter(m_used_symbols, m_used_symbols.end()))(i->condition());
            equations.erase(i++);
          }
          else
          {
            ++i;
          }
        }
      }
    }

  public:

    /// \brief Check whether data equation relates to used symbols, and therefore is important.
    bool operator()(data_equation const& e) const
    {
      std::set< function_symbol > used_symbols;

      data::detail::make_find_function_symbols_traverser<data::data_expression_traverser>(std::inserter(used_symbols, used_symbols.end()))(e.lhs());

      return std::includes(m_used_symbols.begin(), m_used_symbols.end(), used_symbols.begin(), used_symbols.end());
    }

    /// \brief context is a range of function symbols
    template <typename Range>
    used_data_equation_selector(data_specification const& data_spec, Range const& context)
    {
      add_symbols(context);
      add_data_specification_symbols(data_spec);
    }

    used_data_equation_selector(const data_specification& specification,
                                const std::set<function_symbol>& function_symbols,
                                const atermpp::set<data::variable>& global_variables
                               )
    {
      // Compensate for symbols that could be used as part of an instantiation of free variables
      for (atermpp::set<data::variable>::const_iterator j = global_variables.begin(); j != global_variables.end(); ++j)
      {
        add_symbols(specification.constructors(j->sort()));
        add_symbols(specification.mappings(j->sort()));
      }
      m_used_symbols.insert(function_symbols.begin(), function_symbols.end());
      add_data_specification_symbols(specification);
    }

    /// \brief select all equations
    used_data_equation_selector(const data_specification& specification)
    {
      add_symbols(specification.constructors());
      add_symbols(specification.mappings());
    }
};

} // namespace data

} // namespace mcrl2

#endif //MCRL2_DATA_UTILITY_H

