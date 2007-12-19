// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file make_match_test.cpp
/// \brief Add your file description here.

// ======================================================================
//
// Copyright (c) 2004, 2005 Wieger Wesselink
//
// ----------------------------------------------------------------------
//
// file          : test/make_match_test.cpp
// date          : 04/25/05
// version       : 0.3
//
// author(s)     : Wieger Wesselink  <J.W.Wesselink@tue.nl>
//
// ======================================================================

#include <iostream>
#include <boost/test/minimal.hpp>

#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_list.h"

using namespace std;
using namespace atermpp;

void test_make_match()
{
  aterm a, b;
  aterm_list l, m;
  int i;
  bool x;

  x = match(make_term("f(16)"), "f(<int>)", i);
  BOOST_CHECK(x);
  BOOST_CHECK(i == 16); 

  x = match(make_term("[1,2,3]"), "[<int>,<list>]", i, a);

  BOOST_CHECK(x);
  BOOST_CHECK(i == 1); 
  BOOST_CHECK(aterm_list(a).size() == 2);

  x = match(make_term("[1,2,3]"), "[<list>]", l);

  x = match(make_term("f([1,2,3])"), "f([<list>])", l);

  x = match(make_term("f(2,[1,2,3])"), "f(<term>,[<list>])", a, l);
  
  x = match(make_term("PBES(f(x),[1,2,3])"), "PBES(<term>,[<list>])", a, l);
  
  x = match(make_term("PBES(f(0),[1,2,3],g(2),[a,b])"), "PBES(<term>,[<list>],<term>,[<list>])", a, l, b, m);
}

int test_main( int, char*[] )
{
  ATERM_LIBRARY_INIT()
  test_make_match();
  return 0;
}
