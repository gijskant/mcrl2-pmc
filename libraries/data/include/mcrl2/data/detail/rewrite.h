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

#include "mcrl2/aterm/aterm2.h"
#include <mcrl2/atermpp/aterm_int.h>
#include "mcrl2/data/substitutions.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/selection.h"
#include "mcrl2/data/detail/rewrite_conversion_helper.h"
#include "mcrl2/data/fresh_variable_generator.h"

#ifndef NO_DYNLOAD
#define MCRL2_JITTYC_AVAILABLE /** \brief If defined the compiling JITty
rewriter is available */
#endif

namespace mcrl2
{
namespace data
{
namespace detail
{

/** \brief Rewrite strategies. */
typedef enum { 
               GS_REWR_JITTY,     /** \brief JITty */
#ifdef MCRL2_JITTYC_AVAILABLE
               GS_REWR_JITTYC,    /** \brief Compiling JITty */
#endif
               GS_REWR_JITTY_P,   /** \brief JITty + Prover */
#ifdef MCRL2_JITTYC_AVAILABLE
               GS_REWR_JITTYC_P,  /** \brief Compiling JITty + Prover*/
#endif
               GS_REWR_INVALID   /** \brief Invalid strategy */
             } RewriteStrategy;


atermpp::aterm_appl toInner(const data_expression term, const bool add_opids);

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
  protected:
    detail::rewrite_conversion_helper< Rewriter> m_conversion_helper; 
    data::fresh_variable_generator<> generator;  //name for variables.

  public:

    const atermpp::aterm_appl internal_true;
    const atermpp::aterm_appl internal_false;
    const atermpp::aterm internal_not;
    const atermpp::aterm internal_and;
    const atermpp::aterm internal_or;


    used_data_equation_selector data_equation_selector;
    /**
     * \brief Constructor. Do not use directly; use createRewriter()
     *        function instead.
     * \sa createRewriter()
     **/
    Rewriter():
       m_conversion_helper(*this),
       generator("v@r"),
       internal_true(toInner(sort_bool::true_(),true)),
       internal_false(toInner(sort_bool::false_(),true)),
       internal_not(toInner(sort_bool::not_(),true)(0)),
       internal_and(toInner(sort_bool::and_(),true)(0)),
       internal_or(toInner(sort_bool::or_(),true)(0))
    { 
      internal_true.protect();
      internal_false.protect();
    }

    /** \brief Destructor. */
    virtual ~Rewriter()
    {
      internal_true.unprotect();
      internal_false.unprotect();
    }

    /**
     * \brief Get rewriter strategy that is used.
     * \return Used rewriter strategy.
     **/
    virtual RewriteStrategy getStrategy() = 0;

    /**
     * \brief Rewrite an mCRL2 data term.
     * \param Term The term to be rewritten. This term should be a data_term
     * \return The normal form of Term.
     **/
    
    virtual data_expression rewrite(const data_expression term, mutable_map_substitution<> &sigma) = 0;

    /**
     * \brief Rewrite a list of mCRL2 data terms.
     * \param Terms The list of terms to be rewritten. These terms
     *              should be in the internal mCRL2 format.
     * \return The list Terms where each element is replaced by its
     *         normal form.
     **/
    virtual data_expression_list rewrite_list(const data_expression_list Terms, mutable_map_substitution<> &sigma);

    /**
     * \brief Convert an mCRL2 data term to a term in the internal
     *        rewriter format.
     * \param Term A mCRL2 data term.
     * \return The term Term in the internal rewriter format.
     **/
    virtual atermpp::aterm_appl toRewriteFormat(const data_expression Term);
    /**
     * \brief Convert a term in the internal rewriter format to a
     *        mCRL2 data term.
     * \param Term A term in the internal rewriter format.
     * \return The term Term as an mCRL2 data term.
     **/
    virtual data_expression fromRewriteFormat(const atermpp::aterm_appl Term);
    /**
     * \brief Rewrite a term in the internal rewriter format.
     * \param Term The term to be rewritten. This term should be
     *             in the internal rewriter format.
     * \return The normal form of Term.
     **/
    virtual atermpp::aterm_appl rewrite_internal(
                     const atermpp::aterm_appl Term, 
                     mutable_map_substitution<atermpp::map < variable,atermpp::aterm_appl> > &sigma);
    /**
     * \brief Rewrite a list of terms in the internal rewriter
     *        format.
     * \param Terms The list of terms to be rewritten. These terms
     *              should be in the internal rewriter format.
     * \return The list Terms where each element is replaced by its
     *         normal form.
     **/
    virtual atermpp::term_list < atermpp::aterm_appl > rewrite_internal_list(
                     const atermpp::term_list < atermpp::aterm_appl > Terms, 
                     mutable_map_substitution<atermpp::map < variable,atermpp::aterm_appl> > &sigma);

    /**
     * \brief Add a rewrite rule to this rewriter.
     * \param Rule A mCRL2 rewrite rule (DataEqn).
     * \return Whether or not the rule was succesfully added. Note
     *         that some rewriters do not support adding of rewrite
     *         rules altogether and will always return false.
     **/
    virtual bool addRewriteRule(const data_equation rule);
    /**
     * \brief Remove a rewrite rule from this rewriter (if present).
     * \param Rule A mCRL2 rewrite rule (DataEqn).
     * \return Whether or not the rule was succesfully removed. Note
     *         that some rewriters do not support removing of
     *         rewrite rules altogether and will always return
     *         false.
     **/
    virtual bool removeRewriteRule(const data_equation rule);

