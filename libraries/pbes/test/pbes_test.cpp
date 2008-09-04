// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbes_test.cpp
/// \brief Add your file description here.

#include <iostream>
#include <iterator>
#include <utility>
#include <boost/test/minimal.hpp>
#include <boost/algorithm/string.hpp>
#include "mcrl2/atermpp/make_list.h"
#include "mcrl2/data/utility.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/pbes_translate.h"
#include "mcrl2/pbes/lps2pbes.h"
#include "mcrl2/pbes/pbes_expression_builder.h"
#include "mcrl2/pbes/detail/quantifier_rename_builder.h"
#include "mcrl2/pbes/rename.h"
#include "mcrl2/pbes/complement.h"

using namespace std;
using namespace atermpp;
using namespace mcrl2;
using namespace mcrl2::core;
using namespace mcrl2::data;
using namespace mcrl2::lps;
using namespace mcrl2::lps::detail;
using namespace mcrl2::modal;
using namespace mcrl2::modal::detail;
using namespace mcrl2::pbes_system;
using namespace mcrl2::pbes_system::detail;

const std::string SPECIFICATION =
"act a:Nat;                               \n"
"                                         \n"
"map smaller: Nat#Nat -> Bool;            \n"
"                                         \n"
"var x,y : Nat;                           \n"
"                                         \n"
"eqn smaller(x,y) = x < y;                \n"
"                                         \n"
"proc P(n:Nat) = sum m: Nat. a(m). P(m);  \n"
"                                         \n"
"init P(0);                               \n";

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

const std::string MPSU_SPECIFICATION =
"% This file describes a controller for a simplified Movable Patient                   \n"
"% Support Unit. It is described in Fokkink, Groote and Reniers,                       \n"
"% Modelling reactive systems.                                                         \n"
"%                                                                                     \n"
"% Jan Friso Groote, September, 2006.                                                  \n"
"                                                                                      \n"
"sort Mode = struct Normal | Emergency ;                                               \n"
"     MotorStatus = struct turnleft | turnright | stopped ;                            \n"
"                                                                                      \n"
"act pressStop, pressResume,                                                           \n"
"    pressUndock, pressLeft,                                                           \n"
"    pressRight, motorLeft,                                                            \n"
"    motorRight, motorOff,                                                             \n"
"    applyBrake, releaseBrake,                                                         \n"
"    isDocked, unlockDock,                                                             \n"
"    atInnermost, atOutermost;                                                         \n"
"                                                                                      \n"
"proc Controller(m:Mode,docked,rightmost,leftmost:Bool,ms:MotorStatus)=                \n"
"       pressStop.Controller(Emergency,docked,rightmost,leftmost,ms)+                  \n"
"       pressResume.Controller(Normal,docked,rightmost,leftmost,ms)+                   \n"
"       pressUndock.                                                                   \n"
"         (docked && rightmost)                                                        \n"
"                -> applyBrake.unlockDock.Controller(m,false,rightmost,leftmost,ms)    \n"
"                <> Controller(m,docked,rightmost,leftmost,ms)+                        \n"
"       pressLeft.                                                                     \n"
"          (docked && ms!=turnleft && !leftmost && m==Normal)                          \n"
"                -> releaseBrake.motorLeft.                                            \n"
"                     Controller(m,docked,false,leftmost,turnleft)                     \n"
"                <> Controller(m,docked,rightmost,leftmost,ms)+                        \n"
"       pressRight.                                                                    \n"
"          (docked && ms!=turnright && !rightmost && m==Normal)                        \n"
"                -> releaseBrake.motorRight.                                           \n"
"                     Controller(m,docked,rightmost,false,turnright)                   \n"
"                <> Controller(m,docked,rightmost,leftmost,ms)+                        \n"
"       isDocked.Controller(m,true,rightmost,leftmost,ms)+                             \n"
"       atInnermost.motorOff.applyBrake.Controller(m,docked,true,false,stopped)+       \n"
"       atOutermost.motorOff.applyBrake.Controller(m,docked,false,true,stopped);       \n"
"                                                                                      \n"
"                                                                                      \n"
"                                                                                      \n"
"init Controller(Normal,true,false,false,stopped);                                     \n"
;

