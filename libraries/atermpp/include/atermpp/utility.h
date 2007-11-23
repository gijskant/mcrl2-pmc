// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file atermpp/utility.h
/// \brief Add your file description here.

// ======================================================================
//
// Copyright (c) 2006 Wieger Wesselink
//
// ----------------------------------------------------------------------
//
// file          : atermpp/utility.h
// date          : 15-12-2006
// version       : 1.0
//
// author(s)     : Wieger Wesselink  <J.W.Wesselink@tue.nl>
//
// ======================================================================

#ifndef ATERM_UTILITY_H
#define ATERM_UTILITY_H

#include <string>
#include "atermpp/aterm_string.h"

namespace atermpp
{

  /// Remove leading and trailing quotes from a quoted aterm_string.
  inline
  std::string unquote(aterm_string t)
  {
    std::string s(t);
    assert(s.size() >= 2 && *s.begin() == '"' && *s.rbegin() == '"');
    return std::string(++s.begin(), --s.end());
  }

} // namespace atermpp

#endif // ATERM_UTILITY_H
