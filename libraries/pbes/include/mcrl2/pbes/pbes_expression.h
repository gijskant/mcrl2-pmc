// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/pbes_expression.h
/// \brief The class pbes_expression.

#ifndef MCRL2_PBES_PBES_EXPRESSION_H
#define MCRL2_PBES_PBES_EXPRESSION_H

#include <iterator>
#include "mcrl2/atermpp/aterm_access.h"
#include "mcrl2/atermpp/set.h"
#include "mcrl2/core/detail/struct.h"
#include "mcrl2/core/detail/constructors.h"
#include "mcrl2/core/detail/join.h"
#include "mcrl2/core/detail/optimized_logic_operators.h"
#include "mcrl2/data/data_variable.h"
#include "mcrl2/pbes/propositional_variable.h"

namespace mcrl2 {

namespace pbes_system {

// prototype
inline
bool is_bes(atermpp::aterm_appl t);

/// \brief pbes expression
///
// <PBExpr>       ::= <DataExpr>
//                  | PBESTrue
//                  | PBESFalse
//                  | PBESNot(<PBExpr>)
//                  | PBESAnd(<PBExpr>, <PBExpr>)
//                  | PBESOr(<PBExpr>, <PBExpr>)
//                  | PBESImp(<PBExpr>, <PBExpr>)
//                  | PBESForall(<DataVarId>+, <PBExpr>)
//                  | PBESExists(<DataVarId>+, <PBExpr>)
//                  | <PropVarInst>
class pbes_expression: public atermpp::aterm_appl
{
  public:
    /// Constructor.
    ///
    pbes_expression()
      : atermpp::aterm_appl(core::detail::constructPBExpr())
    {}

    /// Constructor.
    ///
    pbes_expression(atermpp::aterm_appl term)
      : atermpp::aterm_appl(term)
    {
      assert(core::detail::check_rule_PBExpr(m_term));
    }

    /// Constructor.
    ///             
    /// \param term A term.
    pbes_expression(ATermAppl term)
      : atermpp::aterm_appl(term)
    {
      assert(core::detail::check_rule_PBExpr(m_term));
    }

    /// Applies a substitution to this pbes expression and returns the result.
    /// The Substitution object must supply the method atermpp::aterm operator()(atermpp::aterm).
    ///
    template <typename Substitution>
    pbes_expression substitute(Substitution f) const
    {
      return pbes_expression(f(*this));
    }     

    /// Returns true if the expression is a boolean expression.
    ///
    bool is_bes() const
    {
      return mcrl2::pbes_system::is_bes(*this);
    }   
};

/// \brief singly linked list of data expressions
///
typedef atermpp::term_list<pbes_expression> pbes_expression_list;

/// \brief The namespace for predicates on pbes expressions.
namespace pbes_expr {

  /// \brief Returns true if the term t is equal to true
  inline bool is_pbes_true(pbes_expression t) { return core::detail::gsIsPBESTrue(t); }

  /// \brief Returns true if the term t is equal to false
  inline bool is_pbes_false(pbes_expression t) { return core::detail::gsIsPBESFalse(t); }

  /// \brief Returns true if the term t is a not expression
  inline bool is_pbes_not(pbes_expression t) { return core::detail::gsIsPBESNot(t); }

  /// \brief Returns true if the term t is an and expression
  inline bool is_pbes_and(pbes_expression t) { return core::detail::gsIsPBESAnd(t); }

  /// \brief Returns true if the term t is an or expression
  inline bool is_pbes_or(pbes_expression t) { return core::detail::gsIsPBESOr(t); }

  /// \brief Returns true if the term t is an imp expression
  inline bool is_pbes_imp(pbes_expression t) { return core::detail::gsIsPBESImp(t); }

  /// \brief Returns true if the term t is a universal quantification
  inline bool is_pbes_forall(pbes_expression t) { return core::detail::gsIsPBESForall(t); }

  /// \brief Returns true if the term t is an existential quantification
  inline bool is_pbes_exists(pbes_expression t) { return core::detail::gsIsPBESExists(t); }

  /// \brief Returns true if the term t is equal to true
  inline bool is_true(pbes_expression t) { return is_pbes_true(t) || data::data_expr::is_true(t); }

