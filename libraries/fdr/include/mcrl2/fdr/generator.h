// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/fdr/generator.h
/// \brief add your file description here.

#ifndef MCRL2_FDR_GENERATOR_H
#define MCRL2_FDR_GENERATOR_H

#include "mcrl2/fdr/term_include_files.h"
#include "mcrl2/fdr/expression_fwd.h"
#include "mcrl2/fdr/seq_expression_fwd.h"
#include "mcrl2/fdr/set_expression_fwd.h"

namespace mcrl2
{

namespace fdr
{

//--- start generated generator class declarations ---//
/// \brief A set generator
class setgen: public atermpp::aterm_appl
{
  public:
    /// \brief Default constructor.
    setgen();

    /// \brief Constructor.
    /// \param term A term
    setgen(atermpp::aterm_appl term);

    /// \brief Constructor.
    setgen(const expression& expr, const set_expression& set);

    expression expr() const;

    set_expression set() const;
};

/// \brief list of setgens
typedef atermpp::term_list<setgen> setgen_list;

/// \brief vector of setgens
typedef atermpp::vector<setgen>    setgen_vector;


/// \brief A seq generator
class seqgen: public atermpp::aterm_appl
{
  public:
    /// \brief Default constructor.
    seqgen();

    /// \brief Constructor.
    /// \param term A term
    seqgen(atermpp::aterm_appl term);

    /// \brief Constructor.
    seqgen(const expression& expr, const seq_expression& seq);

    expression expr() const;

    seq_expression seq() const;
};

/// \brief list of seqgens
typedef atermpp::term_list<seqgen> seqgen_list;

/// \brief vector of seqgens
typedef atermpp::vector<seqgen>    seqgen_vector;

//--- end generated generator class declarations ---//

//--- start generated is-functions ---//
//--- end generated is-functions ---//

} // namespace fdr

} // namespace mcrl2

#endif // MCRL2_FDR_GENERATOR_H
