// Author(s): Aad Mathijssen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file libstruct_core.h

#ifndef MCRL2_LIBSTRUCT_CORE_H
#define MCRL2_LIBSTRUCT_CORE_H

//This file describes the functions that can be used for the internal aterm
//structure.

#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/atermpp/aterm_list.h"

namespace mcrl2
{
namespace core
{
namespace detail
{

using namespace atermpp;

inline
bool operator==(const size_t x, const atermpp::function_symbol& y)
{
  // return x.name() == y.name() && x.arity() == y.arity() && x.is_quoted() == y.is_quoted();
  return x == function_symbol(y).number();
}

//Global precondition: the aterm library has been initialised

// DataAppl
inline std::vector<atermpp::function_symbol>& function_symbols_DataAppl()
{
  static std::vector<atermpp::function_symbol> function_symbols_DataAppl;
  return function_symbols_DataAppl;
}

inline
const atermpp::function_symbol& function_symbol_DataAppl(size_t i)
{
  std::vector<atermpp::function_symbol>& syms = function_symbols_DataAppl();
  while (i >= syms.size())
  {
    syms.push_back(atermpp::function_symbol("DataAppl", syms.size()));
  }
  return syms[i];
}

inline
bool gsIsDataAppl(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_DataAppl(Term.function().arity());
}

// DataVarIdNoIndex
inline
const atermpp::function_symbol& function_symbol_DataVarIdNoIndex()
{
  static atermpp::function_symbol f = atermpp::function_symbol("DataVarIdNoIndex", 2);
  return f;
}

// OpIdIndex
inline
const atermpp::function_symbol& function_symbol_OpIdNoIndex()
{
  static atermpp::function_symbol f = atermpp::function_symbol("OpIdNoIndex", 2);
  return f;
}

// ProcVarIdNoIndex
inline
const atermpp::function_symbol& function_symbol_ProcVarIdNoIndex()
{
  static atermpp::function_symbol f = atermpp::function_symbol("ProcVarIdNoIndex", 2);
  return f;
}

// BooleanVariableNoIndex
inline
const atermpp::function_symbol& function_symbol_BooleanVariableNoIndex()
{
  static atermpp::function_symbol f = atermpp::function_symbol("BooleanVariableNoIndex", 1);
  return f;
}

// PropVarInstNoIndex
inline
const atermpp::function_symbol& function_symbol_PropVarInstNoIndex()
{
  static atermpp::function_symbol f = atermpp::function_symbol("PropVarInstNoIndex", 2);
  return f;
}

//--- start generated code ---//
// ActAnd
inline
const atermpp::function_symbol& function_symbol_ActAnd()
{
  static atermpp::function_symbol function_symbol_ActAnd = atermpp::function_symbol("ActAnd", 2);
  return function_symbol_ActAnd;
}

inline
bool gsIsActAnd(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActAnd();
}

// ActAt
inline
const atermpp::function_symbol& function_symbol_ActAt()
{
  static atermpp::function_symbol function_symbol_ActAt = atermpp::function_symbol("ActAt", 2);
  return function_symbol_ActAt;
}

inline
bool gsIsActAt(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActAt();
}

// ActExists
inline
const atermpp::function_symbol& function_symbol_ActExists()
{
  static atermpp::function_symbol function_symbol_ActExists = atermpp::function_symbol("ActExists", 2);
  return function_symbol_ActExists;
}

inline
bool gsIsActExists(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActExists();
}

// ActFalse
inline
const atermpp::function_symbol& function_symbol_ActFalse()
{
  static atermpp::function_symbol function_symbol_ActFalse = atermpp::function_symbol("ActFalse", 0);
  return function_symbol_ActFalse;
}

inline
bool gsIsActFalse(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActFalse();
}

// ActForall
inline
const atermpp::function_symbol& function_symbol_ActForall()
{
  static atermpp::function_symbol function_symbol_ActForall = atermpp::function_symbol("ActForall", 2);
  return function_symbol_ActForall;
}

inline
bool gsIsActForall(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActForall();
}

// ActId
inline
const atermpp::function_symbol& function_symbol_ActId()
{
  static atermpp::function_symbol function_symbol_ActId = atermpp::function_symbol("ActId", 2);
  return function_symbol_ActId;
}

inline
bool gsIsActId(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActId();
}

// ActImp
inline
const atermpp::function_symbol& function_symbol_ActImp()
{
  static atermpp::function_symbol function_symbol_ActImp = atermpp::function_symbol("ActImp", 2);
  return function_symbol_ActImp;
}

inline
bool gsIsActImp(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActImp();
}

// ActMultAct
inline
const atermpp::function_symbol& function_symbol_ActMultAct()
{
  static atermpp::function_symbol function_symbol_ActMultAct = atermpp::function_symbol("ActMultAct", 1);
  return function_symbol_ActMultAct;
}

inline
bool gsIsActMultAct(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActMultAct();
}

// ActNot
inline
const atermpp::function_symbol& function_symbol_ActNot()
{
  static atermpp::function_symbol function_symbol_ActNot = atermpp::function_symbol("ActNot", 1);
  return function_symbol_ActNot;
}

inline
bool gsIsActNot(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActNot();
}

// ActOr
inline
const atermpp::function_symbol& function_symbol_ActOr()
{
  static atermpp::function_symbol function_symbol_ActOr = atermpp::function_symbol("ActOr", 2);
  return function_symbol_ActOr;
}

inline
bool gsIsActOr(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActOr();
}

// ActSpec
inline
const atermpp::function_symbol& function_symbol_ActSpec()
{
  static atermpp::function_symbol function_symbol_ActSpec = atermpp::function_symbol("ActSpec", 1);
  return function_symbol_ActSpec;
}

inline
bool gsIsActSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActSpec();
}

// ActTrue
inline
const atermpp::function_symbol& function_symbol_ActTrue()
{
  static atermpp::function_symbol function_symbol_ActTrue = atermpp::function_symbol("ActTrue", 0);
  return function_symbol_ActTrue;
}

inline
bool gsIsActTrue(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActTrue();
}

// Action
inline
const atermpp::function_symbol& function_symbol_Action()
{
  static atermpp::function_symbol function_symbol_Action = atermpp::function_symbol("Action", 2);
  return function_symbol_Action;
}

inline
bool gsIsAction(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Action();
}

// ActionRenameRule
inline
const atermpp::function_symbol& function_symbol_ActionRenameRule()
{
  static atermpp::function_symbol function_symbol_ActionRenameRule = atermpp::function_symbol("ActionRenameRule", 4);
  return function_symbol_ActionRenameRule;
}

inline
bool gsIsActionRenameRule(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActionRenameRule();
}

// ActionRenameRules
inline
const atermpp::function_symbol& function_symbol_ActionRenameRules()
{
  static atermpp::function_symbol function_symbol_ActionRenameRules = atermpp::function_symbol("ActionRenameRules", 1);
  return function_symbol_ActionRenameRules;
}

inline
bool gsIsActionRenameRules(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActionRenameRules();
}

// ActionRenameSpec
inline
const atermpp::function_symbol& function_symbol_ActionRenameSpec()
{
  static atermpp::function_symbol function_symbol_ActionRenameSpec = atermpp::function_symbol("ActionRenameSpec", 3);
  return function_symbol_ActionRenameSpec;
}

inline
bool gsIsActionRenameSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ActionRenameSpec();
}

// Allow
inline
const atermpp::function_symbol& function_symbol_Allow()
{
  static atermpp::function_symbol function_symbol_Allow = atermpp::function_symbol("Allow", 2);
  return function_symbol_Allow;
}

inline
bool gsIsAllow(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Allow();
}

// AtTime
inline
const atermpp::function_symbol& function_symbol_AtTime()
{
  static atermpp::function_symbol function_symbol_AtTime = atermpp::function_symbol("AtTime", 2);
  return function_symbol_AtTime;
}

inline
bool gsIsAtTime(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_AtTime();
}

// BES
inline
const atermpp::function_symbol& function_symbol_BES()
{
  static atermpp::function_symbol function_symbol_BES = atermpp::function_symbol("BES", 2);
  return function_symbol_BES;
}

inline
bool gsIsBES(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BES();
}

// BInit
inline
const atermpp::function_symbol& function_symbol_BInit()
{
  static atermpp::function_symbol function_symbol_BInit = atermpp::function_symbol("BInit", 2);
  return function_symbol_BInit;
}

inline
bool gsIsBInit(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BInit();
}

// BagComp
inline
const atermpp::function_symbol& function_symbol_BagComp()
{
  static atermpp::function_symbol function_symbol_BagComp = atermpp::function_symbol("BagComp", 0);
  return function_symbol_BagComp;
}

inline
bool gsIsBagComp(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BagComp();
}

// Binder
inline
const atermpp::function_symbol& function_symbol_Binder()
{
  static atermpp::function_symbol function_symbol_Binder = atermpp::function_symbol("Binder", 3);
  return function_symbol_Binder;
}

inline
bool gsIsBinder(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Binder();
}

// Block
inline
const atermpp::function_symbol& function_symbol_Block()
{
  static atermpp::function_symbol function_symbol_Block = atermpp::function_symbol("Block", 2);
  return function_symbol_Block;
}

inline
bool gsIsBlock(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Block();
}

// BooleanAnd
inline
const atermpp::function_symbol& function_symbol_BooleanAnd()
{
  static atermpp::function_symbol function_symbol_BooleanAnd = atermpp::function_symbol("BooleanAnd", 2);
  return function_symbol_BooleanAnd;
}

inline
bool gsIsBooleanAnd(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BooleanAnd();
}

// BooleanEquation
inline
const atermpp::function_symbol& function_symbol_BooleanEquation()
{
  static atermpp::function_symbol function_symbol_BooleanEquation = atermpp::function_symbol("BooleanEquation", 3);
  return function_symbol_BooleanEquation;
}

inline
bool gsIsBooleanEquation(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BooleanEquation();
}

// BooleanFalse
inline
const atermpp::function_symbol& function_symbol_BooleanFalse()
{
  static atermpp::function_symbol function_symbol_BooleanFalse = atermpp::function_symbol("BooleanFalse", 0);
  return function_symbol_BooleanFalse;
}

inline
bool gsIsBooleanFalse(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BooleanFalse();
}

// BooleanImp
inline
const atermpp::function_symbol& function_symbol_BooleanImp()
{
  static atermpp::function_symbol function_symbol_BooleanImp = atermpp::function_symbol("BooleanImp", 2);
  return function_symbol_BooleanImp;
}

inline
bool gsIsBooleanImp(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BooleanImp();
}

// BooleanNot
inline
const atermpp::function_symbol& function_symbol_BooleanNot()
{
  static atermpp::function_symbol function_symbol_BooleanNot = atermpp::function_symbol("BooleanNot", 1);
  return function_symbol_BooleanNot;
}

inline
bool gsIsBooleanNot(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BooleanNot();
}

// BooleanOr
inline
const atermpp::function_symbol& function_symbol_BooleanOr()
{
  static atermpp::function_symbol function_symbol_BooleanOr = atermpp::function_symbol("BooleanOr", 2);
  return function_symbol_BooleanOr;
}

inline
bool gsIsBooleanOr(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BooleanOr();
}

// BooleanTrue
inline
const atermpp::function_symbol& function_symbol_BooleanTrue()
{
  static atermpp::function_symbol function_symbol_BooleanTrue = atermpp::function_symbol("BooleanTrue", 0);
  return function_symbol_BooleanTrue;
}

inline
bool gsIsBooleanTrue(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BooleanTrue();
}

// BooleanVariable
inline
const atermpp::function_symbol& function_symbol_BooleanVariable()
{
  static atermpp::function_symbol function_symbol_BooleanVariable = atermpp::function_symbol("BooleanVariable", 2);
  return function_symbol_BooleanVariable;
}

inline
bool gsIsBooleanVariable(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_BooleanVariable();
}

// Choice
inline
const atermpp::function_symbol& function_symbol_Choice()
{
  static atermpp::function_symbol function_symbol_Choice = atermpp::function_symbol("Choice", 2);
  return function_symbol_Choice;
}

inline
bool gsIsChoice(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Choice();
}

// Comm
inline
const atermpp::function_symbol& function_symbol_Comm()
{
  static atermpp::function_symbol function_symbol_Comm = atermpp::function_symbol("Comm", 2);
  return function_symbol_Comm;
}

inline
bool gsIsComm(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Comm();
}

// CommExpr
inline
const atermpp::function_symbol& function_symbol_CommExpr()
{
  static atermpp::function_symbol function_symbol_CommExpr = atermpp::function_symbol("CommExpr", 2);
  return function_symbol_CommExpr;
}

inline
bool gsIsCommExpr(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_CommExpr();
}

// ConsSpec
inline
const atermpp::function_symbol& function_symbol_ConsSpec()
{
  static atermpp::function_symbol function_symbol_ConsSpec = atermpp::function_symbol("ConsSpec", 1);
  return function_symbol_ConsSpec;
}

inline
bool gsIsConsSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ConsSpec();
}

