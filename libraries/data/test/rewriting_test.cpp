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
#include <boost/test/minimal.hpp>
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/atermpp/map.h"
#include "mcrl2/core/text_utility.h"
#include "mcrl2/data/nat.h"
#include "mcrl2/data/int.h"
#include "mcrl2/data/real.h"
#include "mcrl2/data/list.h"
#include "mcrl2/data/set.h"
#include "mcrl2/data/bag.h"
#include "mcrl2/data/structured_sort.h"
#include "mcrl2/data/find.h"
#include "mcrl2/data/parse.h"
#include "mcrl2/data/replace.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/data/function_sort.h"
#include "mcrl2/data/utility.h"
#include "mcrl2/data/standard_utility.h"
#include "mcrl2/data/detail/data_functional.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/core/garbage_collection.h"

using namespace atermpp;
using namespace mcrl2;
using namespace mcrl2::core;
using namespace mcrl2::data;
using namespace mcrl2::data::detail;

template <typename Rewriter>
void data_rewrite_test(Rewriter& R, data_expression const& input, data_expression const& expected_output) 
{ data_expression output = R(input);

  BOOST_CHECK(output == expected_output);

  if (output != expected_output) {
    std::clog << "--- test failed --- " << core::pp(input) << " ->* " << core::pp(expected_output) << std::endl
              << "input    " << core::pp(input) << std::endl
              << "expected " << core::pp(expected_output) << std::endl
              << "output " << core::pp(output) << std::endl
              << " -- term representations -- " << std::endl
              << "input    " << input << std::endl
              << "expected " << expected_output << std::endl
              << "R(input) " << output << std::endl;
  }
}

void bool_rewrite_test() {
  using namespace mcrl2::data::sort_bool;

  data_specification specification;

  specification.add_context_sort(bool_());

  data::rewriter R(specification);

  data_rewrite_test(R, true_(), true_());
  data_rewrite_test(R, false_(), false_());

  data_rewrite_test(R, and_(true_(), false_()), false_());
  data_rewrite_test(R, and_(false_(), true_()), false_());

  data_rewrite_test(R, or_(true_(), false_()), true_());
  data_rewrite_test(R, or_(false_(), true_()), true_());

  data_rewrite_test(R, implies(true_(), false_()), false_());
  data_rewrite_test(R, implies(false_(), true_()), true_());
}

void pos_rewrite_test() {
  using namespace mcrl2::data::sort_pos;
  std::cerr << "pos_rewrite_test\n";

  data_specification specification;

  specification.add_context_sort(pos());

  data::rewriter R(specification);
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

  data_rewrite_test(R, sort_pos::abs(p4), p4);
}

