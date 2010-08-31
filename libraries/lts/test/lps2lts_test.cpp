// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lts_test.cpp
/// \brief Add your file description here.

#ifndef MCRL2_WITH_GARBAGE_COLLECTION
#define MCRL2_WITH_GARBAGE_COLLECTION
#endif

#include <stdio.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/test/included/unit_test_framework.hpp>
#include "mcrl2/core/garbage_collection.h"
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/parse.h"
#include "mcrl2/lts/exploration.h"
#include "mcrl2/lts/lts.h"
#include "mcrl2/utilities/test_utilities.h"

using mcrl2::utilities::collect_after_test_case;
using namespace mcrl2;

// Get filename based on timestamp
// Warning: is prone to race conditions
std::string temporary_filename(std::string const& prefix = "")
{
  time_t now = time(NULL);
  std::stringstream now_s;
  now_s << now;

  std::string basename(prefix + now_s.str());
  boost::filesystem::path result(basename);
  int suffix = 0;
  while (boost::filesystem::exists(result))
  {
    std::stringstream suffix_s;
    suffix_s << suffix;
    result = boost::filesystem::path(basename + suffix_s.str());
    ++suffix;
  }
  return result.string();
}

std::string nextstate_format_to_string(const NextStateFormat f)
{
  switch(f)
  {
    case GS_STATE_VECTOR:
      return std::string("vector");
    case GS_STATE_TREE:
      return std::string("tree");
    default:
      return std::string("unknown");
  }
}

BOOST_GLOBAL_FIXTURE(collect_after_test_case)

lts::lts translate_lps_to_lts(lps::specification const& specification,
                              lts::exploration_strategy const strategy = lts::es_breadth,
                              mcrl2::data::rewriter::strategy const rewrite_strategy = mcrl2::data::rewriter::jitty,
                              NextStateFormat format = GS_STATE_VECTOR,
                              std::string priority_action = "")
{
  std::clog << "Translating LPS to LTS with exploration strategy " << std::string(expl_strat_to_str(strategy)) << ", rewrite strategy " << pp(rewrite_strategy) << ", and state format " << nextstate_format_to_string(format) << "." << std::endl;
  lts::lts_generation_options options;
  options.trace_prefix = "lps2lts_test";
  options.specification = specification;
  options.priority_action = priority_action;
  options.strat = rewrite_strategy;
  options.expl_strat = strategy;
  options.stateformat = format;

  options.lts = temporary_filename("lps2lts_test");
  options.outformat = lts::lts_aut;

  lts::lps2lts_algorithm lps2lts;
  core::garbage_collect();
  lps2lts.initialise_lts_generation(&options);
  core::garbage_collect();
  lps2lts.generate_lts();
  core::garbage_collect();
  lps2lts.finalise_lts_generation();
  core::garbage_collect();

  lts::lts result(options.lts, options.outformat);

  boost::filesystem::remove(options.lts.c_str()); // Clean up after ourselves

  return result;
}

// Configure rewrite strategies to be used.
typedef mcrl2::data::basic_rewriter< mcrl2::data::data_expression >::strategy rewrite_strategy;
typedef std::vector<rewrite_strategy > rewrite_strategy_vector;

static inline
rewrite_strategy_vector initialise_rewrite_strategies()
{
  std::vector<rewrite_strategy> result;
  result.push_back(mcrl2::data::basic_rewriter< mcrl2::data::data_expression >::jitty);
  result.push_back(mcrl2::data::basic_rewriter< mcrl2::data::data_expression >::innermost);
#ifdef MCRL2_TEST_COMPILERS
#ifdef MCRL2_JITTYC_AVAILABLE
  result.push_back(mcrl2::data::basic_rewriter< mcrl2::data::data_expression >::jitty_compiling);
#endif // MCRL2_JITTYC_AVAILABLE
#ifdef MCRL2_INNERC_AVAILABLE
  result.push_back(mcrl2::data::basic_rewriter< mcrl2::data::data_expression >::innermost_compiling);
#endif // MCRL2_JITTYC_AVAILABLE
#endif // MCRL2_TEST_COMPILERS
  return result;
}

