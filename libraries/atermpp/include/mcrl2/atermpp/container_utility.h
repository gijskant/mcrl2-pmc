// Author(s): Jeroen Keiren, Jeroen van der Wulp
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/atermpp/container_utility.h
/// \brief Provides utilities for working with container.

#ifndef MCRL2_ATERMPP_CONTAINER_UTILITY_H
#define MCRL2_ATERMPP_CONTAINER_UTILITY_H

#include <algorithm>
#include <list>
#include <set>
#include <vector>
#include <type_traits>

#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/aterm_list.h"

namespace atermpp
{

/// \cond INTERNAL_DOCS
namespace detail
{



} // namespace detail

/// \brief Constructs a vector with element type T of one argument.
///
/// \param[in] t1 The first element of the vector.
template <typename T>
std::vector<T> make_vector(const T& t1)
{
  std::vector<T> v;
  v.push_back(t1);
  return v;
}

/// \brief Constructs a vector with element type T of two arguments.
///
/// \param[in] t1 The first element of the vector.
/// \param[in] t2 The second element of the vector.
template <typename T>
std::vector<T> make_vector(const T& t1, const T& t2)
{
  std::vector<T> v;
  v.push_back(t1);
  v.push_back(t2);
  return v;
}

/// \brief Constructs a vector with element type T of three arguments.
///
/// \param[in] t1 The first element of the vector.
/// \param[in] t2 The second element of the vector.
/// \param[in] t3 The third element of the vector.
template <typename T>
std::vector<T> make_vector(const T& t1, const T& t2, const T& t3)
{
  std::vector<T> v;
  v.push_back(t1);
  v.push_back(t2);
  v.push_back(t3);
  return v;
}

/// \brief Constructs a vector with element type T of four arguments.
///
/// \param[in] t1 The first element of the vector.
/// \param[in] t2 The second element of the vector.
/// \param[in] t3 The third element of the vector.
/// \param[in] t4 The fourth element of the vector.
template <typename T>
std::vector<T> make_vector(const T& t1, const T& t2, const T& t3, const T& t4)
{
  std::vector<T> v;
  v.push_back(t1);
  v.push_back(t2);
  v.push_back(t3);
  v.push_back(t4);
  return v;
}

/// \brief Constructs a vector with element type T of four arguments.
///
/// \param[in] t1 The first element of the vector.
/// \param[in] t2 The second element of the vector.
/// \param[in] t3 The third element of the vector.
/// \param[in] t4 The fourth element of the vector.
/// \param[in] t5 The fifth element of the vector.
template <typename T>
std::vector<T> make_vector(const T& t1, const T& t2, const T& t3, const T& t4, const T& t5)
{
  std::vector<T> v;
  v.push_back(t1);
  v.push_back(t2);
  v.push_back(t3);
  v.push_back(t4);
  v.push_back(t5);
  return v;
}

/// \brief Constructs a vector with element type T of four arguments.
///
/// \param[in] t1 The first element of the vector.
/// \param[in] t2 The second element of the vector.
/// \param[in] t3 The third element of the vector.
/// \param[in] t4 The fourth element of the vector.
/// \param[in] t5 The fifth element of the vector.
/// \param[in] t6 The sixth element of the vector.
template <typename T>
std::vector<T> make_vector(const T& t1, const T& t2, const T& t3, const T& t4, const T& t5, const T& t6)
{
  std::vector<T> v;
  v.push_back(t1);
  v.push_back(t2);
  v.push_back(t3);
  v.push_back(t4);
  v.push_back(t5);
  v.push_back(t6);
  return v;
}

/// \brief Constructs a vector with element type T of four arguments.
///
/// \param[in] t1 The first element of the vector.
/// \param[in] t2 The second element of the vector.
/// \param[in] t3 The third element of the vector.
/// \param[in] t4 The fourth element of the vector.
/// \param[in] t5 The fifth element of the vector.
/// \param[in] t6 The sixth element of the vector.
/// \param[in] t7 The seventh element of the vector.
template <typename T>
std::vector<T> make_vector(const T& t1, const T& t2, const T& t3, const T& t4, const T& t5, const T& t6, const T& t7)
{
  std::vector<T> v;
  v.push_back(t1);
  v.push_back(t2);
  v.push_back(t3);
  v.push_back(t4);
  v.push_back(t5);
  v.push_back(t6);
  v.push_back(t7);
  return v;
}

/// \brief Constructs a vector with element type T of four arguments.
///
/// \param[in] t1 The first element of the vector.
/// \param[in] t2 The second element of the vector.
/// \param[in] t3 The third element of the vector.
/// \param[in] t4 The fourth element of the vector.
/// \param[in] t5 The fifth element of the vector.
/// \param[in] t6 The sixth element of the vector.
/// \param[in] t7 The seventh element of the vector.
/// \param[in] t8 The eighth element of the vector.
template <typename T>
std::vector<T> make_vector(const T& t1, const T& t2, const T& t3, const T& t4, const T& t5, const T& t6, const T& t7, const T& t8)
{
  std::vector<T> v;
  v.push_back(t1);
  v.push_back(t2);
  v.push_back(t3);
  v.push_back(t4);
  v.push_back(t5);
  v.push_back(t6);
  v.push_back(t7);
  v.push_back(t8);
  return v;
}

} // namespace atermpp

#endif // MCRL2_ATERMPP_CONTAINER_UTILITY_H
