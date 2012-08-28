// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/detail/control_flow.h
/// \brief add your file description here.

#ifndef MCRL2_PBES_DETAIL_CONTROL_FLOW_H
#define MCRL2_PBES_DETAIL_CONTROL_FLOW_H

#include <algorithm>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <vector>
#include "mcrl2/data/replace.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/data/standard.h"
#include "mcrl2/data/standard_utility.h"
#include "mcrl2/data/detail/simplify_rewrite_builder.h"
#include "mcrl2/data/detail/sorted_sequence_algorithm.h"
#include "mcrl2/pbes/find.h"
#include "mcrl2/pbes/rewrite.h"
#include "mcrl2/pbes/rewriter.h"
#include "mcrl2/pbes/detail/is_pfnf.h"
#include "mcrl2/pbes/detail/pfnf_pbes.h"
#include "mcrl2/pbes/detail/simplify_quantifier_builder.h"
#include "mcrl2/pbes/detail/control_flow_influence.h"
#include "mcrl2/pbes/detail/control_flow_source_dest.h"
#include "mcrl2/pbes/detail/control_flow_utility.h"
#include "mcrl2/utilities/logger.h"

namespace mcrl2 {

namespace pbes_system {

namespace detail {

// Adds some simplifications to simplify_rewrite_builder.
template <typename Term, typename DataRewriter, typename SubstitutionFunction = no_substitution>
struct control_flow_simplify_quantifier_builder: public pbes_system::detail::simplify_quantifier_builder<Term, DataRewriter, SubstitutionFunction>
{
  typedef pbes_system::detail::simplify_quantifier_builder<Term, DataRewriter, SubstitutionFunction> super;
  typedef SubstitutionFunction                                                                       argument_type;
  typedef typename super::term_type                                                                  term_type;
  typedef typename core::term_traits<term_type>::data_term_type                                      data_term_type;
  typedef typename core::term_traits<term_type>::data_term_sequence_type                             data_term_sequence_type;
  typedef typename core::term_traits<term_type>::variable_sequence_type                              variable_sequence_type;
  typedef typename core::term_traits<term_type>::propositional_variable_type                         propositional_variable_type;
  typedef core::term_traits<Term> tr;

  /// \brief Constructor.
  /// \param rewr A data rewriter
  control_flow_simplify_quantifier_builder(const DataRewriter& rewr)
    : simplify_quantifier_builder<Term, DataRewriter, SubstitutionFunction>(rewr)
  { }

  bool is_data_not(const pbes_expression& x) const
  {
    return data::is_data_expression(x) && data::sort_bool::is_not_application(x);
  }

  // replace !(y || z) by !y && !z
  // replace !(y && z) by !y || !z
  // replace !(y => z) by y || !z
  // replace y => z by !y || z
  term_type post_process(const term_type& x)
  {
    term_type result = x;
    if (tr::is_not(x))
    {
      term_type t = tr::not_arg(x);
      if (tr::is_and(t)) // x = !(y && z)
      {
        term_type y = utilities::optimized_not(tr::left(t));
        term_type z = utilities::optimized_not(tr::right(t));
        result = utilities::optimized_and(y, z);
      }
      else if (tr::is_or(t)) // x = !(y || z)
      {
        term_type y = utilities::optimized_not(tr::left(t));
        term_type z = utilities::optimized_not(tr::right(t));
        result = utilities::optimized_or(y, z);
      }
      else if (tr::is_imp(t)) // x = !(y => z)
      {
        term_type y = tr::left(t);
        term_type z = utilities::optimized_not(tr::right(t));
        result = utilities::optimized_or(y, z);
      }
      else if (is_data_not(t)) // x = !val(!y)
      {
        term_type y = data::application(t).arguments().front();
        result = y;
      }
    }
    else if (tr::is_imp(x)) // x = y => z
    {
      term_type y = utilities::optimized_not(tr::left(x));
      term_type z = tr::right(x);
      result = utilities::optimized_or(y, z);
    }
    return result;
  }

  // replace the data expression y != z by !(y == z)
  term_type visit_data_expression(const term_type& x, const data_term_type& d, SubstitutionFunction& sigma)
  {
    typedef core::term_traits<data::data_expression> tt;
    term_type result = super::visit_data_expression(x, d, sigma);
    data::data_expression t = result;
    if (data::is_not_equal_to_application(t)) // result = y != z
    {
      data::data_expression y = tt::left(t);
      data::data_expression z = tt::right(t);
      result = tr::not_(data::equal_to(y, z));
    }
    return post_process(result);
  }

