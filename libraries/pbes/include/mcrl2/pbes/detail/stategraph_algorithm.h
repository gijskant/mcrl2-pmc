// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/detail/stategraph_algorithm.h
/// \brief add your file description here.

#ifndef MCRL2_PBES_DETAIL_STATEGRAPH_ALGORITHM_H
#define MCRL2_PBES_DETAIL_STATEGRAPH_ALGORITHM_H

#include <sstream>
#include <boost/bind.hpp>
#include "mcrl2/data/detail/print_utility.h"
#include "mcrl2/data/detail/sorted_sequence_algorithm.h"
#include "mcrl2/pbes/detail/stategraph_graph.h"
#include "mcrl2/pbes/detail/stategraph_utility.h"
#include "mcrl2/utilities/sequence.h"

namespace mcrl2 {

namespace pbes_system {

namespace detail {

class control_flow_graph_vertex
{
  protected:
    core::identifier_string m_name;
    std::size_t m_index;
    data::variable m_variable;
    std::set<control_flow_graph_vertex*> m_neighbors;

  public:
    control_flow_graph_vertex(const core::identifier_string& name, std::size_t index, const data::variable& variable)
      : m_name(name), m_index(index), m_variable(variable)
    {}

    const core::identifier_string& name() const
    {
      return m_name;
    }

    std::size_t index() const
    {
      return m_index;
    }

    const data::variable& variable() const
    {
      return m_variable;
    }

    const std::set<control_flow_graph_vertex*>& neighbors() const
    {
      return m_neighbors;
    }

    std::set<control_flow_graph_vertex*>& neighbors()
    {
      return m_neighbors;
    }
};

inline
std::ostream& operator<<(std::ostream& out, const control_flow_graph_vertex& u)
{
  return out << '(' << u.name() << ", " << u.index() << ", " << data::pp(u.variable()) << ')';
}

/// \brief Algorithm class for the computation of the stategraph graph
class stategraph_algorithm
{
  public:
    // simplify and rewrite the expression x
    pbes_expression simplify(const pbes_expression& x) const
    {
      data::detail::simplify_rewriter r;
      stategraph_simplifying_rewriter<pbes_expression, data::detail::simplify_rewriter> R(r);
      return R(x);
    }

    // simplify and rewrite the guards of the pbes p
    void simplify(stategraph_pbes& p) const
    {
      std::vector<stategraph_equation>& equations = p.equations();
      for (std::size_t k = 0; k < equations.size(); k++)
      {
        stategraph_equation& eqn = equations[k];
        std::vector<predicate_variable>& predvars = eqn.predicate_variables();
        for (std::size_t i = 0; i < predvars.size(); i++)
        {
          predvars[i].guard = simplify(predvars[i].guard);
        }
      }
    }

    typedef control_flow_graph::vertex_iterator vertex_iterator;

  protected:
    // the pbes that is considered
    stategraph_pbes m_pbes;

    // a data rewriter
    data::rewriter m_datar;

    // determines how local control flow parameters are computed
    //
    // Keuze uit twee alternatieven voor de berekening van lokale CFPs.
    //  * Een parameter d^X[n] is een LCFP indien voor alle i waarvoor geldt pred(phi_X,i) = X, danwel:
    //          a. copy(X,i,n) is undefined en source(X,i,n) and dest(X,i,n) zijn beide defined, of
    //          b. copy(X,i,n) is defined (en gelijk aan n) en source(X,i,n) en dest(X,i,n) zijn beide undefined.
    //  * Een parameter d^X[n] is een LCFP indien voor alle i waarvoor geldt pred(phi_X,i) = X, danwel:
    //          a. source(X,i,n) and dest(X,i,n) zijn beide defined, of
    //          b. copy(X,i,n) is defined en gelijk aan n.
    //
    // De eerste definieert in feite een exclusive or, terwijl de tweede een standaard or is.
    bool m_use_alternative_lcfp_criterion;

    // determines how global control flow parameters are related
    //
    // Keuze uit twee alternatieven voor het relateren van CFPs.
    //  * Parameters d^X[n] and d^Y[m] zijn gerelateerd als danwel:
    //         a. er is een i z.d.d. copy(X, i, n) = m, of
    //         b. er is een i z.d.d. copy(Y, i, m) = n
    //  * Parameters d^X[n] and d^Y[m] zijn gerelateerd als danwel:
    //         a. er is een i z.d.d. copy(X, i, n) = m, en dest(X, i, m) is ongedefinieerd, of
    //         b. er is een i z.d.d. copy(Y, i, m) = n en dest(Y, i, n) is ongedefinieerd.
    // Hier zit het verschil er dus in dat we, in het tweede geval, parameters alleen relateren als er een copy is
    // van de een naar de ander EN de dest in dat geval ongedefinieerd is.
    bool m_use_alternative_gcfp_relation;

