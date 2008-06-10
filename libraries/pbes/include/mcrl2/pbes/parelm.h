// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/parelm.h
/// \brief The parelm algorithm.

#ifndef MCRL2_PBES_PARELM_H
#define MCRL2_PBES_PARELM_H

#include <iterator>
#include <algorithm>
#include <map>
#include <set>
#include <vector>
#include <boost/bind.hpp>
#include <boost/graph/adjacency_list.hpp>
#include "mcrl2/atermpp/algorithm.h"
#include "mcrl2/core/reachable_nodes.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/data/find.h"
#include "mcrl2/data/utility.h"
#include "mcrl2/data/detail/data_assignment_functional.h"
#include "mcrl2/data/detail/sorted_sequence_algorithm.h"
#include "mcrl2/lps/mcrl22lps.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/detail/remove_parameters.h"
#include "mcrl2/pbes/detail/free_variable_visitor.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/remove_parameters.h"
#include "mcrl2/pbes/find.h"

namespace mcrl2 {

namespace pbes_system {

class pbes_parelm_algorithm
{
  protected:
    typedef boost::adjacency_list<boost::setS, boost::vecS, boost::directedS> graph;
    typedef boost::graph_traits<graph>::vertex_descriptor vertex_descriptor;
    typedef boost::graph_traits<graph>::edge_descriptor edge_descriptor;
    
    template <class Iter, class T>
    void iota(Iter first, Iter last, T value) const
    {
      while (first != last)
      {
        *first++ = value++;
      }
    }

    std::set<data::data_variable> unbound_variables(pbes_expression t, data::data_variable_list bound_variables) const
    {
      bool search_propositional_variables = false;
      detail::free_variable_visitor visitor(bound_variables, search_propositional_variables);
      visitor.visit(t);
      return visitor.result;      
    }
    
    int variable_index(data::data_variable_list v, data::data_variable d) const
    {
      int index = 0;
      for (data::data_variable_list::iterator i = v.begin(); i != v.end(); ++i)
      {
        if (*i == d)
        {
          return index;
        }
        index++;
      }
      return -1;
    }

    template <typename Container>
    void print_pp_container(const Container& v, std::string message = "<variables>", bool print_index = false) const
    {
      std::cout << message << std::endl;
      int index = 0;
      for (typename Container::const_iterator i = v.begin(); i != v.end(); ++i)
      {
        if (print_index)
        {
          std::cout << index++ << " " << mcrl2::core::pp(*i) << std::endl;
        }
        else
        {
          std::cout << mcrl2::core::pp(*i) << " ";
        }
      }
      std::cout << std::endl;
    }

    template <typename Container>
    void print_container(const Container& v, std::string message = "<variables>") const
    {
      std::cout << message << std::endl;
      for (typename Container::const_iterator i = v.begin(); i != v.end(); ++i)
      {
        std::cout << *i << " ";
      }
      std::cout << std::endl;
    }

    template <typename Container>
    void print_map(const Container& v, std::string message = "<variables>") const
    {
      std::cout << message << std::endl;
      for (typename Container::const_iterator i = v.begin(); i != v.end(); ++i)
      {
        std::cout << i->first << " -> " << i->second << std::endl;
      }
      std::cout << std::endl;
    }

    template <typename Container>
    core::identifier_string find_predicate_variable(const pbes<Container>& p, int index) const
    {
      int offset = 0;
      for (typename Container::const_iterator i = p.equations().begin(); i != p.equations().end(); ++i)
      {
        int size = i->variable().parameters().size();
        if (offset + size > index)
        {
          return i->variable().name();
        }
        offset += i->variable().parameters().size();
      }
      return core::identifier_string("<not found>");
    }

