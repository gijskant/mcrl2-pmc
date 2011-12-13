// Author(s): Jeroen van der Wulp
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file rewriter_test.cpp
/// \brief Add your file description here.

#include <iostream>
#include <string>
#include <set>
#include <boost/test/included/unit_test_framework.hpp>
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/atermpp/map.h"
#include "mcrl2/utilities/text_utility.h"
#include "mcrl2/data/nat.h"
#include "mcrl2/data/int.h"
#include "mcrl2/data/real.h"
#include "mcrl2/data/list.h"
#include "mcrl2/data/set.h"
#include "mcrl2/data/bag.h"
#include "mcrl2/data/structured_sort.h"
#include "mcrl2/data/find.h"
#include "mcrl2/data/parse.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/data/function_sort.h"
#include "mcrl2/data/utility.h"
#include "mcrl2/data/standard_utility.h"
#include "mcrl2/data/detail/data_functional.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/utilities/test_utilities.h"

using mcrl2::utilities::collect_after_test_case;
using namespace mcrl2;
using namespace mcrl2::core;
using namespace mcrl2::data;
using namespace mcrl2::data::detail;

BOOST_GLOBAL_FIXTURE(collect_after_test_case)

typedef mcrl2::data::basic_rewriter< mcrl2::data::data_expression >::strategy rewrite_strategy;
typedef std::vector<rewrite_strategy > rewrite_strategy_vector;

template <typename Rewriter>
void data_rewrite_test(Rewriter& R, data_expression const& input, data_expression const& expected_output)
{
  data_expression output = R(input);

  BOOST_CHECK(output == expected_output);

  if (output != expected_output)
  {
    std::cerr << "--- test failed --- " << data::pp(input) << " ->* " << data::pp(expected_output) << std::endl
              << "input    " << data::pp(input) << std::endl
              << "expected " << data::pp(expected_output) << std::endl
              << "output " << data::pp(output) << std::endl
              << " -- term representations -- " << std::endl
              << "input    " << input << std::endl
              << "expected " << expected_output << std::endl
              << "R(input) " << output << std::endl;
  }
}

BOOST_AUTO_TEST_CASE(bool_rewrite_test)
{
  using namespace mcrl2::data::sort_bool;

  data_specification specification;

  specification.add_context_sort(bool_());

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy1: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data_rewrite_test(R, true_(), true_());
    data_rewrite_test(R, false_(), false_());

    data_rewrite_test(R, and_(true_(), false_()), false_());
    data_rewrite_test(R, and_(false_(), true_()), false_());

    data_rewrite_test(R, or_(true_(), false_()), true_());
    data_rewrite_test(R, or_(false_(), true_()), true_());

    data_rewrite_test(R, implies(true_(), false_()), false_());
    data_rewrite_test(R, implies(false_(), true_()), true_());

    data::variable_vector v;
    v.push_back(data::variable("b", bool_()));
    v.push_back(data::variable("c", bool_()));
    data::data_expression e(parse_data_expression("b&&(b&&c)", v.begin(), v.end()));
    data_rewrite_test(R, e, e);
  }

}

BOOST_AUTO_TEST_CASE(pos_rewrite_test)
{
  using namespace mcrl2::data::sort_pos;
  std::cerr << "pos_rewrite_test\n";

  data_specification specification;

  specification.add_context_sort(pos());

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy2: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data_expression p1(pos("1"));
    data_expression p2(pos("2"));
    data_expression p3(pos("3"));
    data_expression p4(pos("4"));

    data_rewrite_test(R, sort_pos::plus(p1, p2), p3);
    data_rewrite_test(R, sort_pos::plus(p2, p1), p3);

    data_rewrite_test(R, sort_pos::times(p1, p1), p1);
    data_rewrite_test(R, sort_pos::times(p1, p2), p2);

    data_rewrite_test(R, (sort_pos::minimum)(p1, p1), p1);
    data_rewrite_test(R, (sort_pos::minimum)(p1, p2), p1);

    data_rewrite_test(R, (sort_pos::maximum)(p1, p1), p1);
    data_rewrite_test(R, (sort_pos::maximum)(p1, p2), p2);

    data_rewrite_test(R, sort_pos::succ(p1), p2);
  }
}

BOOST_AUTO_TEST_CASE(nat_rewrite_test)
{
  using namespace mcrl2::data::sort_nat;
  std::cerr << "nat_rewrite_test\n";

  data_specification specification;

  specification.add_context_sort(nat());

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy3: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data_expression p0(nat(0));
    data_expression p1(nat(1));
    data_expression p2(nat(2));
    data_expression p3(nat(3));
    data_expression p4(nat(4));

    data_rewrite_test(R, plus(p0, p2), p2);
    data_rewrite_test(R, plus(p2, p0), p2);
    data_rewrite_test(R, plus(p1, p2), p3);
    data_rewrite_test(R, plus(p2, p1), p3);

    data_rewrite_test(R, times(p1, p1), p1);
    data_rewrite_test(R, times(p0, p2), p0);
    data_rewrite_test(R, times(p2, p0), p0);
    data_rewrite_test(R, times(p1, p2), p2);

    data_rewrite_test(R, (minimum)(p1, p1), p1);
    data_rewrite_test(R, (minimum)(p0, p2), p0);
    data_rewrite_test(R, (minimum)(p2, p0), p0);
    data_rewrite_test(R, (minimum)(p1, p2), p1);

    data_rewrite_test(R, (maximum)(p1, p1), p1);
    data_rewrite_test(R, (maximum)(p0, p2), p2);
    data_rewrite_test(R, (maximum)(p2, p0), p2);
    data_rewrite_test(R, (maximum)(p1, p2), p2);

    data_rewrite_test(R, succ(p0), R(nat2pos(p1)));
    data_rewrite_test(R, succ(p1), R(nat2pos(p2)));

    data_rewrite_test(R, pred(nat2pos(p1)), p0);
    data_rewrite_test(R, pred(nat2pos(p2)), p1);

    data_rewrite_test(R, div(p0, sort_pos::pos(2)), p0);
    data_rewrite_test(R, div(p2, sort_pos::pos(1)), p2);
    data_rewrite_test(R, div(p4, sort_pos::pos(2)), p2);

    data_rewrite_test(R, mod(p1, nat2pos(p1)), p0);
    data_rewrite_test(R, mod(p0, nat2pos(p2)), p0);
    data_rewrite_test(R, mod(p2, nat2pos(p1)), p0);
    data_rewrite_test(R, mod(p4, nat2pos(p3)), p1);

    data_rewrite_test(R, exp(p2, p2), p4);

    // Added a few additional checks (Wieger)
    data::rewriter datar(specification);
    data::data_expression x = data::parse_data_expression("n >= 0", "n:Nat;");

    BOOST_CHECK(datar(x) == sort_bool::true_());
    data_rewrite_test(R, greater_equal(variable("n", nat()), p0), sort_bool::true_());
  }
}

