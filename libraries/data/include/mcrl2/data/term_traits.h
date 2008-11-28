// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/term_traits.h
/// \brief Contains term traits for data_expression and data_expression_with_variables.

#ifndef MCRL2_DATA_TERM_TRAITS_H
#define MCRL2_DATA_TERM_TRAITS_H

#include "mcrl2/core/term_traits.h"
#include "mcrl2/data/data_expression.h"
#include "mcrl2/data/data_expression_with_variables.h"
#include "mcrl2/data/data_variable.h"
#include "mcrl2/data/find.h"

namespace mcrl2 {

namespace core {

  /// \brief Contains type information for data expressions.
  template <>
  struct term_traits<data::data_expression>
  {
    /// \brief The term type
    typedef data::data_expression term_type;

    /// \brief The variable type
    typedef data::data_variable variable_type;

    /// \brief The variable sequence type
    typedef data::data_variable_list variable_sequence_type;
    
    /// \return The value true
    static inline
    term_type true_() { return data::data_expr::true_(); }
    
    /// \return The value false
    static inline
    term_type false_() { return data::data_expr::false_(); }
    
    /// \param p A term
    /// \return Operator not applied to p
    static inline
    term_type not_(term_type p) { return data::data_expr::not_(p); }
    
    /// \param p A term
    /// \param q A term
    /// \return Operator and applied to p and q
    static inline
    term_type and_(term_type p, term_type q) { return data::data_expr::and_(p, q); }
    
    /// \param p A term
    /// \param q A term
    /// \return Operator or applied to p and q
    static inline
    term_type or_(term_type p, term_type q) { return data::data_expr::or_(p, q); }
    
    /// \param t A term
    /// \return True if the term has the value true
    static inline
    bool is_true(term_type t) { return data::data_expr::is_true(t); }
    
    /// \param t A term
    /// \return True if the term has the value false
    static inline 
    bool is_false(term_type t) { return data::data_expr::is_false(t); }
    
    /// \param t A term
    /// \return True if the term is of type not
    static inline 
    bool is_not(term_type t) { return data::data_expr::is_not(t); }
    
    /// \param t A term
    /// \return True if the term is of type and
    static inline 
    bool is_and(term_type t) { return data::data_expr::is_and(t); }
    
    /// \param t A term
    /// \return True if the term is of type or
    static inline 
    bool is_or(term_type t) { return data::data_expr::is_or(t); }
    
    /// \param t A term
    /// \return True if the term is an implication
    static inline 
    bool is_imp(term_type t) { return data::data_expr::is_imp(t); }
    
    /// \param t A term
    /// \return True if the term is an universal quantification
    static inline 
    bool is_forall(term_type t) { return data::data_expr::is_forall(t); }
    
    /// \param t A term
    /// \return True if the term is an existential quantification
    static inline 
    bool is_exists(term_type t) { return data::data_expr::is_exists(t); }

    /// \param v A variable
    /// \brief Converts a variable to a term
    static inline
    term_type variable2term(variable_type v)
    {
      return v;
    }

    /// \param t A term
    /// \return True if the term is constant
    static inline
    bool is_constant(term_type t)
    {
      return find_all_data_variables(t).empty();
    }

    /// \param t A term
    /// \brief Returns a pretty print representation of the term
    static inline
    std::string pp(term_type t)
    {
      return core::pp(t);
    }
  };

  /// \brief Contains type information for data expressions with variables.
  template <>
  struct term_traits<data::data_expression_with_variables>
  {
    /// \brief The term type
    typedef data::data_expression_with_variables term_type;

    /// \brief The variable type
    typedef data::data_variable variable_type;

    /// \brief The variable sequence type
    typedef data::data_variable_list variable_sequence_type;
    
    /// \return The value true
    static inline
    term_type true_() { return data::data_expr::true_(); }
    
    /// \return The value false
    static inline
    term_type false_() { return data::data_expr::false_(); }

    /// \param p A term
    /// \return Operator not applied to p
    static inline
    term_type not_(term_type p) { return term_type(data::data_expr::not_(p), p.variables()); }
    
    /// \param p A term
    /// \param q A term
    /// \return Operator and applied to p and q
    static inline
    term_type and_(term_type p, term_type q)
    {
      return term_type(data::data_expr::and_(p, q), data::data_variable_list_union(p.variables(), q.variables()));
    }
    
    /// \param p A term
    /// \param q A term
    /// \return Operator or applied to p and q
    static inline
    term_type or_(term_type p, term_type q)
    {
      return term_type(data::data_expr::or_(p, q), data::data_variable_list_union(p.variables(), q.variables()));
    }
    
    /// \param t A term
    /// \return True if the term has the value true
    static inline
    bool is_true(term_type t) { return data::data_expr::is_true(t); }
    
    /// \param t A term
    /// \return True if the term has the value false
    static inline 
    bool is_false(term_type t) { return data::data_expr::is_false(t); }
    
    /// \param t A term
    /// \return True if the term is of type not
    static inline 
    bool is_not(term_type t) { return data::data_expr::is_not(t); }
    
    /// \param t A term
    /// \return True if the term is of type and
    static inline 
    bool is_and(term_type t) { return data::data_expr::is_and(t); }
    
    /// \param t A term
    /// \return True if the term is of type or
    static inline 
    bool is_or(term_type t) { return data::data_expr::is_or(t); }
    
    /// \param t A term
    /// \return True if the term is an implication
    static inline 
    bool is_imp(term_type t) { return data::data_expr::is_imp(t); }
    
    /// \param t A term
    /// \return True if the term is an universal quantification
    static inline 
    bool is_forall(term_type t) { return data::data_expr::is_forall(t); }
    
    /// \param t A term
    /// \return True if the term is an existential quantification
    static inline 
    bool is_exists(term_type t) { return data::data_expr::is_exists(t); }

    /// \param v A variable
    /// \brief Converts a variable to a term
    static inline
    term_type variable2term(variable_type v)
    {
      return term_type(v, atermpp::make_list(v));
    }

    /// \param t A term
    /// \return True if the term is constant
    static inline
    bool is_constant(term_type t)
    {
      return t.variables().empty();
    }

    /// \param t A term
    /// \brief Returns a pretty print representation of the term
    static inline
    std::string pp(term_type t)
    {
      return core::pp(t) + " " + core::pp(t.variables());
    }
  };
  
} // namespace core

} // namespace mcrl2

#endif // MCRL2_DATA_TERM_TRAITS_H
