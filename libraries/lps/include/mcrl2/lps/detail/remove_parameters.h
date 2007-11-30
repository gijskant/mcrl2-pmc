// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file remove_parameters.h
/// \brief Add your file description here.

#ifndef MCRL2_LPS_DETAIL_REMOVE_PARAMETERS_H
#define MCRL2_LPS_DETAIL_REMOVE_PARAMETERS_H

#include <algorithm>
#include <set>
#include <vector>
#include "mcrl2/lps/specification.h"

namespace lps {

namespace detail {

/// Removes the parameters in to_be_removed from l.
inline
data_assignment_list remove_parameters(const data_assignment_list& l, const std::set<data_variable>& to_be_removed)
{
  std::vector<data_assignment> a(l.begin(), l.end());
  a.erase(std::remove_if(a.begin(), a.end(), detail::has_left_hand_side_in(to_be_removed)), a.end());
  return data_assignment_list(a.begin(), a.end()); 
}

/// Removes the parameters in to_be_removed from p.
inline
linear_process remove_parameters(const linear_process& p, const std::set<data_variable>& to_be_removed)
{
  std::vector<data_variable> v(p.process_parameters().begin(), p.process_parameters().end());
  std::vector<summand> s(p.summands().begin(), p.summands().end());

  for (std::set<data_variable>::const_iterator i = to_be_removed.begin(); i != to_be_removed.end(); ++i)
  {
    v.erase(std::remove(v.begin(), v.end(), *i), v.end());
  }

  for (std::vector<summand>::iterator i = s.begin(); i != s.end(); ++i)
  {
    *i = set_assignments(*i, remove_parameters(i->assignments(), to_be_removed));
  }
  
  data_variable_list new_process_parameters(v.begin(), v.end());
  summand_list new_summands(s.begin(), s.end());
  linear_process result = set_process_parameters(p, new_process_parameters);
  result = set_summands(result, new_summands);

  return result;
}

/// Removes the parameters in to_be_removed from spec.
inline
specification remove_parameters(const specification& spec, const std::set<data_variable>& to_be_removed)
{
  process_initializer new_initial_state(spec.initial_process().free_variables(),
                                        remove_parameters(spec.initial_process().assignments(), to_be_removed)
                                       );
  linear_process p = remove_parameters(spec.process(), to_be_removed);
  specification result = set_lps(spec, p);
  result = set_initial_process(result, new_initial_state);
  return result;
}

} // namespace detail

} // namespace lps

#endif // MCRL2_LPS_DETAIL_REMOVE_PARAMETERS_H
