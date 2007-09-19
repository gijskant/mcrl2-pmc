// Author(s): Jan Friso Groote, Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file bes.h
/// \brief Add your file description here.

#ifndef BES_H
#define BES_H

#include "atermpp/aterm.h"
#include "atermpp/aterm_traits.h"
#include "atermpp/aterm_access.h"
#include "atermpp/aterm_appl.h"
#include "atermpp/aterm_int.h"
#include "atermpp/indexed_set.h"
#include "atermpp/table.h"
#include "mcrl2/pbes/pbes.h"
#include <algorithm>
#include <deque>

#define RELEVANCE_MASK 1
#define FIXPOINT_MASK 2
#define RANK_SHIFT 2

namespace bes 
{
  using atermpp::aterm_int;
  using atermpp::aterm;
  using atermpp::aterm_appl;
  using atermpp::arg1;
  using atermpp::arg2;
  using lps::fixpoint_symbol;


// a bes variable_type is an unsigned long. 

  typedef unsigned long variable_type; /* valid values start at 1 */

  typedef enum reason { UNKNOWN, 
                        MU_CYCLE, 
                        NU_CYCLE,
                        SET_TO_FALSE, 
                        SET_TO_TRUE, 
                        FORWARD_SUBSTITUTION_FALSE,
                        FORWARD_SUBSTITUTION_TRUE,
                        SUBSTITUTION_FALSE,
                        SUBSTITUTION_TRUE,
                        APPROXIMATION_FALSE,
                        APPROXIMATION_TRUE,
                        APPROXIMATION } reason;

  class counter_example
  { 
    private: 
      bes::variable_type v;
      reason r;

    public:
      counter_example()
        : v(0),
          r(UNKNOWN)
      {}

      counter_example(variable_type w, reason s)
        : v(w),
          r(s)
      { 
      }

      variable_type get_variable(void)
      { return v;
      }

      void set_variable(variable_type w)
      { v=w;
      }

      reason get_reason(void)
      { return r;
      }

      void set_reason(reason s)
      { r=s;
      }
      
      std::string print_reason(void)
      { switch (r)
        { case UNKNOWN:                    return "Unknown     ";
          case MU_CYCLE:                   return "Mu Cycle    ";
          case NU_CYCLE:                   return "Nu Cycle    ";
          case SET_TO_FALSE:               return "Set:false   ";
          case SET_TO_TRUE:                return "Set:true    ";
          case FORWARD_SUBSTITUTION_FALSE: return "FSubst:false";
          case FORWARD_SUBSTITUTION_TRUE:  return "FSubst:true ";
          case SUBSTITUTION_FALSE:         return "Subst:false ";
          case SUBSTITUTION_TRUE:          return "Subst:true  ";
          case APPROXIMATION_FALSE:        return "Appr:false  ";
          case APPROXIMATION_TRUE:         return "Appr:true   ";
          case APPROXIMATION:              return "Approxim    ";
          default: return "ERROR UNKNOWN CASE";
        }
      }
  };

  // wrap global variables in a class, to enable multiple inclusion of this header file
  // note, T is only a dummy

  template <class T>
  struct bes_global_variables
  {
    static std::deque<counter_example> COUNTER_EXAMPLE_NULL_QUEUE;
    static std::deque<variable_type> TODO_NULL_QUEUE;
    static bool opt_use_hashtables;
  };

  template <class T>
  bool bes_global_variables<T>::opt_use_hashtables = false;

  template <class T>
  std::deque<counter_example> bes_global_variables<T>::COUNTER_EXAMPLE_NULL_QUEUE;

  template <class T>
  std::deque<variable_type> bes_global_variables<T>::TODO_NULL_QUEUE;


///////////////////////////////////////////////////////////////////////////////
// bes_expression
/// \brief bes expression.
///
// <BESExpr>       ::= 
//                  | BESTrue
//                  | BESFalse
//                  | And(<BESExpr>, <BESExpr>)
//                  | Or(<BESExpr>, <BESExpr>)
//                  | If(<BESExpr>,<BESExpr>,<BESExpr>)
//                  | <variable>
//
// Moreover, there is a dummy bes expression, that is used
// as default expression.


// below we define functions to construct bes_expressions using aterms.
//

  class bes_expression: public aterm
  {
    public:
      bes_expression()
        : aterm()
      {}

      bes_expression(aterm_appl term)
          : aterm(term)
      {}

      // bes_expression(atermpp::term_appl<atermpp::aterm> &term)
      //    : aterm_appl(term)
      // {}

      // allow assignment from aterms
      bes_expression& operator=(const atermpp::aterm &t)
      {
        m_term = t;
        return *this;
      }
  };