  term_type visit_true(const term_type& x, SubstitutionFunction& sigma)
  {
    return post_process(super::visit_true(x, sigma));
  }

  term_type visit_false(const term_type& x, SubstitutionFunction& sigma)
  {
    return post_process(super::visit_false(x, sigma));
  }

  term_type visit_not(const term_type& x, const term_type& n, SubstitutionFunction& sigma)
  {
    return post_process(super::visit_not(x, n, sigma));
  }

  term_type visit_and(const term_type& x, const term_type& left, const term_type& right, SubstitutionFunction& sigma)
  {
    return post_process(super::visit_and(x, left, right, sigma));
  }

  term_type visit_or(const term_type& x, const term_type& left, const term_type& right, SubstitutionFunction& sigma)
  {
    return post_process(super::visit_or(x, left, right, sigma));
  }

  term_type visit_imp(const term_type& x, const term_type& left, const term_type& right, SubstitutionFunction& sigma)
  {
    return post_process(super::visit_imp(x, left, right, sigma));
  }

  term_type visit_forall(const term_type& x, const variable_sequence_type&  variables, const term_type&  expression, SubstitutionFunction& sigma)
  {
    return post_process(super::visit_forall(x, variables, expression, sigma));
  }

  term_type visit_exists(const term_type& x, const variable_sequence_type&  variables, const term_type&  expression, SubstitutionFunction& sigma)
  {
    return post_process(super::visit_exists(x, variables, expression, sigma));
  }

  term_type visit_propositional_variable(const term_type& x, const propositional_variable_type&  v, SubstitutionFunction& sigma)
  {
    return post_process(super::visit_propositional_variable(x, v, sigma));
  }
};

template <typename Term, typename DataRewriter>
class control_flow_simplifying_rewriter
{
  protected:
    DataRewriter m_rewriter;

  public:
    typedef typename core::term_traits<Term>::term_type term_type;
    typedef typename core::term_traits<Term>::variable_type variable_type;

    control_flow_simplifying_rewriter(const DataRewriter& rewriter)
      : m_rewriter(rewriter)
    {}

    term_type operator()(const term_type& x) const
    {
      control_flow_simplify_quantifier_builder<Term, DataRewriter> r(m_rewriter);
      return r(x);
    }

    template <typename SubstitutionFunction>
    term_type operator()(const term_type& x, SubstitutionFunction sigma) const
    {
      control_flow_simplify_quantifier_builder<Term, DataRewriter, SubstitutionFunction> r(m_rewriter);
      return r(x, sigma);
    }
};

/// \brief Algorithm class for the control_flow algorithm
class pbes_control_flow_algorithm
{
  public:
    struct control_flow_vertex;

    // edge of the control flow graph
    struct control_flow_edge
    {
      control_flow_vertex* source;
      control_flow_vertex* target;
      propositional_variable_instantiation label;

      control_flow_edge(control_flow_vertex* source_,
                        control_flow_vertex* target_,
                        const propositional_variable_instantiation& label_
                       )
       : source(source_),
         target(target_),
         label(label_)
       {}

      bool operator<(const control_flow_edge& other) const
      {
        if (source == other.source)
        {
          return target < other.target;
        }
        else
        {
          return source < other.source;
        }
      }
    };

    // vertex of the control flow graph
    struct control_flow_vertex
    {
      propositional_variable_instantiation X;
      std::set<control_flow_edge> incoming_edges;
      std::set<control_flow_edge> outgoing_edges;
      pbes_expression guard;
      std::set<data::variable> marking;

      control_flow_vertex(const propositional_variable_instantiation& X_, pbes_expression guard_ = true_())
        : X(X_), guard(guard_)
      {}

      std::string print() const
      {
        std::ostringstream out;
        out << pbes_system::pp(X);
        out << " edges:";
        for (std::set<control_flow_edge>::const_iterator i = outgoing_edges.begin(); i != outgoing_edges.end(); ++i)
        {
          out << " " << pbes_system::pp(i->target->X);
        }
        return out.str();
      }
    };

    struct control_flow_substitution
    {
      std::map<std::size_t, data::data_expression> values;

      propositional_variable_instantiation operator()(const propositional_variable_instantiation& x) const
      {
        data::data_expression_vector e = atermpp::convert<data::data_expression_vector>(x.parameters());
        for (std::map<std::size_t, data::data_expression>::const_iterator i = values.begin(); i != values.end(); ++i)
        {
          e[i->first] = i->second;
        }
        return propositional_variable_instantiation(x.name(), atermpp::convert<data::data_expression_list>(e));
      }
    };

