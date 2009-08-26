// Author(s): Jeroen van der Wulp
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/classic_enumerator.h
/// \brief Template class for createing enumerator components

#ifndef _MCRL2_DATA_CLASSIC_ENUMERATOR__HPP_
#define _MCRL2_DATA_CLASSIC_ENUMERATOR__HPP_

#include <set>

#include "boost/assert.hpp"
#include "boost/scoped_ptr.hpp"
#include "boost/iterator/iterator_facade.hpp"

#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/expression_traits.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/data/substitution.h"
#include "mcrl2/data/detail/classic_enumerator_impl.h"
#include "mcrl2/data/detail/enum/standard.h"

namespace mcrl2 {
  namespace data {

    /** \brief Selection predicates for data expressions for use with enumerator components
     **/
    namespace selectors {
      /** \brief Configuration component for expression selection
       *
       * Evaluatates and selects substitutions that make the condition evaluate to Value.
       **/
      template < bool Value >
      struct select {
        /// \brief returns true if and only if the argument is equal to true
        template < typename ExpressionType >
        static bool test(ExpressionType const& e) {
          return core::term_traits< ExpressionType >::is_true(e);
        }
      };

      /** \brief Configuration component for expression selection
       *
       * Evaluatates and selects substitutions that make the condition evaluate to Value.
       **/
      template < >
      struct select< false > {
        /// \brief returns true if and only if the argument is equal to false
        template < typename ExpressionType >
        static bool test(ExpressionType const& e) {
          return core::term_traits< ExpressionType >::is_false(e);
        }
      };

      /** \brief Configuration component for expression selection
       *
       * Evaluatates and selects substitutions that do not make the condition evaluate to Value.
       **/
      template < bool Value >
      struct select_not {
        /// \brief returns true if and only if not select< !Value >::test(e)
        template < typename ExpressionType >
        static bool test(ExpressionType const& e) {
          return !selectors::select< Value >::template test< ExpressionType >(e);
        }
      };

      /** \brief Configuration component for expression selection
       *
       * No selection criterion, select everything.
       **/
      struct select_all {
        /// \brief Always returns true
        template < typename ExpressionType >
        static bool test(ExpressionType const&) {
          return true;
        }
      };
    }

    template < typename MutableSubstitution = mutable_map_substitution< >,
               typename Evaluator = rewriter,
               typename Selector = selectors::select< true > >
    class classic_enumerator;