    // determines how global control flow parameters are selected
    //
    // Keuze voor de selectie van globale CFPs (of globale consistentie eisen).
    //  * Een set van CFPs is consistent als voor elke d^X[n], en voor alle Y in bnd(E)\{X} (dus in alle andere vergelijkingen), voor alle i waarvoor geldt pred(phi_Y, i) = X, danwel:
    //         a. dest(Y, i, n) is gedefinieerd, of
    //         b. copy(Y, i, m) = n voor een of andere globale CFP d^Y[m]
    // Deze eis is in principe voldoende om globale CFPs te identificeren. Als we echter een strikte scheiding tussen control flow parameters en data parameters willen bewerkstelligen, dan moet hier optioneel de volgende eis aan toegevoegd worden:
    //  * Een set van CFPs is consistent als de voorgaande eisen gelden, en bovendien voor elke d^X[n] geldt dat voor alle i waarvoor pred(phi_X, i) = Y != X, als d^X[n] affects data(phi_X, i)[m], dan is d^Y[m] een globale control flow parameter.
    // Waar de eerste gemarkeerd is als "detect conflicts for parameters of Y in equations of the form X(d) = ... Y(e)"
    // en de tweede als "detect conflicts for parameters of X in equations of the form X(d) = ... Y(e)".
    bool m_use_alternative_gcfp_consistency;

    // the control flow parameters
    std::map<core::identifier_string, std::vector<bool> > m_is_control_flow;

    // the local control flow parameters
    std::map<core::identifier_string, std::vector<bool> > m_is_LCFP;

    // the global control flow parameters
    std::map<core::identifier_string, std::vector<bool> > m_is_GCFP;

    // the vertices of the control flow graph
    std::vector<control_flow_graph_vertex> m_control_flow_graph_vertices;

    // the connected components in the control flow graph; a component contains the indices in the vector m_control_flow_graph_vertices
    std::vector<std::set<std::size_t> > m_connected_components;

    // the control flow graph(s)
    std::vector<control_flow_graph> m_control_flow_graphs;

    // for readability
    std::set<data::variable> FV(const data::data_expression& x) const
    {
      return data::find_free_variables(x);
    }

    // for readability
    template <typename Set>
    bool contains(const Set& S, const typename Set::value_type& s) const
    {
      return S.find(s) != S.end();
    }

    propositional_variable find_propvar(const pbes& p, const core::identifier_string& X) const
    {
      const std::vector<pbes_equation>& equations = p.equations();
      for (std::vector<pbes_equation>::const_iterator i = equations.begin(); i != equations.end(); ++i)
      {
        if (i->variable().name() == X)
        {
          return i->variable();
        }
      }
      throw mcrl2::runtime_error("find_propvar failed!");
      return propositional_variable();
    }

  public:
    std::string print_control_flow_parameters()
    {
      std::ostringstream out;
      out << "--- control flow parameters ---" << std::endl;
      const std::vector<stategraph_equation>& equations = m_pbes.equations();
      for (std::vector<stategraph_equation>::const_iterator k = equations.begin(); k != equations.end(); ++k)
      {
        propositional_variable X = k->variable();
        const std::vector<data::variable>& dX = k->parameters();
        const std::vector<bool>& cf = m_is_control_flow[X.name()];

        out << core::pp(X.name()) << " ";
        for (std::size_t i = 0; i < cf.size(); ++i)
        {
          if (cf[i])
          {
            out << data::pp(dX[i]) << " ";
          }
        }
        out << std::endl;
      }
      return out.str();
    }

    std::string print_propvar_parameter(const core::identifier_string& X, std::size_t index) const
    {
      return "(" + core::pp(X) + ", " + data::pp(find_equation(m_pbes, X)->parameters()[index]) + ")";
    }

    std::string print_stategraph_assignment(bool stategraph_value,
                                              std::size_t index,
                                              const pbes_system::propositional_variable& X,
                                              const pbes_system::propositional_variable_instantiation& Y,
                                              const std::string& message,
                                              const data::variable& previous_value = data::variable()
                                             ) const
    {
      std::ostringstream out;
      out << message << ": " << print_propvar_parameter(Y.name(), index) << " -> " << std::boolalpha << stategraph_value;
      out << " because of equation " << core::pp(X.name());
      data::variable_list v = X.parameters();
      if (v.size() > 0)
      {
        out << "(";
        for (data::variable_list::iterator i = v.begin(); i != v.end(); ++i)
        {
          if (i != v.begin())
          {
            out << ", ";
          }
          out << core::pp(i->name());
        }
        out << ")";
      }
      out << " = ... " << pbes_system::pp(Y) << " index = " << index << " " << data::pp(previous_value) << std::endl;
      return out.str();
    }