  inline
  void use_hashtables(void)
  { bes_global_variables<int>::opt_use_hashtables=true;
  }

/*
  inline bes_expression initBEStrue(bes_expression &BEStrue)
  { BEStrue = ATmakeAppl0(ATmakeAFun("BESTrue", 0, ATfalse));
    ATprotect(reinterpret_cast<ATerm*>(&BEStrue));
    return BEStrue;
  }

  inline bes_expression initBESfalse(bes_expression &BESfalse)
  { BESfalse = ATmakeAppl0(ATmakeAFun("BESFalse", 0, ATfalse));
    ATprotect(reinterpret_cast<ATerm*>(&BESfalse));
    return BESfalse;
  }

  inline bes_expression initBESdummy(bes_expression &BESdummy)
  { BESdummy = ATmakeAppl0(ATmakeAFun("BESDummy", 0, ATfalse));
    ATprotect(reinterpret_cast<ATerm*>(&BESdummy));
    return BESdummy;
  }
*/
  inline AFun initAFunBESAnd(AFun& f)
  { f = ATmakeAFun("BESAnd", 2, ATfalse);
    ATprotectAFun(f);
    return f;
  }

  inline AFun AFunBESAnd()
  { static AFun BESAnd = initAFunBESAnd(BESAnd);
    return BESAnd;
  }

  inline AFun initAFunBESOr(AFun& f)
  { f = ATmakeAFun("BESOr", 2, ATfalse);
    ATprotectAFun(f);
    return f;
  }

  inline AFun AFunBESOr()
  { static AFun BESOr = initAFunBESOr(BESOr);
    return BESOr;
  }

  inline AFun initAFunBESIf(AFun& f)
  { f = ATmakeAFun("BESIf", 3, ATfalse);
    ATprotectAFun(f);
    return f;
  }

  inline AFun AFunBESIf()
  { static AFun BESIf = initAFunBESIf(BESIf);
    return BESIf;
  }

/* This can't be put in a header file
  bes_expression BESfalse;
  inline bes_expression false_()
  { return (BESfalse?BESfalse:BESfalse=initBESfalse(BESfalse));
  }

  bes_expression BEStrue;
  inline bes_expression true_()
  {
    return (BEStrue?BEStrue:BEStrue=initBEStrue(BEStrue));
  }

  bes_expression BESdummy;
  inline bes_expression dummy()
  {
    return (BESdummy?BESdummy:BESdummy=initBESdummy(BESdummy));
  }
*/

  // BESFalse
  inline
  AFun initAFunBESFalse(AFun& f)
  {
    f = ATmakeAFun("BESFalse", 0, ATfalse);
    ATprotectAFun(f);
    return f;
  }
  
  inline
  AFun gsAFunBESFalse()
  {
    static AFun AFunBESFalse = initAFunBESFalse(AFunBESFalse);
    return AFunBESFalse;
  }
  
  inline
  bool gsIsBESFalse(ATermAppl Term)
  {
    return ATgetAFun(Term) == gsAFunBESFalse();
  }
  
  // BESTrue
  inline
  AFun initAFunBESTrue(AFun& f)
  {
    f = ATmakeAFun("BESTrue", 0, ATfalse);
    ATprotectAFun(f);
    return f;
  }
  
  inline
  AFun gsAFunBESTrue()
  {
    static AFun AFunBESTrue = initAFunBESTrue(AFunBESTrue);
    return AFunBESTrue;
  }
  
  inline
  bool gsIsBESTrue(ATermAppl Term)
  {
    return ATgetAFun(Term) == gsAFunBESTrue();
  }
  
  // BESDummy
  inline
  AFun initAFunBESDummy(AFun& f)
  {
    f = ATmakeAFun("BESDummy", 0, ATfalse);
    ATprotectAFun(f);
    return f;
  }
  
  inline
  AFun gsAFunBESDummy()
  {
    static AFun AFunBESDummy = initAFunBESDummy(AFunBESDummy);
    return AFunBESDummy;
  }
  
  inline
  bool gsIsBESDummy(ATermAppl Term)
  {
    return ATgetAFun(Term) == gsAFunBESDummy();
  }

  inline
  ATermAppl gsMakeBESFalse()
  {
    return ATmakeAppl0(gsAFunBESFalse());
  }

  inline
  ATermAppl gsMakeBESTrue()
  {
    return ATmakeAppl0(gsAFunBESTrue());
  }
  
  inline
  ATermAppl gsMakeBESDummy()
  {
    return ATmakeAppl0(gsAFunBESDummy());
  }

  /// \brief Returns the expression true
  inline
  bes_expression true_()
  {
    return bes_expression(gsMakeBESTrue());
  }

  /// \brief Returns the expression false
  inline
  bes_expression false_()
  {
    return bes_expression(gsMakeBESFalse());
  }

  /// \brief Returns the expression dummy (???)
  inline
  bes_expression dummy()
  {
    return bes_expression(gsMakeBESDummy());
  }

  inline bes_expression and_(bes_expression b1,bes_expression b2)
  { return bes_expression(
               ATmakeAppl2(AFunBESAnd(),
                           (aterm)(b1),
                           (aterm)(b2)));
  }

  inline bes_expression or_(bes_expression b1,bes_expression b2)
  { return bes_expression(
               ATmakeAppl2(AFunBESOr(),
                           (aterm)(b1),
                           (aterm)(b2)));
  }

  inline bool is_variable(bes_expression b)
  { return b.type()==AT_INT;
  }

  inline bes_expression if_(bes_expression b1,bes_expression b2,bes_expression b3)
  { 
    return bes_expression(
               ATmakeAppl3(AFunBESIf(),
                           (aterm)(b1),
                           (aterm)(b2),
                           (aterm)(b3)));
  }