  /// \brief Returns true if the term t is equal to false
  inline bool is_false(pbes_expression t) { return is_pbes_false(t) || data::data_expr::is_false(t); }

  /// \brief Returns true if the term t is a not expression
  inline bool is_not(pbes_expression t) { return is_pbes_not(t) || data::data_expr::is_not(t); }

  /// \brief Returns true if the term t is an and expression
  inline bool is_and(pbes_expression t) { return is_pbes_and(t) || data::data_expr::is_and(t); }

  /// \brief Returns true if the term t is an or expression
  inline bool is_or(pbes_expression t) { return is_pbes_or(t) || data::data_expr::is_or(t); }

  /// \brief Returns true if the term t is an imp expression
  inline bool is_imp(pbes_expression t) { return is_pbes_imp(t) || data::data_expr::is_imp(t); }

  /// \brief Returns true if the term t is a universal quantification
  inline bool is_forall(pbes_expression t) { return is_pbes_forall(t); }

  /// \brief Returns true if the term t is an existential quantification
  inline bool is_exists(pbes_expression t) { return is_pbes_exists(t); }

  /// \brief Returns true if the term t is a data expression
  inline bool is_data(pbes_expression t) { return core::detail::gsIsDataExpr(t); }

  /// \brief Returns true if the term t is a propositional variable expression
  inline bool is_propositional_variable_instantiation(pbes_expression t) { return core::detail::gsIsPropVarInst(t); }

} // namespace pbes_expr

/// \brief The namespace for accessor functions on pbes expressions.
namespace accessors {

  /// Conversion of a pbes expression to a data expression.
  /// \pre The pbes expression must be of the form val(d) for
  /// some data variable d.
  inline
  data::data_expression val(pbes_expression t)
  {
    assert(core::detail::gsIsDataExpr(t));
    return atermpp::aterm_appl(t);
  }

  /// \brief Returns the pbes expression argument of expressions of type not,
  /// exists and forall.
  inline
  pbes_expression arg(pbes_expression t)
  {
    if (pbes_expr::is_pbes_not(t))
    {
      return atermpp::arg1(t);
    }
    assert(data::data_expr::is_not(t) ||
           pbes_expr::is_forall(t)    ||
           pbes_expr::is_exists(t)
          );
    return atermpp::arg2(t); 
  }

  /// \brief Returns the left hand side of an expression of type and, or or imp.
  inline
  pbes_expression left(pbes_expression t)
  {
    assert(pbes_expr::is_and(t) || pbes_expr::is_or(t) || pbes_expr::is_imp(t));
    return atermpp::arg1(t);
  }
  
  /// \brief Returns the right hand side of an expression of type and, or or imp.
  inline
  pbes_expression right(pbes_expression t)
  {
    assert(pbes_expr::is_and(t) || pbes_expr::is_or(t) || pbes_expr::is_imp(t));
    return atermpp::arg2(t);
  }
  
  /// \brief Returns the variables of a quantification expression
  inline
  data::data_variable_list var(pbes_expression t)
  {
    assert(pbes_expr::is_forall(t) || pbes_expr::is_exists(t));
    return atermpp::list_arg1(t);
  }
  
  /// \brief Returns the name of a propositional variable expression
  inline
  core::identifier_string name(pbes_expression t)
  {
    assert(pbes_expr::is_propositional_variable_instantiation(t));
    return atermpp::arg1(t);
  }
  
  /// \brief Returns the parameters of a propositional variable instantiation.
  inline
  data::data_expression_list param(pbes_expression t)
  {
    assert(pbes_expr::is_propositional_variable_instantiation(t));
    return atermpp::list_arg2(t);
  }
} // accessors

/// Accessor functions and predicates for pbes expressions.
namespace pbes_expr {

  /// \brief Returns the expression true
  inline
  pbes_expression true_()
  {
    return pbes_expression(core::detail::gsMakePBESTrue());
  }

  /// \brief Returns the expression false
  inline
  pbes_expression false_()
  {
    return pbes_expression(core::detail::gsMakePBESFalse());
  }

