// Author(s): Aad Mathijssen
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file dataimpl.cpp

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include "mcrl2/core/detail/data_implementation.h"
#include "mcrl2/core/detail/data_common.h"
#include "mcrl2/core/struct.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"

using namespace ::mcrl2::utilities;
namespace mcrl2 {
  namespace core {
   namespace detail {
//local declarations
//------------------


static ATermAppl impl_sort_refs(ATermAppl spec);
//Pre: spec is a specification that adheres to the internal syntax after
//     data implementation, with the exception that sort references may occur
//Ret: spec in which all sort references are implemented, i.e.:
//     - all sort references are removed from spec
//     - if a sort reference is of the form sort_ref(n, e), where e is the
//       implementation of a type constructor and sort_ref(n, e) is the first
//       sort reference with e as a rhs, e is replaced by n in spec;
//       otherwise, n is replaced by e in spec

static ATermAppl impl_standard_functions_spec(ATermAppl spec);
//Pre: spec is a specification that adheres to the internal syntax after
//     data implementation
//Ret: spec in which an implementation for equality, inequality and if
//     is added for each sort occurring in spec.

static ATermAppl impl_numerical_pattern_matching(ATermAppl spec);
//Pre: spec is a specification that adheres to the internal syntax after
//     data implementation
//Ret: spec in which numerical patterns can be matched; this means that in the
//     left-hand-sides of equations the following patterns are implemented:
//     - Pos2Nat(p) : replace by cNat(p)
//     - Pos2Int(p) : replace by cInt(cNat(p))
//     - Pos2Real(p): replace by cReal(cInt(cNat(p)))
//     - Nat2Int(n) : replace by cInt(n)
//     - Nat2Real(n): replace by cReal(cInt(n))
//     - Int2Real(x): replace by cReal(x)
//     TODO:
//     - p+k, where k is a constant of sort Pos: replace by p,
//       add condition p>=k, and add substitution [p -> Int2Pos(p-k)]
//       (for the condition and the rhs)
//     - n+k, where k is a constant of sort Nat: replace by n,
//       add condition n>=k, and add substitution [n -> Int2Nat(n-k)]
//     - -p: replace by cNeg(p)
//     - -n: replace by the following two patterns:
//           c0,      and add substitution [n -> c0]
//           cNeg(p), and add substitution [p -> cNat(p)]

static ATermAppl impl_numerical_pattern_matching_expr(ATermAppl data_expr,
  bool top_level);
//Pre: data_expr is a data expression that adheres to the internal syntax after
//     data implementation
//     top_level indicates if data_expr contains the top-level operation
//Ret: data_expr in which numerical patterns can be matched if they do not occur
//     at top level

static ATermAppl impl_exprs_with_spec(ATermAppl part, ATermAppl& spec);
//Pre: part is an expression that adheres to the internal syntax after type
//     checking.
//     spec represents an LPS specification in the internal syntax after type
//     checking.
//Post: The datatypes of spec are implemented as higher-order abstract data
//     types in spec
//Ret: if the data implementation went well, an equivalent version of part
//     is returned that adheres to the internal ATerm structure after data
//     implementation.
//     If something went wrong, an appropriate error message is printed and
//     NULL is returned.


static ATermAppl impl_exprs_appl(ATermAppl part, ATermList *p_substs,
  t_data_decls *p_data_decls);
//Pre: part is a part of a specification that adheres to the internal syntax
//     after type checking
//     p_substs is a pointer to a list of substitutions induced by the context
//     of part
//     p_data_decls represents a pointer to new data declarations, induced by
//     the context of part
//Ret: part in which:
//     - all substitutions of *p_substs are performed on the elements of part
//     - each substituted element is implemented, where the new data
//       declarations are stored in *p_data_decls

static ATermList impl_exprs_list(ATermList parts, ATermList *p_substs,
  t_data_decls *p_data_decls);
//Pre: parts consists of parts of a specification that adheres to the internal
//     syntax after type checking
//     p_substs is a pointer to a list of substitutions induced by the context
//     of parts
//     p_data_decls represents a pointer to new data declarations, induced by
//     the context of part
//Ret: parts in which:
//     - all substitutions of *p_substs are performed on the elements of parts
//     - each substituted element is implemented, where the new data
//       declarations are stored in *p_data_decls

static ATermAppl impl_set_enum(ATermList elts, ATermAppl sort_expr);
//Pre: elts is a list containing 1 or more data expressions, all of the same
//     sort
//     sort_expr is a sort expression
//Ret: Implementation of the set enumeration of the elements in elts with
//     result sort sort_expr

static ATermAppl impl_bag_enum(ATermList elts, ATermAppl sort_expr);
//Pre: elts is a list containing 1 or more data expressions, all of the same
//     sort
//     sort_expr is a sort expression
//Ret: Implementation of the bag enumeration of the elements in elts with
//     result sort sort_expr

static ATermAppl impl_sort_struct(ATermAppl sort_struct, ATermList *p_substs,
  t_data_decls *p_data_decls);
//Pre: sort_struct is a structured sort
//     p_substs is a pointer to a list of substitutions induced by the context
//     of sort_struct
//     p_data_decls represents a pointer to new data declarations, induced by
//     the context of sort_struct
//Post:an implementation of sort_struct is added to *p_data_decls and new induced
//     substitutions are added *p_substs
//Ret: a sort identifier which is the implementation of sort_struct

static void split_sort_decls(ATermList sort_decls, ATermList *p_sort_ids,
  ATermList *p_sort_refs);
//Pre: sort_decls is a list of sort_id's and sort_ref's
//Post:*p_sort_ids and *p_sort_refs contain the sort_id's and sort_ref's from
//     sort_decls, in the same order

static ATermAppl make_fresh_struct_sort_id(ATerm term);
//Pre: term is not NULL
//Ret: sort identifier for the implementation of a structured sort with prefix
//     struct_prefix, that does not occur in term

static ATermAppl make_fresh_list_sort_id(ATerm term);
//Pre: term is not NULL
//Ret: fresh sort identifier for the implementation of a list sort with prefix
//     list_prefix, that does not occur in term

static ATermAppl make_fresh_set_sort_id(ATerm term);
//Pre: term is not NULL
//Ret: fresh sort identifier for the implementation of a set sort with prefix
//     set_prefix, that does not occur in term

static ATermAppl make_fresh_bag_sort_id(ATerm term);
//Pre: term is not NULL
//Ret: fresh sort identifier for the implementation of a bag sort with prefix
//     bag_prefix, that does not occur in term

static ATermAppl make_fresh_lambda_op_id(ATermAppl sort_expr, ATerm term);
//Pre: sort_expr is a sort expression
//     term is not NULL
//Ret: operation identifier op_id(n, s) for the implementation of a lambda
//     abstraction, where s is sort_expr and n is a name with prefix
//     lambda_prefix, that does not occur in term

//implementation
//--------------


ATermAppl implement_data_proc_spec(ATermAppl spec)
{
  assert(gsIsSpecV1(spec));
  int occ =
    gsCount((ATerm) gsMakeSortUnknown(), (ATerm) spec) +
    gsCountAFun(gsAFunSortsPossible(), (ATerm) spec);
  if (occ > 0) {
    gsErrorMsg("specification contains %d unresolved type%s\n", occ, (occ != 1)?"s":"");
    return NULL;
  }
  //implement system sorts and data expressions occurring in spec
  ATermList substs     = ATmakeList0();
  t_data_decls data_decls;
  initialize_data_decls(&data_decls);
  spec = impl_exprs_appl(spec, &substs, &data_decls);
  //perform substitutions on data declarations
  data_decls.sorts     = gsSubstValues_List(substs, data_decls.sorts,     true);
  data_decls.cons_ops  = gsSubstValues_List(substs, data_decls.cons_ops,  true);
  data_decls.ops       = gsSubstValues_List(substs, data_decls.ops,       true);
  data_decls.data_eqns = gsSubstValues_List(substs, data_decls.data_eqns, true);
  //add implementation of sort Pos and Bool
  impl_sort_pos(&data_decls);
  impl_sort_bool(&data_decls);
  //add new data declarations to spec
  spec = add_data_decls(spec, data_decls);
  //implement numerical pattern matching
  spec = impl_numerical_pattern_matching(spec);
  //implement sort references
  spec = impl_sort_refs(spec);
  //implement standard functions
  spec = impl_standard_functions_spec(spec);
  return spec;
}

ATermAppl implement_data_sort_expr(ATermAppl sort_expr, ATermAppl& spec)
{
  assert(gsIsSortExpr(sort_expr));
  return impl_exprs_with_spec(sort_expr, spec);
}

ATermAppl implement_data_data_expr(ATermAppl data_expr, ATermAppl& spec)
{
  assert(gsIsDataExpr(data_expr));
  return impl_exprs_with_spec(data_expr, spec);
}

ATermAppl implement_data_mult_act(ATermAppl mult_act, ATermAppl& spec)
{
  assert(gsIsMultAct(mult_act));
  return impl_exprs_with_spec(mult_act, spec);
}

ATermAppl implement_data_proc_expr(ATermAppl proc_expr, ATermAppl& spec)
{
  assert(gsIsProcExpr(proc_expr));
  return impl_exprs_with_spec(proc_expr, spec);
}

ATermAppl implement_data_state_frm(ATermAppl state_frm, ATermAppl& spec)
{
  assert(gsIsStateFrm(state_frm));
  return impl_exprs_with_spec(state_frm, spec);
}

ATermAppl implement_data_action_rename_spec(ATermAppl ar_spec, ATermAppl& lps_spec)
{
  assert(gsIsActionRenameSpec(ar_spec));
  assert(gsIsSpecV1(lps_spec));

  int occ =
    gsCount((ATerm) gsMakeSortUnknown(), (ATerm) lps_spec) +
    gsCountAFun(gsAFunSortsPossible(), (ATerm) lps_spec);
  if (occ > 0) {
    gsErrorMsg("specification contains %d unresolved type%s\n", occ, (occ != 1)?"s":"");
    return NULL;
  }
  occ = gsCount((ATerm) gsMakeSortUnknown(), (ATerm) ar_spec);
  if (occ > 0) {
    gsErrorMsg("action rename specification contains %d unknown type%s\n", occ, (occ != 1)?"s":"");
    return NULL;
  }

  //append data declarations and action declarations for the ar_spec to the lps_spec
  t_data_decls lps_data = get_data_decls(lps_spec);
  t_data_decls ar_data = get_data_decls(ar_spec);
  concat_data_decls(&lps_data, &ar_data);
  lps_spec = set_data_decls(lps_spec, lps_data);
  ATermList lps_actions = ATLgetArgument(ATAgetArgument(lps_spec, 1), 0);
  ATermList ar_actions = ATLgetArgument(ATAgetArgument(ar_spec, 1), 0);
  ATermList new_actions = ATconcat(lps_actions, ar_actions);
  lps_spec = ATsetArgument(lps_spec, (ATerm)gsMakeActSpec(new_actions), 1);
  //remove decls from ar_spec
  initialize_data_decls(&ar_data);
  ar_spec = set_data_decls(ar_spec, ar_data);
  ar_spec = ATsetArgument(ar_spec, (ATerm)gsMakeActSpec(ATmakeList0()), 1);

  //implement system sorts and data expressions occurring in spec
  ATermList substs     = ATmakeList0();
  t_data_decls data_decls;
  initialize_data_decls(&data_decls);
  lps_spec = impl_exprs_appl(lps_spec, &substs, &data_decls);
  ar_spec = impl_exprs_appl(ar_spec, &substs, &data_decls);
  //perform substitutions on data declarations
  data_decls.sorts     = gsSubstValues_List(substs, data_decls.sorts,     true);
  data_decls.cons_ops  = gsSubstValues_List(substs, data_decls.cons_ops,  true);
  data_decls.ops       = gsSubstValues_List(substs, data_decls.ops,       true);
  data_decls.data_eqns = gsSubstValues_List(substs, data_decls.data_eqns, true);
  //add implementation of sort Pos and Bool
  impl_sort_pos(&data_decls);
  impl_sort_bool(&data_decls);
  //add new data declarations to spec
  lps_spec = add_data_decls(lps_spec, data_decls);
  // We need to copy the data declarations of the lps_spec to the ar_spec.
  // Just adding data_decls does not suffice. This causes implementation of sort
  // references (caused by structured sorts) to fail.
  ar_spec = ATsetArgument(ar_spec, ATgetArgument(lps_spec, 0), 0);
  //implement numerical pattern matching
  lps_spec = impl_numerical_pattern_matching(lps_spec);
  ar_spec = impl_numerical_pattern_matching(ar_spec);
  //implement sort references
  lps_spec = impl_sort_refs(lps_spec);
  ar_spec = impl_sort_refs(ar_spec);
  //implement standard functions
  lps_spec = impl_standard_functions_spec(lps_spec);
  ar_spec = impl_standard_functions_spec(ar_spec);
  return ar_spec;
}

ATermAppl implement_data_pbes_spec(ATermAppl spec)
{
  assert(gsIsPBES(spec));
  int occ =
    gsCount((ATerm) gsMakeSortUnknown(), (ATerm) spec) +
    gsCountAFun(gsAFunSortsPossible(), (ATerm) spec);
  if (occ > 0) {
    gsErrorMsg("specification contains %d unresolved type%s\n", occ, (occ != 1)?"s":"");
    return NULL;
  }
  //implement system sorts and data expressions occurring in spec
  ATermList substs     = ATmakeList0();
  t_data_decls data_decls;
  initialize_data_decls(&data_decls);
  spec = impl_exprs_appl(spec, &substs, &data_decls);
  //perform substitutions on data declarations
  data_decls.sorts     = gsSubstValues_List(substs, data_decls.sorts,     true);
  data_decls.cons_ops  = gsSubstValues_List(substs, data_decls.cons_ops,  true);
  data_decls.ops       = gsSubstValues_List(substs, data_decls.ops,       true);
  data_decls.data_eqns = gsSubstValues_List(substs, data_decls.data_eqns, true);
  //add implementation of sort Pos and Bool
  impl_sort_pos(&data_decls);
  impl_sort_bool(&data_decls);
  //add new data declarations to spec
  spec = add_data_decls(spec, data_decls);
  //implement numerical pattern matching
  spec = impl_numerical_pattern_matching(spec);
  //implement sort references
  spec = impl_sort_refs(spec);
  //implement standard functions
  spec = impl_standard_functions_spec(spec);
  return spec;
}

ATermAppl impl_sort_refs(ATermAppl spec)
{
  assert(gsIsSpecV1(spec) || gsIsPBES(spec) || gsIsActionRenameSpec(spec));
  //get sort declarations
  ATermAppl data_spec = ATAgetArgument(spec, 0);
  ATermAppl sort_spec = ATAgetArgument(data_spec, 0);
  ATermList sort_decls = ATLgetArgument(sort_spec, 0);
  //split sort declarations in sort id's and sort references
  ATermList sort_ids = NULL;
  ATermList sort_refs = NULL;
  split_sort_decls(sort_decls, &sort_ids, &sort_refs);
  //replace the sort declarations in spec by the sort_ids, the list of
  //identifiers
  sort_spec = ATsetArgument(sort_spec, (ATerm) sort_ids, 0);  
  data_spec = ATsetArgument(data_spec, (ATerm) sort_spec, 0);
  spec = ATsetArgument(spec, (ATerm) data_spec, 0);
  //make list of substitutions from sort_refs, the list of sort references
  ATermList substs = ATmakeList0();
  while (!ATisEmpty(sort_refs))
  {
    ATermAppl sort_ref = ATAgetFirst(sort_refs);
    ATermAppl lhs = gsMakeSortId(ATAgetArgument(sort_ref, 0));
    ATermAppl rhs = ATAgetArgument(sort_ref, 1);
    //if rhs is the first occurrence of an implementation of a type constructor
    //at the rhs of a sort reference, add rhs := lhs; otherwise add lhs := rhs
    ATermAppl subst;
    if (is_struct_sort_id(rhs) || is_list_sort_id(rhs) || is_set_sort_id(rhs) ||
      is_bag_sort_id(rhs))
    {
      subst = gsMakeSubst_Appl(rhs, lhs);
    } else {
      subst = gsMakeSubst_Appl(lhs, rhs);
    }
    substs = ATinsert(substs, (ATerm) subst);
    //perform substitution on the remaining elements of sort_refs
    sort_refs = ATgetNext(sort_refs);    
    sort_refs = gsSubstValues_List(ATmakeList1((ATerm) subst), sort_refs, true);
  }
  //perform substitutions on spec
  spec = gsSubstValues_Appl(substs, spec, true);
  return spec;
}

ATermAppl impl_standard_functions_spec(ATermAppl spec)
{
  assert(gsIsSpecV1(spec) || gsIsPBES(spec) || gsIsActionRenameSpec(spec));
  //initalise data declarations
  t_data_decls data_decls;
  initialize_data_decls(&data_decls);
  //get sorts occurring in spec
  ATermList sorts = get_sorts((ATerm) spec);
  //implement standard functions for each sort in sorts
  while (!ATisEmpty(sorts))
  {
    impl_standard_functions_sort(ATAgetFirst(sorts), &data_decls);
    sorts = ATgetNext(sorts);
  }
  //add new data declarations to spec
  spec = add_data_decls(spec, data_decls);
  return spec;
}

ATermAppl impl_numerical_pattern_matching(ATermAppl spec)
{
  assert(gsIsSpecV1(spec) || gsIsPBES(spec) || gsIsActionRenameSpec(spec));
  //get data equations
  ATermAppl data_spec = ATAgetArgument(spec, 0);
  ATermAppl data_eqn_spec = ATAgetArgument(data_spec, 3);
  ATermList data_eqns = ATLgetArgument(data_eqn_spec, 0);
  //implement pattern matching for each equation
  ATermList l = ATmakeList0();
  while (!ATisEmpty(data_eqns)) {
    ATermAppl data_eqn = ATAgetFirst(data_eqns);
    //implement pattern matching on the left-hand side of the data equation
    ATermAppl lhs = ATAgetArgument(data_eqn, 2);
    lhs = impl_numerical_pattern_matching_expr(lhs, true);
    data_eqn = ATsetArgument(data_eqn, (ATerm) lhs, 2);
    l = ATinsert(l, (ATerm) data_eqn);
    data_eqns = ATgetNext(data_eqns);
  }
  data_eqns = ATreverse(l);
  data_eqn_spec = ATsetArgument(data_eqn_spec, (ATerm) data_eqns, 0);
  data_spec = ATsetArgument(data_spec, (ATerm) data_eqn_spec, 3);
  spec = ATsetArgument(spec, (ATerm) data_spec, 0);
  return spec;
}

ATermAppl impl_numerical_pattern_matching_expr(ATermAppl data_expr, bool top_level)
{
  assert(gsIsDataExpr(data_expr));
  if (gsIsDataVarId(data_expr) || gsIsOpId(data_expr)) {
    return data_expr;
  }
  assert(gsIsDataAppl(data_expr));
  if (!top_level) {
    //implement numerical patterns
    if (gsIsDataExprPos2Nat(data_expr)) {
      data_expr = gsMakeDataExprCNat(ATAgetFirst(ATLgetArgument(data_expr, 1)));
    } else if (gsIsDataExprPos2Int(data_expr)) {
      data_expr = gsMakeDataExprCInt(gsMakeDataExprCNat(ATAgetFirst(ATLgetArgument(data_expr, 1))));
    } else if (gsIsDataExprPos2Real(data_expr)) {
      data_expr = gsMakeDataExprCReal(gsMakeDataExprCInt(gsMakeDataExprCNat(ATAgetFirst(ATLgetArgument(data_expr, 1)))));
    } else if (gsIsDataExprNat2Int(data_expr)) {
      data_expr = gsMakeDataExprCInt(ATAgetFirst(ATLgetArgument(data_expr, 1)));
    } else if (gsIsDataExprNat2Real(data_expr)) {
      data_expr = gsMakeDataExprCReal(gsMakeDataExprCInt(ATAgetFirst(ATLgetArgument(data_expr, 1))));
    } else if (gsIsDataExprInt2Real(data_expr)) {
      data_expr = gsMakeDataExprCReal(ATAgetFirst(ATLgetArgument(data_expr, 1)));
    } else if (gsIsDataExprNeg(data_expr)) {
      if (ATisEqual(gsMakeOpIdNeg(gsMakeSortExprPos()),ATAgetArgument(data_expr,0))) {
        data_expr = gsMakeDataExprCNeg(ATAgetFirst(ATLgetArgument(data_expr, 1)));
      }
    }
  }
  //implement pattern matching in the head of data_expr
  ATermAppl head = ATAgetArgument(data_expr, 0);
  head = impl_numerical_pattern_matching_expr(head, top_level);
  //implement pattern matching in the arguments of data_expr
  ATermList args = ATLgetArgument(data_expr, 1);
  ATermList l = ATmakeList0();
  while (!ATisEmpty(args)) {
    ATermAppl arg = ATAgetFirst(args);
    arg = impl_numerical_pattern_matching_expr(arg, false);
    l = ATinsert(l, (ATerm) arg);
    args = ATgetNext(args);
  }
  args = ATreverse(l);
  return gsMakeDataAppl(head, args);
}

ATermAppl impl_exprs_with_spec(ATermAppl part, ATermAppl& spec)
{
  assert(gsIsSpecV1(spec) || gsIsPBES(spec) || gsIsDataSpec(spec));
  int occ =
    gsCount((ATerm) gsMakeSortUnknown(), (ATerm) spec) +
    gsCountAFun(gsAFunSortsPossible(), (ATerm) spec);
  if (occ > 0) {
    gsErrorMsg("specification contains %d unresolved type%s\n", occ, (occ != 1)?"s":"");
    return NULL;
  }
  occ = gsCount((ATerm) gsMakeSortUnknown(), (ATerm) part);
  if (occ > 0) {
    gsErrorMsg("part contains %d unknown type%s\n", occ, (occ != 1)?"s":"");
    return NULL;
  }

  //implement system sorts and data expressions occurring in spec
  ATermList substs     = ATmakeList0();
  t_data_decls data_decls;
  initialize_data_decls(&data_decls);
  spec = impl_exprs_appl(spec, &substs, &data_decls);
  part = impl_exprs_appl(part, &substs, &data_decls);
  //perform substitutions on data declarations
  data_decls.sorts     = gsSubstValues_List(substs, data_decls.sorts,     true);
  data_decls.cons_ops  = gsSubstValues_List(substs, data_decls.cons_ops,  true);
  data_decls.ops       = gsSubstValues_List(substs, data_decls.ops,       true);
  data_decls.data_eqns = gsSubstValues_List(substs, data_decls.data_eqns, true);
  //add implementation of sort Pos and Bool
  impl_sort_pos(&data_decls);
  impl_sort_bool(&data_decls);
  //add new data declarations to spec
  spec = add_data_decls(spec, data_decls);
  //implement numerical pattern matching
  spec = impl_numerical_pattern_matching(spec);
  //implement sort references
  spec = impl_sort_refs(spec);
  //implement standard functions
  spec = impl_standard_functions_spec(spec);
  return part;
}

ATermAppl impl_exprs_appl(ATermAppl part, ATermList *p_substs,
  t_data_decls *p_data_decls)
{
  bool recursive = true;
  //perform substitutions from *p_substs on part
  part = gsSubstValues_Appl(*p_substs, part, false);
  //replace part by an implementation if the head of part is a special
  //expression
  if (gsIsSortArrow(part)) {
    //part is a product arrow sort; skip
  } else if (gsIsSortStruct(part)) {
    //part is a structured sort; replace by a new sort and add data
    //declarations for this sort
    part = impl_sort_struct(part, p_substs, p_data_decls);
  } else if (gsIsSortExprList(part)) {
    //part is a list sort; replace by a new sort and add data declarations for
    //this sort
    part = impl_sort_list(part, p_substs, p_data_decls);
  } else if (gsIsSortExprSet(part)) {
    //part is a set sort; replace by a new sort and add data declarations for
    //this sort
    part = impl_sort_set(part, p_substs, p_data_decls);
  } else if (gsIsSortExprBag(part)) {
    //part is a bag sort; replace by a new sort and add data declarations for
    //this sort
    part = impl_sort_bag(part, p_substs, p_data_decls);
  } else if (gsIsSortId(part)) {
    //part is a sort identifier; add data declarations for this sort, if needed
    if (ATisEqual(part,gsMakeSortIdNat()))
    {
      //add implementation of sort Nat, if necessary
      if (ATindexOf(p_data_decls->sorts, (ATerm) gsMakeSortIdNat(), 0) == -1) {
        impl_sort_nat(p_data_decls);
      }
    } else if (ATisEqual(part, gsMakeSortIdInt())) {
      //add implementation of sort Int, if necessary
      if (ATindexOf(p_data_decls->sorts, (ATerm) gsMakeSortIdInt(), 0) == -1) {
        impl_sort_int(p_data_decls);
      }
    } else if (ATisEqual(part, gsMakeSortIdReal())) {
      //add implementation of sort Int, if necessary
      if (ATindexOf(p_data_decls->sorts, (ATerm) gsMakeSortIdReal(), 0) == -1) {
        impl_sort_real(p_data_decls);
      }
    }
  } else if (gsIsDataAppl(part)) {
    //part is a product data application
    //there are some cases that need implementation work
    ATermList l = ATLgetArgument(part, 1);
    ATermAppl newpart = ATAgetArgument(part, 0);
    if (gsIsOpId(newpart)) {
      ATermAppl Name = ATAgetArgument(newpart, 0);
      if (Name == gsMakeOpIdNameListEnum()) {
        //part is a list enumeration; replace by its internal representation
        ATermAppl sort = gsGetSortExprResult(ATAgetArgument(newpart, 1));
        if (ATgetLength(l) == 0) {
          //enumeration consists of 0 elements
          gsWarningMsg(
            "%P can not be implemented because it has 0 elements\n", newpart);
        } else {
          //make cons list
          l = ATreverse(l);
          part = gsMakeDataExprEmptyList(sort);
          while (!ATisEmpty(l))
          {
            part = gsMakeDataExprCons(ATAgetFirst(l), part);
            l = ATgetNext(l);
          }
        }
      } else if (Name == gsMakeOpIdNameSetEnum()) {
        //part is a set enumeration; replace by a set comprehension
        ATermAppl sort = gsGetSortExprResult(ATAgetArgument(newpart, 1));
        if (ATgetLength(l) == 0) {
          //enumeration consists of 0 elements
          gsWarningMsg(
            "%P can not be implemented because it has 0 elements\n", newpart);
        } else {
          part = impl_set_enum(l, sort);
        }
      } else if (Name == gsMakeOpIdNameBagEnum()) {
        //part is a bag enumeration; replace by a bag comprehension
        ATermAppl sort = gsGetSortExprResult(ATAgetArgument(newpart, 1));
        if (ATgetLength(l) == 0) {
          //enumeration consists of 0 elements
          gsWarningMsg(
            "%P can not be implemented because it has 0 elements\n", newpart);
        } else {
          part = impl_bag_enum(l, sort);
        }
      }
    }
  } else if (gsIsDataExprNumber(part)) {
    //part is a number; replace by its internal representation
    ATermAppl Number = ATAgetArgument(part, 0);
    ATermAppl sort = ATAgetArgument(part, 1);
    if (ATisEqual(sort, gsMakeSortExprPos()))
      part = gsMakeDataExprPos(gsATermAppl2String(Number));
    else if (ATisEqual(sort, gsMakeSortExprNat()))
      part = gsMakeDataExprNat(gsATermAppl2String(Number));
    else if (ATisEqual(sort, gsMakeSortExprInt()))
      part = gsMakeDataExprInt(gsATermAppl2String(Number));
    else //sort of part is wrong
      gsWarningMsg("%P can not be implemented because its sort differs from "
        "Pos, Nat or Int\n", part);
  } else if (gsIsBinder(part)) {
    ATermAppl binding_operator = ATAgetArgument(part, 0);
    if (gsIsSetBagComp(binding_operator)) {
      //part is a set/bag comprehension; replace by its implementation
      ATermList vars = ATLgetArgument(part, 1);
      ATermAppl var = ATAgetFirst(vars);
      ATermAppl body = ATAgetArgument(part, 2);
      ATermAppl body_sort = gsGetSort(body);
      ATermAppl var_sort = gsGetSort(var);
      if (!(ATisEqual(body_sort, gsMakeSortIdBool()) ||
          ATisEqual(body_sort, gsMakeSortIdNat()))) {
        //sort of the comprehension is wrong
        gsWarningMsg("%P can not be implemented because the body is of sort %P "
          "instead of Bool or Nat\n", part, body_sort);
      } else {
        if (ATisEqual(body_sort, gsMakeSortIdBool())) {
          //part is a set comprehension
          part = gsMakeDataExprSetComp(gsMakeBinder(gsMakeLambda(), vars, body), 
            gsMakeSortExprSet(var_sort));
        } else {
          //part is a bag comprehension
          part = gsMakeDataExprBagComp(gsMakeBinder(gsMakeLambda(), vars, body), 
            gsMakeSortExprBag(var_sort));
        }
      }
    } else if (gsIsSetComp(binding_operator)) {
      //part is a set comprehension; replace by its implementation
      ATermList vars = ATLgetArgument(part, 1);
      assert(!ATisEmpty(vars));
      ATermAppl var = ATAgetFirst(vars);
      ATermAppl body = ATAgetArgument(part, 2);
      ATermAppl var_sort = gsGetSort(var);
      part = gsMakeDataExprSetComp(gsMakeBinder(gsMakeLambda(), vars, body), 
        gsMakeSortExprSet(var_sort));
    } else if (gsIsBagComp(binding_operator)) {
      //part is a bag comprehension; replace by its implementation
      ATermList vars = ATLgetArgument(part, 1);
      ATermAppl var = ATAgetFirst(vars);
      ATermAppl body = ATAgetArgument(part, 2);
      ATermAppl var_sort = gsGetSort(var);
      part = gsMakeDataExprBagComp(gsMakeBinder(gsMakeLambda(), vars, body),
        gsMakeSortExprBag(var_sort));
    } else if (gsIsForall(binding_operator) || gsIsExists(binding_operator)) {
      //part is a quantification; replace by its implementation
      ATermList vars = ATLgetArgument(part, 1);
      bool is_forall = gsIsForall(binding_operator);
      part = gsMakeBinder(gsMakeLambda(), vars, ATAgetArgument(part, 2));
      part = is_forall?gsMakeDataExprForall(part):gsMakeDataExprExists(part);
    } else if (gsIsLambda(binding_operator)) {
      //part is a lambda abstraction; replace by a named function
      //implement the body, the bound variables and the free variables
      ATermList bound_vars = impl_exprs_list(ATLgetArgument(part, 1),
        p_substs, p_data_decls);
      assert(!ATisEmpty(bound_vars));
      ATermAppl body = impl_exprs_appl(ATAgetArgument(part, 2),
        p_substs, p_data_decls);
      ATermList free_vars = impl_exprs_list(get_free_vars(part),
        p_substs, p_data_decls);
      //create sort for the new operation identifier
      ATermAppl op_id_sort = gsMakeSortArrowList(gsGetSorts(free_vars), gsMakeSortArrowList(gsGetSorts(bound_vars), gsGetSort(body)));
      //create new operation identifier
      ATermAppl op_id = make_fresh_lambda_op_id(op_id_sort, (ATerm) p_data_decls->ops);
      //add operation identifier to the data declarations
      p_data_decls->ops = ATinsert(p_data_decls->ops, (ATerm) op_id);
      //add data equation for the operation to the data declarations
      p_data_decls->data_eqns = ATinsert(p_data_decls->data_eqns, (ATerm)
        gsMakeDataEqn(ATconcat(free_vars, bound_vars), gsMakeNil(), gsMakeDataApplList(gsMakeDataApplList(op_id, free_vars), bound_vars), body));
      //replace part
      ATermAppl new_part = gsMakeDataApplList(op_id, free_vars);
      *p_substs = gsAddSubstToSubsts(gsMakeSubst_Appl(part, new_part), *p_substs);
      part = new_part;
      recursive = false;
    }
  } else if (gsIsWhr(part)) {
    //part is a where clause; replace by its corresponding lambda expression
    ATermAppl body = ATAgetArgument(part, 0);
    ATermList whr_decls = ATLgetArgument(part, 1);
    if (ATgetLength(whr_decls) == 0) {
      //where clause consists of 0 where clause declarations
      gsWarningMsg("%P can not be implemented because it has 0 where clause "
         "declarations\n", part);
    } else {
      //make list of variables and where expressions
      whr_decls = ATreverse(whr_decls);
      ATermList vars = ATmakeList0();
      ATermList exprs = ATmakeList0();
      while (!ATisEmpty(whr_decls))
      {
        ATermAppl whr_decl = ATAgetFirst(whr_decls);
        ATermAppl var = ATAgetArgument(whr_decl, 0);
        ATermAppl expr = ATAgetArgument(whr_decl, 1);
        vars = ATinsert(vars, (ATerm) var);
        exprs = ATinsert(exprs, (ATerm) expr);
        whr_decls = ATgetNext(whr_decls);
      }
      //replace part
      part = gsMakeDataApplList(gsMakeBinder(gsMakeLambda(), vars, body), exprs);
    }
  }
  //implement expressions in the arguments of part
  if (recursive) {
    AFun head = ATgetAFun(part);
    int nr_args = ATgetArity(head);      
    if (nr_args > 0) {
      DECL_A(args,ATerm,nr_args);
      for (int i = 0; i < nr_args; i++) {
        ATerm arg = ATgetArgument(part, i);
        if (ATgetType(arg) == AT_APPL)
          args[i] = (ATerm) impl_exprs_appl((ATermAppl) arg, p_substs,
            p_data_decls);
        else //ATgetType(arg) == AT_LIST
          args[i] = (ATerm) impl_exprs_list((ATermList) arg, p_substs,
            p_data_decls);
      }
      part = ATmakeApplArray(head, args);
      FREE_A(args);
    }
  }
  return part;
}

ATermList impl_exprs_list(ATermList parts, ATermList *p_substs,
  t_data_decls *p_data_decls)
{
  ATermList result = ATmakeList0();
  while (!ATisEmpty(parts))
  {
    result = ATinsert(result, (ATerm)
      impl_exprs_appl(ATAgetFirst(parts), p_substs, p_data_decls));
    parts = ATgetNext(parts);
  }
  return ATreverse(result);
}

ATermAppl impl_set_enum(ATermList elts, ATermAppl sort_expr)
{
  assert(ATgetLength(elts) > 0);
  ATermAppl result;
  //introduce a fresh variable
  ATermAppl var =
    gsMakeDataVarId(gsFreshString2ATermAppl("x", (ATerm) elts, true),
      gsGetSort(ATAgetFirst(elts)));
  //make body for the lambda abstraction
  elts = ATreverse(elts);
  ATermAppl elt = ATAgetFirst(elts);
  result = gsMakeDataExprEq(var, elt);
  elts = ATgetNext(elts);
  while (!ATisEmpty(elts)) {
    elt = ATAgetFirst(elts);
    result = gsMakeDataExprOr(gsMakeDataExprEq(var, elt), result);
    elts = ATgetNext(elts);
  }
  //make lambda abstraction
  result = gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) var), result);
  //make set comprehension
  result = gsMakeDataExprSetComp(result, sort_expr);
  //return result
  return result;
}

