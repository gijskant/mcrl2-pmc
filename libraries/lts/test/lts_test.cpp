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

#include <iostream>
#include <sstream>
#include <boost/test/minimal.hpp>
#include "mcrl2/lts/lts_algorithm.h"
#include "mcrl2/lts/lts_aut.h"

using namespace mcrl2;

class expected_sizes
{
  public:
    size_t states_plain, transitions_plain, labels_plain;
    size_t states_bisimulation, transitions_bisimulation, labels_bisimulation;
    size_t states_branching_bisimulation, transitions_branching_bisimulation, labels_branching_bisimulation;
    size_t states_divergence_preserving_branching_bisimulation, transitions_divergence_preserving_branching_bisimulation, labels_divergence_preserving_branching_bisimulation;
    size_t states_weak_bisimulation, transitions_weak_bisimulation, labels_weak_bisimulation;
    size_t states_divergence_preserving_weak_bisimulation, transitions_divergence_preserving_weak_bisimulation, labels_divergence_preserving_weak_bisimulation;
    size_t states_simulation, transitions_simulation, labels_simulation;
    size_t states_trace_equivalence, transitions_trace_equivalence, labels_trace_equivalence;
    size_t states_weak_trace_equivalence, transitions_weak_trace_equivalence, labels_weak_trace_equivalence;
    size_t states_determinisation, transitions_determinisation, labels_determinisation;
    bool is_deterministic;
};

void test_lts(const std::string& test_description,
              const lts::lts_aut_t& l,
              size_t expected_label_count,
              size_t expected_state_count,
              size_t expected_transition_count
             )
{
  std::cerr << "LPS test: " << test_description << " -----------------------------------------------\n";
  BOOST_CHECK(l.num_action_labels() == expected_label_count);
  if (l.num_action_labels() != expected_label_count)
  {
    std::cerr << "Expected # of labels " << expected_label_count << " Actual # " << l.num_action_labels() << "\n";
  }
  BOOST_CHECK(l.num_states() == expected_state_count);
  if (l.num_states() != expected_state_count)
  {
    std::cerr << "Expected # of states " << expected_state_count << " Actual # " << l.num_states() << "\n";
  }
  BOOST_CHECK(l.num_transitions() == expected_transition_count);
  if (l.num_transitions() != expected_transition_count)
  {
    std::cerr << "Expected # of transitions " << expected_transition_count << " Actual # " << l.num_transitions() << "\n";
  }
}

