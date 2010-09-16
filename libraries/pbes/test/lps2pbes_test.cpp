// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lps2pbes_test.cpp
/// \brief Add your file description here.

// Test program for timed lps2pbes.

#define MCRL2_PBES_TRANSLATE_DEBUG

#include <iostream>
#include <iterator>
#include <boost/test/minimal.hpp>
#include <boost/algorithm/string.hpp>
#include "mcrl2/core/text_utility.h"
#include "mcrl2/lps/linearise.h"
#include "mcrl2/modal_formula/parse.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/lps2pbes.h"
#include "mcrl2/pbes/detail/test_utility.h"
#include "mcrl2/pbes/pbes_solver_test.h"
#include "test_specifications.h"
#include "mcrl2/core/garbage_collection.h"
#include "mcrl2/atermpp/aterm_init.h"

using namespace std;
using namespace mcrl2;
using namespace mcrl2::core;
using namespace mcrl2::data;
using namespace mcrl2::lps;
using namespace mcrl2::lps::detail;
using namespace mcrl2::state_formulas;
using namespace mcrl2::pbes_system;
using namespace mcrl2::pbes_system::detail;

#include "boost/filesystem/operations.hpp"
#include "boost/filesystem/path.hpp"
namespace fs = boost::filesystem;

const std::string ABP_SPECIFICATION =
"% This file contains the alternating bit protocol, as described in W.J.    \n"
"% Fokkink, J.F. Groote and M.A. Reniers, Modelling Reactive Systems.       \n"
"%                                                                          \n"
"% The only exception is that the domain D consists of two data elements to \n"
"% facilitate simulation.                                                   \n"
"                                                                           \n"
"sort                                                                       \n"
"  D     = struct d1 | d2;                                                  \n"
"  Error = struct e;                                                        \n"
"                                                                           \n"
"act                                                                        \n"
"  r1,s4: D;                                                                \n"
"  s2,r2,c2: D # Bool;                                                      \n"
"  s3,r3,c3: D # Bool;                                                      \n"
"  s3,r3,c3: Error;                                                         \n"
"  s5,r5,c5: Bool;                                                          \n"
"  s6,r6,c6: Bool;                                                          \n"
"  s6,r6,c6: Error;                                                         \n"
"  i;                                                                       \n"
"                                                                           \n"
"proc                                                                       \n"
"  S(b:Bool)     = sum d:D. r1(d).T(d,b);                                   \n"
"  T(d:D,b:Bool) = s2(d,b).(r6(b).S(!b)+(r6(!b)+r6(e)).T(d,b));             \n"
"                                                                           \n"
"  R(b:Bool)     = sum d:D. r3(d,b).s4(d).s5(b).R(!b)+                      \n"
"                  (sum d:D.r3(d,!b)+r3(e)).s5(!b).R(b);                    \n"
"                                                                           \n"
"  K             = sum d:D,b:Bool. r2(d,b).(i.s3(d,b)+i.s3(e)).K;           \n"
"                                                                           \n"
"  L             = sum b:Bool. r5(b).(i.s6(b)+i.s6(e)).L;                   \n"
"                                                                           \n"
"init                                                                       \n"
"  allow({r1,s4,c2,c3,c5,c6,i},                                             \n"
"    comm({r2|s2->c2, r3|s3->c3, r5|s5->c5, r6|s6->c6},                     \n"
"        S(true) || K || L || R(true)                                       \n"
"    )                                                                      \n"
"  );                                                                       \n"
;

const std::string TRIVIAL_FORMULA  = "[true*]<true*>true";

void test_trivial()
{
  specification spec    = linearise(ABP_SPECIFICATION);
  state_formula formula = state_formulas::parse_state_formula(TRIVIAL_FORMULA, spec);
  bool timed = false;
  pbes<> p = lps2pbes(spec, formula, timed);
  BOOST_CHECK(p.is_well_typed());
  core::garbage_collect();
}