ATermAppl impl_bag_enum(ATermList elts, ATermAppl sort_expr)
{
  assert(ATgetLength(elts) > 0);
  ATermAppl result;
  //introduce a fresh variable
  ATermAppl var =
    gsMakeDataVarId(gsFreshString2ATermAppl("x", (ATerm) elts, true),
      gsGetSort(ATAgetFirst(elts)));
  //make body for the lambda abstraction
  elts = ATreverse(elts);
  ATermAppl amt = ATAgetFirst(elts);
  elts = ATgetNext(elts);
  ATermAppl elt = ATAgetFirst(elts);
  result = gsMakeDataExprIf(gsMakeDataExprEq(var, elt),
    amt, gsMakeDataExprC0());
  elts = ATgetNext(elts);
  while (!ATisEmpty(elts)) {
    amt = ATAgetFirst(elts);
    elts = ATgetNext(elts);
    elt = ATAgetFirst(elts);
    result = gsMakeDataExprAdd(
      gsMakeDataExprIf(gsMakeDataExprEq(var, elt),
      amt, gsMakeDataExprC0()), result);
    elts = ATgetNext(elts);
  }
  //make lambda abstraction
  result = gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) var), result);
  //make bag comprehension
  result = gsMakeDataExprBagComp(result, sort_expr);
  return result;
}