  /// \brief Returns not applied to p
  inline
  pbes_expression not_(pbes_expression p)
  {
    return pbes_expression(core::detail::gsMakePBESNot(p));
  }
  
  /// \brief Returns and applied to p and q
  inline
  pbes_expression and_(pbes_expression p, pbes_expression q)
  {
    return pbes_expression(core::detail::gsMakePBESAnd(p,q));
  }
  
  /// \brief Returns or applied to p and q
  inline
  pbes_expression or_(pbes_expression p, pbes_expression q)
  {
    return pbes_expression(core::detail::gsMakePBESOr(p,q));
  }
  
  /// \brief Returns imp applied to p and q
  inline
  pbes_expression imp(pbes_expression p, pbes_expression q)
  {
    return pbes_expression(core::detail::gsMakePBESImp(p,q));
  } 
  
  /// \brief Returns the universal quantification of the expression p over the variables in l.
  inline
  pbes_expression forall(data::data_variable_list l, pbes_expression p)
  {
    if (l.empty())
    {
      return p;
    }
    return pbes_expression(core::detail::gsMakePBESForall(l, p));
  }
  
  /// \brief Returns the existential quantification of the expression p over the variables in l.
  inline
  pbes_expression exists(data::data_variable_list l, pbes_expression p)
  {
    if (l.empty())
    {
      return p;
    }
    return pbes_expression(core::detail::gsMakePBESExists(l, p));
  }

  /// \brief Returns or applied to the sequence of pbes expressions [first, last[
  template <typename FwdIt>
  pbes_expression join_or(FwdIt first, FwdIt last)
  {
    return core::detail::join(first, last, or_, false_());
  }
  
  /// \brief Returns and applied to the sequence of pbes expressions [first, last[
  template <typename FwdIt>
  pbes_expression join_and(FwdIt first, FwdIt last)
  {
    return core::detail::join(first, last, and_, true_());
  }

  /// Given a pbes expression of the form p1 || p2 || .... || pn, this will yield a 
  /// set of the form { p1, p2, ..., pn }, assuming that pi does not have a || as main 
  /// function symbol.
  inline
  atermpp::set<pbes_expression> split_or(const pbes_expression& expr)
  {
    using namespace accessors;
    atermpp::set<pbes_expression> result;
    core::detail::split(expr, std::insert_iterator<atermpp::set<pbes_expression> >(result, result.begin()), is_or, left, right);
    return result;
  }
  
  /// Given a pbes expression of the form p1 && p2 && .... && pn, this will yield a 
  /// set of the form { p1, p2, ..., pn }, assuming that pi does not have a && as main 
  /// function symbol.
  inline
  atermpp::set<pbes_expression> split_and(const pbes_expression& expr)
  {
    using namespace accessors;
    atermpp::set<pbes_expression> result;
    core::detail::split(expr, std::insert_iterator<atermpp::set<pbes_expression> >(result, result.begin()), is_and, left, right);
    return result;
  }
} // namespace pbes_expr

namespace pbes_expr_optimized {
  using pbes_expr::is_pbes_true;
  using pbes_expr::is_pbes_false;
  using pbes_expr::is_pbes_not;
  using pbes_expr::is_pbes_and;
  using pbes_expr::is_pbes_or;
  using pbes_expr::is_pbes_imp;
  using pbes_expr::is_pbes_forall;
  using pbes_expr::is_pbes_exists;
  using pbes_expr::is_true;
  using pbes_expr::is_false;
  using pbes_expr::is_not;
  using pbes_expr::is_and;
  using pbes_expr::is_or;
  using pbes_expr::is_imp;
  using pbes_expr::is_forall;
  using pbes_expr::is_exists;
  using pbes_expr::is_data;
  using pbes_expr::is_propositional_variable_instantiation;
  using pbes_expr::true_;
  using pbes_expr::false_;
  using pbes_expr::split_and;
  using pbes_expr::split_or;

  /// \brief Returns not applied to p, and simplifies the result.
  inline
  pbes_expression not_(pbes_expression p)
  {
    return core::detail::optimized_not(p, pbes_expr::not_, true_(), is_true, false_(), is_false);
  }
  