BOOST_AUTO_TEST_CASE(int_rewrite_test)
{
  using namespace mcrl2::data::sort_int;
  std::cerr << "int_rewrite_test\n";

  data_specification specification;

  specification.add_context_sort(int_());

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy4: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data_expression p0(int_(0));
    data_expression p1(int_(1));
    data_expression p2(int_(2));
    data_expression p3(int_(3));
    data_expression p4(int_(4));

    data_rewrite_test(R, plus(p0, p2), p2);
    data_rewrite_test(R, plus(p2, p0), p2);
    data_rewrite_test(R, plus(p1, p2), p3);
    data_rewrite_test(R, plus(p2, p1), p3);
    data_rewrite_test(R, plus(negate(p4), p4), p0);
    data_rewrite_test(R, minus(p4, p4), p0);

    data_rewrite_test(R, times(p1, p1), p1);
    data_rewrite_test(R, times(p0, p2), p0);
    data_rewrite_test(R, times(p2, p0), p0);
    data_rewrite_test(R, times(p1, p2), p2);

    data_rewrite_test(R, (minimum)(p1, p1), p1);
    data_rewrite_test(R, (minimum)(p0, p2), p0);
    data_rewrite_test(R, (minimum)(p2, p0), p0);
    data_rewrite_test(R, (minimum)(p1, p2), p1);

    data_rewrite_test(R, (maximum)(p1, p1), p1);
    data_rewrite_test(R, (maximum)(p0, p2), p2);
    data_rewrite_test(R, (maximum)(p2, p0), p2);
    data_rewrite_test(R, (maximum)(p1, p2), p2);

    data_rewrite_test(R, succ(p0), p1);
    data_rewrite_test(R, succ(p1), p2);

    data_rewrite_test(R, pred(p1), p0);
    data_rewrite_test(R, pred(p2), p1);

    data_rewrite_test(R, nat2int(abs(p1)), p1);

    data_rewrite_test(R, div(p1, int2pos(p1)), p1);
    data_rewrite_test(R, div(p0, int2pos(p2)), p0);
    data_rewrite_test(R, div(p2, int2pos(p1)), p2);
    data_rewrite_test(R, div(p4, int2pos(p2)), p2);

    data_rewrite_test(R, mod(p1, int2pos(p1)), R(int2nat(p0)));
    data_rewrite_test(R, mod(p0, int2pos(p2)), R(int2nat(p0)));
    data_rewrite_test(R, mod(p2, int2pos(p1)), R(int2nat(p0)));
    data_rewrite_test(R, mod(p4, int2pos(p3)), R(int2nat(p1)));

    data_rewrite_test(R, exp(p2, int2nat(p2)), p4);
  }
}

BOOST_AUTO_TEST_CASE(real_rewrite_test)
{
  using namespace mcrl2::data::sort_real;
  std::cerr << "real_rewrite_test\n";

  data_specification specification;
  specification.add_context_sort(real_());

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy5: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data_expression p0(real_(0));
    data_expression p1(real_(1));
    data_expression p2(real_(2));
    data_expression p3(real_(3));
    data_expression p4(real_(4));

    data_rewrite_test(R, plus(p0, p2), p2);
    data_rewrite_test(R, plus(p2, p0), p2);
    data_rewrite_test(R, plus(p1, p2), p3);
    data_rewrite_test(R, plus(p2, p1), p3);
    data_rewrite_test(R, plus(negate(p4), p4), p0);
    data_rewrite_test(R, minus(p4, p4), p0);

    data_rewrite_test(R, times(p1, p1), p1);
    data_rewrite_test(R, times(p0, p2), p0);
    data_rewrite_test(R, times(p2, p0), p0);
    data_rewrite_test(R, times(p1, p2), p2);

    data_rewrite_test(R, (minimum)(p1, p1), p1);
    data_rewrite_test(R, (minimum)(p0, p2), p0);
    data_rewrite_test(R, (minimum)(p2, p0), p0);
    data_rewrite_test(R, (minimum)(p1, p2), p1);

    data_rewrite_test(R, (maximum)(p1, p1), p1);
    data_rewrite_test(R, (maximum)(p0, p2), p2);
    data_rewrite_test(R, (maximum)(p2, p0), p2);
    data_rewrite_test(R, (maximum)(p1, p2), p2);

    data_rewrite_test(R, succ(p0), p1);
    data_rewrite_test(R, succ(p1), p2);

    data_rewrite_test(R, pred(p1), p0);
    data_rewrite_test(R, pred(p2), p1);

    data_rewrite_test(R, abs(p1), p1);

    data_rewrite_test(R, divides(p1, p1), p1);
    data_rewrite_test(R, divides(p0, p2), p0);
    data_rewrite_test(R, divides(p2, p1), p2);
    data_rewrite_test(R, divides(p4, p2), p2);

    data_rewrite_test(R, exp(p2, real2int(p2)), p4);

    data_rewrite_test(R, int2real(floor(real_(29, 10))), p2);

    data_rewrite_test(R, int2real(ceil(real_(12, 10))), p2);

    data_rewrite_test(R, int2real(round(real_(16, 10))), p2);
    data_rewrite_test(R, int2real(round(real_(24, 10))), p2);
  }
}