void test_lps2pbes()
{
  std::string SPECIFICATION;
  std::string FORMULA;
  specification spec;
  state_formula formula;
  pbes<> p;
  bool timed = false;

  SPECIFICATION =
  "act a;                                  \n"
  "proc X(n : Nat) = (n > 2) -> a. X(n+1); \n"
  "init X(3);                              \n"
  ;
  FORMULA = "true => false";
  spec    = linearise(SPECIFICATION);
  formula = state_formulas::parse_state_formula(FORMULA, spec);
  p = lps2pbes(spec, formula, timed);
  BOOST_CHECK(p.is_well_typed());

  SPECIFICATION =
  "act a : Nat;                           \n"
  "proc X(n:Nat) = (n>2) -> a(n). X(n+1); \n"
  "init X(3);                             \n"
  ;
  FORMULA = "nu X. (X && forall m:Nat. [a(m)]false)";
  spec    = linearise(SPECIFICATION);
  formula = state_formulas::parse_state_formula(FORMULA, spec);
  p = lps2pbes(spec, formula, timed);
  BOOST_CHECK(p.is_well_typed());

  SPECIFICATION =
  "act a;         \n"
  "proc X = a. X; \n"
  "init X;        \n"
  ;
  FORMULA =
  "(                                 \n"
  "  ( mu A. [!a]A)                  \n"
  "||                                \n"
  "  ( mu B. exists t3:Pos . [!a]B ) \n"
  ")                                 \n"
  ;
  spec    = linearise(SPECIFICATION);
  formula = state_formulas::parse_state_formula(FORMULA, spec);
  p = lps2pbes(spec, formula, timed);
  BOOST_CHECK(p.is_well_typed());
  
  SPECIFICATION =
    "sort Closure = List(Bool);                                                   \n"
    "sort State = struct state(closure: Closure, copy: Nat);                      \n"
    "                                                                             \n"
    "map initial: State -> Bool;                                                  \n"
    "var q: State;                                                                \n"
    "eqn initial(q) = closure(q).0 && (copy(q) == 0);                             \n"
    "                                                                             \n"
    "map accept: State -> Bool;                                                   \n"
    "var q: State;                                                                \n"
    "eqn accept(q) = ((copy(q) == 0) && (closure(q).0 => closure(q).2));          \n"
    "                                                                             \n"
    "map nextstate: State # State -> Bool;                                        \n"
    "var q, q': State;                                                            \n"
    "eqn nextstate(q, q') =                                                       \n"
    "      (#closure(q) == #closure(q')) &&                                       \n"
    "      (accept(q) => (copy(q') == (copy(q) + 1) mod 1)) &&                    \n"
    "      (!accept(q) => (copy(q') == copy(q))) &&                               \n"
    "      (closure(q).0 == (closure(q).2 || (closure(q).1 && closure(q').0))) && \n"
    "      (closure(q').0 => closure(q').1 || closure(q').2) &&                   \n"
    "      (closure(q').2 => closure(q').0);                                      \n"
    "                                                                             \n"
    "act a, b;                                                                    \n"
    "proc P(s: Bool) =  s -> (a . P(false)) <> delta +                            \n"
    "                  !s -> (b . P(s)) <> delta;                                 \n"
    "init P(true);                                                                \n"
    ;

  FORMULA =
    "forall c1: State .                                                                                                                                            \n"
    " (exists c0: State .                                                                                                                                          \n"
    "   (val(initial(c0) && nextstate(c0, c1)) &&                                                                                                                  \n"
    "    (((<a>true) => val(closure(c1).1)) && (val(closure(c1).1) => (<a>true)) && ((<b>true) => val(closure(c1).2)) && (val(closure(c1).2) => (<b>true)))        \n"
    "   )                                                                                                                                                          \n"
    " ) => (                                                                                                                                                       \n"
    "   mu X(c'': State = c1) . (                                                                                                                                  \n"
    "     nu Y(c: State = c'') . (                                                                                                                                 \n"
    "       forall c': State . (                                                                                                                                   \n"
    "         val(nextstate(c, c')) =>                                                                                                                             \n"
    "         [true](                                                                                                                                              \n"
    "           (((<a>true) => val(closure(c).1)) && (val(closure(c).1) => (<a>true)) && ((<b>true) => val(closure(c).2)) && (val(closure(c).2) => (<b>true))) =>  \n"
    "           ((val(accept(c)) && X(c')) || (val(!accept(c)) && Y(c')))                                                                                          \n"
    "         )                                                                                                                                                    \n"
    "       )                                                                                                                                                      \n"
    "     )                                                                                                                                                        \n"
    "   )                                                                                                                                                          \n"
    " )                                                                                                                                                            \n"
    ;

  spec    = linearise(SPECIFICATION);
  formula = state_formulas::parse_state_formula(FORMULA, spec);
  p = lps2pbes(spec, formula, timed);
  BOOST_CHECK(p.is_well_typed());

  core::garbage_collect();
}

