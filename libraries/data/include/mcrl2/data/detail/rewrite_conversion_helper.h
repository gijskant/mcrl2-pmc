// Author(s): Jeroen van der Wulp
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/detail/implement_helper.h
/// \brief The class rewriter.

#ifndef MCRL2_DATA_DETAIL_REWRITE_CONVERSION_HELPER_H
#define MCRL2_DATA_DETAIL_REWRITE_CONVERSION_HELPER_H

#include "boost/assert.hpp"

#include "mcrl2/atermpp/aterm_list.h"
#include "mcrl2/data/sort_expression.h"
#include "mcrl2/data/data_expression.h"
#include "mcrl2/data/abstraction.h"
#include "mcrl2/data/lambda.h"
#include "mcrl2/data/set.h"
#include "mcrl2/data/bag.h"
#include "mcrl2/data/exists.h"
#include "mcrl2/data/forall.h"
#include "mcrl2/data/where_clause.h"
#include "mcrl2/data/standard_utility.h"
#include "mcrl2/data/find.h"
#include "mcrl2/data/utility.h"
#include "mcrl2/data/map_substitution_adapter.h"
#include "mcrl2/data/detail/rewrite.h"
#include "mcrl2/exception.h"

namespace mcrl2 {

  namespace data {

    namespace detail {

      /// \brief Component that provides minimal `implementation' and `reconstruction'
      ///
      /// part of old data implementation that is needed by the rewriter
      /// This code should become obsolete when the rewriter can deal with
      /// abstraction.
      class rewrite_conversion_helper {

        private:

          /// \brief the known sorts (pointer type to allow assignment)
          data_specification const*                                m_data_specification;

          /// \brief associated rewriter object (pointer type to allow assignment)
          mcrl2::data::detail::Rewriter*                           m_rewriter;

          /// \brief before rewriting
          atermpp::map< data_expression, data_expression >         m_implementation_context;

          /// \brief after rewriting
          atermpp::map< data_expression, data_expression >         m_reconstruction_context;

        protected:

          template < typename Sequence >
          void initialise(Sequence const& s)
          {
            // Add rewrite rules (needed only for lambda expressions)
            for (typename Sequence::const_iterator i = s.begin(); i != s.end(); ++i)
            {
              if (!m_rewriter->addRewriteRule(implement(*i)))
              {
                throw mcrl2::runtime_error("Could not add rewrite rule!");
              }
            }
          }

        public:

          // For normalising sort expressions
          sort_expression implement(sort_expression const& expression)
          {
            return m_data_specification->normalise(expression);
          }

          function_symbol implement(function_symbol const& f)
          {
            return function_symbol(f.name(), implement(f.sort()));
          }

          variable implement(variable const& v)
          {
            return variable(v.name(), implement(v.sort()));
          }

          data_equation implement(data_equation const& equation)
          {
            return data_equation(
              implement(equation.variables()),
              implement(equation.condition()),
              implement(equation.lhs()),
              implement(equation.rhs()));
          }

          data_expression implement(lambda const& expression)
          {
            using namespace mcrl2::core::detail;

            // Using local generator because existing generators do not satisfy requirements and performance expectations
            struct local_generator {

              std::string m_current;

              local_generator() : m_current("lambda@")
              {}

              function_symbol operator()(mcrl2::data::sort_expression const& sort)
              {
                bool carry = true;

                for (std::string::iterator i = m_current.begin() + 7; i != m_current.end() && carry; ++i) {
                  if (*i < '9') {
                    ++(*i);

                    carry = false;
                  }
                  else if (*i < 'a') {
                    *i = 'a';

                    carry = false;
                  }
                  else if (*i < 'z') {
                    ++(*i);

                    carry = false;
                  }
                  else {
                    *i = '0';
                  }
                }

                if (carry) {
                  m_current.append("0");
                }

                return mcrl2::data::function_symbol(m_current, sort);
              }
            };

            static local_generator symbol_generator;

            atermpp::map< data_expression, data_expression >::const_iterator i = m_implementation_context.find(expression);

            if (i == m_implementation_context.end())
            { // implementation with previously generated function
              atermpp::term_list< variable > bound_variables = implement(expression.variables());

              if (!bound_variables.empty())
              { // function with non-empty domain
                data_expression body(implement(expression.body()));
                atermpp::term_list< variable > free_variables(implement(
                                                 boost::make_iterator_range(find_free_variables(expression))));

                function_sort   new_function_sort(make_sort_range(bound_variables), sort_expression(body.sort()));

                data_expression new_function(symbol_generator((free_variables.empty()) ? new_function_sort :
                                      function_sort(make_sort_range(free_variables), new_function_sort)));

                // lambda f : type_of(free_variables). lambda b. type_of(bound_variables) = body
                if (free_variables.empty())
                {
                  m_rewriter->addRewriteRule(data_equation(bound_variables, application(new_function, bound_variables), body));
                }
                else
                {
                  new_function = application(new_function, free_variables);

                  m_rewriter->addRewriteRule(data_equation(free_variables + bound_variables, application(new_function, bound_variables), body));
                }

                m_implementation_context[expression]   = new_function;
                m_reconstruction_context[new_function] = expression;

                return new_function;
              }

              return implement(expression.body());
            }

            return i->second;
          }