BOOST_AUTO_TEST_CASE(list_rewrite_test)
{
  using namespace mcrl2::data::sort_bool;
  using namespace mcrl2::data::sort_list;
  std::cerr << "list_rewrite_test\n";
  data_specification specification;

  specification.add_context_sort(list(bool_()));

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy6: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data_expression empty(R(nil(bool_())));
    data_expression head_true(cons_(bool_(), true_(), empty));

    data_rewrite_test(R, in(bool_(), true_(), head_true), true_());
    data_rewrite_test(R, in(bool_(), false_(), head_true), false_());
    data_rewrite_test(R, count(bool_(), head_true), sort_nat::nat(1));
    data_rewrite_test(R, in(bool_(), false_(), snoc(bool_(), head_true, true_())), false_());
    data_rewrite_test(R, concat(bool_(), head_true, head_true), R(cons_(bool_(), true_(), head_true)));
    data_rewrite_test(R, element_at(bool_(), head_true, sort_nat::nat(0)), true_());
    data_rewrite_test(R, head(bool_(), head_true), true_());
    data_rewrite_test(R, rhead(bool_(), head_true), true_());
    data_rewrite_test(R, rtail(bool_(), head_true), empty);
    data_rewrite_test(R, tail(bool_(), head_true), empty);
  }
}

BOOST_AUTO_TEST_CASE(struct_list_rewrite_test)
{
  using namespace mcrl2::data::sort_bool;
  using namespace mcrl2::data::sort_list;
  std::cerr << "struct_list_rewrite_test\n";
  data_specification specification=parse_data_specification("sort L = struct l( R: List(Bool) );"
                                                            "map  f:Set(L);");   // Map is added to guarantee that rewrite rules for sets
                                                                                  // are present.


  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy7: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    const data_expression e1=parse_data_expression("l( [true]) in {l( [])}",specification);
    const data_expression e2=parse_data_expression("l( [true]) in {l( [true])}",specification);
    const data_expression e3=parse_data_expression("l( [true]) in {l( [true]),l([false])}",specification);
    const data_expression e4=parse_data_expression("l( [true]) in {l([false])}",specification);

    data_rewrite_test(R, e1, false_());
    data_rewrite_test(R, e2, true_());
    data_rewrite_test(R, e3, true_());
    data_rewrite_test(R, e4, false_());
  }
}


BOOST_AUTO_TEST_CASE(set_rewrite_test)
{
  using namespace mcrl2::data::sort_set;
  using namespace mcrl2::data::sort_fset;
  using namespace mcrl2::data::sort_nat;
  using namespace mcrl2::data::sort_bool;
  std::cerr << "set_rewrite_test" << std::endl;
//  data_specification specification = parse_data_specification(
//    "sort A = Set(Nat);"
//  );

  data_specification specification;
  specification.add_context_sort(set_(nat()));
  specification.add_context_sort(set_(bool_()));

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy8: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    sort_expression set_nat(set_(nat()));

    data_expression empty(R(normalize_sorts(emptyset(nat()),specification)));

    data_expression p0(nat(0));
    data_expression p1(nat(1));
    data_expression p2(nat(2));

    data_rewrite_test(R, normalize_sorts(equal_to(true_function(nat()), true_function(nat())),specification), true_());
    data_rewrite_test(R, normalize_sorts(equal_to(false_function(nat()), false_function(nat())),specification), true_());
    data_rewrite_test(R, normalize_sorts(equal_to(true_function(nat()), false_function(nat())),specification), false_());
    data_rewrite_test(R, normalize_sorts(equal_to(false_function(nat()), true_function(nat())),specification), false_());

    data_expression s1(R(normalize_sorts(setfset(nat(), fsetinsert(nat(), p1, fset_empty(nat()))),specification)));
    data_expression s2(R(normalize_sorts(setfset(nat(), fsetinsert(nat(), p2, fset_empty(nat()))),specification)));
    data_expression s(R(normalize_sorts(setfset(nat(), fsetinsert(nat(), p1, fsetinsert(nat(), p2, fset_empty(nat())))),specification)));

    data_expression empty_complement(normalize_sorts(setcomplement(nat(),emptyset(nat())),specification));
    data_expression intersection(normalize_sorts(setintersection(nat(),setcomplement(nat(),emptyset(nat())),emptyset(nat())),specification));
    //data_rewrite_test(R, normalize_sorts(less_equal(empty_complement,empty),specification), false_());
    data_rewrite_test(R, normalize_sorts(less_equal(empty,empty_complement),specification), true_());
    //data_rewrite_test(R, normalize_sorts(equal_to(intersection, empty_complement),specification), true_());
    //data_rewrite_test(R, normalize_sorts(equal_to(empty_complement, intersection),specification), true_());

    data_rewrite_test(R, normalize_sorts(setin(nat(), p0, s),specification), false_());
    data_rewrite_test(R, normalize_sorts(setin(nat(), p1, s),specification), true_());
    data_rewrite_test(R, normalize_sorts(setin(nat(), p2, s),specification), true_());

    data_rewrite_test(R, normalize_sorts(setunion_(nat(), s, empty),specification), s);
    data_rewrite_test(R, normalize_sorts(setunion_(nat(), s1, s2),specification), s);

    data_rewrite_test(R, normalize_sorts(setintersection(nat(), s, empty),specification), empty);
    data_rewrite_test(R, normalize_sorts(setintersection(nat(), s, s1),specification), s1);
    data_rewrite_test(R, normalize_sorts(setintersection(nat(), s, s2),specification), s2);

    data_rewrite_test(R, normalize_sorts(setdifference(nat(), s, empty),specification), s);
    data_rewrite_test(R, normalize_sorts(setdifference(nat(), s, s1),specification), s2);
    data_rewrite_test(R, normalize_sorts(setdifference(nat(), s, s2),specification), s1);

    data_rewrite_test(R, normalize_sorts(setin(nat(), p0, setcomplement(nat(), s)),specification), true_());
    // extra tests by Jan Friso
    data::data_expression x = data::parse_data_expression("1 in {n:Nat|n<5}",specification);
    data_rewrite_test(R, x, true_());
    x = data::parse_data_expression("7 in {n:Nat|n<5}",specification);
    data_rewrite_test(R, x, false_());
    x = data::parse_data_expression("true in {n:Bool|n || !n}",specification);
    data_rewrite_test(R, x, true_());
    x = data::parse_data_expression("false in {n:Bool|n && !n}",specification);
    data_rewrite_test(R, x, false_());

    // test for a variation on bug #721,
    // see also set_bool_rewrite_test()
    x = not_(fsetin(nat(), c0(), fsetinsert(nat(), cnat(sort_pos::c1()), fset_cons(nat(), c0(), fset_empty(nat())))));
    x = normalize_sorts(x,specification);
    data_rewrite_test(R, x, false_());
  }
}