    // simplify and rewrite the expression x
    pbes_expression simplify(const pbes_expression& x) const
    {
      data::detail::simplify_rewriter r;
      control_flow_simplifying_rewriter<pbes_expression, data::detail::simplify_rewriter> R(r);
      return R(x);
    }

    // simplify and rewrite the guards of the pbes p
    void simplify(pfnf_pbes& p) const
    {
      std::vector<pfnf_equation>& equations = p.equations();
      for (std::vector<pfnf_equation>::iterator k = equations.begin(); k != equations.end(); ++k)
      {
        simplify(k->h());
        std::vector<pfnf_implication>& implications = k->implications();
        for (std::vector<pfnf_implication>::iterator i = implications.begin(); i != implications.end(); ++i)
        {
          simplify(i->g());
        }
      }
    }

    pbes_control_flow_algorithm(const pbes<>& p)
      : m_pbes(p)
    {
      simplify(m_pbes);
    }

  protected:
    typedef atermpp::map<propositional_variable_instantiation, control_flow_vertex>::iterator vertex_iterator;

    // vertices of the control flow graph
    atermpp::map<propositional_variable_instantiation, control_flow_vertex> m_control_vertices;

    // the pbes that is considered
    pfnf_pbes m_pbes;

    // the control flow parameters
    std::map<core::identifier_string, std::vector<bool> > m_is_control_flow;

    propositional_variable find_propvar(const pbes<>& p, const core::identifier_string& X) const
    {
      const atermpp::vector<pbes_equation>& equations = p.equations();
      for (atermpp::vector<pbes_equation>::const_iterator i = equations.begin(); i != equations.end(); ++i)
      {
        if (i->variable().name() == X)
        {
          return i->variable();
        }
      }
      throw mcrl2::runtime_error("find_propvar failed!");
      return propositional_variable();
    }

    void print_control_flow_parameters()
    {
      std::cout << "--- control flow parameters ---" << std::endl;
      const std::vector<pfnf_equation>& equations = m_pbes.equations();
      for (std::vector<pfnf_equation>::const_iterator k = equations.begin(); k != equations.end(); ++k)
      {
        propositional_variable X = k->variable();
        const std::vector<data::variable>& d_X = k->parameters();
        const std::vector<bool>& cf = m_is_control_flow[X.name()];

        std::cout << core::pp(X.name()) << " ";
        for (std::size_t i = 0; i < cf.size(); ++i)
        {
          if (cf[i])
          {
            std::cout << data::pp(d_X[i]) << " ";
          }
        }
        std::cout << std::endl;
      }
    }

    std::string print_propvar_parameter(const core::identifier_string& X, std::size_t index) const
    {
      return "(" + core::pp(X) + ", " + data::pp(find_equation(m_pbes, X)->parameters()[index]) + ")";
    }

    std::string print_control_flow_assignment(bool control_flow_value,
                                              std::size_t index,
                                              const pbes_system::propositional_variable& X,
                                              const pbes_system::propositional_variable_instantiation& Xij,
                                              const std::string& message,
                                              const data::variable& previous_value = data::variable()
                                             ) const
    {
      std::ostringstream out;
      out << "[cf] " << message << ": " << print_propvar_parameter(Xij.name(), index) << " -> " << std::boolalpha << control_flow_value;
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
      out << " = ... " << pbes_system::pp(Xij) << " index = " << index << " " << data::pp(previous_value) << std::endl;
      return out.str();
    }