// DataEqn
inline
const atermpp::function_symbol& function_symbol_DataEqn()
{
  static atermpp::function_symbol function_symbol_DataEqn = atermpp::function_symbol("DataEqn", 4);
  return function_symbol_DataEqn;
}

inline
bool gsIsDataEqn(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_DataEqn();
}

// DataEqnSpec
inline
const atermpp::function_symbol& function_symbol_DataEqnSpec()
{
  static atermpp::function_symbol function_symbol_DataEqnSpec = atermpp::function_symbol("DataEqnSpec", 1);
  return function_symbol_DataEqnSpec;
}

inline
bool gsIsDataEqnSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_DataEqnSpec();
}

// DataSpec
inline
const atermpp::function_symbol& function_symbol_DataSpec()
{
  static atermpp::function_symbol function_symbol_DataSpec = atermpp::function_symbol("DataSpec", 4);
  return function_symbol_DataSpec;
}

inline
bool gsIsDataSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_DataSpec();
}

// DataVarId
inline
const atermpp::function_symbol& function_symbol_DataVarId()
{
  static atermpp::function_symbol function_symbol_DataVarId = atermpp::function_symbol("DataVarId", 3);
  return function_symbol_DataVarId;
}

inline
bool gsIsDataVarId(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_DataVarId();
}

