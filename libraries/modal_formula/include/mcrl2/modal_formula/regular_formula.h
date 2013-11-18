// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/regular_formula.h
/// \brief Add your file description here.

#ifndef MCRL2_MODAL_REGULAR_FORMULA_H
#define MCRL2_MODAL_REGULAR_FORMULA_H

#include <iostream> // for debugging

#include <string>
#include <cassert>
#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/core/detail/constructors.h"
#include "mcrl2/core/detail/precedence.h"
#include "mcrl2/core/detail/soundness_checks.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/modal_formula/action_formula.h"

namespace mcrl2
{

namespace regular_formulas
{

//--- start generated classes ---//
/// \brief A regular formula
class regular_formula: public atermpp::aterm_appl
{
  public:
    /// \brief Default constructor.
    regular_formula()
      : atermpp::aterm_appl(core::detail::constructRegFrm())
    {}

    /// \brief Constructor.
    /// \param term A term
    explicit regular_formula(const atermpp::aterm& term)
      : atermpp::aterm_appl(term)
    {
      assert(core::detail::check_rule_RegFrm(*this));
    }

    /// \brief Constructor.
    regular_formula(const action_formulas::action_formula& x)
      : atermpp::aterm_appl(x)
    {}
};

/// \brief list of regular_formulas
typedef atermpp::term_list<regular_formula> regular_formula_list;

/// \brief vector of regular_formulas
typedef std::vector<regular_formula>    regular_formula_vector;

// prototypes
inline bool is_nil(const atermpp::aterm_appl& x);
inline bool is_seq(const atermpp::aterm_appl& x);
inline bool is_alt(const atermpp::aterm_appl& x);
inline bool is_trans(const atermpp::aterm_appl& x);
inline bool is_trans_or_nil(const atermpp::aterm_appl& x);

/// \brief Test for a regular_formula expression
/// \param x A term
/// \return True if \a x is a regular_formula expression
inline
bool is_regular_formula(const atermpp::aterm_appl& x)
{
  return action_formulas::is_action_formula(x) ||
         regular_formulas::is_nil(x) ||
         regular_formulas::is_seq(x) ||
         regular_formulas::is_alt(x) ||
         regular_formulas::is_trans(x) ||
         regular_formulas::is_trans_or_nil(x);
}

// prototype declaration
std::string pp(const regular_formula& x);

/// \brief Outputs the object to a stream
/// \param out An output stream
/// \return The output stream
inline
std::ostream& operator<<(std::ostream& out, const regular_formula& x)
{
  return out << regular_formulas::pp(x);
}

/// \brief swap overload
inline void swap(regular_formula& t1, regular_formula& t2)
{
  t1.swap(t2);
}


/// \brief The value nil for regular formulas
class nil: public regular_formula
{
  public:
    /// \brief Default constructor.
    nil()
      : regular_formula(core::detail::constructRegNil())
    {}

    /// \brief Constructor.
    /// \param term A term
    explicit nil(const atermpp::aterm& term)
      : regular_formula(term)
    {
      assert(core::detail::check_term_RegNil(*this));
    }
};

/// \brief Test for a nil expression
/// \param x A term
/// \return True if \a x is a nil expression
inline
bool is_nil(const atermpp::aterm_appl& x)
{
  return core::detail::gsIsRegNil(x);
}

// prototype declaration
std::string pp(const nil& x);

/// \brief Outputs the object to a stream
/// \param out An output stream
/// \return The output stream
inline
std::ostream& operator<<(std::ostream& out, const nil& x)
{
  return out << regular_formulas::pp(x);
}

/// \brief swap overload
inline void swap(nil& t1, nil& t2)
{
  t1.swap(t2);
}


/// \brief The seq operator for regular formulas
class seq: public regular_formula
{
  public:
    /// \brief Default constructor.
    seq()
      : regular_formula(core::detail::constructRegSeq())
    {}

    /// \brief Constructor.
    /// \param term A term
    explicit seq(const atermpp::aterm& term)
      : regular_formula(term)
    {
      assert(core::detail::check_term_RegSeq(*this));
    }

    /// \brief Constructor.
    seq(const regular_formula& left, const regular_formula& right)
      : regular_formula(core::detail::gsMakeRegSeq(left, right))
    {}

    const regular_formula& left() const
    {
      return atermpp::aterm_cast<const regular_formula>((*this)[0]);
    }

    const regular_formula& right() const
    {
      return atermpp::aterm_cast<const regular_formula>((*this)[1]);
    }
};

/// \brief Test for a seq expression
/// \param x A term
/// \return True if \a x is a seq expression
inline
bool is_seq(const atermpp::aterm_appl& x)
{
  return core::detail::gsIsRegSeq(x);
}

// prototype declaration
std::string pp(const seq& x);

/// \brief Outputs the object to a stream
/// \param out An output stream
/// \return The output stream
inline
std::ostream& operator<<(std::ostream& out, const seq& x)
{
  return out << regular_formulas::pp(x);
}

/// \brief swap overload
inline void swap(seq& t1, seq& t2)
{
  t1.swap(t2);
}


/// \brief The alt operator for regular formulas
class alt: public regular_formula
{
  public:
    /// \brief Default constructor.
    alt()
      : regular_formula(core::detail::constructRegAlt())
    {}

