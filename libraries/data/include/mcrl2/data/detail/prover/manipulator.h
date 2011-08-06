// Author(s): Luc Engelen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/detail/prover/manipulator.h
/// \brief Interface to classes ATerm_Manipulator, AM_Jitty and AM_Inner

#ifndef MANIPULATOR_H
#define MANIPULATOR_H

#include "mcrl2/data/detail/prover/info.h"
#include "mcrl2/aterm/aterm_ext.h"

namespace mcrl2
{
namespace data
{
namespace detail
{

/// \brief Base class for classes that provide functionality to modify or create
/// \brief terms in one of the internal formats of the rewriter.

class ATerm_Manipulator
{
  protected:
    /// \brief The rewriter used to translate formulas to one of the internal formats of the rewriter.
    boost::shared_ptr<detail::Rewriter> f_rewriter;

    /// \brief A class that provides information on the structure of expressions in one of the
    /// \brief internal formats of the rewriter.
    ATerm_Info* f_info;

    /// \brief A table used by the method ATerm_Manipulator::orient.
    /// The method ATerm_Manipulator::orient stores resulting terms in this
    /// table. If a term is encountered that has already been processed, it is
    /// not processed again, but retreived from this table.
    atermpp::map < atermpp::aterm_appl, atermpp::aterm_appl> f_orient;

    /// \brief ATerm representing \c true in one of the internal formats of the rewriter.
    atermpp::aterm_appl f_true;

    /// \brief ATerm representing \c false in one of the internal formats of the rewriter.
    atermpp::aterm_appl f_false;

    /// \brief ATerm representing the \c if \c then \c else function in one of the internal formats of the rewriter.
    atermpp::aterm f_if_then_else;

    /// \brief Replaces all occurences of \c a_guard in \c a_formula by \c true. Additionally, if the variable
    /// \brief on the righthand side of the guard is encountered in \c a_formula, it is replaced by the variable
    /// \brief on the lefthand side.
    virtual atermpp::aterm_appl set_true_auxiliary(
              const atermpp::aterm_appl a_formula, 
              const atermpp::aterm_appl a_guard, 
              atermpp::map < atermpp::aterm_appl, atermpp::aterm_appl > &) = 0;

    /// \brief Replaces all occurences of \c a_guard in \c a_formula by \c false.
    virtual atermpp::aterm_appl set_false_auxiliary(
              const atermpp::aterm_appl a_formula, 
              const atermpp::aterm_appl a_guard, 
              atermpp::map < atermpp::aterm_appl, atermpp::aterm_appl > &f_set_true) = 0;

    /// \brief Returns an expression in one of the internal formats of the rewriter.
    /// \brief The main operator of this expression is an \c if \c then \c else function. Its guard is \c a_expr,
    /// \brief the true-branch is \c a_high and the false-branch is \c a_low.
    virtual atermpp::aterm_appl make_if_then_else(atermpp::aterm_appl a_expr, atermpp::aterm_appl a_high, atermpp::aterm_appl a_low) = 0;
  public:
    /// \brief Constructor initializing the rewriter and the field \c f_info.
    ATerm_Manipulator(boost::shared_ptr<detail::Rewriter> a_rewriter, ATerm_Info* a_info)
    {
      f_rewriter = a_rewriter;
      f_info = a_info;
    }

    /// \brief Destructor with no particular functionality.
    virtual ~ATerm_Manipulator()
    {
    }

    /// \brief Returns an expression in one of the internal formats of the rewriter.
    /// \brief The main operator of this expression is an \c if \c then \c else function. Its guard is \c a_expr,
    /// \brief the true-branch is \c a_high and the false-branch is \c a_low. If \c a_high equals \c a_low, the
    /// \brief method returns \c a_high instead.
    virtual atermpp::aterm_appl make_reduced_if_then_else(atermpp::aterm_appl a_expr, atermpp::aterm_appl a_high, atermpp::aterm_appl a_low) = 0;

    /// \brief Orients the term \c a_term such that all equations of the form t1 == t2 are
    /// \brief replaced by t2 == t1 if t1 > t2.
    virtual atermpp::aterm_appl orient(atermpp::aterm_appl a_term) = 0;

    /// \brief Initializes the table ATerm_Manipulator::f_set_true and calls the method
    /// \brief ATerm_Manipulator::f_set_true_auxiliary.
    virtual atermpp::aterm_appl set_true(atermpp::aterm_appl a_formula, atermpp::aterm_appl a_guard) = 0;

