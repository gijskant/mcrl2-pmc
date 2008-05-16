// Author(s): Aad Mathijssen
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file include/mcrl2/utilities/numeric_string.h
/// \brief Add your file description here.

#ifndef __NUMERIC_STRING_FUNCTIONS_H__
#define __NUMERIC_STRING_FUNCTIONS_H__

#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <stdarg.h>
#include <assert.h>
#include <aterm2.h>

#include <workarounds.h>

#ifdef __cplusplus
namespace mcrl2 {
  namespace utilities {
    extern "C" {
#endif

//String representations of numbers
//---------------------------------

/**
 * \brief Divides by two in decimal string representation
 * \param[in] n an arbitrary integer in decimal representation
 * \pre n is of the form "0 | [1-9][0-9]*"
 * \return the smallest string representation of n div 2
 * \note The result is created with malloc, so it has to be freed
 **/
char *gsStringDiv2(const char *n);
     
/**
 * \brief Computes remainder of division by two in decimal string representation
 * \param[in] n an arbitrary integer in decimal representation
 * \pre n is of the form "0 | [1-9][0-9]*"
 * \return  the value of n mod 2
 **/
int gsStringMod2(const char *n);

/**
 * \brief Computes modulo 2 in decimal string representation
 * \param[in] n a string that represents a number
 * \param[in] inc a displacement (+0 or +1)
 * \pre n is of the form "0 | [1-9][0-9]*" and 0 <= inc <= 1
 * \return the smallest string representation of 2*n + inc,
 * \note The result is created with malloc, so it has to be freed
 **/
char *gsStringDub(const char *n, const int inc);

/**
 * \brief Computes number of characters of the decimal representation
 * \param[in] n an integer number
 * \return the number of characters of the decimal representation of n
 **/
int NrOfChars(const int n);

#ifdef __cplusplus
    }
  }
}
#endif

#endif