// DataVarIdInit
inline
const atermpp::function_symbol& function_symbol_DataVarIdInit()
{
  static atermpp::function_symbol function_symbol_DataVarIdInit = atermpp::function_symbol("DataVarIdInit", 2);
  return function_symbol_DataVarIdInit;
}

inline
bool gsIsDataVarIdInit(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_DataVarIdInit();
}

// Delta
inline
const atermpp::function_symbol& function_symbol_Delta()
{
  static atermpp::function_symbol function_symbol_Delta = atermpp::function_symbol("Delta", 0);
  return function_symbol_Delta;
}

inline
bool gsIsDelta(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Delta();
}

// Exists
inline
const atermpp::function_symbol& function_symbol_Exists()
{
  static atermpp::function_symbol function_symbol_Exists = atermpp::function_symbol("Exists", 0);
  return function_symbol_Exists;
}

inline
bool gsIsExists(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Exists();
}

// Forall
inline
const atermpp::function_symbol& function_symbol_Forall()
{
  static atermpp::function_symbol function_symbol_Forall = atermpp::function_symbol("Forall", 0);
  return function_symbol_Forall;
}

inline
bool gsIsForall(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Forall();
}

// GlobVarSpec
inline
const atermpp::function_symbol& function_symbol_GlobVarSpec()
{
  static atermpp::function_symbol function_symbol_GlobVarSpec = atermpp::function_symbol("GlobVarSpec", 1);
  return function_symbol_GlobVarSpec;
}

inline
bool gsIsGlobVarSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_GlobVarSpec();
}

// Hide
inline
const atermpp::function_symbol& function_symbol_Hide()
{
  static atermpp::function_symbol function_symbol_Hide = atermpp::function_symbol("Hide", 2);
  return function_symbol_Hide;
}

inline
bool gsIsHide(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Hide();
}

// IfThen
inline
const atermpp::function_symbol& function_symbol_IfThen()
{
  static atermpp::function_symbol function_symbol_IfThen = atermpp::function_symbol("IfThen", 2);
  return function_symbol_IfThen;
}

inline
bool gsIsIfThen(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_IfThen();
}

// IfThenElse
inline
const atermpp::function_symbol& function_symbol_IfThenElse()
{
  static atermpp::function_symbol function_symbol_IfThenElse = atermpp::function_symbol("IfThenElse", 3);
  return function_symbol_IfThenElse;
}