    /** \brief Specialised template class for generating data enumerator components
     *
     * A data enumerator represents a sequence of valuations (specified as a
     * substitution) for a given set of variables that satisfy a given
     * condition. The classic enumerator when constructed represents a
     * sequence of <em>all</em> possible evaluations that satisfy a condition.
     *
     * An classic object is an iterator over a possibly unbounded
     * sequence of valuations. Think of iteration as picking elements from a
     * stream. The iterator itself points to an element in the sequence and
     * represents the computation of the remainder of that sequence.
     *
     * As an example of what problems an enumerator can solve consider finding
     * all lists of even length smaller than 100, containing Natural numbers
     * smaller than 100 that are prime. Provided that this criterion can be
     * captured as an open boolean expression, and that the condition
     * evaluation process is complete (enough) and terminates.
     *
     * This type models concept Data Enumerator when the template arguments satisfy:
     *  \arg MutableSubstitution a model of the MutableSubstitution concept
     *  \arg Evaluator a model of Evaluator concept
     *
     * The following example shows a function that takes a data specification,
     * a data expression that represents a condition and a set of variables
     * that occur free in the condition.
     *
     * \code
     * void enumerate(data_specification const& d,
     *          std::set< variable > variables const& v,
     *                  data_expression const& c) {
     *
     *   using namespace mcrl2::data;
     *
     *   for (classic_enumerator< > i(d, variables, c); i != classic_enumerator< >(); ++i) {
     *     std::cerr << mcrl2::core::pp((*i)(c)) << std::endl;
     *   }
     * }
     * \endcode
     *
     * Besides the set of variables passed as argument the condition is allowed
     * to contain other free variables. The condition evaluation component
     * selects all expressions that cannot be reduced to false. This is
     * especially useful when an enumerator must be utilised from a context
     * where bound variables occur that cannot be eliminated from the
     * condition.
     *
     * In addition to <a href="http://tinyurl.com/9xtcmg">Single Pass Iterator</a> which is
     * required by the Enumerator concept, this type also models the
     * <a href="http://tinyurl.com/ayp9xb">Forward Traversal Iterator</a>
     * concept.  As a result the computation can be branched at any point as
     * illustrated by the following example (using the types from the previous
     * example).
     *
     * \code
     * void enumerate(data_specification const& d,
     *          std::set< variable > variables const& v,
     *                  data_expression const& c) {
     *
     *   enumerator_type i(d, variables, c);
     *
     *   for (enumerator_type j = i; i != enumerator_type(); ++i, ++j) {
     *     assert(*i == *j);
     *   }
     * }
     * \endcode
     *
     * A shared context is kept between iterator objects to keep the cost of
     * copying low. As a consequence the iterator can be used in combination
     * with the Boost.Graph library.
     **/
    template < typename MutableSubstitution, typename Evaluator, typename Selector >
    class classic_enumerator :
           public boost::iterator_facade< classic_enumerator< MutableSubstitution, Evaluator, Selector >,
             const MutableSubstitution, boost::forward_traversal_tag > {

      public:

        /// \brief The type of objects that represent substitutions
        typedef MutableSubstitution                                            substitution_type;
        /// \brief The type of objects that represent variables
        typedef typename MutableSubstitution::variable_type                    variable_type;
        /// \brief The type of objects that represent expressions
        typedef typename MutableSubstitution::expression_type                  expression_type;
        /// \brief The type of objects that represent evaluator components
        typedef Evaluator                                                      evaluator_type;
        /// \brief The type that represent selector components
        typedef Selector                                                       selector_type;

      private:

        typedef detail::classic_enumerator_impl< MutableSubstitution, Evaluator, Selector > implementation_type;

        friend class boost::iterator_core_access;

        template < typename T >
        friend class enumerator_factory;

        template < typename M, typename E, typename S >
        friend class classic_enumerator;

      private:

        // For past-end iterator: m_impl.get() == 0, for cheap iterator construction and comparison
        boost::scoped_ptr< implementation_type >  m_impl;

      private:

        void increment() {
          if (!m_impl->increment()) {
            m_impl.reset();
          }
        }

        bool equal(classic_enumerator const& other) const {
          implementation_type* left  = m_impl.get();
          implementation_type* right = other.m_impl.get();

          return (left == right) || (left != 0 && right != 0 && (left->dereference() == right->dereference()));
        }

        substitution_type const& dereference() const {
          BOOST_ASSERT(m_impl.get() != 0);

          return m_impl->dereference();
        }

        typename std::set< variable_type > make_set(variable const& variable) {
          typename std::set< variable_type > variables;

          variables.insert(variable);

          return variables;
        }

        typedef typename implementation_type::shared_context_type shared_context_type;

        classic_enumerator(boost::shared_ptr< shared_context_type > const& context,
             variable_type const& variable, expression_type const& condition,
             substitution_type const& substitution, Evaluator const& evaluator) {

          implementation_type::create(m_impl, context, make_set(variable), condition, evaluator, substitution);
        }

        template < typename Container >
        classic_enumerator(boost::shared_ptr< shared_context_type > const& context,
             Container const& variables, expression_type const& condition,
             substitution_type const& substitution, Evaluator const& evaluator, typename detail::enable_if_container< Container, variable >::type* = 0) {

          implementation_type::create(m_impl, context, variables, condition, evaluator, substitution);
        }

      public:

        /// \brief Constructs the past-end iterator
        classic_enumerator() {
        }

        /// \brief Copy constructor
        classic_enumerator(classic_enumerator const& other) {
          if (other.m_impl) {
            m_impl.reset(new implementation_type(*other.m_impl));
          }
        }

        /// \brief Copy constructor
        classic_enumerator& operator=(classic_enumerator const& other) {
          if (other.m_impl) {
            m_impl.reset(new implementation_type(*other.m_impl));
          }

          return *this;
        }

        /** \brief Constructs iterator representing a sequence of expressions
         *
         * Convenience function for enumeration over a single variable
         * \param[in] specification specification containing the definitions of sorts
         * \param[in] variable the variable for which to find valuatations
         * \param[in] condition the condition used for filtering generated substitutions
         * \param[in] substitution template for the substitution that is returned
         **/
        classic_enumerator(data_specification const& specification,
            variable_type const& variable,
            expression_type const& condition,
            substitution_type const& substitution) {

          implementation_type::create(m_impl, specification, make_set(variable), condition, substitution);
        }

        /** \brief Constructs iterator representing a sequence of expressions
         *
         * \param[in] specification specification containing the definitions of sorts
         * \param[in] variables the set of variables for which to find valuatations
         * \param[in] condition the condition used for filtering generated substitutions
         * \param[in] evaluator component that is used for evaluating conditions
         **/
        template < typename Container >
        classic_enumerator(data_specification const& specification,
            Container const& variables, Evaluator const& evaluator,
            expression_type const& condition = sort_bool::true_(),
            typename detail::enable_if_container< Container, variable >::type* = 0) {

          implementation_type::create(m_impl, specification, variables, condition, evaluator);
        }

        /** \brief Constructs iterator representing a sequence of expressions
         *
         * \param[in] specification specification containing the definitions of sorts
         * \param[in] variables the set of variables for which to find valuatations
         * \param[in] condition the condition used for filtering generated substitutions
         * \param[in] evaluator component that is used for evaluating conditions
         * \param[in] substitution template for the substitution that is returned
         **/
        template < typename Container >
        classic_enumerator(data_specification const& specification,
            Container const& variables,
            Evaluator const& evaluator,
            expression_type const& condition,
            substitution_type const& substitution,
            typename detail::enable_if_container< Container, variable >::type* = 0) {
          implementation_type::create(m_impl, specification, variables, condition, evaluator, substitution);
        }

        /** \brief Constructs iterator representing a sequence of expressions
         *
         * Convenience function for enumeration over a single variable
         * \param[in] specification specification containing the definitions of sorts
         * \param[in] variable the variable for which to find valuatations
         * \param[in] condition the condition used for filtering generated substitutions
         * \param[in] evaluator component that is used for evaluating conditions
         **/
        classic_enumerator(data_specification const& specification,
            variable_type const& variable, Evaluator const& evaluator,
            expression_type const& condition = sort_bool::true_()) {

          implementation_type::create(m_impl, specification, make_set(variable), condition, evaluator);
        }

        /** \brief Constructs iterator representing a sequence of expressions
         *
         * Convenience function for enumeration over a single variable
         * \param[in] specification specification containing the definitions of sorts
         * \param[in] variable the variable for which to find valuatations
         * \param[in] condition the condition used for filtering generated substitutions
         * \param[in] evaluator component that is used for evaluating conditions
         * \param[in] substitution template for the substitution that is returned
         **/
        classic_enumerator(data_specification const& specification,
            variable_type const& variable,
            Evaluator const& evaluator,
            expression_type const& condition,
            substitution_type const& substitution) {
          implementation_type::create(m_impl, specification, make_set(variable), condition, evaluator, substitution);
        }
    };

