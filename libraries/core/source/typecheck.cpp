// Author(s): Yaroslav Usenko
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>

#include "mcrl2/core/typecheck.h"
#include "mcrl2/core/detail/struct_core.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/core/print.h"
#include <aterm_ext.h>
#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/map.h"
#include "mcrl2/data/basic_sort.h"
#include "mcrl2/data/bool.h"
#include "mcrl2/data/pos.h"
#include "mcrl2/data/nat.h"
#include "mcrl2/data/int.h"
#include "mcrl2/data/real.h"
#include "mcrl2/data/list.h"
#include "mcrl2/data/set.h"
#include "mcrl2/data/bag.h"
#include "mcrl2/data/standard.h"
#include "mcrl2/data/standard_utility.h"
#include "mcrl2/data/find.h"
#include "mcrl2/data/unknown_sort.h"
#include "mcrl2/data/multiple_possible_sorts.h"
#include "mcrl2/data/function_update.h"

using namespace mcrl2::core::detail;
using namespace mcrl2::data;

namespace mcrl2
{
namespace core
{

// Static data

static bool was_warning_upcasting=false;
static bool was_ambiguous=false;

// system constants and functions
typedef struct
{
  ATermTable constants;   //name -> Set(sort expression)
  ATermTable functions;   //name -> Set(sort expression)
} gsSystem;
static gsSystem gssystem;

static ATermList list_minus(ATermList l, ATermList m)
{
  ATermList n = ATmakeList0();
  for (; !ATisEmpty(l); l=ATgetNext(l))
  {
    if (ATindexOf(m,ATgetFirst(l),0) == ATERM_NON_EXISTING_POSITION)
    {
      n = ATinsert(n,ATgetFirst(l));
    }
  }
  return ATreverse(n);
}

// the static context of the spec will be checked and used, not transformed
typedef struct
{
  ATermIndexedSet basic_sorts;
  ATermTable defined_sorts; //name -> sort expression
  ATermTable constants;   //name -> Set(sort expression)
  ATermTable functions;   //name -> Set(sort expression)
  ATermTable actions;         //name -> Set(List(sort expression)) because of action polymorphism
  //ATermIndexedSet typedactions; //name#type
  ATermTable processes;         //name -> Set(List(sort expression)) bacause of process polymorphism
  //ATermIndexedSet typedprocesses: use keys of body.proc_parsa
  ATermTable glob_vars;   //name -> Type: global variables (for proc, pbes and init)
  ATermTable PBs;
} Context;
static Context context;

// the body may be transformed
typedef struct
{
  ATermList equations;
  //ATermTable proc_freevars;     //name#type -> List(Vars)
  ATermTable proc_pars;         //name#type -> List(Vars)
  ATermTable proc_bodies; //name#type -> rhs
  //ATermAppl init;             //in the hash tables proc_pars and proc_bodies with key "init#[]" (beware when writing)
} Body;
static Body body;

// Static function declarations
static void gstcDataInit(void);
static void gstcDataDestroy(void);
static bool gstcReadInSorts(ATermList);
static bool gstcReadInConstructors(ATermList NewSorts=NULL);
static bool gstcReadInFuncs(ATermList, ATermList);
static bool gstcReadInActs(ATermList);
static bool gstcReadInProcsAndInit(ATermList, ATermAppl);
static bool gstcReadInPBESAndInit(ATermAppl, ATermAppl);

static bool gstcTransformVarConsTypeData(void);
static bool gstcTransformActProcVarConst(void);
static bool gstcTransformPBESVarConst(void);

static ATermList gstcWriteProcs(ATermList);
static ATermList gstcWritePBES(ATermList);

static bool gstcInTypesA(ATermAppl, ATermList);
static bool gstcEqTypesA(ATermAppl, ATermAppl);
static bool gstcInTypesL(ATermList, ATermList);
static bool gstcEqTypesL(ATermList, ATermList);

static bool gstcIsSortDeclared(ATermAppl SortName);
static bool gstcIsSortExprDeclared(ATermAppl SortExpr);
static bool gstcIsSortExprListDeclared(ATermList SortExprList);
static bool gstcReadInSortStruct(ATermAppl);
static bool gstcAddConstant(ATermAppl, const char*);
static bool gstcAddFunction(ATermAppl, const char*, bool allow_double_decls=false);

static void gstcAddSystemConstant(ATermAppl);
static void gstcAddSystemFunction(ATermAppl);

static void gstcATermTableCopy(ATermTable Vars, ATermTable CopyVars);

static ATermTable gstcAddVars2Table(ATermTable,ATermList);
static ATermTable gstcRemoveVars(ATermTable Vars, ATermList VarDecls);
static bool gstcVarsUnique(ATermList VarDecls);
static ATermAppl gstcRewrActProc(ATermTable, ATermAppl, bool is_pbes=false);
static inline ATermAppl gstcMakeActionOrProc(bool, ATermAppl, ATermList, ATermList);
static ATermAppl gstcTraverseActProcVarConstP(ATermTable, ATermAppl);
static ATermAppl gstcTraversePBESVarConstPB(ATermTable, ATermAppl);
static ATermAppl gstcTraverseVarConsTypeD(ATermTable DeclaredVars, ATermTable AllowedVars, ATermAppl*, ATermAppl, ATermTable FreeVars=NULL, bool strict_ambiguous=true, bool warn_upcasting=false);
static ATermAppl gstcTraverseVarConsTypeDN(ATermTable DeclaredVars, ATermTable AllowedVars, ATermAppl* , ATermAppl, ATermTable FreeVars=NULL, bool strict_ambiguous=true, size_t nPars = ATERM_NON_EXISTING_POSITION, bool warn_upcasting=false);

static ATermList gstcInsertType(ATermList TypeList, ATermAppl Type);

static inline bool gstcIsPos(ATermAppl Number)
{
  char c=gsATermAppl2String(Number)[0];
  return (bool)(isdigit(c) && c>'0');
}
static inline bool gstcIsNat(ATermAppl Number)
{
  return isdigit(gsATermAppl2String(Number)[0]) != 0;
}

static inline ATermAppl INIT_KEY(void)
{
  return gsMakeProcVarId(gsString2ATermAppl("init"),ATmakeList0());
}

static ATermAppl gstcUpCastNumericType(ATermAppl NeededType, ATermAppl Type, ATermAppl* Par, bool warn_upcasting=false);
static ATermAppl gstcMaximumType(ATermAppl Type1, ATermAppl Type2);

static ATermList gstcGetNotInferredList(ATermList TypeListList);
static ATermList gstcAdjustNotInferredList(ATermList TypeList, ATermList TypeListList);
static bool gstcIsNotInferredL(ATermList TypeListList);
static bool gstcIsTypeAllowedA(ATermAppl Type, ATermAppl PosType);
static bool gstcIsTypeAllowedL(ATermList TypeList, ATermList PosTypeList);
static ATermAppl gstcUnwindType(ATermAppl Type);
static ATermAppl gstcUnSet(ATermAppl PosType);
static ATermAppl gstcUnBag(ATermAppl PosType);
static ATermAppl gstcUnList(ATermAppl PosType);
static ATermAppl gstcUnArrowProd(ATermList ArgTypes, ATermAppl PosType);
static ATermList gstcTypeListsIntersect(ATermList TypeListList1, ATermList TypeListList2);
static ATermList gstcGetVarTypes(ATermList VarDecls);
static ATermAppl gstcTypeMatchA(ATermAppl Type, ATermAppl PosType);
static ATermList gstcTypeMatchL(ATermList TypeList, ATermList PosTypeList);
static bool gstcHasUnknown(ATermAppl Type);
static bool gstcIsNumericType(ATermAppl Type);
static ATermAppl gstcExpandNumTypesUp(ATermAppl Type);
static ATermAppl gstcExpandNumTypesDown(ATermAppl Type);
static ATermAppl gstcMinType(ATermList TypeList);
static bool gstcMActIn(ATermList MAct, ATermList MActs);
static bool gstcMActEq(ATermList MAct1, ATermList MAct2);
static bool gstcMActSubEq(ATermList MAct1, ATermList MAct2);
static ATermAppl gstcUnifyMinType(ATermAppl Type1, ATermAppl Type2);
static ATermAppl gstcMatchIf(ATermAppl Type);
static ATermAppl gstcMatchEqNeqComparison(ATermAppl Type);
static ATermAppl gstcMatchListOpCons(ATermAppl Type);
static ATermAppl gstcMatchListOpSnoc(ATermAppl Type);
static ATermAppl gstcMatchListOpConcat(ATermAppl Type);
static ATermAppl gstcMatchListOpEltAt(ATermAppl Type);
static ATermAppl gstcMatchListOpHead(ATermAppl Type);
static ATermAppl gstcMatchListOpTail(ATermAppl Type);
static ATermAppl gstcMatchSetOpSet2Bag(ATermAppl Type);
static ATermAppl gstcMatchListSetBagOpIn(ATermAppl Type);
static ATermAppl gstcMatchSetBagOpUnionDiffIntersect(ATermAppl Type);
static ATermAppl gstcMatchSetOpSetCompl(ATermAppl Type);
static ATermAppl gstcMatchBagOpBag2Set(ATermAppl Type);
static ATermAppl gstcMatchBagOpBagCount(ATermAppl Type);
static ATermAppl gstcMatchFuncUpdate(ATermAppl Type);


static void gstcErrorMsgCannotCast(ATermAppl CandidateType, ATermList Arguments, ATermList ArgumentTypes);

// Typechecking modal formulas
static ATermAppl gstcTraverseStateFrm(ATermTable Vars, ATermTable StateVars, ATermAppl StateFrm);
static ATermAppl gstcTraverseRegFrm(ATermTable Vars, ATermAppl RegFrm);
static ATermAppl gstcTraverseActFrm(ATermTable Vars, ATermAppl ActFrm);


static ATermAppl gstcFoldSortRefs(ATermAppl Spec);
//Pre: Spec is a specification that adheres to the internal syntax after
//     type checking
//Ret: Spec in which all sort references are maximally folded, i.e.:
//     - sort references to SortId's and SortArrow's are removed from the
//       rest of Spec (including the other sort references) by means of
//       forward substitition
//     - other sort references are removed from the rest of Spec by means of
//       backward substitution
//     - self references are removed, i.e. sort references of the form A = A

static ATermList gstcFoldSortRefsInSortRefs(ATermList SortRefs);
//Pre: SortRefs is a list of sort references
//Ret: SortRefs in which all sort references are maximally folded

static void gstcSplitSortDecls(ATermList SortDecls, ATermList* PSortIds,
                               ATermList* PSortRefs);
//Pre: SortDecls is a list of SortId's and SortRef's
//Post:*PSortIds and *PSortRefs contain the SortId's and SortRef's from
//     SortDecls, in the same order

static ATermAppl gstcUpdateSortSpec(ATermAppl Spec, ATermAppl SortSpec);
//Pre: Spec and SortSpec are resp. specifications and sort specifications that
//     adhere to the internal syntax after type checking
//Ret: Spec in which the sort specification is replaced by SortSpec

///\brief Increases the value of each key in map
///\param[in] m A mapping from an ATerm to a boolean
///\return m in which all values are negated
static inline atermpp::map<atermpp::aterm,bool> neg_values(atermpp::map<atermpp::aterm,bool> m)
{
  for (atermpp::map<atermpp::aterm,bool>::iterator i = m.begin() ; i != m.end(); i++)
  {
    m[i->first] = !i->second;
  }
  return m;
}


//type checking functions
//-----------------------

ATermAppl type_check_data_spec(ATermAppl data_spec)
{
  if (gsVerbose)
  {
    std::cerr << "type checking data specification...\n";
  }

  ATermAppl Result=NULL;
  if (gsDebug)
  {
    std::cerr << "type checking phase started\n";
  }
  gstcDataInit();

  if (gsDebug)
  {
    std::cerr << "type checking read-in phase started\n";
  }

  if (gstcReadInSorts(ATLgetArgument(ATAgetArgument(data_spec,0),0)))
  {
    // Check sorts for loops
    // Unwind sorts to enable equiv and subtype relations
    if (gstcReadInConstructors())
    {
      if (gstcReadInFuncs(ATLgetArgument(ATAgetArgument(data_spec,1),0),
                          ATLgetArgument(ATAgetArgument(data_spec,2),0)))
      {
        body.equations=ATLgetArgument(ATAgetArgument(data_spec,3),0);
        if (gsDebug)
        {
          std::cerr << "type checking read-in phase finished\n";
        }

        if (gsDebug)
        {
          std::cerr << "type checking transform VarConst phase started\n";
        }

        if (gstcTransformVarConsTypeData())
        {
          if (gsDebug)
          {
            std::cerr << "type checking transform VarConst phase finished\n";
          }

          Result = ATsetArgument(data_spec, (ATerm) gsMakeDataEqnSpec(body.equations),3);

          Result = gstcFoldSortRefs(Result);

          if (gsDebug)
          {
            std::cerr << "type checking phase finished\n";
          }
        }
      }
    }
  }

  gstcDataDestroy();
  return Result;
}

ATermAppl type_check_proc_spec(ATermAppl proc_spec)
{
  if (gsVerbose)
  {
    std::cerr << "type checking process specification...\n";
  }

  ATermAppl Result=NULL;

  if (gsDebug)
  {
    std::cerr << "type checking phase started: " << pp(proc_spec) << "\n";
  }
  gstcDataInit();

  ATermAppl data_spec = ATAgetArgument(proc_spec, 0);
  if (gstcReadInSorts(ATLgetArgument(ATAgetArgument(data_spec,0),0)))
  {
    // Check sorts for loops
    // Unwind sorts to enable equiv and subtype relations
    if (gstcReadInConstructors())
    {
      if (gstcReadInFuncs(ATLgetArgument(ATAgetArgument(data_spec,1),0),
                          ATLgetArgument(ATAgetArgument(data_spec,2),0)))
      {
        body.equations=ATLgetArgument(ATAgetArgument(data_spec,3),0);
        if (gstcReadInActs(ATLgetArgument(ATAgetArgument(proc_spec,1),0)))
        {
          ATermAppl glob_var_spec = ATAgetArgument(proc_spec,2);
          ATermList glob_vars = ATLgetArgument(glob_var_spec,0);
          if (gstcAddVars2Table(context.glob_vars, glob_vars))
          {
            if (gstcReadInProcsAndInit(ATLgetArgument(ATAgetArgument(proc_spec,3),0),
                                       ATAgetArgument(ATAgetArgument(proc_spec,4),0)))
            {
              if (gsDebug)
              {
                std::cerr << "type checking read-in phase finished\n";
              }

              if (gsDebug)
              {
                std::cerr << "type checking transform ActProc+VarConst phase started\n";
              }
              if (gstcTransformVarConsTypeData())
              {
                if (gstcTransformActProcVarConst())
                {
                  if (gsDebug)
                  {
                    std::cerr << "type checking transform ActProc+VarConst phase finished\n";
                  }

                  data_spec=ATAgetArgument(proc_spec,0);
                  data_spec=ATsetArgument(data_spec, (ATerm) gsMakeDataEqnSpec(body.equations),3);
                  Result=ATsetArgument(proc_spec,(ATerm)data_spec,0);
                  Result=ATsetArgument(Result,(ATerm)gsMakeProcEqnSpec(gstcWriteProcs(ATLgetArgument(ATAgetArgument(proc_spec,3),0))),3);
                  Result=ATsetArgument(Result,(ATerm)gsMakeProcessInit(ATAtableGet(body.proc_bodies,(ATerm)INIT_KEY())),4);

                  Result=gstcFoldSortRefs(Result);

                  if (gsDebug)
                  {
                    std::cerr << "type checking phase finished\n";
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  gstcDataDestroy();
  return Result;
}

ATermAppl type_check_sort_expr(ATermAppl sort_expr, ATermAppl spec)
{
  if (gsVerbose)
  {
    std::cerr << "type checking sort expression...\n";
  }
  //check correctness of the sort expression in sort_expr
  //using the specification in spec
  assert(gsIsSortExpr(sort_expr));
  assert(gsIsProcSpec(spec) || gsIsLinProcSpec(spec) || gsIsPBES(spec) || gsIsDataSpec(spec));
  //gsWarningMsg("type checking of sort expressions is partially implemented\n");

  ATermAppl Result=NULL;

  if (gsDebug)
  {
    std::cerr << "type checking phase started\n";
  }

  gstcDataInit();

  if (gsDebug)
  {
    std::cerr << "type checking of sort expressions read-in phase started\n";
  }

  ATermAppl data_spec = NULL;
  if (gsIsDataSpec(spec))
  {
    data_spec = spec;
  }
  else
  {
    data_spec = ATAgetArgument(spec, 0);
  }
  ATermList sorts = ATLgetArgument(ATAgetArgument(data_spec, 0), 0);

  //XXX read-in from spec (not finished)
  if (gstcReadInSorts(sorts))
  {
    if (gsDebug)
    {
      std::cerr << "type checking of sort expressions read-in phase finished\n";
    }

    if (!is_unknown_sort(sort_expr) && !is_multiple_possible_sorts(sort_expr))
    {
      if (gstcIsSortExprDeclared(sort_expr))
      {
        Result=sort_expr;
      }
    }
    else
    {
      gsErrorMsg("type checking of sort expressions failed (%T is not a sort expression)\n",sort_expr);
    }
  }
  else
  {
    gsErrorMsg("reading Sorts from LPS failed\n");
  }

  gstcDataDestroy();
  return Result;
}

ATermAppl type_check_data_expr(ATermAppl data_expr, ATermAppl sort_expr, ATermAppl spec, ATermTable Vars)
{
  if (gsVerbose)
  {
    std::cerr << "type checking data expression...\n";
  }
  //check correctness of the data expression in data_expr using
  //the specification in spec

  //check preconditions
  assert(gsIsProcSpec(spec) || gsIsLinProcSpec(spec) || gsIsPBES(spec) || gsIsDataSpec(spec));
  assert(gsIsDataExpr(data_expr));
  assert((sort_expr == NULL) || gsIsSortExpr(sort_expr));

  ATermAppl Result=NULL;

  if (gsDebug)
  {
    std::cerr << "type checking phase started\n";
  }

  gstcDataInit();

  if (gsDebug)
  {
    std::cerr << "type checking of data expression read-in phase started\n";
  }

  ATermAppl data_spec = NULL;
  if (gsIsDataSpec(spec))
  {
    data_spec = spec;
  }
  else
  {
    data_spec = ATAgetArgument(spec, 0);
  }
  ATermList sorts = ATLgetArgument(ATAgetArgument(data_spec, 0), 0);
  ATermList constructors = ATLgetArgument(ATAgetArgument(data_spec, 1), 0);
  ATermList mappings = ATLgetArgument(ATAgetArgument(data_spec, 2), 0);

  //XXX read-in from spec (not finished)
  if (gstcReadInSorts(sorts) &&
      gstcReadInConstructors() &&
      gstcReadInFuncs(constructors,mappings))
  {
    if (gsDebug)
    {
      std::cerr << "type checking of data expression read-in phase finished\n";
    }

    if ((sort_expr != NULL) && (is_unknown_sort(sort_expr) || is_multiple_possible_sorts(sort_expr)))
    {
      gsErrorMsg("type checking of data expression failed (%T is not a sort expression)\n",sort_expr);
    }
    else if ((sort_expr == NULL) || gstcIsSortExprDeclared(sort_expr))
    {
      bool destroy_vars=(Vars == NULL);
      if (destroy_vars)
      {
        Vars=ATtableCreate(63,50);
      }
      ATermAppl data=data_expr;
      ATermAppl Type=gstcTraverseVarConsTypeD(Vars,Vars,&data,(sort_expr==NULL)?(ATermAppl)data::unknown_sort():sort_expr);
      if (destroy_vars)
      {
        ATtableDestroy(Vars);
      }
      if (Type && !data::is_unknown_sort(data::sort_expression(Type)))
      {
        Result=data;
      }
      else
      {
        gsErrorMsg("type checking of data expression failed\n");
      }
    }
  }
  else
  {
    gsErrorMsg("reading from LPS failed\n");
  }
  gstcDataDestroy();

  return Result;
}

ATermAppl type_check_mult_act(
  ATermAppl mult_act,
  ATermAppl data_spec,
  ATermList action_labels)
{
  if (gsDebug)
  {
    std::cerr << "type checking multiaction...\n";
  }
  //check correctness of the multi-action in mult_act using
  //the process specification or LPS in spec
  // assert (gsIsProcSpec(spec) || gsIsLinProcSpec(spec));
  ATermAppl Result=NULL;

  if (gsDebug)
  {
    std::cerr << "type checking phase started\n";
  }
  gstcDataInit();

  if (gsDebug)
  {
    std::cerr << "type checking of multiactions read-in phase started\n";
  }

  // ATermAppl data_spec = ATAgetArgument(spec, 0);
  ATermList sorts = ATLgetArgument(ATAgetArgument(data_spec, 0), 0);
  ATermList constructors = ATLgetArgument(ATAgetArgument(data_spec, 1), 0);
  ATermList mappings = ATLgetArgument(ATAgetArgument(data_spec, 2), 0);
  // ATermList action_labels = ATLgetArgument(ATAgetArgument(spec, 1), 0);

  //XXX read-in from spec (not finished)
  if (gstcReadInSorts(sorts)
      && gstcReadInConstructors()
      && gstcReadInFuncs(constructors,mappings)
      && gstcReadInActs(action_labels)
     )
  {
    if (gsDebug)
    {
      std::cerr << "type checking of multiactions read-in phase finished\n";
    }

    if (gsIsMultAct(mult_act))
    {
      ATermTable Vars=ATtableCreate(63,50);
      ATermList r=ATmakeList0();
      for (ATermList l=ATLgetArgument(mult_act,0); !ATisEmpty(l); l=ATgetNext(l))
      {
        ATermAppl o=ATAgetFirst(l);
        assert(gsIsParamId(o));
        o=gstcTraverseActProcVarConstP(Vars,o);
        if (!o)
        {
          goto done;
        }
        r=ATinsert(r,(ATerm)o);
      }
      Result=ATsetArgument(mult_act,(ATerm)ATreverse(r),0);

done:
      ATtableDestroy(Vars);
    }
    else
    {
      gsErrorMsg("type checking of multiactions failed (%T is not a multiaction)\n",mult_act);
    }
  }
  else
  {
    gsErrorMsg("reading from LPS failed\n");
  }
  gstcDataDestroy();
  return Result;
}

ATermList type_check_mult_actions(
  ATermList mult_actions,
  ATermAppl data_spec,
  ATermList action_labels)
{
  if (gsDebug)
  {
    std::cerr << "type checking multiactions...\n";
  }
  //check correctness of the multi-action in mult_act using
  //the process specification or LPS in spec
  // assert (gsIsProcSpec(spec) || gsIsLinProcSpec(spec));

  if (gsDebug)
  {
    std::cerr << "type checking phase started\n";
  }
  gstcDataInit();

  if (gsDebug)
  {
    std::cerr << "type checking of multiactions read-in phase started\n";
  }

  // ATermAppl data_spec = ATAgetArgument(spec, 0);
  ATermList sorts = ATLgetArgument(ATAgetArgument(data_spec, 0), 0);
  ATermList constructors = ATLgetArgument(ATAgetArgument(data_spec, 1), 0);
  ATermList mappings = ATLgetArgument(ATAgetArgument(data_spec, 2), 0);
  // ATermList action_labels = ATLgetArgument(ATAgetArgument(spec, 1), 0);

  //XXX read-in from spec (not finished)
  ATermList result=ATempty;
  if (gstcReadInSorts(sorts)
      && gstcReadInConstructors()
      && gstcReadInFuncs(constructors,mappings)
      && gstcReadInActs(action_labels))
  {
    if (gsDebug)
    {
      std::cerr << "type checking of multiactions read-in phase finished\n";
    }

    for (; !ATisEmpty(mult_actions); mult_actions=ATgetNext(mult_actions))
    {
      ATermTable Vars=ATtableCreate(63,50);
      ATermList r=ATmakeList0();

      for (ATermList l=(ATermList)ATgetFirst(mult_actions) ; !ATisEmpty(l); l=ATgetNext(l))
      {
        ATermAppl o=ATAgetFirst(l);
        assert(gsIsParamId(o));
        o=gstcTraverseActProcVarConstP(Vars,o);
        if (!o)
        {
          ATtableDestroy(Vars);
          throw mcrl2::runtime_error("Typechecking action failed: "+ core::pp(ATAgetFirst(l)));
        }
        r=ATinsert(r,(ATerm)o);
      }
      result = ATinsert(result,(ATerm)ATreverse(r));
    }
  }
  else
  {
    throw mcrl2::runtime_error("reading data/action specification failed");
  }
  gstcDataDestroy();
  return ATreverse(result);
}

ATermAppl type_check_proc_expr(ATermAppl proc_expr, ATermAppl spec)
{
  if (gsVerbose)
  {
    std::cerr << "type checking process expression...\n";
  }

  //check correctness of the process expression in proc_expr using
  //the process specification or LPS in spec
  assert(gsIsProcSpec(spec) || gsIsLinProcSpec(spec));
  gsWarningMsg("type checking of process expressions is not yet implemented\n");
  return proc_expr;
}

ATermAppl type_check_state_frm(ATermAppl state_frm, ATermAppl spec)
{
  if (gsVerbose)
  {
    std::cerr << "type checking state formula...\n";
  }
  assert(gsIsProcSpec(spec) || gsIsLinProcSpec(spec));
  //check correctness of the state formula in state_formula using
  //the process specification or LPS in spec as follows:
  //1) determine the types of actions according to the definitions
  //   in spec
  //2) determine the types of data expressions according to the
  //   definitions in spec
  //3) check for name conflicts of data variable declarations in
  //   forall, exists, mu and nu quantifiers
  //4) check for monotonicity of fixpoint variables

  ATermAppl Result=NULL;
  if (gsDebug)
  {
    std::cerr << "type checking phase started\n";
  }
  gstcDataInit();

  if (gsDebug)
  {
    std::cerr << "type checking of state formulas read-in phase started\n";
  }

  ATermAppl data_spec = ATAgetArgument(spec, 0);
  ATermList action_labels = ATLgetArgument(ATAgetArgument(spec, 1), 0);

  ATermList sorts = ATLgetArgument(ATAgetArgument(data_spec, 0), 0);
  ATermList constructors = ATLgetArgument(ATAgetArgument(data_spec, 1), 0);
  ATermList mappings = ATLgetArgument(ATAgetArgument(data_spec, 2), 0);

  //XXX read-in from spec (not finished)
  if (gstcReadInSorts(sorts))
  {
    if (gstcReadInConstructors())
    {
      if (gstcReadInFuncs(constructors,mappings))
      {
        if (action_labels != NULL)
        {
          if (!gstcReadInActs(action_labels))
          {
            gsWarningMsg("ignoring the previous error(s), the formula will be typechecked without action label information\n");
          }
        }
        else
        {
          gsWarningMsg("ignoring the previous error(s), the formula will be typechecked without action label information\n");
        }
        if (gsDebug)
        {
          std::cerr << "type checking of state formulas read-in phase finished\n";
        }

        ATermTable Vars=ATtableCreate(63,50);
        Result=gstcTraverseStateFrm(Vars,Vars,state_frm);
        ATtableDestroy(Vars);
      }
      else
      {
        gsErrorMsg("reading functions from LPS failed\n");
      }
    }
    else
    {
      gsErrorMsg("reading structure constructors from LPS failed.\n");
    }
  }
  else
  {
    gsErrorMsg("reading sorts from LPS failed\n");
  }
  gstcDataDestroy();
  return Result;
}

ATermAppl type_check_action_rename_spec(ATermAppl ar_spec, ATermAppl lps_spec)
{

  if (gsVerbose)
  {
    std::cerr << "type checking action rename specification...\n";
  }

  //check precondition
  assert(gsIsActionRenameSpec(ar_spec));

  ATermAppl Result=NULL;
  if (gsDebug)
  {
    std::cerr << "type checking phase started\n";
  }
  gstcDataInit();

  if (gsDebug)
  {
    std::cerr << "type checking of action rename specification read-in phase started\n";
  }

  ATermTable actions_from_lps=ATtableCreate(63,50);

  ATermAppl lps_data_spec = ATAgetArgument(lps_spec, 0);
  ATermList lps_sorts = ATLgetArgument(ATAgetArgument(lps_data_spec, 0), 0);
  ATermList lps_constructors = ATLgetArgument(ATAgetArgument(lps_data_spec, 1), 0);
  ATermList lps_mappings = ATLgetArgument(ATAgetArgument(lps_data_spec, 2), 0);
  ATermList lps_action_labels = ATLgetArgument(ATAgetArgument(lps_spec, 1), 0);

  //XXX read-in from LPS (not finished)
  if (gstcReadInSorts((ATermList) lps_sorts))
  {
    if (gstcReadInConstructors())
    {
      if (gstcReadInFuncs(lps_constructors, lps_mappings))
      {
        if (!gstcReadInActs(lps_action_labels))
        {
          gsWarningMsg("ignoring the previous error(s), the formula will be typechecked without action label information\n");
        }
        if (gsDebug)
        {
          std::cerr << "type checking of action rename specification read-in phase of LPS finished\n";
        }
        if (gsDebug)
        {
          std::cerr << "type checking of action rename specification read-in phase of rename file started\n";
        }

        ATermAppl data_spec = ATAgetArgument(ar_spec, 0);
        ATermList LPSSorts=ATtableKeys(context.defined_sorts); // remember the sorts from the LPS.
        if (!gstcReadInSorts(ATLgetArgument(ATAgetArgument(data_spec,0),0)))
        {
          goto finally;
        }
        if (gsDebug)
        {
          std::cerr << "type checking of action rename specification read-in phase of rename file sorts finished\n";
        }

        // Check sorts for loops
        // Unwind sorts to enable equiv and subtype relations
        if (!gstcReadInConstructors(list_minus(ATtableKeys(context.defined_sorts),LPSSorts)))
        {
          goto finally;
        }
        if (gsDebug)
        {
          std::cerr << "type checking of action rename specification read-in phase of rename file constructors finished\n";
        }

        if (!gstcReadInFuncs(ATLgetArgument(ATAgetArgument(data_spec,1),0),
                             ATLgetArgument(ATAgetArgument(data_spec,2),0)))
        {
          goto finally;
        }
        if (gsDebug)
        {
          std::cerr << "type checking of action rename specification read-in phase of rename file functions finished\n";
        }

        body.equations=ATLgetArgument(ATAgetArgument(data_spec,3),0);

        //Save the actions from LPS only for the latter use.
        gstcATermTableCopy(context.actions,actions_from_lps);
        if (!gstcReadInActs(ATLgetArgument(ATAgetArgument(ar_spec,1),0)))
        {
          goto finally;
        }
        if (gsDebug)
        {
          std::cerr << "type checking action rename specification read-in phase of the ActionRenameSpec finished\n";
        }

        if (!gstcTransformVarConsTypeData())
        {
          goto finally;
        }
        if (gsDebug)
        {
          std::cerr << "type checking transform VarConstTypeData phase finished\n";
        }

        data_spec=ATsetArgument(data_spec, (ATerm) gsMakeDataEqnSpec(body.equations),3);
        Result=ATsetArgument(ar_spec,(ATerm)data_spec,0);
        Result=gstcFoldSortRefs(Result);


        // now the action renaming rules themselves.
        ATermAppl ActionRenameRules=ATAgetArgument(ar_spec, 2);
        ATermList NewRules=ATmakeList0();

        ATermTable DeclaredVars=ATtableCreate(63,50);
        ATermTable FreeVars=ATtableCreate(63,50);

        bool b = true;

        for (ATermList l=ATLgetArgument(ActionRenameRules,0); !ATisEmpty(l); l=ATgetNext(l))
        {
          ATermAppl Rule=ATAgetFirst(l);
          assert(gsIsActionRenameRule(Rule));

          ATermList VarList=ATLgetArgument(Rule,0);
          if (!gstcVarsUnique(VarList))
          {
            b = false;
            gsErrorMsg("the variables in action rename rule %P are not unique\n",VarList,Rule);
            break;
          }

          ATermTable NewDeclaredVars=gstcAddVars2Table(DeclaredVars,VarList);
          if (!NewDeclaredVars)
          {
            b = false;
            break;
          }
          else
          {
            DeclaredVars=NewDeclaredVars;
          }

          ATermAppl Left=ATAgetArgument(Rule,2);
          assert(gsIsParamId(Left));
          {
            //extra check requested by Tom: actions in the LHS can only come from the LPS
            ATermTable temp=context.actions;
            context.actions=actions_from_lps;
            Left=gstcTraverseActProcVarConstP(DeclaredVars,Left);
            context.actions=temp;
            if (!Left)
            {
              b = false;
              break;
            }
          }

          ATermAppl Cond=ATAgetArgument(Rule,1);
          if (!(gstcTraverseVarConsTypeD(DeclaredVars,DeclaredVars,&Cond,sort_bool::bool_())))
          {
            b = false;  // JK 15/10/2009 remove gsIsNil check
            break;
          }

          ATermAppl Right=ATAgetArgument(Rule,3);
          assert(gsIsParamId(Right) || gsIsTau(Right) || gsIsDelta(Right));
          Right=gstcTraverseActProcVarConstP(DeclaredVars,Right);
          if (!Right)
          {
            b = false;
            break;
          }

          NewRules=ATinsert(NewRules,(ATerm)gsMakeActionRenameRule(VarList,Cond,Left,Right));
        }
        ATtableDestroy(FreeVars);
        ATtableDestroy(DeclaredVars);
        if (!b)
        {
          Result = NULL;
          goto finally;
        }

        ActionRenameRules=ATsetArgument(ActionRenameRules,(ATerm)ATreverse(NewRules),0);
        Result=ATsetArgument(Result,(ATerm)ActionRenameRules,2);
        if (gsDebug)
        {
          std::cerr << "type checking transform VarConstTypeData phase finished\n";
        }
      }
      else
      {
        gsErrorMsg("reading functions from LPS failed\n");
      }
    }
    else
    {
      gsErrorMsg("reading structure constructors from LPS failed\n");
    }
  }
  else
  {
    gsErrorMsg("reading sorts from LPS failed\n");
  }

finally:
  ATtableDestroy(actions_from_lps);
  gstcDataDestroy();
  return Result;
}

ATermAppl type_check_pbes_spec(ATermAppl pbes_spec)
{
  //check correctness of the PBES specification in pbes_spec
  //gsWarningMsg("type checking of PBES specifications is only partially implemented\n");

  if (gsVerbose)
  {
    std::cerr << "type checking PBES specification...\n";
  }

  assert(gsIsPBES(pbes_spec));

  ATermAppl Result=NULL;
  if (gsDebug)
  {
    std::cerr << "type checking phase of PBES specifications started\n";
  }
  gstcDataInit();

  if (gsDebug)
  {
    std::cerr << "type checking of PBES specification read-in phase started\n";
  }


  ATermAppl data_spec = ATAgetArgument(pbes_spec,0);
  ATermAppl pb_eqn_spec = ATAgetArgument(pbes_spec,2);
  ATermAppl pb_init = ATAgetArgument(pbes_spec,3);
  ATermAppl glob_var_spec = ATAgetArgument(pbes_spec,1);

  if (!gstcReadInSorts(ATLgetArgument(ATAgetArgument(data_spec,0),0)))
  {
    goto finally;
  }
  if (gsDebug)
  {
    std::cerr << "type checking of PBES specification read-in phase of sorts finished\n";
  }

  // Check sorts for loops
  // Unwind sorts to enable equiv and subtype relations
  if (!gstcReadInConstructors())
  {
    goto finally;
  }
  if (gsDebug)
  {
    std::cerr << "type checking of PBES specification read-in phase of constructors finished\n";
  }

  if (!gstcReadInFuncs(ATLgetArgument(ATAgetArgument(data_spec,1),0),
                       ATLgetArgument(ATAgetArgument(data_spec,2),0)))
  {
    goto finally;
  }
  if (gsDebug)
  {
    std::cerr << "type checking of PBES specification read-in phase of functions finished\n";
  }

  body.equations=ATLgetArgument(ATAgetArgument(data_spec,3),0);

  if (!gstcAddVars2Table(context.glob_vars, ATLgetArgument(glob_var_spec,0)))
  {
    goto finally;
  }
  if (gsDebug)
  {
    std::cerr << "type checking of PBES specification read-in phase of global variables finished\n";
  }

  if (!gstcReadInPBESAndInit(pb_eqn_spec,pb_init))
  {
    goto finally;
  }
  if (gsDebug)
  {
    std::cerr << "type checking PBES read-in phase finished\n";
  }

  if (gsDebug)
  {
    std::cerr << "type checking transform Data+PBES phase started\n";
  }
  if (!gstcTransformVarConsTypeData())
  {
    goto finally;
  }
  if (!gstcTransformPBESVarConst())
  {
    goto finally;
  }
  if (gsDebug)
  {
    std::cerr << "type checking transform Data+PBES phase finished\n";
  }

  data_spec=ATsetArgument(data_spec,(ATerm)gsMakeDataEqnSpec(body.equations),3);
  Result=ATsetArgument(pbes_spec,(ATerm)data_spec,0);

  pb_eqn_spec=ATsetArgument(pb_eqn_spec,(ATerm)gstcWritePBES(ATLgetArgument(pb_eqn_spec,0)),0);
  Result=ATsetArgument(Result,(ATerm)pb_eqn_spec,2);

  pb_init=ATsetArgument(pb_init,(ATerm)ATAtableGet(body.proc_bodies,(ATerm)INIT_KEY()),0);
  Result=ATsetArgument(Result,(ATerm)pb_init,3);

  Result=gstcFoldSortRefs(Result);

finally:
  gstcDataDestroy();
  return Result;
}

ATermList type_check_data_vars(ATermList data_vars, ATermAppl spec)
{
  if (gsVerbose)
  {
    std::cerr << "type checking data variables...\n";
  }
  //check correctness of the data variable declaration in sort_expr
  //using the specification in spec

  assert(gsIsProcSpec(spec) || gsIsLinProcSpec(spec) || gsIsPBES(spec) || gsIsDataSpec(spec));

  if (gsDebug)
  {
    std::cerr << "type checking phase started\n";
  }

  gstcDataInit();

  if (gsDebug)
  {
    std::cerr << "type checking of data variables read-in phase started\n";
  }

  ATermAppl data_spec = NULL;
  if (gsIsDataSpec(spec))
  {
    data_spec = spec;
  }
  else
  {
    data_spec = ATAgetArgument(spec, 0);
  }
  ATermList sorts = ATLgetArgument(ATAgetArgument(data_spec, 0), 0);

  //XXX read-in from spec (not finished)
  if (gstcReadInSorts(sorts))
  {
    if (gsDebug)
    {
      std::cerr << "type checking of data variables read-in phase finished\n";
    }

    ATermTable Vars=ATtableCreate(63,50);
    ATermTable NewVars=gstcAddVars2Table(Vars,data_vars);
    if (!NewVars)
    {
      ATtableDestroy(Vars);
      gsErrorMsg("type error while typechecking data variables\n");
      return NULL;
    }
    ATtableDestroy(Vars);
  }
  else
  {
    gsErrorMsg("reading from LPS failed\n");
  }
  gstcDataDestroy();

  return data_vars;
}

//local functions
//---------------

// fold functions

void gstcSplitSortDecls(ATermList SortDecls, ATermList* PSortIds,
                        ATermList* PSortRefs)
{
  ATermList SortIds = ATmakeList0();
  ATermList SortRefs = ATmakeList0();
  while (!ATisEmpty(SortDecls))
  {
    ATermAppl SortDecl = ATAgetFirst(SortDecls);
    if (gsIsSortRef(SortDecl))
    {
      SortRefs = ATinsert(SortRefs, (ATerm) SortDecl);
    }
    else     //gsIsSortId(SortDecl)
    {
      SortIds = ATinsert(SortIds, (ATerm) SortDecl);
    }
    SortDecls = ATgetNext(SortDecls);
  }
  *PSortIds = ATreverse(SortIds);
  *PSortRefs = ATreverse(SortRefs);
}

ATermAppl gstcUpdateSortSpec(ATermAppl Spec, ATermAppl SortSpec)
{
  assert(gsIsDataSpec(Spec) || gsIsProcSpec(Spec) || gsIsLinProcSpec(Spec) || gsIsPBES(Spec) || gsIsActionRenameSpec(Spec));
  assert(gsIsSortSpec(SortSpec));
  if (gsIsDataSpec(Spec))
  {
    Spec = ATsetArgument(Spec, (ATerm) SortSpec, 0);
  }
  else
  {
    ATermAppl DataSpec = ATAgetArgument(Spec, 0);
    DataSpec = ATsetArgument(DataSpec, (ATerm) SortSpec, 0);
    Spec = ATsetArgument(Spec, (ATerm) DataSpec, 0);
  }
  return Spec;
}

ATermAppl gstcFoldSortRefs(ATermAppl Spec)
{
  assert(gsIsDataSpec(Spec) || gsIsProcSpec(Spec) || gsIsLinProcSpec(Spec) || gsIsPBES(Spec) || gsIsActionRenameSpec(Spec));
  if (gsDebug)
  {
    std::cerr << "specification before folding:" << pp(Spec) << "\n";
  }
  //get sort declarations
  ATermAppl DataSpec;
  if (gsIsDataSpec(Spec))
  {
    DataSpec = Spec;
  }
  else
  {
    DataSpec = ATAgetArgument(Spec, 0);
  }
  ATermAppl SortSpec = ATAgetArgument(DataSpec, 0);
  ATermList SortDecls = ATLgetArgument(SortSpec, 0);
  //split sort declarations in sort id's and sort references
  ATermList SortIds = NULL;
  ATermList SortRefs = NULL;
  gstcSplitSortDecls(SortDecls, &SortIds, &SortRefs);
  //fold sort references in the sort references themselves
  SortRefs = gstcFoldSortRefsInSortRefs(SortRefs);
  //substitute sort references in the rest of Spec, i.e.
  //(a) remove sort references from Spec
  Spec = gstcUpdateSortSpec(Spec, gsMakeSortSpec(SortIds));
  //(b) build substitution table
  ATermTable Substs = ATtableCreate(2*ATgetLength(SortRefs),50);
  ATermList l = SortRefs;
  while (!ATisEmpty(l))
  {
    ATermAppl SortRef = ATAgetFirst(l);
    //add substitution for SortRef
    ATermAppl LHS = gsMakeSortId(ATAgetArgument(SortRef, 0));
    ATermAppl RHS = ATAgetArgument(SortRef, 1);
    if (gsIsSortId(RHS) || gsIsSortArrow(RHS))
    {
      //add forward substitution
      ATtablePut(Substs, (ATerm) LHS, (ATerm) RHS);
    }
    else
    {
      //add backward substitution
      ATtablePut(Substs, (ATerm) RHS, (ATerm) LHS);
    }
    l = ATgetNext(l);
  }
  //(c) perform substitutions until the specification becomes stable
  ATermAppl NewSpec = Spec;
  do
  {
    if (gsDebug)
    {
      std::cerr << "substituting sort references in specification\n";
    }
    Spec = NewSpec;
    NewSpec = (ATermAppl) gsSubstValuesTable(Substs, (ATerm) Spec, true);
  }
  while (!ATisEqual(NewSpec, Spec));
  ATtableDestroy(Substs);

  //add the removed sort references back to Spec
  Spec = gstcUpdateSortSpec(Spec, gsMakeSortSpec(ATconcat(SortIds, SortRefs)));
  if (gsDebug)
  {
    std::cerr << "specification after folding:\n" << pp(Spec) << "\n" ;
  }
  return Spec;
}

ATermList gstcFoldSortRefsInSortRefs(ATermList SortRefs)
{
  //fold sort references in SortRefs by means of repeated forward and backward
  //substitution
  ATermList NewSortRefs = SortRefs;
  size_t n = ATgetLength(SortRefs);
  //perform substitutions until the list of sort references becomes stable
  do
  {
    SortRefs = NewSortRefs;
    if (gsDebug)
    {
      std::cerr << "SortRefs contains the following sort references:\n" << pp(gsMakeSortSpec(SortRefs)) << "\n";
    }
    //perform substitutions implied by sort references in NewSortRefs to the
    //other elements in NewSortRefs
    for (size_t i = 0; i < n; i++)
    {
      ATermAppl SortRef = ATAelementAt(NewSortRefs, i);
      //turn SortRef into a substitution
      ATermAppl LHS = gsMakeSortId(ATAgetArgument(SortRef, 0));
      ATermAppl RHS = ATAgetArgument(SortRef, 1);
      ATermAppl Subst;
      if (gsIsSortId(RHS) || gsIsSortArrow(RHS))
      {
        //make forward substitution
        Subst = gsMakeSubst_Appl(LHS, RHS);
      }
      else
      {
        //make backward substitution
        Subst = gsMakeSubst_Appl(RHS, LHS);
      }
      if (gsDebug)
      {
        std::cerr << "performing substition " << pp(ATgetArgument(Subst, 0)) << " := " << pp(ATgetArgument(Subst, 1)) << "\n";
      }
      //perform Subst on all elements of NewSortRefs except for the i'th
      ATermList Substs = ATmakeList1((ATerm) Subst);
      for (size_t j = 0; j < n; j++)
      {
        if (i != j)
        {
          ATermAppl OldSortRef = ATAelementAt(NewSortRefs, j);
          ATermAppl NewSortRef = gsSubstValues_Appl(Substs, OldSortRef, true);
          if (!ATisEqual(NewSortRef, OldSortRef))
          {
            NewSortRefs = ATreplace(NewSortRefs, (ATerm) NewSortRef, j);
          }
        }
      }
    }
    if (gsDebug)
    {
      std::cerr << "\n";
    }
  }
  while (!ATisEqual(NewSortRefs, SortRefs));
  //remove self references
  ATermList l = ATmakeList0();
  while (!ATisEmpty(SortRefs))
  {
    ATermAppl SortRef = ATAgetFirst(SortRefs);
    if (!ATisEqual(gsMakeSortId(ATAgetArgument(SortRef, 0)),
                   ATAgetArgument(SortRef, 1)))
    {
      l = ATinsert(l, (ATerm) SortRef);
    }
    SortRefs = ATgetNext(SortRefs);
  }
  SortRefs = ATreverse(l);
  if (gsDebug)
  {
    std::cerr << "SortRefs, after removing self references:\n" << pp(gsMakeSortSpec(SortRefs)) << "\n";
  }
  return SortRefs;
}

// ========= main processing functions
void gstcDataInit(void)
{
  gssystem.constants=ATtableCreate(63,50);
  gssystem.functions=ATtableCreate(63,50);
  context.basic_sorts=ATindexedSetCreate(63,50);
  context.defined_sorts=ATtableCreate(63,50);
  context.constants=ATtableCreate(63,50);
  context.functions=ATtableCreate(63,50);
  context.actions=ATtableCreate(63,50);
  context.processes=ATtableCreate(63,50);
  context.PBs=ATtableCreate(63,50);
  context.glob_vars=ATtableCreate(63,50);
  body.proc_pars=ATtableCreate(63,50);
  body.proc_bodies=ATtableCreate(63,50);
  body.equations=NULL;
  ATprotectList(&body.equations);
  body.equations = ATmakeList0();

  //Creation of operation identifiers for system defined operations.
  //Bool
  gstcAddSystemConstant(sort_bool::true_());
  gstcAddSystemConstant(sort_bool::false_());
  gstcAddSystemFunction(sort_bool::not_());
  gstcAddSystemFunction(sort_bool::and_());
  gstcAddSystemFunction(sort_bool::or_());
  gstcAddSystemFunction(sort_bool::implies());
  gstcAddSystemFunction(equal_to(data::unknown_sort()));
  gstcAddSystemFunction(not_equal_to(data::unknown_sort()));
  gstcAddSystemFunction(if_(data::unknown_sort()));
  gstcAddSystemFunction(less(data::unknown_sort()));
  gstcAddSystemFunction(less_equal(data::unknown_sort()));
  gstcAddSystemFunction(greater_equal(data::unknown_sort()));
  gstcAddSystemFunction(greater(data::unknown_sort()));
  //Numbers
  gstcAddSystemFunction(sort_nat::pos2nat());
  gstcAddSystemFunction(sort_nat::cnat());
  gstcAddSystemFunction(sort_int::pos2int());
  gstcAddSystemFunction(sort_real::pos2real());
  gstcAddSystemFunction(sort_nat::nat2pos());
  gstcAddSystemFunction(sort_int::nat2int());
  gstcAddSystemFunction(sort_int::cint());
  gstcAddSystemFunction(sort_real::nat2real());
  gstcAddSystemFunction(sort_int::int2pos());
  gstcAddSystemFunction(sort_int::int2nat());
  gstcAddSystemFunction(sort_real::int2real());
  gstcAddSystemFunction(sort_real::creal());
  gstcAddSystemFunction(sort_real::real2pos());
  gstcAddSystemFunction(sort_real::real2nat());
  gstcAddSystemFunction(sort_real::real2int());
  gstcAddSystemConstant(sort_pos::c1());
  //more
  gstcAddSystemFunction(sort_real::maximum(sort_pos::pos(),sort_pos::pos()));
  gstcAddSystemFunction(sort_real::maximum(sort_pos::pos(),sort_nat::nat()));
  gstcAddSystemFunction(sort_real::maximum(sort_nat::nat(),sort_pos::pos()));
  gstcAddSystemFunction(sort_real::maximum(sort_nat::nat(),sort_nat::nat()));
  gstcAddSystemFunction(sort_real::maximum(sort_pos::pos(),sort_int::int_()));
  gstcAddSystemFunction(sort_real::maximum(sort_int::int_(),sort_pos::pos()));
  gstcAddSystemFunction(sort_real::maximum(sort_nat::nat(),sort_int::int_()));
  gstcAddSystemFunction(sort_real::maximum(sort_int::int_(),sort_nat::nat()));
  gstcAddSystemFunction(sort_real::maximum(sort_int::int_(),sort_int::int_()));
  gstcAddSystemFunction(sort_real::maximum(sort_real::real_(),sort_real::real_()));
  //more
  gstcAddSystemFunction(sort_real::minimum(sort_pos::pos(), sort_pos::pos()));
  gstcAddSystemFunction(sort_real::minimum(sort_nat::nat(), sort_nat::nat()));
  gstcAddSystemFunction(sort_real::minimum(sort_int::int_(), sort_int::int_()));
  gstcAddSystemFunction(sort_real::minimum(sort_real::real_(), sort_real::real_()));
  //more
  gstcAddSystemFunction(sort_real::abs(sort_pos::pos()));
  gstcAddSystemFunction(sort_real::abs(sort_nat::nat()));
  gstcAddSystemFunction(sort_real::abs(sort_int::int_()));
  gstcAddSystemFunction(sort_real::abs(sort_real::real_()));
  //more
  gstcAddSystemFunction(sort_real::negate(sort_pos::pos()));
  gstcAddSystemFunction(sort_real::negate(sort_nat::nat()));
  gstcAddSystemFunction(sort_real::negate(sort_int::int_()));
  gstcAddSystemFunction(sort_real::negate(sort_real::real_()));
  gstcAddSystemFunction(sort_real::succ(sort_pos::pos()));
  gstcAddSystemFunction(sort_real::succ(sort_nat::nat()));
  gstcAddSystemFunction(sort_real::succ(sort_int::int_()));
  gstcAddSystemFunction(sort_real::succ(sort_real::real_()));
  gstcAddSystemFunction(sort_real::pred(sort_pos::pos()));
  gstcAddSystemFunction(sort_real::pred(sort_nat::nat()));
  gstcAddSystemFunction(sort_real::pred(sort_int::int_()));
  gstcAddSystemFunction(sort_real::pred(sort_real::real_()));
  gstcAddSystemFunction(sort_real::plus(sort_pos::pos(),sort_pos::pos()));
  gstcAddSystemFunction(sort_real::plus(sort_pos::pos(),sort_nat::nat()));
  gstcAddSystemFunction(sort_real::plus(sort_nat::nat(),sort_pos::pos()));
  gstcAddSystemFunction(sort_real::plus(sort_nat::nat(),sort_nat::nat()));
  gstcAddSystemFunction(sort_real::plus(sort_int::int_(),sort_int::int_()));
  gstcAddSystemFunction(sort_real::plus(sort_real::real_(),sort_real::real_()));
  //more
  gstcAddSystemFunction(sort_real::minus(sort_pos::pos(), sort_pos::pos()));
  gstcAddSystemFunction(sort_real::minus(sort_nat::nat(), sort_nat::nat()));
  gstcAddSystemFunction(sort_real::minus(sort_int::int_(), sort_int::int_()));
  gstcAddSystemFunction(sort_real::minus(sort_real::real_(), sort_real::real_()));
  gstcAddSystemFunction(sort_real::times(sort_pos::pos(), sort_pos::pos()));
  gstcAddSystemFunction(sort_real::times(sort_nat::nat(), sort_nat::nat()));
  gstcAddSystemFunction(sort_real::times(sort_int::int_(), sort_int::int_()));
  gstcAddSystemFunction(sort_real::times(sort_real::real_(), sort_real::real_()));
  //more
  gstcAddSystemFunction(sort_int::div(sort_pos::pos(), sort_pos::pos()));
  gstcAddSystemFunction(sort_int::div(sort_nat::nat(), sort_pos::pos()));
  gstcAddSystemFunction(sort_int::div(sort_int::int_(), sort_pos::pos()));
  gstcAddSystemFunction(sort_int::mod(sort_pos::pos(), sort_pos::pos()));
  gstcAddSystemFunction(sort_int::mod(sort_nat::nat(), sort_pos::pos()));
  gstcAddSystemFunction(sort_int::mod(sort_int::int_(), sort_pos::pos()));
  gstcAddSystemFunction(sort_real::divides(sort_pos::pos(), sort_pos::pos()));
  gstcAddSystemFunction(sort_real::divides(sort_nat::nat(), sort_nat::nat()));
  gstcAddSystemFunction(sort_real::divides(sort_int::int_(), sort_int::int_()));
  gstcAddSystemFunction(sort_real::divides(sort_real::real_(), sort_real::real_()));
  gstcAddSystemFunction(sort_real::exp(sort_pos::pos(), sort_nat::nat()));
  gstcAddSystemFunction(sort_real::exp(sort_nat::nat(), sort_nat::nat()));
  gstcAddSystemFunction(sort_real::exp(sort_int::int_(), sort_nat::nat()));
  gstcAddSystemFunction(sort_real::exp(sort_real::real_(), sort_int::int_()));
  gstcAddSystemFunction(sort_real::floor());
  gstcAddSystemFunction(sort_real::ceil());
  gstcAddSystemFunction(sort_real::round());
  //Lists
  gstcAddSystemConstant(sort_list::nil(data::unknown_sort()));
  gstcAddSystemFunction(sort_list::cons_(data::unknown_sort()));
  gstcAddSystemFunction(sort_list::count(data::unknown_sort()));
  gstcAddSystemFunction(sort_list::snoc(data::unknown_sort()));
  gstcAddSystemFunction(sort_list::concat(data::unknown_sort()));
  gstcAddSystemFunction(sort_list::element_at(data::unknown_sort()));
  gstcAddSystemFunction(sort_list::head(data::unknown_sort()));
  gstcAddSystemFunction(sort_list::tail(data::unknown_sort()));
  gstcAddSystemFunction(sort_list::rhead(data::unknown_sort()));
  gstcAddSystemFunction(sort_list::rtail(data::unknown_sort()));
  gstcAddSystemFunction(sort_list::in(data::unknown_sort()));
  //Sets
  gstcAddSystemFunction(sort_bag::set2bag(data::unknown_sort()));
  gstcAddSystemConstant(sort_set::emptyset(data::unknown_sort()));
  gstcAddSystemFunction(sort_set::setin(data::unknown_sort()));
  gstcAddSystemFunction(sort_set::setunion_(data::unknown_sort()));
  gstcAddSystemFunction(sort_set::setdifference(data::unknown_sort()));
  gstcAddSystemFunction(sort_set::setintersection(data::unknown_sort()));
  gstcAddSystemFunction(sort_set::setcomplement(data::unknown_sort()));

  //Bags
  gstcAddSystemFunction(sort_bag::bag2set(data::unknown_sort()));
  gstcAddSystemConstant(sort_bag::emptybag(data::unknown_sort()));
  gstcAddSystemFunction(sort_bag::bagin(data::unknown_sort()));
  gstcAddSystemFunction(sort_bag::bagcount(data::unknown_sort()));
  gstcAddSystemFunction(sort_bag::bagjoin(data::unknown_sort()));
  gstcAddSystemFunction(sort_bag::bagdifference(data::unknown_sort()));
  gstcAddSystemFunction(sort_bag::bagintersect(data::unknown_sort()));

  // function update
  gstcAddSystemFunction(data::function_update(data::unknown_sort(),data::unknown_sort()));
}

void gstcDataDestroy(void)
{
  ATtableDestroy(gssystem.constants);
  ATtableDestroy(gssystem.functions);
  ATindexedSetDestroy(context.basic_sorts);
  ATtableDestroy(context.defined_sorts);
  ATtableDestroy(context.constants);
  ATtableDestroy(context.functions);
  ATtableDestroy(context.actions);
  ATtableDestroy(context.processes);
  ATtableDestroy(context.PBs);
  ATtableDestroy(context.glob_vars);
  ATtableDestroy(body.proc_pars);
  ATtableDestroy(body.proc_bodies);
  ATunprotectList(&body.equations);
}

static bool gstc_check_for_sort_alias_loop_through_function_sort_via_expression(
  const sort_expression& sort_expression_start_search,
  const basic_sort& end_search,
  std::set < basic_sort > &visited,
  const bool observed_a_sort_constructor);

// This function checks whether there is a path of sort aliases
// from start_search to end_search.
// The boolean observed_a_sort_constructor indicates whether on the
// current path a sort constainer or a function sort has been
// observed. The set visited indicates which basic sorts have been
// encountered on the current path. This is only used for loop  checking.
// The current algorithm is exponential, which is considered not too
// much of a problem, given that sort expressions are generally small.
static bool gstc_check_for_sort_alias_loop_through_function_sort(
  const basic_sort& start_search,
  const basic_sort& end_search,
  std::set < basic_sort > &visited,
  const bool observed_a_sort_constructor)
{
  ATermAppl aterm_reference=(ATermAppl)ATtableGet(context.defined_sorts,
                            (ATerm)static_cast<ATermAppl>(start_search.name()));

  if (aterm_reference==NULL)
  {
    // start_search is not a sort alias, and hence not a recursive sort.
    return false;
  }

  if (start_search==end_search)
  {
    // We found a loop.
    return observed_a_sort_constructor;
  }
  if (visited.find(start_search)!=visited.end())
  {
    // start_search has already been encountered. end_search will not be found via this path.
    return false;
  }

  visited.insert(start_search);
  const sort_expression reference(aterm_reference);
  return gstc_check_for_sort_alias_loop_through_function_sort_via_expression(reference,end_search,visited,observed_a_sort_constructor);
}

static bool gstc_check_for_sort_alias_loop_through_function_sort_via_expression(
  const sort_expression& sort_expression_start_search,
  const basic_sort& end_search,
  std::set < basic_sort > &visited,
  const bool observed_a_sort_constructor)
{
  if (is_basic_sort(sort_expression_start_search))
  {
    const basic_sort start_search(sort_expression_start_search);
    if (end_search==start_search)
    {
      return observed_a_sort_constructor;
    }
    else
    {
      return gstc_check_for_sort_alias_loop_through_function_sort(start_search,end_search,visited,observed_a_sort_constructor);
    }
  }

  if (is_container_sort(sort_expression_start_search))
  {
    // A loop through a list container is allowed, but a loop through a set or bag container
    // is problematic.
    const container_sort start_search_container(sort_expression_start_search);
    return gstc_check_for_sort_alias_loop_through_function_sort_via_expression(
             start_search_container.element_sort(),end_search,visited,
             start_search_container.container_name()!=list_container());
  }

  if (is_function_sort(sort_expression_start_search))
  {
    const function_sort f_start_search(sort_expression_start_search);
    if (gstc_check_for_sort_alias_loop_through_function_sort_via_expression(
          f_start_search.codomain(),end_search,visited,true))
    {
      return true;
    }
    for (sort_expression_list::const_iterator i=f_start_search.domain().begin();
         i!=f_start_search.domain().end(); ++i)
    {
      if (gstc_check_for_sort_alias_loop_through_function_sort_via_expression(
            *i,end_search,visited,true))
      {
        return true;
      }
    }
    // end_search has not been found, so:
    return false;
  }

  if (is_structured_sort(sort_expression_start_search))
  {
    const structured_sort struct_start_search(sort_expression_start_search);
    const function_symbol_vector constructor_functions=struct_start_search.constructor_functions();
    for (function_symbol_vector::const_iterator i=constructor_functions.begin();
         i!=constructor_functions.end(); ++i)
    {
      if (is_function_sort(i->sort()))
      {
        const sort_expression_list domain_sorts=function_sort(i->sort()).domain();
        for (sort_expression_list::const_iterator j=domain_sorts.begin();
             j!=domain_sorts.end(); ++j)
        {
          if (gstc_check_for_sort_alias_loop_through_function_sort_via_expression(
                *j,end_search,visited,observed_a_sort_constructor))
          {
            return true;
          }
        }
      }
    }
    return false;

  }

  assert(0); // start_search cannot be a multiple_possible_sorts, or an unknown sort.
  return false;
}

static bool gstcReadInSorts(ATermList Sorts)
{
  bool nnew;
  bool Result=true;
  for (; !ATisEmpty(Sorts); Sorts=ATgetNext(Sorts))
  {
    ATermAppl Sort=ATAgetFirst(Sorts);
    ATermAppl SortName=ATAgetArgument(Sort,0);
    if (sort_bool::is_bool(basic_sort(core::identifier_string(SortName))))
    {
      gsErrorMsg("attempt to redeclare sort Bool\n");
      return false;
    }
    if (sort_pos::is_pos(basic_sort(core::identifier_string(SortName))))
    {
      gsErrorMsg("attempt to redeclare sort Pos\n");
      return false;
    }
    if (sort_nat::is_nat(basic_sort(core::identifier_string(SortName))))
    {
      gsErrorMsg("attempt to redeclare sort Nat\n");
      return false;
    }
    if (sort_int::is_int(basic_sort(core::identifier_string(SortName))))
    {
      gsErrorMsg("attempt to redeclare sort Int\n");
      return false;
    }
    if (sort_real::is_real(basic_sort(core::identifier_string(SortName))))
    {
      gsErrorMsg("attempt to redeclare sort Real\n");
      return false;
    }
    if (ATindexedSetGetIndex(context.basic_sorts, (ATerm)SortName)>=0
        || ATAtableGet(context.defined_sorts, (ATerm)SortName))
    {

      gsErrorMsg("double declaration of sort %P\n",SortName);
      return false;
    }
    if (gsIsSortId(Sort))
    {
      ATindexedSetPut(context.basic_sorts, (ATerm)SortName, &nnew);
    }
    else if (gsIsSortRef(Sort))
    {
      ATtablePut(context.defined_sorts, (ATerm)SortName, (ATerm)ATAgetArgument(Sort,1));
      if (gsDebug)
      {
        std::cerr << "Add sort alias " << pp(SortName) << "  " << pp((ATerm)ATAgetArgument(Sort,1)) << "\n";
      }
    }
    else
    {
      assert(0);
    }
  }

  // Check for sorts that are recursive through container sorts.
  // E.g. sort L=List(L);
  // This is forbidden.

  ATermList sort_aliases=ATtableKeys(context.defined_sorts);
  for (; sort_aliases!=ATempty ; sort_aliases=ATgetNext(sort_aliases))
  {
    std::set < basic_sort > visited;
    const basic_sort s(core::identifier_string((ATermAppl)ATgetFirst(sort_aliases)));
    ATermAppl aterm_reference=(ATermAppl)ATtableGet(context.defined_sorts,
                              (ATerm)static_cast<ATermAppl>(s.name()));
    assert(aterm_reference!=NULL);
    const sort_expression ar(aterm_reference);
    if (gstc_check_for_sort_alias_loop_through_function_sort_via_expression(ar,s,visited,false))
    {
      gsErrorMsg("sort %P is recursively defined via a function sort, or a set or a bag type container\n",ATgetFirst(sort_aliases));
      return false;
    }
  }

  return Result;
}

static bool gstcReadInConstructors(ATermList NewSorts)
{
  ATermList Sorts=NewSorts;
  if (!Sorts)
  {
    Sorts=ATtableKeys(context.defined_sorts);
  }
  for (; !ATisEmpty(Sorts); Sorts=ATgetNext(Sorts))
  {
    ATermAppl SortExpr=ATAtableGet(context.defined_sorts,ATgetFirst(Sorts));
    if (!gstcIsSortExprDeclared(SortExpr))
    {
      return false;
    }
    if (!gstcReadInSortStruct(SortExpr))
    {
      return false;
    }
  }
  return true;
}


atermpp::map < data::sort_expression, data::basic_sort > construct_normalised_aliases()
{
  // This function does the same as data_specification::reconstruct_m_normalised_aliases().
  // Therefore, it should be replaced by that function, after restructuring the type checker.
  // First reset the normalised aliases and the mappings and constructors that have been
  // inherited to basic sort aliases during a previous round of sort normalisation.
  atermpp::map < data::sort_expression, data::basic_sort > normalised_aliases;

  // Fill normalised_aliases. Simple aliases are stored from left to
  // right. If the right hand side is non trivial (struct, list, set or bag)
  // the alias is stored from right to left.
  for (ATermList sort_walker=ATtableKeys(context.defined_sorts);  sort_walker!=ATempty; sort_walker=ATgetNext(sort_walker))
  {
    const core::identifier_string sort_name(ATAgetFirst(sort_walker));
    const data::basic_sort first(sort_name);
    const data::sort_expression second(atermpp::aterm_appl((ATermAppl)ATtableGet(context.defined_sorts,(ATerm)static_cast<ATermAppl>(sort_name))));
    if (is_structured_sort(second) ||
        is_function_sort(second) ||
        is_container_sort(second))
    {
      // We deal here with a declaration of the shape sort A=ComplexType.
      // Rewrite every occurrence of ComplexType to A. Suppose that there are
      // two declarations of the shape sort A=ComplexType; B=ComplexType then
      // ComplexType is rewritten to A and B is also rewritten to A.
      const atermpp::map< sort_expression, basic_sort >::const_iterator j=normalised_aliases.find(second);
      if (j!=normalised_aliases.end())
      {
        normalised_aliases[first]=j->second;
      }
      else
      {
        normalised_aliases[second]=first;
      }
    }
    else
    {
      // We are dealing with a sort declaration of the shape sort A=B.
      // Every occurrence of sort A is normalised to sort B.
      normalised_aliases[first]=second;
    }
  }

  // Close the mapping normalised_aliases under itself. If a rewriting
  // loop is detected, throw a runtime error.

  for (atermpp::map< sort_expression, basic_sort >::iterator i=normalised_aliases.begin();
       i!=normalised_aliases.end(); i++)
  {
    std::set < sort_expression > sort_already_seen;
    sort_expression result_sort=i->second;

    std::set< sort_expression > all_sorts;
    if (is_container_sort(i->first) || is_function_sort(i->first))
    {
      find_sort_expressions(i->first, std::inserter(all_sorts, all_sorts.end()));
    }
    while (normalised_aliases.count(result_sort)>0)
    {
      sort_already_seen.insert(result_sort);
      result_sort= normalised_aliases.find(result_sort)->second;
      if (sort_already_seen.count(result_sort))
      {
        throw mcrl2::runtime_error("Sort alias " + pp(result_sort) + " is defined in terms of itself.");
      }

      for (std::set< sort_expression >::const_iterator j = all_sorts.begin(); j != all_sorts.end(); ++j)
      {
        if (*j==result_sort)
        {
          throw mcrl2::runtime_error("Sort alias " + pp(i->first) + " depends on sort" +
                                     pp(result_sort) + ", which is circularly defined.\n");
        }
      }
    }
    // So the normalised sort of i->first is result_sort.
    i->second=result_sort;
  }
  return normalised_aliases;
}

static sort_expression mapping(sort_expression s,atermpp::map < sort_expression, basic_sort > &m)
{
  if (m.find(s)==m.end())
  {
    return s;
  }
  return m[s];
}

// Under the assumption that constructor_list contains all the constructors, this
// function checks whether there are sorts that must be necessarily empty because they
// have only constructors referring to the domain itself, e.g. sort D; cons f:D->D;
// If such a domain exists a message is printed to stderr and true is returned.
//
// The algorithm works by putting all target sorts of constructors in a set possibly_empty_constructor_sorts.
// Subsequently, those sorts for which there is a constructor function with
// argument sorts not in this set, are removed, until no more sorts can be removed.
// All sorts remaining in possibly_empty_constructor_sorts are empty constructor sorts
// and are reported. If this set is empty, true is reported, i.e. showing no problem.

static bool gstc_check_for_empty_constructor_domains(ATermList constructor_list)
{
  // First add the constructors for structured sorts to the constructor list;
  try
  {
    ATermList defined_sorts=ATtableKeys(context.defined_sorts);
    atermpp::map < sort_expression, basic_sort > normalised_aliases=construct_normalised_aliases();
    std::set< sort_expression > all_sorts;
    for (; defined_sorts!=ATempty; defined_sorts=ATgetNext(defined_sorts))
    {
      const basic_sort s(core::identifier_string(gstcUnwindType(ATAgetFirst(defined_sorts))));
      ATermAppl reference=ATAtableGet(context.defined_sorts,(ATerm)static_cast<ATermAppl>(s.name()));
      // if (is_container_sort(i->first) || is_function_sort(i->first))
      find_sort_expressions(sort_expression(reference), std::inserter(all_sorts, all_sorts.end()));
    }

    for (std::set< sort_expression > ::const_iterator i=all_sorts.begin(); i!=all_sorts.end(); ++i)
    {
      if (is_structured_sort(*i))
      {
        const function_symbol_vector r=structured_sort(*i).constructor_functions();
        for (function_symbol_vector::const_iterator j=r.begin();
             j!=r.end(); ++j)
        {
          constructor_list=ATinsert(constructor_list,(ATerm)static_cast<ATermAppl>(*j));
        }
      }

      if (is_structured_sort(*i))
      {
        const function_symbol_vector r=structured_sort(*i).constructor_functions();
        for (function_symbol_vector::const_iterator i=r.begin();
             i!=r.end(); ++i)
        {
          constructor_list=ATinsert(constructor_list,(ATerm)static_cast<ATermAppl>(*i));
        }
      }

    }

    std::set < sort_expression > possibly_empty_constructor_sorts;
    for (ATermList constructor_list_walker=constructor_list;
         constructor_list_walker!=ATempty; constructor_list_walker=ATgetNext(constructor_list_walker))
    {
      const sort_expression s=function_symbol(ATgetFirst(constructor_list_walker)).sort();
      if (is_function_sort(s))
      {
        // if s is a constant sort, nothing needs to be added.
        possibly_empty_constructor_sorts.insert(mapping(function_sort(s).codomain(),normalised_aliases));
      }
    }

    // Walk through the constructors removing constructor sorts that are not empty,
    // until no more constructors sorts can be removed.
    for (bool stable=false ; !stable ;)
    {
      stable=true;
      for (ATermList constructor_list_walker=constructor_list;
           constructor_list_walker!=ATempty; constructor_list_walker=ATgetNext(constructor_list_walker))
      {
        const sort_expression s=function_symbol(ATgetFirst(constructor_list_walker)).sort();
        if (!is_function_sort(s))
        {
          if (possibly_empty_constructor_sorts.erase(mapping(s,normalised_aliases))==1) // True if one element has been removed.
          {
            stable=false;
          }
        }
        else
        {
          sort_expression_list r=function_sort(s).domain();
          bool has_a_domain_sort_possibly_empty_sorts=false;
          for (sort_expression_list::const_iterator i=r.begin();
               i!=r.end(); ++i)
          {
            if (possibly_empty_constructor_sorts.find(mapping(*i,normalised_aliases))!=possibly_empty_constructor_sorts.end())
            {
              //
              has_a_domain_sort_possibly_empty_sorts=true;
              continue;
            }
          }
          if (!has_a_domain_sort_possibly_empty_sorts)
          {
            // Condition below is true if one element has been removed.
            if (possibly_empty_constructor_sorts.erase(mapping(function_sort(s).codomain(),normalised_aliases))==1)
            {
              stable=false;
            }
          }
        }
      }
    }
    // Print the sorts remaining in possibly_empty_constructor_sorts, as they must be empty
    if (possibly_empty_constructor_sorts.empty())
    {
      return true; // There are no empty sorts
    }
    else
    {
      gsErrorMsg("the following domains are empty due to recursive constructors:\n");
      for (std::set < sort_expression >:: const_iterator i=possibly_empty_constructor_sorts.begin();
           i!=possibly_empty_constructor_sorts.end(); ++i)
      {
        gsErrorMsg("%P\n",(ATerm)static_cast<ATermAppl>(*i));
      }
      return false;
    }
  }
  catch (mcrl2::runtime_error& e)
  {
    gsErrorMsg("%s",e.what());
    return false;
  }

}

static bool gstcReadInFuncs(ATermList Cons, ATermList Maps)
{
  if (gsDebug)
  {
    std::cerr << "Start Read-in Func\n";
  }
  bool Result=true;

  size_t constr_number=ATgetLength(Cons);
  for (ATermList Funcs=ATconcat(Cons,Maps); !ATisEmpty(Funcs); Funcs=ATgetNext(Funcs))
  {
    ATermAppl Func=ATAgetFirst(Funcs);
    ATermAppl FuncName=ATAgetArgument(Func,0);
    ATermAppl FuncType=ATAgetArgument(Func,1);

    if (!gstcIsSortExprDeclared(FuncType))
    {
      return false;
    }

    //if FuncType is a defined function sort, unwind it
    //{ ATermAppl NewFuncType;
    //  if(gsIsSortId(FuncType)
    //   && (NewFuncType=ATAtableGet(context.defined_sorts,(ATerm)ATAgetArgument(FuncType,0)))
    //   && gsIsSortArrow(NewFuncType))
    //  FuncType=NewFuncType;
    //  }

    //if FuncType is a defined function sort, unwind it
    if (gsIsSortId(FuncType))
    {
      ATermAppl NewFuncType=gstcUnwindType(FuncType);
      if (gsIsSortArrow(NewFuncType))
      {
        FuncType=NewFuncType;
      }
    }

    if ((gsIsSortArrow(FuncType)))
    {
      if (!gstcAddFunction(gsMakeOpId(FuncName,FuncType),"function"))
      {
        return false;
      }
    }
    else
    {
      if (!gstcAddConstant(gsMakeOpId(FuncName,FuncType),"constant"))
      {
        gsErrorMsg("could not add constant\n");
        return false;
      }
    }

    if (constr_number)
    {
      constr_number--;

      //Here checks for the constructors
      ATermAppl ConstructorType=FuncType;
      if (gsIsSortArrow(ConstructorType))
      {
        ConstructorType=ATAgetArgument(ConstructorType,1);
      }
      ConstructorType=gstcUnwindType(ConstructorType);
      if (!gsIsSortId(ConstructorType))
      {
        gsErrorMsg("Could not add constructor %P of sort %P. Constructors of a built-in sorts are not allowed.\n",FuncName,FuncType);
        return false;
      }
      if (sort_bool::is_bool(sort_expression(ConstructorType)))
      {
        gsErrorMsg("Could not add constructor %P of sort %P. Constructors of a built-in sorts are not allowed.\n",FuncName,FuncType);
        return false;
      }
      if (sort_pos::is_pos(sort_expression(ConstructorType)))
      {
        gsErrorMsg("Could not add constructor %P of sort %P. Constructors of a built-in sorts are not allowed.\n",FuncName,FuncType);
        return false;
      }
      if (sort_nat::is_nat(sort_expression(ConstructorType)))
      {
        gsErrorMsg("Could not add constructor %P of sort %P. Constructors of a built-in sorts are not allowed.\n",FuncName,FuncType);
        return false;
      }
      if (sort_int::is_int(sort_expression(ConstructorType)))
      {
        gsErrorMsg("Could not add constructor %P of sort %P. Constructors of a built-in sorts are not allowed.\n",FuncName,FuncType);
        return false;
      }
      if (sort_real::is_real(sort_expression(ConstructorType)))
      {
        gsErrorMsg("Could not add constructor %P of sort %P. Constructors of a built-in sorts are not allowed.\n",FuncName,FuncType);
        return false;
      }
    }

    if (gsDebug)
    {
      std::cerr << "Read-in Func " << pp(FuncName) << ", Types " << pp(FuncType) << "\n";
    }
  }

  // Check that the constructors are defined such that they cannot generate an empty sort.
  // E.g. in the specification sort D; cons f:D->D; the sort D must be necessarily empty, which is
  // forbidden. The function below checks whether such malicious specifications occur.

  if (!gstc_check_for_empty_constructor_domains(Cons))
  {
    return false;
  }

  return Result;
}

static bool gstcReadInActs(ATermList Acts)
{
  bool Result=true;
  for (; !ATisEmpty(Acts); Acts=ATgetNext(Acts))
  {
    ATermAppl Act=ATAgetFirst(Acts);
    ATermAppl ActName=ATAgetArgument(Act,0);
    ATermList ActType=ATLgetArgument(Act,1);

    if (!gstcIsSortExprListDeclared(ActType))
    {
      return false;
    }

    ATermList Types=ATLtableGet(context.actions, (ATerm)ActName);
    if (!Types)
    {
      Types=ATmakeList1((ATerm)ActType);
    }
    else
    {
      // the table context.actions contains a list of types for each
      // action name. We need to check if there is already such a type
      // in the list. If so -- error, otherwise -- add
      if (gstcInTypesL(ActType, Types))
      {
        gsErrorMsg("double declaration of action %P\n", ActName);
        return false;
      }
      else
      {
        Types=ATappend(Types,(ATerm)ActType);
      }
    }
    ATtablePut(context.actions,(ATerm)ActName,(ATerm)Types);
    if (gsDebug)
    {
      std::cerr << "Read-in Act Name " << pp(ActName) << ", Types ";
      ATfprintf(stderr,"%t\n",Types); // Types is a list of list, on which pp does not work.
    }
  }

  return Result;
}

static bool gstcReadInProcsAndInit(ATermList Procs, ATermAppl Init)
{
  bool Result=true;
  for (; !ATisEmpty(Procs); Procs=ATgetNext(Procs))
  {
    ATermAppl Proc=ATAgetFirst(Procs);
    ATermAppl ProcName=ATAgetArgument(ATAgetArgument(Proc,0),0);

    if (ATLtableGet(context.actions, (ATerm)ProcName))
    {
      gsErrorMsg("declaration of both process and action %P\n", ProcName);
      return false;
    }

    ATermList ProcType=ATLgetArgument(ATAgetArgument(Proc,0),1);

    if (!gstcIsSortExprListDeclared(ProcType))
    {
      return false;
    }

    ATermList Types=ATLtableGet(context.processes,(ATerm)ProcName);
    if (!Types)
    {
      Types=ATmakeList1((ATerm)ProcType);
    }
    else
    {
      // the table context.processes contains a list of types for each
      // process name. We need to check if there is already such a type
      // in the list. If so -- error, otherwise -- add
      if (gstcInTypesL(ProcType, Types))
      {
        gsErrorMsg("double declaration of process %P\n", ProcName);
        return false;
      }
      else
      {
        Types=ATappend(Types,(ATerm)ProcType);
      }
    }
    ATtablePut(context.processes,(ATerm)ProcName,(ATerm)Types);

    //check that all formal parameters of the process are unique.
    ATermList ProcVars=ATLgetArgument(Proc,1);
    if (!gstcVarsUnique(ProcVars))
    {
      gsErrorMsg("the formal variables in process %P are not unique\n",Proc);
      return false;
    }

    ATtablePut(body.proc_pars,(ATerm)ATAgetArgument(Proc,0),(ATerm)ATLgetArgument(Proc,1));
    ATtablePut(body.proc_bodies,(ATerm)ATAgetArgument(Proc,0),(ATerm)ATAgetArgument(Proc,2));
    if (gsDebug)
    {
      std::cerr << "Read-in Proc Name " << pp(ProcName) << ", Types " ;
      ATfprintf(stderr,"%t\n",Types); // Types is a list of list, on which pp does not work.
    }
  }
  ATtablePut(body.proc_pars,(ATerm)INIT_KEY(),(ATerm)ATmakeList0());
  ATtablePut(body.proc_bodies,(ATerm)INIT_KEY(),(ATerm)Init);

  return Result;
}

static bool gstcReadInPBESAndInit(ATermAppl PBEqnSpec, ATermAppl PBInit)
{
  bool Result=true;

  //ATermList PBFreeVars=ATLgetArgument(GlobVarSpec,0);
  ATermList PBEqns=ATLgetArgument(PBEqnSpec,0);

  for (; !ATisEmpty(PBEqns); PBEqns=ATgetNext(PBEqns))
  {
    ATermAppl PBEqn=ATAgetFirst(PBEqns);
    ATermAppl PBName=ATAgetArgument(ATAgetArgument(PBEqn,1),0);

    ATermList PBVars=ATLgetArgument(ATAgetArgument(PBEqn,1),1);

    ATermList PBType=ATmakeList0();
    for (ATermList l=PBVars; !ATisEmpty(l); l=ATgetNext(l))
    {
      PBType=ATinsert(PBType,(ATerm)ATAgetArgument(ATAgetFirst(l),1));
    }
    PBType=ATreverse(PBType);

    if (!gstcIsSortExprListDeclared(PBType))
    {
      return false;
    }

    ATermList Types=ATLtableGet(context.PBs,(ATerm)PBName);
    if (!Types)
    {
      Types=ATmakeList1((ATerm)PBType);
    }
    else
    {
      // temporarily prohibit overloading here
      gsErrorMsg("attempt to overload propositional variable %P\n", PBName);
      return false;
      // the table context.PBs contains a list of types for each
      // PBES name. We need to check if there is already such a type
      // in the list. If so -- error, otherwise -- add
      if (gstcInTypesL(PBType, Types))
      {
        gsErrorMsg("double declaration of propositional variable %P\n", PBName);
        return false;
      }
      else
      {
        Types=ATappend(Types,(ATerm)PBType);
      }
    }
    ATtablePut(context.PBs,(ATerm)PBName,(ATerm)Types);

    //check that all formal parameters of the PBES are unique.
    //if(!gstcVarsUnique(ATconcat(PBVars,PBFreeVars))){ gsErrorMsg("the formal and/or global variables in PBES %P are not unique\n",PBEqn); return ATfalse;}

    //This is a fake ProcVarId (There is no PBVarId)
    ATermAppl Index=gsMakeProcVarId(PBName,PBType);
    //ATtablePut(body.proc_freevars,(ATerm)Index,(ATerm)PBFreeVars);
    ATtablePut(body.proc_pars,(ATerm)Index,(ATerm)PBVars);
    ATtablePut(body.proc_bodies,(ATerm)Index,(ATerm)ATAgetArgument(PBEqn,2));
    if (gsDebug)
    {
      std::cerr << "Read-in Proc Name " << pp(PBName) << ", Types ";
      ATfprintf(stderr,"%t\n",Types); // pp cannot print list of lists of types
    }
  }
  //ATtablePut(body.proc_freevars,(ATerm)INIT_KEY(),(ATerm)PBFreeVars);
  ATtablePut(body.proc_pars,(ATerm)INIT_KEY(),(ATerm)ATmakeList0());
  ATtablePut(body.proc_bodies,(ATerm)INIT_KEY(),(ATerm)ATAgetArgument(PBInit,0));

  return Result;
}

static ATermList gstcWriteProcs(ATermList oldprocs)
{
  ATermList Result=ATmakeList0();
  for (ATermList l=oldprocs; !ATisEmpty(l); l=ATgetNext(l))
  {
    ATermAppl ProcVar=ATAgetArgument(ATAgetFirst(l),0);
    if (ProcVar==INIT_KEY())
    {
      continue;
    }
    Result=ATinsert(Result,(ATerm)gsMakeProcEqn(ProcVar,
                    ATLtableGet(body.proc_pars,(ATerm)ProcVar),
                    ATAtableGet(body.proc_bodies,(ATerm)ProcVar)
                                               )
                   );
  }
  Result=ATreverse(Result);
  return Result;
}

static ATermList gstcWritePBES(ATermList oldPBES)
{
  ATermList Result=ATmakeList0();
  for (ATermList PBEqns=oldPBES; !ATisEmpty(PBEqns); PBEqns=ATgetNext(PBEqns))
  {
    ATermAppl PBEqn=ATAgetFirst(PBEqns);
    ATermAppl PBESVar=ATAgetArgument(PBEqn,1);

    ATermList PBType=ATmakeList0();
    for (ATermList l=ATLgetArgument(PBESVar,1); !ATisEmpty(l); l=ATgetNext(l))
    {
      PBType=ATinsert(PBType,(ATerm)ATAgetArgument(ATAgetFirst(l),1));
    }
    PBType=ATreverse(PBType);

    ATermAppl Index=gsMakeProcVarId(ATAgetArgument(PBESVar,0),PBType);

    if (Index==INIT_KEY())
    {
      continue;
    }
    Result=ATinsert(Result,(ATerm)ATsetArgument(PBEqn,(ATerm)ATAtableGet(body.proc_bodies,(ATerm)Index),2));
  }
  return ATreverse(Result);
}


static bool gstcTransformVarConsTypeData(void)
{
  bool Result=true;
  ATermTable DeclaredVars=ATtableCreate(63,50);
  ATermTable FreeVars=ATtableCreate(63,50);

  //data terms in equations
  ATermList NewEqns=ATmakeList0();
  bool b = true;
  for (ATermList Eqns=body.equations; !ATisEmpty(Eqns); Eqns=ATgetNext(Eqns))
  {
    ATermAppl Eqn=ATAgetFirst(Eqns);
    ATermList VarList=ATLgetArgument(Eqn,0);

    if (!gstcVarsUnique(VarList))
    {
      b = false;
      gsErrorMsg("the variables in equation declaration %P are not unique\n",VarList,Eqn);
      break;
    }

    ATermTable NewDeclaredVars=gstcAddVars2Table(DeclaredVars,VarList);
    if (!NewDeclaredVars)
    {
      b = false;
      break;
    }
    else
    {
      DeclaredVars=NewDeclaredVars;
    }

    ATermAppl Left=ATAgetArgument(Eqn,2);
    ATermAppl LeftType=gstcTraverseVarConsTypeD(DeclaredVars,DeclaredVars,&Left,data::unknown_sort(),FreeVars,false,true);
    if (!LeftType)
    {
      b = false;
      gsErrorMsg("error occurred while typechecking %P as left hand side of equation %P\n",Left,Eqn);
      break;
    }
    if (was_warning_upcasting)
    {
      was_warning_upcasting=false;
      gsWarningMsg("warning occurred while typechecking %P as left hand side of equation %P\n",Left,Eqn);
    }

    ATermAppl Cond=ATAgetArgument(Eqn,1);
    if (!gstcTraverseVarConsTypeD(DeclaredVars,FreeVars,&Cond,sort_bool::bool_()))
    {
      b = false;
      break;
    }
    ATermAppl Right=ATAgetArgument(Eqn,3);
    ATermAppl RightType=gstcTraverseVarConsTypeD(DeclaredVars,FreeVars,&Right,LeftType,NULL,false);
    if (!RightType)
    {
      b = false;
      gsErrorMsg("error occurred while typechecking %P as right hand side of equation %P\n",Right,Eqn);
      break;
    }

    //If the types are not uniquely the same now: do once more:
    if (!gstcEqTypesA(LeftType,RightType))
    {
      // if (gsDebug) { std::cerr << "Doing again for the equation %P, LeftType: %P, RightType: %P\n",Eqn,LeftType,RightType); }
      ATermAppl Type=gstcTypeMatchA(LeftType,RightType);
      if (!Type)
      {
        gsErrorMsg("types of the left- (%P) and right- (%P) hand-sides of the equation %P do not match\n",LeftType,RightType,Eqn);
        b = false;
        break;
      }
      Left=ATAgetArgument(Eqn,2);
      ATtableReset(FreeVars);
      LeftType=gstcTraverseVarConsTypeD(DeclaredVars,DeclaredVars,&Left,Type,FreeVars,true);
      if (!LeftType)
      {
        b = false;
        gsErrorMsg("types of the left- and right-hand-sides of the equation %P do not match\n",Eqn);
        break;
      }
      if (was_warning_upcasting)
      {
        was_warning_upcasting=false;
        gsWarningMsg("warning occurred while typechecking %P as left hand side of equation %P\n",Left,Eqn);
      }
      Right=ATAgetArgument(Eqn,3);
      RightType=gstcTraverseVarConsTypeD(DeclaredVars,DeclaredVars,&Right,LeftType,FreeVars);
      if (!RightType)
      {
        b = false;
        gsErrorMsg("types of the left- and right-hand-sides of the equation %P do not match\n",Eqn);
        break;
      }
      Type=gstcTypeMatchA(LeftType,RightType);
      if (!Type)
      {
        gsErrorMsg("types of the left- (%P) and right- (%P) hand-sides of the equation %P do not match\n",LeftType,RightType,Eqn);
        b = false;
        break;
      }
      if (gstcHasUnknown(Type))
      {
        gsErrorMsg("types of the left- (%P) and right- (%P) hand-sides of the equation %P cannot be uniquely determined\n",LeftType,RightType,Eqn);
        b = false;
        break;
      }
    }
    ATtableReset(DeclaredVars);
    NewEqns=ATinsert(NewEqns,(ATerm)gsMakeDataEqn(VarList,Cond,Left,Right));
  }
  if (b)
  {
    body.equations=ATreverse(NewEqns);
  }

  ATtableDestroy(FreeVars);
  ATtableDestroy(DeclaredVars);
  return b?Result:false;
}

static bool gstcTransformActProcVarConst(void)
{
  bool Result=true;
  ATermTable Vars=ATtableCreate(63,50);

  //process and data terms in processes and init
  for (ATermList ProcVars=ATtableKeys(body.proc_pars); !ATisEmpty(ProcVars); ProcVars=ATgetNext(ProcVars))
  {
    ATermAppl ProcVar=ATAgetFirst(ProcVars);
    ATtableReset(Vars);
    gstcATermTableCopy(context.glob_vars,Vars);

    ATermTable NewVars=gstcAddVars2Table(Vars,ATLtableGet(body.proc_pars,(ATerm)ProcVar));
    if (!NewVars)
    {
      Result = false;
      break;
    }
    else
    {
      Vars=NewVars;
    }

    ATermAppl NewProcTerm=gstcTraverseActProcVarConstP(Vars,ATAtableGet(body.proc_bodies,(ATerm)ProcVar));
    if (!NewProcTerm)
    {
      Result = false;
      break;
    }
    ATtablePut(body.proc_bodies,(ATerm)ProcVar,(ATerm)NewProcTerm);
  }

  ATtableDestroy(Vars);
  return Result;
}

static bool gstcTransformPBESVarConst(void)
{
  bool Result=true;
  ATermTable Vars=ATtableCreate(63,50);

  //PBEs and data terms in PBEqns and init
  for (ATermList PBVars=ATtableKeys(body.proc_pars); !ATisEmpty(PBVars); PBVars=ATgetNext(PBVars))
  {
    ATermAppl PBVar=ATAgetFirst(PBVars);
    ATtableReset(Vars);
    gstcATermTableCopy(context.glob_vars,Vars);

    ATermTable NewVars=gstcAddVars2Table(Vars,ATLtableGet(body.proc_pars,(ATerm)PBVar));
    if (!NewVars)
    {
      Result = false;
      break;
    }
    else
    {
      Vars=NewVars;
    }

    ATermAppl NewPBTerm=gstcTraversePBESVarConstPB(Vars,ATAtableGet(body.proc_bodies,(ATerm)PBVar));
    if (!NewPBTerm)
    {
      Result = false;
      break;
    }
    ATtablePut(body.proc_bodies,(ATerm)PBVar,(ATerm)NewPBTerm);
  }

  ATtableDestroy(Vars);
  return Result;
}


// ======== Auxiliary functions
static bool gstcInTypesA(ATermAppl Type, ATermList Types)
{
  for (; !ATisEmpty(Types); Types=ATgetNext(Types))
    if (gstcEqTypesA(Type,ATAgetFirst(Types)))
    {
      return true;
    }
  return false;
}

static bool gstcEqTypesA(ATermAppl Type1, ATermAppl Type2)
{
  if (ATisEqual(Type1, Type2))
  {
    return true;
  }

  if (!Type1 || !Type2)
  {
    return false;
  }

  return ATisEqual(gstcUnwindType(Type1),gstcUnwindType(Type2));
}

static bool gstcInTypesL(ATermList Type, ATermList Types)
{
  for (; !ATisEmpty(Types); Types=ATgetNext(Types))
    if (gstcEqTypesL(Type,ATLgetFirst(Types)))
    {
      return true;
    }
  return false;
}

static bool gstcEqTypesL(ATermList Type1, ATermList Type2)
{
  if (ATisEqual(Type1, Type2))
  {
    return true;
  }
  if (!Type1 || !Type2)
  {
    return false;
  }
  if (ATgetLength(Type1)!=ATgetLength(Type2))
  {
    return false;
  }
  for (; !ATisEmpty(Type1); Type1=ATgetNext(Type1),Type2=ATgetNext(Type2))
    if (!gstcEqTypesA(ATAgetFirst(Type1),ATAgetFirst(Type2)))
    {
      return false;
    }
  return true;
}

static bool gstcIsSortDeclared(ATermAppl SortName)
{

  // if (gsDebug) { std::cerr << "gstcIsSortDeclared: SortName %P\n",SortName);

  if (sort_bool::is_bool(basic_sort(core::identifier_string(SortName))) ||
      sort_pos::is_pos(basic_sort(core::identifier_string(SortName))) ||
      sort_nat::is_nat(basic_sort(core::identifier_string(SortName))) ||
      sort_int::is_int(basic_sort(core::identifier_string(SortName))) ||
      sort_real::is_real(basic_sort(core::identifier_string(SortName))))
  {
    return true;
  }
  if (ATindexedSetGetIndex(context.basic_sorts, (ATerm)SortName)>=0)
  {
    return true;
  }
  if (ATAtableGet(context.defined_sorts,(ATerm)SortName))
  {
    return true;
  }
  return false;
}

static bool gstcIsSortExprDeclared(ATermAppl SortExpr)
{
  if (gsIsSortId(SortExpr))
  {
    ATermAppl SortName=ATAgetArgument(SortExpr,0);
    if (!gstcIsSortDeclared(SortName))
    {
      gsErrorMsg("basic or defined sort %P is not declared\n",SortName);
      return false;
    }
    return true;
  }

  if (gsIsSortCons(SortExpr))
  {
    return gstcIsSortExprDeclared(ATAgetArgument(SortExpr, 1));
  }

  if (gsIsSortArrow(SortExpr))
  {
    if (!gstcIsSortExprDeclared(ATAgetArgument(SortExpr,1)))
    {
      return false;
    }
    if (!gstcIsSortExprListDeclared(ATLgetArgument(SortExpr,0)))
    {
      return false;
    }
    return true;
  }

  if (gsIsSortStruct(SortExpr))
  {
    for (ATermList Constrs=ATLgetArgument(SortExpr,0); !ATisEmpty(Constrs); Constrs=ATgetNext(Constrs))
    {
      ATermAppl Constr=ATAgetFirst(Constrs);

      ATermList Projs=ATLgetArgument(Constr,1);
      for (; !ATisEmpty(Projs); Projs=ATgetNext(Projs))
      {
        ATermAppl Proj=ATAgetFirst(Projs);
        ATermAppl ProjSort=ATAgetArgument(Proj,1);

        // not to forget, recursive call for ProjSort ;-)
        if (!gstcIsSortExprDeclared(ProjSort))
        {
          return false;
        }
      }
    }
    return true;
  }

  assert(0);
  gsErrorMsg("this is not a sort expression %T\n",SortExpr);
  return false;
}

static bool gstcIsSortExprListDeclared(ATermList SortExprList)
{
  for (; !ATisEmpty(SortExprList); SortExprList=ATgetNext(SortExprList))
    if (!gstcIsSortExprDeclared(ATAgetFirst(SortExprList)))
    {
      return false;
    }
  return true;
}


static bool gstcReadInSortStruct(ATermAppl SortExpr)
{
  bool Result=true;

  if (gsIsSortId(SortExpr))
  {
    ATermAppl SortName=ATAgetArgument(SortExpr,0);
    if (!gstcIsSortDeclared(SortName))
    {
      gsErrorMsg("basic or defined sort %P is not declared\n",SortName);
      return false;
    }
    return true;
  }

  if (gsIsSortCons(SortExpr))
  {
    return gstcReadInSortStruct(ATAgetArgument(SortExpr,1));
  }

  if (gsIsSortArrow(SortExpr))
  {
    if (!gstcReadInSortStruct(ATAgetArgument(SortExpr,1)))
    {
      return false;
    }
    for (ATermList Sorts=ATLgetArgument(SortExpr,0); !ATisEmpty(Sorts); Sorts=ATgetNext(Sorts))
    {
      if (!gstcReadInSortStruct(ATAgetFirst(Sorts)))
      {
        return false;
      }
    }
    return true;
  }

  if (gsIsSortStruct(SortExpr))
  {
    for (ATermList Constrs=ATLgetArgument(SortExpr,0); !ATisEmpty(Constrs); Constrs=ATgetNext(Constrs))
    {
      ATermAppl Constr=ATAgetFirst(Constrs);

      // recognizer -- if present -- a function from SortExpr to Bool
      ATermAppl Name=ATAgetArgument(Constr,2);
      if (!gsIsNil(Name) &&
          !gstcAddFunction(gsMakeOpId(Name,gsMakeSortArrow(ATmakeList1((ATerm)SortExpr),sort_bool::bool_())),"recognizer"))
      {
        return false;
      }

      // constructor type and projections
      ATermList Projs=ATLgetArgument(Constr,1);
      Name=ATAgetArgument(Constr,0);
      if (ATisEmpty(Projs))
      {
        if (!gstcAddConstant(gsMakeOpId(Name,SortExpr),"constructor constant"))
        {
          return false;
        }
        else
        {
          continue;
        }
      }

      ATermList ConstructorType=ATmakeList0();
      for (; !ATisEmpty(Projs); Projs=ATgetNext(Projs))
      {
        ATermAppl Proj=ATAgetFirst(Projs);
        ATermAppl ProjSort=ATAgetArgument(Proj,1);

        // not to forget, recursive call for ProjSort ;-)
        if (!gstcReadInSortStruct(ProjSort))
        {
          return false;
        }

        ATermAppl ProjName=ATAgetArgument(Proj,0);
        if (!gsIsNil(ProjName) &&
            !gstcAddFunction(gsMakeOpId(ProjName,gsMakeSortArrow(ATmakeList1((ATerm)SortExpr),ProjSort)),"projection",true))
        {
          return false;
        }
        ConstructorType=ATinsert(ConstructorType,(ATerm)ProjSort);
      }
      if (!gstcAddFunction(gsMakeOpId(Name,gsMakeSortArrow(ATreverse(ConstructorType),SortExpr)),"constructor"))
      {
        return false;
      }
    }
    return true;
  }

  assert(0);
  return Result;
}

static bool gstcAddConstant(ATermAppl OpId, const char* msg)
{
  assert(gsIsOpId(OpId));
  bool Result=true;

  ATermAppl Name = function_symbol(OpId).name();
  ATermAppl Sort = function_symbol(OpId).sort();

  if (ATAtableGet(context.constants, (ATerm)Name) /*|| ATLtableGet(context.functions, (ATerm)Name)*/)
  {
    gsErrorMsg("double declaration of %s %P\n", msg, Name);
    return false;
  }

  if (ATLtableGet(gssystem.constants, (ATerm)Name) || ATLtableGet(gssystem.functions, (ATerm)Name))
  {
    gsErrorMsg("attempt to declare a constant with the name that is a built-in identifier (%P)\n", Name);
    return false;
  }

  ATtablePut(context.constants, (ATerm)Name, (ATerm)Sort);
  return Result;
}

static bool gstcAddFunction(ATermAppl OpId, const char* msg, bool allow_double_decls)
{
  assert(gsIsOpId(OpId));
  bool Result=true;
  const function_symbol f(OpId);
  const sort_expression_list domain=function_sort(f.sort()).domain();
  ATermAppl Name = f.name();
  ATermAppl Sort = f.sort();

  //constants and functions can have the same names
  //  if(ATAtableGet(context.constants, (ATerm)Name)){
  //    ThrowMF("Double declaration of constant and %s %T\n", msg, Name);
  //  }

  if (domain.size()==0)
  {
    if (ATAtableGet(gssystem.constants, (ATerm)Name))
    {
      gsErrorMsg("attempt to redeclare the system constant with %s %P\n", msg, OpId);
      return false;
    }
  }
  else // domain.size()>0
  {
    ATermList L=ATLtableGet(gssystem.functions, (ATerm)Name);
    for (; L!=NULL && L!=ATempty ; L=ATgetNext(L))
    {
      if (gstcTypeMatchA(Sort,(ATermAppl)ATgetFirst(L))!=NULL)
      {
        // f matches a predefined function
        gsErrorMsg("attempt to redeclare a system function with %s %P:%P\n", msg, OpId,Sort);
        return false;
      }
    }
  }

  ATermList Types=ATLtableGet(context.functions, (ATerm)Name);
  // the table context.functions contains a list of types for each
  // function name. We need to check if there is already such a type
  // in the list. If so -- error, otherwise -- add
  if (Types && gstcInTypesA(Sort, Types))
  {
    if (!allow_double_decls)
    {
      gsErrorMsg("double declaration of %s %P\n", msg, Name);
      return false;
    }
  }
  else
  {
    if (!Types)
    {
      Types=ATmakeList0();
    }
    Types=ATappend(Types,(ATerm)Sort);
    ATtablePut(context.functions,(ATerm)Name,(ATerm)Types);
  }
  if (gsDebug)
  {
    std::cerr << "Read-in " << msg << " " << pp(Name) << ". Type " << pp(Types) << "\n";
  }
  return Result;
}

static void gstcAddSystemConstant(ATermAppl OpId)
{
  //Pre: OpId is an OpId
  // append the Type to the entry of the Name of the OpId in gssystem.constants table
  assert(gsIsOpId(OpId));
  ATermAppl OpIdName = function_symbol(OpId).name();
  ATermAppl Type = function_symbol(OpId).sort();

  ATermList Types=ATLtableGet(gssystem.constants, (ATerm)OpIdName);

  if (!Types)
  {
    Types=ATmakeList0();
  }
  Types=ATappend(Types,(ATerm)Type);
  ATtablePut(gssystem.constants,(ATerm)OpIdName,(ATerm)Types);
}

static void gstcAddSystemFunction(ATermAppl OpId)
{
  //Pre: OpId is an OpId
  // append the Type to the entry of the Name of the OpId in gssystem.functions table
  assert(gsIsOpId(OpId));
  ATermAppl OpIdName = function_symbol(OpId).name();
  ATermAppl Type = function_symbol(OpId).sort();
  assert(gsIsSortArrow(Type));

  ATermList Types=ATLtableGet(gssystem.functions, (ATerm)OpIdName);

  if (!Types)
  {
    Types=ATmakeList0();
  }
  Types=ATappend(Types,(ATerm)Type);  // TODO: Avoid ATappend!!!! But the order is essential.
  ATtablePut(gssystem.functions,(ATerm)OpIdName,(ATerm)Types);
}

static void gstcATermTableCopy(ATermTable Orig, ATermTable Copy)
{
  for (ATermList Keys=ATtableKeys(Orig); !ATisEmpty(Keys); Keys=ATgetNext(Keys))
  {
    ATerm Key=ATgetFirst(Keys);
    ATtablePut(Copy,Key,ATtableGet(Orig,Key));
  }
}


static bool gstcVarsUnique(ATermList VarDecls)
{
  bool Result=true;
  ATermIndexedSet Temp=ATindexedSetCreate(63,50);

  for (; !ATisEmpty(VarDecls); VarDecls=ATgetNext(VarDecls))
  {
    ATermAppl VarDecl=ATAgetFirst(VarDecls);
    ATermAppl VarName=ATAgetArgument(VarDecl,0);
    // if already defined -- replace (other option -- warning)
    // if variable name is a constant name -- it has more priority (other options -- warning, error)
    bool nnew;
    ATindexedSetPut(Temp, (ATerm)VarName, &nnew);
    if (!nnew)
    {
      Result=false;
      goto final;
    }
  }

final:
  ATindexedSetDestroy(Temp);
  return Result;
}

static ATermTable gstcAddVars2Table(ATermTable Vars, ATermList VarDecls)
{
  for (; !ATisEmpty(VarDecls); VarDecls=ATgetNext(VarDecls))
  {
    ATermAppl VarDecl=ATAgetFirst(VarDecls);
    ATermAppl VarName=ATAgetArgument(VarDecl,0);
    ATermAppl VarType=ATAgetArgument(VarDecl,1);
    //test the type
    if (!gstcIsSortExprDeclared(VarType))
    {
      return NULL;
    }

    // if already defined -- replace (other option -- warning)
    // if variable name is a constant name -- it has more priority (other options -- warning, error)
    ATtablePut(Vars, (ATerm)VarName, (ATerm)VarType);
  }

  return Vars;
}

static ATermTable gstcRemoveVars(ATermTable Vars, ATermList VarDecls)
{
  for (; !ATisEmpty(VarDecls); VarDecls=ATgetNext(VarDecls))
  {
    ATermAppl VarDecl=ATAgetFirst(VarDecls);
    ATermAppl VarName=ATAgetArgument(VarDecl,0);
    //ATermAppl VarType=ATAgetArgument(VarDecl,1);

    ATtableRemove(Vars, (ATerm)VarName);
  }

  return Vars;
}

static ATermAppl gstcRewrActProc(ATermTable Vars, ATermAppl ProcTerm, bool is_pbes)
{
  ATermAppl Result=NULL;
  ATermAppl Name=ATAgetArgument(ProcTerm,0);
  ATermList ParList;

  bool action=false;

  if (!is_pbes)
  {
    if ((ParList=ATLtableGet(context.actions,(ATerm)Name)))
    {
      action=true;
    }
    else
    {
      if ((ParList=ATLtableGet(context.processes,(ATerm)Name)))
      {
        action=false;
      }
      else
      {
        gsErrorMsg("action or process %P not declared\n", Name);
        return NULL;
      }
    }
  }
  else
  {
    if (!(ParList=ATLtableGet(context.PBs,(ATerm)Name)))
    {
      gsErrorMsg("propositional variable %P not declared\n", Name);
      return NULL;
    }
  }
  assert(!ATisEmpty(ParList));

  size_t nFactPars=ATgetLength(ATLgetArgument(ProcTerm,1));
  const char* msg=(is_pbes)?"propositional variable":((action)?"action":"process");


  //filter the list of lists ParList to keep only the lists of lenth nFactPars
  {
    ATermList NewParList=ATmakeList0();
    for (; !ATisEmpty(ParList); ParList=ATgetNext(ParList))
    {
      ATermList Par=ATLgetFirst(ParList);
      if (ATgetLength(Par)==nFactPars)
      {
        NewParList=ATinsert(NewParList,(ATerm)Par);
      }
    }
    ParList=ATreverse(NewParList);
  }

  if (ATisEmpty(ParList))
  {
    gsErrorMsg("no %s %P with %d parameter%s is declared (while typechecking %P)\n",
               msg, Name, nFactPars, (nFactPars != 1)?"s":"", ProcTerm);
    return NULL;
  }

  if (ATgetLength(ParList)==1)
  {
    Result=gstcMakeActionOrProc(action,Name,ATLgetFirst(ParList),ATLgetArgument(ProcTerm,1));
  }
  else
  {
    // we need typechecking to find the correct type of the action.
    // make the list of possible types for the parameters
    Result=gstcMakeActionOrProc(action,Name,gstcGetNotInferredList(ParList),ATLgetArgument(ProcTerm,1));
  }

  //process the arguments

  //possible types for the arguments of the action. (not inferred if ambiguous action).
  ATermList PosTypeList=ATLgetArgument(ATAgetArgument(Result,0),1);

  ATermList NewPars=ATmakeList0();
  ATermList NewPosTypeList=ATmakeList0();
  for (ATermList Pars=ATLgetArgument(ProcTerm,1); !ATisEmpty(Pars); Pars=ATgetNext(Pars),PosTypeList=ATgetNext(PosTypeList))
  {
    ATermAppl Par=ATAgetFirst(Pars);
    ATermAppl PosType=ATAgetFirst(PosTypeList);

    ATermAppl NewPosType=gstcTraverseVarConsTypeD(Vars,Vars,&Par,PosType); //gstcExpandNumTypesDown(PosType));

    if (!NewPosType)
    {
      gsErrorMsg("cannot typecheck %P as type %P (while typechecking %P)\n",Par,gstcExpandNumTypesDown(PosType),ProcTerm);
      return NULL;
    }
    NewPars=ATinsert(NewPars,(ATerm)Par);
    NewPosTypeList=ATinsert(NewPosTypeList,(ATerm)NewPosType);
  }
  NewPars=ATreverse(NewPars);
  NewPosTypeList=ATreverse(NewPosTypeList);

  PosTypeList=gstcAdjustNotInferredList(NewPosTypeList,ParList);

  if (!PosTypeList)
  {
    PosTypeList=ATLgetArgument(ATAgetArgument(Result,0),1);
    ATermList Pars=NewPars;
    NewPars=ATmakeList0();
    ATermList CastedPosTypeList=ATmakeList0();
    for (; !ATisEmpty(Pars); Pars=ATgetNext(Pars),PosTypeList=ATgetNext(PosTypeList),NewPosTypeList=ATgetNext(NewPosTypeList))
    {
      ATermAppl Par=ATAgetFirst(Pars);
      ATermAppl PosType=ATAgetFirst(PosTypeList);
      ATermAppl NewPosType=ATAgetFirst(NewPosTypeList);

      ATermAppl CastedNewPosType=gstcUpCastNumericType(PosType,NewPosType,&Par);
      if (!CastedNewPosType)
      {
        gsErrorMsg("cannot cast %P to %P (while typechecking %P in %P)\n",NewPosType,PosType,Par,ProcTerm);
        return NULL;
      }

      NewPars=ATinsert(NewPars,(ATerm)Par);
      CastedPosTypeList=ATinsert(CastedPosTypeList,(ATerm)CastedNewPosType);
    }
    NewPars=ATreverse(NewPars);
    NewPosTypeList=ATreverse(CastedPosTypeList);

    PosTypeList=gstcAdjustNotInferredList(NewPosTypeList,ParList);
  }

  if (!PosTypeList)
  {
    gsErrorMsg("no %s %P with type %P is declared (while typechecking %P)\n",msg,Name,NewPosTypeList,ProcTerm);
    return NULL;
  }

  if (gstcIsNotInferredL(PosTypeList))
  {
    gsErrorMsg("ambiguous %s %P\n",msg,Name);
    return NULL;
  }

  Result=gstcMakeActionOrProc(action,Name,PosTypeList,NewPars);

  if (is_pbes)
  {
    Result=ATsetArgument(ProcTerm,(ATerm)NewPars,1);
  }

  // if (gsDebug) { std::cerr << "recognized %s %T\n",msg,Result);
  return Result;
}

static inline ATermAppl gstcMakeActionOrProc(bool action, ATermAppl Name,
    ATermList FormParList, ATermList FactParList)
{
  return (action)?gsMakeAction(gsMakeActId(Name,FormParList),FactParList)
         :gsMakeProcess(gsMakeProcVarId(Name,FormParList),FactParList);
}

static ATermAppl gstcTraverseActProcVarConstP(ATermTable Vars, ATermAppl ProcTerm)
{
  ATermAppl Result=NULL;
  size_t n = ATgetArity(ATgetAFun(ProcTerm));
  if (n==0)
  {
    return ProcTerm;
  }

  //Here the code for short-hand assignments begins.
  if (gsIsIdAssignment(ProcTerm))
  {
    if (gsDebug) { std::cerr << "typechecking a process call with short-hand assignments " << ProcTerm << "\n"; }
    ATermAppl Name=ATAgetArgument(ProcTerm,0);
    ATermList ParList=ATLtableGet(context.processes,(ATerm)Name);
    if (!ParList)
    {
      gsErrorMsg("process %P not declared\n", Name);
      return NULL;
    }

    // Put the assignments into a table
    ATermTable As=ATtableCreate(63,50);
    for (ATermList l=ATLgetArgument(ProcTerm,1); !ATisEmpty(l); l=ATgetNext(l))
    {
      ATermAppl a=ATAgetFirst(l);
      ATerm existing_rhs = ATtableGet(As,(ATerm)ATAgetArgument(a,0));
      if (existing_rhs == NULL) // An assignment of the shape x:=t does not yet exist, this is OK.
      { 
        ATtablePut(As,(ATerm)ATAgetArgument(a,0),(ATerm)ATAgetArgument(a,1));
      }
      else
      {
        gsErrorMsg("Double assignment to variable %P (detected assigned values are %P and %P)\n",
                          ATAgetArgument(a,0),existing_rhs,ATAgetArgument(a,1));
        return NULL;
      }
    }

    {
      // Now filter the ParList to contain only the processes with parameters in this process call with assignments
      ATermList NewParList=ATmakeList0();
      assert(!ATisEmpty(ParList));
      ATermAppl Culprit=NULL; // Variable used for more intelligible error messages.
      for (; !ATisEmpty(ParList); ParList=ATgetNext(ParList))
      {
        ATermList Par=ATLgetFirst(ParList);

        // get the formal parameter names
        ATermList FormalPars=ATLtableGet(body.proc_pars,(ATerm)gsMakeProcVarId(Name,Par));
        // we only need the names of the parameters, not the types
        ATermList FormalParNames=ATmakeList0();
        for (; !ATisEmpty(FormalPars); FormalPars=ATgetNext(FormalPars))
        {
          FormalParNames=ATinsert(FormalParNames,(ATerm)ATAgetArgument(ATAgetFirst(FormalPars),0));
        }

        ATermList l=list_minus(ATtableKeys(As),FormalParNames);
        if (ATisEmpty(l))
        {
          NewParList=ATinsert(NewParList,(ATerm)Par);
        }
        else 
        { 
          Culprit=ATAgetFirst(l);
        }
      }
      ParList=ATreverse(NewParList);
  
      if (ATisEmpty(ParList))
      {
        ATtableDestroy(As);
        gsErrorMsg("no process %P containing all assignments in %P.\nProblematic variable is %P.\n", Name, ProcTerm,Culprit);
        return NULL;
      }
      if (!ATisEmpty(ATgetNext(ParList)))
      {
        ATtableDestroy(As);
        gsErrorMsg("ambiguous process %P containing all assignments in %P.\n", Name, ProcTerm);
        return NULL;
      }
    }

    // get the formal parameter names
    ATermList ActualPars=ATmakeList0();
    ATermList FormalPars=ATLtableGet(body.proc_pars,(ATerm)gsMakeProcVarId(Name,ATLgetFirst(ParList)));
    {
      // we only need the names of the parameters, not the types
      for (ATermList l=FormalPars; !ATisEmpty(l); l=ATgetNext(l))
      {
        ATermAppl FormalParName=ATAgetArgument(ATAgetFirst(l),0);
        ATermAppl ActualPar=ATAtableGet(As,(ATerm)FormalParName);
        if (!ActualPar)
        {
          ActualPar=gsMakeId(FormalParName);
        }
        ActualPars=ATinsert(ActualPars,(ATerm)ActualPar);
      }
      ActualPars=ATreverse(ActualPars);
    }

    // if (gsDebug) { std::cerr << "transformed into a process call without short-hand assignments %T\n\n", gsMakeParamId(Name,ActualPars));

    ATermAppl TypeCheckedProcTerm=gstcRewrActProc(Vars, gsMakeParamId(Name,ActualPars));
    if (!TypeCheckedProcTerm)
    {
      ATtableDestroy(As);
      gsErrorMsg("type error occurred while typechecking the process call with short-hand assignments %P\n", ProcTerm);
      return NULL;
    }

    // if (gsDebug) { std::cerr << "successfully typechecked it into %T\n\n", TypeCheckedProcTerm);

    //reverse the assignments
    ATtableReset(As);
    for (ATermList l=ATLgetArgument(TypeCheckedProcTerm,1),m=FormalPars; !ATisEmpty(l); l=ATgetNext(l),m=ATgetNext(m))
    {
      ATermAppl act_par=ATAgetFirst(l);
      ATermAppl form_par=ATAgetFirst(m);
      if (ATisEqual(form_par,act_par))
      {
        continue;  //parameter does not change
      }
      ATtablePut(As,(ATerm)ATAgetArgument(form_par,0),(ATerm)gsMakeDataVarIdInit(form_par,act_par));
    }

    ATermList TypedAssignments=ATmakeList0();
    for (ATermList l=ATLgetArgument(ProcTerm,1); !ATisEmpty(l); l=ATgetNext(l))
    {
      ATermAppl a=ATAgetFirst(l);
      a=ATAtableGet(As,(ATerm)ATAgetArgument(a,0));
      if (!a)
      {
        continue;
      }
      TypedAssignments=ATinsert(TypedAssignments,(ATerm)a);
    }
    TypedAssignments=ATreverse(TypedAssignments);

    ATtableDestroy(As);

    TypeCheckedProcTerm=gsMakeProcessAssignment(ATAgetArgument(TypeCheckedProcTerm,0),TypedAssignments);

    // if (gsDebug) { std::cerr << "the resulting process call is %T\n\n", TypeCheckedProcTerm);

    return TypeCheckedProcTerm;
  }
  //Here it ends.

  if (gsIsParamId(ProcTerm))
  {
    return gstcRewrActProc(Vars,ProcTerm);
  }

  if (gsIsBlock(ProcTerm) || gsIsHide(ProcTerm) ||
      gsIsRename(ProcTerm) || gsIsComm(ProcTerm) || gsIsAllow(ProcTerm))
  {

    //block & hide
    if (gsIsBlock(ProcTerm) || gsIsHide(ProcTerm))
    {
      const char* msg=gsIsBlock(ProcTerm)?"Blocking":"Hiding";
      ATermList ActList=ATLgetArgument(ProcTerm,0);
      if (ATisEmpty(ActList))
      {
        gsWarningMsg("%s empty set of actions (typechecking %P)\n",msg,ProcTerm);
      }

      ATermIndexedSet Acts=ATindexedSetCreate(63,50);
      for (; !ATisEmpty(ActList); ActList=ATgetNext(ActList))
      {
        ATermAppl Act=ATAgetFirst(ActList);

        //Actions must be declared
        if (!ATtableGet(context.actions,(ATerm)Act))
        {
          gsErrorMsg("%s an undefined action %P (typechecking %P)\n",msg,Act,ProcTerm);
          return NULL;
        }
        bool nnew;
        ATindexedSetPut(Acts,(ATerm)Act,&nnew);
        if (!nnew)
        {
          gsWarningMsg("%s action %P twice (typechecking %P)\n",msg,Act,ProcTerm);
        }
      }
      ATindexedSetDestroy(Acts);
    }

    //rename
    if (gsIsRename(ProcTerm))
    {
      ATermList RenList=ATLgetArgument(ProcTerm,0);

      if (ATisEmpty(RenList))
      {
        gsWarningMsg("renaming empty set of actions (typechecking %P)\n",ProcTerm);
      }

      ATermIndexedSet ActsFrom=ATindexedSetCreate(63,50);

      for (; !ATisEmpty(RenList); RenList=ATgetNext(RenList))
      {
        ATermAppl Ren=ATAgetFirst(RenList);
        ATermAppl ActFrom=ATAgetArgument(Ren,0);
        ATermAppl ActTo=ATAgetArgument(Ren,1);

        if (ATisEqual(ActFrom,ActTo))
        {
          gsWarningMsg("renaming action %P into itself (typechecking %P)\n",ActFrom,ProcTerm);
        }

        //Actions must be declared and of the same types
        ATermList TypesFrom,TypesTo;
        if (!(TypesFrom=ATLtableGet(context.actions,(ATerm)ActFrom)))
        {
          gsErrorMsg("renaming an undefined action %P (typechecking %P)\n",ActFrom,ProcTerm);
          return NULL;
        }
        if (!(TypesTo=ATLtableGet(context.actions,(ATerm)ActTo)))
        {
          gsErrorMsg("renaming into an undefined action %P (typechecking %P)\n",ActTo,ProcTerm);
          return NULL;
        }

        TypesTo=gstcTypeListsIntersect(TypesFrom,TypesTo);
        if (!TypesTo || ATisEmpty(TypesTo))
        {
          gsErrorMsg("renaming action %P into action %P: these two have no common type (typechecking %P)\n",ActTo,ActFrom,ProcTerm);
          return NULL;
        }

        bool nnew;
        ATindexedSetPut(ActsFrom,(ATerm)ActFrom,&nnew);
        if (!nnew)
        {
          gsErrorMsg("renaming action %P twice (typechecking %P)\n",ActFrom,ProcTerm);
          return NULL;
        }
      }
      ATindexedSetDestroy(ActsFrom);
    }

    //comm: like renaming multiactions (with the same parameters) to action/tau
    if (gsIsComm(ProcTerm))
    {
      ATermList CommList=ATLgetArgument(ProcTerm,0);

      if (ATisEmpty(CommList))
      {
        gsWarningMsg("synchronizing empty set of (multi)actions (typechecking %P)\n",ProcTerm);
      }
      else
      {
        ATermList ActsFrom=ATmakeList0();

        for (; !ATisEmpty(CommList); CommList=ATgetNext(CommList))
        {
          ATermAppl Comm=ATAgetFirst(CommList);
          ATermList MActFrom=ATLgetArgument(ATAgetArgument(Comm,0),0);
          ATermList BackupMActFrom=MActFrom;
          assert(!ATisEmpty(MActFrom));
          ATermAppl ActTo=ATAgetArgument(Comm,1);

          if (ATgetLength(MActFrom)==1)
          {
            gsErrorMsg("using synchronization as renaming/hiding of action %P into %P (typechecking %P)\n",
                       ATgetFirst(MActFrom),ActTo,ProcTerm);
            return NULL;
          }

          //Actions must be declared
          ATermList ResTypes=NULL;

          if (!gsIsNil(ActTo))
          {
            ResTypes=ATLtableGet(context.actions,(ATerm)ActTo);
            if (!ResTypes)
            {
              gsErrorMsg("synchronizing to an undefined action %P (typechecking %P)\n",ActTo,ProcTerm);
              return NULL;
            }
          }

          for (; !ATisEmpty(MActFrom); MActFrom=ATgetNext(MActFrom))
          {
            ATermAppl Act=ATAgetFirst(MActFrom);
            ATermList Types=ATLtableGet(context.actions,(ATerm)Act);
            if (!Types)
            {
              gsErrorMsg("synchronizing an undefined action %P in (multi)action %P (typechecking %P)\n",Act,MActFrom,ProcTerm);
              return NULL;
            }
            ResTypes=(ResTypes)?gstcTypeListsIntersect(ResTypes,Types):Types;
            if (!ResTypes || ATisEmpty(ResTypes))
            {
              gsErrorMsg("synchronizing action %P from (multi)action %P into action %P: these have no common type (typechecking %P), ResTypes: %T\n",
                         Act,BackupMActFrom,ActTo,ProcTerm,ResTypes);
              return NULL;
            }
          }
          MActFrom=BackupMActFrom;

          //the multiactions in the lhss of comm should not intersect.
          //make the list of unique actions
          ATermList Acts=ATmakeList0();
          for (; !ATisEmpty(MActFrom); MActFrom=ATgetNext(MActFrom))
          {
            ATermAppl Act=ATAgetFirst(MActFrom);
            if (ATindexOf(Acts,(ATerm)Act,0)==ATERM_NON_EXISTING_POSITION)
            {
              Acts=ATinsert(Acts,(ATerm)Act);
            }
          }
          for (; !ATisEmpty(Acts); Acts=ATgetNext(Acts))
          {
            ATermAppl Act=ATAgetFirst(Acts);
            if (ATindexOf(ActsFrom,(ATerm)Act,0)!=ATERM_NON_EXISTING_POSITION)
            {
              gsErrorMsg("synchronizing action %P in different ways (typechecking %P)\n",Act,ProcTerm);
              return NULL;
            }
            else
            {
              ActsFrom=ATinsert(ActsFrom,(ATerm)Act);
            }
          }
        }
      }
    }

    //allow
    if (gsIsAllow(ProcTerm))
    {
      ATermList MActList=ATLgetArgument(ProcTerm,0);

      if (ATisEmpty(MActList))
      {
        gsWarningMsg("allowing empty set of (multi) actions (typechecking %P)\n",ProcTerm);
      }
      else
      {
        ATermList MActs=ATmakeList0();

        for (; !ATisEmpty(MActList); MActList=ATgetNext(MActList))
        {
          ATermList MAct=ATLgetArgument(ATAgetFirst(MActList),0);

          //Actions must be declared
          for (; !ATisEmpty(MAct); MAct=ATgetNext(MAct))
          {
            ATermAppl Act=ATAgetFirst(MAct);
            if (!ATLtableGet(context.actions,(ATerm)Act))
            {
              gsErrorMsg("allowing an undefined action %P in (multi)action %P (typechecking %P)\n",Act,MAct,ProcTerm);
              return NULL;
            }
          }

          MAct=ATLgetArgument(ATAgetFirst(MActList),0);
          if (gstcMActIn(MAct,MActs))
          {
            gsWarningMsg("allowing (multi)action %P twice (typechecking %P)\n",MAct,ProcTerm);
          }
          else
          {
            MActs=ATinsert(MActs,(ATerm)MAct);
          }
        }
      }
    }

    ATermAppl NewProc=gstcTraverseActProcVarConstP(Vars,ATAgetArgument(ProcTerm,1));
    if (!NewProc)
    {
      return NULL;
    }
    return ATsetArgument(ProcTerm,(ATerm)NewProc,1);
  }

  if (gsIsSync(ProcTerm) || gsIsSeq(ProcTerm) || gsIsBInit(ProcTerm) ||
      gsIsMerge(ProcTerm) || gsIsLMerge(ProcTerm) || gsIsChoice(ProcTerm))
  {
    ATermAppl NewLeft=gstcTraverseActProcVarConstP(Vars,ATAgetArgument(ProcTerm,0));

    if (!NewLeft)
    {
      return NULL;
    }
    ATermAppl NewRight=gstcTraverseActProcVarConstP(Vars,ATAgetArgument(ProcTerm,1));
    if (!NewRight)
    {
      return NULL;
    }
    return ATsetArgument(ATsetArgument(ProcTerm,(ATerm)NewLeft,0),(ATerm)NewRight,1);
  }

  if (gsIsAtTime(ProcTerm))
  {
    ATermAppl NewProc=gstcTraverseActProcVarConstP(Vars,ATAgetArgument(ProcTerm,0));
    if (!NewProc)
    {
      return NULL;
    }
    ATermAppl Time=ATAgetArgument(ProcTerm,1);
    ATermAppl NewType=gstcTraverseVarConsTypeD(Vars,Vars,&Time,gstcExpandNumTypesDown(sort_real::real_()));
    if (!NewType)
    {
      return NULL;
    }

    if (!gstcTypeMatchA(sort_real::real_(),NewType))
    {
      //upcasting
      ATermAppl CastedNewType=gstcUpCastNumericType(sort_real::real_(),NewType,&Time);
      if (!CastedNewType)
      {
        gsErrorMsg("cannot (up)cast time value %P to type Real\n",Time);
        return NULL;
      }
    }

    return gsMakeAtTime(NewProc,Time);
  }

  if (gsIsIfThen(ProcTerm))
  {
    ATermAppl Cond=ATAgetArgument(ProcTerm,0);
    ATermAppl NewType=gstcTraverseVarConsTypeD(Vars,Vars,&Cond,sort_bool::bool_());
    if (!NewType)
    {
      return NULL;
    }
    ATermAppl NewThen=gstcTraverseActProcVarConstP(Vars,ATAgetArgument(ProcTerm,1));
    if (!NewThen)
    {
      return NULL;
    }
    return gsMakeIfThen(Cond,NewThen);
  }

  if (gsIsIfThenElse(ProcTerm))
  {
    ATermAppl Cond=ATAgetArgument(ProcTerm,0);
    ATermAppl NewType=gstcTraverseVarConsTypeD(Vars,Vars,&Cond,sort_bool::bool_());
    if (!NewType)
    {
      return NULL;
    }
    ATermAppl NewThen=gstcTraverseActProcVarConstP(Vars,ATAgetArgument(ProcTerm,1));
    if (!NewThen)
    {
      return NULL;
    }
    ATermAppl NewElse=gstcTraverseActProcVarConstP(Vars,ATAgetArgument(ProcTerm,2));
    if (!NewElse)
    {
      return NULL;
    }
    return gsMakeIfThenElse(Cond,NewThen,NewElse);
  }

  if (gsIsSum(ProcTerm))
  {
    ATermTable CopyVars=ATtableCreate(63,50);
    gstcATermTableCopy(Vars,CopyVars);

    ATermTable NewVars=gstcAddVars2Table(CopyVars,ATLgetArgument(ProcTerm,0));
    if (!NewVars)
    {
      ATtableDestroy(CopyVars);
      gsErrorMsg("type error while typechecking %P\n",ProcTerm);
      return NULL;
    }
    ATermAppl NewProc=gstcTraverseActProcVarConstP(NewVars,ATAgetArgument(ProcTerm,1));
    ATtableDestroy(CopyVars);
    if (!NewProc)
    {
      gsErrorMsg("while typechecking %P\n",ProcTerm);
      return NULL;
    }
    return ATsetArgument(ProcTerm,(ATerm)NewProc,1);
  }

  assert(0);
  return Result;
}

static ATermAppl gstcTraversePBESVarConstPB(ATermTable Vars, ATermAppl PBESTerm)
{
  ATermAppl Result=NULL;

  if (gsIsDataExpr(PBESTerm))
  {
    ATermAppl NewType=gstcTraverseVarConsTypeD(Vars,Vars,&PBESTerm,sort_bool::bool_());
    if (!NewType)
    {
      return NULL;
    }
    return PBESTerm;
  }

  if (gsIsPBESTrue(PBESTerm) || gsIsPBESFalse(PBESTerm))
  {
    return PBESTerm;
  }

  if (gsIsPBESNot(PBESTerm))
  {
    ATermAppl NewArg=gstcTraversePBESVarConstPB(Vars,ATAgetArgument(PBESTerm,0));
    if (!NewArg)
    {
      return NULL;
    }
    return ATsetArgument(PBESTerm,(ATerm)NewArg,0);
  }

  if (gsIsPBESAnd(PBESTerm) || gsIsPBESOr(PBESTerm) || gsIsPBESImp(PBESTerm))
  {
    ATermAppl NewLeft=gstcTraversePBESVarConstPB(Vars,ATAgetArgument(PBESTerm,0));
    if (!NewLeft)
    {
      return NULL;
    }
    ATermAppl NewRight=gstcTraversePBESVarConstPB(Vars,ATAgetArgument(PBESTerm,1));
    if (!NewRight)
    {
      return NULL;
    }
    return ATsetArgument(ATsetArgument(PBESTerm,(ATerm)NewLeft,0),(ATerm)NewRight,1);
  }

  if (gsIsPBESForall(PBESTerm)||gsIsPBESExists(PBESTerm))
  {
    ATermTable CopyVars=ATtableCreate(63,50);
    gstcATermTableCopy(Vars,CopyVars);

    ATermTable NewVars=gstcAddVars2Table(CopyVars,ATLgetArgument(PBESTerm,0));
    if (!NewVars)
    {
      ATtableDestroy(CopyVars);
      gsErrorMsg("type error while typechecking %P\n",PBESTerm);
      return NULL;
    }
    ATermAppl NewPBES=gstcTraversePBESVarConstPB(NewVars,ATAgetArgument(PBESTerm,1));
    ATtableDestroy(CopyVars);
    if (!NewPBES)
    {
      gsErrorMsg("while typechecking %P\n",PBESTerm);
      return NULL;
    }
    return ATsetArgument(PBESTerm,(ATerm)NewPBES,1);
  }

  if (gsIsPropVarInst(PBESTerm))
  {
    return gstcRewrActProc(Vars, PBESTerm, true);
  }

  assert(0);
  return Result;
}

static ATermAppl gstcTraverseVarConsTypeD(
  ATermTable DeclaredVars,
  ATermTable AllowedVars,
  ATermAppl* DataTerm,
  ATermAppl PosType,
  ATermTable FreeVars,
  const bool strict_ambiguous,
  const bool warn_upcasting)
{
  //Type checks and transforms *DataTerm replacing Unknown datatype with other ones.
  //Returns the type of the term
  //which should match the PosType
  //all the variables should be in AllowedVars
  //if a variable is in DeclaredVars and not in AllowedVars,
  //a different error message is generated.
  //all free variables (if any) are added to FreeVars

  ATermAppl Result=NULL;

  if (gsDebug)
  {
    std::cerr << "gstcTraverseVarConsTypeD: DataTerm " << pp(*DataTerm) <<
              " with PosType " << pp(PosType) << "\n";
  }

  if (gsIsBinder(*DataTerm))
  {
    //The variable declaration of a binder should have at least 1 declaration
    if (ATAgetFirst(ATLgetArgument(*DataTerm, 1)) == NULL)
    {
      gsErrorMsg("binder %P should have at least one declared variable\n",*DataTerm);
      return NULL;
    }

    ATermAppl BindingOperator = ATAgetArgument(*DataTerm, 0);
    ATermTable CopyAllowedVars=ATtableCreate(63,50);
    gstcATermTableCopy(AllowedVars,CopyAllowedVars);
    ATermTable CopyDeclaredVars=ATtableCreate(63,50);
    //if(AllowedVars!=DeclaredVars)
    gstcATermTableCopy(DeclaredVars,CopyDeclaredVars);

    if (gsIsSetBagComp(BindingOperator) || gsIsSetComp(BindingOperator)
        || gsIsBagComp(BindingOperator))
    {
      ATermList VarDecls=ATLgetArgument(*DataTerm,1);
      ATermAppl VarDecl=ATAgetFirst(VarDecls);

      //A Set/bag comprehension should have exactly one variable declared
      VarDecls=ATgetNext(VarDecls);
      if (ATAgetFirst(VarDecls) != NULL)
      {
        gsErrorMsg("set/bag comprehension %P should have exactly one declared variable\n", *DataTerm);
        return NULL;
      }

      ATermAppl NewType=ATAgetArgument(VarDecl,1);
      ATermList VarList=ATmakeList1((ATerm)VarDecl);
      ATermTable NewAllowedVars=gstcAddVars2Table(CopyAllowedVars,VarList);
      if (!NewAllowedVars)
      {
        ATtableDestroy(CopyAllowedVars);
        ATtableDestroy(CopyDeclaredVars);
        return NULL;
      }
      ATermTable NewDeclaredVars=gstcAddVars2Table(CopyDeclaredVars,VarList);
      if (!NewDeclaredVars)
      {
        ATtableDestroy(CopyAllowedVars);
        ATtableDestroy(CopyDeclaredVars);
        return NULL;
      }
      ATermAppl Data=ATAgetArgument(*DataTerm,2);

      ATermAppl ResType=gstcTraverseVarConsTypeD(NewDeclaredVars,NewAllowedVars,&Data,data::unknown_sort(),FreeVars,strict_ambiguous,warn_upcasting);
      ATtableDestroy(CopyAllowedVars);
      ATtableDestroy(CopyDeclaredVars);

      if (!ResType)
      {
        return NULL;
      }
      if (gstcTypeMatchA(sort_bool::bool_(),ResType))
      {
        NewType=sort_set::set_(sort_expression(NewType));
        *DataTerm = ATsetArgument(*DataTerm, (ATerm)gsMakeSetComp(), 0);
      }
      else if (gstcTypeMatchA(sort_nat::nat(),ResType))
      {
        NewType=sort_bag::bag(sort_expression(NewType));
        *DataTerm = ATsetArgument(*DataTerm, (ATerm)gsMakeBagComp(), 0);
      }
      else
      {
        return NULL;
      }

      if (!(NewType=gstcTypeMatchA(NewType,PosType)))
      {
        gsErrorMsg("a set or bag comprehension of type %P does not match possible type %P (while typechecking %P)\n",ATAgetArgument(VarDecl,1),PosType,*DataTerm);
        return NULL;
      }

      if (FreeVars)
      {
        gstcRemoveVars(FreeVars,VarList);
      }
      *DataTerm=ATsetArgument(*DataTerm,(ATerm)Data,2);
      return NewType;
    }

    if (gsIsForall(BindingOperator) || gsIsExists(BindingOperator))
    {
      ATermList VarList=ATLgetArgument(*DataTerm,1);
      ATermTable NewAllowedVars=gstcAddVars2Table(CopyAllowedVars,VarList);
      if (!NewAllowedVars)
      {
        ATtableDestroy(CopyAllowedVars);
        ATtableDestroy(CopyDeclaredVars);
        return NULL;
      }
      ATermTable NewDeclaredVars=gstcAddVars2Table(CopyDeclaredVars,VarList);
      if (!NewDeclaredVars)
      {
        ATtableDestroy(CopyAllowedVars);
        ATtableDestroy(CopyDeclaredVars);
        return NULL;
      }

      ATermAppl Data=ATAgetArgument(*DataTerm,2);
      if (!gstcTypeMatchA(sort_bool::bool_(),PosType))
      {
        ATtableDestroy(CopyAllowedVars);
        ATtableDestroy(CopyDeclaredVars);
        return NULL;
      }
      ATermAppl NewType=gstcTraverseVarConsTypeD(NewDeclaredVars,NewAllowedVars,&Data,sort_bool::bool_(),FreeVars,strict_ambiguous,warn_upcasting);
      ATtableDestroy(CopyAllowedVars);
      ATtableDestroy(CopyDeclaredVars);

      if (!NewType)
      {
        return NULL;
      }
      if (!gstcTypeMatchA(sort_bool::bool_(),NewType))
      {
        return NULL;
      }

      if (FreeVars)
      {
        gstcRemoveVars(FreeVars,VarList);
      }
      *DataTerm=ATsetArgument(*DataTerm,(ATerm)Data,2);
      return sort_bool::bool_();
    }

    if (gsIsLambda(BindingOperator))
    {
      ATermList VarList=ATLgetArgument(*DataTerm,1);
      ATermTable NewAllowedVars=gstcAddVars2Table(CopyAllowedVars,VarList);
      if (!NewAllowedVars)
      {
        ATtableDestroy(CopyAllowedVars);
        ATtableDestroy(CopyDeclaredVars);
        return NULL;
      }
      ATermTable NewDeclaredVars=gstcAddVars2Table(CopyDeclaredVars,VarList);
      if (!NewDeclaredVars)
      {
        ATtableDestroy(CopyAllowedVars);
        ATtableDestroy(CopyDeclaredVars);
        return NULL;
      }

      ATermList ArgTypes=gstcGetVarTypes(VarList);
      ATermAppl NewType=gstcUnArrowProd(ArgTypes,PosType);
      if (!NewType)
      {
        ATtableDestroy(CopyAllowedVars);
        ATtableDestroy(CopyDeclaredVars);
        gsErrorMsg("no functions with arguments %P among %P (while typechecking %P)\n", ArgTypes,PosType,*DataTerm);
        return NULL;
      }
      ATermAppl Data=ATAgetArgument(*DataTerm,2);

      NewType=gstcTraverseVarConsTypeD(NewDeclaredVars,NewAllowedVars,&Data,NewType,FreeVars,strict_ambiguous,warn_upcasting);

      if (gsDebug)
      {
        std::cerr << "Result of gstcTraverseVarConsTypeD: DataTerm " << pp(Data) << "\n";
      }

      ATtableDestroy(CopyAllowedVars);
      ATtableDestroy(CopyDeclaredVars);

      if (FreeVars)
      {
        gstcRemoveVars(FreeVars,VarList);
      }
      if (!NewType)
      {
        return NULL;
      }
      *DataTerm=ATsetArgument(*DataTerm,(ATerm)Data,2);
      return gsMakeSortArrow(ArgTypes,NewType);
    }
  }

  if (gsIsWhr(*DataTerm))
  {
    ATermList WhereVarList=ATmakeList0();
    ATermList NewWhereList=ATmakeList0();
    for (ATermList WhereList=ATLgetArgument(*DataTerm,1); !ATisEmpty(WhereList); WhereList=ATgetNext(WhereList))
    {
      ATermAppl WhereElem=ATAgetFirst(WhereList);
      ATermAppl WhereTerm=ATAgetArgument(WhereElem,1);
      ATermAppl WhereType=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&WhereTerm,data::unknown_sort(),FreeVars,strict_ambiguous,warn_upcasting);
      if (!WhereType)
      {
        return NULL;
      }

      ATermAppl NewWhereVar=NULL;
      if (gsIsDataVarId(ATAgetArgument(WhereElem,0)))
      {
        // The variable in WhereElem is type checked, and a proper variable.
        NewWhereVar=ATAgetArgument(WhereElem,0);
      }
      else
      {
        // The variable in WhereElem is just a string and needs to be transformed to a DataVarId.
        NewWhereVar=gsMakeDataVarId(ATAgetArgument(WhereElem,0),WhereType);
      }
      WhereVarList=ATinsert(WhereVarList,(ATerm)NewWhereVar);
      NewWhereList=ATinsert(NewWhereList,(ATerm)gsMakeDataVarIdInit(NewWhereVar,WhereTerm));
    }
    NewWhereList=ATreverse(NewWhereList);

    ATermTable CopyAllowedVars=ATtableCreate(63,50);
    gstcATermTableCopy(AllowedVars,CopyAllowedVars);
    ATermTable CopyDeclaredVars=ATtableCreate(63,50);
    //if(AllowedVars!=DeclaredVars)
    gstcATermTableCopy(DeclaredVars,CopyDeclaredVars);

    ATermList VarList=ATreverse(WhereVarList);
    ATermTable NewAllowedVars=gstcAddVars2Table(CopyAllowedVars,VarList);
    if (!NewAllowedVars)
    {
      ATtableDestroy(CopyAllowedVars);
      ATtableDestroy(CopyDeclaredVars);
      return NULL;
    }
    ATermTable NewDeclaredVars=gstcAddVars2Table(CopyDeclaredVars,VarList);
    if (!NewDeclaredVars)
    {
      ATtableDestroy(CopyAllowedVars);
      ATtableDestroy(CopyDeclaredVars);
      return NULL;
    }

    ATermAppl Data=ATAgetArgument(*DataTerm,0);
    ATermAppl NewType=gstcTraverseVarConsTypeD(NewDeclaredVars,NewAllowedVars,&Data,PosType,FreeVars,strict_ambiguous,warn_upcasting);
    ATtableDestroy(CopyAllowedVars);
    ATtableDestroy(CopyDeclaredVars);

    if (!NewType)
    {
      return NULL;
    }
    if (FreeVars)
    {
      gstcRemoveVars(FreeVars,VarList);
    }
    *DataTerm=gsMakeWhr(Data,NewWhereList);
    return NewType;
  }

  if (gsIsDataAppl(*DataTerm))
  {
    //arguments
    ATermList Arguments=ATLgetArgument(*DataTerm,1);
    size_t nArguments=ATgetLength(Arguments);

    //The following is needed to check enumerations
    ATermAppl Arg0 = ATAgetArgument(*DataTerm,0);
    if (gsIsOpId(Arg0) || gsIsId(Arg0))
    {
      ATermAppl Name = ATAgetArgument(Arg0,0);
      if (Name == sort_list::list_enumeration_name())
      {
        ATermAppl Type=gstcUnList(PosType);
        if (!Type)
        {
          gsErrorMsg("not possible to cast %s to %P (while typechecking %P)\n", "list", PosType,Arguments);
          return NULL;
        }

        ATermList OldArguments=Arguments;

        //First time to determine the common type only!
        ATermList NewArguments=ATmakeList0();
        bool Type_is_stable=true;
        bool first_time=true;
        for (; !ATisEmpty(Arguments); Arguments=ATgetNext(Arguments))
        {
          ATermAppl Argument=ATAgetFirst(Arguments);
          ATermAppl Type0=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Argument,Type,FreeVars,strict_ambiguous,warn_upcasting);
          if (!Type0)
          {
            return NULL;
          }
          NewArguments=ATinsert(NewArguments,(ATerm)Argument);
          Type_is_stable=first_time||(Type==Type0);
          first_time=false;
          Type=Type0;
        }
        Arguments=OldArguments;

        //Second time to do the real work, but only if the elements in the list have different types.
        if (!Type_is_stable)
        {
          NewArguments=ATmakeList0();
          for (; !ATisEmpty(Arguments); Arguments=ATgetNext(Arguments))
          {
            ATermAppl Argument=ATAgetFirst(Arguments);
            ATermAppl Type0=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Argument,Type,FreeVars,strict_ambiguous,warn_upcasting);
            if (!Type0)
            {
              return NULL;
            }
            NewArguments=ATinsert(NewArguments,(ATerm)Argument);
            Type=Type0;
          }
        }
        Arguments=ATreverse(NewArguments);

        Type=sort_list::list(sort_expression(Type));
        *DataTerm=sort_list::list_enumeration(sort_expression(Type), atermpp::aterm_list(Arguments));
        return Type;
      }
      if (Name == sort_set::set_enumeration_name())
      {
        ATermAppl Type=gstcUnSet(PosType);
        if (!Type)
        {
          gsErrorMsg("not possible to cast set to %P (while typechecking %P)\n", PosType,Arguments);
          return NULL;
        }

        ATermList OldArguments=Arguments;

        //First time to determine the common type only (which will be NewType)!
        ATermAppl NewType=NULL;
        for (; !ATisEmpty(Arguments); Arguments=ATgetNext(Arguments))
        {
          ATermAppl Argument=ATAgetFirst(Arguments);
          ATermAppl Type0=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Argument,Type,FreeVars,strict_ambiguous,warn_upcasting);
          if (!Type0)
          {
            gsErrorMsg("not possible to cast element to %P (while typechecking %P)\n", Type,Argument);
            return NULL;
          }

          ATermAppl OldNewType=NewType;
          if (NewType==NULL)
          {
            NewType=Type0;
          }
          else
          {
            NewType=gstcMaximumType(NewType,Type0);
          }

          if (NewType==NULL)
          {
            gsErrorMsg("Set contains incompatible elements of sorts %P and %P (while typechecking %P)\n", OldNewType,Type0,Argument);
            return NULL;
          }
        }

        // Type is now the most generic type to be used.
        assert(Type!=NULL);
        Type=NewType;
        Arguments=OldArguments;

        //Second time to do the real work.
        ATermList NewArguments=ATmakeList0();
        for (; !ATisEmpty(Arguments); Arguments=ATgetNext(Arguments))
        {
          ATermAppl Argument=ATAgetFirst(Arguments);
          ATermAppl Type0=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Argument,Type,FreeVars,strict_ambiguous,warn_upcasting);
          if (!Type0)
          {
            gsErrorMsg("not possible to cast element to %P (while typechecking %P)\n", Type,Argument);
            return NULL;
          }
          NewArguments=ATinsert(NewArguments,(ATerm)Argument);
          Type=Type0;
        }
        Arguments=ATreverse(NewArguments);
        Type=sort_set::set_(sort_expression(Type));
        *DataTerm=sort_set::set_enumeration(sort_expression(Type),atermpp::aterm_list(Arguments));
        return Type;
      }
      if (Name == sort_bag::bag_enumeration_name())
      {
        ATermAppl Type=gstcUnBag(PosType);
        if (!Type)
        {
          gsErrorMsg("not possible to cast bag to %P (while typechecking %P)\n", PosType,Arguments);
          return NULL;
        }

        ATermList OldArguments=Arguments;

        //First time to determine the common type only!
        ATermAppl NewType=NULL;
        for (; !ATisEmpty(Arguments); Arguments=ATgetNext(Arguments))
        {
          ATermAppl Argument0=ATAgetFirst(Arguments);
          Arguments=ATgetNext(Arguments);
          ATermAppl Argument1=ATAgetFirst(Arguments);
          ATermAppl Type0=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Argument0,Type,FreeVars,strict_ambiguous,warn_upcasting);
          if (!Type0)
          {
            gsErrorMsg("not possible to cast %s to %P (while typechecking %P)\n", "element", Type,Argument0);
            return NULL;
          }
          ATermAppl Type1=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Argument1,sort_nat::nat(),FreeVars,strict_ambiguous,warn_upcasting);
          if (!Type1)
          {
            gsErrorMsg("not possible to cast number to %P (while typechecking %P)\n",
                       static_cast<ATermAppl>(sort_nat::nat()),Argument1);
            return NULL;
          }
          ATermAppl OldNewType=NewType;
          if (NewType==NULL)
          {
            NewType=Type0;
          }
          else
          {
            NewType=gstcMaximumType(NewType,Type0);
          }
          if (NewType==NULL)
          {
            gsErrorMsg("Bag contains incompatible elements of sorts %P and %P (while typechecking %P)\n",
                       OldNewType,Type0,Argument0);
            return NULL;
          }
        }
        assert(Type!=NULL);
        Type=NewType;
        Arguments=OldArguments;

        //Second time to do the real work.
        ATermList NewArguments=ATmakeList0();
        for (; !ATisEmpty(Arguments); Arguments=ATgetNext(Arguments))
        {
          ATermAppl Argument0=ATAgetFirst(Arguments);
          Arguments=ATgetNext(Arguments);
          ATermAppl Argument1=ATAgetFirst(Arguments);
          ATermAppl Type0=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Argument0,Type,FreeVars,strict_ambiguous,warn_upcasting);
          if (!Type0)
          {
            gsErrorMsg("not possible to cast %s to %P (while typechecking %P)\n", "element", Type,Argument0);
            return NULL;
          }
          ATermAppl Type1=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Argument1,sort_nat::nat(),FreeVars,strict_ambiguous,warn_upcasting);
          if (!Type1)
          {
            gsErrorMsg("not possible to cast %s to %P (while typechecking %P)\n", "number", static_cast<ATermAppl>(sort_nat::nat()),Argument1);
            return NULL;
          }
          NewArguments=ATinsert(NewArguments,(ATerm)Argument0);
          NewArguments=ATinsert(NewArguments,(ATerm)Argument1);
          Type=Type0;
        }
        Arguments=ATreverse(NewArguments);
        Type=sort_bag::bag(sort_expression(Type));
        *DataTerm=sort_bag::bag_enumeration(sort_expression(Type), atermpp::aterm_list(Arguments));
        return Type;
      }
    }

    ATermList NewArgumentTypes=ATmakeList0();
    ATermList NewArguments=ATmakeList0();

    for (; !ATisEmpty(Arguments); Arguments=ATgetNext(Arguments))
    {
      ATermAppl Arg=ATAgetFirst(Arguments);
      ATermAppl Type=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Arg,data::unknown_sort(),FreeVars,false,warn_upcasting);
      if (!Type)
      {
        return NULL;
      }
      NewArguments=ATinsert(NewArguments,(ATerm)Arg);
      NewArgumentTypes=ATinsert(NewArgumentTypes,(ATerm)Type);
    }
    Arguments=ATreverse(NewArguments);
    ATermList ArgumentTypes=ATreverse(NewArgumentTypes);

    //function
    ATermAppl Data=ATAgetArgument(*DataTerm,0);
    ATermAppl NewType=gstcTraverseVarConsTypeDN(DeclaredVars,AllowedVars,
                      &Data,data::unknown_sort()/*gsMakeSortArrow(ArgumentTypes,PosType)*/,
                      FreeVars,false,nArguments,warn_upcasting);
    if (gsDebug)
    {
      std::cerr << "Result of gstcTraverseVarConsTypeD: DataTerm " << pp(Data) << "\n";
    }

    if (!NewType)
    {
      if (was_ambiguous)
      {
        was_ambiguous=false;
      }
      else if (gsIsOpId(Data)||gsIsDataVarId(Data))
      {
        gstcErrorMsgCannotCast(ATAgetArgument(Data,1),Arguments,ArgumentTypes);
      }
      gsErrorMsg("type error while trying to cast %P to type %P\n",gsMakeDataAppl(Data,Arguments),PosType);
      return NULL;
    }

    //it is possible that:
    //1) a cast has happened
    //2) some parameter Types became sharper.
    //we do the arguments again with the types.

    if (gsIsSortArrow(gstcUnwindType(NewType)))
    {
      ATermList NeededArgumentTypes=ATLgetArgument(gstcUnwindType(NewType),0);

      if (gsDebug)
      {
        std::cerr << "Arguments again: NeededArgumentTypes: " << pp(NeededArgumentTypes) <<
                  ", ArgumentTypes: " << pp(ArgumentTypes) << "\n";
      }

      //arguments again
      ATermList NewArgumentTypes=ATmakeList0();
      ATermList NewArguments=ATmakeList0();
      for (; !ATisEmpty(Arguments); Arguments=ATgetNext(Arguments),
           ArgumentTypes=ATgetNext(ArgumentTypes),NeededArgumentTypes=ATgetNext(NeededArgumentTypes))
      {
        ATermAppl Arg=ATAgetFirst(Arguments);
        ATermAppl NeededType=ATAgetFirst(NeededArgumentTypes);
        ATermAppl Type=ATAgetFirst(ArgumentTypes);

        if (!gstcEqTypesA(NeededType,Type))
        {
          //upcasting
          ATermAppl CastedNewType=gstcUpCastNumericType(NeededType,Type,&Arg,warn_upcasting);
          if (CastedNewType)
          {
            Type=CastedNewType;
          }
        }
        if (!gstcEqTypesA(NeededType,Type))
        {
          if (gsDebug)
          {
            std::cerr << "Doing again on " << pp(Arg) << ", Type: " << pp(Type) << ", Needed type: " << pp(NeededType) << "\n";
          }
          ATermAppl NewArgType=gstcTypeMatchA(NeededType,Type);
          if (!NewArgType)
          {
            NewArgType=gstcTypeMatchA(NeededType,gstcExpandNumTypesUp(Type));
          }
          if (!NewArgType)
          {
            NewArgType=NeededType;
          }
          NewArgType=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Arg,NewArgType,FreeVars,strict_ambiguous,warn_upcasting);
          if (gsDebug)
          {
            std::cerr << "Result of Doing again gstcTraverseVarConsTypeD: DataTerm " << pp(Arg) << "\n";
          }
          if (!NewArgType)
          {
            gsErrorMsg("needed type %P does not match possible type %P (while typechecking %P in %P)\n",NeededType,Type,Arg,*DataTerm);
            return NULL;
          }
          Type=NewArgType;
        }
        NewArguments=ATinsert(NewArguments,(ATerm)Arg);
        NewArgumentTypes=ATinsert(NewArgumentTypes,(ATerm)Type);
      }
      Arguments=ATreverse(NewArguments);
      ArgumentTypes=ATreverse(NewArgumentTypes);
    }

