// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file quantifier_rename_builder.h
/// \brief Add your file description here.

#ifndef MCRL2_PBES_DETAIL_QUANTIFIER_RENAME_BUILDER_H
#define MCRL2_PBES_DETAIL_QUANTIFIER_RENAME_BUILDER_H

#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>
#include <boost/iterator/transform_iterator.hpp>
#include "mcrl2/atermpp/deque.h"
#include "mcrl2/data/detail/data_functional.h"
#include "mcrl2/pbes/pbes_expr_builder.h"
#include "mcrl2/pbes/pbes_expression.h"
#include "mcrl2/data/set_identifier_generator.h"
#include "mcrl2/data/substitute.h"

namespace atermpp
{
/// \cond INTERNAL_DOCS
template<>
struct aterm_traits<std::pair<mcrl2::data::variable, mcrl2::data::variable> >
{
  typedef ATermAppl aterm_type;
  static void protect(std::pair<mcrl2::data::variable, mcrl2::data::variable> t)
  {
    t.first.protect();
    t.second.protect();
  }
  static void unprotect(std::pair<mcrl2::data::variable, mcrl2::data::variable> t)
  {
    t.first.unprotect();
    t.second.unprotect();
  }
  static void mark(std::pair<mcrl2::data::variable, mcrl2::data::variable> t)
  {
    t.first.mark();
    t.second.mark();
  }
};
} // namespace atermpp

namespace mcrl2
{

namespace pbes_system
{

namespace detail
{

/// Visitor that renames quantifier variables, to make sure that within the scope of a quantifier
/// variable, no other quantifier variables or free variables with the same name occur.
/// The identifier generator that is supplied via the constructor is used to choose new names.
template <typename IdentifierGenerator>
struct quantifier_rename_builder: public pbes_expr_builder<pbes_expression>
{
  IdentifierGenerator& generator;
  std::vector<data::variable_list> quantifier_stack;
  atermpp::deque<std::pair<data::variable, data::variable> > replacements;

  quantifier_rename_builder(IdentifierGenerator& generator)
    : generator(generator)
  {}

  /// \brief Returns true if the quantifier_stack contains a data variable with the given name
  /// \param name A
  /// \return True if the quantifier_stack contains a data variable with the given name
  bool is_in_quantifier_stack(core::identifier_string name) const
  {
    for (std::vector<data::variable_list>::const_iterator i = quantifier_stack.begin(); i != quantifier_stack.end(); ++i)
    {
      if (std::find(boost::make_transform_iterator(i->begin(), data::detail::variable_name()),
                    boost::make_transform_iterator(i->end()  , data::detail::variable_name()),
                    name
                   ) != boost::make_transform_iterator(i->end()  , data::detail::variable_name())
         )
      {
        return true;
      }
    }
    return false;
  }

  /// \brief Adds variables to the quantifier stack, and adds replacements for the name clashes to replacements.
  /// \param variables A sequence of data variables
  /// \return The number of replacements that were added.
  size_t push(const data::variable_list& variables)
  {
    size_t replacement_count = 0;

    // check for new name clashes
    for (data::variable_list::const_iterator i = variables.begin(); i != variables.end(); ++i)
    {
      if (is_in_quantifier_stack(i->name()))
      {
        replacement_count++;
        data::variable new_variable(generator(i->name()), i->sort());

        // add the replacement in front, since it needs to be applied first
        replacements.push_front(std::make_pair(*i, new_variable));
      }
    }
    quantifier_stack.push_back(variables);
    generator.add_to_context(variables);

    return replacement_count;
  }

  /// \brief Removes the last added variable list from the quantifier stack, and removes
  /// replacement_count replacements.
  /// \param replacement_count A positive integer
  void pop(size_t replacement_count)
  {
    generator.remove_from_context(quantifier_stack.back());
    for (size_t i = 0; i < replacement_count; i++)
    {
      generator.remove_identifier(replacements.front().second.name());
      replacements.pop_front();
    }
    quantifier_stack.pop_back();
  }

  /// \brief Visit data_expression node
  /// \param e A PBES expression
  /// \param d A data expression
  /// \return The result of visiting the node
  pbes_expression visit_data_expression(const pbes_expression& /* e */, const data::data_expression& d)
  {
    return data::substitute_free_variables(d, data::make_pair_sequence_substitution(replacements));
  }

  /// \brief Visit forall node
  /// \param e A PBES expression
  /// \param variables A sequence of data variables
  /// \param expression A PBES expression
  /// \return The result of visiting the node
  pbes_expression visit_forall(const pbes_expression& /* e */, const data::variable_list& variables, const pbes_expression& expression)
  {
    size_t replacement_count = push(variables);
    pbes_expression new_expression = visit(expression);
    data::variable_list new_variables = replacement_count > 0 ? data::substitute_variables(variables, data::make_pair_sequence_substitution(replacements)) : variables;
    pop(replacement_count);
    return pbes_expr::forall(new_variables, new_expression);
  }

  /// \brief Visit exists node
  /// \param e A PBES expression
  /// \param variables A sequence of data variables
  /// \param expression A PBES expression
  /// \return The result of visiting the node
  pbes_expression visit_exists(const pbes_expression& /* e */, const data::variable_list& variables, const pbes_expression& expression)
  {
    size_t replacement_count = push(variables);
    pbes_expression new_expression = visit(expression);
    data::variable_list new_variables = replacement_count > 0 ? data::substitute_variables(variables, data::make_pair_sequence_substitution(replacements)) : variables;
    pop(replacement_count);
    return pbes_expr::exists(new_variables, new_expression);
  }
};

/// \brief Returns a quantifier_rename_builder
/// \param generator A generator for fresh identifiers
/// \return A quantifier_rename_builder
template <typename IdentifierGenerator>
quantifier_rename_builder<IdentifierGenerator> make_quantifier_rename_builder(IdentifierGenerator& generator)
{
  return quantifier_rename_builder<IdentifierGenerator>(generator);
}

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_DETAIL_QUANTIFIER_RENAME_BUILDER_H
