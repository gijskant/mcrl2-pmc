// Author(s): Wieger Wesselink
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
#include "mcrl2/atermpp/make_list.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/parser.h"

using namespace std;
using namespace atermpp;
using namespace mcrl2::data;
using namespace mcrl2::lps;
using namespace mcrl2::lps::detail;
using namespace mcrl2::modal;
using namespace mcrl2::modal::detail;
using namespace mcrl2::pbes_system;
using namespace mcrl2::pbes_system::detail;

std::string EXPRESSIONS =
"variables                                \n"
"                                         \n"
"expressions                              \n"
"nu X. nu Y. nu Z. X && Y && Z            \n"
"                                         \n"
"nu Y. X || Y                             \n"
;

void print(atermpp::set<pbes_expression> q)
{
  for (atermpp::set<pbes_expression>::iterator i = q.begin(); i != q.end(); ++i)
  {
    std::cout << pp(*i) << std::endl;
  }
}

void test_accessors()
{
  using namespace accessors;

  std::vector<pbes_expression> expressions = parse_pbes_expressions(EXPRESSIONS).first;
  pbes_expression x = expressions[0];
  pbes_expression y = expressions[1];
  data_variable d(core::identifier_string("d"), sort_expr::nat());
  data_variable_list v = make_list(d);
  pbes_expression z = pbes_expr::val(d); 
  propositional_variable_instantiation X(identifier_string("X"), make_list(d));

  atermpp::set<pbes_expression> q;
  q.insert(x);
  q.insert(y);
  q.insert(z);

  {
    using namespace pbes_expr;

    pbes_expression a, b, c;
    data_variable_list w;
    identifier_string s;
    data_expression e;
    atermpp::set<pbes_expression> q1;

    e = val_arg(z);
    
    a = not_(x);
    b = not_arg(a);
    BOOST_CHECK(x == b);

    a = and_(x, y);
    b = lhs(a);
    c = rhs(a);
    BOOST_CHECK(x == b);
    BOOST_CHECK(y == c);

    a = or_(x, y);
    b = lhs(a);
    c = rhs(a);
    BOOST_CHECK(x == b);
    BOOST_CHECK(y == c);

    a = imp(x, y);
    b = lhs(a);
    c = rhs(a);
    BOOST_CHECK(x == b);
    BOOST_CHECK(y == c);

    a = forall(v, x);
    w = quant_vars(a);
    b = quant_expr(a);
    BOOST_CHECK(v == w);
    BOOST_CHECK(x == b);
   
    a = exists(v, x);
    w = quant_vars(a);
    b = quant_expr(a);
    BOOST_CHECK(v == w);
    BOOST_CHECK(x == b);

    s = var_name(X);
    BOOST_CHECK(s == identifier_string("X"));
    
    data_expression_list f = var_val(X);
    data_expression_list g = make_list(d);
    BOOST_CHECK(f == g);

    print(q);

    a = join_or(q.begin(), q.end());
    q1 = split_or(a);    
    BOOST_CHECK(q == q1);

    print(q1);

    a = join_and(q.begin(), q.end());
    q1 = split_and(a);
    BOOST_CHECK(q == q1);

    print(q1);
  } 

  {
    using namespace pbes_expr_optimized;

    pbes_expression a, b, c;
    data_variable_list w;
    identifier_string s;
    data_expression e;
    atermpp::set<pbes_expression> q1;

    e = val_arg(z);
    
    a = not_(x);
    a = and_(x, y);
    a = or_(x, y);
    a = imp(x, y);
    a = forall(v, x);
    a = exists(v, x);
    s = var_name(X);
    data_expression_list f = var_val(X);
    a = join_or(q.begin(), q.end());
    a = join_and(q.begin(), q.end());
    q1 = split_or(a);
    q1 = split_and(a);
  } 
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERM_INIT(argc, argv)

  test_accessors();

  return 0;
}
