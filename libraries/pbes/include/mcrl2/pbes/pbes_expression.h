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

#define MCRL2_SMART_ARGUMENT_SORTING

#ifndef MCRL2_PBES_PBES_EXPRESSION_H
#define MCRL2_PBES_PBES_EXPRESSION_H

#include <iterator>
#include "mcrl2/atermpp/aterm_access.h"
#include "mcrl2/atermpp/set.h"
#include "mcrl2/core/detail/struct_core.h"
#include "mcrl2/core/detail/constructors.h"
#include "mcrl2/core/detail/join.h"
#include "mcrl2/core/detail/soundness_checks.h"
#include "mcrl2/core/detail/optimized_logic_operators.h"
#include "mcrl2/data/variable.h"
#include "mcrl2/data/bool.h"
#include "mcrl2/pbes/propositional_variable.h"
#include "mcrl2/pbes/detail/free_variable_visitor.h"
#include "mcrl2/pbes/detail/compare_pbes_expression_visitor.h"

namespace mcrl2 {

namespace pbes_system {

// prototype
inline
bool is_bes(atermpp::aterm_appl t);

/// \brief pbes expression
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
    /// \brief Constructor.
    pbes_expression()
      : atermpp::aterm_appl(core::detail::constructPBExpr())
    {}

    /// \brief Constructor.
    /// \param term A term
    pbes_expression(const atermpp::aterm_appl& term)
      : atermpp::aterm_appl(term)
    { assert(core::detail::check_rule_PBExpr(m_term));
    }

    ///// \brief Constructor.
    ///// \param term A term
    //pbes_expression(ATermAppl term)
    //  : atermpp::aterm_appl(term)
    //{
    //  assert(core::detail::check_rule_PBExpr(m_term));
    //}

    /// \brief Applies a low level substitution function to this term and returns the result.
    /// \param f A
    /// The function <tt>f</tt> must supply the method <tt>aterm operator()(aterm)</tt>.
    /// This function is applied to all <tt>aterm</tt> noded appearing in this term.
    /// \deprecated
    /// \return The substitution result.
    template <typename Substitution>
    pbes_expression substitute(Substitution f) const
    {
      return pbes_expression(f(*this));
    }

    /// \brief Returns true if the expression is a boolean expression.
    /// \return True if the expression is a boolean expression.
    bool is_bes() const
    {
      return mcrl2::pbes_system::is_bes(*this);
    }
};

/// \brief Read-only singly linked list of data expressions
typedef atermpp::term_list<pbes_expression> pbes_expression_list;

//--- start generated classes ---//
/// \brief A propositional variable instantiation
class propositional_variable_instantiation: public pbes_expression
{
  public:
    /// \brief Default constructor.
    propositional_variable_instantiation()
      : pbes_expression(core::detail::constructPropVarInst())
    {}

    /// \brief Constructor.
    /// \param term A term
    propositional_variable_instantiation(const atermpp::aterm_appl& term)
      : pbes_expression(term)
    {
      assert(core::detail::check_term_PropVarInst(m_term));
    }

    /// \brief Constructor.
    propositional_variable_instantiation(const core::identifier_string& name, const data::data_expression_list& parameters)
      : pbes_expression(core::detail::gsMakePropVarInst(name, parameters))
    {}

    /// \brief Constructor.
    propositional_variable_instantiation(const std::string& name, const data::data_expression_list& parameters)
      : pbes_expression(core::detail::gsMakePropVarInst(core::identifier_string(name), parameters))
    {}

    core::identifier_string name() const
    {
      return atermpp::arg1(*this);
    }

    data::data_expression_list parameters() const
    {
      return atermpp::list_arg2(*this);
    }
//--- start user section propositional_variable_instantiation ---//
    /// \brief Type of the parameters.
    typedef data::data_expression parameter_type;
    
    /// \brief Constructor.
    /// \param s A string
    propositional_variable_instantiation(std::string const& s)
    {
      std::pair<std::string, data::data_expression_list> p = data::detail::parse_variable(s);
      core::identifier_string name(p.first);
      data::variable_list parameters = atermpp::convert<data::variable_list>(p.second);
      m_term = reinterpret_cast<ATerm>(core::detail::gsMakePropVarInst(name, parameters));
    }
//--- end user section propositional_variable_instantiation ---//
};

/// \brief list of propositional_variable_instantiations
typedef atermpp::term_list<propositional_variable_instantiation> propositional_variable_instantiation_list;

/// \brief vector of propositional_variable_instantiations
typedef atermpp::vector<propositional_variable_instantiation>    propositional_variable_instantiation_vector;


/// \brief The value true for pbes expressions
class true_: public pbes_expression
{
  public:
    /// \brief Default constructor.
    true_()
      : pbes_expression(core::detail::constructPBESTrue())
    {}