          data_expression implement(abstraction const& expression)
          {
            using namespace mcrl2::core::detail;
            using namespace mcrl2::data::sort_set;
            using namespace mcrl2::data::sort_bag;

            if (!expression.is_lambda())
            {
              data_expression abstract_body(implement(lambda(expression.variables(), expression.body())));

              if (is_setcomprehension_application(expression))
              {
                return setcomprehension(set_(expression.variables()[0].sort()), abstract_body);
              }
              else if (is_bagcomprehension_application(expression))
              {
                return bagcomprehension(bag(expression.variables()[0].sort()), abstract_body);
              }
              else if (expression.is_exists())
              {
                return application(function_symbol("exists", function_sort(abstract_body.sort(), sort_bool::bool_())), abstract_body);
              }
              else if (expression.is_forall())
              {
                return application(function_symbol("forall", function_sort(abstract_body.sort(), sort_bool::bool_())), abstract_body);
              }
            }
             
            return implement(lambda(expression));
          }

          data_expression implement(where_clause const& w)
          { // return corresponding lambda expression
            return (make_assignment_left_hand_side_range(w.declarations()).empty()) ? implement(w.body()) :
                application(implement(lambda(make_assignment_left_hand_side_range(w.declarations()), w.body())),
                  implement(make_assignment_right_hand_side_range(w.declarations())));
          }

          template < typename ForwardTraversalIterator >
          atermpp::term_list< typename ForwardTraversalIterator::value_type > implement(boost::iterator_range< ForwardTraversalIterator > const& range)
          {
            atermpp::vector< typename ForwardTraversalIterator::value_type > result;

            for (typename boost::iterator_range< ForwardTraversalIterator >::const_iterator
                                                          i(range.begin()); i != range.end(); ++i)
            {
              result.push_back(implement(*i));
            }

            return convert< atermpp::term_list< typename ForwardTraversalIterator::value_type > >(result);
          }

          data_expression implement(application const& expression)
          {
            return application(implement(expression.head()), implement(expression.arguments()));
          }

          data_expression reconstruct(application const& expression)
          {
            if (expression.head().is_function_symbol())
            {
              function_symbol head(expression.head());

              if (head.name() == "exists")
              {
                lambda argument(reconstruct(expression.arguments()[0]));

                return exists(argument.variables(), argument.body());
              }
              else if (head.name() == "forall")
              {
                lambda argument(reconstruct(expression.arguments()[0]));

                return forall(argument.variables(), argument.body());
              }
            }

            atermpp::vector< data_expression > arguments;

            for (application::arguments_const_range r(expression.arguments()); !r.empty(); r.advance_begin(1))
            {
              arguments.push_back(reconstruct(r.front()));
            }

            return application(reconstruct(expression.head()), arguments);
          }

          data_expression reconstruct(data_expression const& expression)
          {
            if (expression.is_function_symbol())
            {
              atermpp::map< data_expression, data_expression >::const_iterator i(m_reconstruction_context.find(expression));

              if (i != m_reconstruction_context.end())
              {
                return i->second;
              }
            }
            else if (expression.is_application())
            {
              return reconstruct(application(expression));
            }

            return expression;
          }

          data_expression lazy_reconstruct(data_expression const& expression)
          {
            if (!m_reconstruction_context.empty())
            {
              return reconstruct(expression);
            }

            return expression;
          }

          data_expression implement(data_expression const& expression)
          {
            if (expression.is_application())
            {
              return implement(application(expression));
            }
            else if (expression.is_variable())
            {
              return implement(variable(expression));
            }
            else if (expression.is_function_symbol())
            {
              return implement(function_symbol(expression));
            }
            else if (expression.is_abstraction())
            {
              return implement(abstraction(expression));
            }
            else if (expression.is_where_clause())
            {
              return implement(where_clause(expression));
            }

            return expression;
          }

          rewrite_conversion_helper(data_specification const& specification,
                                    detail::Rewriter& rewriter) :
                   m_data_specification(&specification),
                   m_rewriter(&rewriter)
          {
            initialise(specification.equations());
          }

          template < typename EquationSelector >
          rewrite_conversion_helper(data_specification const& specification,
                                    detail::Rewriter& rewriter, EquationSelector& selector) :
                   m_data_specification(&specification),
                   m_rewriter(&rewriter)
          {
            initialise(selector(specification.equations()));
          }

          template < typename EquationSelector >
          rewrite_conversion_helper(data_specification const& specification,
                                    detail::Rewriter& rewriter, EquationSelector const& selector) :
                   m_data_specification(&specification),
                   m_rewriter(&rewriter)
          {
            initialise(selector(specification.equations()));
          }
      };
    } // namespace detail
  } // namespace data
} // namespace mcrl2

#endif
