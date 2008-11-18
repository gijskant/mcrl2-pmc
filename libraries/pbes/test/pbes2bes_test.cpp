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

//#define MCRL2_ENUMERATE_QUANTIFIERS_BUILDER_DEBUG
#define MCRL2_ENUMERATE_QUANTIFIERS_BUILDER_DEBUG2

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
using pbes_system::enumerate_quantifiers_rewriter;
using pbes_system::pbes;
using pbes_system::pbes2bes_algorithm;
using pbes_system::txt2pbes;

inline
pbes<> pbes2bes(const pbes<>& p, bool finite = true)
{
  data::rewriter datar(p.data());
  data::rewriter_with_variables datarv(p.data());
  data::number_postfix_generator generator("UNIQUE_PREFIX");
  data::data_enumerator<> datae(p.data(), datar, generator);
  enumerate_quantifiers_rewriter<pbes_system::pbes_expression, data::rewriter_with_variables, data::data_enumerator<> > pbesr(datarv, datae);
  pbes<> result = (finite ? do_finite_algorithm(p, pbesr) : do_lazy_algorithm(p, pbesr));
  return result;
}

inline
pbes<> pbes2bes_new(const pbes<>& p)
{
  pbes2bes_algorithm algorithm(p.data());
  algorithm.run(p);
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

void test_pbes(const std::string& pbes_spec, bool test_finite, bool test_lazy)
{
  core::gsSetNormalMsg();
  pbes<> p = txt2pbes(pbes_spec);
  std::cout << "------------------------------\n" << core::pp(p) << std::endl;
  if (!p.is_closed())
  {
    std::cout << "ERROR: the pbes is not closed!" << std::endl;
    return;
  }

  if (test_finite)
  {
    std::cout << "FINITE" << std::endl;
    try
    {
      pbes<> q1 = pbes2bes(p, true);
      core::gsSetVerboseMsg();
      std::cout << core::pp(q1) << std::endl;
    }
    catch (mcrl2::runtime_error e)
    {
      std::cout << "pbes2bes failed: " << e.what() << std::endl;
    }
  }

  if (test_lazy)
  {
    std::cout << "LAZY" << std::endl;
    try
    {
      pbes<> q1 = pbes2bes(p, false);
      core::gsSetVerboseMsg();
      std::cout << core::pp(q1) << std::endl;
    }
    catch (mcrl2::runtime_error e)
    {
      std::cout << "pbes2bes failed: " << e.what() << std::endl;
    }

    std::cout << "NEWLAZY" << std::endl;
    try
    {
      using namespace pbes_system;
      pbes<> q1 = pbes2bes_new(p);
      std::cout << core::pp(q1) << std::endl;
    }
    catch (mcrl2::runtime_error e)
    {
      std::cout << "pbes2bes failed: " << e.what() << std::endl;
    }
  }
}

void test_pbes2bes()
{ 
  test_pbes(test1, true, false);
  test_pbes(test2, true, true);
  test_pbes(test3, true, false);
  test_pbes(test4, true, true);
  test_pbes(test5, true, true);
  test_pbes(test6, true, true);
  test_pbes(test7, true, true);
  test_pbes(test8, true, true);
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT(argc, argv)
  test_pbes2bes();

  return 0;
}