    /// \brief Constructor.
    /// \param term A term
    true_(const atermpp::aterm_appl& term)
      : pbes_expression(term)
    {
      assert(core::detail::check_term_PBESTrue(m_term));
    }
};

/// \brief The value false for pbes expressions
class false_: public pbes_expression
{
  public:
    /// \brief Default constructor.
    false_()
      : pbes_expression(core::detail::constructPBESFalse())
    {}

    /// \brief Constructor.
    /// \param term A term
    false_(const atermpp::aterm_appl& term)
      : pbes_expression(term)
    {
      assert(core::detail::check_term_PBESFalse(m_term));
    }
};

/// \brief The not operator for pbes expressions
class not_: public pbes_expression
{
  public:
    /// \brief Default constructor.
    not_()
      : pbes_expression(core::detail::constructPBESNot())
    {}

    /// \brief Constructor.
    /// \param term A term
    not_(const atermpp::aterm_appl& term)
      : pbes_expression(term)
    {
      assert(core::detail::check_term_PBESNot(m_term));
    }

    /// \brief Constructor.
    not_(const pbes_expression& operand)
      : pbes_expression(core::detail::gsMakePBESNot(operand))
    {}

    pbes_expression operand() const
    {
      return atermpp::arg1(*this);
    }
};

/// \brief The and operator for pbes expressions
class and_: public pbes_expression
{
  public:
    /// \brief Default constructor.
    and_()
      : pbes_expression(core::detail::constructPBESAnd())
    {}

    /// \brief Constructor.
    /// \param term A term
    and_(const atermpp::aterm_appl& term)
      : pbes_expression(term)
    {
      assert(core::detail::check_term_PBESAnd(m_term));
    }

    /// \brief Constructor.
    and_(const pbes_expression& left, const pbes_expression& right)
      : pbes_expression(core::detail::gsMakePBESAnd(left, right))
    {}

    pbes_expression left() const
    {
      return atermpp::arg1(*this);
    }

    pbes_expression right() const
    {
      return atermpp::arg2(*this);
    }
};

/// \brief The or operator for pbes expressions
class or_: public pbes_expression
{
  public:
    /// \brief Default constructor.
    or_()
      : pbes_expression(core::detail::constructPBESOr())
    {}

    /// \brief Constructor.
    /// \param term A term
    or_(const atermpp::aterm_appl& term)
      : pbes_expression(term)
    {
      assert(core::detail::check_term_PBESOr(m_term));
    }

    /// \brief Constructor.
    or_(const pbes_expression& left, const pbes_expression& right)
      : pbes_expression(core::detail::gsMakePBESOr(left, right))
    {}

    pbes_expression left() const
    {
      return atermpp::arg1(*this);
    }

    pbes_expression right() const
    {
      return atermpp::arg2(*this);
    }
};

/// \brief The implication operator for pbes expressions
class imp: public pbes_expression
{
  public:
    /// \brief Default constructor.
    imp()
      : pbes_expression(core::detail::constructPBESImp())
    {}

    /// \brief Constructor.
    /// \param term A term
    imp(const atermpp::aterm_appl& term)
      : pbes_expression(term)
    {
      assert(core::detail::check_term_PBESImp(m_term));
    }

    /// \brief Constructor.
    imp(const pbes_expression& left, const pbes_expression& right)
      : pbes_expression(core::detail::gsMakePBESImp(left, right))
    {}

    pbes_expression left() const
    {
      return atermpp::arg1(*this);
    }

    pbes_expression right() const
    {
      return atermpp::arg2(*this);
    }
};

/// \brief The universal quantification operator for pbes expressions
class forall: public pbes_expression
{
  public:
    /// \brief Default constructor.
    forall()
      : pbes_expression(core::detail::constructPBESForall())
    {}

    /// \brief Constructor.
    /// \param term A term
    forall(const atermpp::aterm_appl& term)
      : pbes_expression(term)
    {
      assert(core::detail::check_term_PBESForall(m_term));
    }

