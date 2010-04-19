// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file bes_io_test.cpp
/// \brief Some io tests for boolean equation systems.

#include <sstream>
#include <string>
#include <boost/test/minimal.hpp>
#include "mcrl2/core/garbage_collection.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/bes.h"
#include "mcrl2/pbes/bes_parse.h"
#include "mcrl2/atermpp/aterm_init.h"

using namespace mcrl2;
using namespace mcrl2::bes;

std::string bes1 =
  "pbes              \n"
  "                  \n"
  "nu X1 = X2 && X1; \n"
  "mu X2 = X1 => X2; \n"
  "                  \n"
  "init X1;          \n"
  ;

void test_parse_bes()
{
  boolean_equation_system<> b;
  std::stringstream from(bes1);
  from >> b;
}

void test_bes()
{
  std::stringstream out;
  //bes2cwi(Iter first, Iter last, std::ostream& out)
  core::garbage_collect();
}

void test_pbes()
{
  core::garbage_collect();
}

int test_main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT_DEBUG(argc, argv)

  test_parse_bes();
  test_bes();
  test_pbes();

  return 0;
}