  inline bes_expression ifAUX_(bes_expression b1,bes_expression b2,bes_expression b3)
  { if (b2==b3)
    { return b2;
    }
    return if_(b1,b2,b3);
  }

  inline bes_expression variable(variable_type n)
  { return bes_expression((aterm)aterm_int(n));
  }

  inline bool is_false(bes_expression b)
  { 
    return b==false_();
  }

  inline bool is_true(bes_expression b)
  { return b==true_();
  }

  inline bool is_dummy(bes_expression b)
  { return b==dummy();
  }

  inline bool is_and(bes_expression b)
  { return ATgetAFun((_ATerm*)b)==AFunBESAnd();
  }

  inline bool is_or(bes_expression b)
  { return ATgetAFun((_ATerm*)b)==AFunBESOr();
  }

  inline bool is_if(bes_expression b)
  { return ATgetAFun((_ATerm*)b)==AFunBESIf();
  }

  inline bes_expression lhs(bes_expression b)
  { assert(is_and(b) || is_or(b));
    return bes_expression(aterm_appl(b)(0));
  }

  inline bes_expression rhs(bes_expression b)
  { assert(is_and(b) || is_or(b));
    return bes_expression(aterm_appl(b)(1));
  }

  inline bes_expression condition(bes_expression b)
  { 
    assert(is_if(b));
    return bes_expression(aterm_appl(b)(0));
  }

  inline bes_expression then_branch(bes_expression b)
  { assert(is_if(b));
    return bes_expression(aterm_appl(b)(1));
  }

  inline bes_expression else_branch(bes_expression b)
  { 
    assert(is_if(b));
    return bes_expression(aterm_appl(b)(2));
  }

  inline variable_type get_variable(bes_expression b)
  { assert(is_variable(b));
    return ((aterm_int)b).value();
  }

  inline bes_expression substitute_true_false_rec(
                      bes_expression b, 
                      const variable_type v, 
                      const bes_expression b_subst,
                      atermpp::table &hashtable,
                      std::deque < counter_example > &counter_example_queue=bes_global_variables<int>::COUNTER_EXAMPLE_NULL_QUEUE)
  { assert(is_true(b_subst)||is_false(b_subst));

    if (is_true(b)||is_false(b)||is_dummy(b))
    { return b;
    }
    
    bes_expression result;

    if (bes_global_variables<int>::opt_use_hashtables)
    { result=hashtable.get(b);
      if (result!=NULL)
      { return result;
      }
    }
    
    if (is_if(b))
    { if (v==get_variable(condition(b)))
      { 
        if (is_true(b_subst))
        { result=then_branch(b);
          if (&counter_example_queue!=&bes_global_variables<int>::COUNTER_EXAMPLE_NULL_QUEUE)
          { counter_example_queue.push_front(counter_example(v,SUBSTITUTION_TRUE));
          }
        }
        else
        { assert(is_false(b_subst));
          if (&counter_example_queue!=&bes_global_variables<int>::COUNTER_EXAMPLE_NULL_QUEUE)
          { counter_example_queue.push_front(counter_example(v,SUBSTITUTION_FALSE));
          }
          result=else_branch(b);
        }
      }
      else
      { result=ifAUX_(condition(b),
                     substitute_true_false_rec(then_branch(b),v,b_subst,hashtable,counter_example_queue),
                     substitute_true_false_rec(else_branch(b),v,b_subst,hashtable,counter_example_queue));
      }
    }
    else if (is_variable(b))
    { if (v==get_variable(b))
      { result=b_subst;
        if (is_true(b_subst))
        { 
          if (&counter_example_queue!=&bes_global_variables<int>::COUNTER_EXAMPLE_NULL_QUEUE)
          { counter_example_queue.push_front(counter_example(v,SUBSTITUTION_TRUE));
          }
        }
        else
        { assert(is_false(b_subst));
          if (&counter_example_queue!=&bes_global_variables<int>::COUNTER_EXAMPLE_NULL_QUEUE)
          { counter_example_queue.push_front(counter_example(v,SUBSTITUTION_FALSE));
          }
        }
      }
      else
      { result=b;
      }
    }
    else if (is_and(b))
    { 
      bes_expression b1=substitute_true_false_rec(lhs(b),v,b_subst,hashtable,counter_example_queue);
      if (is_false(b1))
      { result=false_();
      }
      else
      { bes_expression b2=substitute_true_false_rec(rhs(b),v,b_subst,hashtable,counter_example_queue);
        if (is_false(b2))
        { result=false_();
        }
        else if (is_true(b1))
        { result=b2;
        }
        else if (is_true(b2))
        { result=b1;
        }
        else
        { result=and_(b1,b2);
        }
      }
    }
    else if (is_or(b))
    { 
      bes_expression b1=substitute_true_false_rec(lhs(b),v,b_subst,hashtable,counter_example_queue);
      if (is_true(b1))
      { result=true_();
      }
      else
      { bes_expression b2=substitute_true_false_rec(rhs(b),v,b_subst,hashtable,counter_example_queue);
        if (is_true(b2))
        { result=true_();
        }
        else if (is_false(b1))
        { result=b2;
        }
        else if (is_false(b2))
        { result=b1;
        }
        else
        { result=or_(b1,b2);
        }
      }
    }

    if (bes_global_variables<int>::opt_use_hashtables)
    { hashtable.put(b,result); 
    }
    return result;
  }