    //the function again
    NewType=gstcTraverseVarConsTypeDN(DeclaredVars,AllowedVars,
                                      &Data,gsMakeSortArrow(ArgumentTypes,PosType),FreeVars,strict_ambiguous,nArguments,warn_upcasting);

    if (gsDebug)
    {
      std::cerr << "Result of gstcTraverseVarConsTypeDN: DataTerm " << pp(Data) << "\n";
    }

    if (!NewType)
    {
      if (was_ambiguous)
      {
        was_ambiguous=false;
      }
      else
      {
        gstcErrorMsgCannotCast(ATAgetArgument(Data,1),Arguments,ArgumentTypes);
      }
      gsErrorMsg("type error while trying to cast %P to type %P\n",gsMakeDataAppl(Data,Arguments),PosType);
      return NULL;
    }

    if (gsDebug)
    {
      std::cerr << "Arguments once more: Arguments " << pp(Arguments) << ", ArgumentTypes: " <<
                pp(ArgumentTypes) << ", NewType: " << pp(NewType) << "\n";
    }

    //and the arguments once more
    if (gsIsSortArrow(gstcUnwindType(NewType)))
    {
      ATermList NeededArgumentTypes=ATLgetArgument(gstcUnwindType(NewType),0);
      ATermList NewArgumentTypes=ATmakeList0();
      ATermList NewArguments=ATmakeList0();
      for (; !ATisEmpty(Arguments); Arguments=ATgetNext(Arguments),
           ArgumentTypes=ATgetNext(ArgumentTypes),NeededArgumentTypes=ATgetNext(NeededArgumentTypes))
      {
        ATermAppl Arg=ATAgetFirst(Arguments);
        ATermAppl NeededType=ATAgetFirst(NeededArgumentTypes);
        ATermAppl Type=ATAgetFirst(ArgumentTypes);

        if (!gstcEqTypesA(NeededType,Type))
        {
          //upcasting
          ATermAppl CastedNewType=gstcUpCastNumericType(NeededType,Type,&Arg,warn_upcasting);
          if (CastedNewType)
          {
            Type=CastedNewType;
          }
        }
        if (!gstcEqTypesA(NeededType,Type))
        {
          if (gsDebug)
          {
            std::cerr << "Doing again on " << pp(Arg) << ", Type: " << pp(Type) << ", Needed type: " << pp(NeededType) << "\n";
          }
          ATermAppl NewArgType=gstcTypeMatchA(NeededType,Type);
          if (!NewArgType)
          {
            NewArgType=gstcTypeMatchA(NeededType,gstcExpandNumTypesUp(Type));
          }
          if (!NewArgType)
          {
            NewArgType=NeededType;
          }
          NewArgType=gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,&Arg,NewArgType,FreeVars,strict_ambiguous,warn_upcasting);
          if (!NewArgType)
          {
            gsErrorMsg("needed type %P does not match possible type %P (while typechecking %P in %P)\n",NeededType,Type,Arg,*DataTerm);
            return NULL;
          }
          Type=NewArgType;
        }

        NewArguments=ATinsert(NewArguments,(ATerm)Arg);
        NewArgumentTypes=ATinsert(NewArgumentTypes,(ATerm)Type);
      }
      Arguments=ATreverse(NewArguments);
      ArgumentTypes=ATreverse(NewArgumentTypes);
    }