BOOST_AUTO_TEST_CASE(bag_rewrite_test)
{
  using namespace mcrl2::data::sort_bag;
  using namespace mcrl2::data::sort_fbag;
  using namespace mcrl2::data::sort_nat;
  using namespace mcrl2::data::sort_pos;
  using namespace mcrl2::data::sort_bool;

  data_specification specification = parse_data_specification(
                                       "sort A = Bag(Nat);"
                                     );

  specification.add_context_sort(bag(nat()));
  specification.add_context_sort(bag(pos()));

  std::cerr << "bag rewrite test\n";
  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy9: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    sort_expression bag_nat(sort_bag::bag(nat()));

    data_expression empty(R(normalize_sorts(emptybag(nat()),specification)));

    data_expression p0(nat(0));
    data_expression p1(nat(1));
    data_expression p2(nat(2));

    data_expression s1(R(normalize_sorts(bagfbag(nat(), fbaginsert(nat(), p1, pos(1), fbag_empty(nat()))),specification)));
    data_expression s2(R(normalize_sorts(bagfbag(nat(), fbaginsert(nat(), p2, pos(2), fbag_empty(nat()))),specification)));
    data_expression s(R(normalize_sorts(bagfbag(nat(), fbaginsert(nat(), p1, pos(1), fbaginsert(nat(), p2, pos(2), fbag_empty(nat())))),specification)));

    data_rewrite_test(R, normalize_sorts(bagin(nat(), p0, s),specification), false_());
    data_rewrite_test(R, normalize_sorts(bagin(nat(), p1, s),specification), true_());
    data_rewrite_test(R, normalize_sorts(bagin(nat(), p2, s),specification), true_());

    data_rewrite_test(R, normalize_sorts(bagcount(nat(), p0, s),specification), p0);
    data_rewrite_test(R, normalize_sorts(bagcount(nat(), p1, s),specification), p1);
    data_rewrite_test(R, normalize_sorts(bagcount(nat(), p2, s),specification), p2);

    data_rewrite_test(R, normalize_sorts(bagjoin(nat(), s, empty),specification), s);
    data_rewrite_test(R, normalize_sorts(bagjoin(nat(), s1, s2),specification), s);

    data_rewrite_test(R, normalize_sorts(bagintersect(nat(), s, empty),specification), empty);
    data_rewrite_test(R, normalize_sorts(bagintersect(nat(), s, s1),specification), s1);
    data_rewrite_test(R, normalize_sorts(bagintersect(nat(), s, s2),specification), s2);

    data_rewrite_test(R, normalize_sorts(bagdifference(nat(), s, empty),specification), s);
    data_rewrite_test(R, normalize_sorts(bagdifference(nat(), s, s1),specification), s2);
    data_rewrite_test(R, normalize_sorts(bagdifference(nat(), s, s2),specification), s1);

    // extra tests by Jan Friso
    data::data_expression x = data::parse_data_expression("count(1,{ 1:1,2:2})==1",specification);
    data_rewrite_test(R, x, true_());
    x = data::parse_data_expression("count(2,{ 1:1,2:2})==2",specification);
    data_rewrite_test(R, x, true_());
    x = data::parse_data_expression("count(2,{ 1:1,2:2})==1",specification);
    data_rewrite_test(R, x, false_());
  }
}