  inline
  bes_expression substitute_true_false(
                      bes_expression b, 
                      const variable_type v, 
                      const bes_expression b_subst,
                      std::deque < counter_example > &counter_example_queue=bes_global_variables<int>::COUNTER_EXAMPLE_NULL_QUEUE)
  { assert(is_true(b_subst)||is_false(b_subst));

    if (is_true(b)||is_false(b)||is_dummy(b))
    { return b;
    }
    
    static atermpp::table hashtable1(10,50);

    bes_expression result=substitute_true_false_rec(b,v,b_subst,hashtable1,counter_example_queue);

    if (bes_global_variables<int>::opt_use_hashtables) 
    { hashtable1.reset();
    }
    return result;
  }

  inline
  bes_expression BDDif_rec(
                             bes_expression b1, 
                             bes_expression b2, 
                             bes_expression b3,
                             atermpp::table &hashtable);


  inline bes_expression BDDif(bes_expression b1, bes_expression b2, bes_expression b3)
  { 
    
    static atermpp::table hashtable(100,75);

    bes_expression b=BDDif_rec(b1,b2,b3,hashtable);
    // hashtable.reset(); This costs a lot of time, currently.
    return b;
  }

  inline bes_expression BDDif_rec(bes_expression b1, bes_expression b2, bes_expression b3,atermpp::table &hashtable)
  { /* Assume that b1, b2 and b3 are ordered BDDs. Return an
       ordered BDD */

    /* Check hash table */
    
    bes_expression b1b2b3;
    bes_expression result;

    if (is_true(b1))
    { return b2;
    }
    else if (is_false(b1))
    { return b3;
    }
    /* b1 has an if_then_else shape */
    else if (b2==b3)
    { return b2;
    }

    if (is_true(b2))
    { /* hence not is_true(b3) */
      if (is_false(b3))
      { return b1;
      }
      else 
      { 
        b1b2b3=if_(b1,b2,b3);
        bes_expression b(hashtable.get(b1b2b3));
        if (b!=NULL)
        { return b;
        }

        /* b3 contains an if then else expression, and b2=true */
        if (condition(b1)==condition(b3))
        { result=ifAUX_(condition(b1),
                        BDDif_rec(then_branch(b1),b2,then_branch(b3),hashtable),
                        BDDif_rec(else_branch(b1),b2,else_branch(b3),hashtable));
        }
        else if (get_variable(condition(b1))<get_variable(condition(b3)))
        { result=ifAUX_(condition(b1),
                        BDDif_rec(then_branch(b1),b2,b3,hashtable),
                        BDDif_rec(else_branch(b1),b2,b3,hashtable));
        }
        else
        { result=ifAUX_(condition(b3),
                        BDDif_rec(b1,b2,then_branch(b3),hashtable),
                        BDDif_rec(b1,b2,else_branch(b3),hashtable));
        }
      }
    }
    else if (is_false(b2))
    { /* hence not is_false(b3) */
      b1b2b3=if_(b1,b2,b3);
      bes_expression b(hashtable.get(b1b2b3));
      if (b!=NULL)
      { return b;
      }

      if (is_true(b3))
      { result=ifAUX_(condition(b1),
                     BDDif_rec(then_branch(b1),b2,b3,hashtable),
                     BDDif_rec(else_branch(b1),b2,b3,hashtable));
      }
      else 
      { /* b3 contains an if then else expression, and b2=true */
        if (condition(b1)==condition(b3))
        { result=ifAUX_(condition(b1),
                        BDDif_rec(then_branch(b1),b2,then_branch(b3),hashtable),
                        BDDif_rec(else_branch(b1),b2,else_branch(b3),hashtable));
        }
        else if (get_variable(condition(b1))<get_variable(condition(b3)))
        { result=ifAUX_(condition(b1),
                        BDDif_rec(then_branch(b1),b2,b3,hashtable),
                        BDDif_rec(else_branch(b1),b2,b3,hashtable));
        }
        else
        { result=ifAUX_(condition(b3),
                        BDDif_rec(b1,b2,then_branch(b3),hashtable),
                        BDDif_rec(b1,b2,else_branch(b3),hashtable));
        }
      }
    }
    else if (is_true(b3)||is_false(b3))
    { 
      if (condition(b1)==condition(b2))
      { result=ifAUX_(condition(b1),
                      BDDif_rec(then_branch(b1),then_branch(b2),b3,hashtable),
                      BDDif_rec(else_branch(b1),else_branch(b2),b3,hashtable));
      }
      else if (get_variable(condition(b1))<get_variable(condition(b2)))
      { result=ifAUX_(condition(b1),
                      BDDif_rec(then_branch(b1),b2,b3,hashtable),
                      BDDif_rec(else_branch(b1),b2,b3,hashtable));
      }
      else
      { result=ifAUX_(condition(b2),
                      BDDif_rec(b1,then_branch(b2),b3,hashtable),
                      BDDif_rec(b1,else_branch(b2),b3,hashtable));
      }
    }
    else
    { /* None of b1, b2 and b3 is true or false, all have the if then else shape */
      if (condition(b1)==condition(b2))
      { 
        if (condition(b2)==condition(b3))
        { /* all conditions are equal */ 
          result=ifAUX_(condition(b1),
                        BDDif_rec(then_branch(b1),then_branch(b2),then_branch(b3),hashtable),
                        BDDif_rec(else_branch(b1),else_branch(b2),else_branch(b3),hashtable));
        }
        else if (get_variable(condition(b1))<get_variable(condition(b3)))
        { 
          result=ifAUX_(condition(b1),
                        BDDif_rec(then_branch(b1),then_branch(b2),b3,hashtable),
                        BDDif_rec(else_branch(b1),else_branch(b2),b3,hashtable));
        }
        else 
        { 
          result=ifAUX_(condition(b3),
                        BDDif_rec(b1,b2,then_branch(b3),hashtable),
                        BDDif_rec(b1,b2,else_branch(b3),hashtable));
        }
      }
      else if (get_variable(condition(b1))<get_variable(condition(b2)))
      { 
        if (condition(b1)==condition(b3))
        { 
          result=ifAUX_(condition(b1),
                        BDDif_rec(then_branch(b1),b2,then_branch(b3),hashtable),
                        BDDif_rec(else_branch(b1),b2,else_branch(b3),hashtable));

        }
        else if (get_variable(condition(b1))<get_variable(condition(b3)))
        {
          result=ifAUX_(condition(b1),
                        BDDif_rec(then_branch(b1),b2,b3,hashtable),
                        BDDif_rec(else_branch(b1),b2,b3,hashtable));
        }
        else if (condition(b2)==condition(b3))
        {
          result=ifAUX_(condition(b2),
                        BDDif_rec(b1,then_branch(b2),then_branch(b3),hashtable),
                        BDDif_rec(b1,else_branch(b2),else_branch(b3),hashtable));
        }
        else if (get_variable(condition(b2))<get_variable(condition(b3)))
        {
          result=ifAUX_(condition(b2),
                        BDDif_rec(b1,then_branch(b2),b3,hashtable),
                        BDDif_rec(b1,else_branch(b2),b3,hashtable));
        }
        else 
        {
          result=ifAUX_(condition(b3),
                        BDDif_rec(b1,b2,then_branch(b3),hashtable),
                        BDDif_rec(b1,b2,else_branch(b3),hashtable));
        }
      }
    }

    /* Add (if_(b1,b2,b3),result) to hashtable. */

    hashtable.put(b1b2b3,result);
    return result;
  }



