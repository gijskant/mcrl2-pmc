#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <ctype.h>

#include "liblowlevel.h"

extern "C" {

//String manipulation
//-------------------

#if !(defined __USE_SVID || defined __USE_BSD || defined __USE_XOPEN_EXTENDED || defined __APPLE__ || defined _MSC_VER)
char *strdup(const char *s)
{
    size_t len;
    char *p;

    len = strlen(s);
    if((p = (char *)malloc(len + 1)) == NULL)
      return NULL;
    return strcpy(p, s);
} 
#endif

//Message printing options
//------------------------

bool gsQuiet   = false;//indicates if quiet mode is enabled
bool gsWarning = true; //indicates if warning messages should be printed
bool gsVerbose = false;//indicates if verbose messages should be printed
bool gsDebug   = false;//indicates if debug messages should be printed

void gsSetQuietMsg(void)
{
  gsQuiet   = true;
  gsWarning = false;
  gsVerbose = false;
  gsDebug   = false;
}

void gsSetNormalMsg(void)
{
  gsQuiet   = false;
  gsWarning = true;
  gsVerbose = false;
  gsDebug   = false;
}

void gsSetVerboseMsg(void)
{
  gsQuiet   = false;
  gsWarning = true;
  gsVerbose = true;
  gsDebug   = false;
}

void gsSetDebugMsg(void)
{
  gsQuiet   = false;
  gsWarning = true;
  gsVerbose = true;
  gsDebug   = true;
}

// Function pointer for a custom message printing routine
void (*custom_message_handler)(gsMessageType, char*) = 0;

// Sets custom_message_handler a custom message printing routine
void gsSetCustomMessageHandler(void (*h)(gsMessageType, char*)) {
  custom_message_handler = h;
}

//Substitutions on ATerm's
//------------------------

static AFun substafun;
static bool substafun_notset = true;
ATermAppl gsMakeSubst(ATerm OldValue, ATerm NewValue)
{
  if ( substafun_notset )
  {
	  substafun = ATmakeAFun("subst", 2, ATfalse);
	  ATprotectAFun(substafun);
	  substafun_notset = false;
  }
  return ATmakeAppl2(substafun, OldValue, NewValue);
}

ATerm gsSubstValues(ATermList Substs, ATerm Term, bool Recursive)
{
  ATermList l = Substs;
  while (!ATisEmpty(l)) {
    ATermAppl Subst = ATAgetFirst(l);
    if (ATisEqual(ATgetArgument(Subst, 0), Term)) {
      return ATgetArgument(Subst, 1);
    }
    l = ATgetNext(l);
  }
  if (!Recursive) {
    return Term;
  } else {
    //Recursive; distribute substitutions over the arguments/elements of Term
    if (ATgetType(Term) == AT_APPL) {
      //Term is an ATermAppl; distribute substitutions over the arguments
      AFun Head = ATgetAFun((ATermAppl) Term);
      int NrArgs = ATgetArity(Head);
      if (NrArgs > 0) {
      	DECL_A(Args,ATerm,NrArgs);
        for (int i = 0; i < NrArgs; i++) {
          Args[i] = gsSubstValues(Substs, ATgetArgument((ATermAppl) Term, i),
            Recursive);
        }
        ATerm a = (ATerm) ATmakeApplArray(Head, Args);
        FREE_A(Args);
        return a;
      } else {
        return Term;
      }
    } else if (ATgetType(Term) == AT_LIST) {
      //Term is an ATermList; distribute substitutions over the elements
      ATermList Result = ATmakeList0();
      while (!ATisEmpty((ATermList) Term)) {
        Result = ATinsert(Result,
          gsSubstValues(Substs, ATgetFirst((ATermList) Term), Recursive));
        Term = (ATerm) ATgetNext((ATermList) Term);
      }
      return (ATerm) ATreverse(Result);
    } else {
      return Term;
    }
  }
}

ATerm gsSubstValuesTable(ATermTable Substs, ATerm Term, bool Recursive)
{
  ATerm Result = ATtableGet(Substs, Term);
  if ((Result) != NULL) {
    return Result;
  }
  if (!Recursive) {
    return Term;
  } else {
    //Recursive; distribute substitutions over the arguments/elements of Term
    if (ATgetType(Term) == AT_APPL) {
      //Term is an ATermAppl; distribute substitutions over the arguments
      AFun Head = ATgetAFun((ATermAppl) Term);
      int NrArgs = ATgetArity(Head);
      if (NrArgs > 0) {
      	DECL_A(Args,ATerm,NrArgs);
        for (int i = 0; i < NrArgs; i++) {
          Args[i] = gsSubstValuesTable(Substs, ATgetArgument((ATermAppl) Term, i),
            Recursive);
        }
        ATerm a = (ATerm) ATmakeApplArray(Head, Args);
        FREE_A(Args);
        return a;
      } else {
        return Term;
      }
    } else if (ATgetType(Term) == AT_LIST) {
      //Term is an ATermList; distribute substitutions over the elements
      ATermList Result = ATmakeList0();
      while (!ATisEmpty((ATermList) Term)) {
        Result = ATinsert(Result,
          gsSubstValuesTable(Substs, ATgetFirst((ATermList) Term), Recursive));
        Term = (ATerm) ATgetNext((ATermList) Term);
      }
      return (ATerm) ATreverse(Result);
    } else {
      return Term;
    }
  }
}

ATermList gsAddSubstToSubsts(ATermAppl Subst, ATermList Substs)
{
  //add Subst to Substs in which Subst is performed on the RHS's
  ATermList Result = ATmakeList0();
  while (!ATisEmpty(Substs)) {
    ATermAppl SubstsElt = ATAgetFirst(Substs);
    Result = ATinsert(Result, (ATerm) ATsetArgument(SubstsElt,
      (ATerm) gsSubstValues(ATmakeList1((ATerm) Subst),
        ATgetArgument(SubstsElt, 1) , true), 1));
    Substs = ATgetNext(Substs);
  }
  return ATinsert(ATreverse(Result), (ATerm) Subst);
}

//Occurrences of ATerm's
//----------------------

bool gsOccurs(ATerm Elt, ATerm Term)
{
  bool Result = false;
  if (ATisEqual(Elt, Term)) {
    Result = true;
  } else {
    //check occurrences of Elt in the arguments/elements of Term
    if (ATgetType(Term) == AT_APPL) {
      AFun Head = ATgetAFun((ATermAppl) Term);
      int NrArgs = ATgetArity(Head);
      for (int i = 0; i < NrArgs && !Result; i++) {
        Result = gsOccurs(Elt, ATgetArgument((ATermAppl) Term, i));
      }
    } else if (ATgetType(Term) == AT_LIST) {
      while (!ATisEmpty((ATermList) Term) && !Result)
      {
        Result = gsOccurs(Elt, ATgetFirst((ATermList) Term));
        Term = (ATerm) ATgetNext((ATermList) Term);
      }
    }
  }
  return Result;
}

int gsCount(ATerm Elt, ATerm Term)
{
  int Result = 0;
  if (ATisEqual(Elt, Term)) {
    Result = 1;
  } else {
    //count occurrences of Elt in the arguments/elements of Term
    if (ATgetType(Term) == AT_APPL) {
      AFun Head = ATgetAFun((ATermAppl) Term);
      int NrArgs = ATgetArity(Head);
      for (int i = 0; i < NrArgs; i++) {
        Result += gsCount(Elt, ATgetArgument((ATermAppl) Term, i));
      }
    } else if (ATgetType(Term) == AT_LIST) {
      while (!ATisEmpty((ATermList) Term))
      {
        Result += gsCount(Elt, ATgetFirst((ATermList) Term));
        Term = (ATerm) ATgetNext((ATermList) Term);
      }
    }
  }
  return Result;
}

//String representations of numbers
//---------------------------------

inline int gsChar2Int(char c)
//Pre: '0' <= c < '9'
//Ret: integer value corresponding to c
{
  assert(c >= '0' && c <= '9');
  return c - '0';
}

inline char gsInt2Char(int n)
//Pre: 0 <= n <= 9
//Ret: character corresponding to the value of n
{
  assert(n >= 0 && n <= 9);
  return n + '0';
}

char *gsStringDiv2(const char *n)
{
  assert(strlen(n) > 0);
  int l = strlen(n); //length of n
  char *r = (char *) malloc((l+1) * sizeof(char)); //result char*
  //calculate r[0]
  r[0] = gsInt2Char(gsChar2Int(n[0])/2);
  //declare counter for the elements of r
  int j = (r[0] == '0')?0:1;
  //calculate remaining indices of r
  for (int i=1; i<l; i++)
  {
    //r[j] = 5*(n[i-1] mod 2) + n[i] div 2
    r[j] = gsInt2Char(5*(gsChar2Int(n[i-1])%2) + gsChar2Int(n[i])/2);
    //update j
    j = j+1;
  }
  //terminate string
  r[j] = '\0';
  return r;
}

int gsStringMod2(const char *n)
{
  assert(strlen(n) > 0);
  return gsChar2Int(n[strlen(n)-1]) % 2;
}

char *gsStringDub(const char *n, const int inc)
{
  assert(strlen(n) > 0);
  int l = strlen(n); //length of n
  char *r = (char *) malloc((l+2) * sizeof(char)); //result char*
  //calculate r[0]: n[0] div 5
  r[0] = gsInt2Char(gsChar2Int(n[0])/5);
  //declare counter for the elements of r
  int j = (r[0] == '0')?0:1;
  //calculate remaining indices of r
  for (int i=0; i<l-1; i++)
  {
    //r[j] = 2*(n[i-1] mod 5) + n[i] div 5
    r[j] = gsInt2Char(2*(gsChar2Int(n[i])%5) + gsChar2Int(n[i+1])/5);
    //update j
    j = j+1;
  }
  //calculate last index of r
  r[j] = gsInt2Char(2*(gsChar2Int(n[l-1])%5) + inc);
  j = j+1;
  //terminate string
  r[j] = '\0';
  return r;
}

int NrOfChars(int n)
{
  if (n > 0)
    return (int) ceil(log10((double) n));
  else if (n == 0)
    return 1;
  else //n < 0
    return (int) ceil(log10((double) abs(n))) + 1;
}

}
