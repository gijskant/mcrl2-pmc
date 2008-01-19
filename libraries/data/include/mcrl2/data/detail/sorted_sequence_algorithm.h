// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file sorted_sequence_algorithm.h
/// \brief Add your file description here.

#ifndef MCRL2_DATA_DETAIL_SORTED_SEQUENCE_ALGORITHM_H
#define MCRL2_DATA_DETAIL_SORTED_SEQUENCE_ALGORITHM_H

#include <algorithm>
#include <iterator>
#include <set>

namespace mcrl2 {

namespace data {

namespace detail {

template <typename T>
std::set<T> set_union(const std::set<T>& x, const std::set<T>& y)
{
  std::set<T> result;
  std::set_union(x.begin(), x.end(), y.begin(), y.end(), std::inserter(result, result.begin()));
  return result;
}

template <typename T>
std::set<T> set_difference(const std::set<T>& x, const std::set<T>& y)
{
  std::set<T> result;
  std::set_difference(x.begin(), x.end(), y.begin(), y.end(), std::inserter(result, result.begin()));
  return result;
}

template <typename T>
std::set<T> set_intersection(const std::set<T>& x, const std::set<T>& y)
{
  std::set<T> result;
  std::set_intersection(x.begin(), x.end(), y.begin(), y.end(), std::inserter(result, result.begin()));
  return result;
}

} // namespace detail

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_SORTED_DETAIL_SEQUENCE_ALGORITHM_H