    // The result is stored in m_is_control_flow.
    void compute_control_flow_parameters()
    {
      mCRL2log(log::debug, "stategraph") << "=== compute control flow parameters ===" << std::endl;
      compute_local_control_flow_parameters();
      compute_global_control_flow_parameters();
      compute_related_global_control_flow_parameters();
      compute_connected_components();
      mCRL2log(log::verbose, "stategraph") << print_control_flow_parameters();
    }

    std::string print_local_control_flow_parameters()
    {
      std::ostringstream out;
      out << "--- local control flow parameters ---" << std::endl;
      const std::vector<stategraph_equation>& equations = m_pbes.equations();
      for (std::vector<stategraph_equation>::const_iterator k = equations.begin(); k != equations.end(); ++k)
      {
        propositional_variable X = k->variable();
        const std::vector<data::variable>& dX = k->parameters();
        const std::vector<bool>& cf = m_is_LCFP[X.name()];

        out << core::pp(X.name()) << " ";
        for (std::size_t i = 0; i < cf.size(); ++i)
        {
          if (cf[i])
          {
            out << data::pp(dX[i]) << " ";
          }
        }
        out << std::endl;
      }
      return out.str();
    }

    // returns true if m is not a key in the map
    template <typename Map>
    bool is_undefined(const Map& m, const typename Map::key_type& key) const
    {
      return m.find(key) == m.end();
    }

    // returns true if m does not map key to value
    template <typename Map>
    bool is_mapped_to(const Map& m, const typename Map::key_type& key, const typename Map::mapped_type& value) const
    {
      auto i = m.find(key);
      return i != m.end() && i->second == value;
    }

    bool maps_to_and_is_GFCP(const std::map<std::size_t, std::size_t>& m, std::size_t value, const core::identifier_string& X)
    {
      for (auto i = m.begin(); i != m.end(); ++i)
      {
        if (i->second == value && m_is_GCFP[X][i->first])
        {
          return true;
        }
      }
      return false;
    }

    void compute_local_control_flow_parameters()
    {
      mCRL2log(log::debug, "stategraph") << "=== compute local control flow parameters ===" << std::endl;

      const std::vector<stategraph_equation>& equations = m_pbes.equations();

      // initialize all control flow parameters to true
      for (auto k = equations.begin(); k != equations.end(); ++k)
      {
        const core::identifier_string& X = k->variable().name();
        const std::vector<data::variable>& dX = k->parameters();
        m_is_LCFP[X] = std::vector<bool>(dX.size(), true);
      }

      for (auto k = equations.begin(); k != equations.end(); ++k)
      {
        const core::identifier_string& X = k->variable().name();
        const std::vector<data::variable>& dX = k->parameters();
        const std::vector<predicate_variable>& predvars = k->predicate_variables();
        for (auto i = predvars.begin(); i != predvars.end(); ++i)
        {
          const predicate_variable& PVI_X_i = *i;
          if (PVI_X_i.X.name() == X)
          {
            for (std::size_t n = 0; n < dX.size(); n++)
            {
              if (m_use_alternative_lcfp_criterion)
              {
                // Een parameter d^X[n] is een LCFP indien voor alle i waarvoor geldt pred(phi_X,i) = X, danwel:
                // 1. copy(X,i,n) is undefined en source(X,i,n) and dest(X,i,n) zijn beide defined, of
                // 2. copy(X,i,n) is defined en source(X,i,n) en dest(X,i,n) zijn beide undefined.
                if (PVI_X_i.copy.find(n) == PVI_X_i.copy.end())
                {
                  // copy(X,i,n) is undefined
                  if (is_undefined(PVI_X_i.source, n) || is_undefined(PVI_X_i.dest, n))
                  {
                    mCRL2log(log::debug, "stategraph") << "(" << X << ", " << n << ") is not an LCFP because of predicate variable " << pbes_system::pp(PVI_X_i.X) << "in equation " << X << " (copy and source/dest undefined)" << std::endl;
                    m_is_LCFP[X][n] = false;
                  }
                }
                else
                {
                  // copy(X,i,n) is defined
                  if (!is_undefined(PVI_X_i.source, n) || !is_undefined(PVI_X_i.dest, n))
                  {
                    mCRL2log(log::debug, "stategraph") << "(" << X << ", " << n << ") is not an LCFP because of predicate variable " << pbes_system::pp(PVI_X_i.X) << "in equation " << X << " (copy defined and source/dest defined)" << std::endl;
                    m_is_LCFP[X][n] = false;
                  }
                }
              }
              else
              {
                if (is_undefined(PVI_X_i.source, n) && is_undefined(PVI_X_i.dest, n) && !is_mapped_to(PVI_X_i.copy, n, n))
                {
                  mCRL2log(log::debug, "stategraph") << "parameter (" << X << ", " << n << ") is not an LCFP due to " << pbes_system::pp(PVI_X_i.X) << "(source and dest undefined, and no copy to self)" << std::endl;
                  m_is_LCFP[X][n] = false;
                }
              }
            }
          }
        }
      }
      mCRL2log(log::debug, "stategraph") << print_local_control_flow_parameters();
    }

