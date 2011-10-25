// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file action_rename_test.cpp
/// \brief Action rename test.

#include <boost/test/minimal.hpp>
#include "mcrl2/lps/action_rename.h"
#include "mcrl2/lps/linearise.h"
#include "mcrl2/core/garbage_collection.h"
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/lps/rewrite.h"
#include "mcrl2/lps/parse.h"
#include "mcrl2/lps/remove.h"

using namespace mcrl2;
using lps::specification;
using lps::action_rename_specification;
// using lps::action_rename;

void test1()
{
  // Check a renaming when more than one renaming rule
  // for one action is present.
  const std::string SPEC =
    "act a:Nat;                               \n"
    "proc P(n:Nat) = sum m: Nat. a(m). P(m);  \n"
    "init P(0);                               \n";

  const std::string AR_SPEC =
    "act b,c:Nat;\n"
    "var n:Nat;\n"
    "rename \n"
    "  (n>4)  -> a(n) => b(n); \n"
    "  (n<22) -> a(n) => c(n); \n";

  specification spec = lps::linearise(SPEC);
  std::istringstream ar_spec_stream(AR_SPEC);
  action_rename_specification ar_spec = parse_action_rename_specification(ar_spec_stream, spec);
  specification new_spec = action_rename(ar_spec,spec);
  BOOST_CHECK(new_spec.process().summand_count()==3);
}

void test2()
{
  // Check whether new declarations in the rename file
  // are read properly. Check for renamings of more than one action.
  const std::string SPEC =
    "act a,b:Nat;                             \n"
    "proc P(n:Nat) = sum m: Nat. a(m). P(m);  \n"
    "init P(0);                               \n";

  const std::string AR_SPEC =
    "map f:Nat->Nat; \n"
    "var n':Nat; \n"
    "eqn f(n')=3; \n"
    "act c:Nat; \n"
    "var n:Nat; \n"
    "rename \n"
    "  (f(n)>23) -> a(n) => b(n); \n"
    "  b(n) => c(n); \n";

  specification spec = lps::linearise(SPEC);
  std::istringstream ar_spec_stream(AR_SPEC);
  action_rename_specification ar_spec = parse_action_rename_specification(ar_spec_stream, spec);
  specification new_spec = action_rename(ar_spec,spec);
  BOOST_CHECK(new_spec.process().summand_count()==2);
}

void test3()
{
  // Check whether constants in an action_rename file are properly translated.
  const std::string SPEC =
    "act a,b:Nat;                             \n"
    "proc P(n:Nat) = a(1). P(n)+b(1).P(1);  \n"
    "init P(0);                               \n";

  const std::string AR_SPEC =
    "rename \n"
    "  a(1) => delta; \n";

  specification spec = lps::linearise(SPEC);
  std::istringstream ar_spec_stream(AR_SPEC);
  action_rename_specification ar_spec = parse_action_rename_specification(ar_spec_stream, spec);
  data::rewriter R (spec.data(), mcrl2::data::rewriter::strategy());
  specification new_spec = action_rename(ar_spec,spec);
  lps::rewrite(new_spec, R);
  lps::remove_trivial_summands(new_spec);
  BOOST_CHECK(new_spec.process().summand_count()==2);
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT(argc, argv)
  test1();
  core::garbage_collect();
  test2();
  core::garbage_collect();
  test3();

  return 0;
}
