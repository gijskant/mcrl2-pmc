// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file boolean_operator.cpp
/// \brief Test for boolean expressions.

#include <iostream>
#include <string>
#include <boost/test/minimal.hpp>
#include <boost/filesystem/operations.hpp>
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/core/garbage_collection.h"
#include "mcrl2/bes/boolean_equation_system.h"
#include "mcrl2/bes/bes2pbes.h"

using namespace mcrl2;

void test_boolean_expressions()
{
  using namespace bes;
  typedef core::term_traits<boolean_expression> tr;

  boolean_variable X1("X1");
  boolean_variable X2("X2");
  boolean_expression t1 = tr::and_(X1, X2);
  boolean_equation e1(fixpoint_symbol::mu(), X1, tr::imp(X1, X2));
  boolean_equation e2(fixpoint_symbol::nu(), X2, tr::or_(X1, X2));
  std::cout << bes::pp(e1) << std::endl;
  std::cout << bes::pp(e2) << std::endl;

  boolean_equation_system<> p;
  p.equations().push_back(e1);
  p.equations().push_back(e2);
  p.initial_state() = X1;
  std::cout << "----------------" << std::endl;
  std::cout << pp(p) << std::endl;

  std::string filename = "boolean_expression_test.out";
  p.save(filename);
  boolean_equation_system<> q;
  q.load(filename);
  BOOST_CHECK(p == q);
  boost::filesystem::remove(boost::filesystem::path(filename));
  core::garbage_collect();
}

void test_bes2pbes()
{
  using namespace bes;
  typedef core::term_traits<boolean_expression> tr;

  boolean_variable X1("X1");
  boolean_variable X2("X2");
  boolean_variable X3("X3");
  boolean_expression t1 = tr::and_(X1, X2);
  boolean_equation e1(fixpoint_symbol::mu(), X1, tr::imp(X1, X2));
  boolean_equation e2(fixpoint_symbol::nu(), X2, tr::or_(X1, X2));
  boolean_equation e3(fixpoint_symbol::nu(), X3, tr::false_());
  std::cout << bes::pp(e1) << std::endl;
  std::cout << bes::pp(e2) << std::endl;
  std::cout << bes::pp(e3) << std::endl;

  boolean_equation_system<> p;
  p.equations().push_back(e1);
  p.equations().push_back(e2);
  p.equations().push_back(e3);
  p.initial_state() = X1;
  std::cout << "----------------" << std::endl;
  std::cout << pp(p) << std::endl;

  pbes_system::pbes<> q = bes2pbes(p);
  std::cout << "----------------" << std::endl;
  std::cout << pp(q) << std::endl;

  core::garbage_collect();
}

int test_main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT_DEBUG(argc, argv)

  test_boolean_expressions();
  test_bes2pbes();

  return 0;
}