    std::string print_global_control_flow_parameters()
    {
      std::ostringstream out;
      out << "--- global control flow parameters ---" << std::endl;
      const std::vector<stategraph_equation>& equations = m_pbes.equations();
      for (std::vector<stategraph_equation>::const_iterator k = equations.begin(); k != equations.end(); ++k)
      {
        propositional_variable X = k->variable();
        const std::vector<data::variable>& dX = k->parameters();
        const std::vector<bool>& cf = m_is_GCFP[X.name()];

        out << core::pp(X.name()) << " ";
        for (std::size_t i = 0; i < cf.size(); ++i)
        {
          if (cf[i])
          {
            out << data::pp(dX[i]) << " ";
          }
        }
        out << std::endl;
      }
      return out.str();
    }

    void compute_global_control_flow_parameters()
    {
      mCRL2log(log::debug, "stategraph") << "=== compute global control flow parameters ===" << std::endl;

      m_is_GCFP = m_is_LCFP;

      const std::vector<stategraph_equation>& equations = m_pbes.equations();

      bool changed;
      do
      {
        changed = false;

        // Detect conflicts for parameters of Y in equations of the form X(d) = ... Y(e)
        for (auto k = equations.begin(); k != equations.end(); ++k)
        {
          const core::identifier_string& X = k->variable().name();
          const std::vector<predicate_variable>& predvars = k->predicate_variables();
          for (auto i = predvars.begin(); i != predvars.end(); ++i)
          {
            const predicate_variable& PVI_X_i = *i;
            const core::identifier_string& Y = PVI_X_i.X.name();
            if (Y != X)
            {
              const stategraph_equation& eq_Y = *find_equation(m_pbes, Y);
              const std::vector<data::variable>& d_Y = eq_Y.parameters();
              for (std::size_t n = 0; n < d_Y.size(); n++)
              {
                if (is_undefined(PVI_X_i.dest, n) && (!maps_to_and_is_GFCP(PVI_X_i.copy, n, X)))
                {
                  if (m_is_GCFP[Y][n])
                  {
                    m_is_GCFP[Y][n] = false;
                    changed = true;
                    mCRL2log(log::debug, "stategraph") << "(" << core::pp(Y) << ", " << n << ") is not a GCFP because of predicate variable " << pbes_system::pp(PVI_X_i.X) << " in equation " << core::pp(X) << std::endl;
                  }
                }
              }
            }
          }
        }

        // Detect conflicts for parameters of X in equations of the form X(d) = ... Y(e)
        if (m_use_alternative_gcfp_consistency)
        {
          for (auto k = equations.begin(); k != equations.end(); ++k)
          {
            const core::identifier_string& X = k->variable().name();
            const std::vector<predicate_variable>& predvars = k->predicate_variables();
            const std::vector<data::variable>& dX = k->parameters();
            std::size_t M = dX.size();

            for (auto i = predvars.begin(); i != predvars.end(); ++i)
            {
              const predicate_variable& PVI_X_i = *i;
              const core::identifier_string& Y = PVI_X_i.X.name();
              if (Y == X)
              {
                continue;
              }
              const data::data_expression_list& e = PVI_X_i.X.parameters();
              std::size_t n = 0;
              for (auto ei = e.begin(); ei != e.end(); ++ei, ++n)
              {
                std::set<data::variable> V = data::find_free_variables(*ei);
                for (std::size_t m = 0; m < M; m++)
                {
                  if (m_is_GCFP[X][m] && !m_is_GCFP[Y][n] && (V.find(dX[m]) != V.end()))
                  {
                    m_is_GCFP[X][m] = false;
                    changed = true;
                    mCRL2log(log::debug, "stategraph") << "(" << core::pp(X) << ", " << m << ") is not a GCFP because of predicate variable " << pbes_system::pp(PVI_X_i.X) << " in equation " << core::pp(X) << std::endl;
                  }
                }
              }
            }
          }
        }
      }
      while (changed);

      mCRL2log(log::verbose, "stategraph") << print_global_control_flow_parameters();
    }

    bool is_control_flow_parameter(const core::identifier_string& X, std::size_t i) const
    {
      std::map<core::identifier_string, std::vector<bool> >::const_iterator j = m_is_control_flow.find(X);
      assert(j != m_is_control_flow.end());
      const std::vector<bool>& cf = j->second;
      assert(i < cf.size());
      return cf[i];
    }