BOOST_AUTO_TEST_CASE(structured_sort_rewrite_test)
{
  using namespace sort_bool;
  using namespace sort_nat;

  data_specification specification;

  atermpp::vector< structured_sort_constructor_argument > arguments;

  arguments.push_back(structured_sort_constructor_argument("a0", bool_()));
  arguments.push_back(structured_sort_constructor_argument(static_cast<sort_expression const&>(bool_())));
  arguments.push_back(structured_sort_constructor_argument("n0", sort_nat::nat()));
  arguments.push_back(structured_sort_constructor_argument("n1", sort_nat::nat()));

  atermpp::vector< structured_sort_constructor > constructors;
  // without arguments or recogniser
  //  c0
  constructors.push_back(structured_sort_constructor("c0"));
  // without arguments, with recogniser
  //  c1?is_one
  constructors.push_back(structured_sort_constructor("c1", std::string("is_one")));
  // with arguments, without recogniser
  //  a(a0 : A)
  constructors.push_back(structured_sort_constructor("a",
                         boost::make_iterator_range(arguments.begin(), arguments.begin() + 1)));
  // two arguments, with recogniser
  //  b(B)?is_b
  constructors.push_back(structured_sort_constructor("b",
                         boost::make_iterator_range(arguments.begin() + 1, arguments.begin() + 2), "is_b"));
  //  c(n0 : Nat, n1 : Nat)?is_c
  constructors.push_back(structured_sort_constructor("c",
                         boost::make_iterator_range(arguments.begin() + 2, arguments.end()), "is_c"));

  data::structured_sort ls(boost::make_iterator_range(constructors));

  specification.add_alias(alias(basic_sort("D"), ls));
  // specification.normalize_sorts();

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy10: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data_expression c0(constructors[0].constructor_function(ls));
    data_expression c1(constructors[1].constructor_function(ls));
    data_expression a(constructors[2].constructor_function(ls)(true_()));
    data_expression b(constructors[3].constructor_function(ls)(false_()));
    data_expression n0(nat("0"));
    data_expression n1(nat("1"));
    data_expression c(constructors[4].constructor_function(ls)(n0, n1));

    // recogniser tests
    data_rewrite_test(R, normalize_sorts(make_application(constructors[1].recogniser_function(ls), c0),specification), false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[3].recogniser_function(ls), c0),specification), false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[4].recogniser_function(ls), c0),specification), false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[1].recogniser_function(ls), c1),specification), true_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[3].recogniser_function(ls), c1),specification), false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[4].recogniser_function(ls), c1),specification), false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[1].recogniser_function(ls), a),specification),  false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[3].recogniser_function(ls), a),specification),  false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[4].recogniser_function(ls), a),specification),  false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[1].recogniser_function(ls), b),specification),  false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[3].recogniser_function(ls), b),specification),  true_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[4].recogniser_function(ls), b),specification),  false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[1].recogniser_function(ls), c),specification),  false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[3].recogniser_function(ls), c),specification),  false_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[4].recogniser_function(ls), c),specification),  true_());

    // projection tests
    data_rewrite_test(R, normalize_sorts(make_application(constructors[2].projection_functions(ls)[0], a),specification),  true_());
    data_rewrite_test(R, normalize_sorts(make_application(constructors[4].projection_functions(ls)[0], c),specification),  R(n0));
    data_rewrite_test(R, normalize_sorts(make_application(constructors[4].projection_functions(ls)[1], c),specification),  R(n1));
  }
}

// Test for bug #721
BOOST_AUTO_TEST_CASE(set_bool_rewrite_test)
{
  using namespace sort_bool;
  using namespace sort_nat;
  using namespace sort_set;
  using namespace sort_fset;

  data_specification specification;

  // Rewrite without sort alias
  specification.add_context_sort(set_(bool_()));
  specification.add_context_sort(fset(bool_()));

  std::cerr << "set bool rewrite test\n";
  data_expression e(not_(fsetin(bool_(), true_(), fsetinsert(bool_(), false_(), fset_cons(bool_(), true_(), fset_empty(bool_()))))));
  e = normalize_sorts(e,specification);
  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy11: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);
    data_rewrite_test(R, e, false_());
  }

  // Rewrite with sort alias
  specification = parse_data_specification("sort S = Set(Bool);\n");
  specification.add_context_sort(set_(bool_()));
  specification.add_context_sort(fset(bool_()));

  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy12: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    e = not_(fsetin(bool_(), true_(), fsetinsert(bool_(), false_(), fset_cons(bool_(), true_(), fset_empty(bool_())))));
    e = normalize_sorts(e,specification);
    data_rewrite_test(R, e, false_());

    // Rewrite with parsing
    e = parse_data_expression("true in {true, false}", specification);
    data_rewrite_test(R, e, true_());
  }


  specification = data_specification();
  specification.add_context_sort(set_(nat()));

  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy13: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    // test for a variation on bug #721
    e = not_(fsetin(nat(), c0(), fsetinsert(nat(), cnat(sort_pos::c1()), fset_cons(nat(), c0(), fset_empty(nat())))));
    e = normalize_sorts(e,specification);
    data_rewrite_test(R, e, false_());
  }
}

BOOST_AUTO_TEST_CASE(finite_set_nat_rewrite_test)
{
  using namespace mcrl2::data::sort_set;
  using namespace mcrl2::data::sort_fset;
  using namespace mcrl2::data::sort_nat;
  using namespace mcrl2::data::sort_bool;
  std::cerr << "finite_set_nat_rewrite_test\n";
  data_specification specification = parse_data_specification(
                                       "sort A = Set(Nat);"
                                     );

  specification.add_context_sort(set_(nat()));
  specification.add_context_sort(set_(bool_()));

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy14: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data::data_expression x = not_(fsetin(nat(), c0(), fsetinsert(nat(), cnat(sort_pos::c1()), fset_cons(nat(), c0(), fset_empty(nat())))));
    x = normalize_sorts(x,specification);
    data_rewrite_test(R, x, false_());
  }
}