    if (gsDebug)
    {
      std::cerr << "Arguments after once more: Arguments " << pp(Arguments) << ", ArgumentTypes: " << pp(ArgumentTypes) << "\n";
    }

    *DataTerm=gsMakeDataAppl(Data,Arguments);

    if (gsIsSortArrow(gstcUnwindType(NewType)))
    {
      return ATAgetArgument(gstcUnwindType(NewType),1);
    }

    if (gstcHasUnknown(gstcUnArrowProd(ArgumentTypes,NewType)))
    {
      gsErrorMsg("Fail to properly type %P\n",*DataTerm);
      return NULL;
    }
    return gstcUnArrowProd(ArgumentTypes,NewType);
  }

  if (gsIsId(*DataTerm)||gsIsOpId(*DataTerm)||gsIsDataVarId(*DataTerm))
  {
    ATermAppl Name=ATAgetArgument(*DataTerm,0);
    if (gsIsNumericString(gsATermAppl2String(Name)))
    {
      ATermAppl Sort=sort_int::int_();
      if (gstcIsPos(Name))
      {
        Sort=sort_pos::pos();
      }
      else if (gstcIsNat(Name))
      {
        Sort=sort_nat::nat();
      }
      *DataTerm=gsMakeOpId(Name,Sort);

      if (gstcTypeMatchA(Sort,PosType))
      {
        return Sort;
      }

      //upcasting
      ATermAppl CastedNewType=gstcUpCastNumericType(PosType,Sort,DataTerm,warn_upcasting);
      if (!CastedNewType)
      {
        gsErrorMsg("cannot (up)cast number %P to type %P\n",*DataTerm, PosType);
        return NULL;
      }
      return CastedNewType;
    }

    ATermAppl Type=ATAtableGet(DeclaredVars,(ATerm)Name);
    if (Type)
    {
      if (gsDebug)
      {
        std::cerr << "Recognised declared variable " << pp(Name) << ", Type: " << pp(Type) << "\n";
      }
      *DataTerm=gsMakeDataVarId(Name,Type);

      if (!ATAtableGet(AllowedVars,(ATerm)Name))
      {
        gsErrorMsg("variable %P occurs freely in the right-hand-side or condition of an equation, but not in the left-hand-side\n", Name);
        return NULL;
      }

      ATermAppl NewType=gstcTypeMatchA(Type,PosType);
      if (NewType)
      {
        Type=NewType;
      }
      else
      {
        //upcasting
        ATermAppl CastedNewType=gstcUpCastNumericType(PosType,Type,DataTerm,warn_upcasting);
        if (!CastedNewType)
        {
          gsErrorMsg("cannot (up)cast variable %P to type %P\n",*DataTerm,PosType);
          return NULL;
        }

        Type=CastedNewType;
      }

      //Add to free variables list
      if (FreeVars)
      {
        ATtablePut(FreeVars, (ATerm)Name, (ATerm)Type);
      }

      return Type;
    }

    if ((Type=ATAtableGet(context.constants,(ATerm)Name)))
    {
      ATermAppl NewType=gstcTypeMatchA(Type,PosType);
      if (NewType)
      {
        Type=NewType;
        *DataTerm=gsMakeOpId(Name,Type);
        return Type;
      }
      else
      {
        // The type cannot be unified. Try upcasting the type.
        *DataTerm=gsMakeOpId(Name,Type);
        ATermAppl NewType=gstcUpCastNumericType(PosType,Type,DataTerm,warn_upcasting);
        if (NewType==NULL)
        {
          gsErrorMsg("no constant %P with type %P\n",*DataTerm,PosType);
          return NULL;
        }
        else
        {
          return NewType;
        }
      }
    }

    ATermList ParList=ATLtableGet(gssystem.constants,(ATerm)Name);
    if (ParList)
    {
      ATermList NewParList=ATmakeList0();
      for (; !ATisEmpty(ParList); ParList=ATgetNext(ParList))
      {
        ATermAppl Par=ATAgetFirst(ParList);
        if ((Par=gstcTypeMatchA(Par,PosType)))
        {
          NewParList=ATinsert(NewParList,(ATerm)Par);
        }
      }
      ParList=ATreverse(NewParList);
      if (ATisEmpty(ParList))
      {
        gsErrorMsg("no system constant %P with type %P\n",*DataTerm,PosType);
        return NULL;
      }

      if (ATgetLength(ParList)==1)
      {
        Type=ATAgetFirst(ParList);
        *DataTerm=gsMakeOpId(Name,Type);
        return Type;
      }
      else
      {
        //gsWarningMsg("ambiguous system constant %T\n",Name);
        *DataTerm=gsMakeOpId(Name,data::unknown_sort());
        return data::unknown_sort();
      }
    }

    ATermList ParListS=ATLtableGet(gssystem.functions,(ATerm)Name);
    ParList=ATLtableGet(context.functions,(ATerm)Name);
    if (!ParList)
    {
      ParList=ParListS;
    }
    else if (ParListS)
    {
      ParList=ATconcat(ParListS,ParList);
    }

    if (!ParList)
    {
      gsErrorMsg("unknown operation %P\n",Name);
      return NULL;
    }

    if (ATgetLength(ParList)==1)
    {
      ATermAppl Type=ATAgetFirst(ParList);
      *DataTerm=gsMakeOpId(Name,Type);
      ATermAppl NewType=gstcUpCastNumericType(PosType,Type,DataTerm,warn_upcasting);
      if (NewType==NULL)
      {
        gsErrorMsg("no constant %P with type %P\n",*DataTerm,PosType);
        return NULL;
      }
      return NewType;
    }
    else
    {
      return gstcTraverseVarConsTypeDN(DeclaredVars, AllowedVars, DataTerm, PosType, FreeVars, strict_ambiguous, ATERM_NON_EXISTING_POSITION, warn_upcasting);
    }
  }

  // if(gsIsDataVarId(*DataTerm)){
  //   return ATAgetArgument(*DataTerm,1);
  // }

  return Result;
}

