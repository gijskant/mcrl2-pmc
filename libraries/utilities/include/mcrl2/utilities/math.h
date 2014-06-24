// Author(s): Jan Friso Groote
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file general_utilities.h
/// \brief Some general mathematical functions not provided by standard libraries.

#ifndef MCRL2_MATH_UTILITIES_H
#define MCRL2_MATH_UTILITIES_H

namespace mcrl2
{
namespace utilities
{

// Compute base 2 logarithm of n, by checking which is the leftmost
// bit that has been set.

inline
size_t ceil_log2(size_t n)
{
  assert(n>0);
  size_t result = 0;
  while(n != 0)
  {
    n = n >> 1;
    ++result;
  }
  return result;
}


} // utilities
} // mcrl2


#endif // MCRL2_MATH_UTILITIES_H

