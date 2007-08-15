// Author(s): Aad Mathijssen, Jeroen Keiren
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file libstruct_ir.cpp

#include <aterm2.h>

#include "libstruct.h"
#include "libstruct_ir.h"
#include "print/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"

using namespace ::mcrl2::utilities;

static const char* struct_prefix = "Struct@";
static const char* list_prefix   = "List@";
static const char* set_prefix    = "Set@";
static const char* bag_prefix    = "Bag@";
static const char* lambda_prefix = "lambda@";

static void get_free_vars_appl(ATermAppl data_expr, ATermList bound_vars,
  ATermList* p_free_vars);
//Pre: data_expr is a data expression or a bag enumeration element that adheres
//     to the internal format after type checking
//     bound_vars and *p_free_vars are lists of data variables, and represent the
//     bound/free variables of the context of data_expr
//Post:*p_free_vars is extended with the free variables in data_expr that did not
//     already occur in *p_free_vars or bound_vars

static void get_free_vars_list(ATermList data_exprs, ATermList bound_vars,
  ATermList* p_free_vars);
//Pre: data_exprs is a list of data expressions or bag enumeration elements that
//     adhere to the internal format after type checking
//     bound_vars and *p_free_vars are lists of data variables, and represent the
//     bound/free variables of the context of data_exprs
//Post:*p_free_vars is extended with the free variables in data_exprs that did not
//     already occur in *p_free_vars or bound_vars


// implementation
// -------------------------------------------------------------

bool is_struct_sort_id(ATermAppl sort_expr)
{
  if (gsIsSortId(sort_expr)) {
    return strncmp(
      struct_prefix,
      ATgetName(ATgetAFun(ATAgetArgument(sort_expr, 0))),
      strlen(struct_prefix)) == 0;
  } else {
    return false;
  }
}

bool is_list_sort_id(ATermAppl sort_expr)
{
  if (gsIsSortId(sort_expr)) {
    return strncmp(
      list_prefix,
      ATgetName(ATgetAFun(ATAgetArgument(sort_expr, 0))),
      strlen(list_prefix)) == 0;
  } else {
    return false;
  }
}

bool is_set_sort_id(ATermAppl sort_expr)
{
  if (gsIsSortId(sort_expr)) {
    return strncmp(
      set_prefix,
      ATgetName(ATgetAFun(ATAgetArgument(sort_expr, 0))),
      strlen(set_prefix)) == 0;
  } else {
    return false;
  }
}

bool is_bag_sort_id(ATermAppl sort_expr)
{
  if (gsIsSortId(sort_expr)) {
    return strncmp(
      bag_prefix,
      ATgetName(ATgetAFun(ATAgetArgument(sort_expr, 0))),
      strlen(bag_prefix)) == 0;
  } else {
    return false;
  }
}

bool is_lambda_op_id(ATermAppl data_expr)
{
  if (gsIsOpId(data_expr)) {
    return strncmp(
      lambda_prefix,
      ATgetName(ATgetAFun(ATAgetArgument(data_expr, 0))),
      strlen(lambda_prefix)) == 0;
  } else {
    return false;
  }
}

ATermList get_free_vars(ATermAppl data_expr)
{
  ATermList result = ATmakeList0();
  get_free_vars_appl(data_expr, ATmakeList0(), &result);
  return ATreverse(result);
}

void get_free_vars_appl(ATermAppl data_expr, ATermList bound_vars,
  ATermList* p_free_vars)
{
  if (gsIsDataVarId(data_expr)) {
    //data_expr is a data variable; add it to *p_free_vars if it does not occur in
    //bound_vars or *p_free_vars
    if ((ATindexOf(bound_vars, (ATerm) data_expr, 0) == -1) &&
        (ATindexOf(*p_free_vars, (ATerm) data_expr, 0) == -1)) {
      *p_free_vars = ATinsert(*p_free_vars, (ATerm) data_expr);
    }
  } else if (gsIsOpId(data_expr)) {
    //data_expr is an operation identifier or a number; do nothing
  } else if (gsIsDataAppl(data_expr)) {
    //data_expr is a product data application; get free variables from the
    //arguments
    get_free_vars_appl(ATAgetArgument(data_expr, 0), bound_vars, p_free_vars);
    get_free_vars_list(ATLgetArgument(data_expr, 1), bound_vars, p_free_vars);
  } else if (gsIsBinder(data_expr)) {
    ATermAppl binding_operator = ATAgetArgument(data_expr, 0);
    if (gsIsSetBagComp(binding_operator) || gsIsSetComp(binding_operator)
        || gsIsBagComp(binding_operator)) {
      //data_expr is a set or bag comprehension; get free variables from the body
      //where bound_vars is extended with the variable declaration
      ATermList vars = ATLgetArgument(data_expr, 1);
      ATermAppl var = ATAgetFirst(vars);
      if (ATindexOf(bound_vars, (ATerm) var, 0) == -1) {
        bound_vars = ATinsert(bound_vars, (ATerm) var);
      }
      get_free_vars_appl(ATAgetArgument(data_expr, 2), bound_vars, p_free_vars);
    } else if (gsIsLambda(binding_operator) || gsIsForall(binding_operator) ||
      gsIsExists(binding_operator)) {
      //data_expr is a lambda abstraction or a quantification; get free variables
      //from the body where bound_vars is extended with the variable declaration
      ATermList vars = ATLgetArgument(data_expr, 1);
      while (!ATisEmpty(vars)) {
        ATermAppl var = ATAgetFirst(vars);
        if (ATindexOf(bound_vars, (ATerm) var, 0) == -1) {
          bound_vars = ATinsert(bound_vars, (ATerm) var);
        }
        vars = ATgetNext(vars);
      }
      get_free_vars_appl(ATAgetArgument(data_expr, 2), bound_vars, p_free_vars);
    }
  } else if (gsIsWhr(data_expr)) {
    //data_expr is a where clause; get free variables from the rhs's of the
    //where clause declarations and from the body where bound_vars is extended
    //with the lhs's of the where clause declarations
    ATermList whr_decls = ATLgetArgument(data_expr, 1);
    //get free variables from the rhs's of the where clause declarations
    while (!ATisEmpty(whr_decls)) {
      get_free_vars_appl(ATAgetArgument(ATAgetFirst(whr_decls), 1),
        bound_vars, p_free_vars);
      whr_decls = ATgetNext(whr_decls);
    }
    //get free variables from the body
    whr_decls = ATLgetArgument(data_expr, 1);
    while (!ATisEmpty(whr_decls)) {
      ATermAppl whr_decl = ATAgetFirst(whr_decls);
      ATermAppl var = ATAgetArgument(whr_decl, 0);
      if (ATindexOf(bound_vars, (ATerm) var, 0) == -1) {
        bound_vars = ATinsert(bound_vars, (ATerm) var);
      }
      whr_decls = ATgetNext(whr_decls);
    }
    get_free_vars_appl(ATAgetArgument(data_expr, 0), bound_vars, p_free_vars);
  } else {
    gsErrorMsg("%P is not a data expression or a bag enumeration element\n",\
      data_expr);
  }
}

void get_free_vars_list(ATermList data_exprs, ATermList bound_vars,
  ATermList *p_free_vars)
{
  while (!ATisEmpty(data_exprs))
  {
    get_free_vars_appl(ATAgetFirst(data_exprs), bound_vars, p_free_vars);
    data_exprs = ATgetNext(data_exprs);
  }
}