    /// \brief Constructor.
    forall(const data::variable_list& variables, const pbes_expression& body)
      : pbes_expression(core::detail::gsMakePBESForall(variables, body))
    {}

    data::variable_list variables() const
    {
      return atermpp::list_arg1(*this);
    }

    pbes_expression body() const
    {
      return atermpp::arg2(*this);
    }
};

/// \brief The existential quantification operator for pbes expressions
class exists: public pbes_expression
{
  public:
    /// \brief Default constructor.
    exists()
      : pbes_expression(core::detail::constructPBESExists())
    {}

    /// \brief Constructor.
    /// \param term A term
    exists(const atermpp::aterm_appl& term)
      : pbes_expression(term)
    {
      assert(core::detail::check_term_PBESExists(m_term));
    }

    /// \brief Constructor.
    exists(const data::variable_list& variables, const pbes_expression& body)
      : pbes_expression(core::detail::gsMakePBESExists(variables, body))
    {}

    data::variable_list variables() const
    {
      return atermpp::list_arg1(*this);
    }

    pbes_expression body() const
    {
      return atermpp::arg2(*this);
    }
};
//--- end generated classes ---//

/// \brief The namespace for predicates on pbes expressions.
//namespace pbes_expr {

  /// \brief Returns true if the term t is equal to true
  /// \param t A PBES expression
  /// \return True if the term t is equal to true
  inline bool is_pbes_true(pbes_expression t)
  { return core::detail::gsIsPBESTrue(t); }

  /// \brief Returns true if the term t is equal to false
  /// \param t A PBES expression
  /// \return True if the term t is equal to false
  inline bool is_pbes_false(pbes_expression t)
  { return core::detail::gsIsPBESFalse(t); }

  /// \brief Returns true if the term t is a not expression
  /// \param t A PBES expression
  /// \return True if the term t is a not expression
  inline bool is_pbes_not(pbes_expression t)
  { return core::detail::gsIsPBESNot(t); }


  /// \brief Returns true if the term t is an and expression
  /// \param t A PBES expression
  /// \return True if the term t is an and expression
  inline bool is_pbes_and(pbes_expression t)
  { return core::detail::gsIsPBESAnd(t); }

  /// \brief Returns true if the term t is an or expression
  /// \param t A PBES expression
  /// \return True if the term t is an or expression
  inline bool is_pbes_or(pbes_expression t)
  { return core::detail::gsIsPBESOr(t); }

  /// \brief Returns true if the term t is an imp expression
  /// \param t A PBES expression
  /// \return True if the term t is an imp expression
  inline bool is_pbes_imp(pbes_expression t)
  { return core::detail::gsIsPBESImp(t); }

  /// \brief Returns true if the term t is a universal quantification
  /// \param t A PBES expression
  /// \return True if the term t is a universal quantification
  inline bool is_pbes_forall(pbes_expression t)
  { return core::detail::gsIsPBESForall(t); }

  /// \brief Returns true if the term t is an existential quantification
  /// \param t A PBES expression
  /// \return True if the term t is an existential quantification
  inline bool is_pbes_exists(pbes_expression t)
  { return core::detail::gsIsPBESExists(t); }

  /// \brief Test for the value true
  /// \param t A PBES expression
  /// \return True if it is the value \p true
  inline bool is_true(pbes_expression t)
  { return is_pbes_true(t) || data::sort_bool::is_true_function_symbol(t); }

  /// \brief Test for the value false
  /// \param t A PBES expression
  /// \return True if it is the value \p false
  inline bool is_false(pbes_expression t)
  { return is_pbes_false(t) || data::sort_bool::is_false_function_symbol(t); }

  /// \brief Test for a negation
  /// \param t A PBES expression
  /// \return True if it is a negation
  inline bool is_not(pbes_expression t)
  {
    return is_pbes_not(t);
  }

  /// \brief Test for a conjunction
  /// \param t A PBES expression
  /// \return True if it is a conjunction
  inline bool is_and(pbes_expression t)
  {
     return is_pbes_and(t);
  }

  /// \brief Test for a disjunction
  /// \param t A PBES expression
  /// \return True if it is a disjunction
  inline bool is_or(pbes_expression t)
  {
     return is_pbes_or(t);
  }

