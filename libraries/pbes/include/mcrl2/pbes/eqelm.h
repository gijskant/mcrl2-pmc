// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/eqelm.h
/// \brief The eqelm algorithm.

#ifndef MCRL2_PBES_EQELM_H
#define MCRL2_PBES_EQELM_H

// #define MCRL2_PBES_EQELM_DEBUG

#include <sstream>
#include <iostream>
#include <utility>
#include <deque>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include "mcrl2/core/messaging.h"
#include "mcrl2/data/sort_expression.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/find.h"
#include "mcrl2/pbes/rewriter.h"
#include "mcrl2/pbes/substitute.h"
#include "mcrl2/pbes/remove_parameters.h"
#include "mcrl2/core/detail/print_utility.h"
#include "mcrl2/core/algorithm.h"

namespace mcrl2 {

namespace pbes_system {

  /// \brief Algorithm class for the eqelm algorithm
  template <typename Term, typename DataRewriter, typename PbesRewriter>
  class pbes_eqelm_algorithm: public core::algorithm
  {
    public:
      /// \brief The term type
      typedef typename core::term_traits<Term>::term_type term_type;

      /// \brief The variable type
      typedef typename core::term_traits<Term>::variable_type variable_type;

      /// \brief The variable sequence type
      typedef typename core::term_traits<Term>::variable_sequence_type variable_sequence_type;

      /// \brief The data term type
      typedef typename core::term_traits<Term>::data_term_type data_term_type;

      /// \brief The data term sequence type
      typedef typename core::term_traits<Term>::data_term_sequence_type data_term_sequence_type;

      /// \brief The string type
      typedef typename core::term_traits<Term>::string_type string_type;

      /// \brief The propositional variable declaration type
      typedef typename core::term_traits<Term>::propositional_variable_decl_type propositional_variable_decl_type;

      /// \brief The propositional variable instantiation type
      typedef typename core::term_traits<Term>::propositional_variable_type propositional_variable_type;

      /// \brief The term traits
      typedef typename core::term_traits<Term> tr;

    protected:
      typedef std::set<variable_type> equivalence_class;
      
      /// \brief Compares data expressions for equality.
      DataRewriter m_data_rewriter;

      /// \brief Compares data expressions for equality.
      PbesRewriter m_pbes_rewriter;

      /// \brief The vertices of the grapth, i.e. the equivalence relations.
      /// It stores the equivalence sets for each propositional variable, for example
      /// X -> [ {x1, x3}, {x2, x4} ]. Equivalence sets of size 1 are not stored.
      std::map<string_type, std::vector<equivalence_class> > m_vertices;

      /// \brief The edges of the graph.
      /// It is a mapping from X to iocc(X).
      std::map<string_type, atermpp::set<propositional_variable_type> > m_edges;

      /// \brief The parameters of the propositional variable declarations.
      std::map<string_type, std::vector<variable_type> > m_parameters;

      /// \brief Puts all parameters of the same sort in the same equivalence set.
      std::vector<equivalence_class> compute_equivalence_sets(const propositional_variable_decl_type& X) const
      {
        std::map< data::sort_expression, equivalence_class> m;
        for (typename variable_sequence_type::const_iterator i = X.parameters().begin(); i != X.parameters().end(); ++i)
        {
          m[i->sort()].insert(*i);
        }
        std::vector<equivalence_class> result;
        for (typename std::map<data::sort_expression, equivalence_class>::iterator i = m.begin(); i != m.end(); ++i)
        {
          if (i->second.size() > 1)
          {
            result.push_back(i->second);
          }
        }
        return result;
      }

