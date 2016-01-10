// Author(s): Jan Friso Groote
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbes_solver_test.h
/// \brief This file contains a simple solver for pbesses.
///        This solver is mainly intended for testing purposes.

#ifndef MCRL2_PBES_PBES_SOLVER_TEST_H
#define MCRL2_PBES_PBES_SOLVER_TEST_H

//General includes
#include "mcrl2/core/detail/print_utility.h"

//Rewriters
#include "mcrl2/data/rewriter.h"
#include "mcrl2/pbes/algorithms.h"

//Data framework
#include "mcrl2/data/enumerator.h"
#include "mcrl2/data/selection.h"

//Boolean equation systems
#include "mcrl2/pbes/pbesinst_alternative_lazy_algorithm.h"
#include "mcrl2/pbes/search_strategy.h"
#include "mcrl2/pbes/transformation_strategy.h"
#include "mcrl2/bes/pbesinst_conversion.h"
#include "mcrl2/bes/local_fixpoints.h"


namespace mcrl2
{

namespace pbes_system
{

/// \brief Straightforward solver for pbesses
/// \details This solver gets a parameterised boolean equation system.
///         It subsequently transforms it to a boolean equation system
///         where quantifiers are eliminated as much as possible (using
///         the quantifier-all pbes rewriter). After generating the equations
///         a straightforward solver is called on the equations to solve
///         these. This method uses the same code as pbes2bool (except that
///         pbes2bool uses more advanced features).

inline
bool pbes2_bool_test(pbes& pbes_spec, data::rewriter::strategy rewrite_strategy = data::jitty)
{
  pbes_system::algorithms::instantiate_global_variables(pbes_spec);

  // Generate an enumerator, a data rewriter and a pbes rewriter.
  data::rewriter datar(pbes_spec.data(),
                       mcrl2::data::used_data_equation_selector(pbes_spec.data(), pbes_system::find_function_symbols(pbes_spec), pbes_spec.global_variables()),
                       rewrite_strategy);

#ifdef MCRL2_REWRITE_RULE_SELECTION_DEBUG
std::clog << "--- rewrite rule selection specification ---\n";
std::clog << pbes_system::pp(pbes_spec) << std::endl;
std::clog << "--- rewrite rule selection function symbols ---\n";
std::clog << core::detail::print_set(pbes_system::find_function_symbols(pbes_spec)) << std::endl;
#endif

  // data::rewriter(pbes_spec.data(), mcrl2::data::used_data_equation_selector(pbes_spec.data(), pbes_spec.equations()), rewrite_strategy());
  // const bool enumerate_infinite_sorts = true;
  // enumerate_quantifiers_rewriter pbesr(datar, datae, enumerate_infinite_sorts);
  // The use of a pbesrewriter is switched off, because the pbesrewriter is too slow for the time being.
  // ::bes::boolean_equation_system bes_equations(pbes_spec, pbesr);
  // ::bes::boolean_equation_system bes_equations(pbes_spec, datar);
  // return solve_bes(bes_equations,false,false);

  pbesinst_alternative_lazy_algorithm algorithm(pbes_spec.data(), rewrite_strategy, breadth_first, on_the_fly);
  algorithm.run(pbes_spec);
  bes::boolean_equation_system bes = bes::pbesinst_conversion(algorithm.get_result());
  std::vector<bool> full_solution;
  return local_fixpoints(bes, &full_solution);
}

} // namespace pbes_system
} // namespace mcrl2

#endif // MCRL2_PBES_PBES_SOLVER_TEST_H
