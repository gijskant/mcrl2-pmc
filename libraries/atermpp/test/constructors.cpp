// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file constructors.cpp
/// \brief Add your file description here.

#include <iostream>
#include <boost/test/minimal.hpp>
#include "mcrl2/atermpp/atermpp.h"
#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/atermpp/aterm_real.h"
#include "mcrl2/atermpp/aterm_string.h"

using namespace std;
using namespace atermpp;

int test_main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)
  aterm_int i(10);
  aterm_real r(2.5);
  aterm_appl f(function_symbol("f", 2), aterm("x"), aterm("y"));
  BOOST_CHECK(f.to_string() == "f(x,y)");
  aterm_appl f1(function_symbol("f", 0));
  BOOST_CHECK(f1.to_string() == "f");

  atermpp::aterm_string s("s"); // g++ 3.4.4 complains if atermpp:: is removed :-(
  // aterm_string s1("s1(x)"); will generate an error!

  return 0;
}