  class equations
  { 
    private:
      // initial size is the initial size of 
      // the vector holding equations;
      // the first position of the vectors (i.e. position 0)
      // is not used.
      //
      // The first bit it is indicated whether this
      // equation is relevant. It is 0 if not relevant.
      // It is 1 if relevant.
      // In the second bit it is indicated whether
      // the equation is nu (0) or a mu (1), the rest of the
      // bits indicate the rank+1 of this equation.
      // The value 0 for control_info indicates a wrong value.
      
      std::vector<unsigned long> control_info;
      atermpp::vector<bes_expression> right_hand_sides;
      bool variable_occurrences_are_stored;
      std::vector< std::set <bes::variable_type> > variable_occurrence_sets;
      atermpp::indexed_set variable_relevance_indexed_set;
      bool count_variable_relevance;
      std::vector < std::deque < counter_example> > data_to_construct_counter_example;
      bool construct_counter_example;

    protected:
      inline void check_vector_sizes(variable_type v)
      { if (v>nr_of_variables())
        { control_info.resize(v+1,0);
          right_hand_sides.resize(v+1,dummy());
          if (variable_occurrences_are_stored)
          { variable_occurrence_sets.resize(v+1,std::set<variable_type>());
          }
         if (construct_counter_example)
          { data_to_construct_counter_example.resize(v+1,std::deque<counter_example>());
          }
        }
      }

    public:
      unsigned long max_rank;

      equations()
        : control_info(1),
          right_hand_sides(1),
          variable_occurrences_are_stored(false),
          variable_occurrence_sets(1),
          variable_relevance_indexed_set(10,50),
          count_variable_relevance(false),
          data_to_construct_counter_example(1),
          construct_counter_example(false),
          max_rank(0)
      {}

      inline unsigned long nr_of_variables()
      { return control_info.size()-1; /* there is no equation at position 0 */
      }

      inline fixpoint_symbol get_fixpoint_symbol(variable_type v)
      {
        assert(v>0); /* variable indices start at 1. 0 is used as an indicator of error */
        assert(v<=nr_of_variables());
        assert(control_info[v]>0);

        return (((control_info[v] & FIXPOINT_MASK) ==0) ? fixpoint_symbol::nu() : fixpoint_symbol::mu());
      }


      inline unsigned long get_rank(variable_type v)
      {  
        assert(v>0);
        check_vector_sizes(v);
  
        return (control_info[v] >> RANK_SHIFT)-1;
      }