void nat_rewrite_test() {
  using namespace mcrl2::data::sort_nat;
  std::cerr << "nat_rewrite_test\n";

  data_specification specification;

  specification.add_context_sort(nat());

  data::rewriter R(specification);

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

  data_rewrite_test(R, abs(p1), p1);

  data_rewrite_test(R, div(p1, sort_pos::pos(1)), p1);
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

void int_rewrite_test() 
{
  using namespace mcrl2::data::sort_int;
  std::cerr << "int_rewrite_test\n";

  data_specification specification;

  specification.add_context_sort(int_());

  data::rewriter R(specification);

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

void real_rewrite_test() 
{ using namespace mcrl2::data::sort_real;
  std::cerr << "real_rewrite_test\n";

  data_specification specification;
  specification.add_context_sort(real_());

  data::rewriter R(specification);

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

void list_rewrite_test() 
{ using namespace mcrl2::data::sort_bool;
  using namespace mcrl2::data::sort_list;
  std::cerr << "list_rewrite_test\n";
  data_specification specification;

  specification.add_context_sort(list(bool_()));

  data::rewriter R(specification);

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

void set_rewrite_test() 
{
  using namespace mcrl2::data::sort_set;
  using namespace mcrl2::data::sort_fset;
  using namespace mcrl2::data::sort_nat;
  using namespace mcrl2::data::sort_bool;
  std::cerr << "set_rewrite_test\n";
  data_specification specification = parse_data_specification(
    "sort A = Set(Nat);"
  );

  specification.add_context_sort(set_(nat()));
  specification.add_context_sort(set_(bool_()));

  data::rewriter R(specification);

  sort_expression set_nat(sort_set::set_(nat()));

  data_expression empty(R(specification.normalise_sorts(emptyset(nat()))));

  data_expression p0(nat(0));
  data_expression p1(nat(1));
  data_expression p2(nat(2));

  data_expression s1(R(specification.normalise_sorts(setfset(nat(), fsetinsert(nat(), p1, fset_empty(nat()))))));
  data_expression s2(R(specification.normalise_sorts(setfset(nat(), fsetinsert(nat(), p2, fset_empty(nat()))))));
  data_expression s(R(specification.normalise_sorts(setfset(nat(), fsetinsert(nat(), p1, fsetinsert(nat(), p2, fset_empty(nat())))))));

  data_rewrite_test(R, specification.normalise_sorts(setin(nat(), p0, s)), false_());
  data_rewrite_test(R, specification.normalise_sorts(setin(nat(), p1, s)), true_());
  data_rewrite_test(R, specification.normalise_sorts(setin(nat(), p2, s)), true_());

  data_rewrite_test(R, specification.normalise_sorts(setunion_(nat(), s, empty)), s);
  data_rewrite_test(R, specification.normalise_sorts(setunion_(nat(), s1, s2)), s);

  data_rewrite_test(R, specification.normalise_sorts(setintersection(nat(), s, empty)), empty);
  data_rewrite_test(R, specification.normalise_sorts(setintersection(nat(), s, s1)), s1);
  data_rewrite_test(R, specification.normalise_sorts(setintersection(nat(), s, s2)), s2);

  data_rewrite_test(R, specification.normalise_sorts(setdifference(nat(), s, empty)), s);
  data_rewrite_test(R, specification.normalise_sorts(setdifference(nat(), s, s1)), s2);
  data_rewrite_test(R, specification.normalise_sorts(setdifference(nat(), s, s2)), s1);

  data_rewrite_test(R, specification.normalise_sorts(setin(nat(), p0, setcomplement(nat(), s))), true_());
  // extra tests by Jan Friso
  data::data_expression x = data::parse_data_expression("1 in {n:Nat|n<5}",specification);
  data_rewrite_test(R, x, true_());
  x = data::parse_data_expression("7 in {n:Nat|n<5}",specification);
  data_rewrite_test(R, x, false_());
  x = data::parse_data_expression("true in {n:Bool|n || !n}",specification);
  data_rewrite_test(R, x, true_());
  x = data::parse_data_expression("false in {n:Bool|n && !n}",specification);
  data_rewrite_test(R, x, false_());
}

void bag_rewrite_test() 
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

  data::rewriter R(specification);

  sort_expression bag_nat(sort_bag::bag(nat()));

  data_expression empty(R(specification.normalise_sorts(emptybag(nat()))));

  data_expression p0(nat(0));
  data_expression p1(nat(1));
  data_expression p2(nat(2));

  data_expression s1(R(specification.normalise_sorts(bagfbag(nat(), fbaginsert(nat(), p1, pos(1), fbag_empty(nat()))))));
  data_expression s2(R(specification.normalise_sorts(bagfbag(nat(), fbaginsert(nat(), p2, pos(2), fbag_empty(nat()))))));
  data_expression s(R(specification.normalise_sorts(bagfbag(nat(), fbaginsert(nat(), p1, pos(1), fbaginsert(nat(), p2, pos(2), fbag_empty(nat())))))));

  data_rewrite_test(R, specification.normalise_sorts(bagin(nat(), p0, s)), false_());
  data_rewrite_test(R, specification.normalise_sorts(bagin(nat(), p1, s)), true_());
  data_rewrite_test(R, specification.normalise_sorts(bagin(nat(), p2, s)), true_());

  data_rewrite_test(R, specification.normalise_sorts(bagcount(nat(), p0, s)), p0);
  data_rewrite_test(R, specification.normalise_sorts(bagcount(nat(), p1, s)), p1);
  data_rewrite_test(R, specification.normalise_sorts(bagcount(nat(), p2, s)), p2);

  data_rewrite_test(R, specification.normalise_sorts(bagjoin(nat(), s, empty)), s);
  data_rewrite_test(R, specification.normalise_sorts(bagjoin(nat(), s1, s2)), s);

  data_rewrite_test(R, specification.normalise_sorts(bagintersect(nat(), s, empty)), empty);
  data_rewrite_test(R, specification.normalise_sorts(bagintersect(nat(), s, s1)), s1);
  data_rewrite_test(R, specification.normalise_sorts(bagintersect(nat(), s, s2)), s2);

  data_rewrite_test(R, specification.normalise_sorts(bagdifference(nat(), s, empty)), s);
  data_rewrite_test(R, specification.normalise_sorts(bagdifference(nat(), s, s1)), s2);
  data_rewrite_test(R, specification.normalise_sorts(bagdifference(nat(), s, s2)), s1);

  // extra tests by Jan Friso
  data::data_expression x = data::parse_data_expression("count(1,{ 1:1,2:2})==1",specification);
  data_rewrite_test(R, x, true_());
  x = data::parse_data_expression("count(2,{ 1:1,2:2})==2",specification);
  data_rewrite_test(R, x, true_());
  x = data::parse_data_expression("count(2,{ 1:1,2:2})==1",specification);
  data_rewrite_test(R, x, false_());
}

void structured_sort_rewrite_test() {
  using namespace sort_bool;
  using namespace sort_nat;

  data_specification specification;

  atermpp::vector< structured_sort_constructor_argument > arguments;

  arguments.push_back(structured_sort_constructor_argument(bool_(), "a0"));
  arguments.push_back(structured_sort_constructor_argument(bool_()));
  arguments.push_back(structured_sort_constructor_argument(sort_nat::nat(), "n0"));
  arguments.push_back(structured_sort_constructor_argument(sort_nat::nat(), "n1"));

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
  // specification.normalise_sorts();

  data::rewriter R(specification);

  data_expression c0(constructors[0].constructor_function(ls));
  data_expression c1(constructors[1].constructor_function(ls));
  data_expression a(constructors[2].constructor_function(ls)(true_()));
  data_expression b(constructors[3].constructor_function(ls)(false_()));
  data_expression n0(nat("0"));
  data_expression n1(nat("1"));
  data_expression c(constructors[4].constructor_function(ls)(n0, n1));

  // recogniser tests
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[1].recogniser_function(ls), c0)), false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[3].recogniser_function(ls), c0)), false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[4].recogniser_function(ls), c0)), false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[1].recogniser_function(ls), c1)), true_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[3].recogniser_function(ls), c1)), false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[4].recogniser_function(ls), c1)), false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[1].recogniser_function(ls), a)),  false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[3].recogniser_function(ls), a)),  false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[4].recogniser_function(ls), a)),  false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[1].recogniser_function(ls), b)),  false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[3].recogniser_function(ls), b)),  true_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[4].recogniser_function(ls), b)),  false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[1].recogniser_function(ls), c)),  false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[3].recogniser_function(ls), c)),  false_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[4].recogniser_function(ls), c)),  true_());

  // projection tests
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[2].projection_functions(ls)[0], a)),  true_());
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[4].projection_functions(ls)[0], c)),  R(n0));
  data_rewrite_test(R, specification.normalise_sorts(make_application(constructors[4].projection_functions(ls)[1], c)),  R(n1));
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT(argc, argv)

//  std::string expr1("struct ");
//  std::string expr1("exists b: Bool, c: Bool. if(b, c, b)");
//  std::string expr1("forall b: Bool, c: Bool. if(b, c, b)");

  bool_rewrite_test();
  core::garbage_collect();

  pos_rewrite_test();
  core::garbage_collect();

  nat_rewrite_test();
  core::garbage_collect();

  int_rewrite_test();
  core::garbage_collect();

  real_rewrite_test();
  core::garbage_collect();

  list_rewrite_test();
  core::garbage_collect();

  set_rewrite_test();
  core::garbage_collect();

  bag_rewrite_test();
  core::garbage_collect();

  structured_sort_rewrite_test();
  core::garbage_collect();

  return 0;
}
