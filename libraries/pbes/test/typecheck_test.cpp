// Author(s): Aad Mathijssen, Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file typecheck_test.cpp
/// \brief Add your file description here.

#include <iostream>
#include <sstream>
#include <string>
#include <boost/test/minimal.hpp>
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/core/garbage_collection.h"
#include "mcrl2/core/parse.h"
#include "mcrl2/core/detail/pp_deprecated.h"
#include "mcrl2/pbes/typecheck.h"
#include "mcrl2/pbes/parse.h"

using namespace mcrl2;

void test_pbes_specification(const std::string& pbes_in, bool test_type_checker = true)
{
  pbes_system::pbes<> p = pbes_system::parse_pbes_new(pbes_in);
  std::string pbes_out = pbes_system::pp(p);
  BOOST_CHECK(pbes_in == pbes_out);

  if (test_type_checker)
  {
    pbes_system::type_check(p);
    pbes_out = pbes_system::pp(p);
    //std::cerr << "The following PBES specifications should be the same:" << std::endl << pbes_in  << std::endl << "and" << std::endl << pbes_out << std::endl;
    BOOST_CHECK(pbes_in == pbes_out);
  }
  core::garbage_collect();
}

void test_pbes_specification1()
{
  //test PBES specification involving global variables
  test_pbes_specification(
    "glob dc: Bool;\n"
    "\n"
    "pbes nu X(b: Bool) =\n"
    "       val(b) && X(dc);\n"
    "\n"
    "init X(dc);\n"
  );
}

void test_pbes_specification2()
{
  //test PBES specification where the type of [10,m] should become List(Nat), not List(Pos).
  //This failed in revision 10180 and before.
  test_pbes_specification(
   "pbes nu X0(m: Nat) =\n"
   "       forall i: Nat. val(!(i < 2)) || X0([10, m] . i);\n"
   "\n"
   "init X0(0);\n"
  );
}

int test_main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  test_pbes_specification1();
  test_pbes_specification2();

  return 0;
}