      void add_equation(variable_type v, 
                        fixpoint_symbol sigma,
                        unsigned long rank,
                        bes_expression rhs,
                        std::deque <variable_type> &todo=bes_global_variables<int>::TODO_NULL_QUEUE)
      { assert(rank>0);  // rank must be positive.
        assert(v>0);     // variables are represented by numbers >0.
        // std::cerr << "Add equation " << v << std::endl;

        check_vector_sizes(v);
        // the vector at position v is now guaranteed to exist.
  
        // if the control info is 0, the value at variable
        // is not initialized.

        control_info[v]=1+(((sigma.is_nu()?0:1)+((rank+1)<<1))<<1);
        right_hand_sides[v]=rhs;
        if (rank>max_rank)
        { max_rank=rank;
        }
        if (variable_occurrences_are_stored)
        { add_variables_to_occurrence_sets(v,rhs);
        }
        if (count_variable_relevance)
        { set_variable_relevance_rec(rhs,todo);
        }
      }

      inline void set_rhs(variable_type v,
                          bes_expression b,
                          variable_type v_except=0,
                          std::deque <variable_type> &todo=bes_global_variables<int>::TODO_NULL_QUEUE)
      { /* set the right hand side of v to b. Update the variable occurrences
           of v in the variables occurrence sets of variables occurring in b, but
           do not update the variable occurrence sets of v_except */
        assert(v>0);
        assert(v<=nr_of_variables());

        control_info[v]=control_info[v]|RELEVANCE_MASK;  // make this variable relevant.
        bes_expression old_rhs=right_hand_sides[v];
        if ((variable_occurrences_are_stored)&& (old_rhs!=b))
        { remove_variables_from_occurrence_sets(v,old_rhs,v_except);
          add_variables_to_occurrence_sets(v,b);
          right_hand_sides[v]=b;
        }
        if (count_variable_relevance)
        { set_variable_relevance_rec(b,todo);
        }

      }

      inline bes_expression get_rhs(variable_type v)
      { 
        if (v>nr_of_variables())
        { 
          // fprintf(stderr,"ACCESSING A VARIABLE THAT DOES NOT EXIST\n");
          return dummy();
        }
        else 
        {
          return right_hand_sides[v];
        }
      }

      void clear_variable_occurrence_set(variable_type v)
      { assert(variable_occurrences_are_stored);
        check_vector_sizes(v);
        variable_occurrence_sets[v].clear();
      }

      std::set< bes::variable_type >::iterator 
            variable_occurrence_set_begin(variable_type v)
      { assert(variable_occurrences_are_stored);
        assert(v>0);
        check_vector_sizes(v);
        return variable_occurrence_sets[v].begin();
      }

      std::set< bes::variable_type >::iterator 
            variable_occurrence_set_end(variable_type v)
      { assert(variable_occurrences_are_stored);
        assert(v>0);
        check_vector_sizes(v);
        return variable_occurrence_sets[v].end();
      }

      void add_variables_to_occurrence_sets(
                      bes::variable_type v,
                      bes_expression b,
                      const bool use_indexed_set,
                      atermpp::indexed_set &indexed_set)
      { 
        assert(v>0);
        assert(variable_occurrences_are_stored);

        if (bes::is_true(b)||bes::is_false(b)||bes::is_dummy(b))
        { return;
        }
    
        if (use_indexed_set)
        { if (!(indexed_set.put(b)).second)  // b is already in the set.
          return;
        }

        if (is_if(b)) 
        { assert(get_variable(condition(b))>0);
          // std::cerr << "ADD " << v << " TO SET " << get_variable(condition(b)) << std::endl;
          variable_type w=get_variable(condition(b));
          check_vector_sizes(w);
          variable_occurrence_sets[w].insert(v);

          add_variables_to_occurrence_sets(v,then_branch(b),use_indexed_set,indexed_set);
          add_variables_to_occurrence_sets(v,else_branch(b),use_indexed_set,indexed_set);
          return;
        }

        if (is_variable(b))
        { variable_type w=get_variable(b);
          check_vector_sizes(w);
          variable_occurrence_sets[w].insert(v);
          return;
        }

        if (is_and(b)||is_or(b))
        { 
          add_variables_to_occurrence_sets(v,lhs(b),use_indexed_set,indexed_set);
          add_variables_to_occurrence_sets(v,rhs(b),use_indexed_set,indexed_set);
          return;
        }

        assert(0); // do not expect other term formats.

      }

      void add_variables_to_occurrence_sets(
                      bes::variable_type v,
                      bes_expression b)
      { static atermpp::indexed_set indexed_set1(10,50);

        add_variables_to_occurrence_sets(v,b,bes_global_variables<int>::opt_use_hashtables,indexed_set1);
        if (bes_global_variables<int>::opt_use_hashtables)
        { indexed_set1.reset();
        }
      }

