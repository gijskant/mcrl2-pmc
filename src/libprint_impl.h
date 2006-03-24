//This file contains a generic implementation of the PrintPart functions for
//both C and C++. It should NEVER be included directly. Use libprint_c.h and
//libprint.h instead.

#if defined(PRINT_C)
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define PRINT_OUTTYPE FILE*
#define PRINT_FUNC(x) x ## _C
#endif

#if defined(PRINT_CXX)
#include <ostream>
#include <iostream>
#define PRINT_OUTTYPE std::ostream&
#define PRINT_FUNC(x) x ## _CXX
#endif

#include <assert.h>
#include <aterm2.h>
#include "libprint_types.h"
#include "librewrite_c.h"
#include "libstruct.h"
#include "liblowlevel.h"

//declarations
//------------

static void PRINT_FUNC(PrintPart_)(PRINT_OUTTYPE OutStream, const ATerm Part,
  t_pp_format pp_format);
/*Pre: OutStream points to a stream to which can be written
       Part is an ATerm containing a part of a mCRL2 specification
  Post:A textual representation of Part is written to OutStream, using method
       pp_format
*/

static void PRINT_FUNC(PrintPart_Appl)(PRINT_OUTTYPE OutStream,
  const ATermAppl Part, t_pp_format pp_format, bool ShowSorts, int PrecLevel);
/*Pre: OutStream points to a stream to which can be written
       Part is an ATermAppl containing a part of a mCRL2 specification
       pp_format != ppInternal
       ShowSorts indicates if sorts should be shown for the part
       PrecLevel indicates the precedence level of the context of the part
       0 <= PrecLevel
  Post:A textual representation of Part is written to OutStream, using method
       pp_format. In this textual representation:
       - the top level symbol is parenthesized if PrecLevel is greater than its
         precedence level
       - sorts are shown for all elements, if ShowSorts is true;
         otherwise, sorts are only shown when necessary
*/

static void PRINT_FUNC(PrintPart_List)(PRINT_OUTTYPE OutStream,
  const ATermList Parts, t_pp_format pp_format, bool ShowSorts, int PrecLevel,
  const char *Terminator, const char *Separator);
/*Pre: OutStream points to a stream to which can be written
       Parts is an ATermList containing parts of a mCRL2 specification
       pp_format != ppInternal
       ShowSorts indicates if sorts should be shown for the part
       PrecLevel indicates the precedence level of the context of the parts
       0 <= PrecLevel
  Post:A textual representation of Part is written to OutStream using method
       pp_format. In this textual representation:
       - PrecLevel and ShowSort are distributed over the parts
       - each part is terminated by Terminator, if it is not NULL
       - two successive parts are separated by Separator, if it is not NULL
*/

static void PRINT_FUNC(PrintEqns)(PRINT_OUTTYPE OutStream,
  const ATermList DataEqns, t_pp_format, bool ShowSorts, int PrecLevel);
/*Pre: OutStream points to a stream to which can be written
       Eqns is an ATermList containing equations from an mCRL2 specification
       pp_format != ppInternal
       ShowSorts indicates if sorts should be shown for each equation
       PrecLevel indicates the precedence level of the context of the equations
       0 <= PrecLevel
  Post:A textual representation of the equations is written to OutStream using
       method pp_format, in which:
       - equations are grouped in equation sections, i.e. variable declarations
         apply to groups of equations
       - PrecLevel and ShowSort are distributed over the equations
*/

static void PRINT_FUNC(PrintDecls)(PRINT_OUTTYPE OutStream,
  const ATermList Decls, t_pp_format pp_format,
  const char *Terminator, const char *Separator);
/*Pre: Decls is an ATermList containing action, operation, or variable
       declarations from a mCRL2 specification
       pp_format != ppInternal
  Ret: A textual representation of the declarations is written to OutStream,
       in which, if pp_format == ppDefault:
       - of two consecutive declarations Decl(x, S) and Decl(y, T), the first
         is printed as:
         + "x,", if S = T
         + "x: S", followed by Terminator and Separator, if S != T
       - the last declaration Decl(x, S) is printed as "x: S", followed by
         Terminator
*/

static void PRINT_FUNC(PrintDecl)(PRINT_OUTTYPE OutStream,
  const ATermAppl Decl, t_pp_format pp_format, const bool ShowSorts);
/*Pre: Decl is an ATermAppl that represents an action, operation, or variable
       declaration from a mCRL2 specification
       pp_format != ppInternal
       ShowSorts indicates if the sort of the declaration should be shown
  Ret: A textual representation of the declaration, say Decl(x, S), is written
       to OutStream, i.e.:
       - "x: S", if ShowSorts
       - "x", otherwise
*/

static void PRINT_FUNC(PrintListEnumElts)(PRINT_OUTTYPE OutStream,
  const ATermAppl DataExpr, t_pp_format pp_format, bool ShowSorts);
/*Pre: OutStream points to a stream to which can be written
       DataExpr is the implementation of a list enumeration
       pp_format != ppInternal
       ShowSorts indicates if the sorts of DataExpr should be shown
  Post:A textual representation of the expression is written to OutStream, in
       which:
       - ShowSorts is taken into account
       - the elements of the list are printed as a comma-separated list
*/

static void PRINT_FUNC(PrintPos)(PRINT_OUTTYPE OutStream,
  const ATermAppl PosExpr, int PrecLevel);
/*Pre: OutStream points to a stream to which can be written
       PosExpr is a data expression of sort Pos
       PrecLevel indicates the precedence level of the context of the part
       0 <= PrecLevel
  Ret: A textual representation of the expression is written to OutStream, in
       in which:
       - PrecLevel is taken into account
       - each constant is represented by its corresponding positive number
       - each non-constant is of the form cDub(b_1)(...(cDub(b_n)(p))) and is
         represented by 2^n*p + 2^(n-1)*b_n + ... + b1
*/

static void PRINT_FUNC(PrintPosMult)(PRINT_OUTTYPE OutStream,
  const ATermAppl PosExpr, int PrecLevel, char *Mult);
/*Pre: OutStream points to a stream to which can be written
       PosExpr is a data expression of sort Pos
       PrecLevel indicates the precedence level of the context of the part
       0 <= PrecLevel
       Mult is the string representation of a natural number
  Ret: A textual representation of Mult * PosExpr is written to OutStream, i.e.
       if PosExpr is the form cDub(b_1)(...(cDub(b_n)(p))), then it is
       represented by (Mult*2^n)*p + (Mult*2^(n-1))*b_n + ... + Mult*b1
       Also PrecLevel is taken into account
*/

static ATermList gsGroupDeclsBySort(const ATermList Decls);
/*Pre: Decls is an ATermList containing declarations of the form
       Decl(Name, Sort) from a mCRL2 specification
  Ret: a list containing the declarations from Decls, where declarations of the
       same sort are placed in sequence
*/

static bool gsHasConsistentContext(const ATermTable DataVarDecls,
  const ATermAppl Part);
/*Pre: DataVarDecls represents the variables from an equation section, where
       the keys are the variable names and the values are the corresponding
       variables
       Part is an ATermAppl containing an equation of a mCRL2 specification,
       or the elements it consists of
  Ret: all operations occurring in Part are consistent with the variables from
       the context
 */

static bool gsHasConsistentContextList(const ATermTable DataVarDecls,
  const ATermList Parts);
/*Pre: DataVarDecls represents the variables from an equation section, where
       the keys are the variable names and the values are the
       corresponding variables
       Parts is an ATermList containing elements of an equation of a mCRL2
       specification
  Ret: all operations occurring in Parts are consistent with the variables from
       the context
 */

static bool gsIsListEnumImpl(ATermAppl DataExpr);
//Ret: DataExpr is the implementation of a list enumeration

static bool gsIsOpIdSetBagComp(ATermAppl Term);
//Ret: DataExpr is an operation identifier of a set/bag comprehension

static bool gsIsOpIdQuant(ATermAppl Term);
//Ret: DataExpr is an operation identifier of a universal/existential quantifier

static bool gsIsOpIdPrefix(ATermAppl Term);
//Ret: DataExpr is a prefix operation identifier

static bool gsIsOpIdInfix(ATermAppl Term);
//Ret: DataExpr is an infix operation identifier

static int gsPrecOpIdInfix(ATermAppl OpIdName);
//Pre: OpIdName is the name of an infix operation identifier
//Ret: Precedence of the operation itself

static int gsPrecOpIdInfixLeft(ATermAppl OpIdName);
//Pre: OpIdInfix is the name of an infix operation identifier
//Ret: Precedence of the left argument of the operation

static int gsPrecOpIdInfixRight(ATermAppl OpIdName);
//Pre: OpIdInfix is the name of an infix operation identifier
//Ret: Precedence of the right argument of the operation

//implementation
//--------------

inline static void PRINT_FUNC(fprints)(PRINT_OUTTYPE OutStream,
  const char *Value)