  /// \brief Test for an implication
  /// \param t A PBES expression
  /// \return True if it is an implication
  inline bool is_imp(pbes_expression t)
  {
     return is_pbes_imp(t);
  }

  /// \brief Test for an universal quantification
  /// \param t A PBES expression
  /// \return True if it is a universal quantification
  inline bool is_forall(pbes_expression t)
  { return is_pbes_forall(t); }

  /// \brief Test for an existential quantification
  /// \param t A PBES expression
  /// \return True if it is an existential quantification
  inline bool is_exists(pbes_expression t)
  { return is_pbes_exists(t); }

  /// \brief Returns true if the term t is a data expression
  /// \param t A PBES expression
  /// \return True if the term t is a data expression
  inline bool is_data(pbes_expression t)
  { return data::is_data_expression(t); }

  /// \brief Returns true if the term t is a propositional variable expression
  /// \param t A PBES expression
  /// \return True if the term t is a propositional variable expression
  inline bool is_propositional_variable_instantiation(pbes_expression t)
  { return core::detail::gsIsPropVarInst(t); }

//} // namespace pbes_expr

/// \brief The namespace for accessor functions on pbes expressions.
namespace accessors {

  /// \brief Conversion of a pbes expression to a data expression.
  /// \pre The pbes expression must be of the form val(d) for some data variable d.
  /// \param t A PBES expression
  /// \return The converted expression
  inline
  data::data_expression val(pbes_expression t)
  {
    assert(data::is_data_expression(t));
    return atermpp::aterm_appl(t);
  }

  /// \brief Returns the pbes expression argument of expressions of type not, exists and forall.
  /// \param t A PBES expression
  /// \return The pbes expression argument of expressions of type not, exists and forall.
  inline
  pbes_expression arg(pbes_expression t)
  {
    if (is_pbes_not(t))
    {
      return atermpp::arg1(t);
    }
    else
    {
      assert(is_forall(t) || is_exists(t));
      return atermpp::arg2(t);
    }
  }

  /// \brief Returns the left hand side of an expression of type and, or or imp.
  /// \param t A PBES expression
  /// \return The left hand side of an expression of type and, or or imp.
  inline
  pbes_expression left(pbes_expression t)
  {
    assert(is_and(t) || is_or(t) || is_imp(t));
    return atermpp::arg1(t);
  }

  /// \brief Returns the right hand side of an expression of type and, or or imp.
  /// \param t A PBES expression
  /// \return The right hand side of an expression of type and, or or imp.
  inline
  pbes_expression right(pbes_expression t)
  {
    return atermpp::arg2(t);
  }

  /// \brief Returns the variables of a quantification expression
  /// \param t A PBES expression
  /// \return The variables of a quantification expression
  inline
  data::variable_list var(pbes_expression t)
  {
    assert(is_forall(t) || is_exists(t));
    return data::variable_list(atermpp::term_list_iterator< data::variable >(atermpp::list_arg1(t)),
                                   atermpp::term_list_iterator< data::variable >());
  }

  /// \brief Returns the name of a propositional variable expression
  /// \param t A PBES expression
  /// \return The name of a propositional variable expression
  inline
  core::identifier_string name(pbes_expression t)
  {
    assert(is_propositional_variable_instantiation(t));
    return atermpp::arg1(t);
  }

  /// \brief Returns the parameters of a propositional variable instantiation.
  /// \param t A PBES expression
  /// \return The parameters of a propositional variable instantiation.
  inline
  data::data_expression_list param(pbes_expression t)
  {
    assert(is_propositional_variable_instantiation(t));
    return data::data_expression_list(atermpp::term_list_iterator< data::data_expression >(atermpp::list_arg2(t)),
                                          atermpp::term_list_iterator< data::data_expression >());
  }
} // accessors

/// Accessor functions and predicates for pbes expressions.
namespace pbes_expr {

  /// \brief Make the value true
  /// \return The value \p true
  inline
  pbes_expression true_()
  {
    return pbes_expression(core::detail::gsMakePBESTrue());
  }

  /// \brief Make the value false
  /// \return The value \p false
  inline
  pbes_expression false_()
  {
    return pbes_expression(core::detail::gsMakePBESFalse());
  }

  /// \brief Make a negation
  /// \param p A PBES expression
  /// \return The value <tt>!p</tt>
  inline
  pbes_expression not_(pbes_expression p)
  {
    return pbes_expression(core::detail::gsMakePBESNot(p));
  }