  /// \brief Returns and applied to p and q, and simplifies the result.
  inline
  pbes_expression and_(pbes_expression p, pbes_expression q)
  {
    return core::detail::optimized_and(p, q, pbes_expr::and_, true_(), is_true, false_(), is_false);
  }
  
  /// \brief Returns or applied to p and q, and simplifies the result.
  inline
  pbes_expression or_(pbes_expression p, pbes_expression q)
  {
    return core::detail::optimized_or(p, q, pbes_expr::or_, true_(), is_true, false_(), is_false);
  }
  
  /// \brief Returns imp applied to p and q, and simplifies the result.
  inline
  pbes_expression imp(pbes_expression p, pbes_expression q)
  {
    return core::detail::optimized_imp(p, q, pbes_expr::imp, not_, true_(), is_true, false_(), is_false);
  }

  /// \brief Returns or applied to the sequence of pbes expressions [first, last[
  template <typename FwdIt>
  inline pbes_expression join_or(FwdIt first, FwdIt last)
  {
    return core::detail::join(first, last, or_, false_());
  }
  
  /// \brief Returns and applied to the sequence of pbes expressions [first, last[
  template <typename FwdIt>
  inline pbes_expression join_and(FwdIt first, FwdIt last)
  {
    return core::detail::join(first, last, and_, true_());
  }

  /// \brief Returns the universal quantification of the expression p over the variables in l.
  /// If l is empty, p is returned.
  inline
  pbes_expression forall(data::data_variable_list l, pbes_expression p)
  {
    if (l.empty())
    {
      return p;
    }
    if (is_false(p))
    {
      // N.B. Here we use the fact that mCRL2 data types are never empty.
      return data::data_expr::false_();
    }
    if (is_true(p))
    {
      return true_();
    }
    return pbes_expr::forall(l, p);
  }
  
  /// \brief Returns the existential quantification of the expression p over the variables in l.
  /// If l is empty, p is returned.
  inline
  pbes_expression exists(data::data_variable_list l, pbes_expression p)
  {
    if (l.empty())
    {
      return p;
    }
    if (is_false(p))
    {
      return data::data_expr::false_();
    }
    if (is_true(p))
    {
      // N.B. Here we use the fact that mCRL2 data types are never empty.
      return data::data_expr::true_();
    }
    return pbes_expr::exists(l, p);
  }

} // namespace pbes_expr_optimized

  /// \brief Returns true if the pbes expression t is a boolean expression
  inline
  bool is_bes(atermpp::aterm_appl t)
  {
    using namespace pbes_expr;
    using namespace accessors;
  
    if(is_pbes_and(t)) {
      return is_bes(left(t)) && is_bes(right(t));
    }
    else if(is_pbes_or(t)) {
      return is_bes(left(t)) && is_bes(right(t));
    }
    else if(is_pbes_forall(t)) {
      return false;
    }
    else if(is_pbes_exists(t)) {
      return false;
    }
    else if(is_propositional_variable_instantiation(t)) {
      return propositional_variable_instantiation(t).parameters().empty();
    }
    else if(is_pbes_true(t)) {
      return true;
    }
    else if(is_pbes_false(t)) {
      return true;
    }
  
    return false;
  }

} // namespace pbes_system

} // namespace mcrl2

/// \cond INTERNAL_DOCS
MCRL2_ATERM_TRAITS_SPECIALIZATION(mcrl2::pbes_system::pbes_expression)
/// \endcond

namespace mcrl2 {

namespace core {

  /// \brief Contains type information for pbes expressions.
  template <>
  struct term_traits<pbes_system::pbes_expression>
  {
    /// \brief The term type
    typedef pbes_system::pbes_expression term_type;

    /// \brief The data term type
    typedef data::data_expression data_term_type;

    /// \brief The data term sequence type
    typedef data::data_expression_list data_term_sequence_type;

    /// \brief The variable type
    typedef data::data_variable variable_type;

    /// \brief The variable sequence type
    typedef data::data_variable_list variable_sequence_type;

    /// \brief The propositional variable declaration type
    typedef pbes_system::propositional_variable propositional_variable_decl_type;   

    /// \brief The propositional variable instantiation type
    typedef pbes_system::propositional_variable_instantiation propositional_variable_type;   