//Pre:  OutStream and Value are not NULL
//Post: Value is written to OutStream in C/C++ style
{
#if (defined(PRINT_C) == defined(PRINT_CXX))
  assert(false);
#endif
#if defined(PRINT_C)
  fprintf(OutStream, Value);
#elif defined(PRINT_CXX)
  OutStream << Value;
#endif
}

inline static void PRINT_FUNC(dbg_prints)(const char *Value)
{
//Pre:  Value is not NULL
//Post: Value is written to stderr in C/C++ style if gsDebug is true
#if (defined(PRINT_C) == defined(PRINT_CXX))
  assert(false);
#endif
#if defined(PRINT_C)
  if (gsDebug) fprintf(stderr, Value);
#elif defined(PRINT_CXX)
  if (gsDebug) std::cerr << Value;
#endif
}

//implementation

void PRINT_FUNC(PrintPart_)(PRINT_OUTTYPE OutStream, const ATerm Part,
  t_pp_format pp_format)
{
  if (pp_format == ppInternal) {
#if defined(PRINT_C)
    ATwriteToTextFile(Part, OutStream);
    fprintf(OutStream, "\n");
#elif defined(PRINT_CXX)
    OutStream << ATwriteToString(Part) << std::endl;
#endif
  } else {
    if (ATgetType(Part) == AT_APPL) {
      PRINT_FUNC(PrintPart_Appl)(OutStream, (ATermAppl) Part, pp_format,
        false, 0);
    } else if (ATgetType(Part) == AT_LIST) {
      PRINT_FUNC(fprints)(OutStream, "[");
      PRINT_FUNC(PrintPart_List)(OutStream, (ATermList) Part,
        pp_format, false, 0, "", ", ");
      PRINT_FUNC(fprints)(OutStream, "]");
    } else {
#if defined(PRINT_C)
      gsErrorMsg("ATerm Part is not an ATermAppl or an ATermList\n");
#elif defined(PRINT_CXX)
      std::cerr  << "error: ATerm Part is not an ATermAppl or an ATermList\n";
#endif
      assert(false);
    }
  }
}