  /// \brief Make a conjunction
  /// \param p A PBES expression
  /// \param q A PBES expression
  /// \return The value <tt>p && q</tt>
  inline
  pbes_expression and_(pbes_expression p, pbes_expression q)
  {
    return pbes_expression(core::detail::gsMakePBESAnd(p,q));
  }

  /// \brief Make a disjunction
  /// \param p A PBES expression
  /// \param q A PBES expression
  /// \return The value <tt>p || q</tt>
  inline
  pbes_expression or_(pbes_expression p, pbes_expression q)
  {
    return pbes_expression(core::detail::gsMakePBESOr(p,q));
  }

  /// \brief Make an implication
  /// \param p A PBES expression
  /// \param q A PBES expression
  /// \return The value <tt>p => q</tt>
  inline
  pbes_expression imp(pbes_expression p, pbes_expression q)
  {
    return pbes_expression(core::detail::gsMakePBESImp(p,q));
  }

  /// \brief Make a universal quantification
  /// \param l A sequence of data variables
  /// \param p A PBES expression
  /// \return The value <tt>forall l.p</tt>
  inline
  pbes_expression forall(data::variable_list l, pbes_expression p)
  {
    if (l.empty())
    {
      return p;
    }
    return pbes_expression(core::detail::gsMakePBESForall(l, p));
  }

  /// \brief Make an existential quantification
  /// \param l A sequence of data variables
  /// \param p A PBES expression
  /// \return The value <tt>exists l.p</tt>
  inline
  pbes_expression exists(data::variable_list l, pbes_expression p)
  {
    if (l.empty())
    {
      return p;
    }
    return pbes_expression(core::detail::gsMakePBESExists(l, p));
  }

  /// \brief Returns or applied to the sequence of pbes expressions [first, last)
  /// \param first Start of a sequence of pbes expressions
  /// \param last End of a sequence of of pbes expressions
  /// \return Or applied to the sequence of pbes expressions [first, last)
  template <typename FwdIt>
  pbes_expression join_or(FwdIt first, FwdIt last)
  {
    return core::detail::join(first, last, or_, false_());
  }

  /// \brief Returns and applied to the sequence of pbes expressions [first, last)
  /// \param first Start of a sequence of pbes expressions
  /// \param last End of a sequence of of pbes expressions
  /// \return And applied to the sequence of pbes expressions [first, last)
  template <typename FwdIt>
  pbes_expression join_and(FwdIt first, FwdIt last)
  {
    return core::detail::join(first, last, and_, true_());
  }

  /// \brief Splits a disjunction into a sequence of operands
  /// Given a pbes expression of the form p1 || p2 || .... || pn, this will yield a
  /// set of the form { p1, p2, ..., pn }, assuming that pi does not have a || as main
  /// function symbol.
  /// \param expr A PBES expression
  /// \return A sequence of operands
  inline
  atermpp::set<pbes_expression> split_or(const pbes_expression& expr)
  {
    using namespace accessors;
    atermpp::set<pbes_expression> result;
    core::detail::split(expr, std::insert_iterator<atermpp::set<pbes_expression> >(result, result.begin()), is_or, left, right);
    return result;
  }

  /// \brief Splits a conjunction into a sequence of operands
  /// Given a pbes expression of the form p1 && p2 && .... && pn, this will yield a
  /// set of the form { p1, p2, ..., pn }, assuming that pi does not have a && as main
  /// function symbol.
  /// \param expr A PBES expression
  /// \return A sequence of operands
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
  using pbes_expr::true_;
  using pbes_expr::false_;
  using pbes_expr::split_and;
  using pbes_expr::split_or;

  /// \brief Make a negation
  /// \param p A PBES expression
  /// \return The value <tt>!p</tt>
  inline
  pbes_expression not_(pbes_expression p)
  {
    return core::detail::optimized_not(p, pbes_expr::not_, true_(), is_true, false_(), is_false);
  }

  /// \brief Make a conjunction
  /// \param p A PBES expression
  /// \param q A PBES expression
  /// \return The value <tt>p && q</tt>
  inline
  pbes_expression and_(pbes_expression p, pbes_expression q)
  {
    return core::detail::optimized_and(p, q, pbes_expr::and_, true_(), is_true, false_(), is_false);
  }