inline
bool gsIsIfThenElse(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_IfThenElse();
}

// LMerge
inline
const atermpp::function_symbol& function_symbol_LMerge()
{
  static atermpp::function_symbol function_symbol_LMerge = atermpp::function_symbol("LMerge", 2);
  return function_symbol_LMerge;
}

inline
bool gsIsLMerge(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_LMerge();
}

// Lambda
inline
const atermpp::function_symbol& function_symbol_Lambda()
{
  static atermpp::function_symbol function_symbol_Lambda = atermpp::function_symbol("Lambda", 0);
  return function_symbol_Lambda;
}

inline
bool gsIsLambda(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Lambda();
}

// LinProcSpec
inline
const atermpp::function_symbol& function_symbol_LinProcSpec()
{
  static atermpp::function_symbol function_symbol_LinProcSpec = atermpp::function_symbol("LinProcSpec", 5);
  return function_symbol_LinProcSpec;
}

inline
bool gsIsLinProcSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_LinProcSpec();
}

// LinearProcess
inline
const atermpp::function_symbol& function_symbol_LinearProcess()
{
  static atermpp::function_symbol function_symbol_LinearProcess = atermpp::function_symbol("LinearProcess", 2);
  return function_symbol_LinearProcess;
}

inline
bool gsIsLinearProcess(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_LinearProcess();
}

// LinearProcessInit
inline
const atermpp::function_symbol& function_symbol_LinearProcessInit()
{
  static atermpp::function_symbol function_symbol_LinearProcessInit = atermpp::function_symbol("LinearProcessInit", 1);
  return function_symbol_LinearProcessInit;
}

inline
bool gsIsLinearProcessInit(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_LinearProcessInit();
}

// LinearProcessSummand
inline
const atermpp::function_symbol& function_symbol_LinearProcessSummand()
{
  static atermpp::function_symbol function_symbol_LinearProcessSummand = atermpp::function_symbol("LinearProcessSummand", 5);
  return function_symbol_LinearProcessSummand;
}

inline
bool gsIsLinearProcessSummand(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_LinearProcessSummand();
}

// MapSpec
inline
const atermpp::function_symbol& function_symbol_MapSpec()
{
  static atermpp::function_symbol function_symbol_MapSpec = atermpp::function_symbol("MapSpec", 1);
  return function_symbol_MapSpec;
}

inline
bool gsIsMapSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_MapSpec();
}

// Merge
inline
const atermpp::function_symbol& function_symbol_Merge()
{
  static atermpp::function_symbol function_symbol_Merge = atermpp::function_symbol("Merge", 2);
  return function_symbol_Merge;
}

inline
bool gsIsMerge(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Merge();
}

// Mu
inline
const atermpp::function_symbol& function_symbol_Mu()
{
  static atermpp::function_symbol function_symbol_Mu = atermpp::function_symbol("Mu", 0);
  return function_symbol_Mu;
}

inline
bool gsIsMu(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Mu();
}

// MultAct
inline
const atermpp::function_symbol& function_symbol_MultAct()
{
  static atermpp::function_symbol function_symbol_MultAct = atermpp::function_symbol("MultAct", 1);
  return function_symbol_MultAct;
}

inline
bool gsIsMultAct(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_MultAct();
}

// MultActName
inline
const atermpp::function_symbol& function_symbol_MultActName()
{
  static atermpp::function_symbol function_symbol_MultActName = atermpp::function_symbol("MultActName", 1);
  return function_symbol_MultActName;
}

inline
bool gsIsMultActName(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_MultActName();
}

// Nil
inline
const atermpp::function_symbol& function_symbol_Nil()
{
  static atermpp::function_symbol function_symbol_Nil = atermpp::function_symbol("Nil", 0);
  return function_symbol_Nil;
}

inline
bool gsIsNil(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Nil();
}

// Nu
inline
const atermpp::function_symbol& function_symbol_Nu()
{
  static atermpp::function_symbol function_symbol_Nu = atermpp::function_symbol("Nu", 0);
  return function_symbol_Nu;
}

inline
bool gsIsNu(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Nu();
}

// OpId
inline
const atermpp::function_symbol& function_symbol_OpId()
{
  static atermpp::function_symbol function_symbol_OpId = atermpp::function_symbol("OpId", 3);
  return function_symbol_OpId;
}

inline
bool gsIsOpId(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_OpId();
}

// PBES
inline
const atermpp::function_symbol& function_symbol_PBES()
{
  static atermpp::function_symbol function_symbol_PBES = atermpp::function_symbol("PBES", 4);
  return function_symbol_PBES;
}

inline
bool gsIsPBES(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBES();
}

// PBESAnd
inline
const atermpp::function_symbol& function_symbol_PBESAnd()
{
  static atermpp::function_symbol function_symbol_PBESAnd = atermpp::function_symbol("PBESAnd", 2);
  return function_symbol_PBESAnd;
}

inline
bool gsIsPBESAnd(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBESAnd();
}

// PBESExists
inline
const atermpp::function_symbol& function_symbol_PBESExists()
{
  static atermpp::function_symbol function_symbol_PBESExists = atermpp::function_symbol("PBESExists", 2);
  return function_symbol_PBESExists;
}

inline
bool gsIsPBESExists(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBESExists();
}

// PBESFalse
inline
const atermpp::function_symbol& function_symbol_PBESFalse()
{
  static atermpp::function_symbol function_symbol_PBESFalse = atermpp::function_symbol("PBESFalse", 0);
  return function_symbol_PBESFalse;
}

inline
bool gsIsPBESFalse(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBESFalse();
}