const std::string FORMULA  = "nu X(n:Nat = 1). [forall m:Nat. a(m)](val(n < 10)  && X(n+2))";
const std::string FORMULA2 = "forall m:Nat. [a(m)]false";

const std::string MPSU_FORMULA =
"% This file describes the modal formulas for property 5 used in section \n"
"% 5.3 of Designing and understanding the behaviour of systems           \n"
"% by J.F. Groote and M.A. Reniers.                                      \n"
"                                                                        \n"
"nu X(b1:Bool=false, b2:Bool=true,b3:Bool=true,b4:Bool=true).            \n"
"        val(b1 && b2 && b3 && b4) => ([pressLeft]                       \n"
"              (mu Y.[!motorLeft &&                                      \n"
"                    !unlockDock &&                                      \n"
"                    !pressStop &&                                       \n"
"                    !atInnermost]Y))                                    \n";

void test_pbes()
{
  specification spec    = mcrl22lps(SPECIFICATION);
  state_formula formula = mcf2statefrm(FORMULA2, spec);
  bool timed = false;
  pbes<> p = lps2pbes(spec, formula, timed);
  pbes_expression e = p.equations().front().formula();

  BOOST_CHECK(!p.is_bes());
  BOOST_CHECK(!e.is_bes());

  try
  {
    p.load("non-existing file");
    BOOST_CHECK(false); // loading is expected to fail
  }
  catch (mcrl2::runtime_error e)
  {
  }

  try
  {
    aterm t = atermpp::make_term("f(x)");
    std::string filename = "write_to_named_text_file.pbes";
    atermpp::write_to_named_text_file(t, filename);
    p.load(filename);
    BOOST_CHECK(false); // loading is expected to fail
  }
  catch (mcrl2::runtime_error e)
  {
  }
  p.save("pbes_test_file.pbes");
  p.load("pbes_test_file.pbes");
}

// void test_xyz_generator()
// {
//   XYZ_identifier_generator generator(propositional_variable("X1(d:D)"));
//   identifier_string x;
//   x = generator(); BOOST_CHECK(std::string(x) == "X");
//   x = generator(); BOOST_CHECK(std::string(x) == "Y");
//   x = generator(); BOOST_CHECK(std::string(x) == "Z");
//   x = generator(); BOOST_CHECK(std::string(x) == "X0");
//   x = generator(); BOOST_CHECK(std::string(x) == "Y0");
//   x = generator(); BOOST_CHECK(std::string(x) == "Z0");
//   x = generator(); BOOST_CHECK(std::string(x) == "Y1"); // X1 should be skipped
// }

void test_free_variables()
{
  pbes<> p;
  try {
    p.load("abp_fv.pbes");
    atermpp::set< data_variable > freevars = p.free_variables();
    cout << freevars.size() << endl;
    BOOST_CHECK(freevars.size() == 20);
    for (atermpp::set< data_variable >::iterator i = freevars.begin(); i != freevars.end(); ++i)
    {
      cout << "<var>" << mcrl2::core::pp(*i) << endl;
    }
    freevars = p.free_variables();
    BOOST_CHECK(freevars.size() == 15);
  }
  catch (mcrl2::runtime_error e)
  {
    cout << e.what() << endl;
    BOOST_CHECK(false); // loading is expected to succeed
  }
}

void test_pbes_expression_builder()
{
  specification mpsu_spec = mcrl22lps(MPSU_SPECIFICATION);
  state_formula mpsu_formula = mcf2statefrm(MPSU_FORMULA, mpsu_spec);
  bool timed = false;
  pbes<> p = lps2pbes(mpsu_spec, mpsu_formula, timed);

  for (atermpp::vector<pbes_equation>::iterator i = p.equations().begin(); i != p.equations().end(); ++i)
  {
    const pbes_expression& q = i->formula();
    pbes_expression_builder<pbes_expression> builder;
    pbes_expression q1 = builder.visit(q);
    BOOST_CHECK(q == q1);
  }
}

