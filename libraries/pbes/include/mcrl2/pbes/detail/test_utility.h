// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file test_utility.h
/// \brief Utility functions for testing.

#ifndef MCRL2_PBES_DETAIL_TEST_UTILITY_H
#define MCRL2_PBES_DETAIL_TEST_UTILITY_H

#include <string>
#include "mcrl2/data/variable.h"
#include "mcrl2/data/data_expression.h"
#include "mcrl2/data/nat.h"
#include "mcrl2/data/bool.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/propositional_variable.h"

namespace mcrl2
{

namespace pbes_system
{

namespace detail
{

/// \brief Returns a data variable of type Nat with a given name
/// \param name A string
/// \return A data variable of type Nat with a given name
data::variable nat(const std::string& name)
{
  return data::variable(core::identifier_string(name), data::sort_nat::nat());
}

/// \brief Returns a data variable of type Pos with a given name
/// \param name A string
/// \return A data variable of type Pos with a given name
data::variable pos(const std::string& name)
{
  return data::variable(core::identifier_string(name), data::sort_pos::pos());
}

/// \brief Returns a data variable of type Bool with a given name
/// \param name A string
/// \return A data variable of type Bool with a given name
data::variable bool_(const std::string& name)
{
  return data::variable(core::identifier_string(name), data::sort_bool::bool_());
}

/// \brief Returns a propositional variable declaration with the given name and parameters
/// \param name A string
/// \param parameters A sequence of data variables
/// \return A propositional variable declaration with the given name and parameters
propositional_variable propvar(const std::string& name, const data::variable_list& parameters)
{
  return propositional_variable(core::identifier_string(name), parameters);
}

/// \brief Returns a propositional variable instantiation with the given name and parameters
/// \param name A string
/// \param parameters A sequence of data expressions
/// \return A propositional variable instantiation with the given name and parameters
propositional_variable_instantiation propvarinst(const std::string& name, const data::data_expression_list& parameters)
{
  return propositional_variable_instantiation(core::identifier_string(name), parameters);
}

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_DETAIL_TEST_UTILITY_H
