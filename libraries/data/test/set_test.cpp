// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file set_test.cpp
/// \brief Basic regression test for set expressions.

#include <boost/test/minimal.hpp>

#include "mcrl2/data/standard.h"
#include "mcrl2/data/set.h"
#include "mcrl2/data/fset.h"
#include "mcrl2/data/parse.h"
#include "mcrl2/data/rewriter.h"


using namespace mcrl2;
using namespace mcrl2::data;
using namespace mcrl2::data::sort_set;
using namespace mcrl2::data::sort_fset;

template <typename Predicate>
void test_data_expression(const std::string& s, const variable_vector& v, Predicate p)
{
  std::cerr << "testing data expression " << s << std::endl;
  data_expression e = parse_data_expression(s, v);
  BOOST_CHECK(p(e));
}

/* Test case for various set expressions, based
   on the following specification:

proc P(s: Set(Nat)) = (1 in s) -> tau . P({} + s - {20} * {40})
                    + ({10} < s) -> tau . P(!s)
                    + (s <= {20} + Bag2Set({20:4, 30:3, 40:2})) -> tau . P(s)
                    + (s <= { n:Nat | true }) -> tau . P(s);

init P({20, 30, 40});

*/
void set_expression_test()
{
  data::data_specification specification;

  specification.add_context_sort(sort_pos::pos());
  specification.add_context_sort(sort_set::set_(sort_pos::pos()));
  specification.add_context_sort(sort_set::set_(sort_bool::bool_()));

  data::rewriter normaliser(specification);

  variable_vector v;
  v.push_back(parse_variable("s:Set(Nat)"));

  test_data_expression("{x : Nat | x < 10}", v, sort_set::is_constructor_application);
  test_data_expression("!s", v, sort_set::is_complement_application);
  test_data_expression("s * {}", v, sort_set::is_intersection_application);
  test_data_expression("s * {1,2,3}", v, sort_set::is_intersection_application);
  test_data_expression("s - {3,1,2}", v, sort_set::is_difference_application);
  test_data_expression("1 in s", v, sort_set::is_in_application);
  test_data_expression("{} + s", v, sort_set::is_union_application);
  test_data_expression("(({} + s) - {20}) * {40}", v, sort_set::is_intersection_application);
  test_data_expression("{10} < s", v, is_less_application<data_expression>);
  test_data_expression("s <= {10}", v, is_less_equal_application<data_expression>);
  test_data_expression("{20} + {30}", v, sort_set::is_union_application);

  data_expression t1d1 = parse_data_expression("{1,2}");
  data_expression t1d2 = parse_data_expression("{2,1}");
  BOOST_CHECK(normaliser(t1d1) == normaliser(t1d2));

  data_expression t1d1a = parse_data_expression("{1,2,3}");
  data_expression t1d2a = parse_data_expression("{2,1}");
  BOOST_CHECK(normaliser(t1d1a) != normaliser(t1d2a));

  data_expression t2d1 = parse_data_expression("{1,2} == {1,2}");
  data_expression t2d2 = parse_data_expression("true");
  BOOST_CHECK(normaliser(t2d1) == normaliser(t2d2));

  data_expression t2d1a = parse_data_expression("{1,2,3} == {1,2,1}");
  data_expression t2d2a = parse_data_expression("false");
  BOOST_CHECK(normaliser(t2d1a) == normaliser(t2d2a));

  data_expression t3d1 = parse_data_expression("({1,2} != {2,3})");
  data_expression t3d2 = parse_data_expression("true");
  BOOST_CHECK(normaliser(t3d1) == normaliser(t3d2));

  data_expression t4d1 = parse_data_expression("(!{1,2}) == {1,2}");
  data_expression t4d2 = parse_data_expression("false");
  BOOST_CHECK(normaliser(t4d1) == normaliser(t4d2));

  data_expression t5d1 = parse_data_expression("(!!{1,2}) == {2,1}");
  data_expression t5d2 = parse_data_expression("true");
  BOOST_CHECK(normaliser(t5d1) == normaliser(t5d2));


  data_expression e = parse_data_expression("{20}", v);
  BOOST_CHECK(sort_fset::is_cons_application(normaliser(e)));

  e = parse_data_expression("{20, 30, 40}", v);
  BOOST_CHECK(sort_fset::is_cons_application(normaliser(e)));

  data_expression t6d1 = parse_data_expression("{} == { b: Bool | true } - { true, false }");
  data_expression t6d2 = parse_data_expression("true");

  BOOST_CHECK(normaliser(t6d1) == normaliser(t6d2));

  data_expression t7d1 = parse_data_expression("{ b: Bool | true } - { true, false } == {}");
  data_expression t7d2 = parse_data_expression("true");
  BOOST_CHECK(normaliser(t7d1) == normaliser(t7d2));

}

int test_main(int argc, char** argv)
{
  set_expression_test();

  return EXIT_SUCCESS;
}

