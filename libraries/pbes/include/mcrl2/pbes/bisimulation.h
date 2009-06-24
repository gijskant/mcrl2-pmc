// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/rename.h
/// \brief Bisimulation algorithms.

#ifndef MCRL2_PBES_BISIMULATION_H
#define MCRL2_PBES_BISIMULATION_H

#include <algorithm>
#include <iterator>
#include <set>
#include <vector>
#include <sstream>
#include <boost/iterator/transform_iterator.hpp>
#include "mcrl2/atermpp/make_list.h"
#include "mcrl2/data/utility.h"
#include "mcrl2/data/detail/data_functional.h"
#include "mcrl2/data/detail/container_utility.h"
#include "mcrl2/data/detail/find.h"
#include "mcrl2/data/set_identifier_generator.h"
#include "mcrl2/lps/multi_action.h"
#include "mcrl2/lps/rename.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/detail/algorithm.h"
#include "mcrl2/data/detail/sorted_sequence_algorithm.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/detail/pbes_translate_impl.h"
#include "mcrl2/pbes/detail/global_variable_visitor.h"

namespace mcrl2 {

namespace pbes_system {

using data::operator+;
using namespace data;
using namespace lps;

/// \brief Base class for bisimulation algorithms.
class bisimulation_algorithm
{
  public:
    /// \brief The iterator type for non-delta summands
    typedef lps::action_summand_vector::const_iterator my_iterator;

  protected:
    /// \brief A map type for mapping summands to strings.
    typedef std::map<const lps::action_summand*, std::string> name_map;

    /// \brief Maps summands to strings.
    name_map summand_names;

    /// \brief Store the address of the model.
    const lps::linear_process* model_ptr;

    /// \brief Store the address of the specification.
    const lps::linear_process* spec_ptr;

    /// \brief Generates a name for an action_list.
    /// \param l A sequence of actions
    /// \return A string representation of the list \p l
    std::string action_list_name(action_list l) const
    {
      std::ostringstream out;
      for (action_list::iterator i = l.begin(); i != l.end(); ++i)
      {
        out << (i != l.begin() ? "-" : "") << std::string(i->label().name());
      }
      std::string result = out.str();
      if (result == "")
      {
        result = "tau";
      }
      return result;
    }

    /// \brief Returns the name of a summand
    /// \param i A summand iterator
    /// \return The name of the summand referred to by \p i
    std::string summand_name(my_iterator i) const
    {
      const lps::action_summand* t = &(*i);
      name_map::const_iterator j = summand_names.find(t);
      assert(j != summand_names.end());
      return j->second;
    }

    /// \brief Returns true if p is the linear process of the model.
    /// \param p A linear process
    /// \return True if p is the linear process of the model.
    bool is_from_model(const linear_process& p) const
    {
      return &p == model_ptr;
    }

    /// \brief Returns a name of a linear process.
    /// \param p A linear process
    /// \return The name of the linear process.
    std::string process_name(const linear_process& p) const
    {
      if (is_from_model(p))
      {
        return "m";
      }
      else
      {
        return "s";
      }
    }

    /// \brief Used for initializing summand names.
    /// \param p A linear process
    void set_summand_names(const linear_process& p)
    {
      set_identifier_generator generator;
      for (my_iterator i = p.action_summands().begin(); i != p.action_summands().end(); ++i)
      {
        std::string name = generator(action_list_name(i->multi_action().actions()));
        const lps::action_summand* t = &(*i);
        summand_names[t] = name;
      }
    }

public:
    /// \brief Creates a name for the propositional variable Xpq
    /// \param p A linear process
    /// \param q A linear process
    /// \return The name for the propositional variable Xpq
    core::identifier_string X(const linear_process& p, const linear_process& q) const
    {
      std::string s = "X" + process_name(p) + process_name(q);
      return core::identifier_string(s);
    }

    /// \brief Creates a name for the propositional variable Ypq
    /// \param p A linear process
    /// \param q A linear process
    /// \return The name for the propositional variable Ypq
    core::identifier_string Y(const linear_process& p, const linear_process& q) const
    {
      std::string s = "Y" + process_name(p) + process_name(q);
      return core::identifier_string(s);
    }