void test_lps2pbes2()
{
  std::string FORMULA;
  pbes<> p;
  bool timed = false;

  FORMULA = "mu X. !!X";
  p = lps2pbes(ABP_SPECIFICATION, FORMULA, timed);
  BOOST_CHECK(p.is_well_typed());

  FORMULA = "nu X. ([true]X && <true>true)";
  p = lps2pbes(ABP_SPECIFICATION, FORMULA, timed);
  BOOST_CHECK(p.is_well_typed());

  FORMULA = "nu X. ([true]X && forall d:D. [r1(d)] mu Y. (<true>Y || <s4(d)>true))";
  p = lps2pbes(ABP_SPECIFICATION, FORMULA, timed);
  BOOST_CHECK(p.is_well_typed());

  FORMULA = "forall d:D. nu X. (([!r1(d)]X && [s4(d)]false))";
  p = lps2pbes(ABP_SPECIFICATION, FORMULA, timed);
  BOOST_CHECK(p.is_well_typed());

  FORMULA = "nu X. ([true]X && forall d:D. [r1(d)]nu Y. ([!r1(d) && !s4(d)]Y && [r1(d)]false))";
  p = lps2pbes(ABP_SPECIFICATION, FORMULA, timed);
  BOOST_CHECK(p.is_well_typed());
  core::garbage_collect();
}

void test_lps2pbes3()
{
  std::string SPEC = "init delta;";

  std::string FORMULA =
    "(mu X(n:Nat = 0) . true) \n"
    "&&                       \n"
    "(mu X(n:Nat = 0) . true) \n"
    ;   

  // Expected result:
  //
  // pbes nu X1 =        
  //        Y(0) && X(0);
  //      mu Y(n: Nat) = 
  //        true;        
  //      mu X(n: Nat) = 
  //        true;        
  //                     
  // init X1;            

  pbes<> p;
  bool timed = false;
  p = lps2pbes(SPEC, FORMULA, timed);
  BOOST_CHECK(p.is_well_typed());
  std::cerr << "p = " << core::pp(pbes_to_aterm(p)) << std::endl;
  core::garbage_collect();
}

void test_directory(int argc, char** argv)
{
  BOOST_CHECK(argc > 1);

  // The dummy file test.test is used to extract the full path of the test directory.
  fs::path dummy_path = fs::system_complete( fs::path( argv[1], fs::native ) );
  fs::path dir = dummy_path.branch_path();
  BOOST_CHECK(fs::is_directory(dir));

  fs::directory_iterator end_iter;
  for ( fs::directory_iterator dir_itr(dir); dir_itr != end_iter; ++dir_itr )
  {
    if ( fs::is_regular( dir_itr->status() ) )
    {
      std::string filename = dir_itr->path().file_string();
      if (boost::ends_with(filename, std::string(".form")))
      {
        std::string timed_result_file   = filename.substr(0, filename.find_last_of('.') + 1) + "expected_timed_result";
        std::string untimed_result_file = filename.substr(0, filename.find_last_of('.') + 1) + "expected_untimed_result";
        std::string formula = mcrl2::core::read_text(filename);
        if (fs::exists(timed_result_file))
        {
          try {
            pbes<> result = lps2pbes(SPEC1, formula, true);
            pbes<> expected_result;
            expected_result.load(timed_result_file);
            bool cmp = (result == expected_result);
            if (!cmp)
              cerr << "ERROR: test " << timed_result_file << " failed!" << endl;
            BOOST_CHECK(cmp);
          }
          catch (mcrl2::runtime_error e)
          {
            cerr << e.what() << endl;
          }
        }
        if (fs::exists(untimed_result_file))
        {
          try {
            pbes<> result = lps2pbes(SPEC1, formula, false);
            BOOST_CHECK(result.is_well_typed());
            pbes<> expected_result;
            expected_result.load(untimed_result_file);
            bool cmp = (result == expected_result);
            if (!cmp)
              cerr << "ERROR: test " << untimed_result_file << " failed!" << endl;
            BOOST_CHECK(cmp);
          }
          catch (mcrl2::runtime_error e)
          {
            cerr << e.what() << endl;
          }
        }
      }
    }
  }
  core::garbage_collect();
}

