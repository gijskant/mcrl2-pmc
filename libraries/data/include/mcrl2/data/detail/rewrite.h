// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/detail/rewrite.h

#ifndef __LIBREWRITE_H
#define __LIBREWRITE_H

#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/data/substitutions.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/selection.h"
#include "mcrl2/data/rewrite_strategy.h"
#include "mcrl2/data/set_identifier_generator.h"

namespace mcrl2
{
namespace data
{
namespace detail
{

atermpp::aterm_appl toInner(const data_expression &term, const bool add_opids);

/**
 * \brief Rewriter interface class.
 *
 * This is the interface class for the rewriters. To create a specific
 * rewriter, use createRewriter.
 *
 * Simple use of the rewriter would be as follow (with t a term in the mCRL2
 * internal format):
 *
 * \code
 *   Rewriter *r = createRewriter(equations);
 *   t = r->rewrite(t);
 *   delete r;
 * \endcode
 **/
class Rewriter
{
  public:
    data::set_identifier_generator generator;  //name for variables.

    typedef mutable_indexed_substitution<data::variable, std::vector< atermpp::aterm_appl > > internal_substitution_type;
    typedef mutable_indexed_substitution<data::variable, std::vector< data::data_expression > > substitution_type;

    atermpp::aterm_appl internal_true;
    atermpp::aterm_appl internal_false;
    atermpp::aterm internal_not;
    atermpp::aterm internal_and;
    atermpp::aterm internal_or;


    used_data_equation_selector data_equation_selector;
    /**
     * \brief Constructor. Do not use directly; use createRewriter()
     *        function instead.
     * \sa createRewriter()
     **/
    Rewriter()
    {
      internal_true=toInner(sort_bool::true_(),true);
      internal_false=toInner(sort_bool::false_(),true);
      internal_not=toInner(sort_bool::not_(),true)[0];
      internal_and=toInner(sort_bool::and_(),true)[0];
      internal_or=toInner(sort_bool::or_(),true)[0];
    }

    /** \brief Destructor. */
    virtual ~Rewriter()
    {
    }

    /**
     * \brief Get rewriter strategy that is used.
     * \return Used rewriter strategy.
     **/
    virtual rewrite_strategy getStrategy() = 0;

    /**
     * \brief Rewrite an mCRL2 data term.
     * \param Term The term to be rewritten. This term should be a data_term
     * \return The normal form of Term.
     **/

    virtual data_expression rewrite(const data_expression &term, substitution_type &sigma) = 0;

    /**
     * \brief Rewrite a list of mCRL2 data terms.
     * \param Terms The list of terms to be rewritten. These terms
     *              should be in the internal mCRL2 format.
     * \return The list Terms where each element is replaced by its
     *         normal form.
     **/
    virtual data_expression_list rewrite_list(const data_expression_list &Terms, substitution_type &sigma);

    /**
     * \brief Convert an mCRL2 data term to a term in the internal
     *        rewriter format.
     * \param Term A mCRL2 data term.
     * \return The term Term in the internal rewriter format.
     **/
    virtual atermpp::aterm_appl toRewriteFormat(const data_expression &Term);
    /**
     * \brief Convert a term in the internal rewriter format to a
     *        mCRL2 data term.
     * \param Term A term in the internal rewriter format.
     * \return The term Term as an mCRL2 data term.
     **/
    data_expression fromRewriteFormat(const atermpp::aterm_appl &Term);
    /**
     * \brief Rewrite a term in the internal rewriter format.
     * \param Term The term to be rewritten. This term should be
     *             in the internal rewriter format.
     * \return The normal form of Term.
     **/
    virtual atermpp::aterm_appl rewrite_internal(
                     const atermpp::aterm_appl &Term,
                     internal_substitution_type &sigma);
    /**
     * \brief Rewrite a list of terms in the internal rewriter
     *        format.
     * \param Terms The list of terms to be rewritten. These terms
     *              should be in the internal rewriter format.
     * \return The list Terms where each element is replaced by its
     *         normal form.
     **/
    virtual atermpp::term_list < atermpp::aterm_appl > rewrite_internal_list(
                     const atermpp::term_list < atermpp::aterm_appl > &Terms,
                     internal_substitution_type &sigma);

