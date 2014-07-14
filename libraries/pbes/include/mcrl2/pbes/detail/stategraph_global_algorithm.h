// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/detail/stategraph_global_algorithm.h
/// \brief add your file description here.

#ifndef MCRL2_PBES_DETAIL_STATEGRAPH_GLOBAL_ALGORITHM_H
#define MCRL2_PBES_DETAIL_STATEGRAPH_GLOBAL_ALGORITHM_H

#include "mcrl2/data/substitutions/sequence_sequence_substitution.h"
#include "mcrl2/pbes/algorithms.h"
#include "mcrl2/pbes/detail/stategraph_algorithm.h"
#include "mcrl2/pbes/detail/stategraph_global_graph.h"
#include "mcrl2/utilities/detail/container_utility.h"

namespace mcrl2 {

namespace pbes_system {

namespace detail {

/// \brief Algorithm class for the global variant of the stategraph algorithm
class stategraph_global_algorithm: public stategraph_algorithm
{
  public:
    typedef stategraph_algorithm super;
    typedef stategraph_global_graph::vertex_iterator vertex_iterator;

  protected:
    // the control flow graph
    stategraph_global_graph m_control_flow_graph;

    // removes parameter values that do not correspond to a control flow parameter
    propositional_variable_instantiation project(const propositional_variable_instantiation& x) const
    {
      auto const& X = x.name();
      auto const& d_X = x.parameters();
      const std::vector<bool>& b = is_GCFP(X);
      std::size_t index = 0;
      std::vector<data::data_expression> d;
      for (auto i = d_X.begin(); i != d_X.end(); ++i, index++)
      {
        assert(index < b.size());
        if (b[index])
        {
          d.push_back(*i);
        }
      }
      return propositional_variable_instantiation(X, data::data_expression_list(d.begin(), d.end()));
    }

    // removes parameter values that do not correspond to a control flow parameter
    propositional_variable project_variable(const propositional_variable& x) const
    {
      auto const& X = x.name();
      auto const& d_X = x.parameters();
      const std::vector<bool>& b = is_GCFP(X);
      std::size_t index = 0;
      std::vector<data::variable> d;
      for (auto i = d_X.begin(); i != d_X.end(); ++i, index++)
      {
        if (b[index])
        {
          d.push_back(*i);
        }
      }
      return propositional_variable(X, data::variable_list(d.begin(), d.end()));
    }

    data::data_expression_list put_dest(const predicate_variable& PV, const stategraph_equation& eqn, std::size_t i) const
    {
      data::data_expression_vector PV_args;
      size_t j = 0;
      for(auto ai = PV.parameters().begin(); ai != PV.parameters().end(); ++ai)
      {
        const std::map<std::size_t, data::data_expression>& target = eqn.predicate_variables()[i].target();
        auto dij = target.find(j);
        if(dij != target.end())
        {
          PV_args.push_back(dij->second);
        }
        else
        {
          PV_args.push_back(*ai);
        }
        ++j;
      }
      return data::data_expression_list(PV_args.begin(), PV_args.end());
    }