void PRINT_FUNC(PrintPart_Appl)(PRINT_OUTTYPE OutStream,
  const ATermAppl Part, t_pp_format pp_format, bool ShowSorts, int PrecLevel)
{
  if (ATisQuoted(ATgetAFun(Part)) == ATtrue) {
    //print string
    PRINT_FUNC(fprints)(OutStream, ATgetName(ATgetAFun(Part)));
  } else if (gsIsSpecV1(Part)) {
    //print specification
    PRINT_FUNC(dbg_prints)("printing specification\n");
    for (int i = 0; i < 7; i++) {
      PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, i),
        pp_format, ShowSorts, PrecLevel);
    }
  } else if (gsIsSortSpec(Part)) {
    //print sort specification
    PRINT_FUNC(dbg_prints)("printing sort specification\n");
    ATermList SortDecls = ATLgetArgument(Part, 0);
    if (ATgetLength(SortDecls) > 0) {
      PRINT_FUNC(fprints)(OutStream, "sort ");
      PRINT_FUNC(PrintPart_List)(OutStream, SortDecls,
        pp_format, ShowSorts, PrecLevel, ";\n", "     ");
      PRINT_FUNC(fprints)(OutStream, "\n");
    }
  } else if (gsIsConsSpec(Part) || gsIsMapSpec(Part)) {
    //print operation specification
    PRINT_FUNC(dbg_prints)("printing operation specification\n");
    ATermList OpIds = ATLgetArgument(Part, 0);
    if (ATgetLength(OpIds) > 0) {
      PRINT_FUNC(fprints)(OutStream, gsIsConsSpec(Part)?"cons ":"map  ");
      PRINT_FUNC(PrintDecls)(OutStream, OpIds, pp_format, ";\n", "     ");
      PRINT_FUNC(fprints)(OutStream, "\n");
    }
  } else if (gsIsDataEqnSpec(Part)) {
    //print equation specification
    PRINT_FUNC(dbg_prints)("printing equation specification\n");
    PRINT_FUNC(PrintEqns)(OutStream, ATLgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
  } else if (gsIsActSpec(Part)) {
    //print action specification
    PRINT_FUNC(dbg_prints)("printing action specification\n");
    ATermList ActIds = ATLgetArgument(Part, 0);
    if (ATgetLength(ActIds) > 0) {
      PRINT_FUNC(fprints)(OutStream, "act  ");
      PRINT_FUNC(PrintDecls)(OutStream, ActIds, pp_format, ";\n", "     ");
      PRINT_FUNC(fprints)(OutStream, "\n");
    }
  } else if (gsIsProcEqnSpec(Part)) {
    //print process specification
    PRINT_FUNC(dbg_prints)("printing process specification\n");
    PRINT_FUNC(PrintEqns)(OutStream, ATLgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
  } else if (gsIsLPE(Part)) {
    //print LPE
    PRINT_FUNC(dbg_prints)("printing LPE\n");
    //print global variables
    ATermList Vars = ATLgetArgument(Part, 0);
    if (ATgetLength(Vars) > 0) {
      PRINT_FUNC(fprints)(OutStream, "var  ");
      PRINT_FUNC(PrintDecls)(OutStream, gsGroupDeclsBySort(Vars),
        pp_format, ";\n", "     ");
    }
    //print process name and variable declarations
    ATermList VarDecls = ATLgetArgument(Part, 1);
    int VarDeclsLength = ATgetLength(VarDecls);
    PRINT_FUNC(fprints)(OutStream, "proc P");
    if (VarDeclsLength > 0) {
      PRINT_FUNC(fprints)(OutStream, "(");
      PRINT_FUNC(PrintDecls)(OutStream, VarDecls, pp_format, NULL, ", ");
      PRINT_FUNC(fprints)(OutStream, ")");
    }
    PRINT_FUNC(fprints)(OutStream, " =");
    //print summations
    ATermList Summands = ATLgetArgument(Part, 2);
    int SummandsLength = ATgetLength(Summands);
    if (SummandsLength == 0) {
      PRINT_FUNC(fprints)(OutStream, " delta\n");
    } else {
      //SummandsLength > 0
      PRINT_FUNC(fprints)(OutStream, "\n       ");
      PRINT_FUNC(PrintPart_List)(OutStream, Summands, pp_format,
        ShowSorts, PrecLevel, NULL, "\n     + ");
      PRINT_FUNC(fprints)(OutStream, ";\n");
    }
    PRINT_FUNC(fprints)(OutStream, "\n");
  } else if (gsIsInit(Part)) {
    //print initialisation
    PRINT_FUNC(dbg_prints)("printing initialisation\n");
    ATermList Vars = ATLgetArgument(Part, 0);
    if (ATgetLength(Vars) > 0) {
      PRINT_FUNC(fprints)(OutStream, "var  ");
      PRINT_FUNC(PrintDecls)(OutStream, gsGroupDeclsBySort(Vars),
        pp_format, ";\n", "     ");
      PRINT_FUNC(fprints)(OutStream, "\n");
    }
    PRINT_FUNC(fprints)(OutStream, "init ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, PrecLevel);
    PRINT_FUNC(fprints)(OutStream, ";\n");
  } else if (gsIsLPEInit(Part)) {
    //print LPE initialisation
    PRINT_FUNC(dbg_prints)("printing LPE initialisation\n");
    ATermList Vars = ATLgetArgument(Part, 0);
    if (ATgetLength(Vars) > 0) {
      PRINT_FUNC(fprints)(OutStream, "var  ");
      PRINT_FUNC(PrintDecls)(OutStream, gsGroupDeclsBySort(Vars),
        pp_format, ";\n", "     ");
    }
    PRINT_FUNC(fprints)(OutStream, "init P");
    ATermList Args = ATLgetArgument(Part, 1);
    if (ATgetLength(Args) > 0) {
      PRINT_FUNC(fprints)(OutStream, "(");
      PRINT_FUNC(PrintPart_List)(OutStream, Args,
        pp_format, ShowSorts, 0, NULL, ", ");
      PRINT_FUNC(fprints)(OutStream, ")");
    }
    PRINT_FUNC(fprints)(OutStream, ";\n");
  } else if (gsIsSortId(Part)) {
    //print sort identifier
    PRINT_FUNC(dbg_prints)("printing standard sort identifier\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
  } else if (gsIsSortRef(Part)) {
    //print sort reference
    PRINT_FUNC(dbg_prints)("printing sort reference declaration\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
    PRINT_FUNC(fprints)(OutStream, " = ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, PrecLevel);
  } else if (gsIsDataEqn(Part)) {
    //print data equation (without variables)
    PRINT_FUNC(dbg_prints)("printing data equation\n");
    ATermAppl Condition = ATAgetArgument(Part, 1);
    if (!gsIsNil(Condition)) {
      PRINT_FUNC(PrintPart_Appl)(OutStream, Condition,
        pp_format, ShowSorts, 0);
      PRINT_FUNC(fprints)(OutStream, "  ->  ");
    }
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 2),
      pp_format, ShowSorts, 0);
    PRINT_FUNC(fprints)(OutStream, "  =  ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 3),
      pp_format, ShowSorts, 0);
  } else if (gsIsActId(Part)) {
    //print action identifier
    PRINT_FUNC(dbg_prints)("printing action identifier\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
    if (ShowSorts) {
      ATermList SortExprs = ATLgetArgument(Part, 1);
      if (ATgetLength(SortExprs) > 0) {
        PRINT_FUNC(fprints)(OutStream, ": ");
        PRINT_FUNC(PrintPart_List)(OutStream, SortExprs,
          pp_format, ShowSorts, 2, NULL, " # ");
      }
    }
  } else if (gsIsProcEqn(Part)) {
    //print process equation (without free variables)
    PRINT_FUNC(dbg_prints)("printing process equation\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, PrecLevel);
    ATermList DataVarIds = ATLgetArgument(Part, 2);
    if (ATgetLength(DataVarIds) > 0) {
      PRINT_FUNC(fprints)(OutStream, "(");
      PRINT_FUNC(PrintDecls)(OutStream, DataVarIds, pp_format, NULL, ", ");
      PRINT_FUNC(fprints)(OutStream, ")");
    }
    PRINT_FUNC(fprints)(OutStream, " = ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 3),
      pp_format, ShowSorts, 0);
  } else if (gsIsProcVarId(Part)) {
    //print process variable
    PRINT_FUNC(dbg_prints)("printing process variable\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
    if (ShowSorts) {
      ATermList SortExprs = ATLgetArgument(Part, 1);
      if (ATgetLength(SortExprs) > 0) {
        PRINT_FUNC(fprints)(OutStream, ": ");
        PRINT_FUNC(PrintPart_List)(OutStream, SortExprs,
          pp_format, ShowSorts, 2, NULL, " # ");
      }
    }
  } else if (gsIsLPESummand(Part)) {
    //print LPE summand
    PRINT_FUNC(dbg_prints)("printing LPE summand\n");
    //print data summations
    ATermList SumVarDecls = ATLgetArgument(Part, 0);
    if (ATgetLength(SumVarDecls) > 0) {
      PRINT_FUNC(fprints)(OutStream, "sum ");
      PRINT_FUNC(PrintDecls)(OutStream, SumVarDecls, pp_format, NULL, ",");
      PRINT_FUNC(fprints)(OutStream, ".\n         ");
    }
    //print condition
    ATermAppl Cond = ATAgetArgument(Part, 1);
    if (!gsIsNil(Cond)) {
      PRINT_FUNC(PrintPart_Appl)(OutStream, Cond, pp_format, ShowSorts, 12);
      PRINT_FUNC(fprints)(OutStream, " ->\n         ");
    }
    //print multiaction
    ATermAppl MultAct = ATAgetArgument(Part, 2);
    ATermAppl Time = ATAgetArgument(Part, 3);
    bool IsTimed = !gsIsNil(Time);
    PRINT_FUNC(PrintPart_Appl)(OutStream, MultAct, pp_format, ShowSorts,
      (IsTimed)?6:5);
    //print time
    if (IsTimed) {
      PRINT_FUNC(fprints)(OutStream, " @ ");
      PRINT_FUNC(PrintPart_Appl)(OutStream, Time, pp_format, ShowSorts, 12);
    }
    //print process reference
    if (!gsIsDelta(MultAct)) {
      PRINT_FUNC(fprints)(OutStream, " .\n         ");
      ATermList Assignments = ATLgetArgument(Part, 4);
      int AssignmentsLength = ATgetLength(Assignments);
      PRINT_FUNC(fprints)(OutStream, "P");
      if (AssignmentsLength > 0) {
        PRINT_FUNC(fprints)(OutStream, "(");
        PRINT_FUNC(PrintPart_List)(OutStream, Assignments,
          pp_format, ShowSorts, PrecLevel, NULL, ", ");
        PRINT_FUNC(fprints)(OutStream, ")");
      }
    }
  } else if (gsIsMultAct(Part)) {
    //print multiaction
    PRINT_FUNC(dbg_prints)("printing multiaction\n");
    ATermList Actions = ATLgetArgument(Part, 0);
    int ActionsLength = ATgetLength(Actions);
    if (ActionsLength == 0) {
      PRINT_FUNC(fprints)(OutStream, "tau");
    } else {
      //ActionsLength > 0
      if (PrecLevel > 7) PRINT_FUNC(fprints)(OutStream, "(");
      PRINT_FUNC(PrintPart_List)(OutStream, Actions,
        pp_format, ShowSorts, PrecLevel, NULL, "|");
      if (PrecLevel > 7) PRINT_FUNC(fprints)(OutStream, ")");
    }
  } else if (gsIsAssignment(Part)) {
    //print assignment
    PRINT_FUNC(dbg_prints)("printing assignment\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
    PRINT_FUNC(fprints)(OutStream, " := ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 0);
  } else if (gsIsSortList(Part)) {
    //print list sort
    PRINT_FUNC(dbg_prints)("printing list sort\n");
    PRINT_FUNC(fprints)(OutStream, "List(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 0);
    PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsSortSet(Part)) {
    //print set sort
    PRINT_FUNC(dbg_prints)("printing set sort\n");
    PRINT_FUNC(fprints)(OutStream, "Set(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 0);
    PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsSortBag(Part)) {
    //print bag sort
    PRINT_FUNC(dbg_prints)("printing bag sort\n");
    PRINT_FUNC(fprints)(OutStream, "Bag(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 0);
    PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsSortStruct(Part)) {
    //print structured sort
    PRINT_FUNC(dbg_prints)("printing structured sort\n");
    if (PrecLevel > 1) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(fprints)(OutStream, "struct ");
    PRINT_FUNC(PrintPart_List)(OutStream, ATLgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel, NULL, " | ");
    if (PrecLevel > 1) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsSortArrowProd(Part)) {
    //print product arrow sort
    PRINT_FUNC(dbg_prints)("printing product arrow sort\n");
    if (PrecLevel > 0) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(PrintPart_List)(OutStream, ATLgetArgument(Part, 0),
      pp_format, ShowSorts, 1, NULL, " # ");
    PRINT_FUNC(fprints)(OutStream, " -> ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 0);
    if (PrecLevel > 0) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsSortArrow(Part)) {
    //print arrow sort
    PRINT_FUNC(dbg_prints)("printing arrow sort\n");
    if (PrecLevel > 0) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 1);
    PRINT_FUNC(fprints)(OutStream, " -> ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 0);
    if (PrecLevel > 0) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsStructCons(Part)) {
    //print structured sort constructor
    PRINT_FUNC(dbg_prints)("printing structured sort constructor\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
    ATermList StructProjs = ATLgetArgument(Part, 1);
    if (ATgetLength(StructProjs) > 0) {
      PRINT_FUNC(fprints)(OutStream, "(");
      PRINT_FUNC(PrintPart_List)(OutStream, StructProjs,
        pp_format, ShowSorts, PrecLevel, NULL, ", ");
      PRINT_FUNC(fprints)(OutStream, ")");
    }
    ATermAppl Recogniser = ATAgetArgument(Part, 2);
    if (!gsIsNil(Recogniser)) {
      PRINT_FUNC(fprints)(OutStream, "?");
      PRINT_FUNC(PrintPart_Appl)(OutStream, Recogniser,
        pp_format, ShowSorts, PrecLevel);
    }
  } else if (gsIsStructProj(Part)) {
    //print structured sort projection
    PRINT_FUNC(dbg_prints)("printing structured sort projection\n");
    ATermAppl Projection = ATAgetArgument(Part, 0);
    if (!gsIsNil(Projection)) {
      PRINT_FUNC(PrintPart_Appl)(OutStream, Projection,
        pp_format, ShowSorts, PrecLevel);
      PRINT_FUNC(fprints)(OutStream, ": ");
    }
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 0);
  } else if (gsIsDataVarIdOpId(Part) || gsIsOpId(Part) || gsIsDataVarId(Part) ||
      gsIsDataAppl(Part) || gsIsDataApplProd(Part)) {
    //print data expression
    PRINT_FUNC(dbg_prints)("printing data expression\n");
    if (pp_format == ppDebug) {
      PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
        pp_format, ShowSorts, 0);
      if (gsIsDataAppl(Part) || gsIsDataApplProd(Part)) {
        PRINT_FUNC(fprints)(OutStream, "(");
        if (gsIsDataAppl(Part)) {
          PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
            pp_format, ShowSorts, 0);
        } else { //gsIsDataApplProd(Part)
          PRINT_FUNC(PrintPart_List)(OutStream, ATLgetArgument(Part, 1),
            pp_format, ShowSorts, 0, NULL, ", ");
        }
        PRINT_FUNC(fprints)(OutStream, ")");
      }
    } else { //pp_format == ppDefault
      //print data expression in the external format, if possible
      ATermAppl Head;
      ATermList Args;
      if (!gsIsDataApplProd(Part)) {
        Head = gsGetDataExprHead(Part);
        Args = gsGetDataExprArgs(Part);
      } else {
        Head = ATAgetArgument(Part, 0);
        Args = ATLgetArgument(Part, 1);
      }
      int ArgsLength = ATgetLength(Args);
      if (gsIsListEnumImpl(Part)) {
        //list enumeration
        PRINT_FUNC(fprints)(OutStream, "[");
        PRINT_FUNC(PrintListEnumElts)(OutStream, Part, pp_format, ShowSorts);
        PRINT_FUNC(fprints)(OutStream, "]");
      } else if (gsIsOpIdSetBagComp(Head) && ArgsLength == 1) {
        //set/bag comprehension
        PRINT_FUNC(fprints)(OutStream, "{ ");
        ATermAppl Body = ATAelementAt(Args, 0);
        ATermAppl Var =
          gsMakeDataVarId(gsFreshString2ATermAppl("x", (ATerm) Body, true),
            ATAgetArgument(gsGetSort(Body), 0)
          );
        PRINT_FUNC(PrintDecl)(OutStream, Var, pp_format, true);
        PRINT_FUNC(fprints)(OutStream, " | ");        
        Body = gsMakeDataAppl(Body, Var);
        if (gsRewriteIsInitialised()) {
          Body = gsRewriteTerm(Body);
        }
        PRINT_FUNC(PrintPart_Appl)(OutStream, Body, pp_format, ShowSorts, 0);
        PRINT_FUNC(fprints)(OutStream, " }");
      } else if (gsIsOpIdQuant(Head) && ArgsLength == 1) {
        //quantification
        if (PrecLevel > 12) PRINT_FUNC(fprints)(OutStream, "(");
        PRINT_FUNC(PrintPart_Appl)(OutStream, Head,
          pp_format, ShowSorts, PrecLevel);
        PRINT_FUNC(fprints)(OutStream, " ");
        ATermAppl Body = ATAelementAt(Args, 0);
        ATermAppl Var =
          gsMakeDataVarId(gsFreshString2ATermAppl("x", (ATerm) Body, true),
            ATAgetArgument(gsGetSort(Body), 0)
          );
        PRINT_FUNC(PrintDecl)(OutStream, Var, pp_format, true);
        PRINT_FUNC(fprints)(OutStream, ". ");        
        Body = gsMakeDataAppl(Body, Var);
        if (gsRewriteIsInitialised()) {
          Body = gsRewriteTerm(Body);
        }
        PRINT_FUNC(PrintPart_Appl)(OutStream, Body, pp_format, ShowSorts, 12);
        if (PrecLevel > 12) PRINT_FUNC(fprints)(OutStream, ")");
      } else if (gsIsOpIdPrefix(Head) && ArgsLength == 1) {
        //print prefix expression
        PRINT_FUNC(dbg_prints)("printing prefix expression\n");
        if (PrecLevel > 12) PRINT_FUNC(fprints)(OutStream, "(");
        PRINT_FUNC(PrintPart_Appl)(OutStream, Head,
          pp_format, ShowSorts, PrecLevel);
        PRINT_FUNC(PrintPart_Appl)(OutStream, ATAelementAt(Args, 0),
          pp_format, ShowSorts, 12);
        if (PrecLevel > 12) PRINT_FUNC(fprints)(OutStream, ")");
      } else if (gsIsOpIdInfix(Head) && ArgsLength == 2) {
        //print infix expression
        PRINT_FUNC(dbg_prints)("printing infix expression\n");
        ATermAppl HeadName = ATAgetArgument(Head, 0);
        if (PrecLevel > gsPrecOpIdInfix(HeadName))
          PRINT_FUNC(fprints)(OutStream, "(");
        PRINT_FUNC(PrintPart_Appl)(OutStream, ATAelementAt(Args, 0),
          pp_format, ShowSorts, gsPrecOpIdInfixLeft(HeadName));
        PRINT_FUNC(fprints)(OutStream, " ");
        PRINT_FUNC(PrintPart_Appl)(OutStream, Head,
          pp_format, ShowSorts, PrecLevel);
        PRINT_FUNC(fprints)(OutStream, " ");
        PRINT_FUNC(PrintPart_Appl)(OutStream, ATAelementAt(Args, 1),
          pp_format, ShowSorts, gsPrecOpIdInfixRight(HeadName));
        if (PrecLevel > gsPrecOpIdInfix(HeadName))
          PRINT_FUNC(fprints)(OutStream, ")");
     } else if (ATisEqual(Head, gsMakeOpId1()) ||
          (ATisEqual(Head, gsMakeOpIdCDub()) && ArgsLength == 2)) {
        //print positive number
        PRINT_FUNC(dbg_prints)("printing positive number\n");
        PRINT_FUNC(PrintPos)(OutStream, Part, PrecLevel);
      } else if (ATisEqual(Head, gsMakeOpId0())) {
        //print 0
        PRINT_FUNC(fprints)(OutStream, "0");
      } else if ((ATisEqual(Head, gsMakeOpIdCNat()) ||
          ATisEqual(Head, gsMakeOpIdCInt()) ||
          ATisEqual(Head, gsMakeOpIdCReal())) && ArgsLength == 1) {
        //print argument (ArgsLength == 1)
        PRINT_FUNC(PrintPart_Appl)(OutStream, ATAelementAt(Args, 0),
          pp_format, ShowSorts, PrecLevel);
      } else if (ATisEqual(Head, gsMakeOpIdCNeg()) && ArgsLength == 1) {
        //print negation (ArgsLength == 1)
        PRINT_FUNC(dbg_prints)("printing negation\n");
        PRINT_FUNC(fprints)(OutStream, "-");
        PRINT_FUNC(PrintPart_Appl)(OutStream, ATAelementAt(Args, 0),
          pp_format, ShowSorts, 12);
      } else if (gsIsDataVarIdOpId(Part)) {
        //print untyped data variable or operation identifier
        PRINT_FUNC(dbg_prints)("printing untyped data variable or operation identifier\n");
        PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
          pp_format, ShowSorts, PrecLevel);
      } else if (gsIsOpId(Part) || gsIsDataVarId(Part)) {
        //print data variable or operation identifier
        PRINT_FUNC(dbg_prints)("printing data variable or operation identifier\n");
        PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
          pp_format, ShowSorts, PrecLevel);
        if (ShowSorts) {
          PRINT_FUNC(fprints)(OutStream, ": ");
          PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
            pp_format, ShowSorts, 0);
        }
      } else {
        //print data application
        PRINT_FUNC(dbg_prints)("printing data application\n");
        if (PrecLevel > 13) PRINT_FUNC(fprints)(OutStream, "(");
        PRINT_FUNC(PrintPart_Appl)(OutStream, Head,
          pp_format, ShowSorts, 13);
        PRINT_FUNC(fprints)(OutStream, "(");
        PRINT_FUNC(PrintPart_List)(OutStream, Args,
          pp_format, ShowSorts, 0, NULL, ", ");
        PRINT_FUNC(fprints)(OutStream, ")");
        if (PrecLevel > 13) PRINT_FUNC(fprints)(OutStream, ")");
      }
    }
  } else if (gsIsNumber(Part)) {
    //print number
    PRINT_FUNC(dbg_prints)("printing number\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
  } else if (gsIsListEnum(Part)) {
    //print list enumeration
    PRINT_FUNC(dbg_prints)("printing list enumeration\n");
    PRINT_FUNC(fprints)(OutStream, "[");
    PRINT_FUNC(PrintPart_List)(OutStream, ATLgetArgument(Part, 0),
      pp_format, ShowSorts, 0, NULL, ", ");
    PRINT_FUNC(fprints)(OutStream, "]");
  } else if (gsIsSetEnum(Part) || gsIsBagEnum(Part)) {
    //print set/bag enumeration
    PRINT_FUNC(dbg_prints)("printing set/bag enumeration\n");
    PRINT_FUNC(fprints)(OutStream, "{");
    PRINT_FUNC(PrintPart_List)(OutStream, ATLgetArgument(Part, 0),
      pp_format, ShowSorts, 0, NULL, ", ");
    PRINT_FUNC(fprints)(OutStream, "}");
  } else if (gsIsSetBagComp(Part)) {
    //print set/bag comprehension
    PRINT_FUNC(dbg_prints)("printing set/bag comprehension\n");
    PRINT_FUNC(fprints)(OutStream, "{ ");
    PRINT_FUNC(PrintDecl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, true);
    PRINT_FUNC(fprints)(OutStream, " | ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 0);
    PRINT_FUNC(fprints)(OutStream, " }");
  } else if (gsIsForall(Part) || gsIsExists(Part)) {
    //print universal/existential quantification
    PRINT_FUNC(dbg_prints)("printing universal/existential quantification\n");
    if (PrecLevel > 12) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(fprints)(OutStream, (gsIsForall(Part))?"forall ":"exists ");
    PRINT_FUNC(PrintDecls)(OutStream, ATLgetArgument(Part, 0),
      pp_format, NULL, ", ");
    PRINT_FUNC(fprints)(OutStream, ". ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 12);
    if (PrecLevel > 12) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsLambda(Part)) {
    //print lambda abstraction
    PRINT_FUNC(dbg_prints)("printing lambda abstraction\n");
    if (PrecLevel > 1) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(fprints)(OutStream, "lambda ");
    PRINT_FUNC(PrintDecls)(OutStream, ATLgetArgument(Part, 0),
      pp_format, NULL, ", ");
    PRINT_FUNC(fprints)(OutStream, ". ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 1);
    if (PrecLevel > 1) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsWhr(Part)) {
    //print where clause
    PRINT_FUNC(dbg_prints)("printing where clause\n");
    if (PrecLevel > 0) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 0);
    PRINT_FUNC(fprints)(OutStream, " whr ");
    PRINT_FUNC(PrintPart_List)(OutStream, ATLgetArgument(Part, 1),
      pp_format, ShowSorts, PrecLevel, NULL, ", ");
    PRINT_FUNC(fprints)(OutStream, " end");
    if (PrecLevel > 0) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsBagEnumElt(Part)) {
    //print bag enumeration element
    PRINT_FUNC(dbg_prints)("printing bag enumeration element\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 0);
    PRINT_FUNC(fprints)(OutStream, ": ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 0);
  } else if (gsIsWhrDecl(Part)) {
    //print where declaration element
    PRINT_FUNC(dbg_prints)("printing where declaration\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
    PRINT_FUNC(fprints)(OutStream, " = ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 0);
  } else if (gsIsActionProcess(Part) || gsIsAction(Part) || gsIsProcess(Part)) {
    //print action or process reference
    PRINT_FUNC(dbg_prints)("printing action or process reference\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
    ATermList Args = ATLgetArgument(Part, 1);
    if (ATgetLength(Args) > 0) {
      PRINT_FUNC(fprints)(OutStream, "(");
      PRINT_FUNC(PrintPart_List)(OutStream, Args,
        pp_format, ShowSorts, 0, NULL, ", ");
      PRINT_FUNC(fprints)(OutStream, ")");
    }
  } else if (gsIsDelta(Part)) {
    //print delta
    PRINT_FUNC(dbg_prints)("printing delta\n");
    PRINT_FUNC(fprints)(OutStream, "delta");
  } else if (gsIsTau(Part)) {
    //print tau
    PRINT_FUNC(dbg_prints)("printing tau\n");
    PRINT_FUNC(fprints)(OutStream, "tau");
  } else if (gsIsChoice(Part)) {
    //print choice
    PRINT_FUNC(dbg_prints)("printing choice\n");
    if (PrecLevel > 0) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 1);
    PRINT_FUNC(fprints)(OutStream, " + ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 0);
    if (PrecLevel > 0) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsSum(Part)) {
    //print summation
    PRINT_FUNC(dbg_prints)("printing summation\n");
    if (PrecLevel > 1) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(fprints)(OutStream, "sum ");
    PRINT_FUNC(PrintDecls)(OutStream, ATLgetArgument(Part, 0),
      pp_format, NULL, ", ");
    PRINT_FUNC(fprints)(OutStream, ". ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 1);
    if (PrecLevel > 1) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsMerge(Part) || gsIsLMerge(Part)) {
    //print merge of left merge
    PRINT_FUNC(dbg_prints)("printing merge or left merge\n");
    if (PrecLevel > 2) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 3);
    if (gsIsMerge(Part)) {
      PRINT_FUNC(fprints)(OutStream, " || ");
    } else {
      PRINT_FUNC(fprints)(OutStream, " ||_ ");
    }
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 2);
    if (PrecLevel > 2) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsBInit(Part)) {
    //print bounded initialisation
    PRINT_FUNC(dbg_prints)("printing bounded initialisation\n");
    if (PrecLevel > 3) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 4);
    PRINT_FUNC(fprints)(OutStream, " << ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 3);
    if (PrecLevel > 3) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsCond(Part)) {
    //print conditional
    PRINT_FUNC(dbg_prints)("printing conditional\n");
    if (PrecLevel > 4) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 12);
    PRINT_FUNC(fprints)(OutStream, " -> ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 5);
    ATermAppl PartElse = ATAgetArgument(Part, 2);
    if (!gsIsDelta(PartElse)) {
      PRINT_FUNC(fprints)(OutStream, " <> ");
      PRINT_FUNC(PrintPart_Appl)(OutStream, PartElse,
        pp_format, ShowSorts, 5);
    }
    if (PrecLevel > 4) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsSeq(Part)) {
    //print sequential composition
    PRINT_FUNC(dbg_prints)("printing sequential composition\n");
    if (PrecLevel > 5) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 6);
    PRINT_FUNC(fprints)(OutStream, " . ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 5);
    if (PrecLevel > 5) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsAtTime(Part)) {
    //print at expression
    PRINT_FUNC(dbg_prints)("printing at expression\n");
    if (PrecLevel > 6) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 6);
    PRINT_FUNC(fprints)(OutStream, " @ ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 12);
    if (PrecLevel > 6) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsSync(Part)) {
    //print sync
    PRINT_FUNC(dbg_prints)("printing sync\n");
    if (PrecLevel > 7) PRINT_FUNC(fprints)(OutStream, "(");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, 8);
    PRINT_FUNC(fprints)(OutStream, " | ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 7);
    if (PrecLevel > 7) PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsBlock(Part) || gsIsHide(Part) || gsIsRename(Part) ||
      gsIsComm(Part) || gsIsAllow(Part)) {
    //print process quantification
    PRINT_FUNC(dbg_prints)("printing process quantification\n");
    if (gsIsBlock(Part)) {
      PRINT_FUNC(fprints)(OutStream, "block");
    } else if (gsIsHide(Part)) {
      PRINT_FUNC(fprints)(OutStream, "hide");
    } else if (gsIsRename(Part)) {
      PRINT_FUNC(fprints)(OutStream, "rename");
    } else if (gsIsComm(Part)) {
      PRINT_FUNC(fprints)(OutStream, "comm");
    } else {
      PRINT_FUNC(fprints)(OutStream, "allow");
    }
    PRINT_FUNC(fprints)(OutStream, "({");
    PRINT_FUNC(PrintPart_List)(OutStream, ATLgetArgument(Part, 0),
      pp_format, ShowSorts, 0, NULL, ", ");
    PRINT_FUNC(fprints)(OutStream, "}, ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, 0);
    PRINT_FUNC(fprints)(OutStream, ")");
  } else if (gsIsMultActName(Part)) {
    //print multi action name
    PRINT_FUNC(dbg_prints)("printing multi action name\n");
    PRINT_FUNC(PrintPart_List)(OutStream, ATLgetArgument(Part, 0),
      pp_format, ShowSorts, 0, NULL, " | ");
  } else if (gsIsRenameExpr(Part)) {
    //print renaming expression
    PRINT_FUNC(dbg_prints)("printing renaming expression\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
    PRINT_FUNC(fprints)(OutStream, " -> ");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 1),
      pp_format, ShowSorts, PrecLevel);
  } else if (gsIsCommExpr(Part)) {
    //print communication expression
    PRINT_FUNC(dbg_prints)("printing communication expression\n");
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Part, 0),
      pp_format, ShowSorts, PrecLevel);
    ATermAppl CommResult = ATAgetArgument(Part, 1);
    if (!gsIsNil(CommResult)) {
      PRINT_FUNC(fprints)(OutStream, " -> ");
      PRINT_FUNC(PrintPart_Appl)(OutStream, CommResult,
        pp_format, ShowSorts, PrecLevel);
    }
  } else if (gsIsNil(Part)) {
    //print nil
    PRINT_FUNC(dbg_prints)("printing nil\n");
    PRINT_FUNC(fprints)(OutStream, "nil");
  } else if (gsIsUnknown(Part)) {
    //print unknown
    PRINT_FUNC(dbg_prints)("printing unknown\n");
    PRINT_FUNC(fprints)(OutStream, "unknown");
  } else {
#if defined(PRINT_C)
    gsErrorMsg("the term %T is not part of the internal format\n", Part);
#elif defined(PRINT_CXX)
    std::cerr
      << "error: the term "
      << ATwriteToString((ATerm) Part)
      << " is not part of the internal format\n";
#endif
    assert(false);
  }
}

void PRINT_FUNC(PrintPart_List)(PRINT_OUTTYPE OutStream,
  const ATermList Parts, t_pp_format pp_format, bool ShowSorts, int PrecLevel,
  const char *Terminator, const char *Separator)
{
  ATermList l = Parts;
  while (!ATisEmpty(l)) {
    if (!ATisEqual(l, Parts) && Separator != NULL) {
      PRINT_FUNC(fprints)(OutStream, Separator);
    }
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetFirst(l),
      pp_format, ShowSorts, PrecLevel);
    if (Terminator != NULL) {
      PRINT_FUNC(fprints)(OutStream, Terminator);
    }
    l = ATgetNext(l);
  }
}

void PRINT_FUNC(PrintEqns)(PRINT_OUTTYPE OutStream, const ATermList Eqns,
  t_pp_format pp_format, bool ShowSorts, int PrecLevel)
{
  if (pp_format == ppDebug) {
    ATermList l = Eqns;
    while (!ATisEmpty(l)) {
      ATermAppl Eqn = ATAgetFirst(l);
      ATermList DataDecls = ATLgetArgument(Eqn, 0);
      if (!ATisEmpty(DataDecls)) {
        PRINT_FUNC(fprints)(OutStream, "var  ");
        PRINT_FUNC(PrintDecls)(OutStream, gsGroupDeclsBySort(DataDecls),
          pp_format, ";\n", "     ");
      }
      if (gsIsDataEqn(Eqn)) {
        PRINT_FUNC(fprints)(OutStream, "eqn  ");
      } else { //gsIsProcEqn(Eqn)
        PRINT_FUNC(fprints)(OutStream, "proc ");
      }
      PRINT_FUNC(PrintPart_Appl)(OutStream, Eqn,
        pp_format, ShowSorts, PrecLevel);
      PRINT_FUNC(fprints)(OutStream, ";\n\n");
      l = ATgetNext(l);
    }
  } else { //pp_format == ppDefault
    int EqnsLength = ATgetLength(Eqns);
    if (EqnsLength > 0) {
      int StartPrefix = 0;
      ATermTable VarDeclTable = ATtableCreate(63, 50);
      //VarDeclTable is a hash table with variable declarations as values, where
      //the name of each variable declaration is used a key.
      //Note that the hash table will be increased if at least 32 values are added,
      //This can be avoided by increasing the initial size.
      int i = 0;
      while (i < EqnsLength) {
        //StartPrefix represents the start index of the maximum consistent prefix
        //of variable declarations in Eqns to which Eqns(i) belongs
        //VarDeclTable represents the variable declarations of Eqns from
        //StartPrefix up to i.
        //Check consistency of Eqns(i) with VarDeclTable and add newly declared
        //variables to VarDeclTable.
        ATermAppl Eqn = ATAelementAt(Eqns, i);
        bool Consistent = gsHasConsistentContext(VarDeclTable, Eqn);
        if (Consistent) {
          //add new variables from Eqns(i) to VarDeclTable
          ATermList VarDecls = ATLgetArgument(Eqn, 0);
          int VarDeclsLength = ATgetLength(VarDecls);
          for (int j = 0; j < VarDeclsLength; j++) {
            ATermAppl VarDecl = ATAelementAt(VarDecls, j);
            ATermAppl VarDeclName = ATAgetArgument(VarDecl, 0);
            if (ATtableGet(VarDeclTable, (ATerm) VarDeclName) == NULL) {
              ATtablePut(VarDeclTable, (ATerm) VarDeclName, (ATerm) VarDecl);
            }
          }
          i++;
        }
        if (!Consistent || (i == EqnsLength)) {
          //VarDeclTable represents the maximum consistent prefix of variable
          //declarations of Eqns starting at StartPrefix. Print this prefixa and
          //the corresponding equations,and if necessary, update StartPrefix and
          //reset VarDeclTable.
          ATermList VarDecls = ATtableValues(VarDeclTable);
          if (ATgetLength(VarDecls) > 0) {
            PRINT_FUNC(fprints)(OutStream, "var  ");
            PRINT_FUNC(PrintDecls)(OutStream, gsGroupDeclsBySort(VarDecls),
              pp_format, ";\n", "     ");
          }
          if (gsIsDataEqn(Eqn)) {
            PRINT_FUNC(fprints)(OutStream, "eqn  ");
          } else { //gsIsProcEqn(Eqn)
            PRINT_FUNC(fprints)(OutStream, "proc ");
          }
          PRINT_FUNC(PrintPart_List)(OutStream, ATgetSlice(Eqns, StartPrefix, i),
             pp_format, ShowSorts, PrecLevel, ";\n", "     ");
          if (i < EqnsLength) {
            PRINT_FUNC(fprints)(OutStream, "\n");
            StartPrefix = i;
            ATtableReset(VarDeclTable);
          }
        }
      }
      //finalisation after printing all (>0) equations
      PRINT_FUNC(fprints)(OutStream, "\n");
      ATtableDestroy(VarDeclTable);
    }
  }
}

void PRINT_FUNC(PrintDecls)(PRINT_OUTTYPE OutStream, const ATermList Decls,
  t_pp_format pp_format, const char *Terminator, const char *Separator)
{
  int n = ATgetLength(Decls);
  if (n > 0) {
    for (int i = 0; i < n-1; i++) {
      ATermAppl Decl = ATAelementAt(Decls, i);
      //check if sorts of Decls(i) and Decls(i+1) are equal
      if (ATisEqual(ATgetArgument(Decl, 1),
          ATgetArgument(ATelementAt(Decls, i+1), 1))) {
        PRINT_FUNC(PrintDecl)(OutStream, Decl, pp_format, false);
        PRINT_FUNC(fprints)(OutStream, ",");
      } else {
        PRINT_FUNC(PrintDecl)(OutStream, Decl, pp_format, true);
        if (Terminator  != NULL) PRINT_FUNC(fprints)(OutStream, Terminator);
        if (Separator  != NULL) PRINT_FUNC(fprints)(OutStream, Separator);
      }
    }
    PRINT_FUNC(PrintDecl)(OutStream, ATAelementAt(Decls, n-1),
      pp_format, true);
    if (Terminator  != NULL) PRINT_FUNC(fprints)(OutStream, Terminator);
  }
}

void PRINT_FUNC(PrintDecl)(PRINT_OUTTYPE OutStream, const ATermAppl Decl,
  t_pp_format pp_format, const bool ShowSorts)
{
  PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Decl, 0),
    pp_format, ShowSorts, 0);
  if (ShowSorts) {
    if (gsIsActId(Decl)) {
      ATermList SortExprs = ATLgetArgument(Decl, 1);
      if (ATgetLength(SortExprs) > 0) {
        PRINT_FUNC(fprints)(OutStream, ": ");
        PRINT_FUNC(PrintPart_List)(OutStream, SortExprs,
          pp_format, ShowSorts, 2, NULL, " # ");
      }
    } else {
      PRINT_FUNC(fprints)(OutStream, ": ");
      PRINT_FUNC(PrintPart_Appl)(OutStream, ATAgetArgument(Decl, 1),
        pp_format, ShowSorts, 0);
    }
  }
}

void PRINT_FUNC(PrintListEnumElts)(PRINT_OUTTYPE OutStream,
  const ATermAppl DataExpr, t_pp_format pp_format, bool ShowSorts)
{
  ATermAppl HeadName = ATAgetArgument(gsGetDataExprHead(DataExpr), 0);
  if (ATisEqual(HeadName, gsMakeOpIdNameCons())) {
    ATermList Args = gsGetDataExprArgs(DataExpr);
    PRINT_FUNC(PrintPart_Appl)(OutStream, ATAelementAt(Args, 0),
      pp_format, ShowSorts, 0);
    ATermAppl Arg1 = ATAelementAt(Args, 1);
    if (ATisEqual(ATAgetArgument(gsGetDataExprHead(Arg1), 0),
      gsMakeOpIdNameCons()))
    {
      PRINT_FUNC(fprints)(OutStream, ", ");
      PRINT_FUNC(PrintListEnumElts)(OutStream, Arg1, pp_format, ShowSorts);
    }
  }
}

void PRINT_FUNC(PrintPos)(PRINT_OUTTYPE OutStream, const ATermAppl PosExpr, int PrecLevel)
{
  if (gsIsPosConstant(PosExpr)) {
    char *PosValue = gsPosValue(PosExpr);
    PRINT_FUNC(fprints)(OutStream, PosValue);
    free(PosValue);
  } else {
    PRINT_FUNC(PrintPosMult)(OutStream, PosExpr, PrecLevel, "1");
  }
}

void PRINT_FUNC(PrintPosMult)(PRINT_OUTTYPE OutStream, const ATermAppl PosExpr, int PrecLevel,
  char *Mult)
{
  ATermAppl Head = gsGetDataExprHead(PosExpr);
  ATermList Args = gsGetDataExprArgs(PosExpr);
  if (ATisEqual(PosExpr, gsMakeOpId1())) {
    //PosExpr is 1; print Mult
    PRINT_FUNC(fprints)(OutStream, Mult);
  } else if (ATisEqual(Head, gsMakeOpIdCDub())) {
    //PosExpr is of the form cDub(b,p); print (Mult*2)*v(p) + Mult*v(b)
    ATermAppl BoolArg = ATAelementAt(Args, 0);
    ATermAppl PosArg = ATAelementAt(Args, 1);
    char *NewMult = gsStringDub(Mult, 0);
    if (ATisEqual(BoolArg, gsMakeDataExprFalse())) {
      //Mult*v(b) = 0
      PRINT_FUNC(PrintPosMult)(OutStream, PosArg, PrecLevel, NewMult);
    } else {
      //Mult*v(b) > 0
      if (PrecLevel > gsPrecOpIdInfix(gsMakeOpIdNameAdd())) {
        PRINT_FUNC(fprints)(OutStream, "(");
      }
      //print (Mult*2)*v(p)
      PRINT_FUNC(PrintPosMult)(OutStream, PosArg, 
        gsPrecOpIdInfixLeft(gsMakeOpIdNameAdd()), NewMult);
      PRINT_FUNC(fprints)(OutStream, " + ");
      if (ATisEqual(BoolArg, gsMakeDataExprTrue())) {
        //Mult*v(b) = Mult
        PRINT_FUNC(fprints)(OutStream, Mult);
      } else if (strcmp(Mult, "1") == 0) {
        //Mult*v(b) = v(b)
        PRINT_FUNC(PrintPart_Appl)(OutStream, BoolArg,
          ppDefault, false, gsPrecOpIdInfixRight(gsMakeOpIdNameAdd()));
      } else {
        //print Mult*v(b)
        PRINT_FUNC(fprints)(OutStream, Mult);
        PRINT_FUNC(fprints)(OutStream, "*");
        PRINT_FUNC(PrintPart_Appl)(OutStream, BoolArg,
          ppDefault, false, gsPrecOpIdInfixRight(gsMakeOpIdNameMult()));
      }
      if (PrecLevel > gsPrecOpIdInfix(gsMakeOpIdNameAdd())) {
        PRINT_FUNC(fprints)(OutStream, ")");
      }
    }
    free(NewMult);
  } else {
    //PosExpr is not a Pos constructor
    if (strcmp(Mult, "1") == 0) {
      PRINT_FUNC(PrintPart_Appl)(OutStream, PosExpr,
        ppDefault, false, PrecLevel);
    } else {
      PRINT_FUNC(fprints)(OutStream, Mult);
      PRINT_FUNC(fprints)(OutStream, "*");
      PRINT_FUNC(PrintPart_Appl)(OutStream, PosExpr,
        ppDefault, false, gsPrecOpIdInfixRight(gsMakeOpIdNameMult()));
    }
  }
}

ATermList gsGroupDeclsBySort(const ATermList Decls)
{
  int DeclsLength = ATgetLength(Decls);
  if (DeclsLength > 0) {
    ATermTable SortDeclsTable = ATtableCreate(2*DeclsLength, 50);
    //Add all variable declarations from Decls to hash table
    //SortDeclsTable
    for (int i = 0; i < DeclsLength; i++) {
      ATermAppl Decl = ATAelementAt(Decls, i);
      ATermAppl DeclSort = ATAgetArgument(Decl, 1);
      ATermList CorDecls = ATLtableGet(SortDeclsTable,
        (ATerm) DeclSort);
      if (CorDecls == NULL) {
        ATtablePut(SortDeclsTable, (ATerm) DeclSort,
          (ATerm) ATmakeList1((ATerm) Decl));
      } else {
        ATtablePut(SortDeclsTable, (ATerm) DeclSort,
          (ATerm) ATappend(CorDecls, (ATerm) Decl));
      }
    }
    //Return the hash table as a list of variable declarations
    ATermList Result = ATmakeList0();
    ATermList DeclSorts = ATtableKeys(SortDeclsTable);
    int DeclSortsLength = ATgetLength(DeclSorts);
    for (int i = 0; i < DeclSortsLength; i++) {
      Result = ATconcat(
        Result,
        ATLtableGet(SortDeclsTable, ATelementAt(DeclSorts, i)));
    }
    ATtableDestroy(SortDeclsTable);
    return Result;
  } else {
    //Decls is empty
    return Decls;
  }
}

bool gsHasConsistentContext(const ATermTable DataVarDecls,
  const ATermAppl Part)
{
  bool Result = true;
  if (gsIsDataEqn(Part) || gsIsProcEqn(Part)) {
    //check consistency of DataVarDecls with the variable declarations
    ATermList VarDecls = ATLgetArgument(Part, 0);
    int n = ATgetLength(VarDecls);
    for (int i = 0; i < n && Result; i++) {
      //check consistency of variable VarDecls(j) with VarDeclTable
      ATermAppl VarDecl = ATAelementAt(VarDecls, i);
      ATermAppl CorVarDecl =
        ATAtableGet(DataVarDecls, ATgetArgument(VarDecl, 0));
      if (CorVarDecl != NULL) {
        //check consistency of VarDecl with CorVarDecl
        Result = (ATisEqual(VarDecl, CorVarDecl) == ATtrue);
      }
    }
  } else if (gsIsOpId(Part) || gsIsDataVarIdOpId(Part))  {
    //Part may be an operation; check that its name does not occur in
    //DataVarDecls
    Result = (ATtableGet(DataVarDecls, ATgetArgument(Part, 0)) == NULL);
  }
  //check consistency in the arguments of Part
  if (Result) {
    AFun Head = ATgetAFun(Part);
    int NrArgs = ATgetArity(Head);      
    for (int i = 0; i < NrArgs && Result; i++) {
      ATerm Arg = ATgetArgument(Part, i);
      if (ATgetType(Arg) == AT_APPL)
        Result = gsHasConsistentContext(DataVarDecls, (ATermAppl) Arg);
      else //ATgetType(Arg) == AT_LIST
        Result = gsHasConsistentContextList(DataVarDecls, (ATermList) Arg);
    }
  }
  return Result;
}

bool gsHasConsistentContextList(const ATermTable DataVarDecls,
  const ATermList Parts)
{
  bool Result = true;
  ATermList l = Parts;
  while (!ATisEmpty(l) && Result) {
    Result = gsHasConsistentContext(DataVarDecls, ATAgetFirst(l));
    l = ATgetNext(l);
  }
  return Result;
}

bool gsIsListEnumImpl(ATermAppl DataExpr)
{
  if (!gsIsDataAppl(DataExpr) && !gsIsOpId(DataExpr)) return false;
  ATermAppl HeadName = ATAgetArgument(gsGetDataExprHead(DataExpr), 0);
  if (ATisEqual(HeadName, gsMakeOpIdNameCons())) {
    ATermList Args = gsGetDataExprArgs(DataExpr);
    if (ATgetLength(Args) == 2) {
      return gsIsListEnumImpl(ATAelementAt(Args, 1));
    } else {
      return false;
    }
  } else {
    return ATisEqual(HeadName, gsMakeOpIdNameEmptyList());
  }
}

bool gsIsOpIdSetBagComp(ATermAppl Term)
{
  if (!gsIsOpId(Term)) {
    return false;
  }
  ATermAppl OpIdName = ATAgetArgument(Term, 0);
  return (OpIdName == gsMakeOpIdNameSetComp()) || (OpIdName == gsMakeOpIdNameBagComp());
}

bool gsIsOpIdQuant(ATermAppl Term)
{
  if (!gsIsOpId(Term)) {
    return false;
  }
  ATermAppl OpIdName = ATAgetArgument(Term, 0);
  return (OpIdName == gsMakeOpIdNameForall()) || (OpIdName == gsMakeOpIdNameExists());
}

bool gsIsOpIdPrefix(ATermAppl Term)
{
  if (!gsIsOpId(Term)) {
    return false;
  }
  ATermAppl OpIdName = ATAgetArgument(Term, 0);
  return (ATgetLength(gsGetSortExprDomain(ATAgetArgument(Term, 1))) == 1) && (
    (OpIdName == gsMakeOpIdNameNot()) || (OpIdName == gsMakeOpIdNameNeg()) ||
    (OpIdName == gsMakeOpIdNameListSize()) || (OpIdName == gsMakeOpIdNameSetCompl())
    );
}

bool gsIsOpIdInfix(ATermAppl Term)
{
  if (!gsIsOpId(Term)) {
    return false;
  }
  ATermAppl OpIdName = ATAgetArgument(Term, 0);
  return (ATgetLength(gsGetSortExprDomain(ATAgetArgument(Term, 1))) == 2) &&
    ((OpIdName == gsMakeOpIdNameImp())          ||
     (OpIdName == gsMakeOpIdNameAnd())          ||
     (OpIdName == gsMakeOpIdNameOr())           ||
     (OpIdName == gsMakeOpIdNameEq())           ||
     (OpIdName == gsMakeOpIdNameNeq())          ||
     (OpIdName == gsMakeOpIdNameLT())           ||
     (OpIdName == gsMakeOpIdNameLTE())          ||
     (OpIdName == gsMakeOpIdNameGT())           ||
     (OpIdName == gsMakeOpIdNameGTE())          ||
     (OpIdName == gsMakeOpIdNameEltIn())        ||
     (OpIdName == gsMakeOpIdNameSubSetEq())     ||
     (OpIdName == gsMakeOpIdNameSubSet())       ||
     (OpIdName == gsMakeOpIdNameSubBagEq())     ||
     (OpIdName == gsMakeOpIdNameSubBag())       ||
     (OpIdName == gsMakeOpIdNameCons())         ||
     (OpIdName == gsMakeOpIdNameSnoc())         ||
     (OpIdName == gsMakeOpIdNameConcat())       ||
     (OpIdName == gsMakeOpIdNameAdd())          ||
     (OpIdName == gsMakeOpIdNameSubt())         ||
     (OpIdName == gsMakeOpIdNameSetUnion())     ||
     (OpIdName == gsMakeOpIdNameSetDiff())      ||
     (OpIdName == gsMakeOpIdNameBagUnion())     ||
     (OpIdName == gsMakeOpIdNameBagDiff())      ||
     (OpIdName == gsMakeOpIdNameDiv())          ||
     (OpIdName == gsMakeOpIdNameMod())          ||
     (OpIdName == gsMakeOpIdNameMult())         ||
     (OpIdName == gsMakeOpIdNameEltAt())        ||
     (OpIdName == gsMakeOpIdNameSetIntersect()) ||
     (OpIdName == gsMakeOpIdNameBagIntersect()));
}

int gsPrecOpIdInfix(ATermAppl OpIdName)
{
  if (OpIdName == gsMakeOpIdNameImp()) {
    return 2;
  } else if ((OpIdName == gsMakeOpIdNameAnd()) || (OpIdName == gsMakeOpIdNameOr())) {
    return 3;
  } else if ((OpIdName == gsMakeOpIdNameEq()) || (OpIdName == gsMakeOpIdNameNeq())) {
    return 4;
  } else if (
      (OpIdName == gsMakeOpIdNameLT()) || (OpIdName == gsMakeOpIdNameLTE()) ||
      (OpIdName == gsMakeOpIdNameGT()) || (OpIdName == gsMakeOpIdNameGTE()) ||
      (OpIdName == gsMakeOpIdNameEltIn()) ||
      (OpIdName == gsMakeOpIdNameSubSetEq()) || (OpIdName == gsMakeOpIdNameSubSet()) ||
      (OpIdName == gsMakeOpIdNameSubBagEq()) || (OpIdName == gsMakeOpIdNameSubBag())
      ) {
    return 5;
  } else if ((OpIdName == gsMakeOpIdNameCons())) {
    return 6;
  } else if ((OpIdName == gsMakeOpIdNameSnoc())) {
    return 7;
  } else if ((OpIdName == gsMakeOpIdNameConcat())) {
    return 8;
  } else if (
      (OpIdName == gsMakeOpIdNameAdd()) || (OpIdName == gsMakeOpIdNameSubt()) ||
      (OpIdName == gsMakeOpIdNameSetUnion()) || (OpIdName == gsMakeOpIdNameSetDiff()) ||
      (OpIdName == gsMakeOpIdNameBagUnion()) || (OpIdName == gsMakeOpIdNameBagDiff())
      ) {
    return 9;
  } else if ((OpIdName == gsMakeOpIdNameDiv()) || (OpIdName == gsMakeOpIdNameMod())) {
    return 10;
  } else if (
      (OpIdName == gsMakeOpIdNameMult()) || (OpIdName == gsMakeOpIdNameEltAt()) ||
      (OpIdName == gsMakeOpIdNameSetIntersect()) ||
      (OpIdName == gsMakeOpIdNameBagIntersect())
      ){
    return 11;
  } else {
    //something went wrong
    return -1;
  }
}

int gsPrecOpIdInfixLeft(ATermAppl OpIdName)
{
  if (OpIdName == gsMakeOpIdNameImp()) {
    return 3;
  } else if ((OpIdName == gsMakeOpIdNameAnd()) || (OpIdName == gsMakeOpIdNameOr())) {
    return 3;
  } else if ((OpIdName == gsMakeOpIdNameEq()) || (OpIdName == gsMakeOpIdNameNeq())) {
    return 4;
  } else if (
      (OpIdName == gsMakeOpIdNameLT()) || (OpIdName == gsMakeOpIdNameLTE()) ||
      (OpIdName == gsMakeOpIdNameGT()) || (OpIdName == gsMakeOpIdNameGTE()) ||
      (OpIdName == gsMakeOpIdNameEltIn()) ||
      (OpIdName == gsMakeOpIdNameSubSetEq()) || (OpIdName == gsMakeOpIdNameSubSet()) ||
      (OpIdName == gsMakeOpIdNameSubBagEq()) || (OpIdName == gsMakeOpIdNameSubBag())
      ) {
    return 6;
  } else if ((OpIdName == gsMakeOpIdNameCons())) {
    return 9;
  } else if ((OpIdName == gsMakeOpIdNameSnoc())) {
    return 7;
  } else if ((OpIdName == gsMakeOpIdNameConcat())) {
    return 8;
  } else if (
      (OpIdName == gsMakeOpIdNameAdd()) || (OpIdName == gsMakeOpIdNameSubt()) ||
      (OpIdName == gsMakeOpIdNameSetUnion()) || (OpIdName == gsMakeOpIdNameSetDiff()) ||
      (OpIdName == gsMakeOpIdNameBagUnion()) || (OpIdName == gsMakeOpIdNameBagDiff())
      ) {
    return 9;
  } else if ((OpIdName == gsMakeOpIdNameDiv()) || (OpIdName == gsMakeOpIdNameMod())) {
    return 10;
  } else if (
      (OpIdName == gsMakeOpIdNameMult()) || (OpIdName == gsMakeOpIdNameEltAt()) ||
      (OpIdName == gsMakeOpIdNameSetIntersect()) ||
      (OpIdName == gsMakeOpIdNameBagIntersect())
      ){
    return 11;
  } else {
    //something went wrong
    return -1;
  }
}

int gsPrecOpIdInfixRight(ATermAppl OpIdName)
{
  if (OpIdName == gsMakeOpIdNameImp()) {
    return 2;
  } else if ((OpIdName == gsMakeOpIdNameAnd()) || (OpIdName == gsMakeOpIdNameOr())) {
    return 4;
  } else if ((OpIdName == gsMakeOpIdNameEq()) || (OpIdName == gsMakeOpIdNameNeq())) {
    return 5;
  } else if (
      (OpIdName == gsMakeOpIdNameLT()) || (OpIdName == gsMakeOpIdNameLTE()) ||
      (OpIdName == gsMakeOpIdNameGT()) || (OpIdName == gsMakeOpIdNameGTE()) ||
      (OpIdName == gsMakeOpIdNameEltIn()) ||
      (OpIdName == gsMakeOpIdNameSubSetEq()) || (OpIdName == gsMakeOpIdNameSubSet()) ||
      (OpIdName == gsMakeOpIdNameSubBagEq()) || (OpIdName == gsMakeOpIdNameSubBag())
      ) {
    return 6;
  } else if ((OpIdName == gsMakeOpIdNameCons())) {
    return 6;
  } else if ((OpIdName == gsMakeOpIdNameSnoc())) {
    return 9;
  } else if ((OpIdName == gsMakeOpIdNameConcat())) {
    return 9;
  } else if (
      (OpIdName == gsMakeOpIdNameAdd()) || (OpIdName == gsMakeOpIdNameSubt()) ||
      (OpIdName == gsMakeOpIdNameSetUnion()) || (OpIdName == gsMakeOpIdNameSetDiff()) ||
      (OpIdName == gsMakeOpIdNameBagUnion()) || (OpIdName == gsMakeOpIdNameBagDiff())
      ) {
    return 10;
  } else if ((OpIdName == gsMakeOpIdNameDiv()) || (OpIdName == gsMakeOpIdNameMod())) {
    return 11;
  } else if (
      (OpIdName == gsMakeOpIdNameMult()) || (OpIdName == gsMakeOpIdNameEltAt()) ||
      (OpIdName == gsMakeOpIdNameSetIntersect()) ||
      (OpIdName == gsMakeOpIdNameBagIntersect())
      ){
    return 12;
  } else {
    //something went wrong
    return -1;
  }
}
