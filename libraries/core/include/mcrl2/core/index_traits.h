// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/core/index_traits.h
/// \brief add your file description here.

#ifndef MCRL2_CORE_INDEX_TRAITS_H
#define MCRL2_CORE_INDEX_TRAITS_H

#include <iostream>
#include <stack>
#include <sstream>
#include <stdexcept>
#include "mcrl2/core/identifier_string.h"

#ifdef MCRL2_INDEX_TRAITS_USE_UNORDERED_MAP
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <utility>
#include "mcrl2/atermpp/aterm.h"
#else
#include <map>
#endif

namespace mcrl2 {

namespace core {

#ifdef MCRL2_INDEX_TRAITS_USE_UNORDERED_MAP

template <typename Variable, typename KeyType>
std::unordered_map<KeyType, std::size_t>& variable_index_map()
{
  static std::unordered_map<KeyType, std::size_t> m;
  return m;
}

inline
std::size_t hash_value(const atermpp::aterm& x)
{
  return atermpp::detail::hash_number(atermpp::detail::address(x));
}

inline
std::size_t hash_value(const atermpp::aterm& x1, const atermpp::aterm& x2)
{
  return boost::hash_value(std::make_pair(atermpp::detail::hash_number(atermpp::detail::address(x1)), atermpp::detail::hash_number(atermpp::detail::address(x2))));
}

#else

template <typename Variable, typename KeyType>
std::map<KeyType, std::size_t>& variable_index_map()
{
  static std::map<KeyType, std::size_t> m;
  return m;
}

#endif // MCRL2_INDEX_TRAITS_USE_UNORDERED_MAP

template <typename Variable, typename KeyType>
std::stack<std::size_t>& variable_map_free_numbers()
{
  static std::stack<std::size_t> s;
  return s;
}

template <typename Variable, typename KeyType>
std::size_t& variable_map_max_index()
{
  static std::size_t s;
  return s;
}

/// \brief For several expressions in mCRL2 an implicit mapping of these expressions
/// to integers is available. This is done for efficiency reasons. Examples are:
///
/// data::variable, process::process_identifier
///
/// The class index_traits is used to implement this mapping. By this, the public
/// interface of the expression classes is not polluted with.
template <typename Variable, typename KeyType>
struct index_traits
{
  /// \brief Returns the index of the variable.
  static inline
  std::size_t index(const Variable& x)
  {
#ifdef MCRL2_VARIABLES_WITH_INDEX
    // N.B. We assume that the index is the last element of the aterm_appl x.
    const atermpp::aterm_int& i = atermpp::aterm_cast<const atermpp::aterm_int>(x[x.size() - 1]);
    return i.value();
#else
    return 0;
#endif
  }

  /// \brief Returns the index of the key.
  static inline
  std::size_t key_index(const KeyType& x)
  {
    auto& m = variable_index_map<Variable, KeyType>();
    auto i = m.find(x);
    if (i == m.end())
    {
      std::ostringstream out;
      out << "error: could not find element";
      throw std::runtime_error(out.str());
    }
    return i->second;
  }

  /// \brief Returns an upper bound for the largest index of a variable that is currently in use.
  static inline
  std::size_t max_index()
  {
    return variable_map_max_index<Variable, KeyType>();
  }

  /// \brief Note: intended for internal use only!
  /// Returns the index of the variable. If the variable was not already in the map, it is added.
  static inline
  std::size_t insert(const KeyType& x)
  {
    auto& m = variable_index_map<Variable, KeyType>();
    auto i = m.find(x);
    if (i == m.end())
    {
      auto& s = variable_map_free_numbers<Variable, KeyType>();
      std::size_t value;
      if (s.empty())
      {
        value = m.size();
        variable_map_max_index<Variable, KeyType>() = value;
      }
      else
      {
        value = s.top();
        s.pop();
      }
      m[x] = value;
      return value;
    }
    return i->second;
  }

  /// \brief Note: intended for internal use only!
  /// Removes the variable from the index map.
  static inline
  void erase(const KeyType& x)
  {
    auto& m = variable_index_map<Variable, KeyType>();
    auto& s = variable_map_free_numbers<Variable, KeyType>();
    auto i = m.find(x);
    assert(i != m.end());
    s.push(i->second);
    m.erase(i);
  }

  /// \brief Note: intended for internal use only!
  /// Removes the variable from the index map.
  static inline
  std::size_t size()
  {
    auto& m = variable_index_map<Variable, KeyType>();
    return m.size();
  }
};

} // namespace core

} // namespace mcrl2

#ifdef MCRL2_INDEX_TRAITS_USE_UNORDERED_MAP

namespace std {

template<>
struct hash<mcrl2::core::identifier_string>
{
  std::size_t operator()(const mcrl2::core::identifier_string& x) const
  {
    return mcrl2::core::hash_value(x);
  }
};

}

#endif

#endif // MCRL2_CORE_INDEX_TRAITS_H