    void compute_global_control_flow_graph()
    {
      using utilities::detail::pick_element;

      mCRL2log(log::debug, "stategraph") << "=== compute control flow graph ===" << std::endl;

      pbes_system::simplify_data_rewriter<data::rewriter> pbesr(m_datar);

      std::set<stategraph_vertex*> todo;

      // handle the initial state
      propositional_variable_instantiation u0 = project(m_pbes.initial_state());
      vertex_iterator i = m_control_flow_graph.insert_vertex(u0);
      todo.insert(&(i->second));
      mCRL2log(log::debug, "stategraph") << "u0 = " << pbes_system::pp(u0) << std::endl;

      while (!todo.empty())
      {
        stategraph_vertex& u = *pick_element(todo);
        stategraph_vertex* source = &u;
        mCRL2log(log::debug, "stategraph") << "selected todo element u = " << pbes_system::pp(u.X) << std::endl;

        const stategraph_equation& eqn = *find_equation(m_pbes, u.X.name());
        mCRL2log(log::debug, "stategraph") << "eqn = " << eqn.print() << std::endl;
        propositional_variable X = project_variable(eqn.variable());
        data::variable_list d = X.parameters();
        data::data_expression_list e = u.X.parameters();
        data::sequence_sequence_substitution<data::variable_list, data::data_expression_list> sigma(d, e);

        u.sig = pbes_system::algorithms::significant_variables(pbesr(eqn.formula(), sigma));

        const std::vector<predicate_variable>& predvars = eqn.predicate_variables();
        for (std::size_t i = 0; i < predvars.size(); i++)
        {
          const predicate_variable& PV = predvars[i];

          mCRL2log(log::debug, "stategraph") << "Y(e) = " << PV << std::endl;
          pbes_expression g = pbesr(PV.guard(), sigma);
          mCRL2log(log::debug, "stategraph") << "g = " << pbes_system::pp(PV.guard()) << sigma << " = " << g << std::endl;
          if (is_false(g))
          {
            continue;
          }

          data::data_expression_list PV_args = put_dest(PV, eqn, i);
          propositional_variable_instantiation Ye = propositional_variable_instantiation(PV.variable().name(), data::data_expression_list(PV_args.begin(), PV_args.end()));

          Ye = core::down_cast<propositional_variable_instantiation>(pbesr(Ye, sigma));
          propositional_variable_instantiation Y = project(Ye);

          mCRL2log(log::debug, "stategraph") << "v = " << pbes_system::pp(Y) << std::endl;

          vertex_iterator q = m_control_flow_graph.find(Y);
          bool has_vertex = q != m_control_flow_graph.end();
          if (!has_vertex)
          {
            mCRL2log(log::debug, "stategraph") << "insert vertex v" << std::endl;
            q = m_control_flow_graph.insert_vertex(Y);
            todo.insert(&(q->second));
          }
          stategraph_vertex& v = q->second;
          stategraph_vertex* target = &v;
          stategraph_edge e(source, target, i);
          mCRL2log(log::debug, "stategraph") << "insert edge (u, v) with label " << i << std::endl;
          u.outgoing_edges.insert(e);
          v.incoming_edges.insert(e);
        }
      }

      m_control_flow_graph.create_index();
    }

    // used for printing the control flow graph
    std::map<core::identifier_string, data::variable_list> print_map() const
    {
      std::map<core::identifier_string, data::variable_list> result;
      const std::vector<stategraph_equation>& eqn = m_pbes.equations();
      for (std::vector<stategraph_equation>::const_iterator i = eqn.begin(); i != eqn.end(); ++i)
      {
        propositional_variable X = project_variable(i->variable());
        result[X.name()] = X.parameters();
      }
      return result;
    }

    // eq_X is the equation corresponding to u = (X, e)
    // eq_Y is the equation corresponding to Yf
    bool enabled_edge(const global_control_flow_graph_vertex& u, const stategraph_equation& eq_X, const predicate_variable& Yf, const stategraph_equation& eq_Y)
    {
      auto const& e = u.values();
      auto const& cfp_X = eq_X.control_flow_parameter_indices();
      for (std::size_t k = 0; k < cfp_X.size(); k++)
      {
// This is slightly more efficient:
//        auto q = Yf.source(cfp_X[k]);
//        if (q != data::undefined_data_expression() && q != nth_element(e, k))
        if (Yf.source(cfp_X[k]) != nth_element(e, k))
        {
          return false;
        }
      }
      return true;
    }

