// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file data_specification_test.cpp
/// \brief Test for data specifications. This test belongs to the data
/// library, but unfortunately the data library contains no parser for
/// data specifications.

#include <boost/test/minimal.hpp>
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/mcrl22lps.h"

using namespace mcrl2::data;
using namespace mcrl2::lps;

const std::string SPECIFICATION =
"act a;                                  \n"
"                                        \n"
"proc P(b:Bool) = a. P(b);               \n"
"                                        \n"
"init P(false);                          \n"
;

int test_main(int argc, char** argv)
{
  MCRL2_ATERM_INIT(argc, argv) 

  specification spec = mcrl22lps(SPECIFICATION);
  data_specification data = spec.data();
  BOOST_CHECK(data.is_finite(sort_expr::bool_()));
  BOOST_CHECK(!data.is_finite(sort_expr::nat()));

  return 0;
}