void test_quantifier_rename_builder()
{
  using namespace pbes_expr;
  namespace d = data_expr; 

  data_variable mN("m:N");
  data_variable nN("n:N");

  pbes_expression f = d::equal_to(mN, nN);
  pbes_expression g = d::not_equal_to(mN, nN);

  multiset_identifier_generator generator(make_list(identifier_string("n00"), identifier_string("n01")));

  pbes_expression p1 = 
  and_(
    forall(make_list(nN), exists(make_list(nN), f)),
    forall(make_list(mN), exists(make_list(mN, nN), g))
  );
  pbes_expression q1 = make_quantifier_rename_builder(generator).visit(p1); 
  std::cout << "p1 = " << mcrl2::core::pp(p1) << std::endl;
  std::cout << "q1 = " << mcrl2::core::pp(q1) << std::endl;

  pbes_expression p2 = 
  and_(
    forall(make_list(nN), exists(make_list(nN), p1)),
    forall(make_list(mN), exists(make_list(mN, nN), q1))
  );
  pbes_expression q2 = rename_quantifier_variables(p2, make_list(data_variable("n00:N"), data_variable("n01:N")));
  std::cout << "p2 = " << mcrl2::core::pp(p2) << std::endl;
  std::cout << "q2 = " << mcrl2::core::pp(q2) << std::endl;

  // BOOST_CHECK(false);
}

void test_complement_method_builder()
{
  using namespace pbes_expr;
  namespace d = data_expr;

  data_variable X("x:X");
  data_variable Y("y:Y");

  pbes_expression p = or_(and_(X,Y), and_(Y,X));
  pbes_expression q = and_(or_(d::not_(X), d::not_(Y)), or_(d::not_(Y),d::not_(X)));
  std::cout << "p             = " << mcrl2::core::pp(p) << std::endl;
  std::cout << "q             = " << mcrl2::core::pp(q) << std::endl;
  std::cout << "complement(p) = " << mcrl2::core::pp(complement(p)) << std::endl;
  BOOST_CHECK(complement(p) == q);
}

void test_pbes_expression()
{
  namespace p = pbes_expr;
  namespace d = data_expr;

  data_variable x1("x1:X");
  pbes_expression e = x1;
  data_expression x2 = mcrl2::pbes_system::accessors::val(e);
  BOOST_CHECK(x1 == x2);
  
  pbes_expression v_expr = propositional_variable_instantiation("v:V");
  propositional_variable_instantiation v1 = v_expr;
  propositional_variable_instantiation v2(v_expr);
}

void test_trivial()
{
  specification spec    = mcrl22lps(ABP_SPECIFICATION);
  state_formula formula = mcf2statefrm(TRIVIAL_FORMULA, spec);
  bool timed = false;
  pbes<> p = lps2pbes(spec, formula, timed);
  BOOST_CHECK(p.is_well_typed());
}

void test_instantiate_free_variables()
{
  std::string spec_text = 
    "act a,b:Nat;             \n"
    "    d;                   \n"
    "proc P(n:Nat)=a(n).delta;\n"
    "init d.P(1);             \n"
  ;
  std::string formula_text = "([true*.a(1)]  (mu X.([!a(1)]X && <true> true)))";
  specification spec    = mcrl22lps(spec_text);
  state_formula formula = mcf2statefrm(formula_text, spec);
  bool timed = false;
  pbes<> p = lps2pbes(spec, formula, timed);
  std::cout << "<before>" << mcrl2::core::pp(p) << std::endl;  
  std::cout << "<lps>" << mcrl2::core::pp(spec) << std::endl;  
  bool result = p.instantiate_free_variables();
  std::cout << "<result>" << result << std::endl;  
  std::cout << "<after>" << mcrl2::core::pp(p) << std::endl;  
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT(argc, argv)

  test_trivial();
  test_pbes();
  // test_xyz_generator();
  // test_free_variables();
  test_pbes_expression_builder();
  test_quantifier_rename_builder();
  test_complement_method_builder();
  test_pbes_expression();
  test_instantiate_free_variables();

  return 0;
}
