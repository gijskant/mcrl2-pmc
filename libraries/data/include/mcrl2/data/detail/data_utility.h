// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/detail/data_utility.h
/// \brief Add your file description here.

#ifndef MCRL2_DATA_DETAIL_DATA_UTILITY_H
#define MCRL2_DATA_DETAIL_DATA_UTILITY_H

#include <algorithm>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/split.hpp>

#include "mcrl2/data/data.h"

namespace mcrl2 {

namespace data {

namespace detail {

// example: "X(d:D,e:E)"
inline
std::pair<std::string, data_expression_list> parse_variable(std::string s)
{
  using boost::algorithm::split;
  using boost::algorithm::is_any_of;

  std::string name;
  data_expression_list variables;

  std::string::size_type idx = s.find('(');
  if (idx == std::string::npos)
  {
    name = s;
  }
  else
  {
    name = s.substr(0, idx);
    assert(*s.rbegin() == ')');
    std::vector<std::string> v;
    std::string w = s.substr(idx + 1, s.size() - idx - 2);
    split(v, w, is_any_of(","));
    // This doesn't compile in combination with 'using namespace std::rel_ops'
    // for Visual C++ 8.0 (looks like a compiler bug)
    // for (std::vector<std::string>::reverse_iterator i = v.rbegin(); i != v.rend(); ++i)
    // {
    //   data_expression d = data_variable(*i);
    //   variables = push_front(variables, d);
    // }
    for (std::vector<std::string>::iterator i = v.begin(); i != v.end(); ++i)
    {
      data_expression d = data_variable(*i);
      variables = push_front(variables, d);
    }
  }
  return std::make_pair(name, atermpp::reverse(variables));
}

/// Returns true if the names of the given variables are unique.
inline
bool unique_names(data_variable_list variables)
{
  std::set<core::identifier_string> variable_names;
  for (data_variable_list::iterator i = variables.begin(); i != variables.end(); ++i)
  {
    variable_names.insert(i->name());
  }
  if (variable_names.size() != variables.size())
  {
    return false;
  }
  return true;
}

/// Returns true if the left hand sides of assignments are contained in variables.
inline
bool check_assignment_variables(data_assignment_list assignments, data_variable_list variables)
{
  std::set<data_variable> v;
  std::copy(variables.begin(), variables.end(), std::inserter(v, v.begin()));
  for (data_assignment_list::iterator i = assignments.begin(); i != assignments.end(); ++i)
  {
    if (v.find(i->lhs()) == v.end())
      return false;
  }
  return true;
}

/// Function object for checking if a term is a constant sort.
struct is_constant_sort
{
  bool operator()(aterm_appl t) const
  {
    return is_sort_expression(t) && !sort_expression(t).is_arrow();
  }
};

/// Function object for checking if an operator is a constant sort.
struct is_constant_operation
{
  bool operator()(aterm_appl t) const
  { data_operation op=t;
    return is_sort_expression(op.sort()) && !sort_expression(op.sort()).is_arrow();
  }
};

/// Function object for checking if an operator is a not a constant sort.
struct is_not_a_constant_operation
{
  bool operator()(aterm_appl t) const
  { data_operation op=t;
    return is_sort_expression(op.sort()) && sort_expression(op.sort()).is_arrow();
  }
};

/// Returns true if the domain sorts and the range sort of the given sort s are
/// contained in sorts.

inline bool check_sort(sort_expression s, const std::set<sort_expression>& sorts)
{
  std::set<sort_expression> s_sorts;
  atermpp::find_all_if(s, is_constant_sort(), std::inserter(s_sorts, s_sorts.begin()));
  return std::includes(sorts.begin(), sorts.end(), s_sorts.begin(), s_sorts.end());
}

/// Returns true if the domain sorts and the range sort of the sorts in the sequence [first, last[
/// are contained in sorts.
template <typename Iterator>
bool check_sorts(Iterator first, Iterator last, const std::set<sort_expression>& sorts)
{
  for (Iterator i = first; i != last; ++i)
  {
    if (!check_sort(*i, sorts))
      return false;
  }
  return true;
}

/// Returns true if the domain sorts and the range sort of the given variables are contained
/// in sorts.
template <typename VariableContainer>
bool check_variable_sorts(const VariableContainer& variables, const std::set<sort_expression>& sorts)
{
  for (typename VariableContainer::const_iterator i = variables.begin(); i != variables.end(); ++i)
  {
    if (!check_sort(i->sort(), sorts))
      return false;
  }
  return true;
}

/// Returns true if names of the given variables are not contained in names.
inline
bool check_variable_names(data_variable_list variables, const std::set<core::identifier_string>& names)
{
  for (data_variable_list::iterator i = variables.begin(); i != variables.end(); ++i)
  {
    if (names.find(i->name()) != names.end())
      return false;
  }
  return true;
}

/// Returns true if the domain sorts and range sort of the given functions are
/// contained in sorts.
inline
bool check_data_spec_sorts(data_operation_list functions, const std::set<sort_expression>& sorts)
{
  for (data_operation_list::iterator i = functions.begin(); i != functions.end(); ++i)
  {
    if (!check_sort(i->sort(), sorts))
      return false;
  }
  return true;
}

/// \brief Returns the names of the variables in t
inline
std::vector<std::string> variable_strings(data_variable_list t)
{
  std::vector<std::string> result;
  for (data_variable_list::iterator i = t.begin(); i != t.end(); ++i)
    result.push_back(i->name());
  return result;
}

} // namespace detail

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_DETAIL_DATA_UTILITY_H