BOOST_AUTO_TEST_CASE(finite_set_nat_rewrite_test_without_alias)
{
  using namespace mcrl2::data::sort_set;
  using namespace mcrl2::data::sort_fset;
  using namespace mcrl2::data::sort_nat;
  using namespace mcrl2::data::sort_bool;
  std::cerr << "finite_set_nat_rewrite_test_without_alias\n";
  data_specification specification;

  specification.add_context_sort(set_(nat()));

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy15: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data::data_expression x;

    x = less(cnat(sort_pos::c1()), c0());
    x = normalize_sorts(x,specification);
    data_rewrite_test(R, x, false_());

    x = less(c0(), cnat(sort_pos::c1()));
    x = normalize_sorts(x,specification);
    data_rewrite_test(R, x, true_());

    x = fsetin(nat(), c0(), fset_cons(nat(), c0(), fset_empty(nat())));
    x = normalize_sorts(x,specification);
    data_rewrite_test(R, x, true_());

    x = fsetinsert(nat(), cnat(sort_pos::c1()), fset_cons(nat(), c0(), fset_empty(nat())));
    x = normalize_sorts(x,specification);
    data_rewrite_test(R, x, fset_cons(nat(), c0(), fset_cons(nat(), cnat(sort_pos::c1()), fset_empty(nat()))));

    x = fsetin(nat(), c0(), fsetinsert(nat(), cnat(sort_pos::c1()), fset_cons(nat(), c0(), fset_empty(nat()))));
    x = normalize_sorts(x,specification);
    data_rewrite_test(R, x, true_());

    x = not_(fsetin(nat(), c0(), fsetinsert(nat(), cnat(sort_pos::c1()), fset_cons(nat(), c0(), fset_empty(nat())))));
    x = normalize_sorts(x,specification);
    data_rewrite_test(R, x, false_());
  }
}

BOOST_AUTO_TEST_CASE(regression_test_bug_723)
{
  std::string s(
    "sort BL = List(Bool);\n"
    "map initial: Nat -> BL;\n"
    "    all_false: BL -> Bool;\n"
    "var b0: Bool;\n"
    "    bl: BL;\n"
    "    n: Nat;\n"
    "eqn initial (1) = [];\n"
    "    n > 1 -> initial (n) = false |> initial(Int2Nat(n-1));\n"
    "    all_false ([]) = true;\n"
    "    all_false (b0 |> bl) = !b0 && all_false(bl);\n"
  );

  std::cerr << "List of booleans test, using initial" << std::endl;
  data_specification specification(parse_data_specification(s));

  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy16: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    const data::data_expression e(parse_data_expression("all_false(initial(1))", specification));
    data_rewrite_test(R, e, sort_bool::true_());

    const data::data_expression e1(parse_data_expression("all_false(initial(2))", specification));
    data_rewrite_test(R, e1, sort_bool::true_());

    const data::data_expression e2(parse_data_expression("all_false(initial(4))", specification));
    data_rewrite_test(R, e2, sort_bool::true_());
  }
}

BOOST_AUTO_TEST_CASE(test_othello_condition)
{
  std::string s(
    "sort Piece = struct Red | White | None;\n"
    "     Row = List(Piece);\n"
    "     Board = List(Row);\n"
    "map  At:Nat#Nat#Board->Piece;\n"
    "     At:Nat#Row->Piece;\n"
    "     Put:Piece#Pos#Pos#Board->Board;\n"
    "     Put:Piece#Pos#Row->Row;\n"
    "     N,M: Pos;\n"
    "var  b,b':Board;\n"
    "     r:Row;\n"
    "     p,p':Piece;\n"
    "     x,y:Nat;\n"
    "     c:Bool;\n"
    "     z:Pos;\n"
    "eqn  N = 4;\n"
    "     M = 4;\n"
    "     y==1 -> At(x,y,r|>b)=At(x,r);\n"
    "     1<y && y<=M -> At(x,y,r|>b)=At(x,Int2Nat(y-1),b);\n"
    "     y==0 || y>M || x==0 || x>N -> At(x,y,b)=None;\n"
    "     At(x,y,if(c,b,b'))=if(c,At(x,y,b),At(x,y,b'));\n"
    "     x==1 -> At(x,p|>r)=p;\n"
    "     1<x && x<=N -> At(x,p|>r)=At(Int2Nat(x-1),r);\n"
    "     x==0 || x>N -> At(x,p|>r)=None;\n"
    "     At(x,Put(p,z,r))=if(x==z,p,At(x,r));\n"
    "var b,b':Board;\n"
    "     r:Row;\n"
    "     p,p':Piece;\n"
    "     x,y:Pos;\n"
    "     c:Bool;\n"
    "eqn  y==1 -> Put(p,x,y,r|>b)=Put(p,x,r)|>b;\n"
    "     y>1 && y<=M -> Put(p,x,y,r|>b)=r|>Put(p,x,Int2Pos(y-1),b);\n"
    "     Put(p,x,y,if(c,b,b'))=if(c,Put(p,x,y,b),Put(p,x,y,b'));\n"
    "     x==1 -> Put(p,x,p'|>r)=p|>r;\n"
    "     x>1 && x<=N -> Put(p,x,p'|>r)=p'|>Put(p,Int2Pos(x-1),r);\n"
  );

  data_specification specification(parse_data_specification(s));

  std::cerr << "othello test\n";
  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy17: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data::data_expression e(parse_data_expression("At(1, 2, [[None, None, None, None], [None, Red, White, None], [None, White, Red, None], [None, None, None, None]]) == None", specification));
    data_rewrite_test(R, e, sort_bool::true_());
  }
}

