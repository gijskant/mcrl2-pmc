// Author(s): Aad Mathijssen
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http:///www.boost.org/LICENSE_1_0.txt)
//
/// \file struct.h
/// \brief Library for operations defined on the ATerm structure.
///
/// Global precondition: the ATerm library has been initialised

#ifndef MCRL2_LIBSTRUCT_H
#define MCRL2_LIBSTRUCT_H

// This file describes the functions that can be used for the internal ATerm
// structure.

#include "mcrl2/core/detail/struct_core.h"

#ifndef __cplusplus
#include <stdbool.h>
#endif
#include <aterm2.h>

namespace mcrl2 {
  namespace core {


// Sort expressions
// ----------------

///\pre Term is not NULL
///\return Term is a sort expression
bool gsIsSortExpr(ATermAppl Term);

///\pre Term is not NULL
///\return Term is either SortUnknown or SortsPossible
bool gsIsNotInferred(ATermAppl Term);

//Creation of names for system sort identifiers

///Creation of name for the system sort identifier "Bool"
ATermAppl gsMakeSortIdNameBool();
///Creation of name for the system sort identifier "Pos"
ATermAppl gsMakeSortIdNamePos();
///Creation of name for the system sort identifier "Nat"
ATermAppl gsMakeSortIdNameNat();
///Creation of name for the system sort identifier "NatPair"
ATermAppl gsMakeSortIdNameNatPair();
///Creation of name for the system sort identifier "Int"
ATermAppl gsMakeSortIdNameInt();
///Creation of name for the system sort identifier "Real"
ATermAppl gsMakeSortIdNameReal();


///Creation of sort identifiers for system defined sorts

///\return Sort identifier for `Bool'
ATermAppl gsMakeSortIdBool(void);

///\return Sort identifier for `Pos'
ATermAppl gsMakeSortIdPos(void);

///\return Sort identifier for `Nat'
ATermAppl gsMakeSortIdNat(void);

///\return Sort identifier for `NatPair'
ATermAppl gsMakeSortIdNatPair(void);

///\return Sort identifier for `Int'
ATermAppl gsMakeSortIdInt(void);

///\return Sort identifier for `Real'
ATermAppl gsMakeSortIdReal(void);


///Creation of sort expressions for system defined sorts

///\return Sort expression for `Bool'
ATermAppl gsMakeSortExprBool(void);

///\return Sort expression for `Pos'
ATermAppl gsMakeSortExprPos(void);

///\return Sort expression for `Nat'
ATermAppl gsMakeSortExprNat(void);

///\return Sort expression for `NatPair'
ATermAppl gsMakeSortExprNatPair(void);

///\return Sort expression for `Int'
ATermAppl gsMakeSortExprInt(void);

///\return Sort expression for `Real'
ATermAppl gsMakeSortExprReal(void);

///\return Sort expression for `List of SortExpr'
ATermAppl gsMakeSortExprList(ATermAppl SortExpr);

///\return Sort expression for `Set of SortExpr'
ATermAppl gsMakeSortExprSet(ATermAppl SortExpr);

///\return Sort expression for `Bag of SortExpr'
ATermAppl gsMakeSortExprBag(ATermAppl SortExpr);

///\return Term is a SortExprList
bool gsIsSortExprList(ATermAppl Term);

///\return Term is a SortExprSet
bool gsIsSortExprSet(ATermAppl Term);

///\return Term is a SortExprBag
bool gsIsSortExprBag(ATermAppl Term);

// Auxiliary functions concerning sort expressions

///\pre SortExprDom and SortExprResult are sort expressions
///\return Internal representation of the sort expression SortExprDom -> SortExprResult
ATermAppl gsMakeSortArrow1(ATermAppl SortExprDom, ATermAppl SortExprResult);

///\pre SortExprDom1, SortExprDom2 and SortExprResult are sort expressions
///\return Internal representation of the sort expression
///     SortExprDom1 -> SortExprDom2 -> SortExprResult, where -> is right
///     associative.
ATermAppl gsMakeSortArrow2(ATermAppl SortExprDom1, ATermAppl SortExprDom2,
  ATermAppl SortExprResult);

///\pre SortExprDom1, SortExprDom2, SortExprDom3 and SortExprResult are sort
///     expressions
///\return Internal representation of the sort expression
///     SortExprDom1 -> SortExprDom2 -> SortExprDom3 -> SortExprResult,
///     where -> is right associative.
ATermAppl gsMakeSortArrow3(ATermAppl SortExprDom1, ATermAppl SortExprDom2,
  ATermAppl SortExprDom3, ATermAppl SortExprResult);

///\pre SortExprDom1, SortExprDom2, SortExprDom3, SortExprDom4 and
///     SortExprResult are sort expressions
///\return Internal representation of the sort expression
///     SortExprDom1 -> SortExprDom2 -> SortExprDom3 -> SortExprDom4 ->
///       SortExprResult, where -> is right associative.
ATermAppl gsMakeSortArrow4(ATermAppl SortExprDom1, ATermAppl SortExprDom2,
  ATermAppl SortExprDom3, ATermAppl SortExprDom4, ATermAppl SortExprResult);

/// - Pre SortExprs is a list of sort expressions, SortExpr is a sort expression
///\return Internal representation of the sort expression.
ATermAppl gsMakeSortArrowList(ATermList SortExprs, ATermAppl SortExprResult);

///\pre SortExpr is a sort expression
///\return the result of the sort expression
ATermAppl gsGetSortExprResult(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return the domain of the sort expression
ATermList gsGetSortExprDomain(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return A list with the domains of the sort expression,
///     so if we have a sort expression of the form
///     AxB->(CxD->E), we return [[A,B],[C,D]]
ATermList gsGetSortExprDomains(ATermAppl SortExpr);

// Data expressions
// ----------------

///\pre Term is not NULL
///\return Term is a data expression
bool gsIsDataExpr(ATermAppl Term);

///\pre DataExpr is an OpId or a DataVarId
///\return The name of the data expression
ATermAppl gsGetName(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the sort of DataExpr, if the sort can be inferred from the sort
///     information in DataExpr
///     Unknown, otherwise
ATermAppl gsGetSort(ATermAppl DataExpr);

///\pre DataExprs is a list of data expressions
///\return the list of sorts belonging to DataExprs
ATermList gsGetSorts(ATermList DataExprs);

///\pre DataExpr is a data expression
///\return the head of the data expression
ATermAppl gsGetDataExprHead(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the arguments of the data expression
ATermList gsGetDataExprArgs(ATermAppl DataExpr);

///Creation of name for the system operation identifier "True"
ATermAppl gsMakeOpIdNameTrue();
///Creation of name for the system operation identifier "False"
ATermAppl gsMakeOpIdNameFalse();
///Creation of name for the system operation identifier "Not"
ATermAppl gsMakeOpIdNameNot();
///Creation of name for the system operation identifier "And"
ATermAppl gsMakeOpIdNameAnd();
///Creation of name for the system operation identifier "Or"
ATermAppl gsMakeOpIdNameOr();
///Creation of name for the system operation identifier "Implies"
ATermAppl gsMakeOpIdNameImp();
///Creation of name for the system operation identifier "Equal"
ATermAppl gsMakeOpIdNameEq();
///Creation of name for the system operation identifier "Not Equal"
ATermAppl gsMakeOpIdNameNeq();
///Creation of name for the system operation identifier "If"
ATermAppl gsMakeOpIdNameIf();
///Creation of name for the system operation identifier "Forall"
ATermAppl gsMakeOpIdNameForall();
///Creation of name for the system operation identifier "Exists"
ATermAppl gsMakeOpIdNameExists();
///Creation of name for the system operation identifier "C1"
ATermAppl gsMakeOpIdNameC1();
///Creation of name for the system operation identifier "CDub"
ATermAppl gsMakeOpIdNameCDub();
///Creation of name for the system operation identifier "C0"
ATermAppl gsMakeOpIdNameC0();
///Creation of name for the system operation identifier "CNat"
ATermAppl gsMakeOpIdNameCNat();
///Creation of name for the system operation identifier "CPair"
ATermAppl gsMakeOpIdNameCPair();
///Creation of name for the system operation identifier "CNeg"
ATermAppl gsMakeOpIdNameCNeg();
///Creation of name for the system operation identifier "CInt"
ATermAppl gsMakeOpIdNameCInt();
///Creation of name for the system operation identifier "CReal"
ATermAppl gsMakeOpIdNameCReal();
///Creation of name for the system operation identifier "Pos2Nat"
ATermAppl gsMakeOpIdNamePos2Nat();
///Creation of name for the system operation identifier "Pos2Int"
ATermAppl gsMakeOpIdNamePos2Int();
///Creation of name for the system operation identifier "Pos2Real"
ATermAppl gsMakeOpIdNamePos2Real();
///Creation of name for the system operation identifier "Nat2Pos"
ATermAppl gsMakeOpIdNameNat2Pos();
///Creation of name for the system operation identifier "Nat2Int"
ATermAppl gsMakeOpIdNameNat2Int();
///Creation of name for the system operation identifier "Nat2Real"
ATermAppl gsMakeOpIdNameNat2Real();
///Creation of name for the system operation identifier "Int2Pos"
ATermAppl gsMakeOpIdNameInt2Pos();
///Creation of name for the system operation identifier "Int2Nat"
ATermAppl gsMakeOpIdNameInt2Nat();
///Creation of name for the system operation identifier "Int2Real"
ATermAppl gsMakeOpIdNameInt2Real();
///Creation of name for the system operation identifier "Real2Pos"
ATermAppl gsMakeOpIdNameReal2Pos();
///Creation of name for the system operation identifier "Real2Nat"
ATermAppl gsMakeOpIdNameReal2Nat();
///Creation of name for the system operation identifier "Real2Int"
ATermAppl gsMakeOpIdNameReal2Int();
///Creation of name for the system operation identifier "LTE"
ATermAppl gsMakeOpIdNameLTE();
///Creation of name for the system operation identifier "LT"
ATermAppl gsMakeOpIdNameLT();
///Creation of name for the system operation identifier "GTE"
ATermAppl gsMakeOpIdNameGTE();
///Creation of name for the system operation identifier "GT"
ATermAppl gsMakeOpIdNameGT();
///Creation of name for the system operation identifier "Mat"
ATermAppl gsMakeOpIdNameMax();
///Creation of name for the system operation identifier "Min"
ATermAppl gsMakeOpIdNameMin();
///Creation of name for the system operation identifier "Abs"
ATermAppl gsMakeOpIdNameAbs();
///Creation of name for the system operation identifier "Neg"
ATermAppl gsMakeOpIdNameNeg();
///Creation of name for the system operation identifier "Succ"
ATermAppl gsMakeOpIdNameSucc();
///Creation of name for the system operation identifier "Pred"
ATermAppl gsMakeOpIdNamePred();
///Creation of name for the system operation identifier "Dub"
ATermAppl gsMakeOpIdNameDub();
///Creation of name for the system operation identifier "Add"
ATermAppl gsMakeOpIdNameAdd();
///Creation of name for the system operation identifier "AddC"
ATermAppl gsMakeOpIdNameAddC();
///Creation of name for the system operation identifier "Subt"
ATermAppl gsMakeOpIdNameSubt();
///Creation of name for the system operation identifier "GTESubt"
ATermAppl gsMakeOpIdNameGTESubt();
///Creation of name for the system operation identifier "GTESubtB"
ATermAppl gsMakeOpIdNameGTESubtB();
///Creation of name for the system operation identifier "Mult"
ATermAppl gsMakeOpIdNameMult();
///Creation of name for the system operation identifier "MultIR"
ATermAppl gsMakeOpIdNameMultIR();
///Creation of name for the system operation identifier "Div"
ATermAppl gsMakeOpIdNameDiv();
///Creation of name for the system operation identifier "Mod"
ATermAppl gsMakeOpIdNameMod();
///Creation of name for the system operation identifier "First"
ATermAppl gsMakeOpIdNameFirst();
///Creation of name for the system operation identifier "Last" 
ATermAppl gsMakeOpIdNameLast();
///Creation of name for the system operation identifier "DivMod"
ATermAppl gsMakeOpIdNameDivMod();
///Creation of name for the system operation identifier "GDivMod"
ATermAppl gsMakeOpIdNameGDivMod();
///Creation of name for the system operation identifier "GGDivMod"
ATermAppl gsMakeOpIdNameGGDivMod();
///Creation of name for the system operation identifier "Exp"
ATermAppl gsMakeOpIdNameExp();
///Creation of name for the system operation identifier "Even"
ATermAppl gsMakeOpIdNameEven();
///Creation of name for the system operation identifier "EmptyList"
ATermAppl gsMakeOpIdNameEmptyList();
///Creation of name for the system operation identifier "ListEnum"
ATermAppl gsMakeOpIdNameListEnum();
///Creation of name for the system operation identifier "ListSize"
ATermAppl gsMakeOpIdNameListSize();
///Creation of name for the system operation identifier "Cons"
ATermAppl gsMakeOpIdNameCons();
///Creation of name for the system operation identifier "Snoc"
ATermAppl gsMakeOpIdNameSnoc();
///Creation of name for the system operation identifier "Concat"
ATermAppl gsMakeOpIdNameConcat();
///Creation of name for the system operation identifier "EltAt"
ATermAppl gsMakeOpIdNameEltAt();
///Creation of name for the system operation identifier "Head"
ATermAppl gsMakeOpIdNameHead();
///Creation of name for the system operation identifier "Tail"
ATermAppl gsMakeOpIdNameTail();
///Creation of name for the system operation identifier "Rhead"
ATermAppl gsMakeOpIdNameRHead();
///Creation of name for the system operation identifier "RTail"
ATermAppl gsMakeOpIdNameRTail();
///Creation of name for the system operation identifier "EltIn"
ATermAppl gsMakeOpIdNameEltIn();
///Creation of name for the system operation identifier "SetComp"
ATermAppl gsMakeOpIdNameSetComp();
///Creation of name for the system operation identifier "EmptySet"
ATermAppl gsMakeOpIdNameEmptySet();
///Creation of name for the system operation identifier "SetEnum"
ATermAppl gsMakeOpIdNameSetEnum();
///Creation of name for the system operation identifier "SubSetEq"
ATermAppl gsMakeOpIdNameSubSetEq();
///Creation of name for the system operation identifier "SubSet"
ATermAppl gsMakeOpIdNameSubSet();
///Creation of name for the system operation identifier "SetUnion"
ATermAppl gsMakeOpIdNameSetUnion();
///Creation of name for the system operation identifier "SetDiff"
ATermAppl gsMakeOpIdNameSetDiff();
///Creation of name for the system operation identifier "SetIntersect"
ATermAppl gsMakeOpIdNameSetIntersect();
///Creation of name for the system operation identifier "SetCompl"
ATermAppl gsMakeOpIdNameSetCompl();
///Creation of name for the system operation identifier "BagComp"
ATermAppl gsMakeOpIdNameBagComp();
///Creation of name for the system operation identifier "Bag2Set"
ATermAppl gsMakeOpIdNameBag2Set();
///Creation of name for the system operation identifier "Set2Bag"
ATermAppl gsMakeOpIdNameSet2Bag();
///Creation of name for the system operation identifier "EmptyBag"
ATermAppl gsMakeOpIdNameEmptyBag();
///Creation of name for the system operation identifier "BagEnum"
ATermAppl gsMakeOpIdNameBagEnum();
///Creation of name for the system operation identifier "Count"
ATermAppl gsMakeOpIdNameCount();
///Creation of name for the system operation identifier "SubBagEq"
ATermAppl gsMakeOpIdNameSubBagEq();
///Creation of name for the system operation identifier "SubBag"
ATermAppl gsMakeOpIdNameSubBag();
///Creation of name for the system operation identifier "BagUnion"
ATermAppl gsMakeOpIdNameBagUnion();
///Creation of name for the system operation identifier "BagDiff"
ATermAppl gsMakeOpIdNameBagDiff();
///Creation of name for the system operation identifier "BagIntersect"
ATermAppl gsMakeOpIdNameBagIntersect();

// Creation of operation identifiers for system defined operations.

///\return Operation identifier for `true'
ATermAppl gsMakeOpIdTrue(void);

///\return Operation identifier for `false'
ATermAppl gsMakeOpIdFalse(void);

///\return Operation identifier for logical negation
ATermAppl gsMakeOpIdNot(void);

///\return Operation identifier for conjunction
ATermAppl gsMakeOpIdAnd(void);

///\return Operation identifier for disjunction
ATermAppl gsMakeOpIdOr(void);

///\return Operation identifier for implication
ATermAppl gsMakeOpIdImp(void);

///\pre SortExpr is a sort expression
///\return Operation identifier for the equality of terms of sort SortExpr
ATermAppl gsMakeOpIdEq(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for the inequality of terms of sort SortExpr
ATermAppl gsMakeOpIdNeq(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for the conditional of terms of sort SortExpr
ATermAppl gsMakeOpIdIf(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for the universal quantification over sort SortExpr
ATermAppl gsMakeOpIdForall(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for the existential quantification over sort
///     SortExpr
ATermAppl gsMakeOpIdExists(ATermAppl SortExpr);

///\return Operation identifier for the Pos constructor `1'
ATermAppl gsMakeOpIdC1(void);

///\return Operation identifier for the Pos constructor `double and add a bit'
ATermAppl gsMakeOpIdCDub(void);

///\return Operation identifier for the Nat constructor `0'
ATermAppl gsMakeOpIdC0(void);

///\return Operation identifier for the creation of a natural number from a
///     positive number
ATermAppl gsMakeOpIdCNat(void);

///\return Operation identifier for the creation of a pair of natural numbers
ATermAppl gsMakeOpIdCPair(void);

///\return Operation identifier for the negation of a positive number
ATermAppl gsMakeOpIdCNeg(void);

///\return Operation identifier for the creation of an integer from a natural
///     number
ATermAppl gsMakeOpIdCInt(void);

///\return Operation identifier for the creation of a real from an integer
ATermAppl gsMakeOpIdCReal(void);

///\return Operation identifier for the conversion of Pos to Nat
ATermAppl gsMakeOpIdPos2Nat(void);

///\return Operation identifier for the conversion of Pos to Int
ATermAppl gsMakeOpIdPos2Int(void);

///\return Operation identifier for the conversion of Pos to Real
ATermAppl gsMakeOpIdPos2Real(void);

///\return Operation identifier for the conversion of Nat to Pos
ATermAppl gsMakeOpIdNat2Pos(void);

///\return Operation identifier for the conversion of Nat to Int
ATermAppl gsMakeOpIdNat2Int(void);

///\return Operation identifier for the conversion of Nat to Real
ATermAppl gsMakeOpIdNat2Real(void);

///\return Operation identifier for the conversion of Int to Pos
ATermAppl gsMakeOpIdInt2Pos(void);

///\return Operation identifier for the conversion of Int to Nat
ATermAppl gsMakeOpIdInt2Nat(void);

///\return Operation identifier for the conversion of Int to Real
ATermAppl gsMakeOpIdInt2Real(void);

///\return Operation identifier for the conversion of Real to Pos
ATermAppl gsMakeOpIdReal2Pos(void);

///\return Operation identifier for the conversion of Real to Nat
ATermAppl gsMakeOpIdReal2Nat(void);

///\return Operation identifier for the conversion of Real to Int
ATermAppl gsMakeOpIdReal2Int(void);

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for `less than or equal' on SortExpr
ATermAppl gsMakeOpIdLTE(ATermAppl SortExpr);

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for `less than' on SortExpr
ATermAppl gsMakeOpIdLT(ATermAppl SortExpr);

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for `greater than or equal' on SortExpr
ATermAppl gsMakeOpIdGTE(ATermAppl SortExpr);

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for `greater than' on SortExpr
ATermAppl gsMakeOpIdGT(ATermAppl SortExpr);

///\pre SortExprLHS, SortExprRHS is Pos, Nat or Int, or SortExprLHS and
///     SortExprRHS are both Real
///\return Operation identifier for `maximum' on SortExprLHS and SortExprRHS. The
///     result sort is the intersection of SortExprLHS and SortExprRHS.
ATermAppl gsMakeOpIdMax(ATermAppl SortExprLHS, ATermAppl SortExprRHS);

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for `minimum' on SortExpr
ATermAppl gsMakeOpIdMin(ATermAppl SortExpr);

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for `absolute value' on SortExpr. The result sort
///     is:
///     - SortExpr, if SortExpr is Pos, Nat or Real
///     - Nat, if SortExpr is Int
ATermAppl gsMakeOpIdAbs(ATermAppl SortExpr);

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for negation. The result sort is union of SortExpr
///     and Int.
ATermAppl gsMakeOpIdNeg(ATermAppl SortExpr);

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for the successor with argument sort SortExpr
///     The corresponding result sort is:
///     - Pos, if SortExpr is Nat
///     - SortExpr, otherwise
ATermAppl gsMakeOpIdSucc(ATermAppl SortExpr);

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for the predecessor with argument sort SortExpr
///     The corresponding result sort is:
///     - Nat, if SortExpr is Pos
///     - the union of SortExpr and Int, otherwise
ATermAppl gsMakeOpIdPred(ATermAppl SortExpr);

///\pre SortExpr is Pos, Nat or Int
///\return Operation identifier for '2*n + |b|', where n is of sort SortExpr and
///     b is of sort Bool
ATermAppl gsMakeOpIdDub(ATermAppl SortExpr);

///\pre SortExprLHS, SortExprRHS is Pos or Nat, or SortExprLHS and SortExprRHS
///     are both equal to Int or Real.
///\return Operation identifier for addition on SortExprLHS and SortExprRHS which
///     has the intersection of SortExprLHS and SortExprRHS as a result sort.
ATermAppl gsMakeOpIdAdd(ATermAppl SortExprLHS, ATermAppl SortExprRHS);

///\return Operation identifier for the addition of two positive numbers and a
///     (carry) bit, which is of sort Bool -> Pos -> Pos -> Pos
ATermAppl gsMakeOpIdAddC();

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for subtraction on SortExpr, which has the union of
///     SortExpr and Int as a result sort.
ATermAppl gsMakeOpIdSubt(ATermAppl SortExpr);

///\pre SortExpr is Pos or Nat
///\return Operation identifier for subtraction 'x - y', where x >= y and x,y are
///     both of sort Pos or Nat
///     The identifier is of sort -> SortExpr -> SortExpr -> Nat
ATermAppl gsMakeOpIdGTESubt(ATermAppl SortExpr);

///\return Operation identifier for 'p - (q + |b|)', i.e. subtraction with borrow,
///     where p >= q + |b|.
///     The identifier is of sort Bool -> Pos -> Pos -> Nat
ATermAppl gsMakeOpIdGTESubtB();

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for multiplication on SortExpr
ATermAppl gsMakeOpIdMult(ATermAppl SortExpr);

///\return Operation identifier for multiplication of two positive numbers, plus an
///     optional intermediate positive result, which is of sort
///     Bool -> Pos -> Pos -> Pos -> Pos
ATermAppl gsMakeOpIdMultIR();

///\pre SortExpr is Pos, Nat or Int
///\return Operation identifier for `quotient after division', which has sort
///     SortExpr -> Pos -> S', where S' stands for the union of SortExpr and Nat
ATermAppl gsMakeOpIdDiv(ATermAppl SortExpr);

///\pre SortExpr is Pos, Nat or Int
///\return Operation identifier for `remainder after division', which has sort
///     SortExpr -> Pos -> Nat
ATermAppl gsMakeOpIdMod(ATermAppl SortExpr);

///\return Operation identifier for `quotient and remainder after division',
///     which has sort Pos -> Pos -> NatPair
///     Specification:
///     divmod(p, q) = <p div q, p mod q>
ATermAppl gsMakeOpIdDivMod();

///\return Operation identifier for `generalised quotient and remainder after division',
///     which has sort NatPair -> Bool -> Pos -> NatPair
///     Specification:
///       gdivmod(< m, n >, b, p)  =  if  l <  p  ->  < 2*m  , l >
///                                   []  l >= p  ->  < 2*m+1, l-p >
///                                   fi whr l = 2*n + b end
ATermAppl gsMakeOpIdGDivMod();

///\return Operation identifier for `generalised generalised quotient and remainder after division',
///     which has sort Nat -> Nat -> Pos -> NatPair
///     Specification:
///       ggdivmod(m, n, p)  =  if  m <  p  ->  < 2*n  , m >
///                             []  m >= p  ->  < 2*n+1, m-p >
///                             fi
ATermAppl gsMakeOpIdGGDivMod();

///\return Operation identifier for 'first' of sort NatPair -> Nat
ATermAppl gsMakeOpIdFirst();

///\return Operation identifier for 'last' of sort NatPair -> Nat
ATermAppl gsMakeOpIdLast();

///\pre SortExpr is Pos, Nat, Int or Real
///\return Operation identifier for exponentiation, which has sort S -> Nat -> S,
///     where S stands for SortExpr
ATermAppl gsMakeOpIdExp(ATermAppl SortExpr);

///\return Operation identifier for 'even' of sort Nat -> Bool
ATermAppl gsMakeOpIdEven();

///\pre SortExpr is a sort expression
///\return Operation identifier for the empty list of sort SortExpr
ATermAppl gsMakeOpIdEmptyList(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for the list enum of sort SortExpr
ATermAppl gsMakeOpIdListEnum(ATermAppl SortExpr);

///\pre SortExprLHS and SortExprRHS are sort expressions
///\return Operation identifier for 'list cons (element at the head of a list)' of
///     sort S -> T -> T, where S and T stand for SortExprLHS and SortExprRHS
ATermAppl gsMakeOpIdCons(ATermAppl SortExprLHS, ATermAppl SortExprRHS);

///\pre SortExpr is a sort expression
///\return Operation identifier for list size of sort SortExpr -> Nat
ATermAppl gsMakeOpIdListSize(ATermAppl SortExpr);

///\pre SortExprLHS and SortExprRHS are a sort expressions
///\return Operation identifier for 'list snoc (element at the tail of a list' of
///     sort S -> T -> S, where S and T stand for SortExprLHS and SortExprRHS
ATermAppl gsMakeOpIdSnoc(ATermAppl SortExprLHS, ATermAppl SortExprRHS);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'list concatenation' of sort S -> S -> S, where
///     S stands for SortExpr
ATermAppl gsMakeOpIdConcat(ATermAppl SortExpr);

///\pre SortExprDom and SortExprResult are sort expressions
///\return Operation identifier for 'element at position', which has sort
///     S -> Nat -> T, where S and T stand for SortExprDom and SortExprResult
ATermAppl gsMakeOpIdEltAt(ATermAppl SortExprDom, ATermAppl SortExprResult);

///\pre SortExprDom and SortExprResult are sort expressions
///\return Operation identifier for 'head', which has sort S -> T, where S and
///     T stand for SortExprLHS and SortExprRHS
ATermAppl gsMakeOpIdHead(ATermAppl SortExprDom, ATermAppl SortExprResult);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'tail', which has sort S -> S, where S
///     stands for SortExpr
ATermAppl gsMakeOpIdTail(ATermAppl SortExpr);

///\pre SortExprDom and SortExprResult are sort expressions
///\return Operation identifier for 'right head', which has sort S -> T, where S
///     and T stand for SortExprLHS and SortExprRHS
ATermAppl gsMakeOpIdRHead(ATermAppl SortExprDom, ATermAppl SortExprResult);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'right tail', which has sort S -> S, where S
///     stands for SortExpr
ATermAppl gsMakeOpIdRTail(ATermAppl SortExpr);

///\pre SortExprLHS and SortExprRHS are sort expressions
///\return Operation identifier for 'element test', which has sort
///     S -> T -> Bool, where S and T stand for SortExprLHS and SortExprRHS
ATermAppl gsMakeOpIdEltIn(ATermAppl SortExprLHS, ATermAppl SortExprRHS);

///\pre SortExprDom and SortExprResult are sort expressions
///\return Operation identifier for set comprehension of sort (S -> Bool) -> T,
///     where S and T stand for SortExprDom and SortExprResult
ATermAppl gsMakeOpIdSetComp(ATermAppl SortExprDom, ATermAppl SortExprResult);
     
///\pre SortExpr is a sort expression
///\return Operation identifier for the empty set of sort SortExpr
ATermAppl gsMakeOpIdEmptySet(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for the set enumeration of sort SortExpr
ATermAppl gsMakeOpIdSetEnum(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'subset or equal', which has sort
///     S -> S -> Bool, where S stands for SortExpr
ATermAppl gsMakeOpIdSubSetEq(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'proper subset', which has sort S -> S -> Bool,
///     where S stands for SortExpr
ATermAppl gsMakeOpIdSubSet(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'set union', which has sort S -> S -> S,
///     where S stands for SortExpr
ATermAppl gsMakeOpIdSetUnion(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'set difference', which has sort S -> S -> S,
///     where S stands for SortExpr
ATermAppl gsMakeOpIdSetDiff(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'set intersection', which has sort S -> S -> S,
///     where S stands for SortExpr
ATermAppl gsMakeOpIdSetIntersect(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'set complement', which has sort S -> S,
///     where S stands for SortExpr
ATermAppl gsMakeOpIdSetCompl(ATermAppl SortExpr);

///\pre SortExprDom and SortExprResult are sort expressions
///\return Operation identifier for bag comprehension of sort (S -> Nat) -> T,
///     where S and T stand for SortExprDom and SortExprResult
ATermAppl gsMakeOpIdBagComp(ATermAppl SortExprDom, ATermAppl SortExprResult);

///\pre SortExpr is a sort expression
///\return Operation identifier for the empty bag of sort SortExpr
ATermAppl gsMakeOpIdEmptyBag(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for the bag enumeration of sort SortExpr
ATermAppl gsMakeOpIdBagEnum(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'bag multiplicity', which has sort
///     S -> T -> Nat, where S and T stand for SortExprLHS and SortExprRHS
ATermAppl gsMakeOpIdCount(ATermAppl SortExprLHS, ATermAppl SortExprRHS);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'subbag or equal', which has sort
///     S -> S -> Bool, where S stands for SortExpr
ATermAppl gsMakeOpIdSubBagEq(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'proper subbag', which has sort S -> S -> Bool,
///     where S stands for SortExpr
ATermAppl gsMakeOpIdSubBag(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'bag union', which has sort S -> S -> S,
///     where S stands for SortExpr
ATermAppl gsMakeOpIdBagUnion(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'bag difference', which has sort S -> S -> S,
///     where S stands for SortExpr
ATermAppl gsMakeOpIdBagDiff(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Operation identifier for 'bag intersection', which has sort S -> S -> S,
///     where S stands for SortExpr
ATermAppl gsMakeOpIdBagIntersect(ATermAppl SortExpr);

///\pre SortExprDom and SortExprResult are sort expressions
///\return Operation identifier for bag to set conversion of sort S -> T,
///     where S and T stand for SortExprDom and SortExprResult
ATermAppl gsMakeOpIdBag2Set(ATermAppl SortExprDom,
  ATermAppl SortExprResult);
     
///\pre SortExprDom and SortExprResult are sort expressions
///\return Operation identifier for set to bag conversion of sort S -> T,
///     where S and T stand for SortExprDom and SortExprResult
ATermAppl gsMakeOpIdSet2Bag(ATermAppl SortExprDom,
  ATermAppl SortExprResult);
     

// Creation of data expressions for system defined operations.

///\return Data expression for `true'
ATermAppl gsMakeDataExprTrue(void);

///\return Data expression for `false'
ATermAppl gsMakeDataExprFalse(void);

///\pre DataExpr is a data expression
///\return Data expression for the negation of DataExpr
ATermAppl gsMakeDataExprNot(ATermAppl DataExpr);

///\pre DataExprLHS and DataExprRHS are data expressions
///\return Data expression for the conjunction of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprAnd(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions
///\return Data expression for the disjunction of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprOr(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions
///\return Data expression for `DataExprLHS implies DataExprRHS'
ATermAppl gsMakeDataExprImp(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExpr is a data expression of sort S -> Bool
///\return Data expression for the universal quantification over DataExpr
ATermAppl gsMakeDataExprForall(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort S -> Bool
///\return Data expression for the existential quantification over DataExpr
ATermAppl gsMakeDataExprExists(ATermAppl DataExpr);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort, and
///     must be different from Unknown
///\return Data expression for the equality of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprEq(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort, and
///     must be different from Unknown
///\return Data expression for the inequality of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprNeq(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprCond is a data expression of sort Bool
///     DataExprThen and DataExprElse are data expressions of the same sort, and
///     must be different from Unknown
///\return Data expression for `if(DataExprCond, DataExprThen, DataExprElse)'
ATermAppl gsMakeDataExprIf(ATermAppl DataExprCond, ATermAppl DataExprThen,
  ATermAppl DataExprElse);

///\return Data expression for `1' of sort Pos
ATermAppl gsMakeDataExprC1(void);

///\pre DataExprBit and DataExprPos are data expressions of sort Bool and Pos,
///     respectively, which we denote by b and p
///\return Data expression for `2*p + |b|', where '|b|' is defined as:
///       |true|  = 1
///       |false| = 0
ATermAppl gsMakeDataExprCDub(ATermAppl DataExprBit, ATermAppl DataExprPos);

///\return Data expression for `0' of sort Nat
ATermAppl gsMakeDataExprC0(void);

///\pre DataExpr is a data expression of sort Pos 
///\return DataExpr as a data expression of sort Nat
ATermAppl gsMakeDataExprCNat(ATermAppl DataExpr);

///\pre DataExprFst and DataExprLst are data expressions of sort Nat
///\return Data expression for the pair of DataExprFst and DataExprLst
ATermAppl gsMakeDataExprCPair(ATermAppl DataExprFst, ATermAppl DataExprLst);

///\pre DataExpr is a data expression of sort Pos 
///\return Data expression for the negation of DataExpr
ATermAppl gsMakeDataExprCNeg(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Nat 
///\return DataExpr as a data expression of sort Int
ATermAppl gsMakeDataExprCInt(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Int
///\return DataExpr as a data expression of sort Real
ATermAppl gsMakeDataExprCReal(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Pos 
///\return Data expression for the conversion of DataExpr to Nat
ATermAppl gsMakeDataExprPos2Nat(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Pos 
///\return Data expression for the conversion of DataExpr to Int
ATermAppl gsMakeDataExprPos2Int(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Pos 
///\return Data expression for the conversion of DataExpr to Real
ATermAppl gsMakeDataExprPos2Real(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Nat 
///\return Data expression for the conversion of DataExpr to Pos
ATermAppl gsMakeDataExprNat2Pos(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Nat 
///\return Data expression for the conversion of DataExpr to Int
ATermAppl gsMakeDataExprNat2Int(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Nat 
///\return Data expression for the conversion of DataExpr to Real
ATermAppl gsMakeDataExprNat2Real(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Int 
///\return Data expression for the conversion of DataExpr to Pos
ATermAppl gsMakeDataExprInt2Pos(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Int 
///\return Data expression for the conversion of DataExpr to Nat
ATermAppl gsMakeDataExprInt2Nat(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Int 
///\return Data expression for the conversion of DataExpr to Real
ATermAppl gsMakeDataExprInt2Real(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Real 
///\return Data expression for the conversion of DataExpr to Pos
ATermAppl gsMakeDataExprReal2Pos(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Real
///\return Data expression for the conversion of DataExpr to Nat
ATermAppl gsMakeDataExprReal2Nat(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Real
///\return Data expression for the conversion of DataExpr to Int
ATermAppl gsMakeDataExprReal2Int(ATermAppl DataExpr);

///\pre DataExprLHS and DataExprRHS are both data expressions of sort Pos, Nat
///     or Int
///\return Data expression for the `less than or equal' of DataExprLHS and
///     DataExprRHS
ATermAppl gsMakeDataExprLTE(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are both data expressions of sort Pos, Nat
///     or Int
///\return Data expression for the `less than' of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprLT(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are both data expressions of sort Pos, Nat
///     or Int
///\return Data expression for the `greater than or equal' of DataExprLHS and
///     DataExprRHS
ATermAppl gsMakeDataExprGTE(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are both data expressions of sort Pos, Nat
///     or Int
///\return Data expression for the `greater than' of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprGT(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are both data expressions of sort Pos, Nat
///     or Int
///\return Data expression for the maximum of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprMax(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are both data expressions of sort Pos, Nat
///     or Int
///\return Data expression for the minimum of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprMin(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExpr is a data expression of sort Pos, Nat or Int
///\return Data expression for the absolute value of DataExpr of sort
///     intersect(SortExpr, Nat)
ATermAppl gsMakeDataExprAbs(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Pos, Nat or Int 
///\return Data expression for the negation of DataExpr, which has sort Int
ATermAppl gsMakeDataExprNeg(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Pos, Nat or Int
///\return Data expression for the successor of DataExpr
///     The result has sort:
///     - Pos, if DataExpr has sort Pos
///     - Pos, if DataExpr has sort Nat
///     - Int, if DataExpr has sort Int
ATermAppl gsMakeDataExprSucc(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Pos, Nat or Int
///\return Data expression for the predecessor of DataExpr
///     The result has sort:
///     - Nat, if DataExpr has sort Pos
///     - Int, if DataExpr has sort Nat
///     - Int, if DataExpr has sort Int
ATermAppl gsMakeDataExprPred(ATermAppl DataExpr);

///\pre DataExprbit and DataExprNum are data expressions of sort Bool and
///     Nat or Int, respectively, which we denote by b and n
///\return Data expression for '2*n + |b|'. The result has sort:
///     - Pos, if DataExpr has sort Nat
///     - Int, if DataExpr has sort Int
ATermAppl gsMakeDataExprDub(ATermAppl DataExprBit, ATermAppl DataExprNum);

///\pre DataExprLHS and DataExprRHS are data expressions of sort Pos, Nat or
///     Int, denoted by t and u
///\return Data expression for t + u (see gsMakeOpIdAdd for information about the
///     result sort)
ATermAppl gsMakeDataExprAdd(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprBit, DataExprLHS and DataExprRHS are data expressions of sort
///     Bool, Pos and Pos, respectively, which we denote by b, p and q.
///\return Data expression for 'p + q + |b|', i.e. addition with carry.
ATermAppl gsMakeDataExprAddC(ATermAppl DataExprBit, ATermAppl DataExprLHS,
  ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are both data expressions of sort Pos, Nat
///     or Int
///\return Data expression for the subtraction of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprSubt(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are both data expressions of sort Pos or
///     Nat, which we denote by x and y.
///     x >= y
///\return Data expression for subtraction 'x - y', of sort Nat
ATermAppl gsMakeDataExprGTESubt(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprBit, DataExprLHS and DataExprRHS are data expressions of sort
///     Bool, Pos and Pos, respectively, which we denote by b, p and q.
///     p >= q + |b|
///\return Data expression for 'p - (q + |b|)', i.e. subtraction with borrow, of
///     sort Nat
ATermAppl gsMakeDataExprGTESubtB(ATermAppl DataExprBit, ATermAppl DataExprLHS,
  ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are both data expressions of sort Pos, Nat
///     or Int
///\return Data expression for the multiplication of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprMult(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprBit, DataExprIR, DataExprLHS and DataExprRHS are data
///     expressions of sort Bool, Pos, Pos, and Pos, respectively, which we
///     denote by b, p, q and r.
///\return Data expression for '|b|*p + q*r', i.e. multiplication which allows
///     for the storage of intermediate results.
ATermAppl gsMakeDataExprMultIR(ATermAppl DataExprBit, ATermAppl DataExprIR,
  ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS is a data expression of sort Pos, Nat or Int, which we
///     denote by x
///     DataExprRHS is a data expression of sort Pos, which we denote by p
///\return Data expression for x div p of sort:
///     - Nat, if x is of sort Pos or Nat
///     - Int, if x is of sort Int
ATermAppl gsMakeDataExprDiv(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS is a data expression of sort Pos, Nat or Int, which we
///     denote by x
///     DataExprRHS is a data expression of sort Pos, which we denote by p
///\return Data expression for x mod p of sort Nat
ATermAppl gsMakeDataExprMod(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of sort Pos
///\return Data expression for quotient and remainder after division,
///     of sort NatPair
ATermAppl gsMakeDataExprDivMod(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprPair, DataExprBool and DataExprPos are data
///     expressions of sort NatPair, Bool and Pos, respectively
///\return Data expression for the generalised quotient and remainder
///     after division, of sort NatPair
ATermAppl gsMakeDataExprGDivMod(ATermAppl DataExprPair, ATermAppl DataExprBool,
  ATermAppl DataExprPos);

///\pre DataExprNat1, DataExprNat2 and DataExprPos are data
///     expressions of sort Nat, Nat and Pos, respectively
///\return Data expression for the generalised generalised quotient and remainder
///     after division, of sort NatPair
ATermAppl gsMakeDataExprGGDivMod(ATermAppl DataExprNat1, ATermAppl DataExprNat2,
  ATermAppl DataExprPos);

///\pre DataExprLHS is a data expression of sort Nat or Int
///     DataExprRHS is a data expression of sort Pos
///\return Data expression for the exponentiation of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprExp(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExpr is a data expression of sort NatPair
///\return Data expression for 'first', of sort Nat
ATermAppl gsMakeDataExprFirst(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort NatPair
///\return Data expression for 'last', of sort Nat
ATermAppl gsMakeDataExprLast(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort Nat, which we denote by n
///\return Data expression for 'even(n)', of sort Bool
ATermAppl gsMakeDataExprEven(ATermAppl DataExpr);

///\pre SortExpr is a sort expression
///\return Data expression for the empty list of sort SortExpr
ATermAppl gsMakeDataExprEmptyList(ATermAppl SortExpr);

///\pre All Data expressions in DataExprs are of the same sort.
///     SortExpr is a sort expression (List of Sort(DataExprs))
///\return Data expression for the list enumeration of DataExpr
ATermAppl gsMakeDataExprListEnum(ATermList DataExprs, ATermAppl SortExpr);

///\pre DataExprLHS and DataExprRHS are data expressions
///\return Data expression for the list cons of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprCons(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExpr is a data expression
///\return Data expression for the list size of DataExpr
ATermAppl gsMakeDataExprListSize(ATermAppl DataExpr);

///\pre DataExprLHS and DataExprRHS are data expressions
///\return Data expression for the list snoc of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprSnoc(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExpr is a sort expression
///\return Data expression for the list concatenation of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprConcat(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions, of which the latter is
///     of sort Nat
///     SortExpr is a sort expression
///\return Data expression for the 'element at position' of DataExprLHS and
///     DataExprRHS with result sort SortExpr
ATermAppl gsMakeDataExprEltAt(ATermAppl DataExprLHS, ATermAppl DataExprRHS,
  ATermAppl SortExpr);
     
///\pre DataExpr is a data expression and SortExpr is a sort expression
///\return Data expression for the head of DataExpr of result sort SortExpr
ATermAppl gsMakeDataExprHead(ATermAppl DataExpr, ATermAppl SortExpr);

///\pre DataExpr is a data expression
///\return Data expression for the tail of DataExpr
ATermAppl gsMakeDataExprTail(ATermAppl DataExpr);

///\pre DataExpr is a data expression and SortExpr is a sort expression
///\return Data expression for the right head of DataExpr of result sort SortExpr
ATermAppl gsMakeDataExprRHead(ATermAppl DataExpr, ATermAppl SortExpr);

///\pre DataExpr is a data expression
///\return Data expression for the right tail of DataExpr
ATermAppl gsMakeDataExprRTail(ATermAppl DataExpr);

///\pre DataExprLHS and DataExprRHS are data expressions
///\return Data expression for element test "e in e'", where e = DataExprLHS and
///     e' = DataExprRHS
ATermAppl gsMakeDataExprEltIn(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExpr is a data expression of sort S -> Bool
///     SortExprResult is a sort expression
///\return Set comprehension for sort S with result sort SortExprResult
ATermAppl gsMakeDataExprSetComp(ATermAppl DataExpr, ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Data expression for the empty set of sort SortExpr
ATermAppl gsMakeDataExprEmptySet(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Data expression for the set enumeration of DataExprs
ATermAppl gsMakeDataExprSetEnum(ATermList DataExprs, ATermAppl SortExpr);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort
///\return Data expression for the subset or equality relation "e <= e'", where
///     e = DataExprLHS and e' = DataExprRHS
ATermAppl gsMakeDataExprSubSetEq(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort
///\return Data expression for the proper subset relation "e < e'", where
///     e = DataExprLHS and e' = DataExprRHS
ATermAppl gsMakeDataExprSubSet(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort
///\return Data expression for the set union of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprSetUnion(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort
///\return Data expression for the set difference of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprSetDiff(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort
///\return Data expression for the set intersection of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprSetInterSect(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExpr is a data expression
///\return Data expression for the set complement of DataExpr
ATermAppl gsMakeDataExprSetCompl(ATermAppl DataExpr);

///\pre DataExpr is a data expression of sort S -> Nat
///     SortExprResult is a sort expression
///\return Bag comprehension for sort S with result sort SortExprResult
ATermAppl gsMakeDataExprBagComp(ATermAppl DataExpr, ATermAppl SortExprResult);

///\pre SortExpr is a sort expression
///\return Data expression for the empty set of sort SortExpr
ATermAppl gsMakeDataExprEmptyBag(ATermAppl SortExpr);

///\pre SortExpr is a sort expression
///\return Data expression for a bag of SortExpr, with DataExprs as bag elements
ATermAppl gsMakeDataExprBagEnum(ATermList DataExprs, ATermAppl SortExpr);

///\pre DataExprLHS and DataExprRHS are data expressions
///\return Data expression for the count of element DataExprLHS in bag DataExprRHS
ATermAppl gsMakeDataExprCount(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort
///\return Data expression for the subbag or equality relation "e <= e'", where
///     e = DataExprLHS and e' = DataExprRHS
ATermAppl gsMakeDataExprSubBagEq(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort
///\return Data expression for the proper subbag relation "e < e'", where
///     e = DataExprLHS and e' = DataExprRHS
ATermAppl gsMakeDataExprSubBag(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort
///\return Data expression for the bag union of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprBagUnion(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort
///\return Data expression for the bag difference of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprBagDiff(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExprLHS and DataExprRHS are data expressions of the same sort
///\return Data expression for the bag intersection of DataExprLHS and DataExprRHS
ATermAppl gsMakeDataExprBagInterSect(ATermAppl DataExprLHS, ATermAppl DataExprRHS);

///\pre DataExpr is a data expression
///\return Data expression for Bag2Set(DataExpr) of sort SortExpr
ATermAppl gsMakeDataExprBag2Set(ATermAppl DataExpr, ATermAppl SortExpr);

///\pre DataExpr is a data expression
///\return Data expression for Set2Bag(DataExpr) of sort SortExpr
ATermAppl gsMakeDataExprSet2Bag(ATermAppl DataExpr, ATermAppl SortExpr);

// Auxiliary functions concerning data expressions 

///\pre DataExpr and DataExprArg1 are data expressions
///\return Internal representation of the data expression
///     DataExpr(DataExprArg1)
ATermAppl gsMakeDataAppl1(ATermAppl DataExpr, ATermAppl DataExprArg1);

///\pre DataExpr, DataExprArg1 and DataExprArg2 are data expressions
///\return Internal representation of the data expression
///     DataExpr(DataExprArg1)(DataExprArg2)
ATermAppl gsMakeDataAppl2(ATermAppl DataExpr, ATermAppl DataExprArg1,
  ATermAppl DataExprArg2);

///\pre DataExpr, DataExprArg1, DataExprArg2 and DataExprArg3 are data
///     expressions
///\return Internal representation of the data expression
///     DataExpr(DataExprArg1)(DataExprArg2)(DataExprArg3)
ATermAppl gsMakeDataAppl3(ATermAppl DataExpr, ATermAppl DataExprArg1,
  ATermAppl DataExprArg2, ATermAppl DataExprArg3);

///\pre DataExpr, DataExprArg1, DataExprArg2, DataExprArg3 and DataExprArg4 are
///     data expressions
///\return Internal representation of the data expression
///     DataExpr(DataExprArg1)(DataExprArg2)(DataExprArg3)(DataExprArg4)
ATermAppl gsMakeDataAppl4(ATermAppl DataExpr, ATermAppl DataExprArg1,
  ATermAppl DataExprArg2, ATermAppl DataExprArg3, ATermAppl DataExprArg4);

///\pre DataExpr is a data expression, which we denote by e.
///     DataExprArgs is of the form [e_0, ..., e_n], where n is a natural
///     number and each e_i, 0 <= i <= n, is a data expression.
///\return Internal representation of the data expression e(e_0)...(e_n).
ATermAppl gsMakeDataApplList(ATermAppl DataExpr, ATermList DataExprArgs);

///\pre p is of the form "[1-9][0-9]*"
///\return data expression of sort Pos that is a representation of p
ATermAppl gsMakeDataExprPos(char *p);

///\pre p > 0
///\return data expression of sort Pos that is a representation of p
ATermAppl gsMakeDataExprPos_int(int p);

///\pre n is of the form "0 | [1-9][0-9]*"
///\return data expression of sort Nat that is a representation of n
ATermAppl gsMakeDataExprNat(char *n);

///\pre n >= 0
///\return data expression of sort Nat that is a representation of n
ATermAppl gsMakeDataExprNat_int(int n);

///\pre z is of the form "0 | -? [1-9][0-9]*"
///\return data expression of sort Int that is a representation of z
ATermAppl gsMakeDataExprInt(char *z);

///\return data expression of sort Int that is a representation of z
ATermAppl gsMakeDataExprInt_int(int z);

///\pre z is of the form "0 | -? [1-9][0-9]*"
///\return data expression of sort Real that is a representation of z
ATermAppl gsMakeDataExprReal(char *z);

///\return data expression of sort Real that is a representation of z
ATermAppl gsMakeDataExprReal_int(int z);

///\pre PosExpr is a data expression of sort Pos
///\return PosExpr is built from constructors only
bool gsIsPosConstant(const ATermAppl PosExpr);

///\pre PosConstant is a data expression of sort Pos built from constructors only
///\return The value of PosExpr
///     Note that the result is created with malloc, so it has to be freed
char *gsPosValue(const ATermAppl PosConstant);

///\pre PosConstant is a data expression of sort Pos built from constructors only
///\return The value of PosExpr
int gsPosValue_int(const ATermAppl PosConstant);

///\pre NatExpr is a data expression of sort Nat
///\return NatExpr is built from constructors only
bool gsIsNatConstant(const ATermAppl NatExpr);

///\pre NatConstant is a data expression of sort Nat built from constructors only
///\return The value of NatExpr
///     Note that the result is created with malloc, so it has to be freed
char *gsNatValue(const ATermAppl NatConstant);

///\pre NatConstant is a data expression of sort Nat built from constructors only
///\return The value of NatExpr
int gsNatValue_int(const ATermAppl NatConstant);

///\pre IntExpr is a data expression of sort Int
///\return IntExpr is built from constructors only
bool gsIsIntConstant(const ATermAppl IntExpr);

///\pre IntConstant is a data expression of sort Int built from constructors only
///\return The value of IntExpr
///     Note that the result is created with malloc, so it has to be freed
char *gsIntValue(const ATermAppl IntConstant);

///\pre IntConstant is a data expression of sort Int built from constructors only
///\return The value of IntExpr
int gsIntValue_int(const ATermAppl IntConstant);

///\pre DataExpr is a data expression
///\return DataExpr is true
bool gsIsDataExprTrue(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is false
bool gsIsDataExprFalse(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a not
bool gsIsDataExprNot(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is an conjunction
bool gsIsDataExprAnd(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a disjunction
bool gsIsDataExprOr(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is an implication
bool gsIsDataExprImp(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is an equality
bool gsIsDataExprEq(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is an inequality
bool gsIsDataExprNeq(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is an if then else
bool gsIsDataExprIf(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is a constructor for 0
bool gsIsDataExprC0(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is a constructor for 1
bool gsIsDataExprC1(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is a constructor CDub.
bool gsIsDataExprCDub(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a CNat
bool gsIsDataExprCNat(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a CPair
bool gsIsDataExprCPair(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a CInt
bool gsIsDataExprCInt(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a CReal
bool gsIsDataExprCReal(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a Pos2Nat
bool gsIsDataExprPos2Nat(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a Pos2Int
bool gsIsDataExprPos2Int(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a Pos2Real
bool gsIsDataExprPos2Real(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a Nat2Int
bool gsIsDataExprNat2Int(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a Nat2Real
bool gsIsDataExprNat2Real(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a Int2Real
bool gsIsDataExprInt2Real(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a CNeg
bool gsIsDataExprCNeg(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a less then or equal
bool gsIsDataExprLTE(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a less then
bool gsIsDataExprLT(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a greater then or equal
bool gsIsDataExprGTE(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a greater then
bool gsIsDataExprGT(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a max
bool gsIsDataExprMax(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a min
bool gsIsDataExprMin(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a abs
bool gsIsDataExprAbs(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a negation (unary minus)
bool gsIsDataExprNeg(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a successor
bool gsIsDataExprSucc(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a predecessor
bool gsIsDataExprPred(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a Dub
bool gsIsDataExprDub(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is an addition
bool gsIsDataExprAdd(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is an addition with carry
bool gsIsDataExprAddC(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a substraction
bool gsIsDataExprSubt(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a GTESubt
bool gsIsDataExprGTESubt(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a GTESubtB
bool gsIsDataExprGTESubtB(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a multiplication
bool gsIsDataExprMult(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a MultIR
bool gsIsDataExprMultIR(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a div
bool gsIsDataExprDiv(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a mod
bool gsIsDataExprMod(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a DivMod
bool gsIsDataExprDivMod(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a GDivMod
bool gsIsDataExprGDivMod(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is a GGDivMod
bool gsIsDataExprGGDivMod(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return DataExpr is an Even
bool gsIsDataExprEven(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is a number
bool gsIsDataExprNumber(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is an expression for an empty list
bool gsIsDataExprEmptyList(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is a list enumeration
bool gsIsDataExprListEnum(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is a cons list.
bool gsIsDataExprCons(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is a set enumeration
bool gsIsDataExprSetEnum(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is a bag enumeration
bool gsIsDataExprBagEnum(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is a set comprehension
bool gsIsDataExprSetComp(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is a bag comprehension
bool gsIsDataExprBagComp(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is the implementation of
///     a universal quantification.
bool gsIsDataExprForall(ATermAppl DataExpr);

///\pre DataExpr is a data expression
///\return the data expression is the implementation of
///     an existential quantification.
bool gsIsDataExprExists(ATermAppl DataExpr);

// Multiactions
// ------------

///\pre MultAct is a multiaction
///\return the sorted variant of the argument
ATermAppl gsSortMultAct(ATermAppl MultAct);


// Process expressions
// -------------------

///\pre Term is not NULL
///\return Term is a process expression
bool gsIsProcExpr(ATermAppl Term);


// Mu-calculus formulas
// --------------------

///\return Term is a state formula
bool gsIsStateFrm(ATermAppl Term);

///\pre Term is not NULL
///\return Term is a regular formula
bool gsIsRegFrm(ATermAppl Term);

///\pre Term is not NULL
///\return Term is a action formula
bool gsIsActFrm(ATermAppl Term);


// PBES's
// ------

///\pre Term is not NULL
///\return Term is a Parameterised Boolean Expression
bool gsIsPBExpr(ATermAppl Term);

///\pre Term is not NULL
///\return Term is a fixpoint
bool gsIsFixpoint(ATermAppl Term);

  }
}

#endif /// MCRL2_LIBSTRUCT_H