// PBESForall
inline
const atermpp::function_symbol& function_symbol_PBESForall()
{
  static atermpp::function_symbol function_symbol_PBESForall = atermpp::function_symbol("PBESForall", 2);
  return function_symbol_PBESForall;
}

inline
bool gsIsPBESForall(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBESForall();
}

// PBESImp
inline
const atermpp::function_symbol& function_symbol_PBESImp()
{
  static atermpp::function_symbol function_symbol_PBESImp = atermpp::function_symbol("PBESImp", 2);
  return function_symbol_PBESImp;
}

inline
bool gsIsPBESImp(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBESImp();
}

// PBESNot
inline
const atermpp::function_symbol& function_symbol_PBESNot()
{
  static atermpp::function_symbol function_symbol_PBESNot = atermpp::function_symbol("PBESNot", 1);
  return function_symbol_PBESNot;
}

inline
bool gsIsPBESNot(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBESNot();
}

// PBESOr
inline
const atermpp::function_symbol& function_symbol_PBESOr()
{
  static atermpp::function_symbol function_symbol_PBESOr = atermpp::function_symbol("PBESOr", 2);
  return function_symbol_PBESOr;
}

inline
bool gsIsPBESOr(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBESOr();
}

// PBESTrue
inline
const atermpp::function_symbol& function_symbol_PBESTrue()
{
  static atermpp::function_symbol function_symbol_PBESTrue = atermpp::function_symbol("PBESTrue", 0);
  return function_symbol_PBESTrue;
}

inline
bool gsIsPBESTrue(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBESTrue();
}

// PBEqn
inline
const atermpp::function_symbol& function_symbol_PBEqn()
{
  static atermpp::function_symbol function_symbol_PBEqn = atermpp::function_symbol("PBEqn", 3);
  return function_symbol_PBEqn;
}

inline
bool gsIsPBEqn(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBEqn();
}

// PBEqnSpec
inline
const atermpp::function_symbol& function_symbol_PBEqnSpec()
{
  static atermpp::function_symbol function_symbol_PBEqnSpec = atermpp::function_symbol("PBEqnSpec", 1);
  return function_symbol_PBEqnSpec;
}

inline
bool gsIsPBEqnSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBEqnSpec();
}

// PBInit
inline
const atermpp::function_symbol& function_symbol_PBInit()
{
  static atermpp::function_symbol function_symbol_PBInit = atermpp::function_symbol("PBInit", 1);
  return function_symbol_PBInit;
}

inline
bool gsIsPBInit(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PBInit();
}

// ProcEqn
inline
const atermpp::function_symbol& function_symbol_ProcEqn()
{
  static atermpp::function_symbol function_symbol_ProcEqn = atermpp::function_symbol("ProcEqn", 3);
  return function_symbol_ProcEqn;
}

inline
bool gsIsProcEqn(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ProcEqn();
}

// ProcEqnSpec
inline
const atermpp::function_symbol& function_symbol_ProcEqnSpec()
{
  static atermpp::function_symbol function_symbol_ProcEqnSpec = atermpp::function_symbol("ProcEqnSpec", 1);
  return function_symbol_ProcEqnSpec;
}

inline
bool gsIsProcEqnSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ProcEqnSpec();
}

// ProcSpec
inline
const atermpp::function_symbol& function_symbol_ProcSpec()
{
  static atermpp::function_symbol function_symbol_ProcSpec = atermpp::function_symbol("ProcSpec", 5);
  return function_symbol_ProcSpec;
}

inline
bool gsIsProcSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ProcSpec();
}

// ProcVarId
inline
const atermpp::function_symbol& function_symbol_ProcVarId()
{
  static atermpp::function_symbol function_symbol_ProcVarId = atermpp::function_symbol("ProcVarId", 3);
  return function_symbol_ProcVarId;
}

inline
bool gsIsProcVarId(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ProcVarId();
}

// Process
inline
const atermpp::function_symbol& function_symbol_Process()
{
  static atermpp::function_symbol function_symbol_Process = atermpp::function_symbol("Process", 2);
  return function_symbol_Process;
}

inline
bool gsIsProcess(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Process();
}

// ProcessAssignment
inline
const atermpp::function_symbol& function_symbol_ProcessAssignment()
{
  static atermpp::function_symbol function_symbol_ProcessAssignment = atermpp::function_symbol("ProcessAssignment", 2);
  return function_symbol_ProcessAssignment;
}

inline
bool gsIsProcessAssignment(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ProcessAssignment();
}

// ProcessInit
inline
const atermpp::function_symbol& function_symbol_ProcessInit()
{
  static atermpp::function_symbol function_symbol_ProcessInit = atermpp::function_symbol("ProcessInit", 1);
  return function_symbol_ProcessInit;
}

inline
bool gsIsProcessInit(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_ProcessInit();
}

// PropVarDecl
inline
const atermpp::function_symbol& function_symbol_PropVarDecl()
{
  static atermpp::function_symbol function_symbol_PropVarDecl = atermpp::function_symbol("PropVarDecl", 2);
  return function_symbol_PropVarDecl;
}

inline
bool gsIsPropVarDecl(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PropVarDecl();
}

// PropVarInst
inline
const atermpp::function_symbol& function_symbol_PropVarInst()
{
  static atermpp::function_symbol function_symbol_PropVarInst = atermpp::function_symbol("PropVarInst", 3);
  return function_symbol_PropVarInst;
}

inline
bool gsIsPropVarInst(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_PropVarInst();
}

// RegAlt
inline
const atermpp::function_symbol& function_symbol_RegAlt()
{
  static atermpp::function_symbol function_symbol_RegAlt = atermpp::function_symbol("RegAlt", 2);
  return function_symbol_RegAlt;
}