void reduce_lts_in_various_ways(const std::string& test_description,
                                const std::string lts,
                                const expected_sizes& expected)
{
  std::istringstream is(lts);
  lts::lts_aut_t l_in;
  l_in.load(is);
  test_lts(test_description + " (plain input)",l_in, expected.labels_plain,expected.states_plain, expected.transitions_plain);
  lts::lts_aut_t l=l_in;
  reduce(l,lts::lts_eq_none);
  test_lts(test_description + " (no reduction)",l, expected.labels_plain,expected.states_plain, expected.transitions_plain);
  l=l_in;
  reduce(l,lts::lts_eq_bisim);
  test_lts(test_description + " (bisimulation)",l, expected.labels_bisimulation,expected.states_bisimulation, expected.transitions_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_bisim_gw);
  test_lts(test_description + " (bisimulation gw)",l, expected.labels_bisimulation,expected.states_bisimulation, expected.transitions_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_bisim_sigref);
  test_lts(test_description + " (bisimulation signature)",l, expected.labels_bisimulation,expected.states_bisimulation, expected.transitions_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_branching_bisim);
  test_lts(test_description + " (branching bisimulation)",l, expected.labels_branching_bisimulation,expected.states_branching_bisimulation, expected.transitions_branching_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_branching_bisim_gw);
  test_lts(test_description + " (branching bisimulation gw)",l, expected.labels_branching_bisimulation,expected.states_branching_bisimulation, expected.transitions_branching_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_branching_bisim_sigref);
  test_lts(test_description + " (branching bisimulation signature)",l, expected.labels_branching_bisimulation,expected.states_branching_bisimulation, expected.transitions_branching_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_divergence_preserving_branching_bisim);
  test_lts(test_description + " (divergence preserving branching bisimulation)",l, 
                                      expected.labels_divergence_preserving_branching_bisimulation,
                                      expected.states_divergence_preserving_branching_bisimulation, 
                                      expected.transitions_divergence_preserving_branching_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_divergence_preserving_branching_bisim_gw);
  test_lts(test_description + " (divergence preserving branching bisimulation gw)",l, 
                                      expected.labels_divergence_preserving_branching_bisimulation,
                                      expected.states_divergence_preserving_branching_bisimulation, 
                                      expected.transitions_divergence_preserving_branching_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_divergence_preserving_branching_bisim_sigref);
  test_lts(test_description + " (divergence preserving branching bisimulation signature)",l, 
                                      expected.labels_divergence_preserving_branching_bisimulation,
                                      expected.states_divergence_preserving_branching_bisimulation, 
                                      expected.transitions_divergence_preserving_branching_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_weak_bisim);
  test_lts(test_description + " (weak bisimulation)",l, expected.labels_weak_bisimulation,expected.states_weak_bisimulation, expected.transitions_weak_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_weak_bisim_gw);
  test_lts(test_description + " (weak bisimulation gw)",l, expected.labels_weak_bisimulation,expected.states_weak_bisimulation, expected.transitions_weak_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_weak_bisim_sigref);
  test_lts(test_description + " (weak bisimulation sigref)",l, expected.labels_weak_bisimulation,expected.states_weak_bisimulation, expected.transitions_weak_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_divergence_preserving_weak_bisim);
  test_lts(test_description + " (divergence preserving weak bisimulation)",l, 
                                      expected.labels_divergence_preserving_weak_bisimulation,
                                      expected.states_divergence_preserving_weak_bisimulation, 
                                      expected.transitions_divergence_preserving_weak_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_divergence_preserving_weak_bisim_gw);
  test_lts(test_description + " (divergence preserving weak bisimulation gw)",l, 
                                      expected.labels_divergence_preserving_weak_bisimulation,
                                      expected.states_divergence_preserving_weak_bisimulation, 
                                      expected.transitions_divergence_preserving_weak_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_divergence_preserving_weak_bisim_sigref);
  test_lts(test_description + " (divergence preserving weak bisimulation signature)",l, 
                                      expected.labels_divergence_preserving_weak_bisimulation,
                                      expected.states_divergence_preserving_weak_bisimulation, 
                                      expected.transitions_divergence_preserving_weak_bisimulation);
  l=l_in;
  reduce(l,lts::lts_eq_sim);
  test_lts(test_description + " (simulation equivalence)",l, expected.labels_simulation,expected.states_simulation, expected.transitions_simulation);
  l=l_in;
  reduce(l,lts::lts_eq_trace);
  test_lts(test_description + " (trace equivalence)",l, expected.labels_trace_equivalence,expected.states_trace_equivalence, expected.transitions_trace_equivalence);
  l=l_in;
  reduce(l,lts::lts_eq_weak_trace);
  test_lts(test_description + " (weak trace equivalence)",l, expected.labels_weak_trace_equivalence,expected.states_weak_trace_equivalence, expected.transitions_weak_trace_equivalence);
  l=l_in;
  if (expected.is_deterministic)
  {
    BOOST_CHECK(is_deterministic(l));
  }
  else
  {
    BOOST_CHECK(!is_deterministic(l));
  }

  reduce(l,lts::lts_red_determinisation);
  test_lts(test_description + " (determinisation)",l, expected.labels_determinisation,expected.states_determinisation, expected.transitions_determinisation);
  BOOST_CHECK(is_deterministic(l));
}