    bool is_local_control_flow_parameter(const core::identifier_string& X, std::size_t i) const
    {
      std::map<core::identifier_string, std::vector<bool> >::const_iterator j = m_is_LCFP.find(X);
      assert(j != m_is_LCFP.end());
      const std::vector<bool>& cf = j->second;
      assert(i < cf.size());
      return cf[i];
    }

    bool is_global_control_flow_parameter(const core::identifier_string& X, std::size_t i) const
    {
      std::map<core::identifier_string, std::vector<bool> >::const_iterator j = m_is_GCFP.find(X);
      assert(j != m_is_GCFP.end());
      const std::vector<bool>& cf = j->second;
      assert(i < cf.size());
      return cf[i];
    }

    std::vector<control_flow_graph_vertex>::iterator find_vertex(const core::identifier_string& X, std::size_t n)
    {
      for (auto i = m_control_flow_graph_vertices.begin(); i != m_control_flow_graph_vertices.end(); ++i)
      {
        if (i->name() == X && i->index() == n)
        {
          return i;
        }
      }
      throw mcrl2::runtime_error("vertex not found in dependency graph");
      return m_control_flow_graph_vertices.end();
    }

    // relate (X, n) and (Y, m) in the dependency graph
    // \pre: the equation of X has a lower rank than the equation of Y
    void relate_control_flow_graph_vertices(const core::identifier_string& X, std::size_t n, const core::identifier_string& Y, std::size_t m)
    {
      control_flow_graph_vertex& u = *find_vertex(X, n);
      control_flow_graph_vertex& v = *find_vertex(Y, m);
      u.neighbors().insert(&v);
      v.neighbors().insert(&u);
    }

    void compute_related_global_control_flow_parameters()
    {
      mCRL2log(log::debug, "stategraph") << "=== compute related global control flow parameters ===" << std::endl;
      const std::vector<stategraph_equation>& equations = m_pbes.equations();

      // step 1: create vertices for the dependency graph
      for (auto k = equations.begin(); k != equations.end(); ++k)
      {
        const core::identifier_string& X = k->variable().name();
        const std::vector<data::variable>& dX = k->parameters();
        for (std::size_t n = 0; n < dX.size(); n++)
        {
          if (is_global_control_flow_parameter(X, n))
          {
            m_control_flow_graph_vertices.push_back(control_flow_graph_vertex(X, n, dX[n]));
          }
        }
      }

      // step 2: find related vertices
      for (auto k = equations.begin(); k != equations.end(); ++k)
      {
        const core::identifier_string& X = k->variable().name();
        const std::vector<predicate_variable>& predvars = k->predicate_variables();
        for (auto i = predvars.begin(); i != predvars.end(); ++i)
        {
          const predicate_variable& PVI_X_i = *i;
          const core::identifier_string& Y = PVI_X_i.X.name();
          for (auto j = PVI_X_i.copy.begin(); j != PVI_X_i.copy.end(); ++j)
          {
            std::size_t n = j->first;
            std::size_t m = j->second;
            if (is_global_control_flow_parameter(X, n) && is_global_control_flow_parameter(Y, m))
            {
              if (m_use_alternative_gcfp_relation)
              {
                mCRL2log(log::debug, "stategraph") << "(" << core::pp(X) << ", " << n << ") and (" << core::pp(Y) << ", " << m << ") are related "
                                                   << "because of recursion " << pp(PVI_X_i.X) << " in the equation for " << core::pp(X) << std::endl;
                relate_control_flow_graph_vertices(X, n, Y, m);
              }
              else
              {
                // Twee parameters zijn alleen gerelateerd als er een copy is van de een naar de ander,
                // EN dat de dest in dat geval ongedefinieerd is (dus we weten echt geen concrete waarde
                // voor de parameter op dat punt).
                if (is_undefined(PVI_X_i.dest, m))
                {
                  mCRL2log(log::debug, "stategraph") << "(" << core::pp(X) << ", " << n << ") and (" << core::pp(Y) << ", " << m << ") are related "
                                                       << "because of recursion " << pp(PVI_X_i.X) << " in the equation for " << core::pp(X) << " (dest is undefined)" << std::endl;
                  relate_control_flow_graph_vertices(X, n, Y, m);
                }
              }
            }
          }
        }
      }
    }

    // a connected component is valid if it does not contain two nodes (X, n) and (Y, m) with X == Y
    bool is_valid_connected_component(const std::set<std::size_t>& component) const
    {
      std::set<core::identifier_string> V;
      for (auto i = component.begin(); i != component.end(); ++i)
      {
        const core::identifier_string& X = m_control_flow_graph_vertices[*i].name();
        if (V.find(X) != V.end())
        {
          return false;
        }
        V.insert(X);
      }
      return true;
    }