    /// \brief Creates a name for the propositional variable Ypqi
    /// \pre The iterator i must be in p.action_summands().
    /// \param p A linear process
    /// \param q A linear process
    /// \param i A summand iterator
    /// \return The name for the propositional variable Ypqi
    core::identifier_string Y(const linear_process& p, const linear_process& q, my_iterator i) const
    {
      std::string s = "Y" + process_name(p) + process_name(q) + "_" + summand_name(i);
      return core::identifier_string(s);
    }

    /// \brief Creates a name for the propositional variable Y1pqi
    /// \pre The iterator i must be in p.action_summands().
    /// \param p A linear process
    /// \param q A linear process
    /// \param i A summand iterator
    /// \return The name for the propositional variable Y1pqi
    core::identifier_string Y1(const linear_process& p, const linear_process& q, my_iterator i) const
    {
      std::string s = "Y1" + process_name(p) + process_name(q) + "_" + summand_name(i);
      return core::identifier_string(s);
    }

    /// \brief Creates a name for the propositional variable Y2pqi
    /// \pre The iterator i must be in p.action_summands().
    /// \param p A linear process
    /// \param q A linear process
    /// \param i A summand iterator
    /// \return The name for the propositional variable Y2pqi
    core::identifier_string Y2(const linear_process& p, const linear_process& q, my_iterator i) const
    {
      std::string s = "Y2" + process_name(p) + process_name(q) + "_" + summand_name(i);
      return core::identifier_string(s);
    }

    /// \brief Creates a propositional variable.
    /// \param name A
    /// \param parameters A sequence of data variables
    /// \return The created propositional variable
    propositional_variable_instantiation var(core::identifier_string name, variable_list  const& parameters) const
    {
      return propositional_variable_instantiation(name, data_expression_list(parameters.begin(), parameters.end()));
    }

    /// \brief Creates a propositional variable.
    /// \param name A
    /// \param parameters A sequence of data expressions
    /// \return The created propositional variable
    propositional_variable_instantiation var(core::identifier_string name, data_expression_list const& parameters) const
    {
      return propositional_variable_instantiation(name, parameters);
    }

    /// \brief Returns a pbes expression that expresses equality of the multi-actions a and b.
    /// \param a A sequence of actions
    /// \param b A sequence of actions
    /// \return Necessary conditions for the equality of a and b
    pbes_expression equals(const multi_action& a, const multi_action& b) const
    {
      return lps::equal_multi_actions(a, b);
    }

    /// \brief Returns the fixpoint symbol mu.
    /// \return The fixpoint symbol mu.
    fixpoint_symbol mu() const
    {
      return fixpoint_symbol::mu();
    }

    /// \brief Returns the fixpoint symbol nu.
    /// \return The fixpoint symbol nu.
    fixpoint_symbol nu() const
    {
      return fixpoint_symbol::nu();
    }

    /// \brief Renames variables in q such that there are no name clashes.
    /// \param p A linear process
    /// \param q A linear process
    /// \return The process q after renaming.
    linear_process resolve_name_clashes(const linear_process& p, const linear_process& q)
    {
      std::set<core::identifier_string> used_names;
      variable_list const& process_parameters(p.process_parameters());
      used_names.insert(boost::make_transform_iterator(process_parameters.begin(), data::detail::variable_name()),
                        boost::make_transform_iterator(process_parameters.end()  , data::detail::variable_name())
                       );
      variable_list const& global_variables(p.global_variables());
      used_names.insert(boost::make_transform_iterator(global_variables.begin(), data::detail::variable_name()),
                        boost::make_transform_iterator(global_variables.end()  , data::detail::variable_name())
                       );
      for (summand_list::iterator i = p.summands().begin(); i != p.summands().end(); ++i)
      {
        variable_list summation_variables(i->summation_variables());
        used_names.insert(boost::make_transform_iterator(summation_variables.begin(), data::detail::variable_name()),
                          boost::make_transform_iterator(summation_variables.end()  , data::detail::variable_name())
                         );
      }
      linear_process result = q;
      result = rename_process_parameters (result, used_names, "_S");
      result = rename_global_variables     (result, used_names, "_S");
      result = rename_summation_variables(result, used_names, "_S");
      return result;
    }

