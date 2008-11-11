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

#include <string>
#include <iostream>
#include <boost/test/minimal.hpp>
#include "mcrl2/core/messaging.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/data/enumerator.h"
#include "mcrl2/pbes/pbes2bes.h"
#include "mcrl2/pbes/txt2pbes.h"
#include "mcrl2/pbes/rewriter.h"
#include "mcrl2/pbes/pbes2bes_algorithm.h"

using namespace mcrl2;

inline
pbes<> pbes2bes(const pbes<>& pbes_spec, bool finite = true)
{
  data::rewriter datar(pbes_spec.data());
  data::number_postfix_generator generator("UNIQUE_PREFIX");
  data::data_enumerator<data::rewriter, data::number_postfix_generator> datae(pbes_spec.data(), datar, generator);
  pbes_system::enumerate_quantifiers_rewriter<pbes_system::pbes_expression, data::rewriter, data::data_enumerator<> > pbesr(datar, datae);
  if (!pbes_spec.is_closed())
  {
    core::gsErrorMsg("The PBES is not closed. Pbes2bes cannot handle this kind of PBES's\nComputation aborted.\n");
  }
  pbes<> result = (finite ? do_finite_algorithm(pbes_spec, pbesr) : do_lazy_algorithm(pbes_spec, pbesr));
  return result;
}

inline
pbes<> pbes2bes_new(const pbes<>& pbes_spec)
{
  pbes2bes_algorithm algorithm(pbes_spec.data());
  algorithm.run(pbes_spec);
  return algorithm.get_result();
}

std::string test1 =
  "pbes                                                                              \n"
  "                                                                                  \n"
  "nu X(b:Bool, n:Nat) = (val(b) => X(!b, n)) && (val(!b) => X(!b, n+1));            \n"
  "                                                                                  \n"
  "init X(true,0);                                                                   \n"
  ;
  
std::string test2 =
  "pbes                                                                              \n"
  "                                                                                  \n"
  "nu X(b:Bool, n:Nat) = forall c:Bool. X(c,n);                                      \n"
  "                                                                                  \n"
  "init X(true,0);                                                                   \n"
  ;

std::string test3 =
  "pbes                                                                              \n"
  "                                                                                  \n"
  "nu X(b:Bool, n:Nat) = exists c:Bool. X(c,n+1);                                    \n"
  "                                                                                  \n"
  "init X(true,0);                                                                   \n"
  ;

std::string test4 =
  "pbes                                                                              \n"
  "                                                                                  \n"
  "nu X(b:Bool, n:Nat) = val(b && n < 10) => X(!b,n+1);                              \n"
  "                                                                                  \n"
  "init X(true,0);                                                                   \n"
  ;

std::string test5 =
  "sort D = struct d1 | d2;                                                          \n"
  "                                                                                  \n"
  "pbes                                                                              \n"
  "                                                                                  \n"
  "nu X(d:D, n:Nat) = val(d == d1 && n < 10) => X(d2,n+1);                           \n"
  "                                                                                  \n"
  "init X(d1,0);                                                                     \n"
  ;

std::string test6 =
  "pbes                                                                              \n"
  "nu X(b:Bool) = forall c:Bool. X(if (c,!c,c));                                     \n"
  "                                                                                  \n"
  "init X(true);                                                                     \n"
  ;

std::string test7 =
  "sort Enum2 = struct e1_5 | e0_5;                                                  \n"
  "                                                                                  \n"
  "map                                                                               \n"
  "                                                                                  \n"
  "     C5_fun2: Enum2 # Enum2 # Enum2  -> Enum2;                                    \n"
  "     C5_fun1: Enum2 # Nat # Nat  -> Nat;                                          \n"
  "                                                                                  \n"
  "var  y23,y22,y21,x5,y14,y13,y12,y11,y10,x2,e3,e2,e1: Enum2;                       \n"
  "     y20,y19,y18,x4,y9,y8,y7,y6,y5,x1: Nat;                                       \n"
  "     y17,y16,y15,x3,y4,y3,y2,y1,y,x: Bool;                                        \n"
  "eqn                                                                               \n"
  "     C5_fun2(e0_5, y14, y13)  =  y14;                                             \n"
  "     C5_fun2(e1_5, y14, y13)  =  y13;                                             \n"
  "     C5_fun2(e3, x2, x2)  =  x2;                                                  \n"
  "     C5_fun1(e0_5,  y6, y5)  =  y5;                                               \n"
  "     C5_fun1(e1_5,  y6, y5)  =  y6;                                               \n"
  "     C5_fun1(e2,  x1, x1)  =  x1;                                                 \n"
  "                                                                                  \n"
  "pbes nu X(s3_P: Enum2,  n_P: Nat) =                                               \n"
  "                                                                                  \n"
  "(forall e: Enum2.  X(C5_fun2(e, e, e1_5), C5_fun1(e, 0, n_P))                     \n"
  "                                                                                  \n"
  ")                                                                                 \n"
  "                                                                                  \n"
  "                                                                                  \n"
  ";                                                                                 \n"
  "                                                                                  \n"
  "init X(e1_5,  0);                                                                 \n"
  ;

std::string test8 =
  "pbes                                                                              \n"
  "                                                                                  \n"
  "nu X(b:Bool) = val(b) && Y(!b);                                                   \n"
  "                                                                                  \n"
  "mu Y(c:Bool) = forall d:Bool. X(d && c) || Y(d);                                  \n"
  "                                                                                  \n"
  "init X(true);                                                                     \n"
  ;

void test_pbes(const std::string& pbes_spec, bool finite)
{
  using namespace pbes_system;

  core::gsSetNormalMsg();
  pbes<> p = txt2pbes(pbes_spec);
  std::cout << "------------------------------\n" << core::pp(p) << std::endl;

  pbes<> q1 = pbes2bes(p, finite);
  core::gsSetVerboseMsg();
  std::cout << "<old version> " << (finite ? "finite\n" : "infinite\n") << core::pp(q1) << std::endl;
}

void test_pbes_new(const std::string& pbes_spec)
{
  using namespace pbes_system;

  pbes<> p = txt2pbes(pbes_spec);
  pbes<> q2 = pbes2bes_new(p);
  std::cout << "<new version>\n" << core::pp(q2) << std::endl;
}

void test_pbes2bes()
{
  test_pbes(test1, true);
  test_pbes(test2, true);
  test_pbes(test3, true);
  test_pbes(test4, true);
  test_pbes(test5, true);
  test_pbes(test6, true);
  test_pbes(test7, true);
  test_pbes(test8, true);

  test_pbes(test2, false);
  test_pbes_new(test2);
  test_pbes(test4, false);
  test_pbes_new(test4);
  test_pbes(test5, false);
  test_pbes_new(test5);
  test_pbes(test6, false);
  test_pbes_new(test6);
  test_pbes(test7, false);
  test_pbes_new(test7);
  test_pbes(test8, false);
  test_pbes_new(test8);
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT(argc, argv)

  test_pbes2bes();
  //BOOST_CHECK(false);

  return 0;
}