ATermAppl apply_op_id_to_vars(ATermAppl op_id, ATermList *p_args,
  ATermList *p_vars, ATerm context)
{
  ATermAppl sort = gsGetSort(op_id);
  assert(gsIsSortId(sort) || gsIsSortArrow(sort));
  *p_args = ATmakeList0();
  if (gsIsSortId(sort)) {
    return op_id;
  }
  
  assert(gsIsSortArrow(sort));
  
  ATermAppl t = op_id;
  ATermList tmpvars = *p_vars; // We only use variables once in a term
 
  ATermList sort_dom = ATLgetArgument(sort, 0);
  while ( !ATisEmpty(sort_dom) )
  {
    // Find a variable with the right sort...
    ATermAppl sort_dom_elt = ATAgetFirst(sort_dom);
    ATermAppl v = NULL;
    for (ATermList n=tmpvars; !ATisEmpty(n) && (v == NULL); n=ATgetNext(n))
    {
      ATermAppl tmpvar = ATAgetFirst(n);
      if ( ATisEqual(gsGetSort(tmpvar), sort_dom_elt) )
      {
        v = tmpvar;
        tmpvars = ATremoveElement(tmpvars,(ATerm) v);
      }
    }
    // ...or create a new one
    if ( v == NULL )
    {
      v = gsMakeDataVarId(gsFreshString2ATermAppl("v",
            (ATerm) ATmakeList2((ATerm) (*p_vars),context),false), sort_dom_elt);                      
      // Add it to *p_vars
      *p_vars = ATinsert(*p_vars,(ATerm) v);
    }
    assert(v != NULL);
 
    // Add v to *p_args
    *p_args = ATinsert(*p_args,(ATerm) v);

    // Next
    sort_dom = ATgetNext(sort_dom);
  }
  
  // Apply t to p_args
  *p_args = ATreverse(*p_args);
  t = gsMakeDataAppl(t, *p_args);
  *p_vars = ATreverse(*p_vars);
  return t;
}     

ATermAppl impl_sort_struct(ATermAppl sort_struct, ATermList *p_substs,
  t_data_decls *p_data_decls)
{
  assert(gsIsSortStruct(sort_struct));
  //declare fresh sort identifier for sort_struct
  ATermAppl sort_id = make_fresh_struct_sort_id((ATerm) p_data_decls->sorts);
  p_data_decls->sorts = ATinsert(p_data_decls->sorts, (ATerm) sort_id);
  //add substitution for this identifier
  ATermAppl subst = gsMakeSubst_Appl(sort_struct, sort_id);
  *p_substs = gsAddSubstToSubsts(subst, *p_substs);
  //store constructor, projection and recogniser operations for this identifier
  ATermList cons_ops = ATmakeList0();
  ATermList proj_ops = ATmakeList0();
  ATermList projs = ATmakeList0();
  ATermList rec_ops = ATmakeList0();
  ATermList recs = ATmakeList0();
  ATermList struct_conss = ATLgetArgument(sort_struct, 0);
  while (!ATisEmpty(struct_conss))
  {
    ATermAppl struct_cons = ATAgetFirst(struct_conss);
    ATermAppl cons_name = ATAgetArgument(struct_cons, 0);
    ATermList struct_projs = ATLgetArgument(struct_cons, 1);
    ATermAppl rec_name = ATAgetArgument(struct_cons, 2);
    ATermList struct_cons_sorts = ATmakeList0();
    //store projection operations in proj_ops and store the implementations of
    //the sorts in struct_cons_sorts
    int i = 0;
    while (!ATisEmpty(struct_projs))
    {
      ATermAppl struct_proj = ATAgetFirst(struct_projs);
      ATermAppl proj_name = ATAgetArgument(struct_proj, 0);
      ATermAppl proj_sort = impl_exprs_appl(ATAgetArgument(struct_proj, 1),
        p_substs, p_data_decls);
      struct_cons_sorts = ATinsert(struct_cons_sorts, (ATerm) proj_sort);
      //store projection operation in proj_ops and projs
      if (!gsIsNil(proj_name)) {
        ATermAppl proj_op = gsMakeOpId(proj_name, gsMakeSortArrow1(sort_id, proj_sort));
        proj_ops = ATinsert(proj_ops, (ATerm) proj_op);
        projs = ATinsert(projs, (ATerm) ATmakeList2((ATerm) proj_op, (ATerm) ATmakeInt(i)));
      }
      struct_projs = ATgetNext(struct_projs);
      i++;
    }
    struct_cons_sorts = ATreverse(struct_cons_sorts);
    //store constructor operation in cons_ops
    ATermAppl cons_op;
    if (ATisEmpty(struct_cons_sorts))
    {
      cons_op = gsMakeOpId(cons_name, sort_id);
    } else {
      cons_op = gsMakeOpId(cons_name, gsMakeSortArrow(struct_cons_sorts, sort_id));
    }
    cons_ops = ATinsert(cons_ops, (ATerm) cons_op);
    //store recogniser in rec_ops and recs
    if (!gsIsNil(rec_name)) {
      ATermAppl rec_op = gsMakeOpId(rec_name, gsMakeSortArrow1(sort_id, gsMakeSortExprBool()));
      rec_ops = ATinsert(rec_ops, (ATerm) rec_op);
      recs = ATinsert(recs, (ATerm) ATmakeList2((ATerm) rec_op, (ATerm) cons_op));
    }
    //add constructor to projs
    ATermList tmpl = ATmakeList0();
    for (; !ATisEmpty(projs); projs=ATgetNext(projs))
    {
      tmpl = ATinsert(tmpl, (ATerm) ATappend(ATLgetFirst(projs), (ATerm) cons_op));
    }
    projs = ATreverse(tmpl);
    struct_conss = ATgetNext(struct_conss);
  }
  //add declarations for the constructor, projection and recogniser operations
  p_data_decls->cons_ops = ATconcat(ATreverse(cons_ops), p_data_decls->cons_ops);
  p_data_decls->ops = ATconcat(ATconcat(ATreverse(proj_ops), ATreverse(rec_ops)),
    p_data_decls->ops);
  //Declare data equations for structured sort
  ATermList op_eqns = ATmakeList0();
  ATermAppl nil = gsMakeNil();
  //ATermAppl t = gsMakeDataExprTrue();
  //ATermAppl f = gsMakeDataExprFalse();
  ATermAppl b = gsMakeDataVarId(gsString2ATermAppl("b"), gsMakeSortExprBool());
  // XXX more intelligent variable names would be nice
  ATermAppl x = gsMakeDataVarId(gsString2ATermAppl("x"), sort_id);
  ATermAppl y = gsMakeDataVarId(gsString2ATermAppl("y"), sort_id);
  ATermList vars = ATmakeList3((ATerm) b, (ATerm) x, (ATerm) y);
  ATermList rhsv = ATmakeList0();
  ATermList lhsv = ATmakeList0();
  ATermList id_ctx = ATconcat(p_data_decls->sorts,
                      ATconcat(p_data_decls->ops,p_data_decls->cons_ops));
  //store equations for projections in op_eqns
  for (; !ATisEmpty(projs); projs=ATgetNext(projs))
  {
    ATermList l = ATLgetFirst(projs);
    ATermAppl proj_op = ATAgetFirst(l);
    l = ATgetNext(l);
    int proj_op_index = ATgetInt((ATermInt) ATgetFirst(l));
    l = ATgetNext(l);
    ATermAppl cons_op = ATAgetFirst(l);
    //Apply constructor cons_op to (fresh) variables and store its arguments in lhsv
    ATermAppl cons_expr = apply_op_id_to_vars(cons_op, &lhsv, &vars, (ATerm) id_ctx);
    //Add equation for projection function proj_op
    op_eqns = ATinsert(op_eqns,
      (ATerm) gsMakeDataEqn(lhsv, nil,
        gsMakeDataAppl1(proj_op, cons_expr),
        ATAelementAt(lhsv, proj_op_index)));
  }
  //store equations for recognition in op_eqns
  for (; !ATisEmpty(recs); recs=ATgetNext(recs))
  {
    ATermList l = ATLgetFirst(recs);
    ATermAppl rec_op = ATAgetFirst(l);
    l = ATgetNext(l);
    ATermAppl rec_cons_op = ATAgetFirst(l);
    // Add equation for every constructor
    for (ATermList m = cons_ops; !ATisEmpty(m); m=ATgetNext(m))
    {
      ATermAppl cons_op = ATAgetFirst(m);
      // Apply constructor cons_op to (fresh) variables and store its
      // arguments in lhsv
      ATermAppl cons_expr = apply_op_id_to_vars(cons_op, &lhsv, &vars, (ATerm) id_ctx);
      // Add right equation to op_eqns
      op_eqns = ATinsert(op_eqns, (ATerm) gsMakeDataEqn(lhsv, nil,
         gsMakeDataAppl1(rec_op, cons_expr),
         ATisEqual(ATAgetFirst(m), rec_cons_op)?gsMakeDataExprTrue():gsMakeDataExprFalse()));
    }
  }
  //store equations for equalities in op_eqns
  //one equation for every pair of constructors
  for (ATermList l=cons_ops; !ATisEmpty(l); l=ATgetNext(l))
  {
    for (ATermList m=cons_ops; !ATisEmpty(m); m=ATgetNext(m))
    {
      ATermAppl cons_op_lhs = ATAgetFirst(l);
      ATermAppl cons_op_rhs = ATAgetFirst(m);
      // Save vars list
      // Apply constructor cons_op_lhs to (fresh) variables and store its
      // arguments in lhsv
      ATermAppl cons_expr_lhs = apply_op_id_to_vars(cons_op_lhs, &lhsv, &vars, (ATerm) id_ctx);
      // Apply constructor cons_op_rhs to (fresh) variables and store its
      // arguments in rhsv (making sure we don't use the vars that occur in t)
      ATermList tmpvars = subtract_list(vars, lhsv);
      ATermAppl cons_expr_rhs = apply_op_id_to_vars(ATAgetFirst(m), &rhsv, &tmpvars, (ATerm) ATconcat(lhsv, id_ctx));
      // Update vars
      vars = merge_list(vars, rhsv);
      // Combine variable lists of lhs and rhs
      ATermList vs = ATconcat(lhsv, rhsv);
      // Create right result
      ATermAppl result_expr = NULL;
      if ( ATisEqual(cons_op_lhs, cons_op_rhs) )
      {
        // Constructors are the same, so match all variables
        if (ATisEmpty(lhsv)) {
          result_expr = gsMakeDataExprTrue();
        } else {
          for (; !ATisEmpty(lhsv); lhsv = ATgetNext(lhsv), rhsv = ATgetNext(rhsv))
          {
            if ( result_expr == NULL )
            {
              result_expr = gsMakeDataExprEq(ATAgetFirst(lhsv), ATAgetFirst(rhsv));
            } else {
              result_expr = gsMakeDataExprAnd(result_expr,
                gsMakeDataExprEq(ATAgetFirst(lhsv), ATAgetFirst(rhsv)));
            }
          }
        }
      } else {
        // Different constructor, so not equal
        result_expr = gsMakeDataExprFalse();
      }
      // Add equation to op_eqns
      op_eqns = ATinsert(op_eqns, (ATerm) gsMakeDataEqn(vs, nil,
        gsMakeDataExprEq(cons_expr_lhs, cons_expr_rhs), result_expr));
    }
  }
  //Add op_eqns to data_eqns
  p_data_decls->data_eqns = ATconcat(op_eqns, p_data_decls->data_eqns);

  return sort_id;
}

ATermList build_list_equations(ATermAppl sort_elt, ATermAppl sort_list)
{
  //Declare data equations for sort sort_id
  ATermList el = ATmakeList0();
  ATermAppl el_sort_id = gsMakeDataExprEmptyList(sort_list);
  ATermAppl s_sort_id = gsMakeDataVarId(gsString2ATermAppl("s"), sort_list);
  ATermAppl t_sort_id = gsMakeDataVarId(gsString2ATermAppl("t"), sort_list);
  ATermAppl d_sort_elt = gsMakeDataVarId(gsString2ATermAppl("d"), sort_elt);
  ATermAppl e_sort_elt = gsMakeDataVarId(gsString2ATermAppl("e"), sort_elt);
  ATermAppl p = gsMakeDataVarId(gsString2ATermAppl("p"), gsMakeSortExprPos());
  ATermAppl ds = gsMakeDataExprCons(d_sort_elt, s_sort_id);
  ATermAppl es = gsMakeDataExprCons(e_sort_elt, s_sort_id);
  ATermAppl et = gsMakeDataExprCons(e_sort_elt, t_sort_id);
  ATermAppl nil = gsMakeNil();
  ATermAppl zero = gsMakeDataExprC0();
  ATermAppl t = gsMakeDataExprTrue();
  ATermAppl f = gsMakeDataExprFalse();
  ATermList dl = ATmakeList1((ATerm) d_sort_elt);
  ATermList sl = ATmakeList1((ATerm) s_sort_id);
  ATermList dsl = ATmakeList2((ATerm) d_sort_elt, (ATerm) s_sort_id);
  ATermList desl = ATmakeList3((ATerm) d_sort_elt, (ATerm) e_sort_elt,
    (ATerm) s_sort_id);
  ATermList dstl = ATmakeList3((ATerm) d_sort_elt, (ATerm) s_sort_id,
    (ATerm) t_sort_id);
  ATermList destl = ATmakeList4((ATerm) d_sort_elt, (ATerm) e_sort_elt,
    (ATerm) s_sort_id, (ATerm) t_sort_id);
  ATermList dspl = ATmakeList3((ATerm) d_sort_elt, (ATerm) s_sort_id, (ATerm) p);

  ATermList new_data_eqns = ATmakeList(21,
      //equality (sort_id -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(dsl, nil, gsMakeDataExprEq(el_sort_id, ds), f),
      (ATerm) gsMakeDataEqn(dsl, nil, gsMakeDataExprEq(ds, el_sort_id), f),
      (ATerm) gsMakeDataEqn(destl, nil,
        gsMakeDataExprEq(ds, et),
        gsMakeDataExprAnd(
          gsMakeDataExprEq(d_sort_elt, e_sort_elt),
          gsMakeDataExprEq(s_sort_id, t_sort_id))),
      (ATerm) gsMakeDataEqn(sl, nil, gsMakeDataExprEq(s_sort_id, s_sort_id), t),
      //element test (sort_elt -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(dl, nil,
        gsMakeDataExprEltIn(d_sort_elt, el_sort_id),
        f),
      (ATerm) gsMakeDataEqn(desl, nil,
        gsMakeDataExprEltIn(d_sort_elt, es),
        gsMakeDataExprOr(
          gsMakeDataExprEq(d_sort_elt, e_sort_elt),
          gsMakeDataExprEltIn(d_sort_elt, s_sort_id))),
      //list size (sort_id -> Nat)
      (ATerm) gsMakeDataEqn(el, nil,
        gsMakeDataExprListSize(el_sort_id),
        gsMakeDataExprC0()),
      (ATerm) gsMakeDataEqn(dsl, nil,
        gsMakeDataExprListSize(ds),
        gsMakeDataExprCNat(
          gsMakeDataExprSucc(gsMakeDataExprListSize(s_sort_id)))),
      //list snoc (sort_id -> sort_elt -> sort_id)
      (ATerm) gsMakeDataEqn(dl, nil,
        gsMakeDataExprSnoc(el_sort_id, d_sort_elt),
        gsMakeDataExprCons(d_sort_elt, el_sort_id)),
      (ATerm) gsMakeDataEqn(desl, nil,
        gsMakeDataExprSnoc(ds, e_sort_elt),
        gsMakeDataExprCons(d_sort_elt, gsMakeDataExprSnoc(s_sort_id, e_sort_elt))),
      //list concatenation (sort_id -> sort_id -> sort_id)
      (ATerm) gsMakeDataEqn(sl, nil,
        gsMakeDataExprConcat(el_sort_id, s_sort_id),
        s_sort_id),
      (ATerm) gsMakeDataEqn(dstl, nil,
        gsMakeDataExprConcat(ds, t_sort_id),
        gsMakeDataExprCons(d_sort_elt, gsMakeDataExprConcat(s_sort_id, t_sort_id))),
      (ATerm) gsMakeDataEqn(sl, nil,
        gsMakeDataExprConcat(s_sort_id, el_sort_id),
        s_sort_id),
      //list element at (sort_id -> Nat -> sort_elt)
      (ATerm) gsMakeDataEqn(dsl, nil,
        gsMakeDataExprEltAt(ds, zero, sort_elt),
        d_sort_elt),
      (ATerm) gsMakeDataEqn(dspl, nil,
        gsMakeDataExprEltAt(ds, gsMakeDataExprCNat(p), sort_elt),
        gsMakeDataExprEltAt(s_sort_id, gsMakeDataExprPred(p), sort_elt)),
      //head (sort_id -> sort_elt)
      (ATerm) gsMakeDataEqn(dsl, nil,
         gsMakeDataExprHead(ds, sort_elt),
         d_sort_elt),
      //tail (sort_id -> sort_id)
      (ATerm) gsMakeDataEqn(dsl, nil,
         gsMakeDataExprTail(ds),
         s_sort_id),
      //right head (sort_id -> sort_elt)
      (ATerm) gsMakeDataEqn(dl, nil,
        gsMakeDataExprRHead(gsMakeDataExprCons(d_sort_elt, el_sort_id), sort_elt),
        d_sort_elt),
      (ATerm) gsMakeDataEqn(desl, nil,
        gsMakeDataExprRHead(gsMakeDataExprCons(d_sort_elt,
          gsMakeDataExprCons(e_sort_elt, s_sort_id)), sort_elt),
        gsMakeDataExprRHead(gsMakeDataExprCons(e_sort_elt, s_sort_id), sort_elt)),
      //right tail (sort_id -> sort_id)
      (ATerm) gsMakeDataEqn(dl, nil,
        gsMakeDataExprRTail(gsMakeDataExprCons(d_sort_elt, el_sort_id)),
        el_sort_id),
      (ATerm) gsMakeDataEqn(desl, nil,
        gsMakeDataExprRTail(gsMakeDataExprCons(d_sort_elt,
          gsMakeDataExprCons(e_sort_elt, s_sort_id))),
        gsMakeDataExprCons(d_sort_elt,
          gsMakeDataExprRTail(gsMakeDataExprCons(e_sort_elt, s_sort_id)))));

  return new_data_eqns;
}