    /**
     * \brief Specialisation for enumerating without enumeration condition
     **/
    template < typename MutableSubstitution >
    class classic_enumerator< MutableSubstitution, void, selectors::select_all > :
                           public classic_enumerator< MutableSubstitution, rewriter, selectors::select_all >
    {
      typedef classic_enumerator< MutableSubstitution, rewriter, selectors::select_all > super;

      public:

        /// \brief The type of objects that represent substitutions
        typedef MutableSubstitution                                            substitution_type;
        /// \brief The type of objects that represent variables
        typedef typename MutableSubstitution::variable_type                    variable_type;
        /// \brief The type of objects that represent expressions
        typedef typename MutableSubstitution::expression_type                  expression_type;

      private:

        static typename super::evaluator_type& get_shared_evaluator()
        {
          static typename super::evaluator_type  evaluator;

          return evaluator;
        }

        static boost::shared_ptr< typename super::shared_context_type >& get_shared_context()
        {
          static boost::shared_ptr< typename super::shared_context_type > context(
            new typename super::shared_context_type(rewriter::default_specification(), get_shared_evaluator()));

          return context;
        }

      public:

         classic_enumerator()
         { }

        /** \brief Constructs iterator representing a sequence of expressions
         *
         * \param[in] specification specification containing the definitions of sorts
         * \param[in] variables the set of variables for which to find valuatations
         **/
        classic_enumerator(data_specification const& specification,
            variable_type const& variable) :
                   super(get_shared_context(), make_set(variable), sort_bool::true_(), substitution_type(), get_shared_evaluator())
        { }

        /** \brief Constructs iterator representing a sequence of expressions
         *
         * \param[in] specification specification containing the definitions of sorts
         * \param[in] variables the set of variables for which to find valuatations
         **/
        template < typename Container >
        classic_enumerator(data_specification const& specification,
            Container const& variables,
            typename detail::enable_if_container< Container, variable >::type* = 0) :
                   super(get_shared_context(), variables, sort_bool::true_(), substitution_type(), get_shared_evaluator())
        { }

    };

    /** \brief Constructs an enumerator without condition evaluation
    *
    * The resulting sequence of substitution is as if enumerating with
    * an arbitrary condition and a selector that selects any valuation of
    * variables.
    *
    * \param[in] specification the data context
    * \param[in] variables the set of variables for which to find valuatations
    **/
    template < typename VariableSequence >
    classic_enumerator< mutable_map_substitution< >, void, selectors::select_all >
    make_simple_classic_enumerator(data_specification const& specification, VariableSequence const& variables)
    {
      return classic_enumerator< mutable_map_substitution< >, void, selectors::select_all >(specification, variables);
    }

    /** \brief Constructs iterator range of expressions generated by
    * applying a sequence of substitutions to a single expression
    *
    * \param[in] specification specification containing the definitions of sorts
    * \param[in] variables the set of variables for which to find valuatations
    **/
    template < typename Enumerator >
    boost::iterator_range< detail::transform_iterator< apply_to_expression< typename Enumerator::expression_type >,
									 Enumerator, typename Enumerator::expression_type > >
    make_enumeration_sequence(typename Enumerator::expression_type const& base_expression, Enumerator const& enumerator)
    {
      typedef detail::transform_iterator< apply_to_expression< typename Enumerator::expression_type >, Enumerator, typename Enumerator::expression_type > iterator_type;

      return boost::make_iterator_range(
        iterator_type(enumerator, apply_to_expression< typename Enumerator::expression_type >(base_expression)),
        iterator_type(Enumerator()));
    }

  } // namespace data
} // namespace mcrl2

#endif