    const global_control_flow_graph_vertex& compute_vertex(global_control_flow_graph& G, const global_control_flow_graph_vertex& u, const stategraph_equation& eq_X, const predicate_variable& Yf, const stategraph_equation& eq_Y)
    {
      data::data_expression_vector f;
      auto const& X = u.name();
      auto const& e = u.values();
      auto const& Y = Yf.name();
      auto const& cfp_X = eq_X.control_flow_parameter_indices();
      auto const& cfp_Y = eq_Y.control_flow_parameter_indices();

      mCRL2log(log::debug1, "stategraph") << "compute_vertex X = " << X << ", e = " << core::detail::print_list(e) << ", Yf = " << Yf << std::endl;
      mCRL2log(log::debug1, "stategraph") << "cfp_X = " << core::detail::print_list(cfp_X) << std::endl;
      mCRL2log(log::debug1, "stategraph") << "cfp_Y = " << core::detail::print_list(cfp_Y) << std::endl;

      for (std::size_t l = 0; l < cfp_Y.size(); l++)
      {
        auto q = Yf.target(cfp_Y[l]);
        if (q != data::undefined_data_expression())
        {
          f.push_back(q);
        }
        else
        {
          std::size_t cfpXk = m_GCFP_graph.related_parameter_index(Y, cfp_Y[l], X);
          std::size_t k;
          for (k = 0; k < cfp_X.size(); ++k)
          {
            if (cfp_X[k] == cfpXk)
            {
              break;
            }
          }
          assert(k != cfp_X.size());
          f.push_back(nth_element(e, k));
        }
      }
      return G.insert_vertex(global_control_flow_graph_vertex(Yf.variable().name(), data::data_expression_list(f.begin(), f.end())));
    }

    void compute_global_control_flow_graph_new()
    {
      using utilities::detail::contains;
      using utilities::detail::pick_element;

      mCRL2log(log::debug, "stategraph") << "--- compute_global_control_flow_graph_new" << std::endl;
      global_control_flow_graph G;
      std::set<const global_control_flow_graph_vertex*> todo;
      std::set<const global_control_flow_graph_vertex*> done;

      // initialize todo
      auto const& Xinit = m_pbes.initial_state();
      auto const& eq_X = *find_equation(m_pbes, Xinit.name());
      auto einit = eq_X.project(Xinit.parameters());
      auto const& u = G.insert_vertex(global_control_flow_graph_vertex(Xinit.name(), einit));
      todo.insert(&u);

      while (!todo.empty())
      {
        // u = (X, e)
        auto const& u = *pick_element(todo);
        auto const& X = u.name();
        auto const& eq_X = *find_equation(m_pbes, X);
        auto const& predvars = eq_X.predicate_variables();

        mCRL2log(log::debug1, "stategraph") << "choose todo element " << u << std::endl;

        for (std::size_t i = 0; i < predvars.size(); i++)
        {
          auto const& Yf = predvars[i];
          auto const& Y = Yf.variable().name();
          auto const& eq_Y = *find_equation(m_pbes, Y);
          if (enabled_edge(u, eq_X, Yf, eq_Y))
          {
            const global_control_flow_graph_vertex& v = compute_vertex(G, u, eq_X, Yf, eq_Y);
            G.insert_edge(u, i, v);
            if (!contains(done, &v))
            {
              mCRL2log(log::debug1, "stategraph") << "insert todo element " << v << std::endl;
              todo.insert(&v);
              done.insert(&v);
            }
          }
        }
      }
      mCRL2log(log::debug, "stategraph") << "--- computed new global control flow graph ---\n" << G << std::endl;
    }

  public:
    stategraph_global_algorithm(const pbes& p, const pbesstategraph_options& options)
      : stategraph_algorithm(p, options)
    { }

    /// \brief Computes the control flow graph
    void run()
    {
      super::run();
      start_timer("compute_global_control_flow_graph");
      compute_global_control_flow_graph();
      finish_timer("compute_global_control_flow_graph");
      mCRL2log(log::verbose) << "Computed global control flow graph" << std::endl;
      mCRL2log(log::debug) << m_control_flow_graph.print(print_map());
      compute_global_control_flow_graph_new();
    }
};

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_DETAIL_STATEGRAPH_GLOBAL_ALGORITHM_H
