// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING).
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file untime_test.cpp
/// \brief Add your file description here.

#include <iostream>
#include <string>
#include <boost/test/minimal.hpp>
#include <mcrl2/lps/specification.h>
#include <mcrl2/lps/untime.h>
#include <mcrl2/lps/mcrl22lps.h>

using namespace atermpp;
using namespace mcrl2::data;
using namespace mcrl2::lps;

/*
 * Trivial test: LPS should be left as is, because there is no time involved.
 */
void test_case_1()
{
  const std::string text(
    "act a,b;\n"
    "proc P = a . b . P;\n"
    "init P;\n"
  );

  specification s0 = mcrl22lps(text);
  specification s1 = untime(s0);
  summand_list summands1 = s1.process().summands();
  for(summand_list::iterator i = summands1.begin(); i != summands1.end(); ++i)
  {
    BOOST_CHECK(!i->has_time());
  }

  BOOST_CHECK(s0 == s1);
}

/*
 * An extra process parameter (say "lat") of type Real is introduced
 * time is removed from the actions, and the condition
 * is weakened with time > lat (i.e. 2 > lat or 3 > lat in this case).
 * Furthermore a summand true->delta is introduced.
 */
void test_case_2()
{
  const std::string text(
    "act a,b;\n"
    "proc P = a@2 . b@3 . P;\n"
    "init P;\n"
  );

  specification s0 = mcrl22lps(text);
  specification s1 = untime(s0);
  summand_list summands0 = s0.process().summands();
  summand_list summands1 = s1.process().summands();
  BOOST_CHECK(s0.process().process_parameters().size() == s1.process().process_parameters().size() - 1);
  for(summand_list::iterator i = summands1.begin(); i != summands1.end(); ++i)  
  {
    BOOST_CHECK(!i->has_time());
  }
}

/*
 * An extra process parameter (say "lat") of type Real is introduced
 * time is removed from the actions, and the condition
 * is weakened with time > lat (i.e. 2 > lat or 3 > lat in this case).
 * In the untimed summand, a summation over Real (say sum tv:Real) is introduced,
 * and the condition is weakened with tv > lat.
 * Furthermore a summand true->delta is introduced.
 */
void test_case_3()
{
  const std::string text(
    "act a,b;\n"
    "proc P = a@2 . b@3 . P\n"
    "       + a . P;\n"
    "init P;\n"
  );

  specification s0 = mcrl22lps(text);
  specification s1 = untime(s0);
  summand_list summands0 = s0.process().summands();
  summand_list summands1 = s1.process().summands();
  BOOST_CHECK(s0.process().process_parameters().size() == s1.process().process_parameters().size() - 1);
  int sumvar_count = 0;
  for(summand_list::iterator i = summands1.begin(); i != summands1.end(); ++i)  
  {
    BOOST_CHECK(!i->has_time());
    sumvar_count += i->summation_variables().size();
  }
  BOOST_CHECK(sumvar_count == 1);
}

/*
 * An extra process parameter (say "lat") of type Real is introduced
 * time is removed from the actions, and the condition
 * is weakened with time > lat (i.e. 2 > lat or 3 > lat in this case).
 * In the untimed summand, a summation over Real (say sum tv:Real) is introduced,
 * and the condition is weakened with tv > lat.
 */
void test_case_4()
{
  const std::string text(
    "act a,b;\n"
    "proc P = a@2 . b@3 . P\n"
    "       + a . P\n"
    "       + true -> delta;\n"
    "init P;\n"
  );

  specification s0 = mcrl22lps(text);
  specification s1 = untime(s0);
  summand_list summands0 = s0.process().summands();
  summand_list summands1 = s1.process().summands();
  BOOST_CHECK(s0.process().process_parameters().size() == s1.process().process_parameters().size() - 1);
  int sumvar_count = 0;
  for(summand_list::iterator i = summands1.begin(); i != summands1.end(); ++i)  
  {
    BOOST_CHECK(!i->has_time());
    sumvar_count += i->summation_variables().size();
  }
  BOOST_CHECK(sumvar_count == 1);
}

int test_main(int ac, char** av)
{
  MCRL2_ATERM_INIT(ac, av)

  test_case_1();
  test_case_2();
  test_case_3();
  test_case_4();

  return 0;
}