    std::string print_connected_component(const std::set<std::size_t>& component) const
    {
      std::ostringstream out;
      out << "component {";
      for (auto i = component.begin(); i != component.end(); ++i)
      {
        if (i != component.begin())
        {
          out << ", ";
        }
        out << m_control_flow_graph_vertices[*i];
      }
      out << "}";
      if (!is_valid_connected_component(component))
      {
        out << " invalid";
      }
      return out.str();
    }

    void print_connected_components() const
    {
      for (auto i = m_connected_components.begin(); i != m_connected_components.end(); ++i)
      {
        mCRL2log(log::debug, "stategraph") << print_connected_component(*i) << std::endl;
      }
    }

    // compute the connected component belonging to the vertex m_control_flow_graph_vertices[i]
    std::set<std::size_t> compute_connected_component(std::size_t i, std::vector<bool>& done) const
    {
      std::set<std::size_t> todo;
      std::set<std::size_t> component;

      todo.insert(i);
      while (!todo.empty())
      {
        auto j = todo.begin();
        std::size_t u_index = *j;
        todo.erase(j);
        const control_flow_graph_vertex& u = m_control_flow_graph_vertices[u_index];
        done[u_index] = true;
        component.insert(u_index);

        for (auto k = u.neighbors().begin(); k != u.neighbors().end(); ++k)
        {
          const control_flow_graph_vertex* w = *k;
          std::size_t w_index = w - &(m_control_flow_graph_vertices.front());
          if (!done[w_index])
          {
            todo.insert(w_index);
          }
        }
      }
      return component;
    }

    void compute_connected_components()
    {
      mCRL2log(log::debug, "stategraph") << "=== compute connected components ===" << std::endl;

      // done[i] means that m_control_flow_graph_vertices[i] has been processed
      std::vector<bool> done(m_control_flow_graph_vertices.size(), false);

      for (std::size_t i = 0; i < done.size(); i++)
      {
        if (done[i])
        {
          continue;
        }
        std::set<std::size_t> component = compute_connected_component(i, done);
        m_connected_components.push_back(component);
        mCRL2log(log::debug, "stategraph") << print_connected_component(component) << std::endl;
      }
    }

    // removes the connected components V for which !is_valid_connected_component(V)
    void remove_invalid_connected_components()
    {
      m_connected_components.erase(std::remove_if(m_connected_components.begin(), m_connected_components.end(),
         !boost::bind(&stategraph_algorithm::is_valid_connected_component, this, _1)));
      mCRL2log(log::debug, "stategraph") << "Removed invalid connected components. The remaining components are:" << std::endl;
      print_connected_components();
    }

    // Returns true if dX[m] is not only copied.
    //
    // A CFP dX[m] is not only copied if
    //    (1) for some i    : source(X, i, m) is defined
    // or (2) for some Y, i : pred(phi_Y , i) = X and dest(Y, i, m) is defined
    bool is_not_only_copied(const core::identifier_string& X, std::size_t m) const
    {
      // check (1)
      const stategraph_equation& eq_X = *find_equation(m_pbes, X);
      const std::vector<predicate_variable>& predvars = eq_X.predicate_variables();
      for (auto i = predvars.begin(); i != predvars.end(); ++i)
      {
        if (!is_undefined(i->source, m))
        {
          return true;
        }
      }

      // check (2)
      const std::vector<stategraph_equation>& equations = m_pbes.equations();
      for (auto k = equations.begin(); k != equations.end(); ++k)
      {
        const std::vector<predicate_variable>& predvars = k->predicate_variables();
        for (auto i = predvars.begin(); i != predvars.end(); ++i)
        {
          if (i->X.name() == X && !is_undefined(i->dest, m))
          {
            return true;
          }
        }
      }
      return false;
    }

    // Returns true if all CFPs in component are 'only copied'
    bool has_only_copied_CFPs(const std::set<std::size_t>& component) const
    {
      for (auto i = component.begin(); i != component.end(); ++i)
      {
        const control_flow_graph_vertex& u = m_control_flow_graph_vertices[*i];
        auto X = u.name();
        auto m = u.index();
        if (is_not_only_copied(X, m))
        {
          return false;
        }
      }
      return true;
    }

    // Removes the connected components V that consist of CFPs that are only copied.
    void remove_only_copy_components()
    {
      m_connected_components.erase(std::remove_if(m_connected_components.begin(), m_connected_components.end(),
         boost::bind(&stategraph_algorithm::has_only_copied_CFPs, this, _1)));
      mCRL2log(log::debug, "stategraph") << "Removed only copy components. The remaining components are:" << std::endl;
      print_connected_components();
    }

