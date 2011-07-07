// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/bes/boolean_expression.h
/// \brief add your file description here.

#ifndef MCRL2_BES_BOOLEAN_EXPRESSION_H
#define MCRL2_BES_BOOLEAN_EXPRESSION_H

#include <cassert>
#include <string>
#include "mcrl2/atermpp/aterm_access.h"
#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/set.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/utilities/detail/join.h"
#include "mcrl2/core/detail/constructors.h"
#include "mcrl2/core/detail/struct_core.h"
#include "mcrl2/core/detail/soundness_checks.h"
#include "mcrl2/core/detail/precedence.h"
#include "mcrl2/core/identifier_string.h"
#include "mcrl2/core/term_traits.h"
#include "mcrl2/core/print.h"
#include "mcrl2/utilities/text_utility.h"

namespace mcrl2
{

namespace bes
{

using namespace core::detail::precedences;

//--- start generated classes ---//
/// \brief A boolean expression
class boolean_expression: public atermpp::aterm_appl
{
  public:
    /// \brief Default constructor.
    boolean_expression()
      : atermpp::aterm_appl(core::detail::constructBooleanExpression())
    {}

    /// \brief Constructor.
    /// \param term A term
    boolean_expression(const atermpp::aterm_appl& term)
      : atermpp::aterm_appl(term)
    {
      assert(core::detail::check_rule_BooleanExpression(m_term));
    }
};

/// \brief list of boolean_expressions
typedef atermpp::term_list<boolean_expression> boolean_expression_list;

/// \brief vector of boolean_expressions
typedef atermpp::vector<boolean_expression>    boolean_expression_vector;


/// \brief The value true for boolean expressions
class true_: public boolean_expression
{
  public:
    /// \brief Default constructor.
    true_()
      : boolean_expression(core::detail::constructBooleanTrue())
    {}

    /// \brief Constructor.
    /// \param term A term
    true_(const atermpp::aterm_appl& term)
      : boolean_expression(term)
    {
      assert(core::detail::check_term_BooleanTrue(m_term));
    }
};

/// \brief Test for a true expression
/// \param t A term
/// \return True if it is a true expression
inline
bool is_true(const boolean_expression& t)
{
  return core::detail::gsIsBooleanTrue(t);
}


/// \brief The value false for boolean expressions
class false_: public boolean_expression
{
  public:
    /// \brief Default constructor.
    false_()
      : boolean_expression(core::detail::constructBooleanFalse())
    {}

    /// \brief Constructor.
    /// \param term A term
    false_(const atermpp::aterm_appl& term)
      : boolean_expression(term)
    {
      assert(core::detail::check_term_BooleanFalse(m_term));
    }
};

/// \brief Test for a false expression
/// \param t A term
/// \return True if it is a false expression
inline
bool is_false(const boolean_expression& t)
{
  return core::detail::gsIsBooleanFalse(t);
}


/// \brief The not operator for boolean expressions
class not_: public boolean_expression
{
  public:
    /// \brief Default constructor.
    not_()
      : boolean_expression(core::detail::constructBooleanNot())
    {}

    /// \brief Constructor.
    /// \param term A term
    not_(const atermpp::aterm_appl& term)
      : boolean_expression(term)
    {
      assert(core::detail::check_term_BooleanNot(m_term));
    }

    /// \brief Constructor.
    not_(const boolean_expression& operand)
      : boolean_expression(core::detail::gsMakeBooleanNot(operand))
    {}

    boolean_expression operand() const
    {
      return atermpp::arg1(*this);
    }
};

/// \brief Test for a not expression
/// \param t A term
/// \return True if it is a not expression
inline
bool is_not(const boolean_expression& t)
{
  return core::detail::gsIsBooleanNot(t);
}


/// \brief The and operator for boolean expressions
class and_: public boolean_expression
{
  public:
    /// \brief Default constructor.
    and_()
      : boolean_expression(core::detail::constructBooleanAnd())
    {}

    /// \brief Constructor.
    /// \param term A term
    and_(const atermpp::aterm_appl& term)
      : boolean_expression(term)
    {
      assert(core::detail::check_term_BooleanAnd(m_term));
    }

    /// \brief Constructor.
    and_(const boolean_expression& left, const boolean_expression& right)
      : boolean_expression(core::detail::gsMakeBooleanAnd(left, right))
    {}