    /// \brief Initializes the name lookup table.
    /// \param model A linear process
    /// \param spec A linear process
    void init(const linear_process& model, const linear_process& spec)
    {
      summand_names.clear();
      set_summand_names(model);
      set_summand_names(spec);
      model_ptr = &model;
      spec_ptr  = &spec;
      assert(is_from_model(model));
      assert(!is_from_model(spec));
    }

    /// \brief Builds a pbes from the given equations.
    /// \param equations A sequence of pbes equations
    /// \param M A specification
    /// \param S A specification
    /// \return The constructed pbes
    pbes<> build_pbes(const atermpp::vector<pbes_equation>& equations,
                      const specification& M,
                      const specification& S
                     )
    {
      const linear_process& m = M.process();
      const linear_process& s = S.process();

      // TODO: the data of the two specification needs to be merged!
      data_specification data = M.data();
      propositional_variable_instantiation init(X(m, s), M.initial_process().state() + S.initial_process().state());

      pbes<> result(data, equations, init);
      assert(result.is_closed());
      return result;
    }
};

//--------------------------------------------------------------//
//                 branching bisimulation
//--------------------------------------------------------------//

/// \brief Algorithm class for branching bisimulation.
class branching_bisimulation_algorithm : public bisimulation_algorithm
{
  public:
    /// \brief The match function.
    /// \param p A linear process
    /// \param q A linear process
    /// \return The function result
    pbes_expression match(const linear_process& p, const linear_process& q) const
    {
      using namespace pbes_expr_optimized;
      std::vector<pbes_expression> result;
      for (my_iterator i = p.action_summands().begin(); i != p.action_summands().end(); ++i)
      {
        data_expression    ci = i->condition();
        const variable_list& d  = p.process_parameters();
        variable_list e  = i->summation_variables();
        const variable_list& d1 = q.process_parameters();
        pbes_expression expr = pbes_expr::forall(e, imp(ci, var(Y(p, q, i), d + d1 + e)));
        result.push_back(expr);
      }
      return join_and(result.begin(), result.end());
    }

    /// \brief The step function.
    /// \param p A linear process
    /// \param q A linear process
    /// \param i A summand iterator
    /// \return The function result
    pbes_expression step(const linear_process& p, const linear_process& q, my_iterator i) const
    {
      using namespace pbes_expr_optimized;
      const variable_list& d1 = q.process_parameters();
      data_expression_list gi = i->next_state(p.process_parameters());
      if (i->is_tau())
      {
        std::vector<pbes_expression> v;
        for (my_iterator j = q.action_summands().begin(); j != q.action_summands().end(); ++j)
        {
          if (!j->is_tau())
          {
            continue;
          }
          data_expression      cj = j->condition();
          variable_list        e1 = j->summation_variables();
          data_expression_list gj = j->next_state(q.process_parameters());
          pbes_expression      expr = pbes_expr::exists(e1, and_(cj, var(X(p, q), gi + gj)));
          v.push_back(expr);
        }
        return or_(join_or(v.begin(), v.end()), var(X(p, q), gi + d1));
      }
      else
      {
        std::vector<pbes_expression> v;
        for (my_iterator j = q.action_summands().begin(); j != q.action_summands().end(); ++j)
        {
          data_expression      cj = j->condition();
          variable_list        e1 = j->summation_variables();
          data_expression_list gj = j->next_state(q.process_parameters());
          multi_action         ai = i->multi_action();
          multi_action         aj = j->multi_action();
          pbes_expression expr = pbes_expr::exists(e1, and_(and_(cj, equals(ai, aj)), var(X(p, q), gi + gj)));
          v.push_back(expr);
        }
        return join_or(v.begin(), v.end());
      }
    }