static ATermAppl gstcTraverseVarConsTypeDN(
  ATermTable DeclaredVars,
  ATermTable AllowedVars,
  ATermAppl* DataTerm,
  ATermAppl PosType,
  ATermTable FreeVars,
  const bool strict_ambiguous,
  const size_t nFactPars,
  const bool warn_upcasting)
{
  // ATERM_NON_EXISTING_POSITION for nFactPars means the number of arguments is not known.
  if (gsDebug)
  {
    std::cerr << "gstcTraverseVarConsTypeDN: DataTerm ";
    ATfprintf(stderr,"%t",*DataTerm);
    std::cerr << " with PosType " << pp(PosType) << ", nFactPars " << nFactPars << "\n";
  }
  if (gsIsId(*DataTerm)||gsIsOpId(*DataTerm))
  {
    ATermAppl Name=ATAgetArgument(*DataTerm,0);
    bool variable=false;
    ATermAppl Type=ATAtableGet(DeclaredVars,(ATerm)Name);
    if (Type)
    {
      const sort_expression Type1(gstcUnwindType(Type));
      if (is_function_sort(Type1)?(function_sort(Type1).domain().size()==nFactPars):(nFactPars==0))
      {
        variable=true;
        if (!ATAtableGet(AllowedVars,(ATerm)Name))
        {
          gsErrorMsg("variable %P occurs freely in the right-hand-side or condition of an equation, but not in the left-hand-side\n", Name);
          return NULL;
        }

        //Add to free variables list
        if (FreeVars)
        {
          ATtablePut(FreeVars, (ATerm)Name, (ATerm)Type);
        }
      }
      else
      {
        Type=NULL;
      }
    }
    ATermList ParList;

    if (nFactPars==0)
    {
      if ((Type=ATAtableGet(DeclaredVars,(ATerm)Name)))
      {
        if (!gstcTypeMatchA(Type,PosType))
        {
          gsErrorMsg("the type %P of variable %P is incompatible with %P (typechecking %P)\n",Type,Name,PosType,*DataTerm);
          return NULL;
        }
        *DataTerm=gsMakeDataVarId(Name,Type);
        return Type;
      }
      else if ((Type=ATAtableGet(context.constants,(ATerm)Name)))
      {
        if (!gstcTypeMatchA(Type,PosType))
        {
          gsErrorMsg("the type %P of constant %P is incompatible with %P (typechecking %P)\n",Type,Name,PosType,*DataTerm);
          return NULL;
        }
        *DataTerm=gsMakeOpId(Name,Type);
        return Type;
      }
      else
      {
        if ((ParList=ATLtableGet(gssystem.constants,(ATerm)Name)))
        {
          if (ATgetLength(ParList)==1)
          {
            ATermAppl Type=ATAgetFirst(ParList);
            *DataTerm=gsMakeOpId(Name,Type);
            return Type;
          }
          else
          {
            gsWarningMsg("ambiguous system constant %P\n",Name);
            *DataTerm=gsMakeOpId(Name,data::unknown_sort());
            return Type;
          }
        }
        else
        {
          gsErrorMsg("unknown constant %P\n",Name);
          return NULL;
        }
      }
    }

    if (Type)
    {
      ParList=ATmakeList1((ATerm)gstcUnwindType(Type));
    }
    else
    {
      ATermList ParListS=ATLtableGet(gssystem.functions,(ATerm)Name);
      ParList=ATLtableGet(context.functions,(ATerm)Name);
      if (!ParList)
      {
        ParList=ParListS;
      }
      else if (ParListS)
      {
        ParList=ATconcat(ParListS,ParList);
      }
    }

    if (!ParList)
    {
      if (nFactPars!=ATERM_NON_EXISTING_POSITION)
      {
        gsErrorMsg("unknown operation %P with %d parameter%s\n",Name, nFactPars, (nFactPars != 1)?"s":"");
      }
      else
      {
        gsErrorMsg("unknown operation %P\n",Name);
      }
      return NULL;
    }
    if (gsDebug)
    {
      std::cerr << "Possible types for Op/Var " << pp(Name) << " with " << nFactPars <<
                " argument are (ParList: " << pp(ParList) << "; PosType: " << pp(PosType) << ")\n";
    }

    ATermList CandidateParList=ParList;

    {
      // filter ParList keeping only functions A_0#...#A_nFactPars->A
      ATermList NewParList;
      if (nFactPars!=ATERM_NON_EXISTING_POSITION)
      {
        NewParList=ATmakeList0();
        for (; !ATisEmpty(ParList); ParList=ATgetNext(ParList))
        {
          ATermAppl Par=ATAgetFirst(ParList);
          if (!gsIsSortArrow(Par))
          {
            continue;
          }
          if ((ATgetLength(ATLgetArgument(Par,0))!=nFactPars))
          {
            continue;
          }
          NewParList=ATinsert(NewParList,(ATerm)Par);
        }
        ParList=ATreverse(NewParList);
      }

      if (!ATisEmpty(ParList))
      {
        CandidateParList=ParList;
      }

      // filter ParList keeping only functions of the right type
      ATermList BackupParList=ParList;
      NewParList=ATmakeList0();
      for (; !ATisEmpty(ParList); ParList=ATgetNext(ParList))
      {
        ATermAppl Par=ATAgetFirst(ParList);
        if ((Par=gstcTypeMatchA(Par,PosType)))
        {
          NewParList=ATinsertUnique(NewParList,(ATerm)Par);
        }
      }
      NewParList=ATreverse(NewParList);

      if (gsDebug)
      {
        std::cerr << "Possible matches w/o casting for Op/Var " << pp(Name) << " with "<< nFactPars <<               " argument are (ParList: " << pp(NewParList) << "; PosType: " << pp(PosType) << "\n";
      }

      if (ATisEmpty(NewParList))
      {
        //Ok, this looks like a type error. We are not that strict.
        //Pos can be Nat, or even Int...
        //So lets make PosType more liberal
        //We change every Pos to NotInferred(Pos,Nat,Int)...
        //and get the list. Then we take the min of the list.

        ParList=BackupParList;
        if (gsDebug)
        {
          std::cerr << "Trying casting for Op " << pp(Name) << " with " << nFactPars << " argument (ParList: " <<                             pp(ParList) << "; PosType: " << pp(PosType) << "\n";
        }
        PosType=gstcExpandNumTypesUp(PosType);
        for (; !ATisEmpty(ParList); ParList=ATgetNext(ParList))
        {
          ATermAppl Par=ATAgetFirst(ParList);
          if ((Par=gstcTypeMatchA(Par,PosType)))
          {
            NewParList=ATinsertUnique(NewParList,(ATerm)Par);
          }
        }
        NewParList=ATreverse(NewParList);
        if (gsDebug)
        {
          std::cerr << "The result of casting is " << pp(NewParList) << "\n";
        }
        if (ATgetLength(NewParList)>1)
        {
          NewParList=ATmakeList1((ATerm)gstcMinType(NewParList));
        }
      }

      if (ATisEmpty(NewParList))
      {
        //Ok, casting of the arguments did not help.
        //Let's try to be more relaxed about the result, e.g. returning Pos or Nat is not a bad idea for int.

        ParList=BackupParList;
        if (gsDebug)
        {
          std::cerr << "Trying result casting for Op " << pp(Name) << " with " << nFactPars << " argument (ParList: "                       << "; PosType: " << pp(PosType) << "\n";
        }
        PosType=gstcExpandNumTypesDown(gstcExpandNumTypesUp(PosType));
        for (; !ATisEmpty(ParList); ParList=ATgetNext(ParList))
        {
          ATermAppl Par=ATAgetFirst(ParList);
          if ((Par=gstcTypeMatchA(Par,PosType)))
          {
            NewParList=ATinsertUnique(NewParList,(ATerm)Par);
          }
        }
        NewParList=ATreverse(NewParList);
        if (gsDebug)
        {
          std::cerr << "The result of casting is " << pp(NewParList) << "\n";
        }
        if (ATgetLength(NewParList)>1)
        {
          NewParList=ATmakeList1((ATerm)gstcMinType(NewParList));
        }
      }

      ParList=NewParList;
    }

    if (ATisEmpty(ParList))
    {
      //provide some information to the upper layer for a better error message
      ATermAppl Sort;
      if (ATgetLength(CandidateParList)==1)
      {
        Sort=ATAgetFirst(CandidateParList);
      }
      else
      {
        Sort=multiple_possible_sorts(atermpp::aterm_list(CandidateParList));
      }
      *DataTerm=gsMakeOpId(Name,Sort);
      if (nFactPars!=ATERM_NON_EXISTING_POSITION) gsErrorMsg("unknown operation/variable %P with %d argument%s that matches type %P\n",
            Name, nFactPars, (nFactPars != 1)?"s":"", PosType);
      else
      {
        gsErrorMsg("unknown operation/variable %P that matches type %P\n",Name,PosType);
      }
      return NULL;
    }

    if (ATgetLength(ParList)==1)
    {
      ATermAppl Type=ATAgetFirst(ParList);
      ATermAppl OldType=Type;
      if (gstcHasUnknown(Type))
      {
        Type=gstcTypeMatchA(Type,PosType);
      }

      if (gstcHasUnknown(Type) && gsIsOpId(*DataTerm))
      {
        Type=gstcTypeMatchA(Type,ATAgetArgument(*DataTerm,1));
      }
      if (Type==NULL)
      {
        gsErrorMsg("fail to match sort %P with %P\n",OldType,PosType);
        return NULL;
      }

      if (ATisEqual(static_cast<ATermAppl>(data::detail::if_symbol()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing if matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchIf(Type);
        if (!NewType)
        {
          gsErrorMsg("the function if has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(data::detail::equal_symbol()),ATAgetArgument(*DataTerm,0))
          || ATisEqual(static_cast<ATermAppl>(data::detail::not_equal_symbol()),ATAgetArgument(*DataTerm,0))
          || ATisEqual(static_cast<ATermAppl>(data::detail::less_symbol()),ATAgetArgument(*DataTerm,0))
          || ATisEqual(static_cast<ATermAppl>(data::detail::less_equal_symbol()),ATAgetArgument(*DataTerm,0))
          || ATisEqual(static_cast<ATermAppl>(data::detail::greater_symbol()),ATAgetArgument(*DataTerm,0))
          || ATisEqual(static_cast<ATermAppl>(data::detail::greater_equal_symbol()),ATAgetArgument(*DataTerm,0))
         )
      {
        if (gsDebug)
        {
          std::cerr << "Doing ==, !=, <, <=, >= or > matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchEqNeqComparison(Type);
        if (!NewType)
        {
          gsErrorMsg("the function %P has incompatible argument types %P (while typechecking %P)\n",ATAgetArgument(*DataTerm,0),Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_list::cons_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing |> matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchListOpCons(Type);
        if (!NewType)
        {
          gsErrorMsg("the function |> has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_list::snoc_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing <| matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchListOpSnoc(Type);
        if (!NewType)
        {
          gsErrorMsg("the function <| has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_list::concat_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing ++ matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchListOpConcat(Type);
        if (!NewType)
        {
          gsErrorMsg("the function |> has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_list::element_at_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing @ matching Type " << pp(Type) << ", PosType " << pp(PosType) << ", DataTerm: " << pp(*DataTerm) << "\n";
        }
        ATermAppl NewType=gstcMatchListOpEltAt(Type);
        if (!NewType)
        {
          gsErrorMsg("the function @ has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_list::head_name()),ATAgetArgument(*DataTerm,0))||
          ATisEqual(static_cast<ATermAppl>(sort_list::rhead_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing {R,L}head matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
// Type==NULL
        ATermAppl NewType=gstcMatchListOpHead(Type);
        if (!NewType)
        {
          gsErrorMsg("the function {R,L}head has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_list::tail_name()),ATAgetArgument(*DataTerm,0))||
          ATisEqual(static_cast<ATermAppl>(sort_list::rtail_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing {R,L}tail matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchListOpTail(Type);
        if (!NewType)
        {
          gsErrorMsg("the function {R,L}tail has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_bag::set2bag_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing Set2Bag matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchSetOpSet2Bag(Type);
        if (!NewType)
        {
          gsErrorMsg("the function Set2Bag has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_list::in_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing {List,Set,Bag} matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchListSetBagOpIn(Type);
        if (!NewType)
        {
          gsErrorMsg("the function {List,Set,Bag}In has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_set::setunion_name()),ATAgetArgument(*DataTerm,0))||
          ATisEqual(static_cast<ATermAppl>(sort_set::setdifference_name()),ATAgetArgument(*DataTerm,0))||
          ATisEqual(static_cast<ATermAppl>(sort_set::setintersection_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing {Set,Bag}{Union,Difference,Intersect} matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchSetBagOpUnionDiffIntersect(Type);
        if (!NewType)
        {
          gsErrorMsg("the function {Set,Bag}{Union,Difference,Intersect} has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_set::setcomplement_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing SetCompl matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchSetOpSetCompl(Type);
        if (!NewType)
        {
          gsErrorMsg("the function SetCompl has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_bag::bag2set_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing Bag2Set matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchBagOpBag2Set(Type);
        if (!NewType)
        {
          gsErrorMsg("the function Bag2Set has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }

      if (ATisEqual(static_cast<ATermAppl>(sort_bag::bagcount_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing BagCount matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchBagOpBagCount(Type);
        if (!NewType)
        {
          gsErrorMsg("the function BagCount has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }


      if (ATisEqual(static_cast<ATermAppl>(data::function_update_name()),ATAgetArgument(*DataTerm,0)))
      {
        if (gsDebug)
        {
          std::cerr << "Doing FuncUpdate matching Type " << pp(Type) << ", PosType " << pp(PosType) << "\n";
        }
        ATermAppl NewType=gstcMatchFuncUpdate(Type);
        if (!NewType)
        {
          gsErrorMsg("function update has incompatible argument types %P (while typechecking %P)\n",Type,*DataTerm);
          return NULL;
        }
        Type=NewType;
      }


      *DataTerm=gsMakeOpId(Name,Type);
      if (variable)
      {
        *DataTerm=gsMakeDataVarId(Name,Type);
      }

      assert(Type);
      return Type;
    }
    else
    {
      was_ambiguous=true;
      if (strict_ambiguous)
      {
        if (gsDebug)
        {
          std::cerr << "ambiguous operation " << pp(Name) << " (ParList " << pp(ParList) << ")\n";
        }
        if (nFactPars!=ATERM_NON_EXISTING_POSITION)
        {
          gsErrorMsg("ambiguous operation %P with %d parameter%s\n", Name, nFactPars, (nFactPars != 1)?"s":"");
        }
        else
        {
          gsErrorMsg("ambiguous operation %P\n", Name);
        }
        return NULL;
      }
      else
      {
        //*DataTerm=gsMakeOpId(Name,data::unknown_sort());
        //if(variable) *DataTerm=gsMakeDataVarId(Name,data::unknown_sort());
        return data::unknown_sort();
      }
    }
  }
  else
  {
    return gstcTraverseVarConsTypeD(DeclaredVars,AllowedVars,DataTerm,PosType,FreeVars,strict_ambiguous,warn_upcasting);
  }
}

// ================================================================================
// Phase 2 -- type inference
// ================================================================================
static ATermList gstcGetNotInferredList(ATermList TypeListList)
{
  //we get: List of Lists of SortExpressions
  //Outer list: possible parameter types 0..nPosParsVectors-1
  //inner lists: parameter types vectors 0..nFormPars-1

  //we constuct 1 vector (list) of sort expressions (NotInferred if ambiguous)
  //0..nFormPars-1

  ATermList Result=ATmakeList0();
  size_t nFormPars=ATgetLength((ATermList)ATgetFirst(TypeListList));
  ATermList* Pars = NULL;
  if (nFormPars > 0)
  {
    Pars = (ATermList*) malloc(nFormPars*sizeof(ATermList));
  }
  //DECLA(ATermList,Pars,nFormPars);
  for (size_t i=0; i<nFormPars; i++)
  {
    Pars[i]=ATmakeList0();
  }

  for (; !ATisEmpty(TypeListList); TypeListList=ATgetNext(TypeListList))
  {
    ATermList TypeList=ATLgetFirst(TypeListList);
    for (size_t i=0; i<nFormPars; TypeList=ATgetNext(TypeList),i++)
    {
      Pars[i]=gstcInsertType(Pars[i],ATAgetFirst(TypeList));
    }
  }

  for (size_t i=nFormPars; i>0; i--)
  {
    ATermAppl Sort;
    if (ATgetLength(Pars[i-1])==1)
    {
      Sort=ATAgetFirst(Pars[i-1]);
    }
    else
    {
      Sort=multiple_possible_sorts(atermpp::aterm_list(ATreverse(Pars[i-1])));
    }
    Result=ATinsert(Result,(ATerm)Sort);
  }
  free(Pars);
  return Result;
}

static ATermAppl gstcUpCastNumericType(ATermAppl NeededType, ATermAppl Type, ATermAppl* Par, bool warn_upcasting)
{
  // Makes upcasting from Type to Needed Type for Par. Returns the resulting type.
  // Moreover, *Par is extended with the required type transformations.

  if (gsDebug)
  {
    std::cerr << "gstcUpCastNumericType " << pp(NeededType) << " -- " << pp(Type) << "\n";
  }

  if (data::is_unknown_sort(data::sort_expression(Type)))
  {
    return Type;
  }
  if (data::is_unknown_sort(data::sort_expression(NeededType)))
  {
    return Type;
  }
  if (gstcEqTypesA(NeededType,Type))
  {
    return Type;
  }

  if (warn_upcasting && gsIsOpId(*Par) && gsIsNumericString(gsATermAppl2String(ATAgetArgument(*Par,0))))
  {
    warn_upcasting=false;
  }

  // Try Upcasting to Pos
  if (gstcTypeMatchA(NeededType,sort_pos::pos()))
  {
    if (gstcTypeMatchA(Type,sort_pos::pos()))
    {
      return sort_pos::pos();
    }
  }

  // Try Upcasting to Nat
  if (gstcTypeMatchA(NeededType,sort_nat::nat()))
  {
    if (gstcTypeMatchA(Type,sort_pos::pos()))
    {
      ATermAppl OldPar=*Par;
      *Par=gsMakeDataAppl(sort_nat::cnat(),ATmakeList1((ATerm)*Par));
      if (warn_upcasting)
      {
        was_warning_upcasting=true;
        gsWarningMsg("Upcasting %P to sort Nat by applying Pos2Nat to it.\n",OldPar);
      }
      return sort_nat::nat();
    }
    if (gstcTypeMatchA(Type,sort_nat::nat()))
    {
      return sort_nat::nat();
    }
  }

  // Try Upcasting to Int
  if (gstcTypeMatchA(NeededType,sort_int::int_()))
  {
    if (gstcTypeMatchA(Type,sort_pos::pos()))
    {
      ATermAppl OldPar=*Par;
      *Par=gsMakeDataAppl(sort_int::cint(),ATmakeList1((ATerm)gsMakeDataAppl(sort_nat::cnat(),ATmakeList1((ATerm)*Par))));
      if (warn_upcasting)
      {
        was_warning_upcasting=true;
        gsWarningMsg("Upcasting %P to sort Int by applying Pos2Int to it.\n",OldPar);
      }
      return sort_int::int_();
    }
    if (gstcTypeMatchA(Type,sort_nat::nat()))
    {
      ATermAppl OldPar=*Par;
      *Par=gsMakeDataAppl(sort_int::cint(),ATmakeList1((ATerm)*Par));
      if (warn_upcasting)
      {
        was_warning_upcasting=true;
        gsWarningMsg("Upcasting %P to sort Int by applying Nat2Int to it.\n",OldPar);
      }
      return sort_int::int_();
    }
    if (gstcTypeMatchA(Type,sort_int::int_()))
    {
      return sort_int::int_();
    }
  }

  // Try Upcasting to Real
  if (gstcTypeMatchA(NeededType,sort_real::real_()))
  {
    if (gstcTypeMatchA(Type,sort_pos::pos()))
    {
      ATermAppl OldPar=*Par;
      *Par=gsMakeDataAppl(sort_real::creal(),ATmakeList2((ATerm)gsMakeDataAppl(sort_int::cint(),
                          ATmakeList1((ATerm)gsMakeDataAppl(sort_nat::cnat(),ATmakeList1((ATerm)*Par)))),
                          // (ATerm)gsMakeOpId(ATmakeAppl0(ATmakeAFun("1",0,ATtrue)),(ATermAppl)sort_pos::pos())));
                          (ATerm)(ATermAppl)sort_pos::c1()));
      if (warn_upcasting)
      {
        was_warning_upcasting=true;
        gsWarningMsg("Upcasting %P to sort Real by applying Pos2Real to it.\n",OldPar);
      }
      return sort_real::real_();
    }
    if (gstcTypeMatchA(Type,sort_nat::nat()))
    {
      ATermAppl OldPar=*Par;
      *Par=gsMakeDataAppl(sort_real::creal(),ATmakeList2((ATerm)gsMakeDataAppl(sort_int::cint(),ATmakeList1((ATerm)*Par)),
                          // (ATerm)gsMakeOpId(ATmakeAppl0(ATmakeAFun("1",0,ATtrue)),(ATermAppl)sort_pos::pos())));
                          (ATerm)(ATermAppl)(sort_pos::c1())));
      if (warn_upcasting)
      {
        was_warning_upcasting=true;
        gsWarningMsg("Upcasting %P to sort Real by applying Nat2Real to it.",OldPar);
      }
      return sort_real::real_();
    }
    if (gstcTypeMatchA(Type,sort_int::int_()))
    {
      ATermAppl OldPar=*Par;
      *Par=gsMakeDataAppl(sort_real::creal(),ATmakeList2((ATerm)*Par,(ATerm)
                          // (ATerm)gsMakeOpId(ATmakeAppl0(ATmakeAFun("1",0,ATtrue)),(ATermAppl)sort_pos::pos())));
                          (ATermAppl)data_expression(sort_pos::c1())));
      if (warn_upcasting)
      {
        was_warning_upcasting=true;
        gsWarningMsg("Upcasting %P to sort Real by applying Int2Real to it.\n",OldPar);
      }
      return sort_real::real_();
    }
    if (gstcTypeMatchA(Type,sort_real::real_()))
    {
      return sort_real::real_();
    }
  }

  return NULL;
}

static ATermList gstcInsertType(ATermList TypeList, ATermAppl Type)
{
  for (ATermList OldTypeList=TypeList; !ATisEmpty(OldTypeList); OldTypeList=ATgetNext(OldTypeList))
  {
    if (gstcEqTypesA(ATAgetFirst(OldTypeList),Type))
    {
      return TypeList;
    }
  }
  return ATinsert(TypeList,(ATerm)Type);
}

static ATermList gstcTypeListsIntersect(ATermList TypeListList1, ATermList TypeListList2)
{
  // returns the intersection of the 2 type list lists

  // if (gsDebug) { std::cerr << "gstcTypesIntersect:  TypeListList1 %T;    TypeListList2: %T\n",TypeListList1,TypeListList2);

  ATermList Result=ATmakeList0();

  for (; !ATisEmpty(TypeListList2); TypeListList2=ATgetNext(TypeListList2))
  {
    ATermList TypeList2=ATLgetFirst(TypeListList2);
    if (gstcInTypesL(TypeList2,TypeListList1))
    {
      Result=ATinsert(Result,(ATerm)TypeList2);
    }
  }
  return ATreverse(Result);
}

static ATermList gstcAdjustNotInferredList(ATermList PosTypeList, ATermList TypeListList)
{
  // PosTypeList -- List of Sortexpressions (possibly NotInferred(List Sortexpr))
  // TypeListList -- List of (Lists of Types)
  // returns: PosTypeList, adjusted to the elements of TypeListList
  // NULL if cannot be ajusted.

  // if (gsDebug) { std::cerr << "gstcAdjustNotInferredList: PosTypeList %T;    TypeListList:%T \n",PosTypeList,TypeListList);

  //if PosTypeList has only normal types -- check if it is in TypeListList,
  //if so return PosTypeList, otherwise return NULL
  if (!gstcIsNotInferredL(PosTypeList))
  {
    if (gstcInTypesL(PosTypeList,TypeListList))
    {
      return PosTypeList;
    }
    else
    {
      return NULL;
    }
  }

  //Filter TypeListList to contain only compatible with TypeList lists of parameters.
  ATermList NewTypeListList=ATmakeList0();
  for (; !ATisEmpty(TypeListList); TypeListList=ATgetNext(TypeListList))
  {
    ATermList TypeList=ATLgetFirst(TypeListList);
    if (gstcIsTypeAllowedL(TypeList,PosTypeList))
    {
      NewTypeListList=ATinsert(NewTypeListList,(ATerm)TypeList);
    }
  }
  if (ATisEmpty(NewTypeListList))
  {
    return NULL;
  }
  if (ATgetLength(NewTypeListList)==1)
  {
    return ATLgetFirst(NewTypeListList);
  }

  // otherwise return not inferred.
  return gstcGetNotInferredList(ATreverse(NewTypeListList));
}

static bool gstcIsTypeAllowedL(ATermList TypeList, ATermList PosTypeList)
{
  //Checks if TypeList is allowed by PosTypeList (each respective element)
  assert(ATgetLength(TypeList)==ATgetLength(PosTypeList));
  for (; !ATisEmpty(TypeList); TypeList=ATgetNext(TypeList),PosTypeList=ATgetNext(PosTypeList))
    if (!gstcIsTypeAllowedA(ATAgetFirst(TypeList),ATAgetFirst(PosTypeList)))
    {
      return false;
    }
  return true;
}

static bool gstcIsTypeAllowedA(ATermAppl Type, ATermAppl PosType)
{
  //Checks if Type is allowed by PosType
  if (data::is_unknown_sort(data::sort_expression(PosType)))
  {
    return true;
  }
  if (gsIsSortsPossible(PosType))
  {
    return gstcInTypesA(Type,ATLgetArgument(PosType,0));
  }

  //PosType is a normal type
  return gstcEqTypesA(Type,PosType);
}

static ATermAppl gstcTypeMatchA(ATermAppl Type, ATermAppl PosType)
{
  // Checks if Type and PosType match by instantiating unknown sorts.
  // It returns the matching instantiation of Type. If matching fails,
  // it returns NULL.

  if (gsDebug)
  {
    std::cerr << "gstcTypeMatchA Type: " << pp(Type) << ";    PosType: " << pp(PosType) << " \n";
  }

  if (data::is_unknown_sort(Type))
  {
    return PosType;
  }
  if (data::is_unknown_sort(PosType) || gstcEqTypesA(Type,PosType))
  {
    return Type;
  }
  if (gsIsSortsPossible(Type) && !gsIsSortsPossible(PosType))
  {
    ATermAppl TmpType=PosType;
    PosType=Type;
    Type=TmpType;
  }
  if (gsIsSortsPossible(PosType))
  {
    ATermList NewTypeList=ATmakeList0();
    for (ATermList PosTypeList=ATLgetArgument(PosType,0); !ATisEmpty(PosTypeList); PosTypeList=ATgetNext(PosTypeList))
    {
      ATermAppl NewPosType=ATAgetFirst(PosTypeList);
      if (gsDebug)
      {
        std::cerr << "Matching candidate gstcTypeMatchA Type: " << pp(Type) << ";    PosType: "
                  << pp(PosType) << " New Type: " << pp(NewPosType) << "\n";
      }

      if ((NewPosType=gstcTypeMatchA(Type,NewPosType)))
      {
        if (gsDebug)
        {
          std::cerr << "Match gstcTypeMatchA Type: " << pp(Type) << ";    PosType: " << pp(PosType) <<
                    " New Type: " << pp(NewPosType) << "\n";
        }
        NewTypeList=ATinsert(NewTypeList,(ATerm)NewPosType);
      }
    }
    if (ATisEmpty(NewTypeList))
    {
      if (gsDebug)
      {
        std::cerr << "No match gstcTypeMatchA Type: " << pp(Type) << ";    PosType: " << pp(PosType) << " \n";
      }
      return NULL;
    }

    if (ATisEmpty(ATgetNext(NewTypeList)))
    {
      return ATAgetFirst(NewTypeList);
    }

    return multiple_possible_sorts(atermpp::aterm_list(ATreverse(NewTypeList)));
  }

  //PosType is a normal type
  //if(!gstcHasUnknown(Type)) return NULL;

  if (gsIsSortId(Type))
  {
    Type=gstcUnwindType(Type);
  }
  if (gsIsSortId(PosType))
  {
    PosType=gstcUnwindType(PosType);
  }
  if (gsIsSortCons(Type))
  {
    ATermAppl ConsType = ATAgetArgument(Type, 0);
    if (gsIsSortList(ConsType))
    {
      if (!sort_list::is_list(sort_expression(PosType)))
      {
        return NULL;
      }
      ATermAppl Res=gstcTypeMatchA(ATAgetArgument(Type,1),ATAgetArgument(PosType,1));
      if (!Res)
      {
        return NULL;
      }
      return sort_list::list(sort_expression(Res));
    }

    if (gsIsSortSet(ConsType))
    {
      if (!sort_set::is_set(sort_expression(PosType)))
      {
        return NULL;
      }
      else
      {
        ATermAppl Res=gstcTypeMatchA(ATAgetArgument(Type,1),ATAgetArgument(PosType,1));
        if (!Res)
        {
          return NULL;
        }
        return sort_set::set_(sort_expression(Res));
      }
    }

    if (gsIsSortBag(ConsType))
    {
      if (!sort_bag::is_bag(sort_expression(PosType)))
      {
        return NULL;
      }
      else
      {
        ATermAppl Res=gstcTypeMatchA(ATAgetArgument(Type,1),ATAgetArgument(PosType,1));
        if (!Res)
        {
          return NULL;
        }
        return sort_bag::bag(sort_expression(Res));
      }
    }
  }

  if (gsIsSortArrow(Type))
  {
    if (!gsIsSortArrow(PosType))
    {
      return NULL;
    }
    else
    {
      ATermList ArgTypes=gstcTypeMatchL(ATLgetArgument(Type,0),ATLgetArgument(PosType,0));
      if (!ArgTypes)
      {
        return NULL;
      }
      ATermAppl ResType=gstcTypeMatchA(ATAgetArgument(Type,1),ATAgetArgument(PosType,1));
      if (!ResType)
      {
        return NULL;
      }
      Type=gsMakeSortArrow(ArgTypes,ResType);
      if (gsDebug)
      {
        std::cerr << "gstcTypeMatchA Done: Type: " << pp(Type) << ";    PosType: " << pp(PosType) << "\n";
      }
      return Type;
    }
  }

  return NULL;
}

static ATermList gstcTypeMatchL(ATermList TypeList, ATermList PosTypeList)
{
  if (gsDebug)
  {
    std::cerr << "gstcTypeMatchL TypeList: " << pp(TypeList) << ";    PosTypeList: " <<
              pp(PosTypeList) << "\n";
  }

  if (ATgetLength(TypeList)!=ATgetLength(PosTypeList))
  {
    return NULL;
  }

  ATermList Result=ATmakeList0();
  for (; !ATisEmpty(TypeList); TypeList=ATgetNext(TypeList),PosTypeList=ATgetNext(PosTypeList))
  {
    ATermAppl Type=gstcTypeMatchA(ATAgetFirst(TypeList),ATAgetFirst(PosTypeList));
    if (!Type)
    {
      return NULL;
    }
    Result=ATinsert(Result,(ATerm)gstcTypeMatchA(ATAgetFirst(TypeList),ATAgetFirst(PosTypeList)));
  }
  return ATreverse(Result);
}

static bool gstcIsNotInferredL(ATermList TypeList)
{
  for (; !ATisEmpty(TypeList); TypeList=ATgetNext(TypeList))
  {
    ATermAppl Type=ATAgetFirst(TypeList);
    if (is_unknown_sort(Type) || is_multiple_possible_sorts(Type))
    {
      return true;
    }
  }
  return false;
}

static ATermAppl gstcUnwindType(ATermAppl Type)
{
  // if (gsDebug) { std::cerr << "gstcUnwindType Type: " << pp(Type) << "\n"; } Becomes too verbose.

  if (gsIsSortCons(Type))
  {
    return ATsetArgument(Type,(ATerm)gstcUnwindType(ATAgetArgument(Type,1)),1);
  }
  if (gsIsSortArrow(Type))
  {
    Type=ATsetArgument(Type,(ATerm)gstcUnwindType(ATAgetArgument(Type,1)),1);
    ATermList Args=ATLgetArgument(Type,0);
    ATermList NewArgs=ATmakeList0();
    for (; !ATisEmpty(Args); Args=ATgetNext(Args))
    {
      NewArgs=ATinsert(NewArgs,(ATerm)gstcUnwindType(ATAgetFirst(Args)));
    }
    NewArgs=ATreverse(NewArgs);
    Type=ATsetArgument(Type,(ATerm)NewArgs,0);
    return Type;
  }

  if (gsIsSortId(Type))
  {
    ATermAppl Value=ATAtableGet(context.defined_sorts,(ATerm)ATAgetArgument(Type,0));
    if (!Value)
    {
      return Type;
    }
    return gstcUnwindType(Value);
  }

  return Type;
}

static ATermAppl gstcUnSet(ATermAppl PosType)
{
  //select Set(Type), elements, return their list of arguments.
  if (gsIsSortId(PosType))
  {
    PosType=gstcUnwindType(PosType);
  }
  if (sort_set::is_set(sort_expression(PosType)))
  {
    return ATAgetArgument(PosType,1);
  }
  if (data::is_unknown_sort(data::sort_expression(PosType)))
  {
    return PosType;
  }

  ATermList NewPosTypes=ATmakeList0();
  if (gsIsSortsPossible(PosType))
  {
    for (ATermList PosTypes=ATLgetArgument(PosType,0); !ATisEmpty(PosTypes); PosTypes=ATgetNext(PosTypes))
    {
      ATermAppl NewPosType=ATAgetFirst(PosTypes);
      if (gsIsSortId(NewPosType))
      {
        NewPosType=gstcUnwindType(NewPosType);
      }
      if (sort_set::is_set(sort_expression(NewPosType)))
      {
        NewPosType=ATAgetArgument(NewPosType,1);
      }
      else if (!data::is_unknown_sort(data::sort_expression(NewPosType)))
      {
        continue;
      }
      NewPosTypes=ATinsert(NewPosTypes,(ATerm)NewPosType);
    }
    NewPosTypes=ATreverse(NewPosTypes);
    return multiple_possible_sorts(atermpp::aterm_list(NewPosTypes));
  }
  return NULL;
}

static ATermAppl gstcUnBag(ATermAppl PosType)
{
  //select Bag(Type), elements, return their list of arguments.
  if (gsIsSortId(PosType))
  {
    PosType=gstcUnwindType(PosType);
  }
  if (sort_bag::is_bag(sort_expression(PosType)))
  {
    return ATAgetArgument(PosType,1);
  }
  if (data::is_unknown_sort(data::sort_expression(PosType)))
  {
    return PosType;
  }

  ATermList NewPosTypes=ATmakeList0();
  if (gsIsSortsPossible(PosType))
  {
    for (ATermList PosTypes=ATLgetArgument(PosType,0); !ATisEmpty(PosTypes); PosTypes=ATgetNext(PosTypes))
    {
      ATermAppl NewPosType=ATAgetFirst(PosTypes);
      if (gsIsSortId(NewPosType))
      {
        NewPosType=gstcUnwindType(NewPosType);
      }
      if (sort_bag::is_bag(sort_expression(NewPosType)))
      {
        NewPosType=ATAgetArgument(NewPosType,1);
      }
      else if (!data::is_unknown_sort(data::sort_expression(NewPosType)))
      {
        continue;
      }
      NewPosTypes=ATinsert(NewPosTypes,(ATerm)NewPosType);
    }
    NewPosTypes=ATreverse(NewPosTypes);
    return multiple_possible_sorts(atermpp::aterm_list(NewPosTypes));
  }
  return NULL;
}

static ATermAppl gstcUnList(ATermAppl PosType)
{
  //select List(Type), elements, return their list of arguments.
  if (gsIsSortId(PosType))
  {
    PosType=gstcUnwindType(PosType);
  }
  if (sort_list::is_list(sort_expression(PosType)))
  {
    return ATAgetArgument(PosType,1);
  }
  if (data::is_unknown_sort(data::sort_expression(PosType)))
  {
    return PosType;
  }

  ATermList NewPosTypes=ATmakeList0();
  if (gsIsSortsPossible(PosType))
  {
    for (ATermList PosTypes=ATLgetArgument(PosType,0); !ATisEmpty(PosTypes); PosTypes=ATgetNext(PosTypes))
    {
      ATermAppl NewPosType=ATAgetFirst(PosTypes);
      if (gsIsSortId(NewPosType))
      {
        NewPosType=gstcUnwindType(NewPosType);
      }
      if (sort_list::is_list(sort_expression(NewPosType)))
      {
        NewPosType=ATAgetArgument(NewPosType,1);
      }
      else if (!data::is_unknown_sort(data::sort_expression(NewPosType)))
      {
        continue;
      }
      NewPosTypes=ATinsert(NewPosTypes,(ATerm)NewPosType);
    }
    NewPosTypes=ATreverse(NewPosTypes);
    return multiple_possible_sorts(atermpp::aterm_list(NewPosTypes));
  }
  return NULL;
}

static ATermAppl gstcUnArrowProd(ATermList ArgTypes, ATermAppl PosType)
{
  //Filter PosType to contain only functions ArgTypes -> TypeX
  //return TypeX if unique, the set of TypeX as NotInferred if many, NULL otherwise

  // if (gsDebug) { std::cerr << "gstcUnArrowProd: ArgTypes %T with PosType %T\n",ArgTypes,PosType);

  if (gsIsSortId(PosType))
  {
    PosType=gstcUnwindType(PosType);
  }
  if (gsIsSortArrow(PosType))
  {
    ATermList PosArgTypes=ATLgetArgument(PosType,0);

    // if (gsDebug) { std::cerr << "gstcUnArrowProd: PosArgTypes %T \n",PosArgTypes);

    if (ATgetLength(PosArgTypes)!=ATgetLength(ArgTypes))
    {
      return NULL;
    }
    if (gstcTypeMatchL(PosArgTypes,ArgTypes))
    {
      return ATAgetArgument(PosType,1);
    }
  }
  if (data::is_unknown_sort(data::sort_expression(PosType)))
  {
    return PosType;
  }

  ATermList NewPosTypes=ATmakeList0();
  if (gsIsSortsPossible(PosType))
  {
    for (ATermList PosTypes=ATLgetArgument(PosType,0); !ATisEmpty(PosTypes); PosTypes=ATgetNext(PosTypes))
    {
      ATermAppl NewPosType=ATAgetFirst(PosTypes);
      if (gsIsSortId(NewPosType))
      {
        NewPosType=gstcUnwindType(NewPosType);
      }
      if (gsIsSortArrow(PosType))
      {
        ATermList PosArgTypes=ATLgetArgument(PosType,0);
        if (ATgetLength(PosArgTypes)!=ATgetLength(ArgTypes))
        {
          continue;
        }
        if (gstcTypeMatchL(PosArgTypes,ArgTypes))
        {
          NewPosType=ATAgetArgument(NewPosType,1);
        }
      }
      else if (!data::is_unknown_sort(data::sort_expression(NewPosType)))
      {
        continue;
      }
      NewPosTypes=ATinsertUnique(NewPosTypes,(ATerm)NewPosType);
    }
    NewPosTypes=ATreverse(NewPosTypes);
    return multiple_possible_sorts(atermpp::aterm_list(NewPosTypes));
  }
  return NULL;
}

static ATermList gstcGetVarTypes(ATermList VarDecls)
{
  ATermList Result=ATmakeList0();
  for (; !ATisEmpty(VarDecls); VarDecls=ATgetNext(VarDecls))
  {
    Result=ATinsert(Result,(ATerm)ATAgetArgument(ATAgetFirst(VarDecls),1));
  }
  return ATreverse(Result);
}

static bool gstcHasUnknown(ATermAppl Type)
{
  if (data::is_unknown_sort(data::sort_expression(Type)))
  {
    return true;
  }
  if (gsIsSortId(Type))
  {
    return false;
  }
  if (gsIsSortCons(Type))
  {
    return gstcHasUnknown(ATAgetArgument(Type,1));
  }
  if (gsIsSortStruct(Type))
  {
    return false;
  }

  if (gsIsSortArrow(Type))
  {
    for (ATermList TypeList=ATLgetArgument(Type,0); !ATisEmpty(TypeList); TypeList=ATgetNext(TypeList))
      if (gstcHasUnknown(ATAgetFirst(TypeList)))
      {
        return true;
      }
    return gstcHasUnknown(ATAgetArgument(Type,1));
  }

  return true;
}

static bool gstcIsNumericType(ATermAppl Type)
{
  //returns true if Type is Bool,Pos,Nat,Int or Real
  //otherwise return fase
  if (data::is_unknown_sort(data::sort_expression(Type)))
  {
    return false;
  }
  return (bool)(sort_bool::is_bool(sort_expression(Type))||
                  sort_pos::is_pos(sort_expression(Type))||
                  sort_nat::is_nat(sort_expression(Type))||
                  sort_int::is_int(sort_expression(Type))||
                  sort_real::is_real(sort_expression(Type)));
}

static ATermAppl gstcMaximumType(ATermAppl Type1, ATermAppl Type2)
{
  // if Type1 is convertible into Type2 or vice versa, the most general
  // of these types are returned. If not conversion is possible the result
  // is NULL. Conversions only apply between numerical types
  if (gstcEqTypesA(Type1,Type2))
  {
    return Type1;
  }
  if (data::is_unknown_sort(data::sort_expression(Type1)))
  {
    return Type2;
  }
  if (data::is_unknown_sort(data::sort_expression(Type2)))
  {
    return Type1;
  }
  if (gstcEqTypesA(Type1,sort_real::real_()))
  {
    if (gstcEqTypesA(Type2,sort_int::int_()))
    {
      return Type1;
    }
    if (gstcEqTypesA(Type2,sort_nat::nat()))
    {
      return Type1;
    }
    if (gstcEqTypesA(Type2,sort_pos::pos()))
    {
      return Type1;
    }
    return NULL;
  }
  if (gstcEqTypesA(Type1,sort_int::int_()))
  {
    if (gstcEqTypesA(Type2,sort_real::real_()))
    {
      return Type2;
    }
    if (gstcEqTypesA(Type2,sort_nat::nat()))
    {
      return Type1;
    }
    if (gstcEqTypesA(Type2,sort_pos::pos()))
    {
      return Type1;
    }
    return NULL;
  }
  if (gstcEqTypesA(Type1,sort_nat::nat()))
  {
    if (gstcEqTypesA(Type2,sort_real::real_()))
    {
      return Type2;
    }
    if (gstcEqTypesA(Type2,sort_int::int_()))
    {
      return Type2;
    }
    if (gstcEqTypesA(Type2,sort_pos::pos()))
    {
      return Type1;
    }
    return NULL;
  }
  if (gstcEqTypesA(Type1,sort_pos::pos()))
  {
    if (gstcEqTypesA(Type2,sort_real::real_()))
    {
      return Type2;
    }
    if (gstcEqTypesA(Type2,sort_int::int_()))
    {
      return Type2;
    }
    if (gstcEqTypesA(Type2,sort_nat::nat()))
    {
      return Type2;
    }
    return NULL;
  }
  return NULL;
}

static ATermAppl gstcExpandNumTypesUp(ATermAppl Type)
{
  //Expand Pos.. to possible bigger types.
  if (data::is_unknown_sort(data::sort_expression(Type)))
  {
    return Type;
  }
  if (gstcEqTypesA(sort_pos::pos(),Type))
  {
    return multiple_possible_sorts(atermpp::make_list(sort_pos::pos(), sort_nat::nat(), sort_int::int_(),sort_real::real_()));
  }
  if (gstcEqTypesA(sort_nat::nat(),Type))
  {
    return multiple_possible_sorts(atermpp::make_list(sort_nat::nat(), sort_int::int_(),sort_real::real_()));
  }
  if (gstcEqTypesA(sort_int::int_(),Type))
  {
    return multiple_possible_sorts(atermpp::make_list(sort_int::int_(), sort_real::real_()));
  }
  if (gsIsSortId(Type))
  {
    return Type;
  }
  if (gsIsSortCons(Type))
  {
    return ATsetArgument(Type,(ATerm)gstcExpandNumTypesUp(ATAgetArgument(Type,1)),1);
  }
  if (gsIsSortStruct(Type))
  {
    return Type;
  }

  if (gsIsSortArrow(Type))
  {
    //the argument types, and if the resulting type is SortArrow -- recursively
    ATermList NewTypeList=ATmakeList0();
    for (ATermList TypeList=ATLgetArgument(Type,0); !ATisEmpty(TypeList); TypeList=ATgetNext(TypeList))
    {
      NewTypeList=ATinsert(NewTypeList,(ATerm)gstcExpandNumTypesUp(gstcUnwindType(ATAgetFirst(TypeList))));
    }
    ATermAppl ResultType=ATAgetArgument(Type,1);
    if (!gsIsSortArrow(ResultType))
    {
      return ATsetArgument(Type,(ATerm)ATreverse(NewTypeList),0);
    }
    else
    {
      return gsMakeSortArrow(ATreverse(NewTypeList),gstcExpandNumTypesUp(gstcUnwindType(ResultType)));
    }
  }

  return Type;
}

static ATermAppl gstcExpandNumTypesDown(ATermAppl Type)
{
  // Expand Numeric types down
  if (data::is_unknown_sort(data::sort_expression(Type)))
  {
    return Type;
  }
  if (gsIsSortId(Type))
  {
    Type=gstcUnwindType(Type);
  }

  bool function=false;
  ATermList Args=NULL;
  if (gsIsSortArrow(Type))
  {
    function=true;
    Args=ATLgetArgument(Type,0);
    Type=ATAgetArgument(Type,1);
  }

  if (gstcEqTypesA(sort_real::real_(),Type))
  {
    Type=multiple_possible_sorts(atermpp::make_list(sort_pos::pos(),sort_nat::nat(),sort_int::int_(),sort_real::real_()));
  }
  if (gstcEqTypesA(sort_int::int_(),Type))
  {
    Type=multiple_possible_sorts(atermpp::make_list(sort_pos::pos(),sort_nat::nat(),sort_int::int_()));
  }
  if (gstcEqTypesA(sort_nat::nat(),Type))
  {
    Type=multiple_possible_sorts(atermpp::make_list(sort_pos::pos(),sort_nat::nat()));
  }

  return (function)?gsMakeSortArrow(Args,Type):Type;
}

static ATermAppl gstcMinType(ATermList TypeList)
{
  return ATAgetFirst(TypeList);
}


// =========================== MultiActions
static bool gstcMActIn(ATermList MAct, ATermList MActs)
{
  //returns true if MAct is in MActs
  for (; !ATisEmpty(MActs); MActs=ATgetNext(MActs))
    if (gstcMActEq(MAct,ATLgetFirst(MActs)))
    {
      return true;
    }

  return false;
}

static bool gstcMActEq(ATermList MAct1, ATermList MAct2)
{
  //returns true if the two multiactions are equal.
  if (ATgetLength(MAct1)!=ATgetLength(MAct2))
  {
    return false;
  }
  if (ATisEmpty(MAct1))
  {
    return true;
  }
  ATermAppl Act1=ATAgetFirst(MAct1);
  MAct1=ATgetNext(MAct1);

  //remove Act1 once from MAct2. if not there -- return ATfalse.
  ATermList NewMAct2=ATmakeList0();
  for (; !ATisEmpty(MAct2); MAct2=ATgetNext(MAct2))
  {
    ATermAppl Act2=ATAgetFirst(MAct2);
    if (ATisEqual(Act1,Act2))
    {
      MAct2=ATconcat(ATreverse(NewMAct2),ATgetNext(MAct2));
      goto gstcMActEq_found;
    }
    else
    {
      NewMAct2=ATinsert(NewMAct2,(ATerm)Act2);
    }
  }
  return false;
gstcMActEq_found:
  return gstcMActEq(MAct1,MAct2);
}

static bool gstcMActSubEq(ATermList MAct1, ATermList MAct2)
{
  //returns true if MAct1 is a submultiaction of MAct2.
  if (ATgetLength(MAct1)>ATgetLength(MAct2))
  {
    return false;
  }
  if (ATisEmpty(MAct1))
  {
    return true;
  }
  ATermAppl Act1=ATAgetFirst(MAct1);
  MAct1=ATgetNext(MAct1);

  //remove Act1 once from MAct2. if not there -- return ATfalse.
  ATermList NewMAct2=ATmakeList0();
  for (; !ATisEmpty(MAct2); MAct2=ATgetNext(MAct2))
  {
    ATermAppl Act2=ATAgetFirst(MAct2);
    if (ATisEqual(Act1,Act2))
    {
      MAct2=ATconcat(ATreverse(NewMAct2),ATgetNext(MAct2));
      goto gstcMActSubEqMA_found;
    }
    else
    {
      NewMAct2=ATinsert(NewMAct2,(ATerm)Act2);
    }
  }
  return false;
gstcMActSubEqMA_found:
  return gstcMActSubEq(MAct1,MAct2);
}

static ATermAppl gstcUnifyMinType(ATermAppl Type1, ATermAppl Type2)
{
  //Find the minimal type that Unifies the 2. If not possible, return NULL.
  ATermAppl Res=gstcTypeMatchA(Type1,Type2);
  if (!Res)
  {
    Res=gstcTypeMatchA(Type1,gstcExpandNumTypesUp(Type2));
    if (!Res)
    {
      Res=gstcTypeMatchA(Type2,gstcExpandNumTypesUp(Type1));
    }
    if (!Res)
    {
      if (gsDebug)
      {
        std::cerr << "gstcUnifyMinType: No match: Type1 " << pp(Type1) << "; Type2 " << pp(Type2) << "; \n";
      }
      return NULL;
    }
  }

  if (gsIsSortsPossible(Res))
  {
    Res=ATAgetFirst(ATLgetArgument(Res,0));
  }
  if (gsDebug)
  {
    std::cerr << "gstcUnifyMinType: Type1 " << pp(Type1) << "; Type2 " << pp(Type2) << "; Res: " << pp(Res) << "\n";
  }
  return Res;
}

static ATermAppl gstcMatchIf(ATermAppl Type)
{
  //tries to sort out the types for if.
  //If some of the parameters are Pos,Nat, or Int do upcasting

  assert(gsIsSortArrow(Type));
  ATermList Args=ATLgetArgument(Type,0);
  ATermAppl Res=ATAgetArgument(Type,1);
  assert((ATgetLength(Args)==3));
  //assert(gsIsBool(ATAgetFirst(Args)));
  Args=ATgetNext(Args);

  if (!(Res=gstcUnifyMinType(Res,ATAgetFirst(Args))))
  {
    return NULL;
  }
  Args=ATgetNext(Args);
  if (!(Res=gstcUnifyMinType(Res,ATAgetFirst(Args))))
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList3((ATerm)static_cast<ATermAppl>(sort_bool::bool_()),(ATerm)Res,(ATerm)Res),Res);
}

static ATermAppl gstcMatchEqNeqComparison(ATermAppl Type)
{
  //tries to sort out the types for ==, !=, <, <=, >= and >.
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==2));
  ATermAppl Arg1=ATAgetFirst(Args);
  Args=ATgetNext(Args);
  ATermAppl Arg2=ATAgetFirst(Args);

  ATermAppl Arg=gstcUnifyMinType(Arg1,Arg2);
  if (!Arg)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList2((ATerm)Arg,(ATerm)Arg),sort_bool::bool_());
}

static ATermAppl gstcMatchListOpCons(ATermAppl Type)
{
  //tries to sort out the types of Cons operations (SxList(S)->List(S))
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  // if (gsDebug) { std::cerr << "gstcMatchListOpCons: Type %T \n",Type);

  assert(gsIsSortArrow(Type));
  ATermAppl Res=ATAgetArgument(Type,1);
  if (gsIsSortId(Res))
  {
    Res=gstcUnwindType(Res);
  }
  assert(sort_list::is_list(sort_expression(gstcUnwindType(Res))));
  Res=ATAgetArgument(Res,1);
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==2));
  ATermAppl Arg1=ATAgetFirst(Args);
  Args=ATgetNext(Args);
  ATermAppl Arg2=ATAgetFirst(Args);
  if (gsIsSortId(Arg2))
  {
    Arg2=gstcUnwindType(Arg2);
  }
  assert(sort_list::is_list(sort_expression(Arg2)));
  Arg2=ATAgetArgument(Arg2,1);

  Res=gstcUnifyMinType(Res,Arg1);
  if (!Res)
  {
    return NULL;
  }

  Res=gstcUnifyMinType(Res,Arg2);
  if (!Res)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList2((ATerm)Res,(ATerm)static_cast<ATermAppl>(sort_list::list(sort_expression(Res)))),sort_list::list(sort_expression(Res)));
}

static ATermAppl gstcMatchListOpSnoc(ATermAppl Type)
{
  //tries to sort out the types of Cons operations (SxList(S)->List(S))
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));
  ATermAppl Res=ATAgetArgument(Type,1);
  if (gsIsSortId(Res))
  {
    Res=gstcUnwindType(Res);
  }
  assert(sort_list::is_list(sort_expression(Res)));
  Res=ATAgetArgument(Res,1);
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==2));
  ATermAppl Arg1=ATAgetFirst(Args);
  if (gsIsSortId(Arg1))
  {
    Arg1=gstcUnwindType(Arg1);
  }
  assert(sort_list::is_list(sort_expression(Arg1)));
  Arg1=ATAgetArgument(Arg1,1);

  Args=ATgetNext(Args);
  ATermAppl Arg2=ATAgetFirst(Args);

  Res=gstcUnifyMinType(Res,Arg1);
  if (!Res)
  {
    return NULL;
  }

  Res=gstcUnifyMinType(Res,Arg2);
  if (!Res)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList2((ATerm)static_cast<ATermAppl>(sort_list::list(sort_expression(Res))),(ATerm)Res),sort_list::list(sort_expression(Res)));
}

static ATermAppl gstcMatchListOpConcat(ATermAppl Type)
{
  //tries to sort out the types of Concat operations (List(S)xList(S)->List(S))
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));
  ATermAppl Res=ATAgetArgument(Type,1);
  if (gsIsSortId(Res))
  {
    Res=gstcUnwindType(Res);
  }
  assert(sort_list::is_list(sort_expression(Res)));
  Res=ATAgetArgument(Res,1);
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==2));

  ATermAppl Arg1=ATAgetFirst(Args);
  if (gsIsSortId(Arg1))
  {
    Arg1=gstcUnwindType(Arg1);
  }
  assert(sort_list::is_list(sort_expression(Arg1)));
  Arg1=ATAgetArgument(Arg1,1);

  Args=ATgetNext(Args);

  ATermAppl Arg2=ATAgetFirst(Args);
  if (gsIsSortId(Arg2))
  {
    Arg2=gstcUnwindType(Arg2);
  }
  assert(sort_list::is_list(sort_expression(Arg2)));
  Arg2=ATAgetArgument(Arg2,1);

  Res=gstcUnifyMinType(Res,Arg1);
  if (!Res)
  {
    return NULL;
  }

  Res=gstcUnifyMinType(Res,Arg2);
  if (!Res)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList2((ATerm)static_cast<ATermAppl>(sort_list::list(sort_expression(Res))),(ATerm)static_cast<ATermAppl>(sort_list::list(sort_expression(Res)))),sort_list::list(sort_expression(Res)));
}

static ATermAppl gstcMatchListOpEltAt(ATermAppl Type)
{
  //tries to sort out the types of EltAt operations (List(S)xNat->S)
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));
  ATermAppl Res=ATAgetArgument(Type,1);
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==2));

  ATermAppl Arg1=ATAgetFirst(Args);
  if (gsIsSortId(Arg1))
  {
    Arg1=gstcUnwindType(Arg1);
  }
  assert(sort_list::is_list(sort_expression(Arg1)));
  Arg1=ATAgetArgument(Arg1,1);

  Res=gstcUnifyMinType(Res,Arg1);
  if (!Res)
  {
    return NULL;
  }

  //assert((gsIsSortNat(ATAgetFirst(ATgetNext(Args))));

  return gsMakeSortArrow(ATmakeList2((ATerm)static_cast<ATermAppl>(sort_list::list(sort_expression(Res))),(ATerm)static_cast<ATermAppl>(sort_nat::nat())),Res);
}

static ATermAppl gstcMatchListOpHead(ATermAppl Type)
{
  //tries to sort out the types of Cons operations (SxList(S)->List(S))
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));
  ATermAppl Res=ATAgetArgument(Type,1);
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==1));
  ATermAppl Arg=ATAgetFirst(Args);
  if (gsIsSortId(Arg))
  {
    Arg=gstcUnwindType(Arg);
  }
  assert(sort_list::is_list(sort_expression(Arg)));
  Arg=ATAgetArgument(Arg,1);

  Res=gstcUnifyMinType(Res,Arg);
  if (!Res)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList1((ATerm)static_cast<ATermAppl>(sort_list::list(sort_expression(Res)))),Res);
}

static ATermAppl gstcMatchListOpTail(ATermAppl Type)
{
  //tries to sort out the types of Cons operations (SxList(S)->List(S))
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));
  ATermAppl Res=ATAgetArgument(Type,1);
  if (gsIsSortId(Res))
  {
    Res=gstcUnwindType(Res);
  }
  assert(sort_list::is_list(sort_expression(Res)));
  Res=ATAgetArgument(Res,1);
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==1));
  ATermAppl Arg=ATAgetFirst(Args);
  if (gsIsSortId(Arg))
  {
    Arg=gstcUnwindType(Arg);
  }
  assert(sort_list::is_list(sort_expression(Arg)));
  Arg=ATAgetArgument(Arg,1);

  Res=gstcUnifyMinType(Res,Arg);
  if (!Res)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList1((ATerm)static_cast<ATermAppl>(sort_list::list(sort_expression(Res)))),sort_list::list(sort_expression(Res)));
}

//Sets
static ATermAppl gstcMatchSetOpSet2Bag(ATermAppl Type)
{
  //tries to sort out the types of Set2Bag (Set(S)->Bag(s))
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));

  ATermAppl Res=ATAgetArgument(Type,1);
  if (gsIsSortId(Res))
  {
    Res=gstcUnwindType(Res);
  }
  assert(sort_bag::is_bag(sort_expression(Res)));
  Res=ATAgetArgument(Res,1);

  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==1));

  ATermAppl Arg=ATAgetFirst(Args);
  if (gsIsSortId(Arg))
  {
    Arg=gstcUnwindType(Arg);
  }
  assert(sort_set::is_set(sort_expression(Arg)));
  Arg=ATAgetArgument(Arg,1);

  Arg=gstcUnifyMinType(Arg,Res);
  if (!Arg)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList1((ATerm)static_cast<ATermAppl>(sort_set::set_(sort_expression(Arg)))),sort_bag::bag(sort_expression(Arg)));
}

static ATermAppl gstcMatchListSetBagOpIn(ATermAppl Type)
{
  //tries to sort out the type of EltIn (SxList(S)->Bool or SxSet(S)->Bool or SxBag(S)->Bool)
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));
  //assert(gsIsBool(ATAgetArgument(Type,1)));
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==2));

  ATermAppl Arg1=ATAgetFirst(Args);

  Args=ATgetNext(Args);
  ATermAppl Arg2=ATAgetFirst(Args);
  if (gsIsSortId(Arg2))
  {
    Arg2=gstcUnwindType(Arg2);
  }
  assert(gsIsSortCons(Arg2));
  ATermAppl Arg2s=ATAgetArgument(Arg2,1);

  ATermAppl Arg=gstcUnifyMinType(Arg1,Arg2s);
  if (!Arg)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList2((ATerm)Arg,(ATerm)ATsetArgument(Arg2,(ATerm)Arg,1)),sort_bool::bool_());
}