    boolean_expression left() const
    {
      return atermpp::arg1(*this);
    }

    boolean_expression right() const
    {
      return atermpp::arg2(*this);
    }
};

/// \brief Test for a and expression
/// \param t A term
/// \return True if it is a and expression
inline
bool is_and(const boolean_expression& t)
{
  return core::detail::gsIsBooleanAnd(t);
}


/// \brief The or operator for boolean expressions
class or_: public boolean_expression
{
  public:
    /// \brief Default constructor.
    or_()
      : boolean_expression(core::detail::constructBooleanOr())
    {}

    /// \brief Constructor.
    /// \param term A term
    or_(const atermpp::aterm_appl& term)
      : boolean_expression(term)
    {
      assert(core::detail::check_term_BooleanOr(m_term));
    }

    /// \brief Constructor.
    or_(const boolean_expression& left, const boolean_expression& right)
      : boolean_expression(core::detail::gsMakeBooleanOr(left, right))
    {}

    boolean_expression left() const
    {
      return atermpp::arg1(*this);
    }

    boolean_expression right() const
    {
      return atermpp::arg2(*this);
    }
};

/// \brief Test for a or expression
/// \param t A term
/// \return True if it is a or expression
inline
bool is_or(const boolean_expression& t)
{
  return core::detail::gsIsBooleanOr(t);
}


/// \brief The implication operator for boolean expressions
class imp: public boolean_expression
{
  public:
    /// \brief Default constructor.
    imp()
      : boolean_expression(core::detail::constructBooleanImp())
    {}

    /// \brief Constructor.
    /// \param term A term
    imp(const atermpp::aterm_appl& term)
      : boolean_expression(term)
    {
      assert(core::detail::check_term_BooleanImp(m_term));
    }

    /// \brief Constructor.
    imp(const boolean_expression& left, const boolean_expression& right)
      : boolean_expression(core::detail::gsMakeBooleanImp(left, right))
    {}

    boolean_expression left() const
    {
      return atermpp::arg1(*this);
    }

    boolean_expression right() const
    {
      return atermpp::arg2(*this);
    }
};

/// \brief Test for a imp expression
/// \param t A term
/// \return True if it is a imp expression
inline
bool is_imp(const boolean_expression& t)
{
  return core::detail::gsIsBooleanImp(t);
}


/// \brief A boolean variable
class boolean_variable: public boolean_expression
{
  public:
    /// \brief Default constructor.
    boolean_variable()
      : boolean_expression(core::detail::constructBooleanVariable())
    {}

    /// \brief Constructor.
    /// \param term A term
    boolean_variable(const atermpp::aterm_appl& term)
      : boolean_expression(term)
    {
      assert(core::detail::check_term_BooleanVariable(m_term));
    }

    /// \brief Constructor.
    boolean_variable(const core::identifier_string& name)
      : boolean_expression(core::detail::gsMakeBooleanVariable(name))
    {}

    /// \brief Constructor.
    boolean_variable(const std::string& name)
      : boolean_expression(core::detail::gsMakeBooleanVariable(core::identifier_string(name)))
    {}

    core::identifier_string name() const
    {
      return atermpp::arg1(*this);
    }
};

/// \brief Test for a boolean_variable expression
/// \param t A term
/// \return True if it is a boolean_variable expression
inline
bool is_boolean_variable(const boolean_expression& t)
{
  return core::detail::gsIsBooleanVariable(t);
}

//--- end generated classes ---//

// From the documentation:
// The "!" operator has the highest priority, followed by "&&" and "||", followed by "=>", followed by "forall" and "exists".
// The infix operators "&&", "||" and "=>" associate to the right.
/// \brief Returns the precedence of pbes expressions
inline
int precedence(const boolean_expression& x)
{
  if (is_imp(x)) 
  {
    return 1;
  }
  else if (is_and(x) || is_or(x)) 
  {
    return 2;
  }
  else if (is_not(x)) 
  {
    return 3;
  }
  return core::detail::precedences::max_precedence;
}

/// \brief Returns true if the operations have the same precedence, but are different
inline
bool is_same_different_precedence(const and_&, const boolean_expression& x)
{
  return is_or(x);
}

/// \brief Returns true if the operations have the same precedence, but are different
inline
bool is_same_different_precedence(const or_&, const boolean_expression& x)
{
  return is_and(x);
}

namespace accessors
{
inline
boolean_expression left(boolean_expression const& e)
{
  assert(is_and(e) || is_or(e) || is_imp(e));
  return atermpp::arg1(e);
}

inline
boolean_expression right(boolean_expression const& e)
{
  assert(is_and(e) || is_or(e) || is_imp(e));
  return atermpp::arg2(e);
}

} // namespace accessors

/// \brief Returns true if the term t is a boolean expression
/// \param t A boolean expression
/// \return True if the term t is a boolean expression
inline
bool is_boolean_expression(atermpp::aterm_appl t)
{
  // TODO: this code should be generated
  return
    core::detail::gsIsBooleanTrue(t) ||
    core::detail::gsIsBooleanFalse(t) ||
    core::detail::gsIsBooleanVariable(t) ||
    core::detail::gsIsBooleanNot(t) ||
    core::detail::gsIsBooleanAnd(t) ||
    core::detail::gsIsBooleanOr(t) ||
    core::detail::gsIsBooleanImp(t)
    ;
}

} // namespace bes

} // namespace mcrl2

