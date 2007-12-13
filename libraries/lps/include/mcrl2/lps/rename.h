// Author(s): Wieger Wesselink and Jan Friso Groote
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/rename.h
/// \brief Add your file description here.

#ifndef MCRL2_LPS_RENAME_H
#define MCRL2_LPS_RENAME_H

#include <set>
#include <vector>
#include <utility>
#include <boost/iterator/transform_iterator.hpp>
#include "atermpp/make_list.h"
#include "atermpp/algorithm.h"
#include "atermpp/vector.h"
#include "mcrl2/data/detail/data_functional.h"
#include "mcrl2/data/postfix_identifier_generator.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/detail/algorithm.h"
#include "mcrl2/lps/detail/linear_process_utility.h"
#include "mcrl2/lps/detail/sorted_sequence_algorithm.h"

namespace lps {

/// \internal
/// Generates a renaming of process parameters of the process p. The parameters are
/// renamed according to the given identifier generator, and well typedness constraints
/// are taken into account. The result consists of two vectors (src, dest) that specify
/// the renaming src[i] := dest[i].
///
template <typename IdentifierGenerator>
std::pair<std::vector<data_variable>, std::vector<data_variable> >
rename_process_parameters_helper(const linear_process& p, IdentifierGenerator& generator)
{
  std::set<identifier_string> forbidden_names = detail::set_union(
    detail::free_variable_names(p),
    detail::summand_variable_names(p)
  );
  
  std::vector<data_variable> src;  // contains the variables that need to be renamed
  std::vector<data_variable> dest; // contains the corresponding replacements
  generator.add_identifiers(forbidden_names);

  for (data_variable_list::iterator i = p.process_parameters().begin(); i != p.process_parameters().end(); ++i)
  {
    identifier_string new_name = generator(i->name());
    if (new_name != i->name())
    {
      // save the old and new value in the src and dest arrays
      src.push_back(*i);
      dest.push_back(data_variable(new_name, i->sort()));
    }
  }
  return std::make_pair(src, dest);
}

/// Renames the process parameters in the process p using the given identifier generator.
template <typename IdentifierGenerator>
linear_process rename_process_parameters(const linear_process& p, IdentifierGenerator& generator)
{
  std::pair<std::vector<data_variable>, std::vector<data_variable> > r = rename_process_parameters_helper(p, generator);
  return atermpp::partial_replace(p, detail::make_data_variable_replacer(r.first, r.second));
}

/// Renames the process parameters in the process p, such that none of them
/// appears in forbidden_names. Postfix is used as a hint for the new name.
inline
linear_process rename_process_parameters(const linear_process& p, const std::set<identifier_string>& forbidden_names, const std::string postfix)
{
  lps::postfix_identifier_generator generator(postfix);
  generator.add_identifiers(forbidden_names);
  return rename_process_parameters(p, generator);
}

/// Renames the process parameters in the specification spec using the given identifier generator.
template <typename IdentifierGenerator>
specification rename_process_parameters(const specification& spec, IdentifierGenerator& generator)
{
  std::pair<std::vector<data_variable>, std::vector<data_variable> > r = rename_process_parameters_helper(spec.process(), generator);

  linear_process new_process              = atermpp::partial_replace(spec.process()        , detail::make_data_variable_replacer(r.first, r.second));
  action_label_list new_action_labels     = atermpp::partial_replace(spec.action_labels()  , detail::make_data_variable_replacer(r.first, r.second));
  process_initializer new_initial_process = atermpp::partial_replace(spec.initial_process(), detail::make_data_variable_replacer(r.first, r.second));
    
  specification result = spec;
  result = set_lps(result, new_process);
  result = set_action_labels(result, new_action_labels);
  result = set_initial_process(result, new_initial_process);
  return result;
}

/// Renames the process parameters in the specification spec, such that none of them
/// appears in forbidden_names. Postfix is used as a hint for the new name.
inline
specification rename_process_parameters(const specification& spec, const std::set<identifier_string>& forbidden_names, const std::string postfix)
{
  lps::postfix_identifier_generator generator(postfix);
  generator.add_identifiers(forbidden_names);
  return rename_process_parameters(spec, generator);
}

/// Renames the free variables in the process p using the given identifier generator.
template <typename IdentifierGenerator>
linear_process rename_free_variables(const linear_process& p, IdentifierGenerator& generator)
{
  std::set<identifier_string> forbidden_names = detail::set_union(
    detail::process_parameter_names(p),
    detail::summand_variable_names(p)
  );

  std::vector<data_variable> src;  // contains the variables that need to be renamed
  std::vector<data_variable> dest; // contains the corresponding replacements
  generator.add_identifiers(forbidden_names);

  for (data_variable_list::iterator i = p.free_variables().begin(); i != p.free_variables().end(); ++i)
  {
    identifier_string new_name = generator(i->name());
    if (new_name != i->name())
    {
      // save the old and new value in the src and dest arrays
      src.push_back(*i);
      dest.push_back(data_variable(new_name, i->sort()));
    }
  }
  return atermpp::partial_replace(p, detail::make_data_variable_replacer(src, dest));
}

/// Renames the free variables in the process p, such that none of them
/// appears in forbidden_names. Postfix is used as a hint for the new name.
inline
linear_process rename_free_variables(const linear_process& p, const std::set<identifier_string>& forbidden_names, const std::string& postfix)
{
  lps::postfix_identifier_generator generator(postfix);
  generator.add_identifiers(forbidden_names);
  return rename_free_variables(p, generator);
}

/// Renames the summation variables in the process p using the given identifier generator.
template <typename IdentifierGenerator>
linear_process rename_summation_variables(const linear_process& p, IdentifierGenerator& generator)
{
  atermpp::vector<summand> new_summands;

  std::set<identifier_string> forbidden_names = detail::set_union(
    detail::process_parameter_names(p),
    detail::free_variable_names(p)
  );
  generator.add_identifiers(forbidden_names);

  for (summand_list::iterator i = p.summands().begin(); i != p.summands().end(); ++i)
  {
    std::vector<data_variable> src;  // contains the variables that need to be renamed
    std::vector<data_variable> dest; // contains the corresponding replacements

    for (data_variable_list::iterator j = i->summation_variables().begin(); j != i->summation_variables().end(); ++j)
    {
      identifier_string new_name = generator(j->name());
      if (new_name != j->name())
      {
        // save the old and new value in the src and dest arrays
        src.push_back(*j);
        dest.push_back(data_variable(new_name, j->sort()));
      }
    }
    new_summands.push_back(atermpp::partial_replace(*i, detail::make_data_variable_replacer(src, dest)));
  }
  
  return set_summands(p, summand_list(new_summands.begin(), new_summands.end()));
}

/// Renames the summation variables in the process p, such that none of them
/// appears in forbidden_names. Postfix is used as a hint for the new name.
inline
linear_process rename_summation_variables(const linear_process& p, const std::set<identifier_string>& forbidden_names, const std::string& postfix)
{
  lps::postfix_identifier_generator generator(postfix);
  generator.add_identifiers(forbidden_names);
  return rename_summation_variables(p, generator);
}

} // namespace lps

#endif // MCRL2_LPS_RENAME_H