static ATermAppl gstcMatchSetBagOpUnionDiffIntersect(ATermAppl Type)
{
  //tries to sort out the types of Set or Bag Union, Diff or Intersect
  //operations (Set(S)xSet(S)->Set(S)). It can also be that this operation is
  //performed on numbers. In this case we do nothing.
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));
  ATermAppl Res=ATAgetArgument(Type,1);
  if (gsIsSortId(Res))
  {
    Res=gstcUnwindType(Res);
  }
  if (gstcIsNumericType(Res))
  {
    return Type;
  }
  assert(sort_set::is_set(sort_expression(Res))||sort_bag::is_bag(sort_expression(Res)));
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==2));

  ATermAppl Arg1=ATAgetFirst(Args);
  if (gsIsSortId(Arg1))
  {
    Arg1=gstcUnwindType(Arg1);
  }
  if (gstcIsNumericType(Arg1))
  {
    return Type;
  }
  assert(sort_set::is_set(sort_expression(Arg1))||sort_bag::is_bag(sort_expression(Arg1)));

  Args=ATgetNext(Args);

  ATermAppl Arg2=ATAgetFirst(Args);
  if (gsIsSortId(Arg2))
  {
    Arg2=gstcUnwindType(Arg2);
  }
  if (gstcIsNumericType(Arg2))
  {
    return Type;
  }
  assert(sort_set::is_set(sort_expression(Arg2))||sort_bag::is_bag(sort_expression(Arg2)));

  Res=gstcUnifyMinType(Res,Arg1);
  if (!Res)
  {
    return NULL;
  }

  Res=gstcUnifyMinType(Res,Arg2);
  if (!Res)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList2((ATerm)Res,(ATerm)Res),Res);
}