namespace mcrl2
{

namespace core
{

/// \brief Contains type information for boolean expressions
template <>
struct term_traits<bes::boolean_expression>
{
  /// The term type
  typedef bes::boolean_expression term_type;

  /// \brief The variable type
  typedef bes::boolean_variable variable_type;

  /// \brief The string type
  typedef core::identifier_string string_type;

  /// \brief The value true
  /// \return The value true
  static inline
  term_type true_()
  {
    return atermpp::aterm_appl(core::detail::gsMakeBooleanTrue());
  }

  /// \brief The value false
  /// \return The value false
  static inline
  term_type false_()
  {
    return atermpp::aterm_appl(core::detail::gsMakeBooleanFalse());
  }

  /// \brief Operator not
  /// \param p A term
  /// \return Operator not applied to p
  static inline
  term_type not_(term_type p)
  {
    return atermpp::aterm_appl(core::detail::gsMakeBooleanNot(p));
  }

  /// \brief Operator and
  /// \param p A term
  /// \param q A term
  /// \return Operator and applied to p and q
  static inline
  term_type and_(term_type p, term_type q)
  {
    return atermpp::aterm_appl(core::detail::gsMakeBooleanAnd(p, q));
  }

  /// \brief Operator or
  /// \param p A term
  /// \param q A term
  /// \return Operator or applied to p and q
  static inline
  term_type or_(term_type p, term_type q)
  {
    return atermpp::aterm_appl(core::detail::gsMakeBooleanOr(p, q));
  }

  /// \brief Implication
  /// \param p A term
  /// \param q A term
  /// \return Implication applied to p and q
  static inline
  term_type imp(term_type p, term_type q)
  {
    return atermpp::aterm_appl(core::detail::gsMakeBooleanImp(p, q));
  }

  /// \brief Test for value true
  /// \param t A term
  /// \return True if the term has the value true
  static inline
  bool is_true(term_type t)
  {
    return core::detail::gsIsBooleanTrue(t);
  }

  /// \brief Test for value false
  /// \param t A term
  /// \return True if the term has the value false
  static inline
  bool is_false(term_type t)
  {
    return core::detail::gsIsBooleanFalse(t);
  }

  /// \brief Test for operator not
  /// \param t A term
  /// \return True if the term is of type and
  static inline
  bool is_not(term_type t)
  {
    return core::detail::gsIsBooleanNot(t);
  }

  /// \brief Test for operator and
  /// \param t A term
  /// \return True if the term is of type and
  static inline
  bool is_and(term_type t)
  {
    return core::detail::gsIsBooleanAnd(t);
  }

  /// \brief Test for operator or
  /// \param t A term
  /// \return True if the term is of type or
  static inline
  bool is_or(term_type t)
  {
    return core::detail::gsIsBooleanOr(t);
  }

  /// \brief Test for implication
  /// \param t A term
  /// \return True if the term is an implication
  static inline
  bool is_imp(term_type t)
  {
    return core::detail::gsIsBooleanImp(t);
  }

  /// \brief Test for boolean variable
  /// \param t A term
  /// \return True if the term is a boolean variable
  static inline
  bool is_variable(term_type t)
  {
    return core::detail::gsIsBooleanVariable(t);
  }