ATermAppl impl_sort_list(ATermAppl sort_list, ATermList *p_substs,
  t_data_decls *p_data_decls)
{
  assert(gsIsSortExprList(sort_list));
  ATermAppl sort_elt = ATAgetArgument(sort_list, 1);

  //declare constructors for sort sort_id
  ATermList new_cons_ops = ATmakeList2(
      (ATerm) gsMakeOpIdEmptyList(sort_list),
      (ATerm) gsMakeOpIdCons(sort_elt, sort_list));

  //Declare operations for sort sort_id
  ATermList new_ops = ATmakeList(9,
      (ATerm) gsMakeOpIdEltIn(sort_elt, sort_list),
      (ATerm) gsMakeOpIdListSize(sort_list),
      (ATerm) gsMakeOpIdSnoc(sort_list, sort_elt),
      (ATerm) gsMakeOpIdConcat(sort_list),
      (ATerm) gsMakeOpIdEltAt(sort_list, sort_elt),
      (ATerm) gsMakeOpIdHead(sort_list, sort_elt),
      (ATerm) gsMakeOpIdTail(sort_list),
      (ATerm) gsMakeOpIdRHead(sort_list, sort_elt),
      (ATerm) gsMakeOpIdRTail(sort_list));

  ATermList new_data_eqns = build_list_equations(sort_elt, sort_list);

  //declare fresh sort identifier for sort_list
  ATermAppl sort_id = make_fresh_list_sort_id((ATerm) p_data_decls->sorts);
  p_data_decls->sorts = ATinsert(p_data_decls->sorts, (ATerm) sort_id);
  //implement sort_elt
  //this needs to be done first in order to keep the substitutions sound!
  sort_elt = impl_exprs_appl(sort_elt, p_substs, p_data_decls);
  //add substitution for sort_list
  ATermAppl subst = gsMakeSubst_Appl(sort_list, sort_id);
  *p_substs = gsAddSubstToSubsts(subst, *p_substs);

  //perform substitutions
  new_cons_ops = gsSubstValues_List(*p_substs, new_cons_ops, true);
  p_data_decls->cons_ops = ATconcat(new_cons_ops, p_data_decls->cons_ops);
  new_ops = gsSubstValues_List(*p_substs, new_ops, true);
  p_data_decls->ops = ATconcat(new_ops, p_data_decls->ops);
  new_data_eqns = gsSubstValues_List(*p_substs, new_data_eqns, true);
  p_data_decls->data_eqns = ATconcat(new_data_eqns, p_data_decls->data_eqns);

  //add implementation of sort Nat, if necessary
  if (ATindexOf(p_data_decls->sorts, (ATerm) gsMakeSortIdNat(), 0) == -1) {
    impl_sort_nat(p_data_decls);
  }
  return sort_id;
}

ATermList build_set_equations(ATermAppl sort_elt, ATermAppl sort_set)
{
  //declare data equations for sort sort_id
  ATermAppl sort_func = gsMakeSortArrow1(sort_elt, gsMakeSortExprBool());
  ATermList el = ATmakeList0();
  ATermAppl s_sort_id = gsMakeDataVarId(gsString2ATermAppl("s"), sort_set);
  ATermAppl t_sort_id = gsMakeDataVarId(gsString2ATermAppl("t"), sort_set);
  ATermAppl f_sort_func = gsMakeDataVarId(gsString2ATermAppl("f"), sort_func);
  ATermAppl g_sort_func = gsMakeDataVarId(gsString2ATermAppl("g"), sort_func);
  ATermAppl d_sort_elt = gsMakeDataVarId(gsString2ATermAppl("d"), sort_elt);
  ATermAppl x_sort_elt = gsMakeDataVarId(gsString2ATermAppl("x"), sort_elt);
  ATermAppl nil = gsMakeNil();
  ATermAppl f = gsMakeDataExprFalse();
  ATermList stl = ATmakeList2((ATerm) s_sort_id, (ATerm) t_sort_id);
  ATermList fl = ATmakeList1((ATerm) f_sort_func);
  ATermList dfl = ATmakeList2((ATerm) d_sort_elt, (ATerm) f_sort_func);
  ATermList fgl = ATmakeList2((ATerm) f_sort_func, (ATerm) g_sort_func);
  ATermAppl false_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt), f);
  ATermAppl imp_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt),
      gsMakeDataExprImp(
        gsMakeDataAppl1(f_sort_func, x_sort_elt),
        gsMakeDataAppl1(g_sort_func, x_sort_elt)
      )
    );
  ATermAppl OrFunc =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt),
      gsMakeDataExprOr(
        gsMakeDataAppl1(f_sort_func, x_sort_elt),
        gsMakeDataAppl1(g_sort_func, x_sort_elt)
      )
    );
  ATermAppl and_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt),
      gsMakeDataExprAnd(
        gsMakeDataAppl1(f_sort_func, x_sort_elt),
        gsMakeDataAppl1(g_sort_func, x_sort_elt)
      )
    );
  ATermAppl not_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt),
      gsMakeDataExprNot(gsMakeDataAppl1(f_sort_func, x_sort_elt))
    );

  ATermList new_data_eqns = ATmakeList(9,
      //equality (sort_id -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(fgl, nil,
        gsMakeDataExprEq(
          gsMakeDataExprSetComp(f_sort_func, sort_set),
          gsMakeDataExprSetComp(g_sort_func, sort_set)), 
        gsMakeDataExprEq(f_sort_func, g_sort_func)),
      //empty set (sort_id)
      (ATerm) gsMakeDataEqn(el, nil,
        gsMakeDataExprEmptySet(sort_set),
        gsMakeDataExprSetComp(false_func, sort_set)),
      //element test (sort_elt -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(dfl, nil,
        gsMakeDataExprEltIn(d_sort_elt, gsMakeDataExprSetComp(f_sort_func, sort_set)),
        gsMakeDataAppl1(f_sort_func, d_sort_elt)),
      //subset or equal (sort_id -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(fgl, nil,
        gsMakeDataExprSubSetEq(
          gsMakeDataExprSetComp(f_sort_func, sort_set),
          gsMakeDataExprSetComp(g_sort_func, sort_set)), 
        gsMakeDataExprForall(imp_func)),
      //proper subset (sort_id -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(stl, nil,
        gsMakeDataExprSubSet(s_sort_id, t_sort_id),
        gsMakeDataExprAnd(
          gsMakeDataExprSubSetEq(s_sort_id, t_sort_id), 
          gsMakeDataExprNeq(s_sort_id, t_sort_id)
        )),
      //union (sort_id -> sort_id -> sort_id)
      (ATerm) gsMakeDataEqn(fgl, nil,
        gsMakeDataExprSetUnion(
          gsMakeDataExprSetComp(f_sort_func, sort_set),
          gsMakeDataExprSetComp(g_sort_func, sort_set)), 
        gsMakeDataExprSetComp(OrFunc, sort_set)),
      //difference (sort_id -> sort_id -> sort_id)
      (ATerm) gsMakeDataEqn(stl, nil,
        gsMakeDataExprSetDiff(s_sort_id, t_sort_id),
        gsMakeDataExprSetInterSect(s_sort_id, gsMakeDataExprSetCompl(t_sort_id))),
      //intersection (sort_id -> sort_id -> sort_id)
      (ATerm) gsMakeDataEqn(fgl, nil,
        gsMakeDataExprSetInterSect(
          gsMakeDataExprSetComp(f_sort_func, sort_set),
          gsMakeDataExprSetComp(g_sort_func, sort_set)), 
        gsMakeDataExprSetComp(and_func, sort_set)),
      //complement (sort_id -> sort_id)
      (ATerm) gsMakeDataEqn(fl, nil,
        gsMakeDataExprSetCompl(gsMakeDataExprSetComp(f_sort_func, sort_set)),
        gsMakeDataExprSetComp(not_func, sort_set)));
      //simplification of combinations of functions false, not, imp, and, or
      ////left unit of the or function
      //(ATerm) gsMakeDataEqn(fl, nil,
      //  gsMakeDataAppl2(gsGetDataExprHead(OrFunc), false_func, f_sort_func),
      //  f_sort_func),
      ////right unit of the or function
      //(ATerm) gsMakeDataEqn(fl, nil,
      //  gsMakeDataAppl2(gsGetDataExprHead(OrFunc), f_sort_func, false_func),
      //  f_sort_func)

  return new_data_eqns;
}

ATermAppl impl_sort_set(ATermAppl sort_set, ATermList *p_substs,
  t_data_decls *p_data_decls)
{
  assert(gsIsSortExprSet(sort_set));
  ATermAppl sort_elt = ATAgetArgument(sort_set, 1);

  //implement expressions in the target sort of sort_set
  //this needs to be done first to keep the substitutions sound!
  impl_exprs_appl(sort_elt, p_substs, p_data_decls);

  //declare fresh sort identifier for sort_set
  ATermAppl sort_id = make_fresh_set_sort_id((ATerm) p_data_decls->sorts);
  p_data_decls->sorts = ATinsert(p_data_decls->sorts, (ATerm) sort_id);
  //add substitution for sort_set
  ATermAppl subst = gsMakeSubst_Appl(sort_set, sort_id);
  *p_substs = gsAddSubstToSubsts(subst, *p_substs);

  //declare operations for sort sort_id
  ATermList new_ops = ATmakeList(9,
      (ATerm) gsMakeOpIdSetComp(sort_elt, sort_set),
      (ATerm) gsMakeOpIdEmptySet(sort_set),
      (ATerm) gsMakeOpIdEltIn(sort_elt, sort_set),
      (ATerm) gsMakeOpIdSubSetEq(sort_set),
      (ATerm) gsMakeOpIdSubSet(sort_set),
      (ATerm) gsMakeOpIdSetUnion(sort_set),
      (ATerm) gsMakeOpIdSetDiff(sort_set),
      (ATerm) gsMakeOpIdSetIntersect(sort_set),
      (ATerm) gsMakeOpIdSetCompl(sort_set));

  ATermList new_data_eqns = build_set_equations(sort_elt, sort_set);

  new_data_eqns = impl_exprs_list(new_data_eqns, p_substs, p_data_decls);

  //perform substitutions
  new_ops = gsSubstValues_List(*p_substs, new_ops, true);
  p_data_decls->ops = ATconcat(new_ops, p_data_decls->ops);
  new_data_eqns = gsSubstValues_List(*p_substs, new_data_eqns, true);
  p_data_decls->data_eqns = ATconcat(new_data_eqns, p_data_decls->data_eqns);

  return sort_id;
}

ATermList build_bag_equations(ATermAppl sort_elt, ATermAppl sort_bag, ATermAppl sort_set)
{
  //declare data equations for sort sort_id
  ATermAppl sort_func = gsMakeSortArrow1(sort_elt, gsMakeSortExprNat());
  ATermList el = ATmakeList0();
  ATermAppl s_sort_id = gsMakeDataVarId(gsString2ATermAppl("s"), sort_bag);
  ATermAppl t_sort_id = gsMakeDataVarId(gsString2ATermAppl("t"), sort_bag);
  ATermAppl f_sort_func = gsMakeDataVarId(gsString2ATermAppl("f"), sort_func);
  ATermAppl g_sort_func = gsMakeDataVarId(gsString2ATermAppl("g"), sort_func);
  ATermAppl d_sort_elt = gsMakeDataVarId(gsString2ATermAppl("d"), sort_elt);
  ATermAppl x_sort_elt = gsMakeDataVarId(gsString2ATermAppl("x"), sort_elt);
  ATermAppl y_sort_elt = gsMakeDataVarId(gsString2ATermAppl("y"), sort_elt);
  ATermAppl u_sort_set = gsMakeDataVarId(gsString2ATermAppl("u"), sort_set);
  ATermAppl m = gsMakeDataVarId(gsString2ATermAppl("m"), gsMakeSortExprNat());
  ATermAppl n = gsMakeDataVarId(gsString2ATermAppl("n"), gsMakeSortExprNat());
  ATermAppl nil = gsMakeNil();
  ATermAppl zero = gsMakeDataExprC0();
  ATermList sl = ATmakeList1((ATerm) s_sort_id);
  ATermList stl = ATmakeList2((ATerm) s_sort_id, (ATerm) t_sort_id);
  ATermList dsl = ATmakeList2((ATerm) d_sort_elt, (ATerm) s_sort_id);
  ATermList ul = ATmakeList1((ATerm) u_sort_set);
  ATermList dfl = ATmakeList2((ATerm) d_sort_elt, (ATerm) f_sort_func);
  ATermList fgl = ATmakeList2((ATerm) f_sort_func, (ATerm) g_sort_func);
  ATermAppl zero_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt), zero);
  ATermAppl lte_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt),
      gsMakeDataExprLTE(
        gsMakeDataAppl1(f_sort_func, x_sort_elt),
        gsMakeDataAppl1(g_sort_func, x_sort_elt)
      )
    );
  ATermAppl add_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt),
      gsMakeDataExprAdd(
        gsMakeDataAppl1(f_sort_func, x_sort_elt),
        gsMakeDataAppl1(g_sort_func, x_sort_elt)
      )
    );
  ATermAppl subt_max0_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) y_sort_elt),
      gsMakeDataAppl2(
        gsMakeBinder(gsMakeLambda(), ATmakeList2((ATerm) m, (ATerm) n),
          gsMakeDataExprIf(
            gsMakeDataExprGT(m, n), gsMakeDataExprGTESubt(m, n), zero
          )
        ),
        gsMakeDataAppl1(f_sort_func, y_sort_elt),
        gsMakeDataAppl1(g_sort_func, y_sort_elt)));
  /*
  ATermAppl subt_max0_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt),
      gsMakeWhr(
        gsMakeDataExprIf(
          gsMakeDataExprGT(m, n), gsMakeDataExprGTESubt(m, n), zero
        ), ATmakeList2(
          (ATerm) gsMakeDataVarIdInit(m, gsMakeDataAppl1(f_sort_func, x_sort_elt)),
          (ATerm) gsMakeDataVarIdInit(n, gsMakeDataAppl1(g_sort_func, x_sort_elt))
        )
      )
    );
  */
  ATermAppl min_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt),
      gsMakeDataExprMin(
        gsMakeDataAppl1(f_sort_func, x_sort_elt),
        gsMakeDataAppl1(g_sort_func, x_sort_elt)
      )
    );
  ATermAppl bag2set_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt),
      gsMakeDataExprEltIn(x_sort_elt, s_sort_id)
    );
  ATermAppl set2bag_func =
    gsMakeBinder(gsMakeLambda(), ATmakeList1((ATerm) x_sort_elt),
      gsMakeDataExprIf(
        gsMakeDataExprEltIn(x_sort_elt, u_sort_set),
        gsMakeDataExprNat_int(1),
        gsMakeDataExprNat_int(0)
      )
    );

  ATermList new_data_eqns = ATmakeList(11,
      //equality (sort_id -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(fgl, nil,
        gsMakeDataExprEq(
          gsMakeDataExprBagComp(f_sort_func, sort_bag),
          gsMakeDataExprBagComp(g_sort_func, sort_bag)), 
        gsMakeDataExprEq(f_sort_func, g_sort_func)),
      //empty bag (sort_id)
      (ATerm) gsMakeDataEqn(el, nil,
        gsMakeDataExprEmptyBag(sort_bag),
        gsMakeDataExprBagComp(zero_func, sort_bag)),
      //count (sort_elt -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(dfl, nil,
        gsMakeDataExprCount(d_sort_elt, gsMakeDataExprBagComp(f_sort_func, sort_bag)),
        gsMakeDataAppl1(f_sort_func, d_sort_elt)),
      //element test (sort_elt -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(dsl, nil,
        gsMakeDataExprEltIn(d_sort_elt, s_sort_id),
        gsMakeDataExprGT(gsMakeDataExprCount(d_sort_elt, s_sort_id), zero)),
      //subbag or equal (sort_id -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(fgl, nil,
        gsMakeDataExprSubBagEq(
          gsMakeDataExprBagComp(f_sort_func, sort_bag),
          gsMakeDataExprBagComp(g_sort_func, sort_bag)), 
        gsMakeDataExprForall(lte_func)),
      //proper subbag (sort_id -> sort_id -> Bool)
      (ATerm) gsMakeDataEqn(stl, nil,
        gsMakeDataExprSubBag(s_sort_id, t_sort_id),
        gsMakeDataExprAnd(
          gsMakeDataExprSubBagEq(s_sort_id, t_sort_id), 
          gsMakeDataExprNeq(s_sort_id, t_sort_id)
        )),
      //union (sort_id -> sort_id -> sort_id)
      (ATerm) gsMakeDataEqn(fgl, nil,
        gsMakeDataExprBagUnion(
          gsMakeDataExprBagComp(f_sort_func, sort_bag),
          gsMakeDataExprBagComp(g_sort_func, sort_bag)), 
        gsMakeDataExprBagComp(add_func, sort_bag)),
      //difference (sort_id -> sort_id -> sort_id)
      (ATerm) gsMakeDataEqn(fgl, nil,
        gsMakeDataExprBagDiff(
          gsMakeDataExprBagComp(f_sort_func, sort_bag),
          gsMakeDataExprBagComp(g_sort_func, sort_bag)), 
        gsMakeDataExprBagComp(subt_max0_func, sort_bag)),
      //intersection (sort_id -> sort_id -> sort_id)
      (ATerm) gsMakeDataEqn(fgl, nil,
        gsMakeDataExprBagInterSect(
          gsMakeDataExprBagComp(f_sort_func, sort_bag),
          gsMakeDataExprBagComp(g_sort_func, sort_bag)), 
        gsMakeDataExprBagComp(min_func, sort_bag)),
      //Bag2Set (sort_id -> sort_set)
      (ATerm) gsMakeDataEqn(sl, nil,
        gsMakeDataExprBag2Set(s_sort_id, sort_set),
        gsMakeDataExprSetComp(bag2set_func, sort_set)),
      //Set2Bag (sort_set -> sort_id)
      (ATerm) gsMakeDataEqn(ul, nil,
        gsMakeDataExprSet2Bag(u_sort_set, sort_bag),
        gsMakeDataExprBagComp(set2bag_func, sort_bag)));

  return new_data_eqns;
}