inline
bool gsIsRegAlt(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_RegAlt();
}

// RegNil
inline
const atermpp::function_symbol& function_symbol_RegNil()
{
  static atermpp::function_symbol function_symbol_RegNil = atermpp::function_symbol("RegNil", 0);
  return function_symbol_RegNil;
}

inline
bool gsIsRegNil(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_RegNil();
}

// RegSeq
inline
const atermpp::function_symbol& function_symbol_RegSeq()
{
  static atermpp::function_symbol function_symbol_RegSeq = atermpp::function_symbol("RegSeq", 2);
  return function_symbol_RegSeq;
}

inline
bool gsIsRegSeq(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_RegSeq();
}

// RegTrans
inline
const atermpp::function_symbol& function_symbol_RegTrans()
{
  static atermpp::function_symbol function_symbol_RegTrans = atermpp::function_symbol("RegTrans", 1);
  return function_symbol_RegTrans;
}

inline
bool gsIsRegTrans(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_RegTrans();
}

// RegTransOrNil
inline
const atermpp::function_symbol& function_symbol_RegTransOrNil()
{
  static atermpp::function_symbol function_symbol_RegTransOrNil = atermpp::function_symbol("RegTransOrNil", 1);
  return function_symbol_RegTransOrNil;
}

inline
bool gsIsRegTransOrNil(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_RegTransOrNil();
}

// Rename
inline
const atermpp::function_symbol& function_symbol_Rename()
{
  static atermpp::function_symbol function_symbol_Rename = atermpp::function_symbol("Rename", 2);
  return function_symbol_Rename;
}

inline
bool gsIsRename(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Rename();
}

// RenameExpr
inline
const atermpp::function_symbol& function_symbol_RenameExpr()
{
  static atermpp::function_symbol function_symbol_RenameExpr = atermpp::function_symbol("RenameExpr", 2);
  return function_symbol_RenameExpr;
}

inline
bool gsIsRenameExpr(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_RenameExpr();
}

// Seq
inline
const atermpp::function_symbol& function_symbol_Seq()
{
  static atermpp::function_symbol function_symbol_Seq = atermpp::function_symbol("Seq", 2);
  return function_symbol_Seq;
}

inline
bool gsIsSeq(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Seq();
}

// SetComp
inline
const atermpp::function_symbol& function_symbol_SetComp()
{
  static atermpp::function_symbol function_symbol_SetComp = atermpp::function_symbol("SetComp", 0);
  return function_symbol_SetComp;
}

inline
bool gsIsSetComp(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SetComp();
}

// SortArrow
inline
const atermpp::function_symbol& function_symbol_SortArrow()
{
  static atermpp::function_symbol function_symbol_SortArrow = atermpp::function_symbol("SortArrow", 2);
  return function_symbol_SortArrow;
}

inline
bool gsIsSortArrow(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortArrow();
}

// SortBag
inline
const atermpp::function_symbol& function_symbol_SortBag()
{
  static atermpp::function_symbol function_symbol_SortBag = atermpp::function_symbol("SortBag", 0);
  return function_symbol_SortBag;
}

inline
bool gsIsSortBag(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortBag();
}

// SortCons
inline
const atermpp::function_symbol& function_symbol_SortCons()
{
  static atermpp::function_symbol function_symbol_SortCons = atermpp::function_symbol("SortCons", 2);
  return function_symbol_SortCons;
}

inline
bool gsIsSortCons(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortCons();
}

// SortFBag
inline
const atermpp::function_symbol& function_symbol_SortFBag()
{
  static atermpp::function_symbol function_symbol_SortFBag = atermpp::function_symbol("SortFBag", 0);
  return function_symbol_SortFBag;
}

inline
bool gsIsSortFBag(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortFBag();
}

// SortFSet
inline
const atermpp::function_symbol& function_symbol_SortFSet()
{
  static atermpp::function_symbol function_symbol_SortFSet = atermpp::function_symbol("SortFSet", 0);
  return function_symbol_SortFSet;
}

inline
bool gsIsSortFSet(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortFSet();
}

// SortId
inline
const atermpp::function_symbol& function_symbol_SortId()
{
  static atermpp::function_symbol function_symbol_SortId = atermpp::function_symbol("SortId", 1);
  return function_symbol_SortId;
}

inline
bool gsIsSortId(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortId();
}

// SortList
inline
const atermpp::function_symbol& function_symbol_SortList()
{
  static atermpp::function_symbol function_symbol_SortList = atermpp::function_symbol("SortList", 0);
  return function_symbol_SortList;
}

inline
bool gsIsSortList(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortList();
}

// SortRef
inline
const atermpp::function_symbol& function_symbol_SortRef()
{
  static atermpp::function_symbol function_symbol_SortRef = atermpp::function_symbol("SortRef", 2);
  return function_symbol_SortRef;
}

inline
bool gsIsSortRef(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortRef();
}

// SortSet
inline
const atermpp::function_symbol& function_symbol_SortSet()
{
  static atermpp::function_symbol function_symbol_SortSet = atermpp::function_symbol("SortSet", 0);
  return function_symbol_SortSet;
}

inline
bool gsIsSortSet(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortSet();
}

// SortSpec
inline
const atermpp::function_symbol& function_symbol_SortSpec()
{
  static atermpp::function_symbol function_symbol_SortSpec = atermpp::function_symbol("SortSpec", 1);
  return function_symbol_SortSpec;
}

inline
bool gsIsSortSpec(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortSpec();
}