    /// \brief Initializes the table ATerm_Manipulator::f_set_false and calls the method
    /// \brief ATerm_Manipulator::f_set_false_auxiliary.
    virtual atermpp::aterm_appl set_false(atermpp::aterm_appl a_formula, atermpp::aterm_appl a_guard) = 0;
};

/// \brief Class that provides functionality to modify or create
/// \brief terms in the internal format of the rewriter with the
/// \brief Jitty strategy.

class AM_Jitty: public ATerm_Manipulator
{
  protected:
    /// \brief Replaces all occurences of \c a_guard in \c a_formula by \c true. Additionally, if the variable
    /// \brief on the righthand side of the guard is encountered in \c a_formula, it is replaced by the variable
    /// \brief on the lefthand side.
    virtual atermpp::aterm_appl set_true_auxiliary(
                const atermpp::aterm_appl a_formula, 
                const atermpp::aterm_appl a_guard,
                atermpp::map < atermpp::aterm_appl, atermpp::aterm_appl > &f_set_true)
    {
      if (a_formula == f_true || a_formula == f_false)
      {
        return a_formula;
      }
      if (a_formula == a_guard)
      {
        return f_true;
      }

      bool v_is_equality;

      v_is_equality = f_info->is_equality(a_guard);
      if (v_is_equality && ATgetArgument(a_guard, 2) == a_formula)
      {
        return ATgetArgument(a_guard, 1);
      }
      if (f_info->is_variable(a_formula))
      {
        return a_formula;
      }

      atermpp::map < atermpp::aterm_appl, atermpp::aterm_appl >::const_iterator i=f_set_true.find(a_formula);
      if (i!=f_set_true.end())
      {
        return i->second;
      }

      AFun v_symbol;
      atermpp::aterm v_function;
      size_t v_arity;

      v_symbol = ATgetAFun(a_formula);
      // v_function = atermpp::aterm_appl(ATgetArgument(a_formula, 0));
      v_function = a_formula(0);
      v_arity = ATgetArity(v_symbol);

      atermpp::aterm* v_parts = new atermpp::aterm[v_arity + 1];
      v_parts[0] = v_function;
      for (size_t i = 1; i < v_arity; i++)
      {
        v_parts[i] = set_true_auxiliary(ATgetArgument(a_formula, i), a_guard,f_set_true);
      }
      atermpp::aterm_appl v_result = atermpp::aterm_appl(ATmakeApplArray(v_symbol, (ATerm*)v_parts));
      f_set_true[a_formula]=v_result;
      delete[] v_parts;
      v_parts = 0;

      return v_result;
    }

    /// \brief Replaces all occurences of \c a_guard in \c a_formula by \c false.
    virtual atermpp::aterm_appl set_false_auxiliary(
              const atermpp::aterm_appl a_formula, 
              const atermpp::aterm_appl a_guard,
              atermpp::map < atermpp::aterm_appl, atermpp::aterm_appl > &f_set_false)
    {
      if (a_formula == f_true || a_formula == f_false)
      {
        return a_formula;
      }
      if (a_formula == a_guard)
      {
        return f_false;
      }
      if (f_info->is_variable(a_formula))
      {
        return a_formula;
      }

      atermpp::map < atermpp::aterm_appl, atermpp::aterm_appl >::const_iterator i=f_set_false.find(a_formula);
      if (i!=f_set_false.end())
      {
        return i->second;
      }

      AFun v_symbol;
      atermpp::aterm v_function;
      size_t v_arity;

      v_symbol = ATgetAFun(a_formula);
      v_function = a_formula(0);
      v_arity = ATgetArity(v_symbol);

      atermpp::aterm* v_parts = new atermpp::aterm[v_arity + 1];
      v_parts[0] = v_function;
      for (size_t i = 1; i < v_arity; i++)
      {
        v_parts[i] = set_false_auxiliary(ATgetArgument(a_formula, i), a_guard,f_set_false);
      }
      atermpp::aterm_appl v_result = atermpp::aterm_appl(ATmakeApplArray(v_symbol, (ATerm*)v_parts));
      f_set_false[a_formula]=v_result;
      delete[] v_parts;
      v_parts = 0;

      return v_result;
    }

