// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file print_test.cpp
/// \brief Regression test for parsing process expressions

#define MCRL2_DEBUG_EXPRESSION_BUILDER

#include <iostream>
#include <boost/test/minimal.hpp>

#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/process/parse.h"
#include "mcrl2/process/print.h"
#include "mcrl2/core/garbage_collection.h"

using namespace mcrl2;
using namespace mcrl2::process;

void test_comm()
{
  std::string text =
    "act  a,b,c;\n"
    "\n"
    "init comm({a | b -> c}, c);\n"
    ;

  process_specification p = parse_process_specification(text);
  std::string text1 = process::pp(p);
  std::cout << text << std::endl;
  std::cout << "---" << std::endl;
  std::cout << text1 << std::endl;
  std::cout << "---" << std::endl;
  BOOST_CHECK(text == text1);
}

void test_action_name_multiset()
{
  atermpp::vector<core::identifier_string> v;
  v.push_back(core::identifier_string("a"));
  v.push_back(core::identifier_string("b"));
  v.push_back(core::identifier_string("c"));
  core::identifier_string_list l(v.begin(), v.end());
  action_name_multiset A(l);
  std::string text = process::pp(A);
  BOOST_CHECK(text == "a | b | c");

  atermpp::term_list<action_name_multiset> w;
  w = atermpp::push_front(w, A);
  w = atermpp::push_front(w, A);
  text = process::pp(w);
  BOOST_CHECK(text == "a | b | c, a | b | c");
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT(argc, argv);

  test_comm();
  test_action_name_multiset();

  return EXIT_SUCCESS;
}
