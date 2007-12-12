// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file table_test.cpp
/// \brief Add your file description here.

#include <iostream>
#include <string>
#include <boost/test/minimal.hpp>

#include <map>
#include "atermpp/atermpp.h"
#include "atermpp/indexed_set.h"

using namespace std;
using namespace atermpp;

void test_indexed_set()
{
  indexed_set t(100, 75);

  std::pair<long, bool> p;
  p = t.put(make_term("a"));
  BOOST_CHECK(t.elements().size() == 1);
  p = t.put(make_term("b"));
  BOOST_CHECK(t.elements().size() == 2);

  {
    indexed_set t1 = t;
  }
  indexed_set t2 = t;

  BOOST_CHECK(t.index(make_term("a")) == 0);
  BOOST_CHECK(t.index(make_term("b")) == 1);

  aterm a = t.get(0);
  BOOST_CHECK(a == make_term("a"));

  aterm b = t.get(1);
  BOOST_CHECK(b == make_term("b"));
  
  t.remove(a);
  BOOST_CHECK(t.elements().size() == 1);

  p = t.put(make_term("c"));
  BOOST_CHECK(t.elements().size() == 2);
  BOOST_CHECK(p.first == 0);

  t.reset();
  BOOST_CHECK(t.elements().size() == 0);
  
  std::map<int, indexed_set> x;
  x[2] = t;
}

int test_main( int, char*[] )
{
  ATERM_LIBRARY_INIT()
  test_indexed_set();
  return 0;
}