      /// \brief Prints the vertices of the dependency graph.
      void LOG_VERTICES(size_t level, const std::string& msg = "") const
      {
        if (check_log_level(level))
        {
          std::clog << msg;
          for (typename std::map<string_type, std::vector<equivalence_class> >::const_iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
          {
            std::clog << core::pp(i->first) << " -> [ ";
            const std::vector<equivalence_class>& v = i->second;
            for (typename std::vector<equivalence_class>::const_iterator j = v.begin(); j != v.end(); ++j)
            {
              if (j != v.begin())
              {
                std::clog << ", ";
              }
              std::clog << core::detail::print_pp_set(*j);
            }
            std::clog << " ]" << std::endl;
          }
        }
      }

      /// \brief Prints the edges of the dependency graph.
      void LOG_EDGES(size_t level, const std::string& msg = "") const
      {
        if (check_log_level(level))
        {
          std::clog << msg;
          for (typename std::map<string_type, atermpp::set<propositional_variable_type> >::const_iterator i = m_edges.begin(); i != m_edges.end(); ++i)
          {
            std::clog << core::pp(i->first) << " -> " << core::detail::print_pp_set(i->second) << std::endl;
          }
        }
      }

      /// \brief Prints the equivalence classes
      void LOG_EQUIVALENCE_CLASSES(size_t level, const std::string& msg = "") const
      {       
        if (check_log_level(level))
        {
          std::clog << msg;
          for (typename std::map<string_type, std::vector<equivalence_class> >::const_iterator i = m_vertices.begin(); i != m_vertices.end(); ++i)
          {
            std::clog << "  vertex " << core::pp(i->first) << ": ";
            for (typename std::vector<equivalence_class>::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
            {
              std::clog << core::detail::print_pp_set(*j) << " ";
            }
            std::clog << std::endl;
          }
        }
      }

      /// \brief Prints the todo list
      void LOG_TODO_LIST(size_t level, const std::set<string_type>& todo, const std::string& msg = "") const
      {       
        if (check_log_level(level))
        {
          std::clog << msg;
          std::clog << core::detail::print_pp_set(todo) << "\n";
        }
      }

      /// \brief Returns true if the vertex X should propagate its values to Y
      bool evaluate_guard(const string_type& /* X */, const propositional_variable_type& /* Y */)
      {
        return true;
      }

      /// \brief Returns the index of the element x in the sequence v
      template <typename VariableContainer>
      size_t index_of(const variable_type& x, const VariableContainer& v)
      {
        return static_cast<size_t>(std::find(v.begin(), v.end(), x) - v.begin());
      }

      /// \brief Propagate the equivalence relations in vertex X over the edge Ye.
      template <typename Substitution>
      void update_equivalence_classes(const string_type& /* X */,
                                      const propositional_variable_type& Ye,
                                      const Substitution& vX,
                                      std::set<string_type>& todo
                                     )
      {
        const string_type& Y = Ye.name();
        std::vector<data_term_type> e(Ye.parameters().begin(), Ye.parameters().end());
          
        std::vector<equivalence_class>& cY = m_vertices[Y];
        std::vector<equivalence_class> cY1;
        for (typename std::vector<equivalence_class>::iterator j = cY.begin(); j != cY.end(); ++j)
        {
          equivalence_class& equiv = *j;
          atermpp::map<data_term_type, equivalence_class> w;
          for (typename equivalence_class::iterator k = equiv.begin(); k != equiv.end(); ++k)
          {
            size_t p = index_of(*k, m_parameters[Y]);
            pbes_system::data_rewriter<Term, DataRewriter> rewr(m_data_rewriter);
            w[rewr(e[p], vX)].insert(*k);
          }
          for (typename std::map<data_term_type, equivalence_class>::iterator i = w.begin(); i != w.end(); ++i)
          {
            if (i->second.size() > 1)
            {
              cY1.push_back(i->second);
            }
          }
        }
        if (cY != cY1)
        {
          todo.insert(Y);
          cY = cY1;
        }
      }

      /// \brief Computes a substitution that corresponds to the equivalence relations in X
      data::mutable_associative_container_substitution<> compute_substitution(const string_type& X)
      {
        data::mutable_associative_container_substitution<> result;
        const std::vector<equivalence_class>& cX = m_vertices[X];
        for (typename std::vector<equivalence_class>::const_iterator i = cX.begin(); i != cX.end(); ++i)
        {
          const equivalence_class& s = *i;
          for (typename equivalence_class::const_iterator j = ++s.begin(); j != s.end(); ++j)
          {
            result[*j] = *s.begin();
          }
        }
        return result;
      }

      /// \brief Chooses one parameter for every equivalence class, and
      /// removes the others. All occurrences of the removed parameters
      /// are replaced by the chosen parameter.
      template <typename Container>
      void apply_equivalence_relations(pbes<Container>& p)
      {
        // first apply the substitutions to the equations
        for (typename Container::iterator i = p.equations().begin(); i != p.equations().end(); ++i)
        {
          string_type X = i->variable().name();
          data::mutable_associative_container_substitution<> sigma = compute_substitution(X);
          if (!sigma.empty())
          {
            i->formula() = pbes_system::substitute_free_variables(i->formula(), sigma);
          }
        }

        // then remove parameters
        std::map<string_type, std::vector<size_t> > to_be_removed;
        for (typename Container::const_iterator i = p.equations().begin(); i != p.equations().end(); ++i)
        {
          string_type X = i->variable().name();
          const std::vector<equivalence_class>& eq = m_vertices[X];
          for (typename std::vector<equivalence_class>::const_iterator j = eq.begin(); j != eq.end(); ++j)
          {
            for (typename equivalence_class::const_iterator k = ++j->begin(); k != j->end(); ++k)
            {  
              to_be_removed[X].push_back(index_of(*k, m_parameters[X]));
            }
          }
        }
        remove_parameters(p, to_be_removed);
      }

    public:
      /// \brief Constructor.
      /// \param datar A data rewriter
      /// \param pbesr A PBES rewriter
      pbes_eqelm_algorithm(DataRewriter datar, PbesRewriter pbesr, size_t log_level = 0)
        : core::algorithm(log_level),
          m_data_rewriter(datar),
          m_pbes_rewriter(pbesr)
      {}

      /// \brief Runs the eqelm algorithm
      /// \param p A pbes
      /// \param ignore_initial_state If true, the initial state is ignored.
      template <typename Container>
      void run(pbes<Container>& p, bool ignore_initial_state = false)
      {
        m_vertices.clear();
        m_edges.clear();
        std::set<string_type> todo;

        // compute the edges and vertices of the graph, and initialize the todo list
        for (typename Container::const_iterator i = p.equations().begin(); i != p.equations().end(); ++i)
        {
          string_type name = i->variable().name();
          m_edges[name] = find_propositional_variable_instantiations(i->formula());
          m_vertices[name] = compute_equivalence_sets(i->variable());
          const variable_sequence_type& param = i->variable().parameters();
          m_parameters[name] = std::vector<variable_type>(param.begin(), param.end());
          todo.insert(name);
        }

        if (!ignore_initial_state)
        {
          todo.clear();
          propositional_variable_type kappa = p.initial_state();
          string_type X = kappa.name();
          data::mutable_associative_container_substitution<> vX = compute_substitution(X);
          std::set<propositional_variable_type> edges = find_propositional_variable_instantiations(kappa);
          for (typename atermpp::set<propositional_variable_type>::const_iterator i = edges.begin(); i != edges.end(); ++i)
          {
            // propagate the equivalence relations in X over the edge Ye
            const propositional_variable_type& Ye = *i;
            if (evaluate_guard(X, Ye))
            {
              todo.insert(Ye.name());
              update_equivalence_classes(X, Ye, vX, todo);
              LOG_EQUIVALENCE_CLASSES(2, "update equivalence classes\n");
            }
          }
        }

        LOG_VERTICES(1, "--- vertices ---\n");
        LOG_EDGES(1, "\n--- edges ---\n");
        LOG_EQUIVALENCE_CLASSES(2, "initial equivalence classes\n");
        LOG_TODO_LIST(2, todo, "initial todo list: ");

        // propagate constraints over the edges until the todo list is empty
        while (!todo.empty())
        {
          string_type X = *todo.begin();
          todo.erase(X);

          LOG(2, "todo element X = " + core::pp(X) + "\n");
          LOG(2, "todo list = " + core::detail::print_pp_set(todo) + "\n");
          LOG_VERTICES(1, "--- vertices ---\n");
          
          // create a substitution function that corresponds to cX
          data::mutable_associative_container_substitution<> vX = compute_substitution(X);
          const atermpp::set<propositional_variable_type>& edges = m_edges[X];
          for (typename atermpp::set<propositional_variable_type>::const_iterator i = edges.begin(); i != edges.end(); ++i)
          {
            // propagate the equivalence relations in X over the edge Ye
            const propositional_variable_type& Ye = *i;
            if (evaluate_guard(X, Ye))
            {
              update_equivalence_classes(X, Ye, vX, todo);
              LOG_EQUIVALENCE_CLASSES(2, "updated equivalence classes\n");
            }
          }
        }
        apply_equivalence_relations(p);
        LOG_VERTICES(1, "\n--- result ---\n");
      }
  };

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_EQELM_H
