// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file print_test.cpp
/// \brief Add your file description here.

#include <string>
#include <boost/test/included/unit_test_framework.hpp>
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/txt2pbes.h"
#include "mcrl2/atermpp/aterm_init.h"

#include "mcrl2/utilities/test_utilities.h"

using namespace mcrl2;
using namespace mcrl2::pbes_system;

using mcrl2::utilities::collect_after_test_case;
BOOST_GLOBAL_FIXTURE(collect_after_test_case)

BOOST_AUTO_TEST_CASE(pbes_with_reals)
{
  std::string input(
    "pbes nu X(T: Real) =\n"
    "       (((true && val(true)) && val(1 > T)) && true) && ((false || val(!true)) || val(!(1 > T))) || X(1);\n"
    "\n"
    "init X(0);\n"
  );

  pbes<> p;
  p = txt2pbes(input);

  std::string output;
  output = pbes_system::pp(p);

  BOOST_CHECK(output.find("Real;") == std::string::npos);

}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  return 0;
}