      void remove_variables_from_occurrence_sets(
                     const bes::variable_type v,
                     bes_expression b,
                     const bes::variable_type v_except,
                     const bool use_indexed_set,
                     atermpp::indexed_set &indexed_set)
      { 
        assert(v>0);
        assert(variable_occurrences_are_stored);
        check_vector_sizes(v);
        if (bes::is_true(b)||bes::is_false(b)||bes::is_dummy(b))
        { return;
        }

        if (use_indexed_set)
        { if (!(indexed_set.put(b)).second)  // b is already in the set.
          return;
        }

        if (is_if(b))
        { bes::variable_type w=get_variable(condition(b));
          assert(w<=nr_of_variables());
          if (w!=v_except)
          { variable_occurrence_sets[w].erase(v);
          }
          // Using hash tables this can be made more efficient, by employing
          // sharing of the aterm representing b.
          remove_variables_from_occurrence_sets(v,then_branch(b),v_except,use_indexed_set,indexed_set);
          remove_variables_from_occurrence_sets(v,else_branch(b),v_except,use_indexed_set,indexed_set);
          return;
        }
        else if (is_variable(b))
        { bes::variable_type w=get_variable(b);
          assert(w<=nr_of_variables());
          if (w!=v_except)
          { variable_occurrence_sets[w].erase(v);
          }
          return;
        }
        else if (is_or(b)||is_and(b))
        {
          remove_variables_from_occurrence_sets(v,lhs(b),v_except,use_indexed_set,indexed_set);
          remove_variables_from_occurrence_sets(v,rhs(b),v_except,use_indexed_set,indexed_set);
          return;
        }
      }

      void remove_variables_from_occurrence_sets(
                     const bes::variable_type v,
                     bes_expression b,
                     const bes::variable_type v_except)
      {
        static atermpp::indexed_set indexed_set2(10,50);

        remove_variables_from_occurrence_sets(v,b,v_except,bes_global_variables<int>::opt_use_hashtables,indexed_set2);
        if (bes_global_variables<int>::opt_use_hashtables)
        { indexed_set2.reset();
        }
      }


      void store_variable_occurrences(void)
      {
        if (variable_occurrences_are_stored)
        { return;
        }
        assert(variable_occurrence_sets.size()==1);
        variable_occurrences_are_stored=true;
        variable_occurrence_sets.resize(nr_of_variables()+1,std::set<variable_type>());
        for(bes::variable_type v=nr_of_variables(); v>0; v--)
        { 
          bes_expression b=get_rhs(v);
          if (b!=dummy())
          { add_variables_to_occurrence_sets(v,get_rhs(v));
          }
        }
      }


      void reset_variable_relevance(void)
      {
        for(std::vector <unsigned long>::iterator v=control_info.begin() ;
                 v!=control_info.end() ;
                 v++)
        { *v =(*v)& ~RELEVANCE_MASK;  // reset the relevance bit to 0
        }
      }
  

      void set_variable_relevance_rec(
                    bes_expression b,
                    std::deque <variable_type> &todo=bes_global_variables<int>::TODO_NULL_QUEUE)
      { 
        assert(count_variable_relevance);
        if (is_true(b)||is_false(b)||is_dummy(b))
        { return;
        }

        if (bes_global_variables<int>::opt_use_hashtables)
        { if (!(variable_relevance_indexed_set.put(b)).second)
          { /* The relevance for the variables in this term has already been set */
            return;
          }
        }

        if (is_variable(b))
        { variable_type v=get_variable(b);
          assert(v>0);
          check_vector_sizes(v);
          if (!is_relevant(v))
          { control_info[v]=control_info[v]|RELEVANCE_MASK;  // Make relevant
            if (get_rhs(v)==dummy()) // v is relevant an unprocessed. Put in on the todo stack.
            { 
              if (&todo!=&bes_global_variables<int>::TODO_NULL_QUEUE)
              { 
                todo.push_back(v);
              }
              return;
            }
            else
            { set_variable_relevance_rec(get_rhs(v),todo);  
              return;
            }
          }
          return;
        }
        
        if (is_if(b))
        { set_variable_relevance_rec(condition(b),todo);
          set_variable_relevance_rec(then_branch(b),todo);
          set_variable_relevance_rec(else_branch(b),todo);
          return;
        }  

        if (is_and(b)||is_or(b))
        { set_variable_relevance_rec(lhs(b),todo);
          set_variable_relevance_rec(rhs(b),todo);
          return;
        }

        assert(0); // do not expect other term formats.
      }

      void refresh_relevances(std::deque <variable_type> &todo=bes_global_variables<int>::TODO_NULL_QUEUE)
      { if (count_variable_relevance)
        { reset_variable_relevance();
          if (bes_global_variables<int>::opt_use_hashtables)
          { variable_relevance_indexed_set.reset();
          }
          if (&todo!=&bes_global_variables<int>::TODO_NULL_QUEUE)
          { todo.clear();
          }
          set_variable_relevance_rec(variable(1),bes_global_variables<int>::TODO_NULL_QUEUE);
          if (&todo!=&bes_global_variables<int>::TODO_NULL_QUEUE)
          { // We add the variables to the todo queue separately,
            // to guarantee that lower numbered variables occur earlier in 
            // the queue. This guarantees shorter counter examples.
            for(unsigned long v=1; v<=nr_of_variables(); v++)
            { if ((get_rhs(v)==dummy()) && is_relevant(v))
              { todo.push_back(v);
              }
            }
          }
        }
      }

      void count_variable_relevance_on(void)
      {
        assert(!count_variable_relevance);
        count_variable_relevance=true;
        refresh_relevances();
      }