    /// \brief Constructor.
    /// \param term A term
    explicit alt(const atermpp::aterm& term)
      : regular_formula(term)
    {
      assert(core::detail::check_term_RegAlt(*this));
    }

    /// \brief Constructor.
    alt(const regular_formula& left, const regular_formula& right)
      : regular_formula(core::detail::gsMakeRegAlt(left, right))
    {}

    const regular_formula& left() const
    {
      return atermpp::aterm_cast<const regular_formula>((*this)[0]);
    }

    const regular_formula& right() const
    {
      return atermpp::aterm_cast<const regular_formula>((*this)[1]);
    }
};

/// \brief Test for a alt expression
/// \param x A term
/// \return True if \a x is a alt expression
inline
bool is_alt(const atermpp::aterm_appl& x)
{
  return core::detail::gsIsRegAlt(x);
}

// prototype declaration
std::string pp(const alt& x);

/// \brief Outputs the object to a stream
/// \param out An output stream
/// \return The output stream
inline
std::ostream& operator<<(std::ostream& out, const alt& x)
{
  return out << regular_formulas::pp(x);
}

/// \brief swap overload
inline void swap(alt& t1, alt& t2)
{
  t1.swap(t2);
}


/// \brief The trans operator for regular formulas
class trans: public regular_formula
{
  public:
    /// \brief Default constructor.
    trans()
      : regular_formula(core::detail::constructRegTrans())
    {}

    /// \brief Constructor.
    /// \param term A term
    explicit trans(const atermpp::aterm& term)
      : regular_formula(term)
    {
      assert(core::detail::check_term_RegTrans(*this));
    }

    /// \brief Constructor.
    trans(const regular_formula& operand)
      : regular_formula(core::detail::gsMakeRegTrans(operand))
    {}

    const regular_formula& operand() const
    {
      return atermpp::aterm_cast<const regular_formula>((*this)[0]);
    }
};

/// \brief Test for a trans expression
/// \param x A term
/// \return True if \a x is a trans expression
inline
bool is_trans(const atermpp::aterm_appl& x)
{
  return core::detail::gsIsRegTrans(x);
}

// prototype declaration
std::string pp(const trans& x);

/// \brief Outputs the object to a stream
/// \param out An output stream
/// \return The output stream
inline
std::ostream& operator<<(std::ostream& out, const trans& x)
{
  return out << regular_formulas::pp(x);
}

/// \brief swap overload
inline void swap(trans& t1, trans& t2)
{
  t1.swap(t2);
}


/// \brief The 'trans or nil' operator for regular formulas
class trans_or_nil: public regular_formula
{
  public:
    /// \brief Default constructor.
    trans_or_nil()
      : regular_formula(core::detail::constructRegTransOrNil())
    {}

    /// \brief Constructor.
    /// \param term A term
    explicit trans_or_nil(const atermpp::aterm& term)
      : regular_formula(term)
    {
      assert(core::detail::check_term_RegTransOrNil(*this));
    }

    /// \brief Constructor.
    trans_or_nil(const regular_formula& operand)
      : regular_formula(core::detail::gsMakeRegTransOrNil(operand))
    {}

    const regular_formula& operand() const
    {
      return atermpp::aterm_cast<const regular_formula>((*this)[0]);
    }
};

/// \brief Test for a trans_or_nil expression
/// \param x A term
/// \return True if \a x is a trans_or_nil expression
inline
bool is_trans_or_nil(const atermpp::aterm_appl& x)
{
  return core::detail::gsIsRegTransOrNil(x);
}

// prototype declaration
std::string pp(const trans_or_nil& x);

/// \brief Outputs the object to a stream
/// \param out An output stream
/// \return The output stream
inline
std::ostream& operator<<(std::ostream& out, const trans_or_nil& x)
{
  return out << regular_formulas::pp(x);
}

/// \brief swap overload
inline void swap(trans_or_nil& t1, trans_or_nil& t2)
{
  t1.swap(t2);
}
//--- end generated classes ---//

inline
int precedence(const regular_formula& x)
{
  if (is_seq(x))
  {
    return 1;
  }
  else if (is_alt(x))
  {
    return 2;
  }
  else if (is_trans(x) || is_trans_or_nil(x))
  {
    return 3;
  }
  return core::detail::precedences::max_precedence;
}

// TODO: is there a cleaner way to make the precedence function work for derived classes?
inline int precedence(const seq& x) { return precedence(static_cast<const regular_formula&>(x)); }
inline int precedence(const alt& x) { return precedence(static_cast<const regular_formula&>(x)); }
inline int precedence(const trans& x) { return precedence(static_cast<const regular_formula&>(x)); }
inline int precedence(const trans_or_nil& x) { return precedence(static_cast<const regular_formula&>(x)); }

inline const regular_formula& unary_operand(const trans& x)        { return x.operand(); }
inline const regular_formula& unary_operand(const trans_or_nil& x) { return x.operand(); }
inline const regular_formula& binary_left(const seq& x)            { return x.left(); }
inline const regular_formula& binary_right(const seq& x)           { return x.right(); }
inline const regular_formula& binary_left(const alt& x)            { return x.left(); }
inline const regular_formula& binary_right(const alt& x)           { return x.right(); }

} // namespace regular_formulas

} // namespace mcrl2

#endif // MCRL2_MODAL_REGULAR_FORMULA_H