ATermAppl impl_sort_bag(ATermAppl sort_bag, ATermList *p_substs,
  t_data_decls *p_data_decls)
{
  assert(gsIsSortExprBag(sort_bag));
  ATermAppl sort_elt = ATAgetArgument(sort_bag, 1);
  ATermAppl sort_set = gsMakeSortExprSet(sort_elt);

  //implement expressions in the target sort of sort_bag
  //this needs to be done first in order to keep the substitutions sound!
  impl_exprs_appl(sort_elt, p_substs, p_data_decls);
  //add implementation of sort Set(sort_elt), if necessary
  ATermAppl sort_set_impl =
    (ATermAppl) gsSubstValues(*p_substs, (ATerm) sort_set, false);
  if (ATisEqual(sort_set_impl, sort_set)) {
    //Set(sort_elt) is not implemented yet, because it does not occur as an lhs
    //in the list of substitutions in *p_substs
    sort_set_impl = impl_sort_set(sort_set, p_substs, p_data_decls);
  }
  //declare fresh sort identifier for sort_bag
  ATermAppl sort_id = make_fresh_bag_sort_id((ATerm) p_data_decls->sorts);
  p_data_decls->sorts = ATinsert(p_data_decls->sorts, (ATerm) sort_id);
  //add substitution for this identifier
  ATermAppl subst = gsMakeSubst_Appl(sort_bag, sort_id);
  *p_substs = gsAddSubstToSubsts(subst, *p_substs);

  //declare operations for sort sort_id
  ATermList new_ops = ATmakeList(11,
      (ATerm) gsMakeOpIdBagComp(sort_elt, sort_bag),
      (ATerm) gsMakeOpIdEmptyBag(sort_bag),
      (ATerm) gsMakeOpIdCount(sort_elt, sort_bag),
      (ATerm) gsMakeOpIdEltIn(sort_elt, sort_bag),
      (ATerm) gsMakeOpIdSubBagEq(sort_bag),
      (ATerm) gsMakeOpIdSubBag(sort_bag),
      (ATerm) gsMakeOpIdBagUnion(sort_bag),
      (ATerm) gsMakeOpIdBagDiff(sort_bag),
      (ATerm) gsMakeOpIdBagIntersect(sort_bag),
      (ATerm) gsMakeOpIdBag2Set(sort_bag, sort_set),
      (ATerm) gsMakeOpIdSet2Bag(sort_set, sort_bag));

  ATermList new_data_eqns = build_bag_equations(sort_elt, sort_bag, sort_set);

  new_data_eqns = impl_exprs_list(new_data_eqns, p_substs, p_data_decls);

  //perform substitutions
  new_ops = gsSubstValues_List(*p_substs, new_ops, true);
  p_data_decls->ops = ATconcat(new_ops, p_data_decls->ops);
  new_data_eqns = gsSubstValues_List(*p_substs, new_data_eqns, true);
  p_data_decls->data_eqns = ATconcat(new_data_eqns, p_data_decls->data_eqns);

  //add implementation of sort Nat, if necessary
  if (ATindexOf(p_data_decls->sorts, (ATerm) gsMakeSortIdNat(), 0) == -1) {
    impl_sort_nat(p_data_decls);
  }
  return sort_id;
}

void impl_sort_bool(t_data_decls *p_data_decls)
{
  //Declare sort Bool
  p_data_decls->sorts = ATinsert(p_data_decls->sorts, (ATerm) gsMakeSortIdBool());
  //Declare constructors for sort Bool
  p_data_decls->cons_ops = ATconcat(ATmakeList2(
      (ATerm) gsMakeOpIdTrue(),
      (ATerm) gsMakeOpIdFalse()
    ), p_data_decls->cons_ops);
  //Declare operations for sort Bool
  ATermAppl se_bool = gsMakeSortExprBool();
  p_data_decls->ops = ATconcat(ATmakeList(4,
      (ATerm) gsMakeOpIdNot(),
      (ATerm) gsMakeOpIdAnd(),
      (ATerm) gsMakeOpIdOr(),
      (ATerm) gsMakeOpIdImp()
    ), p_data_decls->ops);
  //Declare data equations for sort Bool
  ATermList el = ATmakeList0();
  ATermAppl nil = gsMakeNil();
  ATermAppl t = gsMakeDataExprTrue();
  ATermAppl f = gsMakeDataExprFalse();
  ATermAppl b = gsMakeDataVarId(gsString2ATermAppl("b"), se_bool);
  ATermList bl = ATmakeList1((ATerm) b);
  p_data_decls->data_eqns = ATconcat(ATmakeList(19,
      //logical negation (Bool -> Bool)
      (ATerm) gsMakeDataEqn(el, nil, gsMakeDataExprNot(t), f),
      (ATerm) gsMakeDataEqn(el, nil, gsMakeDataExprNot(f), t),
      (ATerm) gsMakeDataEqn(bl, nil,
                     gsMakeDataExprNot(gsMakeDataExprNot(b)), b),
      //conjunction (Bool -> Bool -> Bool)
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprAnd(b, t), b),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprAnd(b, f), f),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprAnd(t, b), b),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprAnd(f, b), f),
      //disjunction (Bool -> Bool -> Bool)
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprOr(b, t), t),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprOr(b, f), b),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprOr(t, b), t),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprOr(f, b), b),
      //implication (Bool -> Bool -> Bool)
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprImp(b, t), t),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprImp(b, f),
                                            gsMakeDataExprNot(b)),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprImp(t, b), b),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprImp(f, b), t),
      //equality (Bool -> Bool -> Bool)
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprEq(t, b), b),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprEq(f, b), gsMakeDataExprNot(b)),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprEq(b, t), b),
      (ATerm) gsMakeDataEqn(bl, nil, gsMakeDataExprEq(b, f), gsMakeDataExprNot(b))
    ), p_data_decls->data_eqns);
}

void impl_sort_pos(t_data_decls *p_data_decls)
{
  //Declare sort Pos
  p_data_decls->sorts = ATinsert(p_data_decls->sorts, (ATerm) gsMakeSortIdPos());
  //Declare constructors for sort Pos
  p_data_decls->cons_ops = ATconcat(ATmakeList2(
      (ATerm) gsMakeOpIdC1(),
      (ATerm) gsMakeOpIdCDub()
    ), p_data_decls->cons_ops);
  //Declare operations for sort Pos
  ATermAppl se_pos = gsMakeSortExprPos();
  ATermAppl se_bool = gsMakeSortExprBool();
  p_data_decls->ops = ATconcat(ATmakeList(12,
      (ATerm) gsMakeOpIdLTE(se_pos),
      (ATerm) gsMakeOpIdLT(se_pos),
      (ATerm) gsMakeOpIdGTE(se_pos),
      (ATerm) gsMakeOpIdGT(se_pos),
      (ATerm) gsMakeOpIdMax(se_pos, se_pos),
      (ATerm) gsMakeOpIdMin(se_pos),
      (ATerm) gsMakeOpIdAbs(se_pos),
      (ATerm) gsMakeOpIdSucc(se_pos),
      (ATerm) gsMakeOpIdAdd(se_pos, se_pos),
      (ATerm) gsMakeOpIdAddC(),
      (ATerm) gsMakeOpIdMult(se_pos),
      (ATerm) gsMakeOpIdMultIR()
    ), p_data_decls->ops);
  //Declare data equations for sort Pos
  ATermList el = ATmakeList0();
  ATermAppl nil = gsMakeNil();
  ATermAppl t = gsMakeDataExprTrue();
  ATermAppl f = gsMakeDataExprFalse();
  ATermAppl one = gsMakeDataExprC1();
  ATermAppl two = gsMakeDataExprCDub(f, one);
  ATermAppl p = gsMakeDataVarId(gsString2ATermAppl("p"), se_pos);
  ATermAppl q = gsMakeDataVarId(gsString2ATermAppl("q"), se_pos);
  ATermAppl r = gsMakeDataVarId(gsString2ATermAppl("r"), se_pos);
  ATermAppl b = gsMakeDataVarId(gsString2ATermAppl("b"), se_bool);
  ATermAppl c = gsMakeDataVarId(gsString2ATermAppl("c"), se_bool);
  ATermList pl = ATmakeList1((ATerm) p);
  ATermList pql = ATmakeList2((ATerm) p, (ATerm) q);
  ATermList pqrl = ATmakeList3((ATerm) p, (ATerm) q, (ATerm) r);
  ATermList bpl = ATmakeList2((ATerm) b, (ATerm) p);
  ATermList bpql = ATmakeList3((ATerm) b, (ATerm) p, (ATerm) q);
  ATermList bcpql = ATmakeList4((ATerm) b, (ATerm) c, (ATerm) p, (ATerm) q);
  ATermList bpqrl = ATmakeList4((ATerm) b, (ATerm) p, (ATerm) q, (ATerm) r);
  p_data_decls->data_eqns = ATconcat(ATmakeList(39,
      //equality (Pos -> Pos -> Bool)
      (ATerm) gsMakeDataEqn(bpl, nil, 
         gsMakeDataExprEq(one, gsMakeDataExprCDub(b, p)), f),
      (ATerm) gsMakeDataEqn(bpl, nil, 
         gsMakeDataExprEq(gsMakeDataExprCDub(b, p), one), f),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprEq(gsMakeDataExprCDub(f, p), gsMakeDataExprCDub(t, q)),
         f),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprEq(gsMakeDataExprCDub(t, p), gsMakeDataExprCDub(f, q)),
         f),
      (ATerm) gsMakeDataEqn(bpql,nil, 
         gsMakeDataExprEq(gsMakeDataExprCDub(b, p), gsMakeDataExprCDub(b, q)),
         gsMakeDataExprEq(p, q)),
      (ATerm) gsMakeDataEqn(bcpql,nil, 
         gsMakeDataExprEq(gsMakeDataExprCDub(b, p), gsMakeDataExprCDub(c, q)),
         gsMakeDataExprAnd(gsMakeDataExprEq(b, c), gsMakeDataExprEq(p, q))),
      //less than or equal (Pos -> Pos -> Bool)
      (ATerm) gsMakeDataEqn(pl, nil, gsMakeDataExprLTE(one, p), t),
      (ATerm) gsMakeDataEqn(bpl, nil, 
         gsMakeDataExprLTE(gsMakeDataExprCDub(b, p), one), f),
      (ATerm) gsMakeDataEqn(bpql,nil, 
         gsMakeDataExprLTE(gsMakeDataExprCDub(b, p), gsMakeDataExprCDub(b, q)),
         gsMakeDataExprLTE(p, q)),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprLTE(gsMakeDataExprCDub(f, p), gsMakeDataExprCDub(t, q)),
         gsMakeDataExprLTE(p, q)),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprLTE(gsMakeDataExprCDub(t, p), gsMakeDataExprCDub(f, q)),
         gsMakeDataExprLT(p, q)),
      //less than (Pos -> Pos -> Bool)
      (ATerm) gsMakeDataEqn(pl, nil, gsMakeDataExprLT(p, one), f),
      (ATerm) gsMakeDataEqn(bpl, nil, 
         gsMakeDataExprLT(one, gsMakeDataExprCDub(b, p)), t),
      (ATerm) gsMakeDataEqn(bpql,nil, 
         gsMakeDataExprLT(gsMakeDataExprCDub(b, p), gsMakeDataExprCDub(b, q)),
         gsMakeDataExprLT(p, q)),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprLT(gsMakeDataExprCDub(f, p), gsMakeDataExprCDub(t, q)),
         gsMakeDataExprLTE(p, q)),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprLT(gsMakeDataExprCDub(t, p), gsMakeDataExprCDub(f, q)),
         gsMakeDataExprLT(p, q)),
      //greater than or equal (Pos -> Pos -> Bool)
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprGTE(p, q), gsMakeDataExprLTE(q, p)),
      //greater than (Pos -> Pos -> Bool)
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprGT(p, q), gsMakeDataExprLT(q, p)),
      //maximum (Pos -> Pos -> Pos)
      (ATerm) gsMakeDataEqn(pql,nil, gsMakeDataExprMax(p, q),
         gsMakeDataExprIf(gsMakeDataExprLTE(p, q), q, p)),
      //minimum (Pos -> Pos -> Pos)
      (ATerm) gsMakeDataEqn(pql,nil, gsMakeDataExprMin(p, q),
         gsMakeDataExprIf(gsMakeDataExprLTE(p, q), p, q)),
      //absolute value (Pos -> Pos)
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprAbs(p), p),
      //successor (Pos -> Pos)
      (ATerm) gsMakeDataEqn(el,nil,
         gsMakeDataExprSucc(one), two),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprSucc(gsMakeDataExprCDub(f, p)),
         gsMakeDataExprCDub(t, p)),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprSucc(gsMakeDataExprCDub(t, p)),
         gsMakeDataExprCDub(f, gsMakeDataExprSucc(p))),
      //addition (Pos -> Pos -> Pos)
      (ATerm) gsMakeDataEqn(pql, nil, gsMakeDataExprAdd(p, q),
         gsMakeDataExprAddC(f, p, q)),
      //addition with carry (Bool -> Pos -> Pos -> Pos)
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprAddC(f, one, p), gsMakeDataExprSucc(p)),
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprAddC(t, one, p),
         gsMakeDataExprSucc(gsMakeDataExprSucc(p))),
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprAddC(f, p, one), gsMakeDataExprSucc(p)),
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprAddC(t, p, one),
         gsMakeDataExprSucc(gsMakeDataExprSucc(p))),
      (ATerm) gsMakeDataEqn(bcpql,nil, gsMakeDataExprAddC(
           b, gsMakeDataExprCDub(c, p), gsMakeDataExprCDub(c, q)),
         gsMakeDataExprCDub(b, gsMakeDataExprAddC(c, p, q))),
      (ATerm) gsMakeDataEqn(bpql,nil, gsMakeDataExprAddC(
           b, gsMakeDataExprCDub(f, p), gsMakeDataExprCDub(t, q)),
         gsMakeDataExprCDub(gsMakeDataExprNot(b), gsMakeDataExprAddC(b, p, q))),
      (ATerm) gsMakeDataEqn(bpql,nil, gsMakeDataExprAddC(
           b, gsMakeDataExprCDub(t, p), gsMakeDataExprCDub(f, q)),
         gsMakeDataExprCDub(gsMakeDataExprNot(b), gsMakeDataExprAddC(b, p, q))),
      //multiplication (Pos -> Pos -> Pos)
      (ATerm) gsMakeDataEqn(pql,
         gsMakeDataExprLTE(p, q),
         gsMakeDataExprMult(p, q),
         gsMakeDataExprMultIR(f, one, p, q)),
      (ATerm) gsMakeDataEqn(pql,
         //gsMakeDataExprGT(p, q),
         gsMakeDataExprLT(q, p),
         gsMakeDataExprMult(p, q),
         gsMakeDataExprMultIR(f, one, q, p)),
      //multiplication with intermediate result
      //  (Bool -> Pos -> Pos -> Pos -> Pos)
      (ATerm) gsMakeDataEqn(pql, nil,
         gsMakeDataExprMultIR(f, p, one, q), q),
      (ATerm) gsMakeDataEqn(pql, nil,
         gsMakeDataExprMultIR(t, p, one, q), gsMakeDataExprAddC(f, p, q)),
      (ATerm) gsMakeDataEqn(bpqrl,nil, 
         gsMakeDataExprMultIR(b, p, gsMakeDataExprCDub(f, q), r),
         gsMakeDataExprMultIR(b, p, q, gsMakeDataExprCDub(f, r))),
      (ATerm) gsMakeDataEqn(pqrl,nil, 
         gsMakeDataExprMultIR(f, p, gsMakeDataExprCDub(t, q), r),
         gsMakeDataExprMultIR(t, r, q, gsMakeDataExprCDub(f, r))),
      (ATerm) gsMakeDataEqn(pqrl,nil, 
         gsMakeDataExprMultIR(t, p, gsMakeDataExprCDub(t, q), r),
         gsMakeDataExprMultIR(t, gsMakeDataExprAddC(f, p, r), q,
           gsMakeDataExprCDub(f, r)))
    ), p_data_decls->data_eqns);
}

