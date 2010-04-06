// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/fdr/test.h
/// \brief add your file description here.

#ifndef MCRL2_FDR_TEST_H
#define MCRL2_FDR_TEST_H

#include "mcrl2/atermpp/aterm_access.h"
#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/fdr/detail/term_functions.h"
#include "mcrl2/fdr/detail/constructors.h"
#include "mcrl2/fdr/detail/syntax_checks.h"

namespace mcrl2 {

namespace fdr {

//--- start generated classes ---//
/// \brief A divergence free
class divergence_free: atermpp::aterm_appl
{
  public:
    /// \brief Default constructor.
    divergence_free()
      : atermpp::aterm_appl(fdr::detail::constructdivergence_free())
    {}

    /// \brief Constructor.
    /// \param term A term
    divergence_free(atermpp::aterm_appl term)
      : atermpp::aterm_appl(term)
    {
      assert(fdr::detail::check_term_divergence_free(m_term));
    }
};

/// \brief list of divergence_frees
typedef atermpp::term_list<divergence_free> divergence_free_list;

/// \brief vector of divergence_frees
typedef atermpp::vector<divergence_free>    divergence_free_vector;


/// \brief A complex test
class test: atermpp::aterm_appl
{
  public:
    /// \brief Default constructor.
    test()
      : atermpp::aterm_appl(fdr::detail::constructTest())
    {}

    /// \brief Constructor.
    /// \param term A term
    test(atermpp::aterm_appl term)
      : atermpp::aterm_appl(term)
    {
      assert(fdr::detail::check_term_Test(m_term));
    }

    /// \brief Constructor.
    test(const testtype& tt, const failuremodel& fm)
      : atermpp::aterm_appl(fdr::detail::gsMakeTest(tt, fm))
    {}

    testtype tt() const
    {
      return atermpp::arg1(*this);
    }

    failuremodel fm() const
    {
      return atermpp::arg2(*this);
    }
};

/// \brief list of tests
typedef atermpp::term_list<test> test_list;

/// \brief vector of tests
typedef atermpp::vector<test>    test_vector;

//--- end generated classes ---//

//--- start generated is-functions ---//
//--- end generated is-functions ---//

} // namespace fdr

} // namespace mcrl2

#endif // MCRL2_FDR_TEST_H