    /**
     * \brief Add a rewrite rule to this rewriter.
     * \param Rule A mCRL2 rewrite rule (DataEqn).
     * \return Whether or not the rule was succesfully added. Note
     *         that some rewriters do not support adding of rewrite
     *         rules altogether and will always return false.
     **/
    virtual bool addRewriteRule(const data_equation &rule);
    /**
     * \brief Remove a rewrite rule from this rewriter (if present).
     * \param Rule A mCRL2 rewrite rule (DataEqn).
     * \return Whether or not the rule was succesfully removed. Note
     *         that some rewriters do not support removing of
     *         rewrite rules altogether and will always return
     *         false.
     **/
    virtual bool removeRewriteRule(const data_equation &rule);

  public:
  /* The functions below are public, because they are used in the compiling jitty rewriter */
    /* atermpp::aterm_appl internal_existential_quantifier_enumeration(
         const atermpp::aterm_appl termInInnerFormat,
         internal_substitution_type &sigma); */
    atermpp::aterm_appl internal_existential_quantifier_enumeration(
         const atermpp::aterm_appl &termInInnerFormat,
         internal_substitution_type &sigma);
    atermpp::aterm_appl internal_existential_quantifier_enumeration(
         const variable_list &vl,
         const atermpp::aterm_appl &t1,
         const bool t1_is_normal_form,
         internal_substitution_type &sigma);

    /* atermpp::aterm_appl internal_universal_quantifier_enumeration(
         const atermpp::aterm_appl termInInnerFormat,
         internal_substitution_type &sigma); */
    atermpp::aterm_appl internal_universal_quantifier_enumeration(
         const atermpp::aterm_appl &termInInnerFormat,
         internal_substitution_type &sigma);
    atermpp::aterm_appl internal_universal_quantifier_enumeration(
         const variable_list &vl,
         const atermpp::aterm_appl &t1,
         const bool t1_is_normal_form,
         internal_substitution_type &sigma);

    // Rewrite a where expression where the subdataexpressions are in internal format.
    // It yields a term without a where expression.
    atermpp::aterm_appl rewrite_where(
                      const atermpp::aterm_appl &term,
                      internal_substitution_type &sigma);

    // Rewrite an expression with a lambda as outermost symbol. The expression is in internal format.
    // Bound variables are replaced by new variables to avoid a clash with variables in the right hand sides
    // of sigma.

    atermpp::aterm_appl rewrite_single_lambda(
                      const variable_list &vl,
                      const atermpp::aterm_appl &body,
                      const bool body_in_normal_form,
                      internal_substitution_type &sigma);

    atermpp::aterm_appl rewrite_lambda_application(
                      const atermpp::aterm_appl &lambda_term,
                      const atermpp::aterm_appl &body,
                      internal_substitution_type &sigma);


  protected:

    mcrl2::data::data_specification m_data_specification_for_enumeration;
    atermpp::aterm_appl internal_quantifier_enumeration(
         const atermpp::aterm_appl &termInInnerFormat,
         internal_substitution_type &sigma);

};

/**
 * \brief Create a rewriter.
 * \param DataSpec A data specification.
 * \param Strategy The rewrite strategy to be used by the rewriter.
 * \return A (pointer to a) rewriter that uses the data specification DataSpec
 *         and strategy Strategy to rewrite.
 **/
Rewriter* createRewriter(
             const data_specification& DataSpec,
             const used_data_equation_selector &equations_selector,
             const rewrite_strategy Strategy = jitty);

/**
 * \brief Check that an mCRL2 data equation is a valid rewrite rule. If not, an runtime_error is thrown indicating the problem.
 * \param DataEqn The mCRL2 data equation to be checked.
 * \throw std::runtime_error containing a reason why DataEqn is not a valid rewrite rule.
 **/
void CheckRewriteRule(const data_equation &dataeqn);

/**
 * \brief Check whether or not an mCRL2 data equation is a valid rewrite rule.
 * \param DataEqn The mCRL2 data equation to be checked.
 * \return Whether or not DataEqn is a valid rewrite rule.
 **/
bool isValidRewriteRule(const data_equation &dataeqn);

extern std::vector <atermpp::function_symbol> apples;

inline void extend_appl_afun_values(const size_t arity)
{
  for (size_t old_num=apples.size(); old_num <=arity; ++old_num)
  {
    assert(old_num==apples.size());
    apples.push_back(atermpp::function_symbol("#REWR#",old_num));
  }
}

/** \brief Get the atermpp::function_symbol number of the internal application symbol with given arity. */
inline const atermpp::function_symbol& get_appl_afun_value(const size_t arity)
{
  if (arity >= apples.size())
  {
    extend_appl_afun_values(arity);
  }
  assert(arity<apples.size());
  return apples[arity];
}

/** \brief Get the atermpp::function_symbol number of the internal application symbol with given arity. */
inline const atermpp::function_symbol& get_appl_afun_value_no_check(const size_t arity)
{
  assert(arity<apples.size());
  return apples[arity];
}

/**
 * \brief The apply functions below takes terms in internal format,
 *        and transform them into a function application. In case
 *        of Apply and ApplyArray the first element of the list
 *        or array is the function symbol.
 **/
// inline atermpp::aterm_appl Apply(const atermpp::term_list < atermpp::aterm > &l)
// {
//   return atermpp::aterm_appl(get_appl_afun_value(l.size()),l.begin(),l.end());
// }

/** \brief See Apply. */
template <class Iterator>
inline atermpp::aterm_appl ApplyArray(const size_t size, const Iterator begin, const Iterator end)
{
  return atermpp::aterm_appl(get_appl_afun_value(size), begin, end);
}

/** \brief See Apply. */
template <class Iterator, class Function>
inline atermpp::aterm_appl ApplyArray(const size_t size, const Iterator begin, const Iterator end, Function f)
{
  return atermpp::aterm_appl(get_appl_afun_value(size), begin, end, f);
}


/** \brief See Apply. */
inline atermpp::aterm_appl Apply0(const atermpp::aterm &head)
{
  return atermpp::aterm_appl(get_appl_afun_value(1),head);
}


/** \brief See Apply. */
inline atermpp::aterm_appl Apply1(
         const atermpp::aterm &head,
         const atermpp::aterm_appl &arg1)
{
  return atermpp::aterm_appl(get_appl_afun_value(2),head,arg1);
}


/** \brief See Apply. */
inline atermpp::aterm_appl Apply2(
         const atermpp::aterm &head,
         const atermpp::aterm_appl &arg1,
         const atermpp::aterm_appl &arg2)
{
  return atermpp::aterm_appl(get_appl_afun_value(3),head,arg1,arg2);
}

/** \brief See Apply. */
inline atermpp::aterm_appl Apply3(
         const atermpp::aterm &head,
         const atermpp::aterm_appl &arg1,
         const atermpp::aterm_appl &arg2,
         const atermpp::aterm_appl &arg3)
{
  return atermpp::aterm_appl(get_appl_afun_value(4),head,arg1, arg2,arg3);
}

/** The functions below are used for fromInner and toInner(c). */

inline size_t getArity(const data::function_symbol &op)
{
  // This function calculates the cumulated length of all
  // potential function arguments.
  sort_expression sort = op.sort();
  size_t arity = 0;

  while (is_function_sort(sort))
  {
    const function_sort fsort(sort);
    sort_expression_list sort_dom = fsort.domain();
    arity += sort_dom.size();
    sort = fsort.codomain();
  }
  return arity;
}

extern std::map< function_symbol, atermpp::aterm_int > term2int;
extern std::vector < data::function_symbol > int2term;

inline size_t get_num_opids()
{
  return int2term.size();
}

inline function_symbol &get_int2term(const size_t n)
{
  assert(n<int2term.size());
  return int2term[n];
}

inline const atermpp::aterm_int &OpId2Int_aux(const function_symbol &term)
{
  const size_t num_opids=get_num_opids();
  atermpp::aterm_int i(num_opids);
  term2int[term] =  i;
  assert(int2term.size()==num_opids);
  int2term.push_back(term);
  return term2int[term];
}

inline const atermpp::aterm_int &OpId2Int(const function_symbol &term)
{
  std::map< function_symbol, atermpp::aterm_int >::const_iterator f = term2int.find(term);
  if (f == term2int.end())
  {
    return OpId2Int_aux(term);
  }
  return f->second;
}

data_expression fromInner(const atermpp::aterm_appl &term);

atermpp::aterm_appl toInner(const data_expression &Term, const bool add_opids);

}
}
}

#endif