// SortStruct
inline
const atermpp::function_symbol& function_symbol_SortStruct()
{
  static atermpp::function_symbol function_symbol_SortStruct = atermpp::function_symbol("SortStruct", 1);
  return function_symbol_SortStruct;
}

inline
bool gsIsSortStruct(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_SortStruct();
}

// StateAnd
inline
const atermpp::function_symbol& function_symbol_StateAnd()
{
  static atermpp::function_symbol function_symbol_StateAnd = atermpp::function_symbol("StateAnd", 2);
  return function_symbol_StateAnd;
}

inline
bool gsIsStateAnd(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateAnd();
}

// StateDelay
inline
const atermpp::function_symbol& function_symbol_StateDelay()
{
  static atermpp::function_symbol function_symbol_StateDelay = atermpp::function_symbol("StateDelay", 0);
  return function_symbol_StateDelay;
}

inline
bool gsIsStateDelay(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateDelay();
}

// StateDelayTimed
inline
const atermpp::function_symbol& function_symbol_StateDelayTimed()
{
  static atermpp::function_symbol function_symbol_StateDelayTimed = atermpp::function_symbol("StateDelayTimed", 1);
  return function_symbol_StateDelayTimed;
}

inline
bool gsIsStateDelayTimed(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateDelayTimed();
}

// StateExists
inline
const atermpp::function_symbol& function_symbol_StateExists()
{
  static atermpp::function_symbol function_symbol_StateExists = atermpp::function_symbol("StateExists", 2);
  return function_symbol_StateExists;
}

inline
bool gsIsStateExists(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateExists();
}

// StateFalse
inline
const atermpp::function_symbol& function_symbol_StateFalse()
{
  static atermpp::function_symbol function_symbol_StateFalse = atermpp::function_symbol("StateFalse", 0);
  return function_symbol_StateFalse;
}

inline
bool gsIsStateFalse(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateFalse();
}

// StateForall
inline
const atermpp::function_symbol& function_symbol_StateForall()
{
  static atermpp::function_symbol function_symbol_StateForall = atermpp::function_symbol("StateForall", 2);
  return function_symbol_StateForall;
}

inline
bool gsIsStateForall(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateForall();
}

// StateImp
inline
const atermpp::function_symbol& function_symbol_StateImp()
{
  static atermpp::function_symbol function_symbol_StateImp = atermpp::function_symbol("StateImp", 2);
  return function_symbol_StateImp;
}

inline
bool gsIsStateImp(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateImp();
}

// StateMay
inline
const atermpp::function_symbol& function_symbol_StateMay()
{
  static atermpp::function_symbol function_symbol_StateMay = atermpp::function_symbol("StateMay", 2);
  return function_symbol_StateMay;
}

inline
bool gsIsStateMay(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateMay();
}

// StateMu
inline
const atermpp::function_symbol& function_symbol_StateMu()
{
  static atermpp::function_symbol function_symbol_StateMu = atermpp::function_symbol("StateMu", 3);
  return function_symbol_StateMu;
}

inline
bool gsIsStateMu(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateMu();
}

// StateMust
inline
const atermpp::function_symbol& function_symbol_StateMust()
{
  static atermpp::function_symbol function_symbol_StateMust = atermpp::function_symbol("StateMust", 2);
  return function_symbol_StateMust;
}

inline
bool gsIsStateMust(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateMust();
}

// StateNot
inline
const atermpp::function_symbol& function_symbol_StateNot()
{
  static atermpp::function_symbol function_symbol_StateNot = atermpp::function_symbol("StateNot", 1);
  return function_symbol_StateNot;
}

inline
bool gsIsStateNot(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateNot();
}

// StateNu
inline
const atermpp::function_symbol& function_symbol_StateNu()
{
  static atermpp::function_symbol function_symbol_StateNu = atermpp::function_symbol("StateNu", 3);
  return function_symbol_StateNu;
}

inline
bool gsIsStateNu(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateNu();
}

// StateOr
inline
const atermpp::function_symbol& function_symbol_StateOr()
{
  static atermpp::function_symbol function_symbol_StateOr = atermpp::function_symbol("StateOr", 2);
  return function_symbol_StateOr;
}

inline
bool gsIsStateOr(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateOr();
}

// StateTrue
inline
const atermpp::function_symbol& function_symbol_StateTrue()
{
  static atermpp::function_symbol function_symbol_StateTrue = atermpp::function_symbol("StateTrue", 0);
  return function_symbol_StateTrue;
}

inline
bool gsIsStateTrue(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateTrue();
}

// StateVar
inline
const atermpp::function_symbol& function_symbol_StateVar()
{
  static atermpp::function_symbol function_symbol_StateVar = atermpp::function_symbol("StateVar", 2);
  return function_symbol_StateVar;
}

inline
bool gsIsStateVar(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateVar();
}

// StateYaled
inline
const atermpp::function_symbol& function_symbol_StateYaled()
{
  static atermpp::function_symbol function_symbol_StateYaled = atermpp::function_symbol("StateYaled", 0);
  return function_symbol_StateYaled;
}

inline
bool gsIsStateYaled(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateYaled();
}

// StateYaledTimed
inline
const atermpp::function_symbol& function_symbol_StateYaledTimed()
{
  static atermpp::function_symbol function_symbol_StateYaledTimed = atermpp::function_symbol("StateYaledTimed", 1);
  return function_symbol_StateYaledTimed;
}

inline
bool gsIsStateYaledTimed(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StateYaledTimed();
}

// StructCons
inline
const atermpp::function_symbol& function_symbol_StructCons()
{
  static atermpp::function_symbol function_symbol_StructCons = atermpp::function_symbol("StructCons", 3);
  return function_symbol_StructCons;
}