    /// \brief The close function.
    /// \param p A linear process
    /// \param q A linear process
    /// \param i A summand iterator
    /// \return The function result
    pbes_expression close(const linear_process& p, const linear_process& q, my_iterator i) const
    {
      using namespace pbes_expr_optimized;
      std::vector<pbes_expression> v;
      const variable_list& d  = p.process_parameters();
      const variable_list& d1 = q.process_parameters();
      variable_list e  = i->summation_variables();
      for (my_iterator j = q.action_summands().begin(); j != q.action_summands().end(); ++j)
      {
        if (!j->is_tau())
        {
          continue;
        }
        data_expression       cj = j->condition();
        variable_list         e1 = j->summation_variables();
        data_expression_list  gj = j->next_state(q.process_parameters());
        pbes_expression       expr = pbes_expr::exists(e1, and_(cj, var(Y(p, q, i), d + gj + e)));
        v.push_back(expr);
      }
      return or_(join_or(v.begin(), v.end()), and_(var(X(p, q), d + d1), step(p, q, i)));
    }

    /// \brief Returns a pbes that expresses branching bisimulation between
    /// two specifications.
    /// \param model A linear process specification
    /// \param spec A linear process specification
    /// \return A pbes that expresses branching bisimulation between the
    /// two specifications.
    pbes<> run(const specification& model, const specification& spec)
    {
      using namespace pbes_expr_optimized;
      const linear_process& m = model.process();
      linear_process s = resolve_name_clashes(m, spec.process());
      init(m, s);

      const variable_list& d  = m.process_parameters();
      const variable_list& d1 = s.process_parameters();
      atermpp::vector<pbes_equation> equations;


      // E1
      equations.push_back(pbes_equation(nu(), propositional_variable(X(m, s), d + d1), and_(match(m, s), match(s, m))));
      equations.push_back(pbes_equation(nu(), propositional_variable(X(s, m), d1 + d), var(X(m, s), d + d1)));

      // E2
      for (my_iterator i = m.action_summands().begin(); i != m.action_summands().end(); ++i)
      {
        variable_list e  = i->summation_variables();
        pbes_equation e1(mu(), propositional_variable(Y(m, s, i), d + d1 + e), close(m, s, i));
        equations.push_back(e1);
      }
      for (my_iterator i = s.action_summands().begin(); i != s.action_summands().end(); ++i)
      {
        variable_list e  = i->summation_variables();
        pbes_equation e1(mu(), propositional_variable(Y(s, m, i), d1 + d + e), close(s, m, i));
        equations.push_back(e1);
      }

      specification spec1 = spec;
      spec1.process() = s;
      return build_pbes(equations, model, spec1);
    }
};

/// \brief Returns a pbes that expresses branching bisimulation between two specifications.
/// \param model A linear process specification
/// \param spec A linear process specification
/// \return A pbes that expresses branching bisimulation between the two specifications.
pbes<> branching_bisimulation(const specification& model, const specification& spec)
{
  return branching_bisimulation_algorithm().run(model, spec);
}

//--------------------------------------------------------------//
//                 strong bisimulation
//--------------------------------------------------------------//

/// \brief Algorithm class for strong bisimulation.
class strong_bisimulation_algorithm : public bisimulation_algorithm
{
  public:
    /// \brief The match function.
    /// \param p A linear process
    /// \param q A linear process
    /// \return The function result
    pbes_expression match(const linear_process& p, const linear_process& q) const
    {
      using namespace pbes_expr_optimized;
      std::vector<pbes_expression> result;
      for (my_iterator i = p.action_summands().begin(); i != p.action_summands().end(); ++i)
      {
        data_expression  ci = i->condition();
        variable_list    e  = i->summation_variables();
        pbes_expression  expr = pbes_expr::forall(e, imp(ci, step(p, q, i)));
        result.push_back(expr);
      }
      return join_and(result.begin(), result.end());
    }

