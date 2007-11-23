// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/parelm.h
/// \brief Add your file description here.

#ifndef MCRL2_LPS_PARELM_H
#define MCRL2_LPS_PARELM_H

#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include "atermpp/algorithm.h"
#include "mcrl2/data/utility.h"
#include "mcrl2/data/detail/data_assignment_functional.h"
#include "mcrl2/lps/mcrl22lps.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/detail/sorted_sequence_algorithm.h"

namespace lps {

/// returns the data variables that appear in the condition, action or time of
/// the summands in the sequence [first, last[
template <typename Iterator>
std::set<data_variable> transition_variables(Iterator first, Iterator last)
{
  std::set<data_variable> result;
  for (Iterator i = first; i != last; ++i)
  {
    if (i->is_delta())
    {
      continue;
    }
    atermpp::find_all_if(i->condition(), is_data_variable, std::inserter(result, result.end()));   
    atermpp::find_all_if(i->actions(), is_data_variable, std::inserter(result, result.end()));   
    if (i->has_time())
    {
      atermpp::find_all_if(i->time(), is_data_variable, std::inserter(result, result.end()));
    }
  }
  return result;
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
    std::vector<data_assignment> a(i->assignments().begin(), i->assignments().end());
    a.erase(std::remove_if(a.begin(), a.end(), detail::has_left_hand_side_in(to_be_removed)), a.end());
    *i = set_assignments(*i, data_assignment_list(a.begin(), a.end()));
  }
  
  data_variable_list new_process_parameters(v.begin(), v.end());
  summand_list new_summands(s.begin(), s.end());
  linear_process result = set_process_parameters(p, new_process_parameters);
  result = set_summands(result, new_summands);
  return result;
}

inline
linear_process parelm(const linear_process& p)
{
  std::set<data_variable> process_parameters(p.process_parameters().begin(), p.process_parameters().end());

  // significant variables may not be removed by parelm
  std::set<data_variable> significant_variables = transition_variables(p.summands().begin(), p.summands().end());

  // recursively extend the set of significant variables
  std::set<data_variable> todo = significant_variables;
  while (!todo.empty())
  {
    data_variable x = *todo.begin();
    todo.erase(todo.begin());

    for (summand_list::iterator i = p.summands().begin(); i != p.summands().end(); ++i)
    {
      if (!i->is_delta())
      {
        data_assignment_list::iterator j = std::find_if(i->assignments().begin(), i->assignments().end(), detail::has_left_hand_side(x));
        if (j != i->assignments().end())
        {
          std::set<data_variable> new_variables = detail::set_difference(find_variables(j->rhs()), significant_variables);
          todo.insert(new_variables.begin(), new_variables.end());
          significant_variables.insert(new_variables.begin(), new_variables.end());
        }
      }
    }
  }

  std::set<data_variable> to_be_removed = detail::set_difference(process_parameters, significant_variables);
  return remove_parameters(p, to_be_removed);
}

} // namespace lps

#endif // MCRL2_LPS_PARELM_H