static ATermAppl gstcMatchSetOpSetCompl(ATermAppl Type)
{
  //tries to sort out the types of SetCompl operation (Set(S)->Set(S))
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));
  ATermAppl Res=ATAgetArgument(Type,1);
  if (gsIsSortId(Res))
  {
    Res=gstcUnwindType(Res);
  }
  if (gstcIsNumericType(Res))
  {
    return Type;
  }
  assert(sort_set::is_set(sort_expression(Res)));
  Res=ATAgetArgument(Res,1);
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==1));

  ATermAppl Arg=ATAgetFirst(Args);
  if (gsIsSortId(Arg))
  {
    Arg=gstcUnwindType(Arg);
  }
  if (gstcIsNumericType(Arg))
  {
    return Type;
  }
  assert(sort_set::is_set(sort_expression(Arg)));
  Arg=ATAgetArgument(Arg,1);

  Res=gstcUnifyMinType(Res,Arg);
  if (!Res)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList1((ATerm)static_cast<ATermAppl>(sort_set::set_(sort_expression(Res)))),sort_set::set_(sort_expression(Res)));
}

//Bags
static ATermAppl gstcMatchBagOpBag2Set(ATermAppl Type)
{
  //tries to sort out the types of Bag2Set (Bag(S)->Set(S))
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));

  ATermAppl Res=ATAgetArgument(Type,1);
  if (gsIsSortId(Res))
  {
    Res=gstcUnwindType(Res);
  }
  assert(sort_set::is_set(sort_expression(Res)));
  Res=ATAgetArgument(Res,1);

  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==1));

  ATermAppl Arg=ATAgetFirst(Args);
  if (gsIsSortId(Arg))
  {
    Arg=gstcUnwindType(Arg);
  }
  assert(sort_bag::is_bag(sort_expression(Arg)));
  Arg=ATAgetArgument(Arg,1);

  Arg=gstcUnifyMinType(Arg,Res);
  if (!Arg)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList1((ATerm)static_cast<ATermAppl>(sort_bag::bag(sort_expression(Arg)))),sort_set::set_(sort_expression(Arg)));
}