void test_formulas()
{
  std::string SPEC =
  "act a:Nat;                             \n"
  "map smaller: Nat#Nat -> Bool;          \n"
  "var x,y : Nat;                         \n"
  "eqn smaller(x,y) = x < y;              \n"
  "proc P(n:Nat) = sum m: Nat. a(m). P(m);\n"
  "init P(0);                             \n"
  ;

  std::vector<string> formulas;
  formulas.push_back("delay@11");
  formulas.push_back("exists m:Nat. <a(m)>true");
  formulas.push_back("exists p:Nat. <a(p)>true");
  formulas.push_back("forall m:Nat. [a(m)]false");
  formulas.push_back("nu X(n:Nat = 1). [forall m:Nat. a(m)](val(n < 10)  && X(n+2))");
  formulas.push_back("mu X(n:Nat = 1). [forall m:Nat. a(m)](val(smaller(n,10) ) && X(n+2))");
  formulas.push_back("<exists m:Nat. a(m)>true");
  formulas.push_back("<a(2)>[a(0)]false");
  formulas.push_back("<a(2)>true");
  formulas.push_back("[forall m:Nat. a(m)]false");
  formulas.push_back("[a(0)]<a(1)>true");
  formulas.push_back("[a(1)]false");
  formulas.push_back("!true");
  formulas.push_back("yaled@10");

  for (std::vector<string>::iterator i = formulas.begin(); i != formulas.end(); ++i)
  {
    std::cout << "<formula>" << *i << std::flush;
    pbes<> result1 = lps2pbes(SPEC, *i, false);
    std::cout << " <timed>" << std::flush;
    pbes<> result2 = lps2pbes(SPEC, *i, true);
    std::cout << " <untimed>" << std::endl;
  }
  core::garbage_collect();
}

action act(std::string name, data_expression_list parameters)
{
  std::vector<sort_expression> sorts;
  for (data_expression_list::iterator i = parameters.begin(); i != parameters.end(); ++i)
  {
    sorts.push_back(i->sort());
  }
  action_label label(name, sort_expression_list(sorts.begin(), sorts.end()));
  return action(label, parameters);
}

void test_multi_actions(action_list a, action_list b, data_expression expected_result = data_expression())
{
  data_expression result = equal_multi_actions(a, b);
  std::cout << mcrl2::core::pp(result) << std::endl;
  BOOST_CHECK(expected_result == data_expression() || result == expected_result);
  core::garbage_collect();
}

void test_equal_multi_actions()
{
  namespace d = data;

  data_expression d1 = nat("d1");
  data_expression d2 = nat("d2");
  data_expression d3 = nat("d3");
  data_expression d4 = nat("d4");
  action_list a1  = make_list(act("a", make_list(d1)));
  action_list a2  = make_list(act("a", make_list(d2)));
  action_list b1  = make_list(act("b", make_list(d1)));
  action_list b2  = make_list(act("b", make_list(d2)));
  action_list a11 = make_list(act("a", make_list(d1)), act("a", make_list(d1)));
  action_list a12 = make_list(act("a", make_list(d1)), act("a", make_list(d2)));
  action_list a21 = make_list(act("a", make_list(d2)), act("a", make_list(d1)));
  action_list a22 = make_list(act("a", make_list(d2)), act("a", make_list(d2)));
  action_list a34 = make_list(act("a", make_list(d3)), act("a", make_list(d4)));
  action_list a12b1 = make_list(act("a", make_list(d1)), act("a", make_list(d2)), act("b", make_list(d1)));
  action_list a34b2 = make_list(act("a", make_list(d3)), act("a", make_list(d4)), act("b", make_list(d2)));

  test_multi_actions( a1,  a1, d::sort_bool::true_());
  test_multi_actions( a1,  a2, d::equal_to(d1, d2));
  test_multi_actions(a11, a11, d::sort_bool::true_());
  test_multi_actions(a12, a21, d::sort_bool::true_());
  test_multi_actions(a11, a22, d::equal_to(d1, d2));
  test_multi_actions(a1, a12,  d::sort_bool::false_());
  test_multi_actions(a1, b1,   d::sort_bool::false_());
  test_multi_actions(a12, a34);
  test_multi_actions(a12b1, a34b2);
}

