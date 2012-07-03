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
#include "mcrl2/data/standard.h"
#include "mcrl2/data/detail/simplify_rewrite_builder.h"
#include "mcrl2/pbes/find.h"
#include "mcrl2/pbes/detail/is_pfnf.h"
#include "mcrl2/pbes/detail/pfnf_pbes.h"
#include "mcrl2/pbes/detail/simplify_quantifier_builder.h"
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
    typedef atermpp::vector<data::data_expression> destination_array; // N.B. data_expression() represents a non-existent value in this vector
    typedef std::map<propositional_variable_instantiation, destination_array> destination_map;

    // vertex of the influence graph
    struct influence_vertex
    {
      core::identifier_string X;
      data::variable v;

      std::string print() const
      {
        std::ostringstream out;
        out << core::pp(X) << ", " << data::pp(v);
        return out.str();
      }

      influence_vertex(const core::identifier_string& X_, const data::variable& v_)
        : X(X_), v(v_)
      {}
    };

    // influence_edge of the influence graph
    struct influence_edge
    {
      std::size_t i;
      std::size_t j;
      const influence_vertex* source;
      const influence_vertex* target;

      std::string print() const
      {
        std::ostringstream out;
        out << core::pp(source->X) << " -- " << i << ", " << j << " --> " << core::pp(target->X);
        return out.str();
      }

      influence_edge(std::size_t i_, std::size_t j_, const influence_vertex* source_, const influence_vertex* target_)
        : i(i_), j(j_), source(source_), target(target_)
      {}
    };

    // vertex of the control flow graph
    struct control_flow_vertex
    {
      propositional_variable_instantiation X;
      std::set<control_flow_vertex*> edges;

      control_flow_vertex(const propositional_variable_instantiation& X_)
        : X(X_)
      {}

      std::string print() const
      {
        std::ostringstream out;
        out << pbes_system::pp(X);
        out << " edges:";
        for (std::set<control_flow_vertex*>::const_iterator i = edges.begin(); i != edges.end(); ++i)
        {
          out << " " << pbes_system::pp((*i)->X);
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

    bool is_constant(const data::data_expression& x) const
    {
      typedef core::term_traits<data::data_expression> tr;
      return tr::is_constant(x);
    }

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

    // rewrite the parameters of the propositional variables, using the substitutions of the source function
    void rewrite_propositional_variables()
    {
      data::rewriter r(m_pbes.data());
      pbes_system::data_rewriter<pbes_expression, data::rewriter> R(r);

      std::vector<pfnf_equation>& equations = m_pbes.equations();
      for (std::size_t k = 0; k < equations.size(); k++)
      {
        const std::vector<data::mutable_map_substitution<> >& src = m_source[k];
        std::vector<pfnf_implication>& implications = equations[k].implications();

        for (std::size_t i = 0; i < implications.size(); i++)
        {
          const data::mutable_map_substitution<>& sigma = src[i];
          std::vector<propositional_variable_instantiation>& v = implications[i].variables();
          for (std::vector<propositional_variable_instantiation>::iterator j = v.begin(); j != v.end(); ++j)
          {
            propositional_variable_instantiation& Xij = *j;
            Xij = pbes_system::rewrite(Xij, R, sigma);
          }
        }
      }
    }

    pbes_control_flow_algorithm(const pbes<>& p)
      : m_pbes(p)
    {
      simplify(m_pbes);
    }

    void print_influence_graph() const
    {
      std::cout << "--- influence graph ---\n";
      for (std::vector<influence_vertex>::const_iterator i = m_influence_vertices.begin(); i != m_influence_vertices.end(); ++i)
      {
        std::cout << i->print() << std::endl;
      }
      for (std::vector<influence_edge>::const_iterator i = m_influence_edges.begin(); i != m_influence_edges.end(); ++i)
      {
        std::cout << i->print() << std::endl;
      }
    }

  protected:
    typedef atermpp::map<propositional_variable_instantiation, control_flow_vertex>::iterator vertex_iterator;

    // vertices of the influence graph
    std::vector<influence_vertex> m_influence_vertices;

    // edges of the influence graph
    std::vector<influence_edge> m_influence_edges;

    // vertices of the control flow graph
    atermpp::map<propositional_variable_instantiation, control_flow_vertex> m_control_vertices;

    // maps X[i] to the corresponding source
    std::vector<std::vector<data::mutable_map_substitution<> > > m_source;

    // maps X[i] to the corresponding destination
    std::vector<std::vector<destination_map> > m_destination;

    // the pbes that is considered
    pfnf_pbes m_pbes;

    // the control flow parameters
    std::map<core::identifier_string, std::vector<bool> > m_is_control_flow;

    // very inefficient
    std::vector<influence_vertex>::const_iterator find_vertex(const core::identifier_string& X, const data::variable& v) const
    {
      for (std::vector<influence_vertex>::const_iterator i = m_influence_vertices.begin(); i != m_influence_vertices.end(); ++i)
      {
        if (i->X == X && i->v == v)
        {
          return i;
        }
      }
      std::cout << "<error>" << core::pp(X) << " " << data::pp(v) << std::endl;
      return m_influence_vertices.end();
    }

    std::vector<pfnf_equation>::const_iterator find_equation(const pfnf_pbes& p, const core::identifier_string& X) const
    {
      const std::vector<pfnf_equation>& equations = p.equations();
      for (std::vector<pfnf_equation>::const_iterator i = equations.begin(); i != equations.end(); ++i)
      {
        if (i->variable().name() == X)
        {
          return i;
        }
      }
      return equations.end();
    }

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

    // extract the propositional variable instantiations from an expression of the form g => \/_j in J . X_j(e_j)
    std::vector<propositional_variable_instantiation> find_propositional_variables(const pbes_expression& x) const
    {
      std::vector<pbes_expression> v;
      pbes_expression y = x;
      if (is_imp(y))
      {
        y = imp(y).right();
      }
      split_or(y, v);

      std::vector<propositional_variable_instantiation> result;
      for (std::vector<pbes_expression>::iterator i = v.begin(); i != v.end(); ++i)
      {
        if (is_propositional_variable_instantiation(*i))
        {
          result.push_back(*i);
        }
      }
      return result;
    }

    void split_and(const pbes_expression& expr, std::vector<pbes_expression>& result) const
    {
      namespace a = combined_access;
      utilities::detail::split(expr, std::back_inserter(result), a::is_and, a::left, a::right);
    }

    // Simplifies the pbes expression x, and extracts all conjuncts d[i] == e from it, for some i in 0 ... d.size(), and with e a constant.
    // The conjuncts are added to the substitution sigma.
    void find_equality_conjuncts(const pbes_expression& x, const std::vector<data::variable>& d, data::mutable_map_substitution<>& sigma) const
    {
      std::vector<data::data_expression> result;

      std::vector<pbes_expression> v;
      split_and(x, v);
      for (std::vector<pbes_expression>::iterator i = v.begin(); i != v.end(); ++i)
      {
        if (data::is_data_expression(*i))
        {
          data::data_expression v_i = *i;
          if (data::is_equal_to_application(v_i))
          {
            data::data_expression left = data::application(v_i).left();
            data::data_expression right = data::application(v_i).right();
            if (data::is_variable(left) && std::find(d.begin(), d.end(), data::variable(left)) != d.end() && is_constant(right))
            {
              sigma[left] = right;
            }
            else if (data::is_variable(right) && std::find(d.begin(), d.end(), data::variable(right)) != d.end() && is_constant(left))
            {
              sigma[right] = left;
            }
          }
          // TODO: handle conjuncts b and !b, with b a variable with sort Bool
          //else if (data::is_variable(v_i) && sort_bool::is_bool(v_i.sort()) && std::find(d.begin(), d.end(), data::variable(v_i)) != d.end())
          //{
          //  sigma[data::variable(v_i)] = sort_bool::true_();
          //}
        }
      }
    }

    void compute_influence_graph()
    {
      // compute the vertices
      const std::vector<pfnf_equation>& equations = m_pbes.equations();
      for (std::vector<pfnf_equation>::const_iterator i = equations.begin(); i != equations.end(); ++i)
      {
        core::identifier_string X = i->variable().name();
        const std::vector<data::variable>& Xparams = i->parameters();
        for (std::vector<data::variable>::const_iterator j = Xparams.begin(); j != Xparams.end(); ++j)
        {
          m_influence_vertices.push_back(influence_vertex(X, *j));
        }
      }

      // compute the edges
      for (std::vector<pfnf_equation>::const_iterator k = equations.begin(); k != equations.end(); ++k)
      {
        const std::vector<data::variable>& d_X = k->parameters();
        const std::vector<pfnf_implication>& implications = k->implications();
        const core::identifier_string Xname = k->variable().name();

        for (std::size_t i = 0; i < implications.size(); i++)
        {
          const std::vector<propositional_variable_instantiation>& propvars = implications[i].variables();
          for (std::size_t j = 0; j < propvars.size(); j++)
          {
            const propositional_variable_instantiation& Y = propvars[j];
            std::vector<data::data_expression> Yparameters(Y.parameters().begin(), Y.parameters().end());
            const pfnf_equation& eqn = *find_equation(m_pbes, Y.name());
            const std::vector<data::variable> d_Y = eqn.parameters();
            for (std::size_t p = 0; p < Yparameters.size(); p++)
            {
              std::set<data::variable> freevars = pbes_system::find_free_variables(Yparameters[p]);
              for (std::size_t m = 0; m < d_X.size(); m++)
              {
                if (std::find(freevars.begin(), freevars.end(), d_X[m]) != freevars.end())
                {
                  std::vector<influence_vertex>::const_iterator source = find_vertex(Xname, d_X[m]);
                  std::vector<influence_vertex>::const_iterator target = find_vertex(Y.name(), d_Y[p]);
                  influence_edge e(i, j, &(*source), &(*target));
                  m_influence_edges.push_back(e);
                }
              }
            }
          }
        }
      }
    }

    // computes the source and the destination function for a pbes equation
    // source[i] contains the source parameters of g_i, represented in the form of a substitution
    // dest[i] maps X_ij(f_ij) to a vector of data expressions
    template <typename DataRewriter>
    void compute_source_destination(const pfnf_equation& eqn,
                                    std::vector<data::mutable_map_substitution<> >& source,
                                    std::vector<destination_map>& dest, DataRewriter rewr
                                   ) const
    {
      typedef core::term_traits<pbes_expression> tr;
      typedef core::term_traits<data::data_expression> tt;

      const std::vector<data::variable>& d_X = eqn.parameters();
      const pbes_expression& h = eqn.h();
      const std::vector<pfnf_implication>& g = eqn.implications();

      data::mutable_map_substitution<> sigma_h;
      find_equality_conjuncts(h, d_X, sigma_h);

      for (std::vector<pfnf_implication>::const_iterator i = g.begin(); i != g.end(); ++i)
      {
        // compute source
        data::mutable_map_substitution<> sigma = sigma_h;
        find_equality_conjuncts(i->g(), d_X, sigma);
        source.push_back(sigma);

        // compute destination
        destination_map dmap;
        const std::vector<propositional_variable_instantiation> propvars = i->variables();
        for (std::vector<propositional_variable_instantiation>::const_iterator j = propvars.begin(); j != propvars.end(); ++j)
        {
          const propositional_variable_instantiation& Xij = *j;
          destination_array dij;
          data::data_expression_list e = Xij.parameters();
          for (data::data_expression_list::iterator k = e.begin(); k != e.end(); ++k)
          {
            data::data_expression e_k = rewr(*k, sigma);
            if (is_constant(e_k))
            {
              dij.push_back(e_k);
            }
            else
            {
              dij.push_back(data::data_expression());
            }
          }
          dmap[Xij] = dij;
        }
        dest.push_back(dmap);
      }
    }

    void compute_source_destination()
    {
      data::rewriter rewr(m_pbes.data());
      const std::vector<pfnf_equation>& equations = m_pbes.equations();

      for (std::vector<pfnf_equation>::const_iterator i = equations.begin(); i != equations.end(); ++i)
      {
        std::vector<data::mutable_map_substitution<> > source;
        std::vector<destination_map> dest;
        compute_source_destination(*i, source, dest, rewr);
        m_source.push_back(source);
        m_destination.push_back(dest);
      }
    }

    void print_destination_array(const propositional_variable_instantiation& X, const destination_array& a) const
    {
      std::cout << "        dest(" << pbes_system::pp(X) << ") = [";
      for (destination_array::const_iterator i = a.begin(); i != a.end(); ++i)
      {
        if (i != a.begin())
        {
          std::cout << ", ";
        }
        if (*i == data::data_expression())
        {
          std::cout << "-";
        }
        else
        {
          std::cout << data::pp(*i);
        }
      }
      std::cout << "]" << std::endl;
    }

    void print_source_destination(const pfnf_equation& eqn, const std::vector<data::mutable_map_substitution<> >& src, const std::vector<destination_map>& dest) const
    {
      propositional_variable X(eqn.variable().name(), data::variable_list(eqn.parameters().begin(), eqn.parameters().end()));
      std::cout << "- predicate variable " << pbes_system::pp(X) << std::endl;

      std::cout << "h     = " << pbes_system::pp(eqn.h()) << std::endl;

      const std::vector<pfnf_implication>& g = eqn.implications();
      for (std::size_t i = 0; i < src.size(); i++)
      {
        const data::mutable_map_substitution<>& sigma = src[i];
        const destination_map& dmap = dest[i];
        std::cout << "g[" << std::setw(2) << (i + 1) << "] = " << g[i] << std::endl;
        std::cout << "        source = " << data::print_substitution(sigma) << std::endl;
        for (destination_map::const_iterator j = dmap.begin(); j != dmap.end(); ++j)
        {
          print_destination_array(j->first, j->second);
        }
        std::cout << std::endl;
      }
    }

    void print_source_destination() const
    {
      std::size_t N = m_pbes.equations().size();
      for (std::size_t i = 0; i < N; i++)
      {
        print_source_destination(m_pbes.equations()[i], m_source[i], m_destination[i]);
      }
    }

    data::data_expression source(std::size_t k, std::size_t i, std::size_t n) const
    {
      const data::mutable_map_substitution<>& sigma = m_source[k][i];
      data::variable d_n = m_pbes.equations()[k].parameters()[n];
      data::data_expression x = sigma(d_n);
      if (x == d_n)
      {
        return data::data_expression();
      }
      else
      {
        return x;
      }
    }

    // returns the constant value fij[n] of equation k, or data::data_expression() if it does not exist,
    // where fij is the j-th propositional variable of the i-th conjunct of the k-th equation
    data::data_expression destination(std::size_t k, std::size_t i, std::size_t j, std::size_t n) const
    {
      const propositional_variable_instantiation& Xij = m_pbes.equations()[k].implications()[i].variables()[j];
      const destination_map& dmap = m_destination[k][i];
      destination_map::const_iterator q = dmap.find(Xij);
      assert (q != dmap.end());
      return q->second[n];
    }

    data::data_expression copy(std::size_t k, std::size_t i, std::size_t j, std::size_t n) const
    {
      data::variable d_n = m_pbes.equations()[k].parameters()[n];
      std::size_t M = m_pbes.equations()[k].implications()[i].variables().size();
      for (std::size_t m = 0; m < M; m++)
      {
        if (destination(k, i, j, m) == d_n)
        {
          return d_n;
        }
      }
      return data::data_expression();
    }

    // returns true if parameter n of X[k] rules conjunct i
    bool is_rule_parameter(std::size_t k, std::size_t n, std::size_t i) const
    {
      const pfnf_equation& eqn = m_pbes.equations()[k];
      const pfnf_implication& g = eqn.implications()[i];
      const data::variable d_n = eqn.parameters()[n];
      if (source(k, i, n) == data::data_expression())
      {
        return false;
      }
      for (std::size_t j = 0; j < g.variables().size(); ++j)
      {
        if (destination(k, i, j, n) == data::data_expression())
        {
          return false;
        }
      }
      std::cout << "parameter " << data::pp(d_n) << " rules conjunct " << i << std::endl;
      return true;
    }

    // return true if parameter n of X[k] is a control flow parameter
    bool is_control_flow_parameter(std::size_t k, std::size_t n)
    {
      const pfnf_equation& eqn = m_pbes.equations()[k];
      for (std::size_t i = 0; i < eqn.implications().size(); i++)
      {
        const pfnf_implication& g = eqn.implications()[i];
        if (is_rule_parameter(k, n, i))
        {
          continue;
        }
        const std::vector<propositional_variable_instantiation>& propvars = g.variables();
        for (std::size_t j = 0; j < propvars.size(); j++)
        {
          const propositional_variable_instantiation& Xij = propvars[j];
          std::size_t M = Xij.parameters().size();
          std::set<data::data_expression> c;
          for (std::size_t m = 0; m < M; m++)
          {
            c.insert(copy(k, i, j, m));
          }
          if (c.size() > 1 || *c.begin() == data::data_expression())
          {
            return false;
          }
        }
      }
      return true;
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

    void compute_control_flow_parameters()
    {
      rewrite_propositional_variables();
      const std::vector<pfnf_equation>& equations = m_pbes.equations();

      // initialize all control flow parameters to true
      for (std::vector<pfnf_equation>::const_iterator k = equations.begin(); k != equations.end(); ++k)
      {
        propositional_variable X = k->variable();
        const std::vector<data::variable>& d_X = k->parameters();
        m_is_control_flow[X.name()] = std::vector<bool>(d_X.size(), true);
      }

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
                  std::cout << "equation " << pbes_system::pp(X) << " variable " << pbes_system::pp(Xij) << " " << index << std::endl;
                }
                else
                {
                  if (X.name() == Xij.name() && (found != d_X.begin() + index))
                  {
                    m_is_control_flow[Xij.name()][index] = false;
                    std::cout << "equation " << pbes_system::pp(X) << " variable " << pbes_system::pp(Xij) << " " << index << std::endl;
                  }
                }
              }
              else
              {
                m_is_control_flow[Xij.name()][index] = false;
                std::cout << "equation " << pbes_system::pp(X) << " variable " << pbes_system::pp(Xij) << " " << index << std::endl;
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
    vertex_iterator insert_control_flow_vertex(const propositional_variable_instantiation& X)
    {
      std::pair<vertex_iterator, bool> p = m_control_vertices.insert(std::make_pair(X, control_flow_vertex(X)));
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
        std::cout << "selected todo element " << pbes_system::pp(v.X) << std::endl;

        const pfnf_equation& eqn = *find_equation(m_pbes, v.X.name());
        propositional_variable X = project_variable(eqn.variable());
        data::variable_list d = X.parameters();
        data::data_expression_list e = v.X.parameters();
        data::sequence_sequence_substitution<data::variable_list, data::data_expression_list> sigma(d, e);

        const std::vector<pfnf_implication>& implications = eqn.implications();
        for (std::vector<pfnf_implication>::const_iterator i = implications.begin(); i != implications.end(); ++i)
        {
          const std::vector<propositional_variable_instantiation>& propvars = i->variables();
          for (std::vector<propositional_variable_instantiation>::const_iterator j = propvars.begin(); j != propvars.end(); ++j)
          {
            propositional_variable_instantiation Xij = project(*j);
            propositional_variable_instantiation Y = apply_substitution(Xij, sigma);
            vertex_iterator j = m_control_vertices.find(Y);
            if (j == m_control_vertices.end())
            {
              // vertex Y does not yet exist
              std::cout << "discovered " << pbes_system::pp(Y) << std::endl;
              vertex_iterator k = insert_control_flow_vertex(Y);
              todo.insert(&(k->second));
              std::cout << "added todo element " << pbes_system::pp(k->first) << std::endl;
              v.edges.insert(&(k->second));
            }
            else
            {
              v.edges.insert(&(j->second));
            }
          }
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

  public:

    /// \brief Runs the control_flow algorithm
    void run()
    {
      compute_influence_graph();
      print_influence_graph();
      compute_source_destination();
      print_source_destination();
      compute_control_flow_graph();
      print_control_flow_parameters();
      print_control_flow_graph();
    }
};

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_DETAIL_CONTROL_FLOW_H