    const std::vector<bool>& stategraph_values(const core::identifier_string& X) const
    {
      std::map<core::identifier_string, std::vector<bool> >::const_iterator i = m_is_control_flow.find(X);
      assert (i != m_is_control_flow.end());
      return i->second;
    }

    // returns the control flow parameters of the propositional variable with name X
    std::set<data::variable> control_flow_parameters(const core::identifier_string& X) const
    {
      std::set<data::variable> result;
      const std::vector<bool>& b = stategraph_values(X);
      const stategraph_equation& eqn = *find_equation(m_pbes, X);
      const std::vector<data::variable>& d = eqn.parameters();
      std::size_t index = 0;
      for (std::vector<data::variable>::const_iterator i = d.begin(); i != d.end(); ++i, index++)
      {
        if (b[index])
        {
          result.insert(*i);
        }
      }
      return result;
    }

    std::vector<std::size_t> control_flow_parameter_indices(const core::identifier_string& X) const
    {
      std::vector<std::size_t> result;
      const stategraph_equation& eqn = *find_equation(m_pbes, X);
      const std::vector<data::variable>& dX = eqn.parameters();
      for (std::size_t k = 0; k < dX.size(); k++)
      {
        if (is_control_flow_parameter(X, k))
        {
          result.push_back(k);
        }
      }
      return result;
    }

    std::vector<std::size_t> data_parameter_indices(const core::identifier_string& X) const
    {
      std::vector<std::size_t> result;
      const stategraph_equation& eqn = *find_equation(m_pbes, X);
      const std::vector<data::variable>& dX = eqn.parameters();
      for (std::size_t k = 0; k < dX.size(); k++)
      {
        if (!is_control_flow_parameter(X, k))
        {
          result.push_back(k);
        }
      }
      return result;
    }

    // returns the parameters of the propositional variable with name X
    std::set<data::variable> propvar_parameters(const core::identifier_string& X) const
    {
      const stategraph_equation& eqn = *find_equation(m_pbes, X);
      const std::vector<data::variable>& d = eqn.parameters();
      return std::set<data::variable>(d.begin(), d.end());
    }

    // removes parameter values that do not correspond to a control flow parameter
    propositional_variable_instantiation project(const propositional_variable_instantiation& x) const
    {
      core::identifier_string X = x.name();
      data::data_expression_list dX = x.parameters();
      const std::vector<bool>& b = stategraph_values(X);
      std::size_t index = 0;
      std::vector<data::data_expression> d;
      for (data::data_expression_list::iterator i = dX.begin(); i != dX.end(); ++i, index++)
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
      core::identifier_string X = x.name();
      data::variable_list dX = x.parameters();
      const std::vector<bool>& b = stategraph_values(X);
      std::size_t index = 0;
      std::vector<data::variable> d;
      for (data::variable_list::iterator i = dX.begin(); i != dX.end(); ++i, index++)
      {
        if (b[index])
        {
          d.push_back(*i);
        }
      }
      return propositional_variable(X, data::variable_list(d.begin(), d.end()));
    }

    void print_control_flow_graphs()
    {
      mCRL2log(log::debug, "stategraph") << "\n=== local control flow graphs ===" << std::endl;
      propositional_variable_instantiation X_init = m_pbes.initial_state();
      const core::identifier_string& X = X_init.name();
      const stategraph_equation& eq_X = *find_equation(m_pbes, X);
      const std::vector<data::variable>& dX = eq_X.parameters();
      std::vector<std::size_t> CFP = control_flow_parameter_indices(X);
      for (std::size_t i = 0; i < CFP.size(); i++)
      {
        std::size_t p = CFP[i];
        mCRL2log(log::debug, "stategraph") << "--- graph for control flow parameter " << data::pp(dX[p]) << " ---" << std::endl;
        // mCRL2log(log::debug, "stategraph") << m_control_flow_graphs[i].print(print_map(X, p)) << std::endl;
      }
    }

    template <typename Substitution>
    propositional_variable_instantiation apply_substitution(const propositional_variable_instantiation& X, Substitution sigma) const
    {
      return propositional_variable_instantiation(X.name(), data::replace_free_variables(X.parameters(), sigma));
    }

    // Notation:
    // use u, v for vertices in the control flow graph
    // use y, z for vertices in the must graph
    void compute_control_flow_graphs()
    {
      mCRL2log(log::debug, "stategraph") << "=== computing local control flow graphs ===" << std::endl;

      for (auto i = m_connected_components.begin(); i != m_connected_components.end(); ++i)
      {
        control_flow_graph G;
        const std::set<std::size_t>& component = *i;
        for (auto j = component.begin(); j != component.end(); ++j)
        {
          // const control_flow_graph_vertex& u = m_control_flow_graph_vertices[*j];
          // TODO: The current control flow graph has a different representation.
          // G.insert_vertex(u.name(), u.index());
        }
        // TODO: determine the edges of the graph G
        G.create_index();
        m_control_flow_graphs.push_back(G);
      }
      // print_control_flow_graphs();
    }