  public:
  /* The functions below are public, because they are used in the compiling jitty rewriter */
    atermpp::aterm_appl internal_existential_quantifier_enumeration(
         const atermpp::aterm_appl termInInnerFormat,
         mutable_map_substitution<atermpp::map < variable,atermpp::aterm_appl> > &sigma);
    atermpp::aterm_appl internal_universal_quantifier_enumeration(
         const atermpp::aterm_appl termInInnerFormat,
         mutable_map_substitution<atermpp::map < variable,atermpp::aterm_appl> > &sigma);

  protected:

    mcrl2::data::data_specification m_data_specification_for_enumeration;
    atermpp::aterm_appl internal_quantifier_enumeration(
         const atermpp::aterm_appl termInInnerFormat,
         mutable_map_substitution<atermpp::map < variable,atermpp::aterm_appl> > &sigma);

    core::identifier_string forall_function_symbol()
    {
      static core::identifier_string forall_function_symbol = initialise_static_expression(forall_function_symbol, core::identifier_string("forall"));
      return forall_function_symbol;
    }

    core::identifier_string exists_function_symbol()
    {
      static core::identifier_string exists_function_symbol = initialise_static_expression(exists_function_symbol, core::identifier_string("exists"));
      return exists_function_symbol;
    }

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
             const RewriteStrategy Strategy = GS_REWR_JITTY);

/**
 * \brief Check that an mCRL2 data equation is a valid rewrite rule. If not, an runtime_error is thrown indicating the problem.
 * \param DataEqn The mCRL2 data equation to be checked.
 * \throw std::runtime_error containing a reason why DataEqn is not a valid rewrite rule.
 **/
void CheckRewriteRule(const data_equation dataeqn);

/**
 * \brief Check whether or not an mCRL2 data equation is a valid rewrite rule.
 * \param DataEqn The mCRL2 data equation to be checked.
 * \return Whether or not DataEqn is a valid rewrite rule.
 **/
bool isValidRewriteRule(const data_equation dataeqn);

/**
 * \brief Print a string representation of a rewrite strategy.
 * \param stream File stream to print to.
 * \param strat  Rewrite strategy to print.
 **/
void PrintRewriteStrategy(FILE* stream, RewriteStrategy strat);
/**
 * \brief Get rewrite strategy from its string representation.
 * \param s String representation of strategy.
 * \return Rewrite strategy represented by s. If s is not a valid rewrite
 *         strategy, ::GS_REWR_INVALID is returned.
 **/
RewriteStrategy RewriteStrategyFromString(char const* s);

// extern size_t num_apples;
extern std::vector <AFun> apples;

/** \brief Get the AFun number of the internal application symbol with given arity. */
inline AFun get_appl_afun_value(size_t arity)
{
  if (arity >= apples.size())
  {
    for (size_t old_num=apples.size(); old_num <=arity; ++old_num)
    {
      assert(old_num==apples.size());
      apples.push_back(ATmakeAFun("#REWR#",old_num,false));
      ATprotectAFun(apples[old_num]);
    }
  }
  assert(arity<apples.size());
  return apples[arity];
}

/**
 * \brief The apply functions below takes terms in internal format,
 *        and transform them into a function application. In case
 *        of Apply and ApplyArray the first element of the list
 *        or array is the function symbol.
 **/
inline atermpp::aterm_appl Apply(const atermpp::term_list < atermpp::aterm > l)
{
  return ATmakeApplList(get_appl_afun_value(l.size()),l);
}

/** \brief See Apply. */
inline atermpp::aterm_appl ApplyArray(const size_t size, const atermpp::aterm *l)
{
  return ATmakeApplArray(get_appl_afun_value(size), (ATerm*)l);
}


/** \brief See Apply. */
inline atermpp::aterm_appl Apply0(const atermpp::aterm head)
{
 return ATmakeAppl1(get_appl_afun_value(1),(ATerm)head);
}


/** \brief See Apply. */
inline atermpp::aterm_appl Apply1(
         const atermpp::aterm head, 
         const atermpp::aterm_appl arg1)
{
 return ATmakeAppl2(get_appl_afun_value(2),(ATerm)head,(ATerm)(ATermAppl)arg1);
}


/** \brief See Apply. */
inline atermpp::aterm_appl Apply2(
         const atermpp::aterm head, 
         const atermpp::aterm_appl arg1, 
         const atermpp::aterm_appl arg2)
{
 return ATmakeAppl3(get_appl_afun_value(3),(ATerm)head,(ATerm)(ATermAppl)arg1,(ATerm)(ATermAppl)arg2);
}

/** \brief See Apply. */
inline atermpp::aterm_appl Apply3(
         const atermpp::aterm head, 
         const atermpp::aterm_appl arg1, 
         const atermpp::aterm_appl arg2, 
         const atermpp::aterm_appl arg3)
{
 return ATmakeAppl4(get_appl_afun_value(4),(ATerm)head,(ATerm)(ATermAppl)arg1,(ATerm)(ATermAppl)arg2,(ATerm)(ATermAppl)arg3);
}

/** The functions below are used for fromInner and toInner(c). */

size_t get_num_opids();

function_symbol get_int2term(const size_t n);

atermpp::map< function_symbol, atermpp::aterm_int >::const_iterator term2int_begin();

atermpp::map< data::function_symbol, atermpp::aterm_int >::const_iterator term2int_end();

inline size_t getArity(const data::function_symbol op)
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


atermpp::aterm_int OpId2Int(const function_symbol);

data_expression fromInner(const atermpp::aterm_appl term);

atermpp::aterm_appl toInner(const data_expression Term, const bool add_opids);

}
}
}

#endif