static ATermAppl gstcMatchBagOpBagCount(ATermAppl Type)
{
  //tries to sort out the types of BagCount (SxBag(S)->Nat)
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  //If the second argument is not a Bag, don't match

  if (!gsIsSortArrow(Type))
  {
    return Type;
  }
  //assert(gsIsNat(ATAgetArgument(Type,1)));
  ATermList Args=ATLgetArgument(Type,0);
  if (!(ATgetLength(Args)==2))
  {
    return Type;
  }

  ATermAppl Arg1=ATAgetFirst(Args);

  Args=ATgetNext(Args);
  ATermAppl Arg2=ATAgetFirst(Args);
  if (gsIsSortId(Arg2))
  {
    Arg2=gstcUnwindType(Arg2);
  }
  if (!sort_bag::is_bag(sort_expression(Arg2)))
  {
    return Type;
  }
  Arg2=ATAgetArgument(Arg2,1);

  ATermAppl Arg=gstcUnifyMinType(Arg1,Arg2);
  if (!Arg)
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList2((ATerm)Arg,(ATerm)static_cast<ATermAppl>(sort_bag::bag(sort_expression(Arg)))),sort_nat::nat());
}


static ATermAppl gstcMatchFuncUpdate(ATermAppl Type)
{
  //tries to sort out the types of FuncUpdate ((A->B)xAxB->(A->B))
  //If some of the parameters are Pos,Nat, or Int do upcasting.

  assert(gsIsSortArrow(Type));
  ATermList Args=ATLgetArgument(Type,0);
  assert((ATgetLength(Args)==3));
  ATermAppl Arg1=ATAgetFirst(Args);
  assert(gsIsSortArrow(Arg1));
  Args=ATgetNext(Args);
  ATermAppl Arg2=ATAgetFirst(Args);
  Args=ATgetNext(Args);
  ATermAppl Arg3=ATAgetFirst(Args);
  ATermAppl Res=ATAgetArgument(Type,1);
  assert(gsIsSortArrow(Res));

  Arg1=gstcUnifyMinType(Arg1,Res);
  if (!Arg1)
  {
    return NULL;
  }

  // determine A and B from Arg1:
  ATermList LA=ATLgetArgument(Arg1,0);
  assert((ATgetLength(LA)==1));
  ATermAppl A=ATAgetFirst(LA);
  ATermAppl B=ATAgetArgument(Arg1,1);

  if (!gstcUnifyMinType(A,Arg2))
  {
    return NULL;
  }
  if (!gstcUnifyMinType(B,Arg3))
  {
    return NULL;
  }

  return gsMakeSortArrow(ATmakeList3((ATerm)Arg1,(ATerm)A,(ATerm)B),Arg1);
}

