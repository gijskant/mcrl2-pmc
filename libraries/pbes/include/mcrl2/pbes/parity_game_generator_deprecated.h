// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/parity_game_generator_deprecated.h
/// \brief A class for generating a parity game from a pbes. This version is
///        more efficient that the one in mcrl2/pbes/parity_game_generator.h

#ifndef MCRL2_PBES_PARITY_GAME_GENERATOR_DEPRECATED_H
#define MCRL2_PBES_PARITY_GAME_GENERATOR_DEPRECATED_H

#include <iostream>
#include <iomanip>
#include <map>
#include <set>
#include <utility>
#include "mcrl2/atermpp/aterm_list.h"
#include "mcrl2/atermpp/map.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/utilities/algorithm.h"
#include "mcrl2/utilities/logger.h"
#include "mcrl2/data/enumerator.h"
#include "mcrl2/data/identifier_generator.h"
#include "mcrl2/data/selection.h"
#include "mcrl2/data/detail/rewriter_wrapper.h"
#include "mcrl2/pbes/normalize.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/rewriter.h"
#include "mcrl2/pbes/detail/bes_equation_limit.h"
#include "mcrl2/pbes/parity_game_generator.h"

#include "mcrl2/bes/bes_deprecated.h" // We use some implementation tricks from this

namespace mcrl2
{

namespace pbes_system
{

/// \brief Parity game generator which uses the internal rewrite format of the
/// legacy rewriters. This is more efficient than parity_game_generator
class parity_game_generator_deprecated: public parity_game_generator
{
  protected:
    /// \brief Type used internally for storing equations
    typedef atermpp::aterm_appl internal_equation_t;

    /// \brief Whether to translation pbes_equations to internal format or not.
    /// Note that this does not work in debug mode.
    bool m_precompile_pbes;

    /// \brief Rewriter used internally to achieve better performance
    data::detail::legacy_rewriter datar_internal;

    /// \brief Maps propositional variables to corresponding PBES equations.
    std::map<core::identifier_string, atermpp::vector<internal_equation_t>::const_iterator > m_pbes_equation_index;

    /// \brief Stores an internal representation of equations
    atermpp::vector<internal_equation_t> m_internal_equations;

    pbes_expression from_rewrite_format(const pbes_expression& e)
    {
      if(is_pbes_true(e) || is_pbes_false(e))
      {
        return e;
      }
      else if(tr::is_and(e))
      {
        return tr::and_(from_rewrite_format(tr::left(e)), from_rewrite_format(tr::right(e)));
      }
      else if(tr::is_or(e))
      {
        return tr::or_(from_rewrite_format(tr::left(e)), from_rewrite_format(tr::right(e)));
      }
      else if(tr::is_prop_var(e))
      {
        tr::data_term_sequence_type args = tr::param(e);
        data::data_expression_vector pretty_args;
        for(tr::data_term_sequence_type::const_iterator i = args.begin(); i != args.end(); ++i)
        {
          pretty_args.push_back(datar_internal.convert_from((atermpp::aterm_appl)*i));
        }
        return tr::prop_var(tr::name(e), pretty_args.begin(), pretty_args.end());
      }
      else
      {
        return datar_internal.convert_from((atermpp::aterm_appl)e);
      }
    }

    virtual
    std::string print(const pbes_expression& e)
    {
      if(m_precompile_pbes)
      {
        return e.to_string() + " (" + core::pp(from_rewrite_format(e)) + ")";
      }
      else
      {
        return core::pp(e);
      }
    }

    /// \brief Check whether e corresponds to true
    virtual
    bool is_true(const pbes_expression& e) const
    {
      bool result = tr::is_true(e);
      if(m_precompile_pbes)
      {
        result = result || e == datar_internal.internal_true;
      }
      return result;
    }

    /// \brief Check whether e corresponds to false
    virtual
    bool is_false(const pbes_expression& e) const
    {
      bool result = tr::is_false(e);
      if(m_precompile_pbes)
      {
        result = result || e == datar_internal.internal_false;
      }
      return result;
    }

    /// \brief Add mappings and equations to datar_internal
    /// Declare constructors to the rewriter to prevent unnecessary compilation for bound variables.
    // This can be removed if the jittyc compilers are not in use anymore.
    void initialize_internal_rewriter()
    {
      std::set < mcrl2::data::variable > vset=mcrl2::pbes_system::find_variables(m_pbes);
      std::set < mcrl2::data::variable > vfset=mcrl2::pbes_system::find_free_variables(m_pbes);
      std::set < mcrl2::data::variable > diff_set;
      std::set_difference(vfset.begin(),vfset.end(),vset.begin(),vset.end(),std::inserter(diff_set,diff_set.begin()));

      std::set < mcrl2::data::sort_expression > bounded_sorts;
      for(std::set < mcrl2::data::variable > :: const_iterator i=diff_set.begin(); i!=diff_set.end(); ++i)
      {
        bounded_sorts.insert(i->sort());
      }
      for(std::set < mcrl2::data::sort_expression > :: const_iterator i=bounded_sorts.begin(); i!=bounded_sorts.end(); ++i)
      {
        const mcrl2::data::function_symbol_vector constructors(m_pbes.data().constructors(*i));
        for (mcrl2::data::function_symbol_vector::const_iterator j = constructors.begin(); j != constructors.end(); ++j)
        {
          datar_internal.convert_to(*i);
        }
      }
    }


