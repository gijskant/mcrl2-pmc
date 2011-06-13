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

// #include "mcrl2/atermpp/map.h"
#include "mcrl2/aterm/aterm2.h"
#include "mcrl2/data/data_specification.h"

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
 *
 * Most rewriters use their own format to store data terms (for reasons of
 * performance). To make optimal use of this, one can convert terms to this
 * format and use the specialised rewrite function on such terms. This is
 * especially useful when rewriting the same term for many different
 * instantiations of variables occurring in that term.
 *
 * Instead of first substituting specific values for variables before rewriting
 * a term, one can tell the rewriter to do this substitution during rewriting.
 * Typical usage would be as follows (with t an mCRL2 data term, var an mCRL2
 * data variable and values a list of mCRL2 data terms):
 *
 * \code
 *   Rewriter *r = createRewriter(equations);
 *   for (iterator i = values.begin(); i != values.end(); i++)
 *   {
 *     r->setSubstitution(var,*i);
 *     ATerm v = t->rewrite(t);
 *     // v is the normal form in of t[var:=*i]
 *     ...
 *   }
 *   delete r;
 * \endcode
 **/
class Rewriter
{
  public:
    /**
     * \brief Constructor. Do not use directly; use createRewriter()
     *        function instead.
     * \sa createRewriter()
     **/
    Rewriter();
    /** \brief Destructor. */
    virtual ~Rewriter();

    /**
     * \brief Get rewriter strategy that is used.
     * \return Used rewriter strategy.
     **/
    virtual RewriteStrategy getStrategy() = 0;

    /**
     * \brief Rewrite an mCRL2 data term.
     * \param Term The term to be rewritten. This term should be
     *             in the internal mCRL2 format.
     * \return The normal form of Term.
     **/
    virtual ATermAppl rewrite(ATermAppl Term) = 0;
    /**
     * \brief Rewrite a list of mCRL2 data terms.
     * \param Terms The list of terms to be rewritten. These terms
     *              should be in the internal mCRL2 format.
     * \return The list Terms where each element is replaced by its
     *         normal form.
     **/
    virtual ATermList rewriteList(ATermList Terms);

    /**
     * \brief Convert an mCRL2 data term to a term in the internal
     *        rewriter format.
     * \param Term A mCRL2 data term.
     * \return The term Term in the internal rewriter format.
     **/
    virtual ATerm toRewriteFormat(ATermAppl Term);
    /**
     * \brief Convert a term in the internal rewriter format to a
     *        mCRL2 data term.
     * \param Term A term in the internal rewriter format.
     * \return The term Term as an mCRL2 data term.
     **/
    virtual ATermAppl fromRewriteFormat(ATerm Term);
    /**
     * \brief Rewrite a term in the internal rewriter format.
     * \param Term The term to be rewritten. This term should be
     *             in the internal rewriter format.
     * \return The normal form of Term.
     **/
    virtual ATerm rewriteInternal(ATerm Term);
    /**
     * \brief Rewrite a list of terms in the internal rewriter
     *        format.
     * \param Terms The list of terms to be rewritten. These terms
     *              should be in the internal rewriter format.
     * \return The list Terms where each element is replaced by its
     *         normal form.
     **/
    virtual ATermList rewriteInternalList(ATermList Terms);

    /**
     * \brief Add a rewrite rule to this rewriter.
     * \param Rule A mCRL2 rewrite rule (DataEqn).
     * \return Whether or not the rule was succesfully added. Note
     *         that some rewriters do not support adding of rewrite
     *         rules altogether and will always return false.
     **/
    virtual bool addRewriteRule(ATermAppl Rule);
    /**
     * \brief Remove a rewrite rule from this rewriter (if present).
     * \param Rule A mCRL2 rewrite rule (DataEqn).
     * \return Whether or not the rule was succesfully removed. Note
     *         that some rewriters do not support removing of
     *         rewrite rules altogether and will always return
     *         false.
     **/
    virtual bool removeRewriteRule(ATermAppl Rule);

    /**
     * \brief Link a variable to a value for on-the-fly
     *        substitution. (Replacing any previous linked value.)
     * \param var  A mCRL2 data variable.
     * \param expr A mCRL2 data expression.
     **/
    virtual void setSubstitution(ATermAppl var, ATermAppl expr)
    {
      setSubstitutionInternal(var,toRewriteFormat(expr));
    }

    /**
     * \brief Link variables to a values for on-the-fly
     *        substitution. (Replacing any previous linked value.)
     * \param Substs A lists of substitutions of mCRL2 data
                 *               variables to mCRL2 data expressions.
     **/
    virtual void setSubstitutionList(ATermList Substs)
    {
      for (; !ATisEmpty(Substs); Substs=ATgetNext(Substs))
      {
        ATermAppl h = (ATermAppl) ATgetFirst(Substs);
        setSubstitutionInternal((ATermAppl) ATgetArgument(h,0),toRewriteFormat((ATermAppl) ATgetArgument(h,1)));
      }
    }

    
    /**
     * \brief Link a variable to a value for on-the-fly
     *        substitution. (Replacing any previous linked value.)
     * \param var  A mCRL2 data variable.
     * \param expr A term in the internal rewriter format.
     **/
    virtual void setSubstitutionInternal(ATermAppl var, ATerm expr)
    {
      size_t n = ATgetAFun(ATgetArgument(var,0));

      if (n>=m_substitution.size())
      {
        m_substitution.resize(n+1);
      }
      m_substitution[n]=(ATermAppl)expr;
    }