    void compute_control_flow_parameters()
    {
      const std::vector<pfnf_equation>& equations = m_pbes.equations();
      std::map<core::identifier_string, std::vector<data::variable> > V;

      // initialize all control flow parameters to true
      // initalize V_km to the empty set
      for (std::vector<pfnf_equation>::const_iterator k = equations.begin(); k != equations.end(); ++k)
      {
        propositional_variable X = k->variable();
        const std::vector<data::variable>& d_X = k->parameters();
        m_is_control_flow[X.name()] = std::vector<bool>(d_X.size(), true);
        V[X.name()] = std::vector<data::variable>(d_X.size(), data::variable());
      }

      // pass 1
      for (std::vector<pfnf_equation>::const_iterator k = equations.begin(); k != equations.end(); ++k)
      {
        propositional_variable X = k->variable();
        const std::vector<data::variable>& d_X = k->parameters();
        const std::vector<pfnf_implication>& implications = k->implications();
        for (std::vector<pfnf_implication>::const_iterator i = implications.begin(); i != implications.end(); ++i)
        {
          const std::vector<propositional_variable_instantiation>& propvars = i->variables();
          for (std::vector<propositional_variable_instantiation>::const_iterator j = propvars.begin(); j != propvars.end(); ++j)
          {
            const propositional_variable_instantiation& Xij = *j;
            data::data_expression_list d = Xij.parameters();
            std::size_t index = 0;
            for (data::data_expression_list::const_iterator q = d.begin(); q != d.end(); ++q, ++index)
            {
              if (data::is_variable(*q))
              {
                std::vector<data::variable>::const_iterator found = std::find(d_X.begin(), d_X.end(), *q);
                if (found != d_X.end())
                {
                  if (V[Xij.name()][index] == data::variable())
                  {
                    V[Xij.name()][index] = *q;
                    mCRL2log(log::debug, "control_flow") << print_control_flow_assignment(true, index, X, Xij, "pass 1");
                  }
                  else
                  {
                    bool is_same_value = (V[Xij.name()][index] == *q);
                    m_is_control_flow[Xij.name()][index] = is_same_value;
                    mCRL2log(log::debug, "control_flow") << print_control_flow_assignment(is_same_value, index, X, Xij, "pass 1", V[Xij.name()][index]);
                  }
                }
              }
            }
          }
        }
      }

      // pass 2
      for (std::vector<pfnf_equation>::const_iterator k = equations.begin(); k != equations.end(); ++k)
      {
        propositional_variable X = k->variable();
        const std::vector<data::variable>& d_X = k->parameters();
        const std::vector<pfnf_implication>& implications = k->implications();
        for (std::vector<pfnf_implication>::const_iterator i = implications.begin(); i != implications.end(); ++i)
        {
          const std::vector<propositional_variable_instantiation>& propvars = i->variables();
          for (std::vector<propositional_variable_instantiation>::const_iterator j = propvars.begin(); j != propvars.end(); ++j)
          {
            const propositional_variable_instantiation& Xij = *j;
            data::data_expression_list d = Xij.parameters();
            std::size_t index = 0;
            for (data::data_expression_list::const_iterator q = d.begin(); q != d.end(); ++q, ++index)
            {
              if (is_constant(*q))
              {
                continue;
              }
              else if (data::is_variable(*q))
              {
                std::vector<data::variable>::const_iterator found = std::find(d_X.begin(), d_X.end(), *q);
                if (found == d_X.end())
                {
                  m_is_control_flow[Xij.name()][index] = false;
                  mCRL2log(log::debug, "control_flow") << print_control_flow_assignment(false, index, X, Xij, "pass 2");
                }
                else
                {
                  if (X.name() == Xij.name() && (found != d_X.begin() + index))
                  {
                    m_is_control_flow[Xij.name()][index] = false;
                    mCRL2log(log::debug, "control_flow") << print_control_flow_assignment(false, index, X, Xij, "pass 2");
                  }
                }
              }
              else
              {
                m_is_control_flow[Xij.name()][index] = false;
                mCRL2log(log::debug, "control_flow") << print_control_flow_assignment(false, index, X, Xij, "pass 2");
              }
            }
          }
        }
      }
    }

    const std::vector<bool>& control_flow_values(const core::identifier_string& X) const
    {
      std::map<core::identifier_string, std::vector<bool> >::const_iterator i = m_is_control_flow.find(X);
      assert (i != m_is_control_flow.end());
      return i->second;
    }