void impl_sort_nat(t_data_decls *p_data_decls)
{
  //add implementation of sort NatPair, if necessary
  if (ATindexOf(p_data_decls->sorts, (ATerm) gsMakeSortIdNatPair(), 0) == -1) {
    impl_sort_nat_pair(p_data_decls);
  }
  //Declare sort Nat
  p_data_decls->sorts = ATinsert(p_data_decls->sorts, (ATerm) gsMakeSortIdNat());
  //Declare constructors for sort Nat
  p_data_decls->cons_ops = ATconcat(ATmakeList2(
      (ATerm) gsMakeOpIdC0(),
      (ATerm) gsMakeOpIdCNat()
    ), p_data_decls->cons_ops);
  //Declare operations for sort Nat
  ATermAppl se_nat = gsMakeSortExprNat();
  ATermAppl se_pos = gsMakeSortExprPos();
  ATermAppl se_bool = gsMakeSortExprBool();
  p_data_decls->ops = ATconcat(ATmakeList(28,
      (ATerm) gsMakeOpIdPos2Nat(),
      (ATerm) gsMakeOpIdNat2Pos(),
      (ATerm) gsMakeOpIdLTE(se_nat),
      (ATerm) gsMakeOpIdLT(se_nat),
      (ATerm) gsMakeOpIdGTE(se_nat),
      (ATerm) gsMakeOpIdGT(se_nat),
      (ATerm) gsMakeOpIdMax(se_pos, se_nat),
      (ATerm) gsMakeOpIdMax(se_nat, se_pos),
      (ATerm) gsMakeOpIdMax(se_nat, se_nat),
      (ATerm) gsMakeOpIdMin(se_nat),
      (ATerm) gsMakeOpIdAbs(se_nat),
      (ATerm) gsMakeOpIdSucc(se_nat),
      (ATerm) gsMakeOpIdPred(se_pos),
      (ATerm) gsMakeOpIdDub(se_nat),
      (ATerm) gsMakeOpIdAdd(se_pos, se_nat),
      (ATerm) gsMakeOpIdAdd(se_nat, se_pos),
      (ATerm) gsMakeOpIdAdd(se_nat, se_nat),
      (ATerm) gsMakeOpIdGTESubt(se_pos),
      (ATerm) gsMakeOpIdGTESubt(se_nat),
      (ATerm) gsMakeOpIdGTESubtB(),
      (ATerm) gsMakeOpIdMult(se_nat),
      (ATerm) gsMakeOpIdDiv(se_pos),
      (ATerm) gsMakeOpIdDiv(se_nat),
      (ATerm) gsMakeOpIdMod(se_pos),
      (ATerm) gsMakeOpIdMod(se_nat),
      (ATerm) gsMakeOpIdExp(se_pos),
      (ATerm) gsMakeOpIdExp(se_nat),
      (ATerm) gsMakeOpIdEven()
    ), p_data_decls->ops);
  //Declare data equations for sort Nat
  ATermList el = ATmakeList0();
  ATermAppl nil = gsMakeNil();
  ATermAppl t = gsMakeDataExprTrue();
  ATermAppl f = gsMakeDataExprFalse();
  ATermAppl zero = gsMakeDataExprC0();
  ATermAppl one = gsMakeDataExprC1();
  ATermAppl two = gsMakeDataExprCDub(f, one);
  ATermAppl b = gsMakeDataVarId(gsString2ATermAppl("b"), se_bool);
  ATermAppl c = gsMakeDataVarId(gsString2ATermAppl("c"), se_bool);
  ATermAppl p = gsMakeDataVarId(gsString2ATermAppl("p"), se_pos);
  ATermAppl q = gsMakeDataVarId(gsString2ATermAppl("q"), se_pos);
  ATermAppl n = gsMakeDataVarId(gsString2ATermAppl("n"), se_nat);
  ATermAppl m = gsMakeDataVarId(gsString2ATermAppl("m"), se_nat);
  ATermList pl = ATmakeList1((ATerm) p);
  ATermList bpl = ATmakeList2((ATerm) b, (ATerm) p);
  ATermList pql = ATmakeList2((ATerm) p, (ATerm) q);
  ATermList bpql = ATmakeList3((ATerm) b, (ATerm) p, (ATerm) q);
  ATermList bcpql = ATmakeList4((ATerm) b, (ATerm) c, (ATerm) p, (ATerm) q);
  ATermList pnl = ATmakeList2((ATerm) p, (ATerm) n);
  ATermList nl = ATmakeList1((ATerm) n);
  ATermList mnl = ATmakeList2((ATerm) m, (ATerm) n);
  p_data_decls->data_eqns = ATconcat(ATmakeList(75,
      //equality (Nat -> Nat -> Bool)
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprEq(zero, gsMakeDataExprCNat(p)), f),
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprEq(gsMakeDataExprCNat(p), zero), f),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprEq(gsMakeDataExprCNat(p), gsMakeDataExprCNat(q)),
         gsMakeDataExprEq(p, q)),
      //convert Pos to Nat (Pos -> Nat)
      (ATerm) gsMakeDataEqn(el,nil, gsMakeOpIdPos2Nat(), gsMakeOpIdCNat()),
      //convert Nat to Pos (Nat -> Pos)
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprNat2Pos(gsMakeDataExprCNat(p)), p),
      //less than or equal (Nat -> Nat -> Bool)
      (ATerm) gsMakeDataEqn(nl, nil, gsMakeDataExprLTE(zero, n), t),
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprLTE(gsMakeDataExprCNat(p), zero), f),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprLTE(gsMakeDataExprCNat(p), gsMakeDataExprCNat(q)),
         gsMakeDataExprLTE(p, q)),
      //less than (Nat -> Nat -> Bool)
      (ATerm) gsMakeDataEqn(nl, nil, gsMakeDataExprLT(n, zero), f),
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprLT(zero, gsMakeDataExprCNat(p)), t),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprLT(gsMakeDataExprCNat(p), gsMakeDataExprCNat(q)),
         gsMakeDataExprLT(p, q)),
      //greater than or equal (Nat -> Nat -> Bool)
      (ATerm) gsMakeDataEqn(mnl,nil,
         gsMakeDataExprGTE(m, n), gsMakeDataExprLTE(n, m)),
      //greater than (Nat -> Nat -> Bool)
      (ATerm) gsMakeDataEqn(mnl,nil,
         gsMakeDataExprGT(m, n), gsMakeDataExprLT(n, m)),
      //maximum (Pos -> Nat -> Pos)
      (ATerm) gsMakeDataEqn(pl,nil, gsMakeDataExprMax(p, zero), p),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprMax(p, gsMakeDataExprCNat(q)),
         //gsMakeDataExprMax(p, q)),
         gsMakeDataExprIf(gsMakeDataExprLTE(p, q), q, p)),
      //maximum (Nat -> Pos -> Pos)
      (ATerm) gsMakeDataEqn(pl,nil, gsMakeDataExprMax(zero, p), p),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprMax(gsMakeDataExprCNat(p), q),
         //gsMakeDataExprMax(p, q)),
         gsMakeDataExprIf(gsMakeDataExprLTE(p, q), q, p)),
      //maximum (Nat -> Nat -> Nat)
      (ATerm) gsMakeDataEqn(mnl,nil, gsMakeDataExprMax(m, n),
         gsMakeDataExprIf(gsMakeDataExprLTE(m, n), n, m)),
      //minimum (Nat -> Nat -> Nat)
      (ATerm) gsMakeDataEqn(mnl,nil, gsMakeDataExprMin(m, n),
         gsMakeDataExprIf(gsMakeDataExprLTE(m, n), m, n)),
      //absolute value (Nat -> Nat)
      (ATerm) gsMakeDataEqn(nl,nil,
         gsMakeDataExprAbs(n), n),
      //successor (Nat -> Pos)
      (ATerm) gsMakeDataEqn(el,nil, gsMakeDataExprSucc(zero), one),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprSucc(gsMakeDataExprCNat(p)), gsMakeDataExprSucc(p)),
      (ATerm) gsMakeDataEqn(nl,nil,
         gsMakeDataExprSucc(gsMakeDataExprSucc(n)),
         gsMakeDataExprCDub(
           gsMakeDataExprEq(gsMakeDataExprMod(n, two), gsMakeDataExprCNat(one)),
           gsMakeDataExprSucc(gsMakeDataExprDiv(n, two)))),
      //predecessor (Pos -> Nat)
      (ATerm) gsMakeDataEqn(el,nil, gsMakeDataExprPred(one), zero),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprPred(gsMakeDataExprCDub(t, p)),
         gsMakeDataExprCNat(gsMakeDataExprCDub(f, p))),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprPred(gsMakeDataExprCDub(f, p)),
         gsMakeDataExprDub(t, gsMakeDataExprPred(p))),
      //double (Bool -> Nat -> Nat)
      (ATerm) gsMakeDataEqn(el,nil, gsMakeDataExprDub(f, zero), zero),
      (ATerm) gsMakeDataEqn(el,nil,
         gsMakeDataExprDub(t, zero), gsMakeDataExprCNat(one)),
      (ATerm) gsMakeDataEqn(bpl,nil,
         gsMakeDataExprDub(b, gsMakeDataExprCNat(p)),
         gsMakeDataExprCNat(gsMakeDataExprCDub(b, p))),
      //addition (Pos -> Nat -> Pos)
      (ATerm) gsMakeDataEqn(pl, nil, gsMakeDataExprAdd(p, zero), p),
      (ATerm) gsMakeDataEqn(pql, nil,
        gsMakeDataExprAdd(p, gsMakeDataExprCNat(q)),
        //gsMakeDataExprAdd(p, q)),
        gsMakeDataExprAddC(f, p, q)),
      //addition (Nat -> Pos -> Pos)
      (ATerm) gsMakeDataEqn(pl, nil, gsMakeDataExprAdd(zero, p), p),
      (ATerm) gsMakeDataEqn(pql, nil,
        gsMakeDataExprAdd(gsMakeDataExprCNat(p), q),
        //gsMakeDataExprAdd(p, q)),
        gsMakeDataExprAddC(f, p, q)),
      //addition (Nat -> Nat -> Nat)
      (ATerm) gsMakeDataEqn(nl, nil, gsMakeDataExprAdd(zero, n), n),
      (ATerm) gsMakeDataEqn(nl, nil, gsMakeDataExprAdd(n, zero), n),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprAdd(gsMakeDataExprCNat(p), gsMakeDataExprCNat(q)),
         gsMakeDataExprCNat(gsMakeDataExprAddC(f, p, q))),
      //GTE subtraction (Pos -> Pos -> Nat)
      (ATerm) gsMakeDataEqn(pql, nil,
         gsMakeDataExprGTESubt(p, q),
         gsMakeDataExprGTESubtB(f, p, q)),
      //GTE subtraction (Nat -> Nat -> Nat)
      (ATerm) gsMakeDataEqn(nl, nil, gsMakeDataExprGTESubt(n, zero), n),
      (ATerm) gsMakeDataEqn(pql, nil,
         gsMakeDataExprGTESubt(gsMakeDataExprCNat(p), gsMakeDataExprCNat(q)),
         //gsMakeDataExprGTESubt(p, q)),
         gsMakeDataExprGTESubtB(f, p, q)),
      //GTE subtraction with borrow (Bool -> Pos -> Pos -> Nat)
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprGTESubtB(f, p, one),
         gsMakeDataExprPred(p)),
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprGTESubtB(t, p, one),
         gsMakeDataExprPred(gsMakeDataExprNat2Pos(gsMakeDataExprPred(p)))),
      (ATerm) gsMakeDataEqn(bcpql, nil,
         gsMakeDataExprGTESubtB(b, gsMakeDataExprCDub(c, p),
           gsMakeDataExprCDub(c, q)),
         gsMakeDataExprDub(b, gsMakeDataExprGTESubtB(b, p, q))),
      (ATerm) gsMakeDataEqn(bpql, nil,
         gsMakeDataExprGTESubtB(b, gsMakeDataExprCDub(f, p),
           gsMakeDataExprCDub(t, q)),
         gsMakeDataExprDub(gsMakeDataExprNot(b),
           gsMakeDataExprGTESubtB(t, p, q))),
      (ATerm) gsMakeDataEqn(bpql, nil,
         gsMakeDataExprGTESubtB(b, gsMakeDataExprCDub(t, p),
           gsMakeDataExprCDub(f, q)),
         gsMakeDataExprDub(gsMakeDataExprNot(b),
           gsMakeDataExprGTESubtB(f, p, q))),
      //multiplication (Nat -> Nat -> Nat)
      (ATerm) gsMakeDataEqn(nl, nil, gsMakeDataExprMult(zero, n), zero),
      (ATerm) gsMakeDataEqn(nl, nil, gsMakeDataExprMult(n, zero), zero),
      (ATerm) gsMakeDataEqn(pql,nil, 
         gsMakeDataExprMult(gsMakeDataExprCNat(p), gsMakeDataExprCNat(q)),
         gsMakeDataExprCNat(gsMakeDataExprMult(p, q))),
      //exponentiation (Pos -> Nat -> Pos)
      (ATerm) gsMakeDataEqn(pl, nil, gsMakeDataExprExp(p, zero), one),
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprExp(p, gsMakeDataExprCNat(one)), p),
      (ATerm) gsMakeDataEqn(pql,nil, 
         gsMakeDataExprExp(p, gsMakeDataExprCNat(gsMakeDataExprCDub(f, q))),
         gsMakeDataExprExp(gsMakeDataExprMultIR(f, one, p, p),
           gsMakeDataExprCNat(q))),
      (ATerm) gsMakeDataEqn(pql,nil, 
         gsMakeDataExprExp(p, gsMakeDataExprCNat(gsMakeDataExprCDub(t, q))),
         gsMakeDataExprMultIR(f, one, p,
           gsMakeDataExprExp(gsMakeDataExprMultIR(f, one, p, p),
             gsMakeDataExprCNat(q)))),
      //exponentiation (Nat -> Nat -> Nat)
      (ATerm) gsMakeDataEqn(nl, nil, gsMakeDataExprExp(n, zero),
        gsMakeDataExprCNat(one)),
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprExp(zero, gsMakeDataExprCNat(p)), zero),
      (ATerm) gsMakeDataEqn(pnl, nil,
         gsMakeDataExprExp(gsMakeDataExprCNat(p), n),
         gsMakeDataExprCNat(gsMakeDataExprExp(p, n))),
      //even (Nat -> Bool)
      (ATerm) gsMakeDataEqn(el, nil, gsMakeDataExprEven(zero), t),
      (ATerm) gsMakeDataEqn(pl, nil,
        gsMakeDataExprEven(gsMakeDataExprCNat(one)), f),
      (ATerm) gsMakeDataEqn(bpl, nil,
        gsMakeDataExprEven(gsMakeDataExprCNat(gsMakeDataExprCDub(b, p))),
        gsMakeDataExprNot(b)),
      //quotient after division (Pos -> Pos -> Nat)
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprDiv(p, one),
         gsMakeDataExprCNat(p)),
      (ATerm) gsMakeDataEqn(bpl, nil,
         gsMakeDataExprDiv(one, gsMakeDataExprCDub(b, p)),
         zero),
      (ATerm) gsMakeDataEqn(bpql, nil,
         gsMakeDataExprDiv(gsMakeDataExprCDub(b, p), gsMakeDataExprCDub(f, q)),
         gsMakeDataExprDiv(p, q)),
      (ATerm) gsMakeDataEqn(pql,
         gsMakeDataExprLTE(p, q),
         gsMakeDataExprDiv(gsMakeDataExprCDub(f, p), gsMakeDataExprCDub(t, q)),
         zero),
      (ATerm) gsMakeDataEqn(pql,
         gsMakeDataExprLT(q, p),
         gsMakeDataExprDiv(gsMakeDataExprCDub(f, p), gsMakeDataExprCDub(t, q)),
         gsMakeDataExprFirst(
           gsMakeDataExprGDivMod(
             gsMakeDataExprDivMod(p, gsMakeDataExprCDub(t, q)),
             f,
             gsMakeDataExprCDub(t, q)
           )
           //XXX The line above is the normal form of the line below, needed
           //for consistency with lpsrewr.
           //gsMakeDataExprDivMod(gsMakeDataExprCDub(f, p), gsMakeDataExprCDub(t, q))
         )),
      (ATerm) gsMakeDataEqn(pql,
         gsMakeDataExprLTE(p, q),
         gsMakeDataExprDiv(gsMakeDataExprCDub(t, p), gsMakeDataExprCDub(t, q)),
         gsMakeDataExprIf(gsMakeDataExprEq(p, q),
           gsMakeDataExprCNat(one), zero
         )),
      (ATerm) gsMakeDataEqn(pql,
         gsMakeDataExprLT(q, p),
         gsMakeDataExprDiv(gsMakeDataExprCDub(t, p), gsMakeDataExprCDub(t, q)),
         gsMakeDataExprFirst(
           gsMakeDataExprGDivMod(
             gsMakeDataExprDivMod(p, gsMakeDataExprCDub(t, q)),
             t,
             gsMakeDataExprCDub(t, q)
           )
           //XXX The line above is the normal form of the line below, needed
           //for consistency with lpsrewr.
           //gsMakeDataExprDivMod(gsMakeDataExprCDub(t, p), gsMakeDataExprCDub(t, q))
         )),
      //quotient after division (Nat -> Pos -> Nat)
      (ATerm) gsMakeDataEqn(pl, nil, gsMakeDataExprDiv(zero, p), zero),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprDiv(gsMakeDataExprCNat(p), q),
         gsMakeDataExprDiv(p, q)),
      //remainder after division (Pos -> Pos -> Nat)
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprMod(p, one),
         zero),
      (ATerm) gsMakeDataEqn(bpl, nil,
         gsMakeDataExprMod(one, gsMakeDataExprCDub(b, p)),
         gsMakeDataExprCNat(one)),
      (ATerm) gsMakeDataEqn(bpql, nil,
         gsMakeDataExprMod(gsMakeDataExprCDub(b, p), gsMakeDataExprCDub(f, q)),
         gsMakeDataExprDub(b, gsMakeDataExprMod(p, q))),
      (ATerm) gsMakeDataEqn(pql,
         gsMakeDataExprLTE(p, q),
         gsMakeDataExprMod(gsMakeDataExprCDub(f, p), gsMakeDataExprCDub(t, q)),
         gsMakeDataExprCNat(gsMakeDataExprCDub(f, p))),
      (ATerm) gsMakeDataEqn(pql,
         gsMakeDataExprLT(q, p),
         gsMakeDataExprMod(gsMakeDataExprCDub(f, p), gsMakeDataExprCDub(t, q)),
         gsMakeDataExprLast(
           gsMakeDataExprGDivMod(
             gsMakeDataExprDivMod(p, gsMakeDataExprCDub(t, q)),
             f,
             gsMakeDataExprCDub(t, q)
           )
           //XXX The line above is the normal form of the line below, needed
           //for consistency with lpsrewr.
           //gsMakeDataExprDivMod(gsMakeDataExprCDub(f, p), gsMakeDataExprCDub(t, q))
         )),
      (ATerm) gsMakeDataEqn(pql,
         gsMakeDataExprLTE(p, q),
         gsMakeDataExprMod(gsMakeDataExprCDub(t, p), gsMakeDataExprCDub(t, q)),
         gsMakeDataExprIf(gsMakeDataExprEq(p, q),
           zero, gsMakeDataExprCNat(gsMakeDataExprCDub(t, p))
         )),
      (ATerm) gsMakeDataEqn(pql,
         gsMakeDataExprLT(q, p),
         gsMakeDataExprMod(gsMakeDataExprCDub(t, p), gsMakeDataExprCDub(t, q)),
         gsMakeDataExprLast(
           gsMakeDataExprGDivMod(
             gsMakeDataExprDivMod(p, gsMakeDataExprCDub(t, q)),
             t,
             gsMakeDataExprCDub(t, q)
           )
           //XXX The line above is the normal form of the line below, needed
           //for consistency with lpsrewr.
           //gsMakeDataExprDivMod(gsMakeDataExprCDub(t, p), gsMakeDataExprCDub(t, q))
         )),
      //remainder after division (Nat -> Pos -> Nat)
      (ATerm) gsMakeDataEqn(pl, nil, gsMakeDataExprMod(zero, p), zero),
      (ATerm) gsMakeDataEqn(pql,nil, 
         gsMakeDataExprMod(gsMakeDataExprCNat(p), q),
         gsMakeDataExprMod(p, q))
    ), p_data_decls->data_eqns);
}

