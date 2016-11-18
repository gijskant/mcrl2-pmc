// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/lps2pbes.h
/// \brief Add your file description here.

#ifndef MCRL2_PBES_LPS2PBES_H
#define MCRL2_PBES_LPS2PBES_H

#include <string>
#include "mcrl2/atermpp/detail/aterm_list_utility.h"
#include "mcrl2/data/set_identifier_generator.h"
#include "mcrl2/modal_formula/algorithms.h"
#include "mcrl2/modal_formula/state_formula.h"
#include "mcrl2/modal_formula/preprocess_state_formula.h"
#include "mcrl2/lps/linearise.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/detail/make_timed_lps.h"
#include "mcrl2/pbes/algorithms.h"
#include "mcrl2/pbes/lps2pbes.h"
#include "mcrl2/pbes/is_monotonous.h"
#include "mcrl2/pbes/detail/lps2pbes_indenter.h"
#include "mcrl2/pbes/detail/lps2pbes_utility.h"
#include "mcrl2/pbes/detail/lps2pbes_rhs.h"
#include "mcrl2/pbes/detail/lps2pbes_e.h"
#include "mcrl2/pbes/detail/term_traits_optimized.h"

namespace mcrl2
{

namespace pbes_system
{

namespace detail
{
  /// \brief Returns a variable that doesn't appear in context
  /// \param ids a set of identifiers to use as context.
  /// \param s A sort expression
  /// \param hint A string
  /// \return A variable that doesn't appear in context
  /// \warning reorganising the identifier context is expensive, consider using an identifier generator
  inline
  data::variable fresh_variable(const std::set<core::identifier_string>& ids, const data::sort_expression& s, const std::string& hint)
  {
    data::set_identifier_generator generator;
    generator.add_identifiers(ids);
    return data::variable(generator(hint), s);
  }

} // namespace detail

/// \brief Algorithm for translating a state formula and a timed specification to a pbes.
class lps2pbes_algorithm
{
  public:
    /// \brief Runs the translation algorithm
    /// \param formula A modal formula that represents a property about the system modeled by the given specification
    /// \param spec A linear process specification
    /// \param structured use the 'structured' approach of generating equations
    /// \param unoptimized do not optimize the resulting PBES.
    /// \param preprocess_modal_operators insert dummy fixpoints in modal operators, which may lead to smaller PBESs
    /// \param T The time parameter. If T == data::variable() the untimed version of lps2pbes is applied.
    /// \return A PBES that encodes the property applied to the given specification
    pbes run(const state_formulas::state_formula& formula, const lps::specification& spec, bool structured = false, bool unoptimized = false, bool preprocess_modal_operators = false, const data::variable& T = data::undefined_real_variable())
    {
      using atermpp::detail::operator+;

      if (!state_formulas::algorithms::is_monotonous(formula))
      {
        throw mcrl2::runtime_error(std::string("lps2pbes error: the formula ") + state_formulas::pp(formula) + " is not monotonous!");
      }

      if (!preprocess_modal_operators && state_formulas::detail::count_modal_operator_nesting(formula) >= 2)
      {
        mCRL2log(log::info) <<
          "Warning: detected nested modal operators. This may result in a long execution time.\n"
          "Use the option -m or insert dummy fix point operators in between manually to speed\n"
          "up the transformation." << std::endl;
      }

      // resolve name conflicts and wrap the formula in a mu or nu if needed
      mCRL2log(log::debug) << "formula before preprocessing: " << state_formulas::pp(formula) << std::endl;
      state_formulas::state_formula f = state_formulas::algorithms::preprocess_state_formula(formula, spec);
      mCRL2log(log::debug) << "formula after preprocessing:  " << state_formulas::pp(f) << std::endl;

      if (preprocess_modal_operators)
      {
        f = state_formulas::preprocess_nested_modal_operators(f);
        mCRL2log(log::debug) << "formula after preprocessing modal operators:  " << state_formulas::pp(f) << std::endl;
      }

      // remove occurrences of ! and =>
      if (!state_formulas::algorithms::is_normalized(f))
      {
        f = state_formulas::algorithms::normalize(f);
      }
      mCRL2log(log::debug) << "formula after normalization:  " << state_formulas::pp(f) << std::endl;
      assert(state_formulas::algorithms::is_normalized(f));

      data::set_identifier_generator id_generator;
      std::set<core::identifier_string> ids = lps::find_identifiers(spec);
      id_generator.add_identifiers(ids);
      ids = data::find_identifiers(spec.data().constructors());
      id_generator.add_identifiers(ids);
      ids = data::find_identifiers(spec.data().mappings());
      id_generator.add_identifiers(ids);
      ids = state_formulas::find_identifiers(f);
      id_generator.add_identifiers(ids);

      // compute the equations
      std::vector<pbes_equation> eqn;
      if (structured)
      {
        data::set_identifier_generator propvar_generator;
        std::set<core::identifier_string> names = state_formulas::algorithms::find_state_variable_names(f);
        propvar_generator.add_identifiers(names);
        if (unoptimized)
        {
          detail::E_structured(f, f, spec.process(), id_generator, propvar_generator, T, eqn, core::term_traits<pbes_expression>());
        }
        else
        {
          detail::E_structured(f, f, spec.process(), id_generator, propvar_generator, T, eqn, core::term_traits_optimized<pbes_expression>());
        }
      }
      else
      {
        if (unoptimized)
        {
          detail::E(f, f, spec.process(), id_generator, T, eqn, core::term_traits<pbes_expression>());
        }
        else
        {
          detail::E(f, f, spec.process(), id_generator, T, eqn, core::term_traits_optimized<pbes_expression>());
        }
      }

      // compute the initial state
      assert(eqn.size() > 0);
      pbes_equation e1 = eqn.front();
      core::identifier_string Xe(e1.variable().name());
      assert(state_formulas::is_mu(f) || state_formulas::is_nu(f));
      const core::identifier_string& Xf = detail::mu_name(f);
      data::data_expression_list fi = detail::mu_expressions(f);
      data::data_expression_list pi = spec.initial_process().state(spec.process().process_parameters());
      data::data_expression_list e = fi + pi + detail::Par(Xf, data::variable_list(), f);
      if (T != data::undefined_real_variable())
      {
        e = data::sort_real::real_(0) + e;
      }
      propositional_variable_instantiation init(Xe, e);

      pbes result(spec.data(), eqn, spec.global_variables(), init);
      assert(is_monotonous(result));
      pbes_system::algorithms::normalize(result);
      assert(pbes_system::algorithms::is_normalized(result));
      assert(result.is_closed());
      complete_data_specification(result);
      return result;
    }
};

/// \brief Translates a linear process specification and a state formulas to a PBES. If the solution of the PBES
/// is true, the formula holds for the specification.
/// \param formula A modal formula
/// \param spec A linear process specification
/// \param timed determines whether the timed or untimed variant of the algorithm is chosen
/// \param structured use the 'structured' approach of generating equations
/// \param unoptimized if true, the resulting PBES is not simplified, if false (default),
///        the PBES is simplified.
/// \return The resulting pbes
inline pbes lps2pbes(const lps::specification& spec, const state_formulas::state_formula& formula, bool timed = false, bool structured = false, bool unoptimized = false, bool preprocess_modal_operators = false)
{
  if ((formula.has_time() || spec.process().has_time()) && !timed)
  {
    mCRL2log(log::warning) << "Switch to timed translation because formula has " << (formula.has_time()?"":"no ") << "time, and process has " << (spec.process().has_time()?"":"no ") << "time" << std::endl;
    timed = true;
  }

  if (timed)
  {
    lps::specification spec_timed = spec;
    std::set<core::identifier_string> ids = lps::find_identifiers(spec);
    std::set<core::identifier_string> fcontext = state_formulas::find_identifiers(formula);
    ids.insert(fcontext.begin(), fcontext.end());
    data::variable T = detail::fresh_variable(ids, data::sort_real::real_(), "T");
    ids.insert(T.name());
    lps::detail::make_timed_lps(spec_timed.process(), ids);
    return lps2pbes_algorithm().run(formula, spec_timed, structured, unoptimized, preprocess_modal_operators, T);
  }
  else
  {
    return lps2pbes_algorithm().run(formula, spec, structured, unoptimized, preprocess_modal_operators);
  }
}

/// \brief Applies the lps2pbes algorithm
/// \param spec_text A string
/// \param formula_text A string
/// \param timed Determines whether the timed or untimed version of the translation algorithm is used
/// \return The result of the algorithm
inline
pbes lps2pbes(const std::string& spec_text, const std::string& formula_text, bool timed = false, bool structured = false, bool unoptimized = false, bool preprocess_modal_operators = false)
{
  pbes result;
  lps::specification spec = remove_stochastic_operators(lps::linearise(spec_text));

  state_formulas::state_formula f = state_formulas::algorithms::parse_state_formula(formula_text, spec);
  return lps2pbes(spec, f, timed, structured, unoptimized, preprocess_modal_operators);
}

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_LPS2PBES_H