    /// \brief The string type
    typedef core::identifier_string string_type;
   
    /// \brief The value true
    /// \return The value true
    static inline
    term_type true_()
    {
      return core::detail::gsMakePBESTrue();
    }
    
    /// \brief The value false
    /// \return The value false
    static inline
    term_type false_()
    {
      return core::detail::gsMakePBESFalse();
    }
    
    /// \brief Operator not
    /// \param p A term
    /// \return Operator not applied to p
    static inline
    term_type not_(term_type p)
    {
      return core::detail::gsMakePBESNot(p);
    }
    
    /// \brief Operator and
    /// \param p A term
    /// \param q A term
    /// \return Operator and applied to p and q
    static inline
    term_type and_(term_type p, term_type q)
    {
      return core::detail::gsMakePBESAnd(p,q);
    }
    
    /// \brief Operator or
    /// \param p A term
    /// \param q A term
    /// \return Operator or applied to p and q
    static inline
    term_type or_(term_type p, term_type q)
    {
      return core::detail::gsMakePBESOr(p,q);
    }
    
    /// \brief Implication
    /// \param p A term
    /// \param q A term
    /// \return Implication applied to p and q
    static inline
    term_type imp(term_type p, term_type q)
    {
      return core::detail::gsMakePBESImp(p,q);
    }
    
    /// \brief Universal quantification
    /// \param l A sequence of variables
    /// \param p A term
    /// \return Universal quantification of p over the variables l
    static inline
    term_type forall(variable_sequence_type l, term_type p)
    {
      if (l.empty())
      {
        return p;
      }
      return core::detail::gsMakePBESForall(l, p);
    }
    
    /// \brief Existential quantification
    /// \param l A sequence of variables
    /// \param p A term
    /// \return Existential quantification of p over the variables l
    static inline
    term_type exists(variable_sequence_type l, term_type p)
    {
      if (l.empty())
      {
        return p;
      }
      return core::detail::gsMakePBESExists(l, p);
    }

    /// \brief Propositional variable instantiation
    /// \param name A string
    /// \param first Start of a sequence of data terms
    /// \param last End of a sequence of data terms
    /// \return Propositional variable instantiation with the given name, and the range [first, last) as data parameters
    template <typename Iter>
    static
    term_type prop_var(const string_type& name, Iter first, Iter last)
    {
      return propositional_variable_type(name, data_term_sequence_type(first, last));
    }
      
    /// \brief Test for value true
    /// \param t A term
    /// \return True if the term has the value true. Also works for data terms
    static inline
    bool is_true(term_type t)
    {
      return core::detail::gsIsPBESTrue(t) || core::detail::gsIsDataExprTrue(t);
    }
    
    /// \brief Test for value false
    /// \param t A term
    /// \return True if the term has the value false. Also works for data terms
    static inline 
    bool is_false(term_type t)
    {
      return core::detail::gsIsPBESFalse(t) || core::detail::gsIsDataExprFalse(t);
    }
    
    /// \brief Test for operator not
    /// \param t A term
    /// \return True if the term is of type and. Also works for data terms
    static inline 
    bool is_not(term_type t)
    {
      return core::detail::gsIsPBESNot(t) || core::detail::gsIsDataExprNot(t);
    }

    /// \brief Test for operator and
    /// \param t A term
    /// \return True if the term is of type and. Also works for data terms
    static inline 
    bool is_and(term_type t)
    {
      return core::detail::gsIsPBESAnd(t) || core::detail::gsIsDataExprAnd(t);
    }
    
    /// \brief Test for operator or
    /// \param t A term
    /// \return True if the term is of type or. Also works for data terms
    static inline 
    bool is_or(term_type t)
    {
      return core::detail::gsIsPBESOr(t) || core::detail::gsIsDataExprOr(t);
    }
    
    /// \brief Test for implication
    /// \param t A term
    /// \return True if the term is an implication. Also works for data terms
    static inline 
    bool is_imp(term_type t)
    {
      return core::detail::gsIsPBESImp(t) || core::detail::gsIsDataExprImp(t);
    }
    
