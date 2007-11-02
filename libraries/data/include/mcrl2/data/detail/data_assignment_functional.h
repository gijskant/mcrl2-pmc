// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/detail/data_assignment_functional.h
/// \brief Add your file description here.

#ifndef MCRL2_DATA_DETAIL_DATA_ASSIGNMENT_FUNCTIONAL_H
#define MCRL2_DATA_DETAIL_DATA_ASSIGNMENT_FUNCTIONAL_H

#include <set>
#include "mcrl2/data/data.h"

namespace lps {

namespace detail {

struct has_left_hand_side
{
  const data_variable& m_variable;
  
  has_left_hand_side(const data_variable& variable)
   : m_variable(variable)
  {}
  
  bool operator()(const data_assignment& a) const
  {
    return a.lhs() == m_variable;
  }
};

struct has_left_hand_side_in
{
  const std::set<data_variable>& m_variables;
  
  has_left_hand_side_in(const std::set<data_variable>& variables)
    : m_variables(variables)
  {}
  
  bool operator()(data_assignment a) const
  {
    return m_variables.find(a.lhs()) != m_variables.end();
  }
};

} // namespace detail

} // namespace lps

#endif // MCRL2_DATA_DETAIL_DATA_ASSIGNMENT_FUNCTIONAL_H