BOOST_AUTO_TEST_CASE(test_lambda_expression)
{
  std::string s(
    "map  n: Pos;\n"
    "eqn  n=2;\n"
    "sort D = struct d1 | d2;\n"
    "     Buf = Nat -> struct data(getdata:D) | empty;\n"
    "map  emptyBuf: Buf;\n"
    "     insert: D#Nat#Buf -> Buf;\n"
    "     remove: Nat#Buf -> Buf;\n"
    "     release: Nat#Nat#Buf -> Buf;\n"
    "     nextempty: Nat#Buf -> Nat;\n"
    "     nextempty_rec: Nat#Buf#Nat -> Nat;\n"
    "     inWindow: Nat#Nat#Nat -> Bool;\n"
    "     nat_const:Nat;\n"
    "var  i,j,k: Nat; d: D; q: Buf;\n"
    "eqn  emptyBuf = lambda j:Nat.empty;\n"
    "     insert(d,i,q) = lambda j:Nat.if(i==j,data(d),q(j));\n"
    "     remove(i,q) = lambda j:Nat.if(i==j,empty,q(j));\n"
    "     i mod 2*n==j mod 2*n -> release(i,j,q) = q;\n"
    "     i mod 2*n!=j mod 2*n ->\n"
    "          release(i,j,q) = release((i+1) mod 2*n,j,remove(i,q));\n"
    "     k<n -> nextempty_rec(i,q,k) =\n"
    "               if(q(i)==empty,i,nextempty_rec((i+1) mod n,q,k+1));\n"
    "     k==n -> nextempty_rec(i,q,k)=i;\n"
    "     nextempty(i,q) = nextempty_rec(i,q,0);\n"
    "     inWindow(i,j,k) = (i<=j && j<k) || (k<i && i<=j) || (j<k && k<i);\n"
  );

  data_specification specification(parse_data_specification(s));

  std::cerr << "lambda rewrite test\n";
  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy18: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data::data_expression e(parse_data_expression("(insert(d2,2,insert(d1,1,emptyBuf)))(nat_const)", specification));
    data_rewrite_test(R, e, R(e));
    e=parse_data_expression("insert(d1,1,emptyBuf)(1)==data(d1)", specification);
    data_rewrite_test(R, e, sort_bool::true_());
    e=parse_data_expression("insert(d1,1,emptyBuf)(1)==empty", specification);
    data_rewrite_test(R, e, sort_bool::false_());
    e=parse_data_expression("remove(1,emptyBuf)(1)==data(d1)", specification);
    data_rewrite_test(R, e, sort_bool::false_());
    e=parse_data_expression("remove(1,emptyBuf)(1)==empty", specification);
    data_rewrite_test(R, e, sort_bool::true_());
    e=parse_data_expression("remove(1,insert(d1,1,emptyBuf))(1)==insert(d1,1,emptyBuf)(1)", specification);
    data_rewrite_test(R, e, sort_bool::false_());
  }
}

BOOST_AUTO_TEST_CASE(test_whether_lists_can_be_put_in_sets)
{
  std::string s(
    "sort T = struct s( List(T) ) | c;\n"
    "map func: Bool;\n"
    "eqn func = s( [c] ) in { s( [] ) };\n"
  );

  data_specification specification(parse_data_specification(s));

  std::cerr << "list in set teste\n";
  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy19: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data::data_expression e(parse_data_expression("func", specification));
    data_rewrite_test(R, e, sort_bool::false_());
  }

}

BOOST_AUTO_TEST_CASE(lambda_predicate_matching)
{
//  Taken from the specification:
//  ================================
//        map  match :(Nat -> Bool)#List(Nat)     -> List(Nat) ;
//             pre: Nat -> Bool;
//             emptyList: List(Nat);
//        eqn  match (pre, [])      = [];
//             emptyList = [];
//
//        init ( match( lambda i:Nat. true, [] ) == emptyList  ) -> tau;
//  ================================


  std::string s(
     "map  match    : (Nat -> Bool)#List(Nat) -> List(Nat);\n"
     "     emptyList: List(Nat);\n"
     "var  v      : Nat  -> Bool;\n"
     "eqn  match(v, [])      = emptyList;"
  );

  data_specification specification(parse_data_specification(s));

  std::cerr << "lambda_predicate_matching\n";
  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy20: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data::data_expression e(parse_data_expression("match( lambda i:Nat. true, [] )", specification));
    data::data_expression f(parse_data_expression("emptyList", specification));
    data_rewrite_test(R, e, f);
  }

}

