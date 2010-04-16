// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/fdr/expression_decl.h
/// \brief add your file description here.

#ifndef MCRL2_FDR_EXPRESSION_DECL_H
#define MCRL2_FDR_EXPRESSION_DECL_H

#include "mcrl2/atermpp/aterm_access.h"
#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/fdr/detail/term_functions.h"
#include "mcrl2/fdr/detail/constructors.h"
#include "mcrl2/fdr/detail/syntax_checks.h"

#include "mcrl2/fdr/numeric_expression_decl.h"

namespace mcrl2 {

namespace fdr {

//--- start generated class declarations ---//
/// \brief class expression
class expression: public atermpp::aterm_appl
{
  public:
    /// \brief Default constructor.
    expression()
      : atermpp::aterm_appl(fdr::detail::constructExpression())
    {}

    /// \brief Constructor.
    /// \param term A term
    expression(atermpp::aterm_appl term)
      : atermpp::aterm_appl(term)
    {
      assert(fdr::detail::check_term_Expression(m_term));
    }
};

/// \brief list of expressions
typedef atermpp::term_list<expression> expression_list;

/// \brief vector of expressions
typedef atermpp::vector<expression>    expression_vector;

/// \brief A numeric expression
class numb: public expression
{
  public:
    /// \brief Default constructor.
    numb();

    /// \brief Constructor.
    /// \param term A term
    numb(atermpp::aterm_appl term);

    /// \brief Constructor.
    numb(const numeric_expression& operand);

    numeric_expression operand() const;
};

/// \brief A boolean expression
class bool_: public expression
{
  public:
    /// \brief Default constructor.
    bool_();

    /// \brief Constructor.
    /// \param term A term
    bool_(atermpp::aterm_appl term);

    /// \brief Constructor.
    bool_(const boolean_expression& operand);

    boolean_expression operand() const;
};

/// \brief A set expression
class set: public expression
{
  public:
    /// \brief Default constructor.
    set();

    /// \brief Constructor.
    /// \param term A term
    set(atermpp::aterm_appl term);

    /// \brief Constructor.
    set(const set_expression& operand);

    set_expression operand() const;
};

/// \brief A seq expression
class seq: public expression
{
  public:
    /// \brief Default constructor.
    seq();

    /// \brief Constructor.
    /// \param term A term
    seq(atermpp::aterm_appl term);

    /// \brief Constructor.
    seq(const seq_expression& operand);

    seq_expression operand() const;
};

/// \brief A tuple expression
class tuple: public expression
{
  public:
    /// \brief Default constructor.
    tuple();

    /// \brief Constructor.
    /// \param term A term
    tuple(atermpp::aterm_appl term);

    /// \brief Constructor.
    tuple(const tuple_expression& operand);

    tuple_expression operand() const;
};

/// \brief A dotted expression
class dotted: public expression
{
  public:
    /// \brief Default constructor.
    dotted();

    /// \brief Constructor.
    /// \param term A term
    dotted(atermpp::aterm_appl term);

    /// \brief Constructor.
    dotted(const dotted_expression& operand);

    dotted_expression operand() const;
};

/// \brief A lambda expression
class lambda: public expression
{
  public:
    /// \brief Default constructor.
    lambda();

    /// \brief Constructor.
    /// \param term A term
    lambda(atermpp::aterm_appl term);

    /// \brief Constructor.
    lambda(const lambda_expression& operand);

    lambda_expression operand() const;
};
//--- end generated class declarations ---//

} // namespace fdr

} // namespace mcrl2

#endif // MCRL2_FDR_EXPRESSION_DECL_H