    /// \brief Translate equations to internal format, and store them in m_internal_equations
    void populate_internal_equations()
    {
      for (atermpp::vector<pbes_equation>::const_iterator i = m_pbes.equations().begin(); i != m_pbes.equations().end(); ++i)
      {
        m_internal_equations.push_back(
          pbes_equation_to_aterm(
            pbes_equation(
              i->symbol(),
              i->variable(),
              rewrite_and_simplify(i->formula())
            )));
      }
    }

    /// \brief Simplify expression e.
    pbes_expression rewrite_and_simplify(const pbes_expression& e)
    {
      return ::bes::pbes_expression_rewrite_and_simplify(e, m_precompile_pbes, datar_internal);
    }

    /// \brief Substitute and rewrite e.
    pbes_expression substitute_and_rewrite(const pbes_expression& e)
    {
      return detail::pbes_expression_substitute_and_rewrite
          (e,
           m_pbes.data(),
           datar_internal,
           m_precompile_pbes
          );
    }

    /// \brief Compute equation index map.
    virtual
    void compute_equation_index_map()
    {
      for (atermpp::vector<internal_equation_t>::const_iterator i = m_internal_equations.begin(); i != m_internal_equations.end(); ++i)
      {
        m_pbes_equation_index[pbes_equation(*i).variable().name()] = i;
      }
    }

    /// \brief Make a substitution in the internal rewriter
    void make_substitution_internal(const data::variable_list& v, const data::data_expression_list& e)
    {
      data::variable_list::const_iterator i = v.begin();
      for(data::data_expression_list::const_iterator j = e.begin();
          i != v.end() && j != e.end(); ++i, ++j)
      {
        if(m_precompile_pbes)
        {
          datar_internal.set_internally_associated_value(*i,(atermpp::aterm)(*j));
        }
        else
        {
          datar_internal.set_internally_associated_value(*i,*j);
        }
      }
    }

    /// \brief Remove a substitution from the internal rewriter
    void clear_substitution_internal(const data::variable_list& v)
    {
      for(data::variable_list::const_iterator i = v.begin();
          i != v.end(); ++i)
      {
        datar_internal.clear_internally_associated_value(*i);
      }
    }

    virtual
    pbes_expression expand_rhs(const pbes_expression& psi)
    {
      // expand the right hand side if needed
      if (tr::is_prop_var(psi))
      {
        const pbes_equation& pbes_eqn = *m_pbes_equation_index[tr::name(psi)];

        mCRL2log(debug2, "parity_game_generator") << "Expanding right hand side of formula " << psi << std::endl << "  rhs: " << (m_precompile_pbes?(pbes_eqn.formula().to_string()):(tr::pp(pbes_eqn.formula()))) << " into ";

        pbes_expression result;

        make_substitution_internal(pbes_eqn.variable().parameters(), tr::param(psi));
        result = substitute_and_rewrite(pbes_eqn.formula());
        clear_substitution_internal(pbes_eqn.variable().parameters());

        mCRL2log(debug2, "parity_game_generator") << (m_precompile_pbes?(result.to_string()):(tr::pp(result))) << std::endl;

        return result;
      }
      return psi;
    }

    virtual
    void initialize_generation()
    {
      if (m_initialized)
      {
        return;
      }
      else
      {
                // Nothing to be done for an empty PBES.
        if (m_pbes.equations().empty())
        {
          return;
        }

        // Normalize the pbes, since the parity game generator currently doesn't handle negation and implication.
        pbes_system::normalize(m_pbes);
        initialize_internal_rewriter();
        populate_internal_equations();
        compute_equation_index_map();
        compute_priorities(m_pbes.equations());

        // Add a BES equation for the initial state.
        propositional_variable_instantiation phi = rewrite_and_simplify(m_pbes.initial_state());
        add_bes_equation(phi, m_priorities[phi.name()]);

        m_initialized = true;
      }
    }

  public:
    using parity_game_generator::PGAME_OR;
    using parity_game_generator::PGAME_AND;

    /// \brief Constructor.
    /// \param p A PBES
    /// \param true_false_dependencies If true, nodes are generated for the values <tt>true</tt> and <tt>false</tt>.
    /// \param is_min_parity If true a min-parity game is produced, otherwise a max-parity game
    parity_game_generator_deprecated(pbes<>& p, bool true_false_dependencies = false, bool is_min_parity = true, size_t log_level = 0, data::rewriter::strategy rewrite_strategy = data::rewriter::jitty)
      :
      parity_game_generator(p, true_false_dependencies, is_min_parity, log_level, rewrite_strategy),
      datar_internal(datar)
    {
#ifdef NDEBUG
      m_precompile_pbes = true;
      mCRL2log(verbose) << "Using precompiled PBES" << std::endl;
#else
      m_precompile_pbes = false;
#endif
    }

};

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_PARITY_GAME_GENERATOR_H