static inline
rewrite_strategy_vector rewrite_strategies()
{
  static rewrite_strategy_vector rewrite_strategies = initialise_rewrite_strategies();
  return rewrite_strategies;
}

// Configure exploration strategies to be tested;
typedef std::vector< lts::exploration_strategy > exploration_strategy_vector;

static inline
exploration_strategy_vector initialise_exploration_strategies()
{
  exploration_strategy_vector result;
  result.push_back(lts::es_breadth);
  result.push_back(lts::es_depth);
  //result.push_back(lts::es_random);
  return result;
}

static inline
exploration_strategy_vector exploration_strategies()
{
  static exploration_strategy_vector exploration_strategies = initialise_exploration_strategies();
  return exploration_strategies;
}

// State formats to be tested;
typedef std::vector< NextStateFormat > nextstate_format_vector;

static inline
nextstate_format_vector initialise_nextstate_formats()
{
  nextstate_format_vector result;
  result.push_back(GS_STATE_VECTOR);
  result.push_back(GS_STATE_TREE);
  return result;
}

static inline
nextstate_format_vector nextstate_formats()
{
  static nextstate_format_vector nextstate_formats = initialise_nextstate_formats();
  return nextstate_formats;
}

void check_lps2lts_specification(std::string const& specification,
                                 const unsigned int expected_states,
                                 const unsigned int expected_transitions,
                                 const unsigned int expected_labels,
                                 std::string priority_action = "")
{
  lps::specification lps = lps::parse_linear_process_specification(specification);

  rewrite_strategy_vector rstrategies(rewrite_strategies());
  for(rewrite_strategy_vector::const_iterator rewr_strategy = rstrategies.begin(); rewr_strategy != rstrategies.end(); ++rewr_strategy)
  {
    exploration_strategy_vector estrategies(exploration_strategies());
    for(exploration_strategy_vector::const_iterator expl_strategy = estrategies.begin(); expl_strategy != estrategies.end(); ++expl_strategy)
    {
      nextstate_format_vector nsformats(nextstate_formats());
      for(nextstate_format_vector::const_iterator state_format = nsformats.begin(); state_format != nsformats.end(); ++state_format)
      {
        lts::lts result = translate_lps_to_lts(lps, *expl_strategy, *rewr_strategy, *state_format, priority_action);

        BOOST_CHECK_EQUAL(result.num_states(), expected_states);
        BOOST_CHECK_EQUAL(result.num_transitions(), expected_transitions);
        BOOST_CHECK_EQUAL(result.num_labels(), expected_labels);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(test_a_delta) {
  std::string lps(
    "act a;\n"
    "proc P(b:Bool) = (b) -> a.P(!b)\n"
    "               + delta;\n"
    "init P(true);\n"
  );
  check_lps2lts_specification(lps, 2, 1, 1);
}

BOOST_AUTO_TEST_CASE(test_abp) {
  std::string abp(
    "sort Error = struct e;\n"
    "     D = struct d1 | d2;\n"
    "\n"
    "act  i;\n"
    "     c6,r6,s6: Error;\n"
    "     c6,r6,s6,c5,r5,s5: Bool;\n"
    "     c3,r3,s3: Error;\n"
    "     c3,r3,s3,c2,r2,s2: D # Bool;\n"
    "     s4,r1: D;\n"
    "\n"
    "glob dc,dc0,dc1,dc3,dc5,dc7,dc13,dc14,dc15,dc16,dc17,dc18: D;\n"
    "     dc2,dc4,dc6,dc8,dc9,dc10,dc11,dc12: Bool;\n"
    "\n"
    "proc P(s30_S: Pos, d_S: D, b_S: Bool, s31_K: Pos, d_K: D, b_K: Bool, s32_L: Pos, b_L: Bool, s33_R: Pos, d_R: D, b_R: Bool) =\n"
    "       sum d0_S: D.\n"
    "         (s30_S == 1) ->\n"
    "         r1(d0_S) .\n"
    "         P(s30_S = 2, d_S = d0_S)\n"
    "     + sum e0_K: Bool.\n"
    "         (s31_K == 2) ->\n"
    "         i .\n"
    "         P(s31_K = if(e0_K, 4, 3), d_K = if(e0_K, dc3, d_K), b_K = if(e0_K, dc4, b_K))\n"
    "     + sum e1_L: Bool.\n"
    "         (s32_L == 2) ->\n"
    "         i .\n"
    "         P(s32_L = if(e1_L, 4, 3), b_L = if(e1_L, dc10, b_L))\n"
    "     + (s33_R == 2) ->\n"
    "         s4(d_R) .\n"
    "         P(s33_R = 3, d_R = dc16)\n"
    "     + sum e2_R: Bool.\n"
    "         (s32_L == 1 && if(e2_R, s33_R == 4, s33_R == 3)) ->\n"
    "         c5(if(e2_R, !b_R, b_R)) .\n"
    "         P(s32_L = 2, b_L = if(e2_R, !b_R, b_R), s33_R = 1, d_R = if(e2_R, dc18, dc17), b_R = if(e2_R, b_R, !b_R))\n"
    "     + (s31_K == 4 && s33_R == 1) ->\n"
    "         c3(e) .\n"
    "         P(s31_K = 1, d_K = dc7, b_K = dc8, s33_R = 4, d_R = dc15)\n"
    "     + sum e3_R: Bool.\n"
    "         ((s31_K == 3 && s33_R == 1) && if(e3_R, !b_R, b_R) == b_K) ->\n"
    "         c3(d_K, if(e3_R, !b_R, b_R)) .\n"
    "         P(s31_K = 1, d_K = dc5, b_K = dc6, s33_R = if(e3_R, 4, 2), d_R = if(e3_R, dc14, d_K))\n"
    "     + (s30_S == 2 && s31_K == 1) ->\n"
    "         c2(d_S, b_S) .\n"
    "         P(s30_S = 3, s31_K = 2, d_K = d_S, b_K = b_S)\n"
    "     + (s30_S == 3 && s32_L == 4) ->\n"
    "         c6(e) .\n"
    "         P(s30_S = 2, s32_L = 1, b_L = dc12)\n"
    "     + sum e_S: Bool.\n"
    "         ((s30_S == 3 && s32_L == 3) && if(e_S, b_S, !b_S) == b_L) ->\n"
    "         c6(if(e_S, b_S, !b_S)) .\n"
    "         P(s30_S = if(e_S, 1, 2), d_S = if(e_S, dc0, d_S), b_S = if(e_S, !b_S, b_S), s32_L = 1, b_L = dc11)\n"
    "     + delta;\n"
    "\n"
    "init P(1, dc, true, 1, dc1, dc2, 1, dc9, 1, dc13, true);\n"
  );
  check_lps2lts_specification(abp, 74, 92, 19);
}

BOOST_AUTO_TEST_CASE(test_confluence)
{
  std::string spec(
    "sort State = struct S_FSM_UNINITIALIZED?isS_FSM_UNINITIALIZED | S_OFF?isS_OFF;\n"
    "     IdList = List(Nat);\n"
    "\n"
    "act  rs: State;\n"
    "\n"
    "proc P(s3_P: Pos, s1_P: State, ActPhaseArgs_pp2_P: IdList) =\n"
    "       (s3_P == 1 && ActPhaseArgs_pp2_P == []) ->\n"
    "         tau .\n"
    "         P(s3_P = 1, s1_P = S_FSM_UNINITIALIZED, ActPhaseArgs_pp2_P = [12, 9])\n"
    "     + sum s11_P: State.\n"
    "         (s3_P == 1 && !(ActPhaseArgs_pp2_P == [])) ->\n"
    "         rs(s11_P) .\n"
    "         P(s3_P = 2, s1_P = s11_P)\n"
    "     + delta;\n"
    "\n"
    "init P(1, S_FSM_UNINITIALIZED, []);\n"
  );
  check_lps2lts_specification(spec, 4, 3, 3);
  check_lps2lts_specification(spec, 3, 2, 2, "tau");
}

BOOST_AUTO_TEST_CASE(test_function_updates)
{
  std::string spec(
    "act  set,s: Pos;\n"
    "\n"
    "proc P(b_Sensor: Pos -> Bool) =\n"
    "       sum n_Sensor: Pos.\n"
    "         (n_Sensor <= 2) ->\n"
    "         s(n_Sensor) .\n"
    "         P(b_Sensor = b_Sensor[n_Sensor -> true])\n"
    "     + sum n_Sensor0: Pos.\n"
    "         (b_Sensor(n_Sensor0) && n_Sensor0 <= 2) ->\n"
    "         set(n_Sensor0) .\n"
    "         P(b_Sensor = b_Sensor[n_Sensor0 -> false])\n"
    "     + delta;\n"
    "\n"
    "init P(lambda n: Pos. false);\n"
  );
  check_lps2lts_specification(spec, 4, 12, 4);
}

BOOST_AUTO_TEST_CASE(test_timed) // For bug #756
{
  std::string spec(
    "act  a,Terminate;\n"
    "\n"
    "proc P(s3: Pos) =\n"
    "       (s3 == 1) ->\n"
    "         a @ 3 .\n"
    "         P(s3 = 2)\n"
    "     + (s3 == 2) ->\n"
    "         Terminate .\n"
    "         P(s3 = 3)\n"
    "     + (s3 == 3) ->\n"
    "         delta;\n"
    "\n"
    "init P(1);\n"
  );
  check_lps2lts_specification(spec, 3, 2, 2);
}

BOOST_AUTO_TEST_CASE(test_struct)
{
  std::string spec(
      "sort Bits = struct b0 | b1;\n"
      "     t_sys_regset_fsm_state = Bits;\n"
      "     t_timer_counter_fsm_state = Bits;\n"
      "map  timer_counter_fsm_state_idle: Bits;\n"
      "act  a: t_sys_regset_fsm_state;\n"
      "glob globd: t_sys_regset_fsm_state;\n"
      "proc P(s3_P: Pos) =\n"
      "       (s3_P == 1) ->\n"
      "         a(globd) .\n"
      "         P(s3_P = 2)\n"
      "     + delta;\n"
      "init P(1);\n"
      );
  check_lps2lts_specification(spec, 2, 1, 1);
}

BOOST_AUTO_TEST_CASE(test_alias_complex)
{
  std::string spec(
      "sort Bits = struct singleBit(bit: Bool)?isSingleBit | bitVector(bitVec: List(Bool))?isBitVector;\n"
      "     t_sys_regset_fsm_state = Bits;\n"
      "     t_timer_counter_fsm_state = Bits;\n"
      "\n"
      "map  repeat_rec: Bool # Nat -> List(Bool);\n"
      "     repeat: Bool # Nat -> Bits;\n"
      "\n"
      "var  b: Bool;\n"
      "     n: Nat;\n"
      "eqn  repeat(b, n)  =  if(n <= 1, singleBit(b), bitVector(repeat_rec(b, n)));\n"
      "\n"
      "act  a: t_sys_regset_fsm_state;\n"
      "\n"
      "proc P(s3: Pos) =\n"
      "       (s3 == 1) ->\n"
      "         a(repeat(true, 32)) .\n"
      "         P(s3 = 2)\n"
      "     + delta;\n"
      "\n"
      "init P(1);\n"
      );
  check_lps2lts_specification(spec, 2, 1, 1);
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  return 0;
}

