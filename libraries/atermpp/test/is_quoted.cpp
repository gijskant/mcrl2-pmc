// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file is_quoted.cpp
/// \brief Add your file description here.

// ======================================================================
//
// Copyright (c) 2004, 2005 Wieger Wesselink
//
// ----------------------------------------------------------------------
//
// file          : test/is_quoted.cpp
// date          : 04/25/05
// version       : 0.3
//
// author(s)     : Wieger Wesselink  <J.W.Wesselink@tue.nl>
//
// ======================================================================

#include <iostream>
#include <string>
#include <boost/test/minimal.hpp>

#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_appl.h"

using namespace std;
using namespace atermpp;

int test_main( int, char*[] )
{
  ATERM_LIBRARY_INIT()
  // unquoted function symbol s == "f"
  function_symbol s("\"f\"", 1, false);
  aterm_appl f(s, aterm("x"));

  // convert to string and back
  aterm_appl g = make_term(f.to_string());

  BOOST_CHECK(g.is_quoted());
  BOOST_CHECK(g.function() != s);
  function_symbol s1("f", 1, true);
  BOOST_CHECK(g.function() == s1);

  return 0;
}