  /// \brief Make a disjunction
  /// \param p A PBES expression
  /// \param q A PBES expression
  /// \return The value <tt>p || q</tt>
  inline
  pbes_expression or_(pbes_expression p, pbes_expression q)
  {
    return core::detail::optimized_or(p, q, pbes_expr::or_, true_(), is_true, false_(), is_false);
  }

  /// \brief Make an implication
  /// \param p A PBES expression
  /// \param q A PBES expression
  /// \return The value <tt>p => q</tt>
  inline
  pbes_expression imp(pbes_expression p, pbes_expression q)
  {
    return core::detail::optimized_imp(p, q, pbes_expr::imp, not_, true_(), is_true, false_(), is_false);
  }

  /// \brief Returns or applied to the sequence of pbes expressions [first, last)
  /// \param first Start of a sequence of pbes expressions
  /// \param last End of a sequence of pbes expressions
  /// \return Or applied to the sequence of pbes expressions [first, last)
  template <typename FwdIt>
  inline pbes_expression join_or(FwdIt first, FwdIt last)
  {
    return core::detail::join(first, last, or_, false_());
  }

  /// \brief Returns and applied to the sequence of pbes expressions [first, last)
  /// \param first Start of a sequence of pbes expressions
  /// \param last End of a sequence of pbes expressions
  /// \return And applied to the sequence of pbes expressions [first, last)
  template <typename FwdIt>
  inline pbes_expression join_and(FwdIt first, FwdIt last)
  {
    return core::detail::join(first, last, and_, true_());
  }

  /// \brief Make a universal quantification
  /// If l is empty, p is returned.
  /// \param l A sequence of data variables
  /// \param p A PBES expression
  /// \return The value <tt>forall l.p</tt>
  inline
  pbes_expression forall(data::variable_list l, pbes_expression p)
  {
    if (l.empty())
    {
      return p;
    }
    if (is_false(p))
    {
      // N.B. Here we use the fact that mCRL2 data types are never empty.
      return data::sort_bool::false_();
    }
    if (is_true(p))
    {
      return true_();
    }
    return pbes_expr::forall(l, p);
  }

  /// \brief Make an existential quantification
  /// If l is empty, p is returned.
  /// \param l A sequence of data variables
  /// \param p A PBES expression
  /// \return The value <tt>exists l.p</tt>
  inline
  pbes_expression exists(data::variable_list l, pbes_expression p)
  {
    if (l.empty())
    {
      return p;
    }
    if (is_false(p))
    {
      return data::sort_bool::false_();
    }
    if (is_true(p))
    {
      // N.B. Here we use the fact that mCRL2 data types are never empty.
      return data::sort_bool::true_();
    }
    return pbes_expr::exists(l, p);
  }

} // namespace pbes_expr_optimized