    /**
     * \brief Link variables to a values for on-the-fly
     *        substitution. (Replacing any previous linked value.)
     * \param Substs A lists of substitutions of mCRL2 data
                 *               variables to terms in the internal rewriter
                 *               format.
     **/
    virtual void setSubstitutionInternalList(ATermList substs)
    {
      for (; !ATisEmpty(substs); substs=ATgetNext(substs))
      {
        const ATermAppl h = (ATermAppl) ATgetFirst(substs);
        setSubstitutionInternal((ATermAppl) ATgetArgument(h,0),ATgetArgument(h,1));
      }
    }

    /**
     * \brief Get the value linked to a variable for on-the-fly
     *        substitution.
     * \param var A mCRL2 data variable.
     * \return The value linked to var as an mCRL2 data expression.
                 *         If no value is linked to var, then NULL is returned.
     **/
    virtual ATermAppl getSubstitution(ATermAppl var)
    {
      return fromRewriteFormat(getSubstitutionInternal(var));
    }

    /**
     * \brief Get the value linked to a variable for on-the-fly
     *        substitution.
     * \param var A mCRL2 data variable.
     * \return The value linked to var as a term in the internal
                 *         rewriter format. If no value is linked to var,
     *         then NULL is returned.
     **/
    virtual ATerm getSubstitutionInternal(ATermAppl var)
    {
      const size_t n = ATgetAFun(ATgetArgument(var,0));
      if (n>=m_substitution.size() || m_substitution[n]==atermpp::aterm_appl())
      {
        return (ATerm)var;
      }
      return (ATerm)(ATermAppl)m_substitution[n];
    }

    /**
     * \brief Remove the value linked to a variable for on-the-fly
     *        substitution. (I.e. make sure that no value is
     *        substituted for this variable during rewriting.)
     * \param var A mCRL2 data variable.
     **/
    virtual void clearSubstitution(ATermAppl var)
    {
      const size_t n = ATgetAFun(ATgetArgument(var,0));

      if (n < m_substitution.size())
      {
        m_substitution[n] = atermpp::aterm_appl();
      }
    }


    /**
     * \brief Remove all values linked to a variable for on-the-fly
     *        substitution. (I.e. make sure that no substitution is
     *        done during rewriting.)
     **/
    virtual void clearSubstitutions()
    {
      m_substitution.clear();
    }

    /**
     * \brief Remove the values linked to variables for on-the-fly
     *        substitution. (I.e. make sure that no value is
     *        substituted for this variable during rewriting.)
     * \param vars A list of mCRL2 data variable.
     **/
    virtual void clearSubstitutions(ATermList vars)
    {
      for (; !ATisEmpty(vars); vars=ATgetNext(vars))
      {
        clearSubstitution((ATermAppl) ATgetFirst(vars));
      }
    }


  /* protected:
    ATerm lookupSubstitution(ATermAppl var); */

  protected:
    atermpp::vector < atermpp::aterm_appl > m_substitution;  // Substitution for variables to terms in internal format.

    mcrl2::data::data_specification m_data_specification_for_enumeration;
    ATerm internal_quantifier_enumeration( ATerm ATermInInnerFormat );

    ATerm internal_existential_quantifier_enumeration( ATerm ATermInInnerFormat );
    ATerm internal_universal_quantifier_enumeration( ATerm ATermInInnerFormat );

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
Rewriter* createRewriter(const data_specification& DataSpec, const RewriteStrategy Strategy = GS_REWR_JITTY, const bool add_rewrite_rules=true);

/**
 * \brief Check that an mCRL2 data equation is a valid rewrite rule. If not, an runtime_error is thrown indicating the problem.
 * \param DataEqn The mCRL2 data equation to be checked.
 * \throw std::runtime_error containing a reason why DataEqn is not a valid rewrite rule.
 **/
void CheckRewriteRule(ATermAppl DataEqn);

/**
 * \brief Check whether or not an mCRL2 data equation is a valid rewrite rule.
 * \param DataEqn The mCRL2 data equation to be checked.
 * \return Whether or not DataEqn is a valid rewrite rule.
 **/
bool isValidRewriteRule(ATermAppl DataEqn);

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
RewriteStrategy RewriteStrategyFromString(const char* s);

/**
 * \brief The apply functions below takes terms in internal format,
 *        and transform them into a function application. In case
 *        of Apply and ApplyArray the first element of the list
 *        or array is the function symbol.
 **/
ATermAppl Apply(ATermList l);

/** \brief See Apply. */
ATermAppl ApplyArray(const size_t size, ATerm *l);

/** \brief See Apply. */
ATermAppl Apply0(const ATerm head);

/** \brief See Apply. */
ATermAppl Apply1(const ATerm head, const ATerm arg1);

/** \brief See Apply. */
ATermAppl Apply2(const ATerm head, const ATerm arg1, const ATerm arg2);

/** \brief Get the AFun number of the internal application symbol with given arity. */
AFun get_appl_afun_value(size_t arity);

/** The functions below are used for fromInner and toInner(c). */

size_t get_num_opids();

ATermAppl get_int2term(const size_t n);

void set_int2term(const size_t n, const ATermAppl t);

atermpp::map< ATerm, ATermInt >::const_iterator term2int_begin();

atermpp::map< ATerm, ATermInt >::const_iterator term2int_end();

size_t getArity(ATermAppl op);

ATerm OpId2Int(ATermAppl Term, bool add_opids);

ATerm toInner(ATermAppl Term, bool add_opids);

ATermAppl fromInner(ATerm Term);

ATermAppl toInnerc(ATermAppl Term, const bool add_opids);

void initialize_internal_translation_table_rewriter();



}
}
}

#endif