    // returns the control flow parameters of the propositional variable with name X
    std::set<data::variable> control_flow_parameters(const core::identifier_string& X) const
    {
      std::set<data::variable> result;
      const std::vector<bool>& b = control_flow_values(X);
      const pfnf_equation& eqn = *find_equation(m_pbes, X);
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

    // returns the parameters of the propositional variable with name X
    std::set<data::variable> propvar_parameters(const core::identifier_string& X) const
    {
      const pfnf_equation& eqn = *find_equation(m_pbes, X);
    	const std::vector<data::variable>& d = eqn.parameters();
    	return std::set<data::variable>(d.begin(), d.end());
    }

    // removes parameter values that do not correspond to a control flow parameter
    propositional_variable_instantiation project(const propositional_variable_instantiation& x) const
    {
      core::identifier_string X = x.name();
      data::data_expression_list d_X = x.parameters();
      const std::vector<bool>& b = control_flow_values(X);
      std::size_t index = 0;
      std::vector<data::data_expression> d;
      for (data::data_expression_list::iterator i = d_X.begin(); i != d_X.end(); ++i, index++)
      {
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
      data::variable_list d_X = x.parameters();
      const std::vector<bool>& b = control_flow_values(X);
      std::size_t index = 0;
      std::vector<data::variable> d;
      for (data::variable_list::iterator i = d_X.begin(); i != d_X.end(); ++i, index++)
      {
        if (b[index])
        {
          d.push_back(*i);
        }
      }
      return propositional_variable(X, data::variable_list(d.begin(), d.end()));
    }

    // x is a projected value
    // \pre x is not present in m_control_vertices
    vertex_iterator insert_control_flow_vertex(const propositional_variable_instantiation& X, pbes_expression guard = true_())
    {
      std::pair<vertex_iterator, bool> p = m_control_vertices.insert(std::make_pair(X, control_flow_vertex(X, guard)));
      assert(p.second);
      return p.first;
    }

    template <typename Substitution>
    propositional_variable_instantiation apply_substitution(const propositional_variable_instantiation& X, Substitution sigma) const
    {
      return propositional_variable_instantiation(X.name(), data::replace_free_variables(X.parameters(), sigma));
    }

    void compute_control_flow_graph()
    {
      compute_control_flow_parameters();

      data::rewriter datar(m_pbes.data());
      pbes_system::simplifying_rewriter<pbes_expression, data::rewriter> pbesr(datar);

      std::set<control_flow_vertex*> todo;

      // handle the initial state
      propositional_variable_instantiation Xinit = project(m_pbes.initial_state());
      vertex_iterator i = insert_control_flow_vertex(Xinit);
      todo.insert(&(i->second));

      while (!todo.empty())
      {
        std::set<control_flow_vertex*>::iterator i = todo.begin();
        todo.erase(i);
        control_flow_vertex& v = **i;
        control_flow_vertex* source = &v;
        mCRL2log(log::debug, "control_flow") << "[cf] selected todo element " << pbes_system::pp(v.X) << std::endl;

        const pfnf_equation& eqn = *find_equation(m_pbes, v.X.name());
        propositional_variable X = project_variable(eqn.variable());
        data::variable_list d = X.parameters();
        data::data_expression_list e = v.X.parameters();
        data::sequence_sequence_substitution<data::variable_list, data::data_expression_list> sigma(d, e);

        const std::vector<pfnf_implication>& implications = eqn.implications();
        for (std::vector<pfnf_implication>::const_iterator i = implications.begin(); i != implications.end(); ++i)
        {
          const std::vector<propositional_variable_instantiation>& propvars = i->variables();
          pbes_expression guard = and_(eqn.h(), i->g());
          pbes_expression evaluate_guard = pbesr(guard, sigma);
          if (is_false(evaluate_guard))
          {
            continue;
          }

          for (std::vector<propositional_variable_instantiation>::const_iterator j = propvars.begin(); j != propvars.end(); ++j)
          {
            propositional_variable_instantiation Xij = project(*j);
            propositional_variable_instantiation Y = apply_substitution(Xij, sigma);
            propositional_variable_instantiation label = Xij;
            vertex_iterator q = m_control_vertices.find(Y);
            if (q == m_control_vertices.end())
            {
              // vertex Y does not yet exist
              mCRL2log(log::debug, "control_flow") << "[cf] discovered " << pbes_system::pp(Y) << std::endl;
              vertex_iterator k = insert_control_flow_vertex(Y, guard);
              todo.insert(&(k->second));
              mCRL2log(log::debug, "control_flow") << "[cf] added todo element " << pbes_system::pp(k->first) << std::endl;
              control_flow_vertex* target = &(k->second);
              v.outgoing_edges.insert(control_flow_edge(source, target, label));
            }
            else
            {
              control_flow_vertex* target = &(q->second);
              v.outgoing_edges.insert(control_flow_edge(source, target, label));
            }
          }
        }
      }

      // add incoming edges
      for (atermpp::map<propositional_variable_instantiation, control_flow_vertex>::iterator i = m_control_vertices.begin(); i != m_control_vertices.end(); ++i)
      {
        control_flow_vertex& v = i->second;
        for (std::set<control_flow_edge>::iterator j = v.outgoing_edges.begin(); j != v.outgoing_edges.end(); ++j)
        {
          const control_flow_edge& e = *j;
          e.target->incoming_edges.insert(e);
        }
      }
    }

    void print_control_flow_graph() const
    {
      std::cout << "--- control flow graph ---" << std::endl;
      for (atermpp::map<propositional_variable_instantiation, control_flow_vertex>::const_iterator i = m_control_vertices.begin(); i != m_control_vertices.end(); ++i)
      {
        std::cout << "vertex " << i->second.print() << std::endl;
      }
    }

    std::string print_variable_set(const std::set<data::variable>& v) const
    {
      std::ostringstream out;
      out << "{";
      for (std::set<data::variable>::const_iterator j = v.begin(); j != v.end(); ++j)
      {
        if (j != v.begin())
        {
          out << ", ";
        }
        out << data::pp(*j);
      }
      out << "}";
      return out.str();
    }

    std::string print_control_flow_marking(const control_flow_vertex& v) const
    {
      std::ostringstream out;
      out << "vertex " << pbes_system::pp(v.X) << " = " << print_variable_set(v.marking);
      return out.str();
    }

    void print_control_flow_marking() const
    {
      std::cout << "--- control flow marking ---" << std::endl;
      for (atermpp::map<propositional_variable_instantiation, control_flow_vertex>::const_iterator i = m_control_vertices.begin(); i != m_control_vertices.end(); ++i)
      {
        const control_flow_vertex& v = i->second;
        std::cout << print_control_flow_marking(v) << std::endl;
      }
    }

    void compute_control_flow_marking()
    {
      // initialization
      for (atermpp::map<propositional_variable_instantiation, control_flow_vertex>::iterator i = m_control_vertices.begin(); i != m_control_vertices.end(); ++i)
      {
        control_flow_vertex& v = i->second;
        std::cout << "<guard>" << pbes_system::pp(v.guard) << std::endl;
        std::set<data::variable> fv = pbes_system::find_free_variables(v.guard);
        std::cout << "<fv>" << print_variable_set(fv) << std::endl;
    	  std::set<data::variable> dx = propvar_parameters(v.X.name());
        std::cout << "<dx>" << print_variable_set(dx) << std::endl;
        std::set<data::variable> cf = control_flow_parameters(v.X.name());
        std::cout << "<cf>" << print_variable_set(cf) << std::endl;
        v.marking = data::detail::set_difference(data::detail::set_intersection(fv, dx), cf);
        mCRL2log(log::debug, "control_flow") << "initial marking " << print_control_flow_marking(v) << "\n";
      }

      // backwards reachability algorithm
      std::set<control_flow_vertex*> todo;
      for (atermpp::map<propositional_variable_instantiation, control_flow_vertex>::iterator i = m_control_vertices.begin(); i != m_control_vertices.end(); ++i)
      {
        control_flow_vertex& v = i->second;
        todo.insert(&v);
      }
      while (!todo.empty())
      {
        std::set<control_flow_vertex*>::iterator i = todo.begin();
        todo.erase(i);
        control_flow_vertex& v = **i;
        mCRL2log(log::debug, "control_flow") << "selected marking todo element " << pbes_system::pp(v.X) << std::endl;

        for (std::set<control_flow_edge>::iterator i = v.incoming_edges.begin(); i != v.incoming_edges.end(); ++i)
        {
          control_flow_vertex& u = *(i->source);
          std::size_t last_size = u.marking.size();
          const propositional_variable_instantiation& Xij = i->label;
          std::set<data::variable> fv = pbes_system::find_free_variables(Xij);
    	    std::set<data::variable> dx = propvar_parameters(Xij.name());
          u.marking = data::detail::set_union(data::detail::set_intersection(fv, dx), u.marking);
          if (u.marking.size() > last_size)
          {
            todo.insert(&u);
            mCRL2log(log::debug, "control_flow") << "updated marking " << print_control_flow_marking(u) << " using edge " << pbes_system::pp(Xij) << "\n";
          }
        }
      }
    }

  public:

    /// \brief Runs the control_flow algorithm
    void run()
    {
      control_flow_influence_graph_algorithm ialgo(m_pbes);
      ialgo.run();

      control_flow_source_dest_algorithm sdalgo(m_pbes);
      sdalgo.compute_source_destination();
      sdalgo.print_source_destination();
      sdalgo.rewrite_propositional_variables();
      // N.B. This modifies m_pbes. It is needed as a precondition for the
      // function compute_control_flow_parameters().

      compute_control_flow_graph();
      print_control_flow_parameters();
      print_control_flow_graph();

      compute_control_flow_marking();
      print_control_flow_marking();
    }
};

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_DETAIL_CONTROL_FLOW_H
