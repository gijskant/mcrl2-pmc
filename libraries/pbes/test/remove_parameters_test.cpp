// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file remove_parameters.cpp
/// \brief Tests for removing parameters..

#include <iostream>
#include <boost/test/minimal.hpp>
#include "mcrl2/data/variable.h"
#include "mcrl2/data/data_expression.h"
#include "mcrl2/atermpp/make_list.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/remove_parameters.h"
#include "mcrl2/pbes/detail/test_utility.h"
#include "mcrl2/atermpp/container_utility.h"
#include "mcrl2/core/garbage_collection.h"
#include "mcrl2/atermpp/aterm_init.h"

using namespace mcrl2::core;
using namespace atermpp;
using namespace mcrl2;
using namespace mcrl2::data;
using namespace mcrl2::pbes_system;
using namespace mcrl2::pbes_system::detail;

void test_propositional_variable()
{
  variable_list d = make_list(nat("n"), pos("p"), bool_("b"), bool_("c"));
  propositional_variable X = propvar("X", d);
  std::vector<size_t> v;
  v.push_back(1);
  v.push_back(3);
  propositional_variable X1 = remove_parameters(X, v);
  variable_list d1 = make_list(nat("n"), bool_("b"));
  BOOST_CHECK(X1 = propvar("X", d1));
}

void test_propositional_variable_instantiation()
{
  data_expression_list d = make_list< data_expression >(nat("n"), pos("p"), bool_("b"), bool_("c"));
  propositional_variable_instantiation X = propvarinst("X", d);
  std::vector<size_t> v;
  v.push_back(1);
  v.push_back(3);
  propositional_variable_instantiation X1 = remove_parameters(X, v);
  data_expression_list d1 = make_list< data_expression >(nat("n"), bool_("b"));
  BOOST_CHECK(X1 = propvarinst("X", d1));
}

void test_pbes_expression()
{
  variable_list d1 = make_list(nat("m"), bool_("b"));
  variable_list d2 = make_list(nat("m"), bool_("b"), nat("p"));
  propositional_variable X1 = propvar("X1", d1);
  propositional_variable X2 = propvar("X2", d2);

  data_expression_list e1 = make_list< data_expression >(sort_nat::plus(nat("m"), nat("n")), bool_("b"));
  data_expression_list e2 = make_list< data_expression >(sort_nat::times(nat("m"), nat("n")), bool_("b"), nat("p"));
  propositional_variable_instantiation x1 = propvarinst("X1", e1);
  propositional_variable_instantiation x2 = propvarinst("X2", e2);

  pbes_expression p = pbes_expr::and_(X1, X2);

  std::map<identifier_string, std::vector<size_t> > to_be_removed;
  std::vector<size_t> v1;
  v1.push_back(1);
  to_be_removed[X1.name()] = v1;
  std::vector<size_t> v2;
  v2.push_back(0);
  v2.push_back(2);
  to_be_removed[X2.name()] = v2;

  pbes_expression q = remove_parameters(p, to_be_removed);

  pbes_expression r;
  {
    variable_list d1 = make_list(nat("m"));
    variable_list d2 = make_list(bool_("b"));
    propositional_variable X1 = propvar("X1", d1);
    propositional_variable X2 = propvar("X2", d2);

    data_expression_list e1 = make_list< data_expression >(sort_nat::plus(nat("m"), nat("n")));
    data_expression_list e2 = make_list< data_expression >(bool_("b"));
    propositional_variable_instantiation x1 = propvarinst("X1", e1);
    propositional_variable_instantiation x2 = propvarinst("X2", e2);

    r = pbes_expr::and_(X1, X2);
  }
  BOOST_CHECK(q == r);
}

int test_main(int argc, char** argv)
{
  MCRL2_ATERMPP_INIT_DEBUG(argc, argv)

  test_propositional_variable_instantiation();
  core::garbage_collect();

  return 0;
}