void reduce_simple_loop()
{
  std::string SIMPLE_AUT =
    "des (0,2,2)\n"
    "(0,\"a\",1)\n"
    "(1,\"a\",0)\n"
    ;

  expected_sizes expected;
  expected.states_plain=2; expected.transitions_plain=2; expected.labels_plain=1;
  expected.states_bisimulation=1, expected.transitions_bisimulation=1, expected.labels_bisimulation=1;
  expected.states_branching_bisimulation=1, expected.transitions_branching_bisimulation=1, expected.labels_branching_bisimulation=1;
  expected.states_divergence_preserving_branching_bisimulation=1, expected.transitions_divergence_preserving_branching_bisimulation=1, expected.labels_divergence_preserving_branching_bisimulation=1;
  expected.states_weak_bisimulation=1, expected.transitions_weak_bisimulation=1, expected.labels_weak_bisimulation=2;
  expected.states_divergence_preserving_weak_bisimulation=1, expected.transitions_divergence_preserving_weak_bisimulation=1, expected.labels_divergence_preserving_weak_bisimulation=3;
  expected.states_simulation=1, expected.transitions_simulation=1, expected.labels_simulation=1;
  expected.states_trace_equivalence=1, expected.transitions_trace_equivalence=1, expected.labels_trace_equivalence=1;
  expected.states_weak_trace_equivalence=1, expected.transitions_weak_trace_equivalence=1, expected.labels_weak_trace_equivalence=1;
  expected.states_determinisation=2, expected.transitions_determinisation=2, expected.labels_determinisation=1;
  expected.is_deterministic=true;

  reduce_lts_in_various_ways("Simple loop", SIMPLE_AUT, expected);
}

void reduce_simple_loop_with_tau()
{
  std::string SIMPLE_AUT =
    "des (0,2,2)\n"
    "(0,\"a\",1)\n"
    "(1,\"tau\",0)\n"
    ;

  expected_sizes expected;
  expected.states_plain=2; expected.transitions_plain=2; expected.labels_plain=2;
  expected.states_bisimulation=2, expected.transitions_bisimulation=2, expected.labels_bisimulation=2;
  expected.states_branching_bisimulation=1, expected.transitions_branching_bisimulation=1, expected.labels_branching_bisimulation=2;
  expected.states_divergence_preserving_branching_bisimulation=1, expected.transitions_divergence_preserving_branching_bisimulation=1, expected.labels_divergence_preserving_branching_bisimulation=2;
  expected.states_weak_bisimulation=1, expected.transitions_weak_bisimulation=1, expected.labels_weak_bisimulation=2;
  expected.states_divergence_preserving_weak_bisimulation=1, expected.transitions_divergence_preserving_weak_bisimulation=1, expected.labels_divergence_preserving_weak_bisimulation=3;
  expected.states_simulation=2, expected.transitions_simulation=2, expected.labels_simulation=2;
  expected.states_trace_equivalence=2, expected.transitions_trace_equivalence=2, expected.labels_trace_equivalence=2;
  expected.states_weak_trace_equivalence=1, expected.transitions_weak_trace_equivalence=1, expected.labels_weak_trace_equivalence=2;
  expected.states_determinisation=2, expected.transitions_determinisation=2, expected.labels_determinisation=2;
  expected.is_deterministic=true;

  reduce_lts_in_various_ways("Simple loop with tau", SIMPLE_AUT, expected);
}