const std::string MACHINE_SPECIFICATION =
  "%% file machine.mcrl2                                             \n"
  "                                                                  \n"
  "act                                                               \n"
  "                                                                  \n"
  "  ch_tea, ch_cof, insq, insd, take_tea, take_cof, want_change,    \n"
  "  sel_tea, sel_cof, accq, accd, put_tea, put_cof, put_change,     \n"
  "  ok_tea, ok_coffee, quarter, dollar, tea, coffee, change ;       \n"
  "                                                                  \n"
  "proc                                                              \n"
  "                                                                  \n"
  "  User = ch_tea.UserTea + ch_cof.UserCof ;                        \n"
  "  UserTea =                                                       \n"
  "    insq.insq.take_tea.User  +                                    \n"
  "    insd.take_tea.( take_tea + want_change ).User ;               \n"
  "  UserCof = ( insq.insq.insq.insq + insd ).take_cof.User ;        \n"
  "                                                                  \n"
  "  Mach = sel_tea.MachTea + sel_cof.MachCof ;                      \n"
  "  MachTea =                                                       \n"
  "    accq.accq.put_tea.Mach +                                      \n"
  "    accd.put_tea.( put_tea + put_change ).Mach ;                  \n"
  "  MachCof =                                                       \n"
  "    ( accq.accq.accq.accq + accd ).put_cof.Mach ;                 \n"
  "                                                                  \n"
  "init                                                              \n"
  "                                                                  \n"
  "  allow(                                                          \n"
  "    { ok_tea, ok_coffee, quarter, dollar, tea, coffee, change } , \n"
  "    comm(                                                         \n"
  "       { ch_tea|sel_tea -> ok_tea, ch_cof|sel_cof -> ok_coffee,   \n"
  "         insq|accq -> quarter, insd|accd -> dollar,               \n"
  "         take_tea|put_tea -> tea, take_cof|put_cof -> coffee,     \n"
  "         want_change|put_change -> change } ,                     \n"
  "       User || Mach                                               \n"
  "    ) ) ;                                                         \n"
  ;

const std::string MACHINE_FORMULA1 =
  "%% after choice for tea and two quarter always tea (true) \n"
  " [ true* . ok_tea . quarter . quarter . !tea ] false      \n"
  ;

const std::string MACHINE_FORMULA2 =
  "%% always eventually action ready (true)     \n"
  " [ true* . dollar . !(tea||coffee) ] false   \n"
  ;

const std::string MACHINE_FORMULA3 =
  "%% after a quarter no change directly (true) \n"
  " [ true* . quarter . change ] false          \n"
  ;

void test_lps2pbes(std::string lps_spec, std::string mcf_formula)
{
  using namespace pbes_system;

  lps::specification spec = lps::linearise(lps_spec);
  state_formulas::state_formula formula = state_formulas::parse_state_formula(mcf_formula, spec);
  bool timed = false;
  pbes<> p = lps2pbes(spec, formula, timed);
  core::garbage_collect();
}

// Submitted by Tim, 2-9-2010
void test_example()
{
  std::string SPEC =
    "act a,b;                                         \n"
    "                                                 \n"
    "proc S = sum n:Nat. (n < 3) -> a.X(n);           \n"
    "                                                 \n"
    "proc X(n:Nat) = (n == 0) -> ( (a+b).X(n))        \n"
    "               +(n > 0 ) -> b.a.X(Int2Nat(n-1)); \n"
    "                                                 \n"
    "init S;                                          \n"
    ;

  std::string FORMULA = "<a>([a]false)";   

  pbes<> p;
  bool timed = false;
  p = lps2pbes(SPEC, FORMULA, timed);
  BOOST_CHECK(p.is_well_typed());
  std::cerr << "p = " << core::pp(pbes_to_aterm(p)) << std::endl;

  bool result = pbes2_bool_test(p);
  BOOST_CHECK(result == true);

  core::garbage_collect();
}