static void gstcErrorMsgCannotCast(ATermAppl CandidateType, ATermList Arguments, ATermList ArgumentTypes)
{
  //prints more information about impossible cast.
  //at this point we know that Arguments cannot be cast to CandidateType. We need to find out why and print.
  assert(ATgetLength(Arguments)==ATgetLength(ArgumentTypes));

  // if (gsDebug) { std::cerr << "CandidateType: %T, Arguments %T, ArgumentTypes %T\n",CandidateType,Arguments,ArgumentTypes);

  ATermList CandidateList;
  if (gsIsSortsPossible(CandidateType))
  {
    CandidateList=ATLgetArgument(CandidateType,0);
  }
  else
  {
    CandidateList=ATmakeList1((ATerm)CandidateType);
  }

  ATermList NewCandidateList=ATmakeList0();
  for (ATermList l=CandidateList; !ATisEmpty(l); l=ATgetNext(l))
  {
    ATermAppl Candidate=ATAgetFirst(l);
    if (!gsIsSortArrow(Candidate))
    {
      continue;
    }
    NewCandidateList=ATinsert(NewCandidateList,(ATerm)ATLgetArgument(Candidate,0));
  }
  CandidateList=ATreverse(NewCandidateList);

  //CandidateList=gstcTraverseListList(CandidateList);
  ATermList CurrentCandidateList=CandidateList;
  CandidateList=ATmakeList0();
  while (true)
  {
    ATermList NewCurrentCandidateList=ATmakeList0();
    ATermList NewList=ATmakeList0();
    for (ATermList l=CurrentCandidateList; !ATisEmpty(l); l=ATgetNext(l))
    {
      ATermList List=ATLgetFirst(l);
      if (!ATisEmpty(List))
      {
        NewList=ATinsert(NewList,(ATerm)ATAgetFirst(List));
        NewCurrentCandidateList=ATinsertUnique(NewCurrentCandidateList,(ATerm)ATgetNext(List));
      }
      else
      {
        NewCurrentCandidateList=ATinsert(NewCurrentCandidateList,(ATerm)ATmakeList0());
      }
    }
    if (ATisEmpty(NewList))
    {
      break;
    }
    CurrentCandidateList=ATreverse(NewCurrentCandidateList);
    CandidateList=ATinsert(CandidateList,(ATerm)ATreverse(NewList));
  }
  CandidateList=ATreverse(CandidateList);

  for (ATermList l=Arguments, m=ArgumentTypes, n=CandidateList; !(ATisEmpty(l)||ATisEmpty(m)||ATisEmpty(n)); l=ATgetNext(l), m=ATgetNext(m), n=ATgetNext(n))
  {
    ATermList PosTypes=ATLgetFirst(n);
    ATermAppl NeededType=ATAgetFirst(m);
    bool found=true;
    for (ATermList k=PosTypes; !ATisEmpty(k); k=ATgetNext(k))
    {
      if (gstcTypeMatchA(ATAgetFirst(k),NeededType))
      {
        found=false;
        break;
      }
    }
    if (found)
    {
      ATermAppl Sort;
      if (ATgetLength(PosTypes)==1)
      {
        Sort=ATAgetFirst(PosTypes);
      }
      else
      {
        Sort=multiple_possible_sorts(atermpp::aterm_list(PosTypes));
      }
      gsErrorMsg("this is, for instance, because cannot cast %P to type %P\n",ATAgetFirst(l),Sort);
      break;
    }
  }
}

//===================================
// Type checking modal formulas
//===================================

static ATermAppl gstcTraverseStateFrm(ATermTable Vars, ATermTable StateVars, ATermAppl StateFrm)
{
  if (gsDebug)
  {
    std::cerr << "gstcTraverseStateFrm: " + pp(StateFrm) + "\n";
  }

  if (gsIsStateTrue(StateFrm) || gsIsStateFalse(StateFrm) || gsIsStateDelay(StateFrm) || gsIsStateYaled(StateFrm))
  {
    return StateFrm;
  }

  if (gsIsStateNot(StateFrm))
  {
    ATermAppl NewArg=gstcTraverseStateFrm(Vars,StateVars,ATAgetArgument(StateFrm,0));
    if (!NewArg)
    {
      return NULL;
    }
    return ATsetArgument(StateFrm,(ATerm)NewArg,0);
  }

  if (gsIsStateAnd(StateFrm) || gsIsStateOr(StateFrm) || gsIsStateImp(StateFrm))
  {
    ATermAppl NewArg1=gstcTraverseStateFrm(Vars,StateVars,ATAgetArgument(StateFrm,0));
    if (!NewArg1)
    {
      return NULL;
    }
    ATermAppl NewArg2=gstcTraverseStateFrm(Vars,StateVars,ATAgetArgument(StateFrm,1));
    if (!NewArg2)
    {
      return NULL;
    }
    return ATsetArgument(ATsetArgument(StateFrm,(ATerm)NewArg1,0),(ATerm)NewArg2,1);
  }

  if (gsIsStateForall(StateFrm) || gsIsStateExists(StateFrm))
  {
    ATermTable CopyVars=ATtableCreate(63,50);
    gstcATermTableCopy(Vars,CopyVars);

    ATermList VarList=ATLgetArgument(StateFrm,0);
    ATermTable NewVars=gstcAddVars2Table(CopyVars,VarList);
    if (!NewVars)
    {
      ATtableDestroy(CopyVars);
      return NULL;
    }

    ATermAppl NewArg2=gstcTraverseStateFrm(NewVars,StateVars,ATAgetArgument(StateFrm,1));
    if (!NewArg2)
    {
      return NULL;
    }
    ATtableDestroy(CopyVars);

    return ATsetArgument(StateFrm,(ATerm)NewArg2,1);
  }

  if (gsIsStateMust(StateFrm) || gsIsStateMay(StateFrm))
  {
    ATermAppl RegFrm=gstcTraverseRegFrm(Vars,ATAgetArgument(StateFrm,0));
    if (!RegFrm)
    {
      return NULL;
    }
    ATermAppl NewArg2=gstcTraverseStateFrm(Vars,StateVars,ATAgetArgument(StateFrm,1));
    if (!NewArg2)
    {
      return NULL;
    }
    return ATsetArgument(ATsetArgument(StateFrm,(ATerm)RegFrm,0),(ATerm)NewArg2,1);
  }

  if (gsIsStateDelayTimed(StateFrm) || gsIsStateYaledTimed(StateFrm))
  {
    ATermAppl Time=ATAgetArgument(StateFrm,0);
    ATermAppl NewType=gstcTraverseVarConsTypeD(Vars,Vars,&Time,gstcExpandNumTypesDown(sort_real::real_()));
    if (!NewType)
    {
      return NULL;
    }

    if (!gstcTypeMatchA(sort_real::real_(),NewType))
    {
      //upcasting
      ATermAppl CastedNewType=gstcUpCastNumericType(sort_real::real_(),NewType,&Time);
      if (!CastedNewType)
      {
        gsErrorMsg("cannot (up)cast time value %P to type Real (typechecking state formula %P)\n",Time,StateFrm);
        return NULL;
      }
    }
    return ATsetArgument(StateFrm,(ATerm)Time,0);
  }

  if (gsIsStateVar(StateFrm))
  {
    ATermAppl StateVarName=ATAgetArgument(StateFrm,0);
    ATermList TypeList=ATLtableGet(StateVars,(ATerm)StateVarName);
    if (!TypeList)
    {
      gsErrorMsg("undefined state variable %P (typechecking state formula %P)\n",StateVarName,StateFrm);
      return NULL;
    }

    ATermList Pars=ATLgetArgument(StateFrm,1);
    if (ATgetLength(TypeList)!=ATgetLength(Pars))
    {
      gsErrorMsg("incorrect number of parameters for state variable %P (typechecking state formula %P)\n",StateVarName,StateFrm);
      return NULL;
    }

    ATermList r=ATmakeList0();
    bool success=true;
    for (; !ATisEmpty(Pars); Pars=ATgetNext(Pars),TypeList=ATgetNext(TypeList))
    {
      ATermAppl Par=ATAgetFirst(Pars);
      ATermAppl ParType=ATAgetFirst(TypeList);
      ATermAppl NewParType=gstcTraverseVarConsTypeD(Vars,Vars,&Par,gstcExpandNumTypesDown(ParType));
      if (!NewParType)
      {
        gsErrorMsg("typechecking %P\n",StateFrm);
        success=false;
        break;
      }

      if (!gstcTypeMatchA(ParType,NewParType))
      {
        //upcasting
        NewParType=gstcUpCastNumericType(ParType,NewParType,&Par);
        if (!NewParType)
        {
          gsErrorMsg("cannot (up)cast %P to type %P (typechecking state formula %P)\n",Par,ParType,StateFrm);
          success=false;
          break;
        }
      }

      r=ATinsert(r,(ATerm)Par);
    }

    if (!success)
    {
      return NULL;
    }

    return ATsetArgument(StateFrm,(ATerm)ATreverse(r),1);

  }

  if (gsIsStateNu(StateFrm) || gsIsStateMu(StateFrm))
  {
    ATermTable CopyStateVars=ATtableCreate(63,50);
    gstcATermTableCopy(StateVars,CopyStateVars);

    // Make the new state variable:
    ATermTable FormPars=ATtableCreate(63,50);
    ATermList r=ATmakeList0();
    ATermList t=ATmakeList0();
    bool success=true;
    for (ATermList l=ATLgetArgument(StateFrm,1); !ATisEmpty(l); l=ATgetNext(l))
    {
      ATermAppl o=ATAgetFirst(l);

      ATermAppl VarName=ATAgetArgument(ATAgetArgument(o,0),0);
      if (ATAtableGet(FormPars,(ATerm)VarName))
      {
        gsErrorMsg("non-unique formal parameter %P (typechecking %P)\n",VarName,StateFrm);
        success=false;
        break;
      }

      ATermAppl VarType=ATAgetArgument(ATAgetArgument(o,0),1);
      if (!gstcIsSortExprDeclared(VarType))
      {
        gsErrorMsg("type error occurred while typechecking %P\n",StateFrm);
        success=false;
        break;
      }

      ATtablePut(FormPars,(ATerm)VarName, (ATerm)VarType);

      ATermAppl VarInit=ATAgetArgument(o,1);
      ATermAppl VarInitType=gstcTraverseVarConsTypeD(Vars,Vars,&VarInit,gstcExpandNumTypesDown(VarType));
      if (!VarInitType)
      {
        gsErrorMsg("typechecking %P\n",StateFrm);
        success=false;
        break;
      }

      if (!gstcTypeMatchA(VarType,VarInitType))
      {
        //upcasting
        VarInitType=gstcUpCastNumericType(VarType,VarInitType,&VarInit);
        if (!VarInitType)
        {
          gsErrorMsg("cannot (up)cast %P to type %P (typechecking state formula %P)\n",VarInit,VarType,StateFrm);
          success=false;
          break;
        }
      }

      r=ATinsert(r,(ATerm)ATsetArgument(o,(ATerm)VarInit,1));
      t=ATinsert(t,(ATerm)VarType);
    }

    if (!success)
    {
      ATtableDestroy(CopyStateVars);
      ATtableDestroy(FormPars);
      return NULL;
    }

    StateFrm=ATsetArgument(StateFrm,(ATerm)ATreverse(r),1);
    ATermTable CopyVars=ATtableCreate(63,50);
    gstcATermTableCopy(Vars,CopyVars);
    gstcATermTableCopy(FormPars,CopyVars);
    ATtableDestroy(FormPars);

    ATtablePut(CopyStateVars,(ATerm)ATAgetArgument(StateFrm,0),(ATerm)ATreverse(t));
    ATermAppl NewArg=gstcTraverseStateFrm(CopyVars,CopyStateVars,ATAgetArgument(StateFrm,2));
    ATtableDestroy(CopyStateVars);
    ATtableDestroy(CopyVars);
    if (!NewArg)
    {
      gsErrorMsg("while typechecking %P\n",StateFrm);
      return NULL;
    }
    return ATsetArgument(StateFrm,(ATerm)NewArg,2);
  }

  if (gsIsDataExpr(StateFrm))
  {
    ATermAppl Type=gstcTraverseVarConsTypeD(Vars, Vars, &StateFrm, sort_bool::bool_());
    if (!Type)
    {
      return NULL;
    }
    return StateFrm;
  }

  assert(0);
  return NULL;
}

static ATermAppl gstcTraverseRegFrm(ATermTable Vars, ATermAppl RegFrm)
{
  if (gsDebug)
  {
    std::cerr << "gstcTraverseRegFrm: " + pp(RegFrm) + "\n";
  }
  if (gsIsRegNil(RegFrm))
  {
    return RegFrm;
  }

  if (gsIsRegSeq(RegFrm) || gsIsRegAlt(RegFrm))
  {
    ATermAppl NewArg1=gstcTraverseRegFrm(Vars,ATAgetArgument(RegFrm,0));
    if (!NewArg1)
    {
      return NULL;
    }
    ATermAppl NewArg2=gstcTraverseRegFrm(Vars,ATAgetArgument(RegFrm,1));
    if (!NewArg2)
    {
      return NULL;
    }
    return ATsetArgument(ATsetArgument(RegFrm,(ATerm)NewArg1,0),(ATerm)NewArg2,1);
  }

  if (gsIsRegTrans(RegFrm) || gsIsRegTransOrNil(RegFrm))
  {
    ATermAppl NewArg=gstcTraverseRegFrm(Vars,ATAgetArgument(RegFrm,0));
    if (!NewArg)
    {
      return NULL;
    }
    return ATsetArgument(RegFrm,(ATerm)NewArg,0);
  }

  if (gsIsActFrm(RegFrm))
  {
    return gstcTraverseActFrm(Vars, RegFrm);
  }

  assert(0);
  return NULL;
}

static ATermAppl gstcTraverseActFrm(ATermTable Vars, ATermAppl ActFrm)
{
  if (gsDebug)
  {
    std::cerr << "gstcTraverseActFrm: " + pp(ActFrm) + "\n";
  }


  if (gsIsActTrue(ActFrm) || gsIsActFalse(ActFrm))
  {
    return ActFrm;
  }

  if (gsIsActNot(ActFrm))
  {
    ATermAppl NewArg=gstcTraverseActFrm(Vars,ATAgetArgument(ActFrm,0));
    if (!NewArg)
    {
      return NULL;
    }
    return ATsetArgument(ActFrm,(ATerm)NewArg,0);
  }

  if (gsIsActAnd(ActFrm) || gsIsActOr(ActFrm) || gsIsActImp(ActFrm))
  {
    ATermAppl NewArg1=gstcTraverseActFrm(Vars,ATAgetArgument(ActFrm,0));
    if (!NewArg1)
    {
      return NULL;
    }
    ATermAppl NewArg2=gstcTraverseActFrm(Vars,ATAgetArgument(ActFrm,1));
    if (!NewArg2)
    {
      return NULL;
    }
    return ATsetArgument(ATsetArgument(ActFrm,(ATerm)NewArg1,0),(ATerm)NewArg2,1);
  }

  if (gsIsActForall(ActFrm) || gsIsActExists(ActFrm))
  {
    ATermTable CopyVars=ATtableCreate(63,50);
    gstcATermTableCopy(Vars,CopyVars);

    ATermList VarList=ATLgetArgument(ActFrm,0);
    ATermTable NewVars=gstcAddVars2Table(CopyVars,VarList);
    if (!NewVars)
    {
      ATtableDestroy(CopyVars);
      return NULL;
    }

    ATermAppl NewArg2=gstcTraverseActFrm(NewVars,ATAgetArgument(ActFrm,1));
    if (!NewArg2)
    {
      return NULL;
    }
    ATtableDestroy(CopyVars);

    return ATsetArgument(ActFrm,(ATerm)NewArg2,1);
  }

  if (gsIsActAt(ActFrm))
  {
    ATermAppl NewArg1=gstcTraverseActFrm(Vars,ATAgetArgument(ActFrm,0));
    if (!NewArg1)
    {
      return NULL;
    }

    ATermAppl Time=ATAgetArgument(ActFrm,1);
    ATermAppl NewType=gstcTraverseVarConsTypeD(Vars,Vars,&Time,gstcExpandNumTypesDown(sort_real::real_()));
    if (!NewType)
    {
      return NULL;
    }

    if (!gstcTypeMatchA(sort_real::real_(),NewType))
    {
      //upcasting
      ATermAppl CastedNewType=gstcUpCastNumericType(sort_real::real_(),NewType,&Time);
      if (!CastedNewType)
      {
        gsErrorMsg("cannot (up)cast time value %P to type Real (typechecking action formula %P)\n",Time,ActFrm);
        return NULL;
      }
    }
    return ATsetArgument(ATsetArgument(ActFrm,(ATerm)NewArg1,0),(ATerm)Time,1);
  }

  if (gsIsMultAct(ActFrm))
  {
    ATermList r=ATmakeList0();
    for (ATermList l=ATLgetArgument(ActFrm,0); !ATisEmpty(l); l=ATgetNext(l))
    {
      ATermAppl o=ATAgetFirst(l);
      assert(gsIsParamId(o));
      o=gstcTraverseActProcVarConstP(Vars,o);
      if (!o)
      {
        return NULL;
      }
      r=ATinsert(r,(ATerm)o);
    }
    return ATsetArgument(ActFrm,(ATerm)ATreverse(r),0);
  }

  if (gsIsDataExpr(ActFrm))
  {
    ATermAppl Type=gstcTraverseVarConsTypeD(Vars, Vars, &ActFrm, sort_bool::bool_());
    if (!Type)
    {
      return NULL;
    }
    return ActFrm;
  }

  assert(0);
  return NULL;
}

}
}