      bool is_relevant(variable_type v)
      { 
        assert(0<v);
        check_vector_sizes(v);
        /* all variables are relevant if relevancy is not maintained */
        return (!count_variable_relevance || (control_info[v] & RELEVANCE_MASK));
      }

      void construct_counter_example_on(void)
      {
        assert(!construct_counter_example);
        construct_counter_example=true;
        data_to_construct_counter_example.resize(nr_of_variables()+1,std::deque<counter_example>());
      }

      std::deque <counter_example>  
                   &counter_example_queue(variable_type v)
      { assert(construct_counter_example);
        check_vector_sizes(v);
        return data_to_construct_counter_example[v];
      }

      std::deque<counter_example>::iterator dummy_deque_iterator;

      std::deque<counter_example>::iterator counter_example_begin(variable_type v)
      { assert(construct_counter_example);
        if (v>nr_of_variables())
        { return dummy_deque_iterator;
        }
        return data_to_construct_counter_example[v].begin();
      }

      std::deque<counter_example>::iterator counter_example_end(variable_type v)
      { assert(construct_counter_example);
        if (v>nr_of_variables())
        { return dummy_deque_iterator;
        }
        return data_to_construct_counter_example[v].end();
      }

      bool find_nu_loop(
             bes_expression b,
             variable_type v,
             unsigned long current_rank,
             std::deque < counter_example > &counter_example_queue=bes_global_variables<int>::COUNTER_EXAMPLE_NULL_QUEUE)
      { if (is_false(b) || is_true(b) || is_dummy(b))
        { return false;
        }

        if (is_if(b))
        { variable_type w=get_variable(condition(b));
          if ((get_rank(w)!=current_rank)||(w>v))
          { return false;
          }
          if (w==v)
          { if ( construct_counter_example)
            { counter_example_queue.push_front(counter_example(w,NU_CYCLE));
            }
            if (is_true(then_branch(b)))
            { return true;
            }
            return false;
          }
          if (is_true(then_branch(b)))
          { return find_nu_loop(else_branch(b),v,current_rank,counter_example_queue);
          }
          if (is_true(else_branch(b)))
          { return find_nu_loop(then_branch(b),v,current_rank,counter_example_queue);
          }
          return false;
        }

        if (is_variable(b))
        { variable_type w=get_variable(b);
          if (w==v)
          { if ( construct_counter_example)
            { counter_example_queue.push_front(counter_example(w,NU_CYCLE));
            }
            return true;
          }
          if ((get_rank(w)!=current_rank)||(w>v))
          { return false;
          }
          return find_nu_loop(get_rhs(w),v,current_rank,counter_example_queue);
        }

        if (is_and(b))
        { return false;
        }
        
        assert(is_or(b));
        return find_nu_loop(lhs(b),v,current_rank,counter_example_queue) || find_nu_loop(rhs(b),v,current_rank,counter_example_queue);
      }

      bool find_mu_loop(
             bes_expression b,
             variable_type v,
             unsigned long current_rank,
             std::deque < counter_example > &counter_example_queue=bes_global_variables<int>::COUNTER_EXAMPLE_NULL_QUEUE)
      { 
        if (is_false(b) || is_true(b) || is_dummy(b))
        { return false;
        }

        if (is_if(b))
        { variable_type w=get_variable(condition(b));
          if ((get_rank(w)!=current_rank)||(w>v))
          { return false;
          }
          if (w==v)
          { 
            if ( construct_counter_example)
            { counter_example_queue.push_front(counter_example(w,MU_CYCLE));
            }

            if (is_false(else_branch(b)))
            { return true;
            }
            return false;
          }
          if (is_false(then_branch(b)))
          { return find_mu_loop(else_branch(b),v,current_rank,counter_example_queue);
          }
          if (is_false(else_branch(b)))
          { return find_mu_loop(then_branch(b),v,current_rank,counter_example_queue);
          }
          return false;
        }

        if (is_variable(b))
        { variable_type w=get_variable(b);
          if (w==v)
          { if ( construct_counter_example)
            { counter_example_queue.push_front(counter_example(w,MU_CYCLE));
            }
            return true;
          }
          if ((get_rank(w)!=current_rank)||(w>v))
          { return false;
          }
          return find_mu_loop(get_rhs(w),v,current_rank,counter_example_queue);
        }

        if (is_and(b))
        { return find_mu_loop(lhs(b),v,current_rank,counter_example_queue) || find_mu_loop(rhs(b),v,current_rank,counter_example_queue);
        }

        assert(is_or(b));
        return false;
      }

  };


} // namespace bes.

namespace atermpp
{
  template<>
    struct aterm_traits<bes::bes_expression>
    {
      typedef ATermAppl aterm_type;
      static void protect(bes::bes_expression t)   { t.protect(); }
      static void unprotect(bes::bes_expression t) { t.unprotect(); }
      static void mark(bes::bes_expression t)      { t.mark(); }
      static ATerm term(bes::bes_expression t)     { return t.term(); }
      static ATerm* ptr(bes::bes_expression& t)    { return &t.term(); }
    };
} // namespace atermpp

#endif