void impl_sort_nat_pair(t_data_decls *p_data_decls)
{
  //Declare sort NatPair
  p_data_decls->sorts = ATinsert(p_data_decls->sorts, (ATerm) gsMakeSortIdNatPair());
  //Declare constructors for sort NatPair
  p_data_decls->cons_ops =
    ATconcat(ATmakeList1((ATerm) gsMakeOpIdCPair()), p_data_decls->cons_ops);
  //Declare operations for sort NatPair
  ATermAppl se_bool = gsMakeSortExprBool();
  ATermAppl se_pos = gsMakeSortExprPos();
  ATermAppl se_nat = gsMakeSortExprNat();
  p_data_decls->ops = ATconcat(ATmakeList(5,
      (ATerm) gsMakeOpIdFirst(),
      (ATerm) gsMakeOpIdLast(),
      (ATerm) gsMakeOpIdDivMod(),
      (ATerm) gsMakeOpIdGDivMod(),
      (ATerm) gsMakeOpIdGGDivMod()
    ), p_data_decls->ops);
  //Declare data equations for sort Int
  ATermList el = ATmakeList0();
  ATermAppl nil = gsMakeNil();
  ATermAppl t = gsMakeDataExprTrue();
  ATermAppl f = gsMakeDataExprFalse();
  ATermAppl one = gsMakeDataExprC1();
  ATermAppl zero = gsMakeDataExprC0();
  ATermAppl b = gsMakeDataVarId(gsString2ATermAppl("b"), se_bool);
  ATermAppl p = gsMakeDataVarId(gsString2ATermAppl("p"), se_pos);
  ATermAppl q = gsMakeDataVarId(gsString2ATermAppl("q"), se_pos);
  ATermAppl m = gsMakeDataVarId(gsString2ATermAppl("m"), se_nat);
  ATermAppl n = gsMakeDataVarId(gsString2ATermAppl("n"), se_nat);
  ATermAppl m_ = gsMakeDataVarId(gsString2ATermAppl("m_"), se_nat);
  ATermAppl n_ = gsMakeDataVarId(gsString2ATermAppl("n_"), se_nat);
  ATermList mnm_n_l = ATmakeList4((ATerm) m, (ATerm) n, (ATerm) m_, (ATerm) n_);
  ATermList bpl = ATmakeList2((ATerm) b, (ATerm) p);
  ATermList bpql = ATmakeList3((ATerm) b, (ATerm) p, (ATerm) q);
  ATermList bpmnl = ATmakeList4((ATerm) b, (ATerm) p, (ATerm) m, (ATerm) n);
  ATermList pnl = ATmakeList2((ATerm) p, (ATerm) n);
  ATermList pqnl = ATmakeList3((ATerm) p, (ATerm) q, (ATerm) n);
  ATermList mnl = ATmakeList2((ATerm) m, (ATerm) n);
  p_data_decls->data_eqns = ATconcat(ATmakeList(10,
      //equality (NatPair -> NatPair -> Bool)
      (ATerm) gsMakeDataEqn(mnm_n_l,nil, 
         gsMakeDataExprEq(gsMakeDataExprCPair(m, n), gsMakeDataExprCPair(m_, n_)),
         gsMakeDataExprAnd(gsMakeDataExprEq(m, m_), gsMakeDataExprEq(n, n_))),
      //first (NatPair -> Nat)
      (ATerm) gsMakeDataEqn(mnl,nil, gsMakeDataExprFirst(gsMakeDataExprCPair(m, n)), m),
      //last (NatPair -> Nat)
      (ATerm) gsMakeDataEqn(mnl,nil, gsMakeDataExprLast(gsMakeDataExprCPair(m, n)), n),
      //quotient and remainder after division (Pos -> Pos -> NatPair)
      (ATerm) gsMakeDataEqn(el,nil,
         gsMakeDataExprDivMod(one, one),
         gsMakeDataExprCPair(gsMakeDataExprCNat(one), zero)),
      (ATerm) gsMakeDataEqn(bpl,nil,
         gsMakeDataExprDivMod(one, gsMakeDataExprCDub(b, p)),
         gsMakeDataExprCPair(zero, gsMakeDataExprCNat(one))),
      (ATerm) gsMakeDataEqn(bpql,nil,
         gsMakeDataExprDivMod(gsMakeDataExprCDub(b, p), q),
         gsMakeDataExprGDivMod(gsMakeDataExprDivMod(p, q), b, q)),
      //generalised quotient and remainder after division (NatPair -> Bool -> Pos -> NatPair)
      (ATerm) gsMakeDataEqn(bpmnl,nil,
         gsMakeDataExprGDivMod(gsMakeDataExprCPair(m, n), b, p),
         gsMakeDataExprGGDivMod(gsMakeDataExprDub(b, n), m, p)),
      //generalised generalised quotient and remainder after division (Nat -> Nat -> Pos -> NatPair)
      (ATerm) gsMakeDataEqn(pnl,nil,
         gsMakeDataExprGGDivMod(zero, n, p),
         gsMakeDataExprCPair(gsMakeDataExprDub(f, n), zero)),
      (ATerm) gsMakeDataEqn(pqnl,
         gsMakeDataExprLT(p, q),
         gsMakeDataExprGGDivMod(gsMakeDataExprCNat(p), n, q),
         gsMakeDataExprCPair(gsMakeDataExprDub(f, n), gsMakeDataExprCNat(p))),
      (ATerm) gsMakeDataEqn(pqnl,
         //gsMakeDataExprGTE(p, q),
         gsMakeDataExprLTE(q, p),
         gsMakeDataExprGGDivMod(gsMakeDataExprCNat(p), n, q),
         gsMakeDataExprCPair(gsMakeDataExprDub(t, n), gsMakeDataExprGTESubtB(f, p, q)))
    ), p_data_decls->data_eqns);
}

void impl_sort_int(t_data_decls *p_data_decls)
{
  //Declare sort Int
  p_data_decls->sorts = ATinsert(p_data_decls->sorts, (ATerm) gsMakeSortIdInt());
  //Declare constructors for sort Int
  p_data_decls->cons_ops = ATconcat(ATmakeList2(
      (ATerm) gsMakeOpIdCInt(),
      (ATerm) gsMakeOpIdCNeg()
    ), p_data_decls->cons_ops);
  //Declare operations for sort Int
  ATermAppl se_pos = gsMakeSortExprPos();
  ATermAppl se_nat = gsMakeSortExprNat();
  ATermAppl se_int = gsMakeSortExprInt();
  p_data_decls->ops = ATconcat(ATmakeList(30,
      (ATerm) gsMakeOpIdNat2Int(),
      (ATerm) gsMakeOpIdInt2Nat(),
      (ATerm) gsMakeOpIdPos2Int(),
      (ATerm) gsMakeOpIdInt2Pos(),
      (ATerm) gsMakeOpIdLTE(se_int),
      (ATerm) gsMakeOpIdLT(se_int),
      (ATerm) gsMakeOpIdGTE(se_int),
      (ATerm) gsMakeOpIdGT(se_int),
      (ATerm) gsMakeOpIdMax(se_pos, se_int),
      (ATerm) gsMakeOpIdMax(se_int, se_pos),
      (ATerm) gsMakeOpIdMax(se_nat, se_int),
      (ATerm) gsMakeOpIdMax(se_int, se_nat),
      (ATerm) gsMakeOpIdMax(se_int, se_int),
      (ATerm) gsMakeOpIdMin(se_int),
      (ATerm) gsMakeOpIdAbs(se_int),
      (ATerm) gsMakeOpIdNeg(se_pos),
      (ATerm) gsMakeOpIdNeg(se_nat),
      (ATerm) gsMakeOpIdNeg(se_int),
      (ATerm) gsMakeOpIdSucc(se_int),
      (ATerm) gsMakeOpIdPred(se_nat),
      (ATerm) gsMakeOpIdPred(se_int),
      (ATerm) gsMakeOpIdDub(se_int),
      (ATerm) gsMakeOpIdAdd(se_int, se_int),
      (ATerm) gsMakeOpIdSubt(se_pos),
      (ATerm) gsMakeOpIdSubt(se_nat),
      (ATerm) gsMakeOpIdSubt(se_int),
      (ATerm) gsMakeOpIdMult(se_int),
      (ATerm) gsMakeOpIdDiv(se_int),
      (ATerm) gsMakeOpIdMod(se_int),
      (ATerm) gsMakeOpIdExp(se_int)
    ), p_data_decls->ops);
  //Declare data equations for sort Int
  ATermList el = ATmakeList0();
  ATermAppl nil = gsMakeNil();
  ATermAppl one = gsMakeDataExprC1();
  ATermAppl zero = gsMakeDataExprC0();
  ATermAppl t = gsMakeDataExprTrue();
  ATermAppl f = gsMakeDataExprFalse();
  ATermAppl b = gsMakeDataVarId(gsString2ATermAppl("b"), gsMakeSortExprBool());
  ATermAppl p = gsMakeDataVarId(gsString2ATermAppl("p"), gsMakeSortExprPos());
  ATermAppl q = gsMakeDataVarId(gsString2ATermAppl("q"), gsMakeSortExprPos());
  ATermAppl m = gsMakeDataVarId(gsString2ATermAppl("m"), gsMakeSortExprNat());
  ATermAppl n = gsMakeDataVarId(gsString2ATermAppl("n"), gsMakeSortExprNat());
  ATermAppl x = gsMakeDataVarId(gsString2ATermAppl("x"), gsMakeSortExprInt());
  ATermAppl y = gsMakeDataVarId(gsString2ATermAppl("y"), gsMakeSortExprInt());
  ATermList pl = ATmakeList1((ATerm) p);
  ATermList pql = ATmakeList2((ATerm) p, (ATerm) q);
  ATermList nl = ATmakeList1((ATerm) n);
  ATermList bnl = ATmakeList2((ATerm) b, (ATerm) n);
  ATermList pnl = ATmakeList2((ATerm) p, (ATerm) n);
  ATermList mnl = ATmakeList2((ATerm) m, (ATerm) n);
  ATermList xyl = ATmakeList2((ATerm) x, (ATerm) y);
  p_data_decls->data_eqns = ATconcat(ATmakeList(64,
      //equality (Int -> Int -> Bool)
      (ATerm) gsMakeDataEqn(mnl,nil, 
         gsMakeDataExprEq(gsMakeDataExprCInt(m), gsMakeDataExprCInt(n)),
         gsMakeDataExprEq(m, n)),
      (ATerm) gsMakeDataEqn(pnl,nil, 
         gsMakeDataExprEq(gsMakeDataExprCInt(n), gsMakeDataExprCNeg(p)), f),
      (ATerm) gsMakeDataEqn(pnl,nil, 
         gsMakeDataExprEq(gsMakeDataExprCNeg(p), gsMakeDataExprCInt(n)), f),
      (ATerm) gsMakeDataEqn(pql,nil, 
         gsMakeDataExprEq(gsMakeDataExprCNeg(p), gsMakeDataExprCNeg(q)),
         gsMakeDataExprEq(p, q)),
      //convert Nat to Int (Nat -> Int)
      (ATerm) gsMakeDataEqn(el, nil, gsMakeOpIdNat2Int(), gsMakeOpIdCInt()),
      //convert Int to Nat (Int -> Nat)
      (ATerm) gsMakeDataEqn(nl, nil,
         gsMakeDataExprInt2Nat(gsMakeDataExprCInt(n)), n),
      //convert Pos to Int (Pos -> Int)
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprPos2Int(p), gsMakeDataExprCInt(gsMakeDataExprCNat(p))),
      //convert Int to Pos (Int -> Pos)
      (ATerm) gsMakeDataEqn(nl, nil,
         gsMakeDataExprInt2Pos(gsMakeDataExprCInt(n)),
         gsMakeDataExprNat2Pos(n)),
      //less than or equal (Int -> Int -> Bool)
      (ATerm) gsMakeDataEqn(mnl,nil, 
         gsMakeDataExprLTE(gsMakeDataExprCInt(m), gsMakeDataExprCInt(n)),
         gsMakeDataExprLTE(m, n)),
      (ATerm) gsMakeDataEqn(pnl,nil, 
         gsMakeDataExprLTE(gsMakeDataExprCInt(n), gsMakeDataExprCNeg(p)), f),
      (ATerm) gsMakeDataEqn(pnl,nil, 
         gsMakeDataExprLTE(gsMakeDataExprCNeg(p), gsMakeDataExprCInt(n)), t),
      (ATerm) gsMakeDataEqn(pql,nil, 
         gsMakeDataExprLTE(gsMakeDataExprCNeg(p), gsMakeDataExprCNeg(q)),
         gsMakeDataExprLTE(q, p)),
      //less than (Int -> Int -> Bool)
      (ATerm) gsMakeDataEqn(mnl,nil, 
         gsMakeDataExprLT(gsMakeDataExprCInt(m), gsMakeDataExprCInt(n)),
         gsMakeDataExprLT(m, n)),
      (ATerm) gsMakeDataEqn(pnl,nil, 
         gsMakeDataExprLT(gsMakeDataExprCInt(n), gsMakeDataExprCNeg(p)), f),
      (ATerm) gsMakeDataEqn(pnl,nil, 
         gsMakeDataExprLT(gsMakeDataExprCNeg(p), gsMakeDataExprCInt(n)), t),
      (ATerm) gsMakeDataEqn(pql,nil, 
         gsMakeDataExprLT(gsMakeDataExprCNeg(p), gsMakeDataExprCNeg(q)),
         gsMakeDataExprLT(q, p)),
      //greater than or equal (Int -> Int -> Bool)
      (ATerm) gsMakeDataEqn(xyl,nil,
         gsMakeDataExprGTE(x, y), gsMakeDataExprLTE(y, x)),
      //greater than (Int -> Int -> Bool)
      (ATerm) gsMakeDataEqn(xyl,nil,
         gsMakeDataExprGT(x, y), gsMakeDataExprLT(y, x)),
      //maximum (Pos -> Int -> Pos)
      (ATerm) gsMakeDataEqn(pnl,nil,
         gsMakeDataExprMax(p, gsMakeDataExprCInt(n)),
         //gsMakeDataExprMax(p, n)),
         gsMakeDataExprMax(p, n)),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprMax(p, gsMakeDataExprCNeg(q)), p),
      //maximum (Int -> Pos -> Pos)
      (ATerm) gsMakeDataEqn(pnl,nil,
         gsMakeDataExprMax(gsMakeDataExprCInt(n), p),
         gsMakeDataExprMax(n, p)),
      (ATerm) gsMakeDataEqn(pql,nil,
         gsMakeDataExprMax(gsMakeDataExprCNeg(q), p), p),
      //maximum (Nat -> Int -> Nat)
      (ATerm) gsMakeDataEqn(mnl,nil,
         gsMakeDataExprMax(m, gsMakeDataExprCInt(n)),
         //gsMakeDataExprMax(m, n)),
         gsMakeDataExprIf(gsMakeDataExprLTE(m, n), n, m)),
      (ATerm) gsMakeDataEqn(pnl,nil,
         gsMakeDataExprMax(n, gsMakeDataExprCNeg(p)), n),
      //maximum (Int -> Nat -> Nat)
      (ATerm) gsMakeDataEqn(mnl,nil,
         gsMakeDataExprMax(gsMakeDataExprCInt(m), n),
         //gsMakeDataExprMax(m, n)),
         gsMakeDataExprIf(gsMakeDataExprLTE(m, n), n, m)),
      (ATerm) gsMakeDataEqn(pnl,nil,
         gsMakeDataExprMax(gsMakeDataExprCNeg(p), n), n),
      //maximum (Int -> Int -> Int)
      (ATerm) gsMakeDataEqn(xyl,nil, gsMakeDataExprMax(x, y),
         gsMakeDataExprIf(gsMakeDataExprLTE(x, y), y, x)),
      //minimum (Int -> Int -> Int)
      (ATerm) gsMakeDataEqn(xyl,nil, gsMakeDataExprMin(x, y),
         gsMakeDataExprIf(gsMakeDataExprLTE(x, y), x, y)),
      //absolute value (Int -> Nat)
      (ATerm) gsMakeDataEqn(nl,nil,
         gsMakeDataExprAbs(gsMakeDataExprCInt(n)), n),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprAbs(gsMakeDataExprCNeg(p)),
         gsMakeDataExprCNat(p)),
      //negation (Pos -> Int)
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprNeg(p), gsMakeDataExprCNeg(p)),
      //negation (Nat -> Int)
      (ATerm) gsMakeDataEqn(el,nil,
         gsMakeDataExprNeg(zero), gsMakeDataExprCInt(zero)),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprNeg(gsMakeDataExprCNat(p)),
         gsMakeDataExprCNeg(p)),
      //negation (Int -> Int)
      (ATerm) gsMakeDataEqn(nl,nil,
         gsMakeDataExprNeg(gsMakeDataExprCInt(n)), gsMakeDataExprNeg(n)),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprNeg(gsMakeDataExprCNeg(p)),
         gsMakeDataExprCInt(gsMakeDataExprCNat(p))),
      //successor (Int -> Int)
      (ATerm) gsMakeDataEqn(nl,nil,
         gsMakeDataExprSucc(gsMakeDataExprCInt(n)),
         gsMakeDataExprCInt(gsMakeDataExprCNat(gsMakeDataExprSucc(n)))),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprSucc(gsMakeDataExprCNeg(p)),
           gsMakeDataExprNeg(gsMakeDataExprPred(p))),      
      //predecessor (Nat -> Int)
      (ATerm) gsMakeDataEqn(el,nil,
         gsMakeDataExprPred(zero), gsMakeDataExprCNeg(one)),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprPred(gsMakeDataExprCNat(p)),
         gsMakeDataExprCInt(gsMakeDataExprPred(p))),
      //predecessor (Int -> Int)
      (ATerm) gsMakeDataEqn(nl,nil,
         gsMakeDataExprPred(gsMakeDataExprCInt(n)), gsMakeDataExprPred(n)),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprPred(gsMakeDataExprCNeg(p)),
         gsMakeDataExprCNeg(gsMakeDataExprSucc(p))),
      //double (Bool -> Int -> Int)
      (ATerm) gsMakeDataEqn(bnl,nil,
         gsMakeDataExprDub(b, gsMakeDataExprCInt(n)),
         gsMakeDataExprCInt(gsMakeDataExprDub(b, n))),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprDub(f, gsMakeDataExprCNeg(p)),
         gsMakeDataExprCNeg(gsMakeDataExprCDub(f, p))),
      (ATerm) gsMakeDataEqn(pl,nil,
         gsMakeDataExprDub(t, gsMakeDataExprCNeg(p)),
         //gsMakeDataExprNeg(gsMakeDataExprPred(gsMakeDataExprCDub(f, p)))),
         gsMakeDataExprNeg(gsMakeDataExprDub(t, gsMakeDataExprPred(p)))),
      //addition (Int -> Int -> Int)
      (ATerm) gsMakeDataEqn(mnl, nil,
         gsMakeDataExprAdd(gsMakeDataExprCInt(m), gsMakeDataExprCInt(n)),
         gsMakeDataExprCInt(gsMakeDataExprAdd(m, n))),
      (ATerm) gsMakeDataEqn(pnl, nil,
         gsMakeDataExprAdd(gsMakeDataExprCInt(n), gsMakeDataExprCNeg(p)),
         gsMakeDataExprSubt(n, gsMakeDataExprCNat(p))),
      (ATerm) gsMakeDataEqn(pnl, nil,
         gsMakeDataExprAdd(gsMakeDataExprCNeg(p), gsMakeDataExprCInt(n)),
         gsMakeDataExprSubt(n, gsMakeDataExprCNat(p))),
      (ATerm) gsMakeDataEqn(pql, nil,
         gsMakeDataExprAdd(gsMakeDataExprCNeg(p), gsMakeDataExprCNeg(q)),
         gsMakeDataExprCNeg(gsMakeDataExprAddC(f, p, q))),
      //subtraction (Pos -> Pos -> Int)
      (ATerm) gsMakeDataEqn(pql,
         //gsMakeDataExprGTE(p, q),
         gsMakeDataExprLTE(q, p),
         gsMakeDataExprSubt(p, q),
         //gsMakeDataExprCInt(gsMakeDataExprGTESubt(p, q))),
         gsMakeDataExprCInt(gsMakeDataExprGTESubtB(f, p, q))),
      (ATerm) gsMakeDataEqn(pql,
         gsMakeDataExprLT(p, q),
         gsMakeDataExprSubt(p, q),
         //gsMakeDataExprNeg(gsMakeDataExprGTESubt(q, p))),
         gsMakeDataExprNeg(gsMakeDataExprGTESubtB(f, q, p))),
      //subtraction (Nat -> Nat -> Int)
      (ATerm) gsMakeDataEqn(mnl,
         //gsMakeDataExprGTE(m, n),
         gsMakeDataExprLTE(n, m),
         gsMakeDataExprSubt(m, n),
         gsMakeDataExprCInt(gsMakeDataExprGTESubt(m, n))),
      (ATerm) gsMakeDataEqn(mnl,
         gsMakeDataExprLT(m, n),
         gsMakeDataExprSubt(m, n),
         gsMakeDataExprNeg(gsMakeDataExprGTESubt(n, m))),
      //subtraction (Int -> Int -> Int)
      (ATerm) gsMakeDataEqn(xyl, nil,
         gsMakeDataExprSubt(x, y), gsMakeDataExprAdd(x, gsMakeDataExprNeg(y))),
      //multiplication (Int -> Int -> Int)
      (ATerm) gsMakeDataEqn(mnl, nil,
         gsMakeDataExprMult(gsMakeDataExprCInt(m), gsMakeDataExprCInt(n)),
         gsMakeDataExprCInt(gsMakeDataExprMult(m, n))),
      (ATerm) gsMakeDataEqn(pnl, nil,
         gsMakeDataExprMult(gsMakeDataExprCInt(n), gsMakeDataExprCNeg(p)),
         gsMakeDataExprNeg(gsMakeDataExprMult(gsMakeDataExprCNat(p), n))),
      (ATerm) gsMakeDataEqn(pnl, nil,
         gsMakeDataExprMult(gsMakeDataExprCNeg(p), gsMakeDataExprCInt(n)),
         gsMakeDataExprNeg(gsMakeDataExprMult(gsMakeDataExprCNat(p), n))),
      (ATerm) gsMakeDataEqn(pql, nil,
         gsMakeDataExprMult(gsMakeDataExprCNeg(p), gsMakeDataExprCNeg(q)),
         gsMakeDataExprCInt(gsMakeDataExprCNat(gsMakeDataExprMult(p, q)))),
      //quotient after division (Int -> Pos -> Int)
      (ATerm) gsMakeDataEqn(pnl, nil,
         gsMakeDataExprDiv(gsMakeDataExprCInt(n), p),
         gsMakeDataExprCInt(gsMakeDataExprDiv(n, p))),
      (ATerm) gsMakeDataEqn(pql, nil,
         gsMakeDataExprDiv(gsMakeDataExprCNeg(p), q),
         gsMakeDataExprCNeg(gsMakeDataExprSucc(
           gsMakeDataExprDiv(gsMakeDataExprPred(p), q)))),
      //remainder after division (Int -> Pos -> Nat)
      (ATerm) gsMakeDataEqn(pnl, nil,
         gsMakeDataExprMod(gsMakeDataExprCInt(n), p),
         gsMakeDataExprMod(n, p)),
      (ATerm) gsMakeDataEqn(pql, nil,
         gsMakeDataExprMod(gsMakeDataExprCNeg(p), q),
         gsMakeDataExprInt2Nat(gsMakeDataExprSubt(q, gsMakeDataExprSucc(
           gsMakeDataExprMod(gsMakeDataExprPred(p), q))))),
      //exponentiation (Int -> Nat -> Int)
      (ATerm) gsMakeDataEqn(mnl,nil,
         gsMakeDataExprExp(gsMakeDataExprCInt(m), n),
         gsMakeDataExprCInt(gsMakeDataExprExp(m, n))),
      (ATerm) gsMakeDataEqn(pnl,
         gsMakeDataExprEven(n),
         gsMakeDataExprExp(gsMakeDataExprCNeg(p), n),
         gsMakeDataExprCInt(gsMakeDataExprCNat(gsMakeDataExprExp(p, n)))),
      (ATerm) gsMakeDataEqn(pnl,
         gsMakeDataExprNot(gsMakeDataExprEven(n)),
         gsMakeDataExprExp(gsMakeDataExprCNeg(p), n),
         gsMakeDataExprCNeg(gsMakeDataExprExp(p, n)))
    ), p_data_decls->data_eqns);
  //add implementation of sort Nat, if necessary
  if (ATindexOf(p_data_decls->sorts, (ATerm) gsMakeSortIdNat(), 0) == -1) {
    impl_sort_nat(p_data_decls);
  }
}