// Submitted by Jeroen Keiren, 10-09-2010
// Formula 2 and 3 give normalization errors.
void test_elevator()
{
  std::string SPEC =
    "% Model of an elevator for n floors.                                                                                           \n"
    "% Originally described in 'Solving Parity Games in Practice' by Oliver                                                         \n"
    "% Friedmann and Martin Lange.                                                                                                  \n"
    "%                                                                                                                              \n"
    "% This is the version with a first in first out policy                                                                         \n"
    "                                                                                                                               \n"
    "sort Floor = Pos;                                                                                                              \n"
    "     DoorStatus = struct open | closed;                                                                                        \n"
    "     Requests = List(Floor);                                                                                                   \n"
    "                                                                                                                               \n"
    "map maxFloor: Floor;                                                                                                           \n"
    "eqn maxFloor = 3;                                                                                                              \n"
    "                                                                                                                               \n"
    "map addRequest : Requests # Floor -> Requests;                                                                                 \n"
    "                                                                                                                               \n"
    "var r: Requests;                                                                                                               \n"
    "    f,g: Floor;                                                                                                                \n"
    "    % FIFO behaviour!                                                                                                          \n"
    "eqn addRequest([], f) = [f];                                                                                                   \n"
    "    (f == g) -> addRequest(g |> r, f) = g |> r;                                                                                \n"
    "    (f != g) -> addRequest(g |> r, f) = g |> addRequest(r, f);                                                                 \n"
    "                                                                                                                               \n"
    "map removeRequest : Requests -> Requests;                                                                                      \n"
    "var r: Requests;                                                                                                               \n"
    "    f: Floor;                                                                                                                  \n"
    "eqn removeRequest(f |> r) = r;                                                                                                 \n"
    "                                                                                                                               \n"
    "map getNext : Requests -> Floor;                                                                                               \n"
    "var r: Requests;                                                                                                               \n"
    "    f: Floor;                                                                                                                  \n"
    "eqn getNext(f |> r) = f;                                                                                                       \n"
    "                                                                                                                               \n"
    "act isAt: Floor;                                                                                                               \n"
    "    request: Floor;                                                                                                            \n"
    "    close, open, up, down;                                                                                                     \n"
    "                                                                                                                               \n"
    "proc Elevator(at: Floor, status: DoorStatus, reqs: Requests, moving: Bool) =                                                   \n"
    "       isAt(at) . Elevator()                                                                                                   \n"
    "     + sum f: Floor. (f <= maxFloor) -> request(f) . Elevator(reqs = addRequest(reqs, f))                                      \n"
    "     + (status == open) -> close . Elevator(status = closed)                                                                   \n"
    "     + (status == closed && reqs != [] && getNext(reqs) > at) -> up . Elevator(at = at + 1, moving = true)                     \n"
    "     + (status == closed && reqs != [] && getNext(reqs) < at) -> down . Elevator(at = Int2Pos(at - 1), moving = true)          \n"
    "     + (status == closed && getNext(reqs) == at) -> open. Elevator(status = open, reqs = removeRequest(reqs), moving = false); \n"
    "                                                                                                                               \n"
    "init Elevator(1, open, [], false);                                                                                             \n"
    ;

  std::string formula1 = "nu U. [true] U && ((mu V . nu W. !([!request(maxFloor)]!W && [request(maxFloor)]!V)) || (nu X . mu Y. [!isAt(maxFloor)] Y &&  [isAt(maxFloor)]X))";
  std::string formula2 = "nu U. [true] U && ((nu V . mu W. ([!request(maxFloor)]W && [request(maxFloor)]V)) => (nu X . mu Y. [!isAt(maxFloor)] Y &&  [isAt(maxFloor)]X))";
  std::string formula3 = "nu U. [true] U && (!(nu V . mu W. ([!request(maxFloor)]W && [request(maxFloor)]V)) || (nu X . mu Y. [!isAt(maxFloor)] Y &&  [isAt(maxFloor)]X))";
  std::string formula4 = "(nu V . mu W. V) => true";
  std::string formula5 = "!(nu V . mu W. V)";

  pbes<> p;
  bool timed = false; 
  p = lps2pbes(SPEC, formula1, timed);
  p = lps2pbes(SPEC, formula2, timed);
  p = lps2pbes(SPEC, formula3, timed);
  p = lps2pbes(SPEC, formula4, timed);

  core::garbage_collect();
}

int test_main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT_DEBUG(argc, argv)

  test_elevator();
  test_example();
  test_lps2pbes();
  test_lps2pbes2();
  test_lps2pbes3();
  test_trivial();
  test_formulas();
  test_equal_multi_actions();

#ifdef MCRL2_EXTENDED_TESTS
  test_lps2pbes(MACHINE_SPECIFICATION, MACHINE_FORMULA1);
  test_lps2pbes(MACHINE_SPECIFICATION, MACHINE_FORMULA2);
  test_lps2pbes(MACHINE_SPECIFICATION, MACHINE_FORMULA3);
#endif

  return 0;
}