BOOST_AUTO_TEST_CASE(difficult_empty_list_in_set)
{
  /* Taken from the specification:
  ================================
  map F1: List(Bool)#Bool#List(Bool)#Bag(Bool) -> List(Bool);
      F2: List(Bool) -> Bag(Bool);
      ELM: List(Bool)#Bool#List(Bool) -> List(Bool);
  var ca: Bool;
      cal: List(Bool);
      b: Bool;
      bs: List(Bool);
      m: Bag(Bool);
      a: Bool;
  eqn
      F1( cal, b, [] ,  m ) = [];
      F1( cal, b, a |> bs, m ) = if( m <= {}, ELM( cal, b, a |> bs ) , [] );
      ELM( [] , b, bs ) =  F1( [] , b, bs, { false:1 } );
      ELM( ca |> cal, b, bs ) = ELM( cal, b , bs );

  act a: List(Bool);

  proc X = sum r: List(Bool). ( r in { a: List(Bool) | exists ac': List(Bool). a ==  F1( [false], false, ac', { false:1 })} )-> a(r) . X;

  init X;
  ================================
  */

  std::string s(
  "map F1: List(Bool)#Bool#List(Bool)#Bag(Bool) -> List(Bool);"
  "    F2: Set(List(Bool)); %declared to add the rewrite rules for Set(List(Bool)).\n"
  "    ELM: List(Bool)#Bool#List(Bool) -> List(Bool);"
  "var ca: Bool;"
  "    cal: List(Bool);"
  "    b: Bool;"
  "    bs: List(Bool);"
  "    m: Bag(Bool);"
  "    a: Bool;"
  "eqn "
  "    F1( cal, b, [] ,  m ) = [];"
  "    F1( cal, b, a |> bs, m ) = if( m <= {}, ELM( cal, b, a |> bs ) , [] );"
  "    ELM( [] , b, bs ) =  F1( [] , b, bs, { false:1 } );"
  "    ELM( ca |> cal, b, bs ) = ELM( cal, b , bs );"
  );

  data_specification specification(parse_data_specification(s));

  std::cerr << "difficult_empty_list_in_set\n";
  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy21: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data::data_expression e(parse_data_expression("[] in { a: List(Bool) | exists ac': List(Bool). a ==  F1( [false], false, ac', { false:1 }) }", specification));
    data::data_expression f(parse_data_expression("true", specification));
    data_rewrite_test(R, e, f);
  }

}

BOOST_AUTO_TEST_CASE(bound_existential_quantifiers_with_same_name)
{
  /* Taken from a pbes provided by Tim Willemse, showing a conflict of variable name x0
  that are bound by different existential quantifiers of different type.
  =====================================================================================
  sort AbsNum = struct e0 | e1 | e2 | e3 | e4 ;

  map
       absplus: AbsNum # AbsNum -> Set(AbsNum);
       absle: AbsNum # AbsNum -> Set(Bool);
       Lifted_and: Set(Bool) # Set(Bool) -> Set(Bool);
       Lifted_absle: Set(AbsNum) # Set(AbsNum) -> Set(Bool);
       Lifted_absplus: Set(AbsNum) # Set(AbsNum) -> Set(AbsNum);
  
  var
       n,m: AbsNum;
  eqn
       absplus(e0, n)  =  {n};
       absplus(e2, e2)  =  {e3};
       absle(n,m) = {false};
  
  var  X0,X1: Set(Bool);
  eqn
       Lifted_and(X0, X1)  =  { y: Bool | exists x0,x1: Bool. (x0 in X0) && (x1 in X1) && (y in {x0 && x1}) } ;
  
  var  X0,X1: Set(AbsNum);
  eqn  Lifted_absle(X0, X1)  =  { y: Bool | exists x0,x1: AbsNum. (x0 in X0) && (x1 in X1) && (y in absle(x0, x1)) };
       Lifted_absplus(X0, X1)  =  { y: AbsNum | exists x0,x1: AbsNum. (x0 in X0) && (x1 in X1) && (y in absplus(x0, x1)) };
  
  
  pbes
       mu X(t_P: AbsNum) = val(false in (Lifted_and( Lifted_absle({e2}, Lifted_absplus({t_P}, {e2})) , {true}))) ;
  
  init X(e0);
  */

  std::string s(
  "sort AbsNum = struct e0 | e1 | e2 | e3 | e4 ;\n"

  "map\n"
  "     absplus: AbsNum # AbsNum -> Set(AbsNum);\n"
  "     absle: AbsNum # AbsNum -> Set(Bool);\n"
  "     Lifted_and: Set(Bool) # Set(Bool) -> Set(Bool);\n"
  "     Lifted_absle: Set(AbsNum) # Set(AbsNum) -> Set(Bool);\n"
  "     Lifted_absplus: Set(AbsNum) # Set(AbsNum) -> Set(AbsNum);\n"
  "\n"
  "var\n"
  "     n,m: AbsNum;\n"
  "eqn\n"
  "     absplus(e0, n)  =  {n};\n"
  "     absplus(e2, e2)  =  {e3};\n"
  "     absle(n,m) = {false};\n"
  "\n"
  "var  X0,X1: Set(Bool);\n"
  "eqn\n"
  "     Lifted_and(X0, X1)  =  { y: Bool | exists x0,x1: Bool. (x0 in X0) && (x1 in X1) && (y in {x0 && x1}) } ;\n"
  "\n"
  "var  X0,X1: Set(AbsNum);\n"
  "eqn  Lifted_absle(X0, X1)  =  { y: Bool | exists x0,x1: AbsNum. (x0 in X0) && (x1 in X1) && (y in absle(x0, x1)) };\n"
  "     Lifted_absplus(X0, X1)  =  { y: AbsNum | exists x0,x1: AbsNum. (x0 in X0) && (x1 in X1) && (y in absplus(x0, x1)) };\n"
  );

  data_specification specification(parse_data_specification(s));

  std::cerr << "existentially bound variable x0 of different types\n";
  rewrite_strategy_vector strategies(utilities::get_test_rewrite_strategies(false));
  for (rewrite_strategy_vector::const_iterator strat = strategies.begin(); strat != strategies.end(); ++strat)
  {
    std::cerr << "  Strategy22: " << data::pp(*strat) << std::endl;
    data::rewriter R(specification, *strat);

    data::data_expression e(parse_data_expression("false in (Lifted_and( Lifted_absle({e2}, Lifted_absplus({e0}, {e2})) , {true}))", specification));
    data::data_expression f(parse_data_expression("true", specification));
    data_rewrite_test(R, e, f);
  }


  
}

boost::unit_test::test_suite* init_unit_test_suite(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  return 0;
}