void reduce_abp()
{
  std::string ABP_AUT =
    "des (0,92,74)\n"
    "(0,\"r1(d1)\",1)\n"
    "(0,\"r1(d2)\",2)\n"
    "(1,\"tau\",3)\n"
    "(2,\"tau\",4)\n"
    "(3,\"tau\",5)\n"
    "(3,\"tau\",6)\n"
    "(4,\"tau\",7)\n"
    "(4,\"tau\",8)\n"
    "(5,\"tau\",9)\n"
    "(6,\"tau\",10)\n"
    "(7,\"tau\",11)\n"
    "(8,\"tau\",12)\n"
    "(9,\"tau\",13)\n"
    "(10,\"s4(d1)\",14)\n"
    "(11,\"tau\",15)\n"
    "(12,\"s4(d2)\",16)\n"
    "(13,\"tau\",17)\n"
    "(13,\"tau\",18)\n"
    "(14,\"tau\",19)\n"
    "(15,\"tau\",20)\n"
    "(15,\"tau\",21)\n"
    "(16,\"tau\",22)\n"
    "(17,\"tau\",1)\n"
    "(18,\"tau\",1)\n"
    "(19,\"tau\",23)\n"
    "(19,\"tau\",24)\n"
    "(20,\"tau\",2)\n"
    "(21,\"tau\",2)\n"
    "(22,\"tau\",25)\n"
    "(22,\"tau\",26)\n"
    "(23,\"tau\",27)\n"
    "(24,\"tau\",28)\n"
    "(25,\"tau\",29)\n"
    "(26,\"tau\",28)\n"
    "(27,\"tau\",30) \n"
    "(28,\"r1(d1)\",31)\n"
    "(28,\"r1(d2)\",32)\n"
    "(29,\"tau\",33)\n"
    "(30,\"tau\",34)\n"
    "(30,\"tau\",35)\n"
    "(31,\"tau\",36)\n"
    "(32,\"tau\",37)\n"
    "(33,\"tau\",38)\n"
    "(33,\"tau\",39)\n"
    "(34,\"tau\",40)\n"
    "(35,\"tau\",40)\n"
    "(36,\"tau\",41)\n"
    "(36,\"tau\",42)\n"
    "(37,\"tau\",43)\n"
    "(37,\"tau\",44)\n"
    "(38,\"tau\",45)\n"
    "(39,\"tau\",45)\n"
    "(40,\"tau\",19)\n"
    "(41,\"tau\",46)\n"
    "(42,\"tau\",47)\n"
    "(43,\"tau\",48)\n"
    "(44,\"tau\",49)\n"
    "(45,\"tau\",22)\n"
    "(46,\"tau\",50)\n"
    "(47,\"s4(d1)\",51)\n"
    "(48,\"tau\",52)\n"
    "(49,\"s4(d2)\",53)\n"
    "(50,\"tau\",54)\n"
    "(50,\"tau\",55)\n"
    "(51,\"tau\",56)\n"
    "(52,\"tau\",57)\n"
    "(52,\"tau\",58)\n"
    "(53,\"tau\",59)\n"
    "(54,\"tau\",31)\n"
    "(55,\"tau\",31)\n"
    "(56,\"tau\",60)\n"
    "(56,\"tau\",61)\n"
    "(57,\"tau\",32)\n"
    "(58,\"tau\",32)\n"
    "(59,\"tau\",62)\n"
    "(59,\"tau\",63)\n"
    "(60,\"tau\",64)\n"
    "(61,\"tau\",0)\n"
    "(62,\"tau\",65)\n"
    "(63,\"tau\",0)\n"
    "(64,\"tau\",66)\n"
    "(65,\"tau\",67)\n"
    "(66,\"tau\",68)\n"
    "(66,\"tau\",69)\n"
    "(67,\"tau\",70)\n"
    "(67,\"tau\",71)\n"
    "(68,\"tau\",72)\n"
    "(69,\"tau\",72)\n"
    "(70,\"tau\",73)\n"
    "(71,\"tau\",73)\n"
    "(72,\"tau\",56)\n"
    "(73,\"tau\",59)\n"
    ;

  expected_sizes expected;
  expected.states_plain=74; expected.transitions_plain=92; expected.labels_plain=5;
  expected.states_bisimulation=24, expected.transitions_bisimulation=28, expected.labels_bisimulation=5;
  expected.states_branching_bisimulation=3, expected.transitions_branching_bisimulation=4, expected.labels_branching_bisimulation=5;
  expected.states_divergence_preserving_branching_bisimulation=6, expected.transitions_divergence_preserving_branching_bisimulation=10, expected.labels_divergence_preserving_branching_bisimulation=5;
  expected.states_weak_bisimulation=3, expected.transitions_weak_bisimulation=4, expected.labels_weak_bisimulation=5;
  expected.states_divergence_preserving_weak_bisimulation=6, expected.transitions_divergence_preserving_weak_bisimulation=10, expected.labels_divergence_preserving_weak_bisimulation=5;
  expected.states_simulation=24, expected.transitions_simulation=28, expected.labels_simulation=5;
  expected.states_trace_equivalence=19, expected.transitions_trace_equivalence=24, expected.labels_trace_equivalence=5;
  expected.states_weak_trace_equivalence=3, expected.transitions_weak_trace_equivalence=4, expected.labels_weak_trace_equivalence=5;
  expected.states_determinisation=53, expected.transitions_determinisation=66, expected.labels_determinisation=5;
  expected.is_deterministic=false;

  reduce_lts_in_various_ways("Alternating bit protocol", ABP_AUT, expected);
}