    /// \brief The step function.
    /// \param p A linear process
    /// \param q A linear process
    /// \param i A summand iterator
    /// \return The function result
    pbes_expression step(const linear_process& p, const linear_process& q, my_iterator i) const
    {
      using namespace pbes_expr_optimized;
      data_expression_list gi = i->next_state(p.process_parameters());

      std::vector<pbes_expression> result;
      for (my_iterator j = q.action_summands().begin(); j != q.action_summands().end(); ++j)
      {
        data_expression       cj = j->condition();
        variable_list         e1 = j->summation_variables();
        data_expression_list  gj = j->next_state(q.process_parameters());
        multi_action          ai = i->multi_action();
        multi_action          aj = j->multi_action();
        pbes_expression       expr = pbes_expr::exists(e1, and_(and_(cj, equals(ai, aj)), var(X(p, q), gi + gj)));
        result.push_back(expr);
      }
      return join_or(result.begin(), result.end());
    }

    /// \brief Runs the algorithm
    /// \param model A linear process specification
    /// \param spec A linear process specification
    /// \return A pbes that expresses strong bisimulation between stwo specifications.
    pbes<> run(const specification& model, const specification& spec)
    {
      using namespace pbes_expr_optimized;
      const linear_process& m = model.process();
      linear_process s = resolve_name_clashes(m, spec.process());
      init(m, s);

      const variable_list& d  = m.process_parameters();
      const variable_list& d1 = s.process_parameters();
      atermpp::vector<pbes_equation> equations;


      // E
      equations.push_back(pbes_equation(nu(), propositional_variable(X(m, s), d + d1), and_(match(m, s), match(s, m))));
      equations.push_back(pbes_equation(nu(), propositional_variable(X(s, m), d1 + d), var(X(m, s), d + d1)));

      specification spec1 = spec;
      spec1.process() = s;
      return build_pbes(equations, model, spec1);
    }
};

/// \brief Returns a pbes that expresses strong bisimulation between two specifications.
/// \param model A linear process specification
/// \param spec A linear process specification
/// \return A pbes that expresses strong bisimulation between the two specifications.
pbes<> strong_bisimulation(const specification& model, const specification& spec)
{
  return strong_bisimulation_algorithm().run(model, spec);
}

//--------------------------------------------------------------//
//                 weak bisimulation
//--------------------------------------------------------------//

/// \brief Algorithm class for weak bisimulation.
class weak_bisimulation_algorithm : public bisimulation_algorithm
{
  public:
    /// \brief The match function.
    /// \param p A linear process
    /// \param q A linear process
    /// \return The function result
    pbes_expression match(const linear_process& p, const linear_process& q) const
    {
      using namespace pbes_expr_optimized;
      std::vector<pbes_expression> result;
      for (my_iterator i = p.action_summands().begin(); i != p.action_summands().end(); ++i)
      {
        data_expression    ci = i->condition();
        const variable_list& d  = p.process_parameters();
        variable_list e  = i->summation_variables();
        const variable_list& d1 = q.process_parameters();
        pbes_expression expr = pbes_expr::forall(e, imp(ci, var(Y1(p, q, i), d + d1 + e)));
        result.push_back(expr);
      }
      return join_and(result.begin(), result.end());
    }

    /// \brief The step function.
    /// \param p A linear process
    /// \param q A linear process
    /// \param i A summand iterator
    /// \return The function result
    pbes_expression step(const linear_process& p, const linear_process& q, my_iterator i) const
    {
      using namespace pbes_expr_optimized;
      const variable_list& d1 = q.process_parameters();
      data_expression_list      gi = i->next_state(p.process_parameters());
      multi_action              ai = i->multi_action().actions();
      if (i->is_tau())
      {
        return close2(p, q, i, gi, data_expression_list(d1.begin(), d1.end()));
      }
      else
      {
        std::vector<pbes_expression> v;
        for (my_iterator j = q.action_summands().begin(); j != q.action_summands().end(); ++j)
        {
          data_expression      cj = j->condition();
          variable_list        e1 = j->summation_variables();
          data_expression_list gj = j->next_state(q.process_parameters());
          multi_action         aj = j->multi_action().actions();
          pbes_expression      expr = pbes_expr::exists(e1, and_(and_(cj, equals(ai, aj)), close2(p, q, i, gi, gj)));
          v.push_back(expr);
        }
        return join_or(v.begin(), v.end());
      }
    }