    /// \brief Test for universal quantification
    /// \param t A term
    /// \return True if the term is an universal quantification. Also works for data terms
    static inline 
    bool is_forall(term_type t)
    {
      return core::detail::gsIsPBESForall(t) || core::detail::gsIsDataExprForall(t);
    }
    
    /// \brief Test for existential quantification
    /// \param t A term
    /// \return True if the term is an existential quantification. Also works for data terms
    static inline 
    bool is_exists(term_type t)
    {
      return core::detail::gsIsPBESExists(t) || core::detail::gsIsDataExprExists(t);
    }
    
    /// \brief Test for data term
    /// \param t A term
    /// \return True if the term is a data term
    static inline 
    bool is_data(term_type t)
    {
      return core::detail::gsIsDataExpr(t);
    }
    
    /// \brief Test for propositional variable instantiation
    /// \param t A term
    /// \return True if the term is a propositional variable instantiation
    static inline 
    bool is_prop_var(term_type t)
    {
      return core::detail::gsIsPropVarInst(t);
    }

    /// \brief Returns the argument of a term of type not, exists or forall
    /// \param t A term
    /// \return The requested argument. Partially works for data terms
    static inline
    term_type arg(term_type t)
    {
      // Forall and exists are not fully supported by the data library
      assert(!core::detail::gsIsDataExprForall(t) && !core::detail::gsIsDataExprExists(t));
      assert(is_not(t) || is_exists(t) || is_forall(t));
      
      if (core::detail::gsIsPBESNot(t))
      {
        return atermpp::arg1(t);
      }
      else
      {
        return atermpp::arg2(t);
      }
    }
    
    /// \brief Returns the left argument of a term of type and, or or imp
    /// \param t A term
    /// \return The left argument of the term. Also works for data terms
    static inline
    term_type left(term_type t)
    {
      assert(is_and(t) || is_or(t) || is_imp(t));
      return atermpp::arg1(t);
    }
    
    /// \brief Returns the right argument of a term of type and, or or imp
    /// \param t A term
    /// \return The right argument of the term. Also works for data terms
    static inline
    term_type right(term_type t)
    {
      assert(is_and(t) || is_or(t) || is_imp(t));
      return atermpp::arg2(t);
    }
    
    /// \brief Returns the quantifier variables of a quantifier expression
    /// \param t A term
    /// \return The requested argument. Doesn't work for data terms
    static inline
    variable_sequence_type var(term_type t)
    {
      // Forall and exists are not fully supported by the data library
      assert(!core::detail::gsIsDataExprForall(t) && !core::detail::gsIsDataExprExists(t));
      assert(is_exists(t) || is_forall(t));

      return atermpp::list_arg1(t);
    }
    
    /// \brief Returns the name of a propositional variable instantiation
    /// \param t A term
    /// \return The name of the propositional variable instantiation
    static inline
    string_type name(term_type t)
    {
      assert(is_prop_var(t));
      return atermpp::arg1(t);
    }      
    
    /// \brief Returns the parameter list of a propositional variable instantiation
    /// \param t A term
    /// \return The parameter list of the propositional variable instantiation
    static inline
    data_term_sequence_type param(term_type t)
    {
      assert(is_prop_var(t));
      return atermpp::list_arg2(t);
    }

    /// \brief Conversion from variable to term
    /// \param v A variable
    /// \returns The converted variable
    static inline
    term_type variable2term(variable_type v)
    {
      return v;
    }

    /// \brief Conversion from data term to term
    /// \param t A data term
    /// \returns The converted term
    static inline
    term_type dataterm2term(data_term_type t)
    {
      return t;
    }  

    /// \brief Conversion from term to data term
    /// \param t A term
    /// \returns The converted term
    static inline
    data_term_type term2dataterm(term_type t)
    {
      return t;
    }

    /// \brief Test if a term is constant
    /// \param t A term
    /// \return True if the term is constant
    static inline
    bool is_constant(term_type t) { return false; }

    /// \brief Pretty print function
    /// \param t A term
    /// \brief Returns a pretty print representation of the term
    static inline
    std::string pp(term_type t)
    {
      return core::pp(t);
    }
  };

} // namespace core

} // namespace mcrl2

#endif // MCRL2_PBES_PBES_EXPRESSION_H