    /// \brief Returns an expression in the internal format of the rewriter with the Jitty strategy.
    /// \brief The main operator of this expression is an \c if \c then \c else function. Its guard is \c a_expr,
    /// \brief the true-branch is \c a_high and the false-branch is \c a_low.
    virtual atermpp::aterm_appl make_if_then_else(atermpp::aterm_appl a_expr, atermpp::aterm_appl a_high, atermpp::aterm_appl a_low)
    {
      /* return (ATerm) ATmakeAppl4(
               ATmakeAFun("@REWR@", 4, false), f_if_then_else, a_expr, a_high, a_low); */
      return (atermpp::aterm_appl)Apply3((ATerm)f_if_then_else, (ATerm)(ATermAppl)a_expr, (ATerm)(ATermAppl)a_high, (ATerm)(ATermAppl)a_low);
    }

  public:
    /// \brief Constructor initializing all fields.
    AM_Jitty(boost::shared_ptr<detail::Rewriter> a_rewriter, ATerm_Info* a_info)
      : ATerm_Manipulator(a_rewriter, a_info)
    {
      using namespace mcrl2::core::detail;
      f_true =  a_rewriter->toRewriteFormat(sort_bool::true_());
      f_false = a_rewriter->toRewriteFormat(sort_bool::false_());
      f_if_then_else = atermpp::aterm(ATgetArgument((ATermAppl) a_rewriter->toRewriteFormat(if_(sort_bool::bool_())), 0));
    }

    /// \brief Destructor with no particular functionality.
    virtual ~AM_Jitty()
    {}

    /// \brief Returns an expression in the internal format of the rewriter with the jitty strategy.
    /// \brief The main operator of this expression is an \c if \c then \c else function. Its guard is \c a_expr,
    /// \brief the true-branch is \c a_high and the false-branch is \c a_low. If \c a_high equals \c a_low, the
    /// \brief method returns \c a_high instead.
    virtual atermpp::aterm_appl make_reduced_if_then_else(atermpp::aterm_appl a_expr, atermpp::aterm_appl a_high, atermpp::aterm_appl a_low)
    {
      if (a_high == a_low)
      {
        return a_high;
      }
      else
      {
        return make_if_then_else(a_expr, a_high, a_low);
      }
    }

    /// \brief Orients the term \c a_term such that all equations of the form t1 == t2 are
    /// \brief replaced by t2 == t1 if t1 > t2.
    virtual atermpp::aterm_appl orient(atermpp::aterm_appl a_term)
    {
      // v_result is NULL if not found; Therefore type ATerm.
      atermpp::map < atermpp::aterm_appl, atermpp::aterm_appl> :: const_iterator it=f_orient.find(a_term); 
      if (it!=f_orient.end())   // found
      {
        return it->second;
      }

      AFun v_symbol;
      atermpp::aterm v_function;
      size_t v_arity;

      v_symbol = ATgetAFun(a_term);
      v_function = a_term(0);
      v_arity = ATgetArity(v_symbol);

      atermpp::aterm v_parts[v_arity + 1];
      v_parts[0] = v_function;
      for (size_t i = 1; i < v_arity; i++)
      {
        v_parts[i] = orient(a_term(i));
      }
      atermpp::aterm_appl v_result = atermpp::aterm_appl(ATmakeApplArray(v_symbol, (ATerm*)v_parts));
      // delete[] v_parts;
      // v_parts = 0;

      if (f_info->is_equality(v_result))
      {
        atermpp::aterm_appl v_term1;
        atermpp::aterm_appl v_term2;

        v_term1 = v_result(1);
        v_term2 = v_result(2);
        if (f_info->compare_term(v_term1, v_term2) == compare_result_bigger)
        {
          v_result = atermpp::aterm_appl(ATmakeAppl3(v_symbol, (ATerm)v_function, (ATerm)(ATermAppl)v_term2, (ATerm)(ATermAppl)v_term1));
        }
      }
      f_orient[a_term]=v_result;

      return v_result;
    }

    /// \brief Initializes the table ATerm_Manipulator::f_set_true and calls the method
    /// \brief AM_Jitty::f_set_true_auxiliary.
    virtual atermpp::aterm_appl set_true(atermpp::aterm_appl a_formula, atermpp::aterm_appl a_guard)
    {
      atermpp::map < atermpp::aterm_appl, atermpp::aterm_appl > f_set_true;
      return set_true_auxiliary(a_formula, a_guard, f_set_true);
    }

    /// \brief Initializes the table ATerm_Manipulator::f_set_false and calls the method
    /// \brief AM_Jitty::f_set_false_auxiliary.
    virtual atermpp::aterm_appl set_false(atermpp::aterm_appl a_formula, atermpp::aterm_appl a_guard)
    {
      atermpp::map < atermpp::aterm_appl, atermpp::aterm_appl > f_set_false;
      return set_false_auxiliary(a_formula, a_guard,f_set_false);
    }
};

}
}
}

#endif
