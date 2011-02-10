// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file next_state_generator_test.cpp
/// \brief Test for next_state_generator class.

#include <queue>
#include <boost/test/included/unit_test_framework.hpp>
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/atermpp/deque.h"
#include "mcrl2/core/garbage_collection.h"
#include "mcrl2/lps/next_state_generator.h"
#include "mcrl2/lps/parse.h"

using namespace mcrl2;
using namespace mcrl2::data;
using namespace mcrl2::data::detail;
using namespace mcrl2::lps;
using namespace mcrl2::lps::detail;

void test_initial_state_successors(const specification& lps_spec)
{
  next_state_generator generator(lps_spec);
  next_state_generator::iterator first = generator.begin();
  while (++first)
  {
    std::cout << generator.print_state(*first) << std::endl;
  }
  core::garbage_collect();
}

void test_next_state_generator(const specification& lps_spec, size_t expected_states, size_t expected_transitions, size_t expected_transition_labels, bool per_summand = false)
{
  next_state_generator generator(lps_spec);

  atermpp::aterm initial_state = generator.initial_state();

  atermpp::set<atermpp::aterm> visited;
  atermpp::set<atermpp::aterm> seen;
  atermpp::set<atermpp::aterm_appl> transition_labels;
  size_t transitions = 0;

  std::queue<atermpp::aterm, atermpp::deque<atermpp::aterm> > q;
  q.push(initial_state);
  seen.insert(initial_state);

  while (!q.empty())
  {
    visited.insert(q.front());

    if (per_summand)
    {
      for (size_t i = 0; i < lps_spec.process().summand_count(); ++i)
      {
        next_state_generator::iterator first = generator.begin(q.front(), i);
        while (++first)
        {
          const next_state_generator::state_type& s = *first;
          transition_labels.insert(s.transition);
          ++transitions;
          if (seen.find(s.state) == seen.end())
          {
            q.push(s.state);
            seen.insert(s.state);
          }
        }
      }
    }
    else
    {
      next_state_generator::iterator first = generator.begin(q.front());
      while (++first)
      {
        const next_state_generator::state_type& s = *first;
        transition_labels.insert(s.transition);
        ++transitions;
        if (seen.find(s.state) == seen.end())
        {
          q.push(s.state);
          seen.insert(s.state);
        }
      }
    }
    q.pop();
  }

  BOOST_CHECK(seen.size() == visited.size());
  BOOST_CHECK(seen.size() == expected_states);
  BOOST_CHECK(transitions == expected_transitions);
  BOOST_CHECK(transition_labels.size() == expected_transition_labels);
}

BOOST_AUTO_TEST_CASE(test_abp)
{
  std::string text(
    "sort Error = struct e;\n"
    "    D = struct d1 | d2;\n"

    "act  i;\n"
    "     c6,r6,s6: Error;\n"
    "     c6,r6,s6,c5,r5,s5: Bool;\n"
    "     c3,r3,s3: Error;\n"
    "     c3,r3,s3,c2,r2,s2: D # Bool;\n"
    "     s4,r1: D;\n"

    "proc P(s30_S: Pos, d_S: D, b_S: Bool, s31_K: Pos, d_K: D, b_K: Bool, s32_L: Pos, b_L: Bool, s33_R: Pos, d_R: D, b_R: Bool) =\n"
    "       sum d0_S: D.\n"
    "         (s30_S == 1) ->\n"
    "         r1(d0_S) .\n"
    "         P(s30_S = 2, d_S = d0_S)\n"
    "     + sum e0_K: Bool.\n"
    "         (s31_K == 2) ->\n"
    "         i .\n"
    "         P(s31_K = if(e0_K, 4, 3), d_K = if(e0_K, d1, d_K), b_K = if(e0_K, true, b_K))\n"
    "     + sum e1_L: Bool.\n"
    "         (s32_L == 2) ->\n"
    "         i .\n"
    "         P(s32_L = if(e1_L, 4, 3), b_L = if(e1_L, true, b_L))\n"
    "     + (s33_R == 2) ->\n"
    "         s4(d_R) .\n"
    "         P(s33_R = 3, d_R = d1)\n"
    "     + sum e2_R: Bool.\n"
    "         (s32_L == 1 && if(e2_R, s33_R == 4, s33_R == 3)) ->\n"
    "         c5(if(e2_R, !b_R, b_R)) .\n"
    "         P(s32_L = 2, b_L = if(e2_R, !b_R, b_R), s33_R = 1, d_R = d1, b_R = if(e2_R, b_R, !b_R))\n"
    "     + (s31_K == 4 && s33_R == 1) ->\n"
    "         c3(e) .\n"
    "         P(s31_K = 1, d_K = d1, b_K = true, s33_R = 4, d_R = d1)\n"
    "     + sum e3_R: Bool.\n"
    "         ((s31_K == 3 && s33_R == 1) && if(e3_R, !b_R, b_R) == b_K) ->\n"
    "         c3(d_K, if(e3_R, !b_R, b_R)) .\n"
    "         P(s31_K = 1, d_K = d1, b_K = true, s33_R = if(e3_R, 4, 2), d_R = if(e3_R, d1, d_K))\n"
    "     + (s30_S == 2 && s31_K == 1) ->\n"
    "         c2(d_S, b_S) .\n"
    "         P(s30_S = 3, s31_K = 2, d_K = d_S, b_K = b_S)\n"
    "     + (s30_S == 3 && s32_L == 4) ->\n"
    "         c6(e) .\n"
    "         P(s30_S = 2, s32_L = 1, b_L = true)\n"
    "     + sum e_S: Bool.\n"
    "         ((s30_S == 3 && s32_L == 3) && if(e_S, b_S, !b_S) == b_L) ->\n"
    "         c6(if(e_S, b_S, !b_S)) .\n"
    "         P(s30_S = if(e_S, 1, 2), d_S = if(e_S, d1, d_S), b_S = if(e_S, !b_S, b_S), s32_L = 1, b_L = true)\n"
    "     + delta;\n"

    "init P(1, d1, true, 1, d1, true, 1, true, 1, d1, true);\n"
  );

  specification spec = parse_linear_process_specification(text);

  // The current next state generator requires this...
  spec.process().deadlock_summands().clear();

  test_initial_state_successors(spec);
  test_next_state_generator(spec, 74, 92, 19);
  test_next_state_generator(spec, 74, 92, 19, true);
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  return 0;
}
