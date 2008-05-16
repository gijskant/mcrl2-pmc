// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file sort_test.cpp
/// \brief Add your file description here.

#include <iostream>
#include <string>
#include <set>
#include <boost/test/minimal.hpp>
#include "mcrl2/atermpp/atermpp.h"
#include "mcrl2/atermpp/make_list.h"
#include "mcrl2/data/data_operation.h"
#include "mcrl2/data/parser.h"
#include "mcrl2/data/sort_arrow.h"
#include "mcrl2/data/detail/data_functional.h"

using namespace atermpp;
using namespace mcrl2::data;

const std::string DATA_SPEC1 =
  "sort A;                     \n"
  "sort B;                     \n"
  "sort C;                     \n"
  "sort D;                     \n"
  "sort S1 = A # B -> C;       \n"
  "sort S2 = (A -> B) -> C;    \n"
  "sort S3 = A -> (B -> C);    \n"
  "sort S4 = A # (A -> B) -> C;\n"
  "sort S5 = A # A -> B -> C;  \n"
  "                            \n"
  "cons s1: S1;                \n"
  "cons s2: S2;                \n"
  "cons s3: S3;                \n"
  "cons s4: S4;                \n"
  "cons s5: S5;                \n" 
;  

void test_sort()
{
  sort_expression A("A");
  sort_expression_list lA = make_list(A);
  sort_expression B("B");
  sort_expression_list lB = make_list(B);
  sort_expression C("C");
  sort_expression_list lC = make_list(C);
  sort_expression D("D");
  sort_expression AB = arrow(lA, B);
  sort_expression_list lAB = make_list(A,B);
  sort_expression BC = arrow(lB, C);
  sort_expression_list lBC = make_list(B,C);
  sort_expression CD = arrow(lC, D);

  sort_expression s;
  sort_expression_list domain;
  sort_expression range;
  sort_expression_list src;
  sort_expression tgt;
  
  s      = arrow(lA, BC);   // A->(B->C)
  domain = domain_sorts(s); // [A,B]
  range  = result_sort(s);   // C
  src    = source(s);       // [A]
  tgt    = target(s);      // B->C
  BOOST_CHECK(domain.size() == 2);
  BOOST_CHECK(std::find(domain.begin(), domain.end(), A) != domain.end());
  BOOST_CHECK(std::find(domain.begin(), domain.end(), B) != domain.end());
  BOOST_CHECK(range == C);
  BOOST_CHECK(src.size() == 1);
  BOOST_CHECK(std::find(src.begin(), src.end(), A) != src.end());
  BOOST_CHECK(tgt == BC);

  s      = arrow(lAB, C);   // (AxB)->C
  domain = domain_sorts(s); // [A,B]
  range  = result_sort(s);   // C
  src    = source(s);       // [A,B]
  tgt    = target(s);      // C
  BOOST_CHECK(domain.size() == 2);
  BOOST_CHECK(std::find(domain.begin(), domain.end(), A) != domain.end());
  BOOST_CHECK(std::find(domain.begin(), domain.end(), B) != domain.end());
  BOOST_CHECK(range == C);
  BOOST_CHECK(src.size() == 2);
  BOOST_CHECK(std::find(src.begin(), src.end(), A) != src.end());
  BOOST_CHECK(std::find(src.begin(), src.end(), B) != src.end());
  BOOST_CHECK(tgt == C);

  s      = arrow(lAB, CD);  // (AxB)->(C->D)
  domain = domain_sorts(s); // [A,B,C]
  range  = result_sort(s);   // D
  src    = source(s);       // [A,B]
  tgt    = target(s);      // C->D
  BOOST_CHECK(domain.size() == 3);
  BOOST_CHECK(std::find(domain.begin(), domain.end(), A) != domain.end());
  BOOST_CHECK(std::find(domain.begin(), domain.end(), B) != domain.end());
  BOOST_CHECK(std::find(domain.begin(), domain.end(), C) != domain.end());
  BOOST_CHECK(range == D);
  BOOST_CHECK(src.size() == 2);
  BOOST_CHECK(std::find(src.begin(), src.end(), A) != src.end());
  BOOST_CHECK(std::find(src.begin(), src.end(), B) != src.end());
  BOOST_CHECK(tgt == CD);

  // check if (A x B) -> C != A -> (B x C)
  sort_expression s1 = sort_arrow(lAB, C);
  sort_expression s2 = sort_arrow(lA, BC);
  BOOST_CHECK(s1 != s2);
}

void test_sort_equality()
{
  using mcrl2::data::detail::find_constructor;

  data_specification data = parse_data_specification(DATA_SPEC1);

  data_operation s1 = find_constructor(data, "s1");
  data_operation s2 = find_constructor(data, "s2");
  data_operation s3 = find_constructor(data, "s3");
  data_operation s4 = find_constructor(data, "s4");
  data_operation s5 = find_constructor(data, "s5");
  
  BOOST_CHECK(s1.sort() != sort_expression());
  BOOST_CHECK(s2.sort() != sort_expression());
  BOOST_CHECK(s3.sort() != sort_expression());
  BOOST_CHECK(s4.sort() != sort_expression());
  BOOST_CHECK(s5.sort() != sort_expression());

  BOOST_CHECK(s1.sort() != s2.sort());
  BOOST_CHECK(s2.sort() != s3.sort());
  BOOST_CHECK(s4.sort() != s5.sort()); 
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT(argc, argv) 

  test_sort();
  test_sort_equality();

  return 0;
}
