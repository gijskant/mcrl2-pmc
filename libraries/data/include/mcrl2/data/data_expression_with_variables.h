// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/data_expression_with_variables.h
/// \brief add your file description here.

#ifndef MCRL2_DATA_DATA_EXPRESSION_WITH_VARIABLES_H
#define MCRL2_DATA_DATA_EXPRESSION_WITH_VARIABLES_H

#include "mcrl2/data/data_expression.h"
#include "mcrl2/data/data_variable.h"
#include "mcrl2/data/find.h"

namespace mcrl2 {

namespace data {

  /// \brief Data expression with a sequence of variables attached to it
  ///
  /// The intended use case is to store the free variables of the expression.
  class data_expression_with_variables: public data_expression
  {
    protected:
      /// The attached variables.
      data_variable_list m_variables;
      
    public:
      /// Constructor.
      ///             
      data_expression_with_variables()
      {}
  
      /// Constructor.
      ///             
      /// \param term A term.
      data_expression_with_variables(atermpp::aterm_appl term)
        : data_expression(term)
      {
        std::set<data_variable> v = find_all_data_variables(*this);
        m_variables = data_variable_list(v.begin(), v.end());
      }
  
      /// Constructor.
      ///             
      /// \param term A term.
      data_expression_with_variables(ATermAppl term)
        : data_expression(term)
      {
        std::set<data_variable> v = find_all_data_variables(*this);
        m_variables = data_variable_list(v.begin(), v.end());
      }

      /// Constructor.
      ///             
      /// \param expression A data expression.
      /// \param variables A sequence of variables.
      data_expression_with_variables(data_expression expression, data_variable_list variables)
        : data_expression(expression), m_variables(variables)
      {}
      
      /// Return the variables.
      ///
      /// \return The variables of the data expression.
      data_variable_list variables() const
      {
        return m_variables;
      }
      
      /// Return the variables.
      ///
      /// \return The variables of the data expression.
      data_variable_list& variables()
      {
        return m_variables;
      }
      
      /// Returns true if the sequence of variables is empty.
      /// \return True if the sequence of variables is empty.
      bool is_constant() const
      {
        return m_variables.empty();
      }
  };

} // namespace data

} // namespace mcrl2

/// \cond INTERNAL_DOCS
MCRL2_ATERM_TRAITS_SPECIALIZATION(mcrl2::data::data_expression_with_variables)
/// \endcond

#endif // MCRL2_DATA_DATA_EXPRESSION_WITH_VARIABLES_H