void impl_sort_real(t_data_decls *p_data_decls)
{
  //Declare sort Real
  p_data_decls->sorts = ATinsert(p_data_decls->sorts, (ATerm) gsMakeSortIdReal());
  //Declare constructors for sort Real
  //Declare operations for sort Real
  ATermAppl se_real = gsMakeSortExprReal();
  p_data_decls->ops = ATconcat(ATmakeList(21,
      (ATerm) gsMakeOpIdCReal(),
      (ATerm) gsMakeOpIdPos2Real(),
      (ATerm) gsMakeOpIdNat2Real(),
      (ATerm) gsMakeOpIdInt2Real(),
      (ATerm) gsMakeOpIdReal2Pos(),
      (ATerm) gsMakeOpIdReal2Nat(),
      (ATerm) gsMakeOpIdReal2Int(),
      (ATerm) gsMakeOpIdLTE(se_real),
      (ATerm) gsMakeOpIdLT(se_real),
      (ATerm) gsMakeOpIdGTE(se_real),
      (ATerm) gsMakeOpIdGT(se_real),
      (ATerm) gsMakeOpIdMax(se_real, se_real),
      (ATerm) gsMakeOpIdMin(se_real),
      (ATerm) gsMakeOpIdAbs(se_real),
      (ATerm) gsMakeOpIdNeg(se_real),
      (ATerm) gsMakeOpIdSucc(se_real),
      (ATerm) gsMakeOpIdPred(se_real),
      (ATerm) gsMakeOpIdAdd(se_real, se_real),
      (ATerm) gsMakeOpIdSubt(se_real),
      (ATerm) gsMakeOpIdMult(se_real),
      (ATerm) gsMakeOpIdExp(se_real)
    ), p_data_decls->ops);
  //Declare data equations for sort Int
  ATermList el = ATmakeList0();
  ATermAppl nil = gsMakeNil();
  ATermAppl p = gsMakeDataVarId(gsString2ATermAppl("p"), gsMakeSortExprPos());
  ATermAppl n = gsMakeDataVarId(gsString2ATermAppl("n"), gsMakeSortExprNat());
  ATermAppl x = gsMakeDataVarId(gsString2ATermAppl("x"), gsMakeSortExprInt());
  ATermAppl y = gsMakeDataVarId(gsString2ATermAppl("y"), gsMakeSortExprInt());
  ATermAppl r = gsMakeDataVarId(gsString2ATermAppl("r"), gsMakeSortExprReal());
  ATermAppl s = gsMakeDataVarId(gsString2ATermAppl("s"), gsMakeSortExprReal());
  ATermList pl = ATmakeList1((ATerm) p);
  ATermList nl = ATmakeList1((ATerm) n);
  ATermList xl  = ATmakeList1((ATerm) x);
  ATermList xyl = ATmakeList2((ATerm) x, (ATerm) y);
  ATermList nxl = ATmakeList2((ATerm) n, (ATerm) x);
  ATermList rsl = ATmakeList2((ATerm) r, (ATerm) s);
  p_data_decls->data_eqns = ATconcat(ATmakeList(21,
      //equality (Real -> Real -> Bool)
      (ATerm) gsMakeDataEqn(xyl,nil,
         gsMakeDataExprEq(gsMakeDataExprCReal(x), gsMakeDataExprCReal(y)),
         gsMakeDataExprEq(x, y)),
      //convert Int to Real (Int -> Real)
      (ATerm) gsMakeDataEqn(el, nil, gsMakeOpIdInt2Real(), gsMakeOpIdCReal()),
      //convert Nat to Real (Nat -> Real)
      (ATerm) gsMakeDataEqn(nl, nil,
         gsMakeDataExprNat2Real(n),
         gsMakeDataExprCReal(gsMakeDataExprCInt(n))),
      //convert Pos to Real (Pos -> Real)
      (ATerm) gsMakeDataEqn(pl, nil,
         gsMakeDataExprPos2Real(p),
         gsMakeDataExprCReal(gsMakeDataExprCInt(gsMakeDataExprCNat(p)))),
      //convert Real to Int (Real -> Int)
      (ATerm) gsMakeDataEqn(xl, nil,
         gsMakeDataExprReal2Int(gsMakeDataExprCReal(x)), x),
      //convert Real to Nat (Real -> Nat)
      (ATerm) gsMakeDataEqn(xl, nil,
         gsMakeDataExprReal2Nat(gsMakeDataExprCReal(x)),
         gsMakeDataExprInt2Nat(x)),
      //convert Real to Pos (Real -> Pos)
      (ATerm) gsMakeDataEqn(xl, nil,
         gsMakeDataExprReal2Pos(gsMakeDataExprCReal(x)),
         gsMakeDataExprInt2Pos(x)),
      //less than or equal (Real -> Real -> Bool)
      (ATerm) gsMakeDataEqn(xyl,nil,
         gsMakeDataExprLTE(gsMakeDataExprCReal(x), gsMakeDataExprCReal(y)),
         gsMakeDataExprLTE(x, y)),
      //less than (Real -> Real -> Bool)
      (ATerm) gsMakeDataEqn(xyl,nil,
         gsMakeDataExprLT(gsMakeDataExprCReal(x), gsMakeDataExprCReal(y)),
         gsMakeDataExprLT(x, y)),
      //greater than or equal (Real -> Real -> Bool)
      (ATerm) gsMakeDataEqn(rsl,nil,
         gsMakeDataExprGTE(r, s), gsMakeDataExprLTE(s, r)),
      //greater than (Real -> Real -> Bool)
      (ATerm) gsMakeDataEqn(rsl,nil,
         gsMakeDataExprGT(r, s), gsMakeDataExprLT(s, r)),
      //maximum (Real -> Real -> Real)
      (ATerm) gsMakeDataEqn(rsl,nil, gsMakeDataExprMax(r, s),
         gsMakeDataExprIf(gsMakeDataExprLTE(r, s), s, r)),
      //minimum (Real -> Real -> Real)
      (ATerm) gsMakeDataEqn(rsl,nil, gsMakeDataExprMin(r, s),
         gsMakeDataExprIf(gsMakeDataExprLTE(r, s), r, s)),
      //absolute value (Real -> Real)
      (ATerm) gsMakeDataEqn(xl,nil,
         gsMakeDataExprAbs(gsMakeDataExprCReal(x)),
         gsMakeDataExprCReal(gsMakeDataExprCInt(gsMakeDataExprAbs(x)))),
      //negation (Real -> Real)
      (ATerm) gsMakeDataEqn(xl,nil,
         gsMakeDataExprNeg(gsMakeDataExprCReal(x)),
         gsMakeDataExprCReal(gsMakeDataExprNeg(x))),
      //successor (Real -> Real)
      (ATerm) gsMakeDataEqn(xl,nil,
         gsMakeDataExprSucc(gsMakeDataExprCReal(x)),
         gsMakeDataExprCReal(gsMakeDataExprSucc(x))),
      //predecessor (Real -> Real)
      (ATerm) gsMakeDataEqn(xl,nil,
         gsMakeDataExprPred(gsMakeDataExprCReal(x)),
         gsMakeDataExprCReal(gsMakeDataExprPred(x))),
      //addition (Real -> Real -> Real)
      (ATerm) gsMakeDataEqn(xyl, nil,
         gsMakeDataExprAdd(gsMakeDataExprCReal(x), gsMakeDataExprCReal(y)),
         gsMakeDataExprCReal(gsMakeDataExprAdd(x, y))),
      //subtraction (Real -> Real -> Real)
      (ATerm) gsMakeDataEqn(xyl, nil,
         gsMakeDataExprSubt(gsMakeDataExprCReal(x), gsMakeDataExprCReal(y)),
         //gsMakeDataExprCReal(gsMakeDataExprSubt(x, y))),
         //x-y == x + (-y)
         gsMakeDataExprCReal(gsMakeDataExprAdd(x, gsMakeDataExprNeg(y)))),
      //multiplication (Real -> Real -> Real)
      (ATerm) gsMakeDataEqn(xyl, nil,
         gsMakeDataExprMult(gsMakeDataExprCReal(x), gsMakeDataExprCReal(y)),
         gsMakeDataExprCReal(gsMakeDataExprMult(x, y))),
      //exponentiation (Real -> Nat -> Real)
      (ATerm) gsMakeDataEqn(nxl,nil,
         gsMakeDataExprExp(gsMakeDataExprCReal(x), n),
         gsMakeDataExprCReal(gsMakeDataExprExp(x, n)))
    ), p_data_decls->data_eqns);
  //add implementation of sort Int, if necessary
  if (ATindexOf(p_data_decls->sorts, (ATerm) gsMakeSortIdInt(), 0) == -1) {
    impl_sort_int(p_data_decls);
  }
}

void impl_standard_functions_sort(ATermAppl sort, t_data_decls *p_data_decls)
{
  assert(gsIsSortExpr(sort));
  //Declare operations for sort
  p_data_decls->ops = ATconcat(ATmakeList(3,
      (ATerm) gsMakeOpIdEq(sort),
      (ATerm) gsMakeOpIdNeq(sort),
      (ATerm) gsMakeOpIdIf(sort)
    ), p_data_decls->ops);
  //Declare data equations for sort sort
  ATermAppl x = gsMakeDataVarId(gsString2ATermAppl("x"), sort);
  ATermAppl y = gsMakeDataVarId(gsString2ATermAppl("y"), sort);
  ATermAppl b = gsMakeDataVarId(gsString2ATermAppl("b"), gsMakeSortExprBool());
  ATermAppl nil = gsMakeNil();
  ATermAppl t = gsMakeDataExprTrue();
  ATermAppl f = gsMakeDataExprFalse();
  ATermList xl = ATmakeList1((ATerm) x);
  ATermList xyl = ATmakeList2((ATerm) x, (ATerm) y);
  ATermList bxl = ATmakeList2((ATerm) b, (ATerm) x);
  p_data_decls->data_eqns = ATconcat(ATmakeList(5,
      //equality (sort_arrow -> sort_arrow -> Bool)
      (ATerm) gsMakeDataEqn(xl, nil,
        gsMakeDataExprEq(x, x), t),
      //inequality (sort_arrow -> sort_arrow -> Bool)
      (ATerm) gsMakeDataEqn(xyl,nil,
        gsMakeDataExprNeq(x, y),
        gsMakeDataExprNot(gsMakeDataExprEq(x, y))),
      //conditional (Bool -> sort_arrow -> sort_arrow -> sort_arrow)
      (ATerm) gsMakeDataEqn(xyl,nil,
        gsMakeDataExprIf(t, x, y),
        x),
      (ATerm) gsMakeDataEqn(xyl,nil,
        gsMakeDataExprIf(f, x, y),
        y),
      (ATerm) gsMakeDataEqn(bxl,nil,
        gsMakeDataExprIf(b, x, x),
        x)
    ), p_data_decls->data_eqns);
}

ATermAppl make_fresh_struct_sort_id(ATerm term)
{
  return gsMakeSortId(gsFreshString2ATermAppl(struct_prefix(), term, false));
}
 
ATermAppl make_fresh_list_sort_id(ATerm term)
{
  return gsMakeSortId(gsFreshString2ATermAppl(list_prefix(), term, false));
}

ATermAppl make_fresh_set_sort_id(ATerm term)
{
  return gsMakeSortId(gsFreshString2ATermAppl(set_prefix(), term, false));
}

ATermAppl make_fresh_bag_sort_id(ATerm term)
{
  return gsMakeSortId(gsFreshString2ATermAppl(bag_prefix(), term, false));
}

ATermAppl make_fresh_lambda_op_id(ATermAppl sort_expr, ATerm term)
{
  return gsMakeOpId(gsFreshString2ATermAppl(lambda_prefix(), term, false),
    sort_expr);
}

void split_sort_decls(ATermList sort_decls, ATermList *p_sort_ids,
  ATermList *p_sort_refs)
{
  ATermList sort_ids = ATmakeList0();
  ATermList sort_refs = ATmakeList0();
  while (!ATisEmpty(sort_decls))
  {
    ATermAppl sortDecl = ATAgetFirst(sort_decls);
    if (gsIsSortRef(sortDecl)) {
      sort_refs = ATinsert(sort_refs, (ATerm) sortDecl);
    } else { //gsIsSortId(sortDecl)
      sort_ids = ATinsert(sort_ids, (ATerm) sortDecl);
    }
    sort_decls = ATgetNext(sort_decls);
  }
  *p_sort_ids = ATreverse(sort_ids);  
  *p_sort_refs = ATreverse(sort_refs);
}
    } // namespace detail
  }   // namespace core
}     // namespace mcrl2