inline
bool gsIsStructCons(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StructCons();
}

// StructProj
inline
const atermpp::function_symbol& function_symbol_StructProj()
{
  static atermpp::function_symbol function_symbol_StructProj = atermpp::function_symbol("StructProj", 2);
  return function_symbol_StructProj;
}

inline
bool gsIsStructProj(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_StructProj();
}

// Sum
inline
const atermpp::function_symbol& function_symbol_Sum()
{
  static atermpp::function_symbol function_symbol_Sum = atermpp::function_symbol("Sum", 2);
  return function_symbol_Sum;
}

inline
bool gsIsSum(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Sum();
}

// Sync
inline
const atermpp::function_symbol& function_symbol_Sync()
{
  static atermpp::function_symbol function_symbol_Sync = atermpp::function_symbol("Sync", 2);
  return function_symbol_Sync;
}

inline
bool gsIsSync(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Sync();
}

// Tau
inline
const atermpp::function_symbol& function_symbol_Tau()
{
  static atermpp::function_symbol function_symbol_Tau = atermpp::function_symbol("Tau", 0);
  return function_symbol_Tau;
}

inline
bool gsIsTau(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Tau();
}

// UntypedActMultAct
inline
const atermpp::function_symbol& function_symbol_UntypedActMultAct()
{
  static atermpp::function_symbol function_symbol_UntypedActMultAct = atermpp::function_symbol("UntypedActMultAct", 1);
  return function_symbol_UntypedActMultAct;
}

inline
bool gsIsUntypedActMultAct(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_UntypedActMultAct();
}

// UntypedAction
inline
const atermpp::function_symbol& function_symbol_UntypedAction()
{
  static atermpp::function_symbol function_symbol_UntypedAction = atermpp::function_symbol("UntypedAction", 2);
  return function_symbol_UntypedAction;
}

inline
bool gsIsUntypedAction(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_UntypedAction();
}

// UntypedIdentifier
inline
const atermpp::function_symbol& function_symbol_UntypedIdentifier()
{
  static atermpp::function_symbol function_symbol_UntypedIdentifier = atermpp::function_symbol("UntypedIdentifier", 1);
  return function_symbol_UntypedIdentifier;
}

inline
bool gsIsUntypedIdentifier(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_UntypedIdentifier();
}

// UntypedIdentifierAssignment
inline
const atermpp::function_symbol& function_symbol_UntypedIdentifierAssignment()
{
  static atermpp::function_symbol function_symbol_UntypedIdentifierAssignment = atermpp::function_symbol("UntypedIdentifierAssignment", 2);
  return function_symbol_UntypedIdentifierAssignment;
}

inline
bool gsIsUntypedIdentifierAssignment(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_UntypedIdentifierAssignment();
}

// UntypedMultAct
inline
const atermpp::function_symbol& function_symbol_UntypedMultAct()
{
  static atermpp::function_symbol function_symbol_UntypedMultAct = atermpp::function_symbol("UntypedMultAct", 1);
  return function_symbol_UntypedMultAct;
}

inline
bool gsIsUntypedMultAct(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_UntypedMultAct();
}

// UntypedParamId
inline
const atermpp::function_symbol& function_symbol_UntypedParamId()
{
  static atermpp::function_symbol function_symbol_UntypedParamId = atermpp::function_symbol("UntypedParamId", 2);
  return function_symbol_UntypedParamId;
}

inline
bool gsIsUntypedParamId(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_UntypedParamId();
}

// UntypedProcessAssignment
inline
const atermpp::function_symbol& function_symbol_UntypedProcessAssignment()
{
  static atermpp::function_symbol function_symbol_UntypedProcessAssignment = atermpp::function_symbol("UntypedProcessAssignment", 2);
  return function_symbol_UntypedProcessAssignment;
}

inline
bool gsIsUntypedProcessAssignment(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_UntypedProcessAssignment();
}

// UntypedSetBagComp
inline
const atermpp::function_symbol& function_symbol_UntypedSetBagComp()
{
  static atermpp::function_symbol function_symbol_UntypedSetBagComp = atermpp::function_symbol("UntypedSetBagComp", 0);
  return function_symbol_UntypedSetBagComp;
}

inline
bool gsIsUntypedSetBagComp(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_UntypedSetBagComp();
}

// UntypedSortUnknown
inline
const atermpp::function_symbol& function_symbol_UntypedSortUnknown()
{
  static atermpp::function_symbol function_symbol_UntypedSortUnknown = atermpp::function_symbol("UntypedSortUnknown", 0);
  return function_symbol_UntypedSortUnknown;
}

inline
bool gsIsUntypedSortUnknown(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_UntypedSortUnknown();
}

// UntypedSortsPossible
inline
const atermpp::function_symbol& function_symbol_UntypedSortsPossible()
{
  static atermpp::function_symbol function_symbol_UntypedSortsPossible = atermpp::function_symbol("UntypedSortsPossible", 1);
  return function_symbol_UntypedSortsPossible;
}

inline
bool gsIsUntypedSortsPossible(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_UntypedSortsPossible();
}

// Whr
inline
const atermpp::function_symbol& function_symbol_Whr()
{
  static atermpp::function_symbol function_symbol_Whr = atermpp::function_symbol("Whr", 2);
  return function_symbol_Whr;
}

inline
bool gsIsWhr(const atermpp::aterm_appl& Term)
{
  return Term.function() == function_symbol_Whr();
}
//--- end generated code ---//

}
}
}

#endif // MCRL2_LIBSTRUCT_CORE_H