  public:
    template <typename Container>
    void run(pbes<Container>& p) const
    {
      data::data_variable_list fvars(p.free_variables().begin(), p.free_variables().end());
      std::vector<data::data_variable> predicate_variables;
      
      // compute a mapping from propositional variable names to offsets
      int offset = 0;
      std::map<core::identifier_string, int> propvar_offsets;
      for (typename Container::const_iterator i = p.equations().begin(); i != p.equations().end(); ++i)
      {
        propvar_offsets[i->variable().name()] = offset;
        offset += i->variable().parameters().size();
        predicate_variables.insert(predicate_variables.end(), i->variable().parameters().begin(), i->variable().parameters().end());
      }
      int N = offset; // # variables

      // compute the initial set v of significant variables
      std::set<int> v;
      offset = 0;
      for (typename Container::const_iterator i = p.equations().begin(); i != p.equations().end(); ++i)
      {
        std::set<data::data_variable> uvars = unbound_variables(i->formula(), fvars);
        for (std::set<data::data_variable>::iterator j = uvars.begin(); j != uvars.end(); ++j)
        {
          int k = variable_index(i->variable().parameters(), *j);
          if (k < 0)
          {
            std::cerr << "<variable error>" << mcrl2::core::pp(*j) << std::endl;
            continue;
          }
          v.insert(offset + k);
        }
        offset += i->variable().parameters().size();
      }

      // compute the dependency graph G
      graph G(N);
      for (typename Container::const_iterator i = p.equations().begin(); i != p.equations().end(); ++i)
      {
        // left hand side (X)
        core::identifier_string X = i->variable().name();
        data::data_variable_list Xparams = i->variable().parameters();

        // right hand side (Y)
        pbes_expression phi = i->formula();
        std::set<propositional_variable_instantiation> propvars = find_all_propositional_variable_instantiations(phi);
        for (std::set<propositional_variable_instantiation>::iterator j = propvars.begin(); j != propvars.end(); ++j)
        {
          core::identifier_string Y = j->name();
          data::data_expression_list Yparams = j->parameters();
          int Yindex = 0;
          for (data::data_expression_list::iterator y = Yparams.begin(); y != Yparams.end(); ++y)
          {
            std::set<data::data_variable> vars = data::find_all_data_variables(*y);
            for (std::set<data::data_variable>::iterator k = vars.begin(); k != vars.end(); ++k)
            {
              int Xindex = variable_index(Xparams, *k);
              if (Xindex < 0)
              {
                continue;
              }
              // parameter (Y, Yindex) is influenced by (X, Xindex)
              boost::add_edge(propvar_offsets[Y] + Yindex, propvar_offsets[X] + Xindex, G);
            }
            Yindex++;
          }
        }
      }

      // compute the indices s of the parameters that need to be removed
      std::vector<int> r = core::reachable_nodes(G, v.begin(), v.end());
      std::sort(r.begin(), r.end());
      std::vector<int> q(N);
      iota(q.begin(), q.end(), 0);
      std::vector<int> s;
      std::set_difference(q.begin(), q.end(), r.begin(), r.end(), std::back_inserter(s));

      // create a map that specifies the parameters that need to be removed
      std::map<core::identifier_string, std::vector<int> > removals;
      int index = 0;
      std::vector<int>::iterator sfirst = s.begin();
      for (typename Container::const_iterator i = p.equations().begin(); i != p.equations().end(); ++i)
      {
        int maxindex = index + i->variable().parameters().size();
        std::vector<int>::iterator slast = std::find_if(sfirst, s.end(), boost::bind(std::greater_equal<int>(), _1, maxindex));
        if (slast > sfirst)
        {
          std::vector<int> w(sfirst, slast);
          std::transform(w.begin(), w.end(), w.begin(), boost::bind(std::minus<int>(), _1, index));
          removals[i->variable().name()] = w;
        }
        index = maxindex;
        sfirst = slast;
      }

      // print debug output
      if (mcrl2::core::gsDebug)
      {
        std::cout << "\ninfluential parameters:" << std::endl;
        for(std::set<int>::iterator i = v.begin(); i != v.end(); ++i)
        {
          core::identifier_string X1 = find_predicate_variable(p, *i);
          data::data_variable v1 = predicate_variables[*i];
          std::cout << "(" + mcrl2::core::pp(X1) + ", " + mcrl2::core::pp(v1) + ")\n";
        }
        std::cout << "\ndependencies:" << std::endl;
        typedef typename boost::graph_traits<graph>::edge_iterator edge_iterator;
        std::pair<edge_iterator, edge_iterator> e = edges(G);
        edge_iterator first = e.first;
        edge_iterator last  = e.second;       
        for( ; first != last; ++first)
        {
          edge_descriptor e = *first;
          int i1 = boost::source(e, G);
          core::identifier_string X1 = find_predicate_variable(p, i1);
          data::data_variable v1 = predicate_variables[i1];
          int i2 = boost::target(e, G);
          core::identifier_string X2 = find_predicate_variable(p, i2);
          data::data_variable v2 = predicate_variables[i2];
          std::string left  = "(" + mcrl2::core::pp(X1) + ", " + mcrl2::core::pp(v1) + ")";
          std::string right = "(" + mcrl2::core::pp(X2) + ", " + mcrl2::core::pp(v2) + ")";
          std::cout << left << " -> " << right << std::endl;
        }
      }

      // print verbose output
      if (mcrl2::core::gsVerbose)
      {
        std::cout << "\nremoving the following parameters:" << std::endl;
        for (std::map<core::identifier_string, std::vector<int> >::const_iterator i = removals.begin(); i != removals.end(); ++i)
        {
          core::identifier_string X1 = i->first;

          for (std::vector<int>::const_iterator j = (i->second).begin(); j != (i->second).end(); ++j)
          {
            data::data_variable v1 = predicate_variables[*j];
            std::cout << "(" + mcrl2::core::pp(X1) + ", " + mcrl2::core::pp(v1) + ")\n";
          }
        }
      }

      // remove the parameters
      remove_parameters(p, removals);
    }
};

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_PARELM_H