    /// \brief The close1 function.
    /// \param p A linear process
    /// \param q A linear process
    /// \param i A summand iterator
    /// \return The function result
    pbes_expression close1(const linear_process& p, const linear_process& q, my_iterator i) const
    {
      using namespace pbes_expr_optimized;
      std::vector<pbes_expression> v;
      const variable_list& d1 = q.process_parameters();
      for (my_iterator j = q.action_summands().begin(); j != q.action_summands().end(); ++j)
      {
        if (!j->is_tau())
        {
          continue;
        }
        data_expression      cj = j->condition();
        variable_list        e1 = j->summation_variables();
        data_expression_list gj = j->next_state(q.process_parameters());
        pbes_expression      expr = pbes_expr::exists(e1, and_(cj, var(Y1(p, q, i), d1 + gj + e1)));
        v.push_back(expr);
      }
      return or_(join_or(v.begin(), v.end()), step(p, q, i));
    }

    /// \brief The close function.
    /// \param p A linear process
    /// \param q A linear process
    /// \param i A summand iterator
    /// \param d A sequence of data expressions
    /// \param d1 A sequence of data expressions
    /// \return The function result
    pbes_expression close2(const linear_process& p, const linear_process& q, my_iterator i, data_expression_list const& d, data_expression_list const& d1) const
    {
      using namespace pbes_expr_optimized;

      //const variable_list& d  = p.process_parameters();
      //const variable_list& d1 = q.process_parameters();
      data_expression_list        gi = i->next_state(p.process_parameters());
      action_list                 ai = i->multi_action().actions();
      std::vector<pbes_expression> v;
      for (my_iterator j = q.action_summands().begin(); j != q.action_summands().end(); ++j)
      {
        if (!j->is_tau())
        {
          continue;
        }
        // d' == q.process_parameters()
        // e' == j->summand_variables()
        data_expression      cj  = j->condition();                        // cj == cj(d',e')
        data_expression_list gj  = j->next_state(q.process_parameters()); // gj == gj(d',e')
        variable_list        e1  = j->summation_variables();              // e1 == e'
        const variable_list& parameters = q.process_parameters();

        // replace d' by d1 (if needed)
        if (d1 != data_expression_list(parameters.begin(), parameters.end()))
        {
          cj = substitute(make_list_substitution(q.process_parameters(), d1), cj);
          gj = substitute(make_list_substitution(q.process_parameters(), d1), gj);
        }

        // replace e' (e1) by fresh variables e'' (e1_new)
        std::set<std::string> used_names = mcrl2::data::detail::find_variable_name_strings(atermpp::make_list(lps::linear_process_to_aterm(p), lps::linear_process_to_aterm(q)));
        variable_list e1_new = fresh_variables(e1, used_names);
        data_expression    cj_new = substitute(make_list_substitution(e1, e1_new), cj);
        data_expression_list gj_new = substitute(make_list_substitution(e1, e1_new), gj);

        pbes_expression expr = pbes_expr::exists(e1_new, and_(cj_new, var(Y2(p, q, i), d + gj_new)));
        v.push_back(expr);
      }
      return or_(var(X(p, q), d + d1), join_or(v.begin(), v.end()));
    }