  /// \brief Returns true if the pbes expression t is a boolean expression
  /// \param t A term
  /// \return True if the pbes expression t is a boolean expression
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
    else if(is_pbes_imp(t)) {
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
    typedef data::variable variable_type;

    /// \brief The variable sequence type
    typedef data::variable_list variable_sequence_type;

    /// \brief The propositional variable declaration type
    typedef pbes_system::propositional_variable propositional_variable_decl_type;

    /// \brief The propositional variable instantiation type
    typedef pbes_system::propositional_variable_instantiation propositional_variable_type;

    /// \brief The string type
    typedef core::identifier_string string_type;

    /// \brief Make the value true
    /// \return The value \p true
    static inline
    term_type true_()
    {
      return atermpp::aterm_appl(core::detail::gsMakePBESTrue());
    }

    /// \brief Make the value false
    /// \return The value \p false
    static inline
    term_type false_()
    {
      return atermpp::aterm_appl(core::detail::gsMakePBESFalse());
    }

    /// \brief Make a negation
    /// \param p A term
    /// \return The value <tt>!p</tt>
    static inline
    term_type not_(term_type p)
    {
      return atermpp::aterm_appl(core::detail::gsMakePBESNot(p));
    }

    static inline
    bool is_sorted(term_type p, term_type q)
    {
      pbes_system::detail::compare_pbes_expression_visitor<term_type> pvisitor;
      pvisitor.visit(p);
      pbes_system::detail::compare_pbes_expression_visitor<term_type> qvisitor;
      qvisitor.visit(q);
      if (pvisitor.has_predicate_variables != qvisitor.has_predicate_variables)
      {
        return qvisitor.has_predicate_variables;
      }
      if (pvisitor.has_quantifiers != qvisitor.has_quantifiers)
      {
        return qvisitor.has_quantifiers;
      }
      if (pvisitor.result.size() != qvisitor.result.size())
      {
        return pvisitor.result.size() < qvisitor.result.size();
      }
      return p < q;
    }

    /// \brief Make a conjunction
    /// \param p A term
    /// \param q A term
    /// \return The value <tt>p && q</tt>
    static inline
    term_type and_(term_type p, term_type q)
    {
      return atermpp::aterm_appl(core::detail::gsMakePBESAnd(p,q));
    }

    /// \brief Make a disjunction
    /// \param p A term
    /// \param q A term
    /// \return The value <tt>p || q</tt>
    static inline
    term_type or_(term_type p, term_type q)
    {
      return atermpp::aterm_appl(core::detail::gsMakePBESOr(p,q));
    }

    /// \brief Make a sorted conjunction
    /// \param p A term
    /// \param q A term
    /// \return The value <tt>p && q</tt>, or <tt>q && p</tt>
    static inline
    term_type sorted_and(term_type p, term_type q)
    {
#ifdef MCRL2_SMART_ARGUMENT_SORTING
      bool sorted = is_sorted(p, q);
#else
      bool sorted = p < q;
#endif
      return sorted ? atermpp::aterm_appl(core::detail::gsMakePBESAnd(p,q)) : atermpp::aterm_appl(core::detail::gsMakePBESAnd(q,p));
    }

    /// \brief Make a sorted disjunction
    /// \param p A term
    /// \param q A term
    /// \return The value <tt>p || q</tt>
    static inline
    term_type sorted_or(term_type p, term_type q)
    {
#ifdef MCRL2_SMART_ARGUMENT_SORTING
      bool sorted = is_sorted(p, q);
#else
      bool sorted = p < q;
#endif
      return sorted ? atermpp::aterm_appl(core::detail::gsMakePBESOr(p,q)) : atermpp::aterm_appl(core::detail::gsMakePBESOr(q,p));
    }

    /// \brief Make an implication
    /// \param p A term
    /// \param q A term
    /// \return The value <tt>p => q</tt>
    static inline
    term_type imp(term_type p, term_type q)
    {
      return atermpp::aterm_appl(core::detail::gsMakePBESImp(p,q));
    }

    /// \brief Make a universal quantification
    /// \param l A sequence of variables
    /// \param p A term
    /// \return The value <tt>forall l.p</tt>
    static inline
    term_type forall(variable_sequence_type l, term_type p)
    {
      if (l.empty())
      {
        return p;
      }
      return atermpp::aterm_appl(core::detail::gsMakePBESForall(l, p));
    }

    /// \brief Make an existential quantification
    /// \param l A sequence of variables
    /// \param p A term
    /// \return The value <tt>exists l.p</tt>
    static inline
    term_type exists(variable_sequence_type l, term_type p)
    {
      if (l.empty())
      {
        return p;
      }
      return atermpp::aterm_appl(core::detail::gsMakePBESExists(l, p));
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

    /// \brief Test for the value true
    /// \param t A term
    /// \return True if it is the value \p true
    static inline
    bool is_true(const term_type& t)
    {
      return core::detail::gsIsPBESTrue(t) || data::sort_bool::is_true_function_symbol(t);
    }

    /// \brief Test for the value false
    /// \param t A term
    /// \return True if it is the value \p false
    static inline
    bool is_false(const term_type& t)
    {
      return core::detail::gsIsPBESFalse(t) || data::sort_bool::is_false_function_symbol(t);
    }

    /// \brief Test for a negation
    /// \param t A term
    /// \return True if it is a negation
    static inline
    bool is_not(const term_type& t)
    {
      return core::detail::gsIsPBESNot(t);
    }

    /// \brief Test for a conjunction
    /// \param t A term
    /// \return True if it is a conjunction
    static inline
    bool is_and(const term_type& t)
    {
      return pbes_system::is_and(t);
    }

    /// \brief Test for a disjunction
    /// \param t A term
    /// \return True if it is a disjunction
    static inline
    bool is_or(const term_type& t)
    {
      return pbes_system::is_or(t);
    }

    /// \brief Test for an implication
    /// \param t A term
    /// \return True if it is an implication
    static inline
    bool is_imp(const term_type& t)
    {
      return pbes_system::is_imp(t);
    }

    /// \brief Test for an universal quantification
    /// \param t A term
    /// \return True if it is a universal quantification
    static inline
    bool is_forall(const term_type& t)
    {
      return pbes_system::is_forall(t);
    }

    /// \brief Test for an existential quantification
    /// \param t A term
    /// \return True if it is an existential quantification
    static inline
    bool is_exists(const term_type& t)
    {
      return pbes_system::is_exists(t);
    }

    /// \brief Test for data term
    /// \param t A term
    /// \return True if the term is a data term
    static inline
    bool is_data(const term_type& t)
    {
      return data::is_data_expression(t);
    }

    /// \brief Test for propositional variable instantiation
    /// \param t A term
    /// \return True if the term is a propositional variable instantiation
    static inline
    bool is_prop_var(const term_type& t)
    {
      return core::detail::gsIsPropVarInst(t);
    }

    /// \brief Returns the argument of a term of type not, exists or forall
    /// \param t A term
    /// \return The requested argument. Partially works for data terms
    static inline
    term_type arg(const term_type& t)
    {
      return pbes_system::accessors::arg(t);
    }

    /// \brief Returns the left argument of a term of type and, or or imp
    /// \param t A term
    /// \return The left argument of the term. Also works for data terms
    static inline
    term_type left(const term_type& t)
    {
      return pbes_system::accessors::left(t);
    }

    /// \brief Returns the right argument of a term of type and, or or imp
    /// \param t A term
    /// \return The right argument of the term. Also works for data terms
    static inline
    term_type right(const term_type& t)
    {
      return pbes_system::accessors::right(t);
    }

    /// \brief Returns the quantifier variables of a quantifier expression
    /// \param t A term
    /// \return The requested argument. Doesn't work for data terms
    static inline
    variable_sequence_type var(const term_type& t)
    {
      // Forall and exists are not fully supported by the data library
      assert(!data::is_data_expression(t) || (!data::is_abstraction(t)
		 || (!data::is_forall(data::abstraction(t)) && !data::is_exists(data::abstraction(t)))));
      assert(is_exists(t) || is_forall(t));

      return variable_sequence_type(atermpp::list_arg1(t));
    }

    /// \brief Returns the name of a propositional variable instantiation
    /// \param t A term
    /// \return The name of the propositional variable instantiation
    static inline
    string_type name(const term_type& t)
    {
      assert(is_prop_var(t));
      return atermpp::arg1(t);
    }

    /// \brief Returns the parameter list of a propositional variable instantiation
    /// \param t A term
    /// \return The parameter list of the propositional variable instantiation
    static inline
    data_term_sequence_type param(const term_type& t)
    {
      assert(is_prop_var(t));
      return variable_sequence_type(atermpp::list_arg2(t));
    }

    /// \brief Conversion from variable to term
    /// \param v A variable
    /// \return The converted variable
    static inline
    term_type variable2term(const variable_type& v)
    {
      return v;
    }

    /// \brief Test if a term is a variable
    /// \param t A term
    /// \return True if the term is a variable
    static inline
    bool is_variable(const term_type& t)
    {
      return data::is_variable(t);
    }

    /// \brief Returns the free variables of a term
    /// \param t A term
    /// \return The free variables of a term
    static inline
    variable_sequence_type free_variables(const term_type& t)
    {
      pbes_system::detail::free_variable_visitor<term_type> visitor;
      visitor.visit(t);
      return variable_sequence_type(visitor.result.begin(), visitor.result.end());
    }

    /// \brief Conversion from data term to term
    /// \param t A data term
    /// \return The converted term
    static inline
    term_type dataterm2term(data_term_type t)
    {
      return t;
    }

    /// \brief Conversion from term to data term
    /// \param t A term
    /// \return The converted term
    static inline
    data_term_type term2dataterm(const term_type& t)
    {
      return t;
    }

    /// \brief Test if a term is constant
    /// \param t A term
    /// \return True if the term is constant
    static inline
    bool is_constant(term_type /* t */) { return false; }

    /// \brief Pretty print function
    /// \param t A term
    /// \return Returns a pretty print representation of the term
    static inline
    std::string pp(const term_type& t)
    {
      return core::pp(t);
    }
  };

} // namespace core

} // namespace mcrl2

#endif // MCRL2_PBES_PBES_EXPRESSION_H
