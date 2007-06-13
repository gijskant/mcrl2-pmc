// ======================================================================
//
// Copyright (c) 2004, 2005 Wieger Wesselink
//
// ----------------------------------------------------------------------
//
// file          : test/algorithm.cpp
// date          : 19/09/06
// version       : 1.0
//
// author(s)     : Wieger Wesselink  <J.W.Wesselink@tue.nl>
//
// ======================================================================

#include <iostream>
#include <iterator>
#include <vector>
#include <boost/test/minimal.hpp>

#include "atermpp/atermpp.h"
#include "atermpp/algorithm.h"
#include "mcrl2/data/data.h"

using namespace atermpp;
using namespace lps;

struct add_zero
{
  aterm_appl operator()(aterm_appl t) const
  {
    if (is_data_variable(t))
    {
      data_variable d(t);
      return data_variable(std::string(d.name()) + "0", d.sort());
    }
    else
      return t;
  }
};

/// Function object for comparing a data variable with the
/// the left hand side of a data assignment.
struct compare_lhs
{
  data_variable m_variable;

  compare_lhs(const data_variable& variable)
    : m_variable(variable)
  {}
  
  bool operator()(const data_assignment& a) const
  {
    return m_variable == a.lhs();
  }
};

/// Utility class for applying a list of assignments to a term.
struct data_assignment_list_replacer
{
  const data_assignment_list& l;
  
  data_assignment_list_replacer(const data_assignment_list& l_)
    : l(l_)
  {}
  
  std::pair<aterm_appl, bool> operator()(aterm_appl t) const
  {
    if (!is_data_variable(t))
    {
      return std::make_pair(t, true); // continue the recursion
    }
    data_assignment_list::iterator i = std::find_if(l.begin(), l.end(), compare_lhs(t));
    if (i == l.end())
    {
      return std::make_pair(t, false); // don't continue the recursion
    }
    else
    {
      return std::make_pair(i->rhs(), false); // don't continue the recursion
    }
  }
};

void test_replace()
{
  using namespace lps::data_expr;

  data_variable d("d:D");
  data_variable e("e:D");
  data_variable f("f:D");
  data_variable d0("d0:D");
  data_variable e0("e0:D");
  data_variable f0("f0:D");

  data_expression g = and_(equal_to(d, e), not_equal_to(e, f));

  data_expression h = replace(g, add_zero());
  BOOST_CHECK(h == and_(equal_to(d0, e0), not_equal_to(e0, f0)));

  data_expression i = replace(g, d, e); 
  BOOST_CHECK(i == and_(equal_to(e, e), not_equal_to(e, f)));
}

void test_data_assignment_list()
{
  using namespace lps::data_expr;

  data_variable d1("d1:D");
  data_variable d2("d2:D");
  data_variable d3("d3:D");
  data_variable e1("e1:D");
  data_variable e2("e2:D");
  data_variable e3("e3:D");

  data_assignment_list l;
  l = push_front(l, data_assignment(d1, e1));
  l = push_front(l, data_assignment(e1, e2));
  l = push_front(l, data_assignment(e2, e3));

  data_expression t  = and_(equal_to(d1, e1), not_equal_to(e2, d3));
  data_expression t0 = and_(equal_to(e1, e2), not_equal_to(e3, d3));
  data_expression t1 = partial_replace(t, data_assignment_list_replacer(l));
  data_expression t2 = t.substitute(assignment_list_substitution(l));
  std::cerr << "t  == " << pp(t) << std::endl;
  std::cerr << "t1 == " << pp(t1) << std::endl;
  std::cerr << "t2 == " << pp(t2) << std::endl;
  BOOST_CHECK(t0 == t1);
  BOOST_CHECK(t0 == t2);
}

int test_main( int, char*[] )
{
  ATerm bottom_of_stack;
  ATinit(0, 0, &bottom_of_stack);
  gsEnableConstructorFunctions();

  test_replace();
  test_data_assignment_list();

  return 0;
}