  /// \brief Test for propositional variable
  /// \param t A term
  /// \return True if the term is a propositional variable
  static inline
  bool is_prop_var(term_type t)
  {
    return is_variable(t);
  }

  /// \brief Returns the argument of a term of type not
  /// \param t A term
  /// \return The requested argument
  static inline
  term_type arg(term_type t)
  {
    assert(is_not(t));
    return atermpp::arg1(t);
  }

  /// \brief Returns the left argument of a term of type and, or or imp
  /// \param t A term
  /// \return The left argument of the term
  static inline
  term_type left(term_type t)
  {
    assert(is_and(t) || is_or(t) || is_imp(t));
    return atermpp::arg1(t);
  }

  /// \brief Returns the right argument of a term of type and, or or imp
  /// \param t A term
  /// \return The right argument of the term
  static inline
  term_type right(term_type t)
  {
    assert(is_and(t) || is_or(t) || is_imp(t));
    return atermpp::arg2(t);
  }

  /// \brief Returns the name of a boolean variable
  /// \param t A term
  /// \return The name of the boolean variable
  static inline
  string_type name(term_type t)
  {
    assert(is_variable(t));
    return atermpp::arg1(t);
  }

  /// \brief Conversion from variable to term
  /// \param v A variable
  /// \returns The converted variable
  static inline
  term_type variable2term(variable_type v)
  {
    return v;
  }

  /// \brief Conversion from term to variable
  /// \param t a term
  /// \returns The converted term
  static inline
  variable_type term2variable(term_type t)
  {
    return t;
  }

  /// \brief Pretty print function
  /// \param t A term
  /// \return Returns a pretty print representation of the term
  static inline
  std::string pp(term_type t)
  {
    return mcrl2::core::pp(t);
  }
};

} // namespace core

} // namespace mcrl2

namespace mcrl2
{

namespace bes
{

/// \brief Returns or applied to the sequence of boolean expressions [first, last)
/// \param first Start of a sequence of boolean expressions
/// \param last End of a sequence of of boolean expressions
/// \return Or applied to the sequence of boolean expressions [first, last)
template <typename FwdIt>
inline
boolean_expression join_or(FwdIt first, FwdIt last)
{
  typedef core::term_traits<boolean_expression> tr;
  return utilities::detail::join(first, last, tr::or_, tr::false_());
}

/// \brief Returns and applied to the sequence of boolean expressions [first, last)
/// \param first Start of a sequence of boolean expressions
/// \param last End of a sequence of of boolean expressions
/// \return And applied to the sequence of boolean expressions [first, last)
template <typename FwdIt>
inline
boolean_expression join_and(FwdIt first, FwdIt last)
{
  typedef core::term_traits<boolean_expression> tr;
  return utilities::detail::join(first, last, tr::and_, tr::true_());
}

inline bool
operator<(const boolean_expression& x, const boolean_expression& y)
{
  return ATermAppl(x) < ATermAppl(y);
}

/// \brief Splits a disjunction into a sequence of operands
/// Given a boolean expression of the form p1 || p2 || .... || pn, this will yield a
/// set of the form { p1, p2, ..., pn }, assuming that pi does not have a || as main
/// function symbol.
/// \param expr A boolean expression
/// \return A sequence of operands
inline
atermpp::set<boolean_expression> split_or(const boolean_expression& expr)
{
  using namespace accessors;
  atermpp::set<boolean_expression> result;
  utilities::detail::split(expr, std::insert_iterator<atermpp::set<boolean_expression> >(result, result.begin()), is_or, left, right);
  return result;
}

/// \brief Splits a conjunction into a sequence of operands
/// Given a boolean expression of the form p1 && p2 && .... && pn, this will yield a
/// set of the form { p1, p2, ..., pn }, assuming that pi does not have a && as main
/// function symbol.
/// \param expr A boolean expression
/// \return A sequence of operands
inline
atermpp::set<boolean_expression> split_and(const boolean_expression& expr)
{
  using namespace accessors;
  atermpp::set<boolean_expression> result;
  utilities::detail::split(expr, std::insert_iterator<atermpp::set<boolean_expression> >(result, result.begin()), is_and, left, right);
  return result;
}

} // namespace bes

} // namespace mcrl2

#endif // MCRL2_BES_BOOLEAN_EXPRESSION_H