    /// \brief Runs the algorithm
    /// \param model A linear process specification
    /// \param spec A linear process specification
    /// \return A pbes that expresses weak bisimulation between two specifications.
    pbes<> run(const specification& model, const specification& spec)
    {
      using namespace pbes_expr_optimized;
      const linear_process& m = model.process();
      linear_process s = resolve_name_clashes(m, spec.process());
      init(m, s);

      variable_list const& d  = m.process_parameters();
      variable_list const& d1 = s.process_parameters();
      atermpp::vector<pbes_equation> equations;


      // E1
      equations.push_back(pbes_equation(nu(), propositional_variable(X(m, s), d + d1), and_(match(m, s), match(s, m))));
      equations.push_back(pbes_equation(nu(), propositional_variable(X(s, m), d1 + d), var(X(m, s), d + d1)));

      // E2
      for (my_iterator i = m.action_summands().begin(); i != m.action_summands().end(); ++i)
      {
        variable_list e  = i->summation_variables();
        pbes_equation e1(mu(), propositional_variable(Y1(m, s, i), d + d1 + e), close1(m, s, i));
        pbes_equation e2(mu(), propositional_variable(Y2(m, s, i), d + d1), close2(m, s, i, data_expression_list(d.begin(), d.end()), data_expression_list(d1.begin(), d1.end())));
        equations.push_back(e1);
        equations.push_back(e2);
      }
      for (my_iterator i = s.action_summands().begin(); i != s.action_summands().end(); ++i)
      {
        variable_list e  = i->summation_variables();
        pbes_equation e1(mu(), propositional_variable(Y1(s, m, i), d1 + d + e), close1(s, m, i));
        pbes_equation e2(mu(), propositional_variable(Y2(s, m, i), d1 + d), close2(s, m, i, data_expression_list(d1.begin(), d1.end()), data_expression_list(d.begin(), d.end())));
        equations.push_back(e1);
        equations.push_back(e2);
      }

      specification spec1 = spec;
      spec1.process() = s;
      return build_pbes(equations, model, spec1);
    }
};

/// \brief Returns a pbes that expresses weak bisimulation between two specifications.
/// \param model A linear process specification
/// \param spec A linear process specification
/// \return A pbes that expresses weak bisimulation between the two specifications.
pbes<> weak_bisimulation(const specification& model, const specification& spec)
{
  return weak_bisimulation_algorithm().run(model, spec);
}

//--------------------------------------------------------------//
//                 branching simulation equivalence
//--------------------------------------------------------------//

/// \brief Algorithm class for branching simulation equivalence.
class branching_simulation_equivalence_algorithm : public branching_bisimulation_algorithm
{
  public:
    /// \brief Runs the algorithm
    /// \param model A linear process specification
    /// \param spec A linear process specification
    /// \return A pbes that expresses branching simulation equivalence between two specifications.
    pbes<> run(const specification& model, const specification& spec)
    {
      using namespace pbes_expr_optimized;
      const linear_process& m = model.process();
      linear_process s = resolve_name_clashes(m, spec.process());
      init(m, s);

      variable_list const& d  = m.process_parameters();
      variable_list const& d1 = s.process_parameters();
      atermpp::vector<pbes_equation> equations;


      // E1
      equations.push_back(pbes_equation(nu(), propositional_variable(X(m, s), d + d1), and_(match(m, s), match(s, m))));
      equations.push_back(pbes_equation(nu(), propositional_variable(X(s, m), d1 + d), var(X(m, s), d + d1)));

      // E2
      for (my_iterator i = m.action_summands().begin(); i != m.action_summands().end(); ++i)
      {
        variable_list e  = i->summation_variables();
        pbes_equation e1(mu(), propositional_variable(Y(m, s, i), d + d1 + e), close(m, s, i));
        equations.push_back(e1);
      }
      for (my_iterator i = s.action_summands().begin(); i != s.action_summands().end(); ++i)
      {
        variable_list e  = i->summation_variables();
        pbes_equation e1(mu(), propositional_variable(Y(s, m, i), d1 + d + e), close(s, m, i));
        equations.push_back(e1);
      }

      specification spec1 = spec;
      spec1.process() = s;
      return build_pbes(equations, model, spec1);
    }
};

/// \brief Returns a pbes that expresses branching simulation equivalence between two specifications.
/// \param model A linear process specification
/// \param spec A linear process specification
/// \return A pbes that expresses branching simulation equivalence between the two specifications.
pbes<> branching_simulation_equivalence(const specification& model, const specification& spec)
{
  return branching_simulation_equivalence_algorithm().run(model, spec);
}

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_BISIMULATION_H