    stategraph_algorithm(const pbes& p, data::rewriter::strategy rewrite_strategy = data::jitty,
                         bool use_alternative_lcfp_criterion = false,
                         bool use_alternative_gcfp_relation = false,
                         bool use_alternative_gcfp_consistency = false
                        )
      : m_datar(p.data(), rewrite_strategy),
        m_use_alternative_lcfp_criterion(use_alternative_lcfp_criterion),
        m_use_alternative_gcfp_relation(use_alternative_gcfp_relation),
        m_use_alternative_gcfp_consistency(use_alternative_gcfp_consistency)
    {
      m_pbes = stategraph_pbes(p);
    }

    const std::vector<control_flow_graph_vertex>& control_flow_graph_vertices() const
    {
      return m_control_flow_graph_vertices;
    }

    const std::vector<std::set<std::size_t> >& connected_components() const
    {
      return m_connected_components;
    }

    std::string print(const control_flow_graph_vertex& u) const
    {
      std::ostringstream out;
      out << '(' << u.name() << ", " << find_equation(m_pbes, u.name())->parameters()[u.index()].name() << ')';
      return out.str();
    }

    /// \brief Computes the values that the CFPs in component can attain.
    std::set<data::data_expression> compute_values(const std::set<std::size_t>& component)
    {
      std::set<data::data_expression> result;

      // search for a node that corresponds to a variable in the init of the PBES
      const propositional_variable_instantiation& init = m_pbes.initial_state();
      const core::identifier_string& X = init.name();
      const stategraph_equation& eq_X = *find_equation(m_pbes, X);
      std::size_t N = eq_X.parameters().size();
      auto j =  m_control_flow_graph_vertices.end(); // the node that corresponds to a variable in the init of the PBES
      for (std::size_t n = 0; n < N; n++)
      {
        j = find_vertex(X, n);
        if (j != m_control_flow_graph_vertices.end())
        {
          result.insert(nth_element(init.parameters(), n));
          break;
        }
      }

      if (j != m_control_flow_graph_vertices.end())
      {
        // source(X, i, k) = v
        for (auto p = component.begin(); p != component.end(); ++p)
        {
          const control_flow_graph_vertex& u = m_control_flow_graph_vertices[*p];
          const core::identifier_string& X = u.name();
          std::size_t k = u.index();
          const stategraph_equation& eq_X = *find_equation(m_pbes, X);
          const std::vector<predicate_variable>& predvars = eq_X.predicate_variables();
          for (auto i = predvars.begin(); i != predvars.end(); ++i)
          {
            auto q = i->source.find(k);
            if (q != i->source.end())
            {
              const data::data_expression& v = q->second;
              result.insert(v);
            }
          }
        }

        // dest(X, i, k) = v
        for (auto p = component.begin(); p != component.end(); ++p)
        {
          const control_flow_graph_vertex& u = m_control_flow_graph_vertices[*p];
          const core::identifier_string& Y = u.name();
          std::size_t k = u.index();
          const stategraph_equation& eq_Y = *find_equation(m_pbes, Y);
          const std::vector<predicate_variable>& predvars = eq_Y.predicate_variables();
          for (auto i = predvars.begin(); i != predvars.end(); ++i)
          {
            if (i->X.name() != Y)
            {
              continue;
            }
            auto q = i->dest.find(k);
            if (q != i->dest.end())
            {
              const data::data_expression& v = q->second;
              result.insert(v);
            }
          }
        }
      }
      return result;
    }

    void compute_values()
    {
      for (auto i = m_connected_components.begin(); i != m_connected_components.end(); ++i)
      {
        std::set<data::data_expression> values = compute_values(*i);
        mCRL2log(log::debug, "stategraph") << print_connected_component(*i) << " values = " << data::detail::print_set(values) << std::endl;
      }
    }

    /// \brief Computes the control flow graph
    void run()
    {
      simplify(m_pbes);
      m_pbes.compute_source_dest_copy();
      mCRL2log(log::debug, "stategraph") << "--- source, dest, copy ---\n" << m_pbes.print_source_dest_copy() << std::endl;
      compute_control_flow_parameters();
      remove_invalid_connected_components();
      remove_only_copy_components();
      compute_values();
    }

    const stategraph_pbes& get_pbes() const
    {
      return m_pbes;
    }
};

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_DETAIL_STATEGRAPH_ALGORITHM_H