void test_reachability()
{
  std::string REACH =
    "des (0,4,5)       \n"
    "(0,\"reachable\",1)\n"
    "(1,\"reachable1\",2)\n"
    "(1,\"reachable2\",3)\n"
    "(4,\"unreachable\",0)\n"
    ;

  size_t expected_label_count = 4;
  size_t expected_state_count = 5;
  size_t expected_transition_count = 4;

  std::istringstream is(REACH);
  lts::lts_aut_t l_reach;
  l_reach.load(is);
  test_lts("reach test",l_reach, expected_label_count, expected_state_count, expected_transition_count);
  BOOST_CHECK(!reachability_check(l_reach,false));
  reachability_check(l_reach,true);
  test_lts("reach test after reachability reduction",l_reach, expected_label_count-1, expected_state_count-1, expected_transition_count-1);
  BOOST_CHECK(reachability_check(l_reach,false));
}

// The example below caused failures in the GW mlogn branching bisimulation 
// algorithm when cleaning the code up.
void failing_test_groote_wijs_algorithm()
{
  std::string GWLTS =
    "des(0,29,10)\n"
    "(0,\"a\",5)\n"
    "(1,\"a\",3)\n"
    "(2,\"a\",6)\n"
    "(3,\"a\",8)\n"
    "(4,\"a\",5)\n"
    "(5,\"a\",6)\n"
    "(6,\"a\",4)\n"
    "(7,\"a\",8)\n"
    "(8,\"a\",9)\n"
    "(9,\"a\",6)\n"
    "(0,\"b\",9)\n"
    "(1,\"b\",3)\n"
    "(2,\"b\",3)\n"
    "(3,\"b\",7)\n"
    "(4,\"b\",9)\n"
    "(5,\"b\",8)\n"
    "(6,\"b\",5)\n"
    "(7,\"b\",9)\n"
    "(8,\"b\",1)\n"
    "(9,\"b\",6)\n"
    "(6,\"c\",7)\n"
    "(8,\"c\",9)\n"
    "(2,\"d\",7)\n"
    "(3,\"d\",2)\n"
    "(5,\"d\",6)\n"
    "(7,\"d\",4)\n"
    "(8,\"tau\",0)\n"
    "(4,\"tau\",1)\n"
    "(3,\"tau\",7)\n"
    ;

  size_t expected_label_count = 5;
  size_t expected_state_count = 10;
  size_t expected_transition_count = 29;

  std::istringstream is(GWLTS);
  lts::lts_aut_t l_gw;
  l_gw.load(is);
  lts::lts_aut_t l=l_gw;
  reduce(l,lts::lts_eq_branching_bisim);
  test_lts("gw problem branching bisimulation",l,expected_label_count, expected_state_count, expected_transition_count);
  l=l_gw;
  reduce(l,lts::lts_eq_branching_bisim_sigref);
  test_lts("gw problem branching bisimulation signature based",l,expected_label_count, expected_state_count, expected_transition_count);
}

int test_main(int argc, char* argv[])
{
  reduce_simple_loop();
  reduce_simple_loop_with_tau();
  // reduce_abp();
  test_reachability();
  failing_test_groote_wijs_algorithm();

  // TODO: Add groote wijs branching bisimulation and add weak bisimulation tests. For the last Peterson is a good candidate. 
  return 0;
}

