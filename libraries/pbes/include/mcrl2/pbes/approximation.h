// Author(s): Gijs Kant
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/approximation.h
/// \brief add your file description here.

#ifndef MCRL2_PBES_APPROXIMATION_H
#define MCRL2_PBES_APPROXIMATION_H

#include "mcrl2/bes/gauss_elimination.h"
#include "mcrl2/data/traverser.h"
#include "mcrl2/data/rewriter.h"
//#include "mcrl2/data/detail/prover/smt_lib_solver.h"
#include "mcrl2/data/detail/bdd_prover.h"
#include "mcrl2/pbes/pbes_gauss_elimination.h"
#include "mcrl2/pbes/pbespgsolve.h"
#include "mcrl2/pbes/detail/pbes2data_rewriter.h"

namespace mcrl2
{

namespace pbes_system
{


inline
bool compare(const pbes_system::pbes_expression& x, const pbes_system::pbes_expression& y)
{
  return x == y;
}


typedef bool (*compare_function)(const pbes_system::pbes_expression& x, const pbes_system::pbes_expression& y);

bool pbes_approximate(pbes_system::pbes& x)
{
  using namespace pbes_system;
  typedef core::term_traits<pbes_expression> tr;

  /// \brief Rewriter for simplifying expressions
  data::rewriter::strategy rewrite_strategy(data::parse_rewrite_strategy("jitty"));
  data::rewriter datar(x.data(), rewrite_strategy);
  pbes_system::simplify_data_rewriter<data::rewriter> rewr(datar);

  gauss_elimination_algorithm<pbes_traits> algorithm;
  algorithm.run(
      x.equations().begin(),
      x.equations().end(),
      approximate<pbes_traits, compare_function, pbes_system::simplify_data_rewriter<data::rewriter>* >
        (compare, 2, &rewr)
  );

  propositional_variable_instantiation init = x.initial_state();
  pbes_equation eq;
  for(auto e: x.equations())
  {
    if (e.variable().name() == init.name())
    {
      eq = e;
      break;
    }
  }
  std::map< data::variable, data::data_expression > s;
  auto it1 = eq.variable().parameters().begin();
  auto it2 = init.parameters().begin();
  for(; it1 != eq.variable().parameters().end() && it2 != init.parameters().end(); ++it1, ++it2)
  {
    s[*it1] = *it2;
  }
  data::mutable_map_substitution<std::map< data::variable, data::data_expression >> sigma(s);
  pbes_expression expr = eq.formula();
  pbes_system::replace_variables(expr, sigma);


  mCRL2log(log::verbose) << "Result for " << init.name() << ": " << std::flush;
  if (tr::is_false(expr))
  {
    mCRL2log(log::verbose) << "false" << std::endl;
    return false;
  }
  else if (tr::is_true(expr))
  {
    mCRL2log(log::verbose) << "true" << std::endl;
    return true;
  }
  else
  {
    mCRL2log(log::verbose) << "undecided" << std::endl;
    return false;
  }
}

bool pbes_partial_solve(pbes_system::pbes& x, bool partial = true)
{
  if (partial)
  {
    return pbes_approximate(x);
  }
  else
  {
    return pbespgsolve(x);
  }
}


} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_APPROXIMATION_H
