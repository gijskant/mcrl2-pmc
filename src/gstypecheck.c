#ifdef __cplusplus
extern "C"
{
#endif

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __cplusplus
}
#endif

/**
@defgroup macros Standardized solution for declaring local variable-sized arrays.
@param type of the elements
@param name of the array variable
@param size of the array expressed in the number of elements
*/
#if defined(__cplusplus) || (defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#define DECLA(type, name, size) type name[size]
#else
#ifdef _MSC_VER
#define DECLA(type, name, size) type *name = (type*) ((size)>0?alloca((size)*sizeof(type)):NULL)
#else
#define DECLA(type, name, size) type *name = (type*) ((size)>0?alloca((size)*sizeof(type)):NULL)
#endif
#endif
/** @} */

#include "gstypecheck.h"
#include "gsfunc.h"

#define ThrowF            {ThrowV(ATfalse);}
//store ATfalse in result and throw an exception

#define ThrowMF(...)      {ThrowVM(ATfalse, __VA_ARGS__);}
//print error message supplied by the first parameter with the remaining
//store ATfalse in result and throw an exception


// Static data 
// system constants and functions 
typedef struct { 
  ATermTable constants;		//name -> Set(sort expression)
  ATermTable functions;		//name -> Set(sort expression)
} gsSystem;
static gsSystem gssystem;

// the static context of the spec will be checked and used, not transformed
typedef struct { 
  ATermIndexedSet basic_sorts;	
  ATermTable defined_sorts;	//name -> sort expression
  ATermTable constants;		//name -> Set(sort expression)
  ATermTable functions;		//name -> Set(sort expression)
  ATermTable actions;	        //name -> Set(List(sort expression)) because of action polymorphism
  //ATermIndexedSet typedactions;	//name#type
  ATermTable processes;	        //name -> Set(List(sort expression)) bacause of process polymorphism
  //ATermIndexedSet typedprocesses: use keys of body.proc_pars
} Context;
static Context context;

// the body may be transformed
typedef struct { 
  ATermList equations;	
  ATermTable proc_pars;	        //name#type -> List(Vars)
  ATermTable proc_bodies;	//name#type -> rhs 
  //ATermAppl init;             //in the hash tables proc_pars and proc_bodies with key "init#[]" (beware when writing)
} Body;
static Body body;

// Static function declarations
static void gstcDataInit(void);
static void gstcDataDestroy(void);
static ATbool gstcReadInSorts (ATermList);
static ATbool gstcReadInConstructors();
static ATbool gstcReadInFuncs(ATermList);
static ATbool gstcReadInActs (ATermList);
static ATbool gstcReadInProcsAndInit (ATermList, ATermAppl);
static ATermList gstcWriteProcs(void);

static ATbool gstcCheckNamesData(void);
static ATbool gstcCheckNamesProc(void);
 
static ATbool gstcTransformActProc(void);
static ATbool gstcTransformVarConst(void);

static inline ATbool gstcInTypesA(ATermAppl, ATermList);
static ATbool gstcEqTypesA(ATermAppl, ATermAppl);
static inline ATbool gstcInTypesL(ATermList, ATermList);
static ATbool gstcEqTypesL(ATermList, ATermList);

static ATbool gstcReadInSortStruct(ATermAppl);

static ATbool gstcCheckNamesP(ATermTable, ATermAppl);
static ATbool gstcCheckNamesD(ATermTable, ATermAppl);

static ATbool gstcAddConstant(ATermAppl, ATermAppl, const char*);
static ATbool gstcAddFunction(ATermAppl, ATermAppl, const char*);
static ATermTable gstcAddVars2Table(ATermTable,ATermList);
static ATermAppl gstcRewrActProc(ATermAppl);
static inline ATermAppl gstcMakeActionOrProc(ATbool, ATermAppl, ATermList, ATermList);
static ATermAppl gstcTraverseActProc(ATermAppl);
static ATermAppl gstcTraverseVarConstP(ATermTable, ATermAppl);
static ATermAppl gstcTraverseVarConstD(ATermTable, ATermAppl);
static ATermAppl gstcTraverseVarConstDN(int, ATermTable, ATermAppl);
static ATermList gstcTraverseVarConstL(ATermTable, ATermList);
static ATermList gstcTraverseVarConstLL(ATermTable, ATermList);

#define INIT_KEY gsMakeProcVarId(ATmakeAppl0(ATmakeAFun("init",0,ATtrue)),ATmakeList0())

// Main function
ATermAppl gsTypeCheck (ATermAppl input){	
  ATermAppl Result=NULL;
  gsVerboseMsg ("type checking phase started\n");
  gstcDataInit();

  gsDebugMsg ("type checking read-in phase started\n");
  if(!gstcReadInSorts(ATLgetArgument(ATAgetArgument(input,0),0))) {throw;}
  // Check soorts for loops
  // Unwind sorts to enable equiv and subtype relations
  if(!gstcReadInConstructors()) {throw;}
  if(!gstcReadInFuncs(ATconcat(ATLgetArgument(ATAgetArgument(input,1),0),
			       ATLgetArgument(ATAgetArgument(input,2),0)))) {throw;}
  body.equations=ATLgetArgument(ATAgetArgument(input,3),0);
  if(!gstcReadInActs(ATLgetArgument(ATAgetArgument(input,4),0))) {throw;}
  if(!gstcReadInProcsAndInit(ATLgetArgument(ATAgetArgument(input,5),0),
			     ATAgetArgument(ATAgetArgument(input,6),0))) {throw;}
  gsDebugMsg ("type checking read-in phase finished\n");
  
  gsDebugMsg ("type checking transform ActProc+VarConst phase started\n");
  if(!gstcTransformActProc()){throw;}
  if(!gstcTransformVarConst()){throw;}
  gsDebugMsg ("type checking transform ActProc+VarConst phase finished\n");

  if(!gstcCheckNamesData()) {throw;} //names and # of arguments
  if(!gstcCheckNamesProc()) {throw;} //names and # of arguments

  Result=ATsetArgument(input,(ATerm)gsMakeDataEqnSpec(body.equations),3);
  Result=ATsetArgument(Result,(ATerm)gsMakeProcEqnSpec(gstcWriteProcs()),5);
  Result=ATsetArgument(Result,(ATerm)gsMakeInit(ATAtableGet(body.proc_bodies,(ATerm)INIT_KEY)),6);

  gsVerboseMsg ("type checking phase finished\n");
 finally:
  if (Result != NULL) {
    gsDebugMsg("return %t\n", Result);
  } 
  else {
    gsDebugMsg("return NULL\n");
  }
  gstcDataDestroy();
  return Result;
}

// ============ Static functions
// ========= main processing functions
void gstcDataInit(void){
  ATprotect((ATerm* )&context.basic_sorts);
  ATprotect((ATerm* )&context.defined_sorts);	
  ATprotect((ATerm* )&context.constants);
  ATprotect((ATerm* )&context.functions);
  ATprotect((ATerm* )&context.actions);	
  ATprotect((ATerm* )&context.processes);
  ATprotect((ATerm* )&body.equations);
  ATprotect((ATerm* )&body.proc_pars);
  ATprotect((ATerm* )&body.proc_bodies);

  context.basic_sorts=ATindexedSetCreate(63,50);
  context.defined_sorts=ATtableCreate(63,50);
  context.constants=ATtableCreate(63,50);
  context.functions=ATtableCreate(63,50);
  context.actions=ATtableCreate(63,50);
  context.processes=ATtableCreate(63,50);
  body.proc_pars=ATtableCreate(63,50);
  body.proc_bodies=ATtableCreate(63,50);
}

void gstcDataDestroy(void){
  ATindexedSetDestroy(context.basic_sorts);
  ATtableDestroy(context.defined_sorts);
  ATtableDestroy(context.constants);
  ATtableDestroy(context.functions);
  ATtableDestroy(context.actions);
  ATtableDestroy(context.processes);
  ATtableDestroy(body.proc_pars);
  ATtableDestroy(body.proc_bodies);
}
 
static ATbool gstcReadInSorts (ATermList Sorts){
  ATbool new;
  ATbool Result=ATtrue;
  for(;!ATisEmpty(Sorts);Sorts=ATgetNext(Sorts)){
    ATermAppl Sort=ATAgetFirst(Sorts);
    ATermAppl SortName=ATAgetArgument(Sort,0);
    if(ATindexedSetGetIndex(context.basic_sorts, (ATerm)SortName)>=0 
       || ATAtableGet(context.defined_sorts, (ATerm)SortName)){
      ThrowMF("Double declaration of sort %t\n", SortName);
    }				
    if(gsIsSortId(Sort)) ATindexedSetPut(context.basic_sorts, (ATerm)SortName, &new);
    else
      if(gsIsSortRef(Sort)){
	ATtablePut(context.defined_sorts, (ATerm)SortName, (ATerm)ATAgetArgument(Sort,1));
	gsDebugMsg("recognized %t %t\n",SortName,(ATerm)ATAgetArgument(Sort,1));    	
      }
      else assert(0);
  }
 finally:
  return Result;
}  

static ATbool gstcReadInConstructors(){
  for(ATermList Sorts=ATtableKeys(context.defined_sorts);!ATisEmpty(Sorts);Sorts=ATgetNext(Sorts))
    if(!gstcReadInSortStruct(ATAtableGet(context.defined_sorts,ATgetFirst(Sorts)))) return ATfalse;
  return ATtrue;
} 

static ATbool gstcReadInFuncs(ATermList Funcs){
  gsDebugMsg("Star Read-in Func\n");    
  ATbool Result=ATtrue;
  for(;!ATisEmpty(Funcs);Funcs=ATgetNext(Funcs)){
    ATermAppl Func=ATAgetFirst(Funcs);
    ATermAppl FuncName=ATAgetArgument(Func,0);
    ATermAppl FuncType=ATAgetArgument(Func,1);
    
    if(gsIsSortArrowProd(FuncType)){
      if(!gstcAddFunction(FuncName,FuncType,"function")) {ThrowF;}
    }
    else{
      if(!gstcAddConstant(FuncName,FuncType,"constant")) {ThrowMF("Could not add constant\n");}
    }
    gsDebugMsg("Read-in Func %t, Types %t\n",FuncName,FuncType);    
  }
 finally:
  return Result;
} 

static ATbool gstcReadInActs (ATermList Acts){
  ATbool Result=ATtrue;
  for(;!ATisEmpty(Acts);Acts=ATgetNext(Acts)){
    ATermAppl Act=ATAgetFirst(Acts);
    ATermAppl ActName=ATAgetArgument(Act,0);
    ATermList ActType=ATLgetArgument(Act,1);
    
    ATermList Types=ATLtableGet(context.actions, (ATerm)ActName);
    if(!Types){
      Types=ATmakeList1((ATerm)ActType);
    }
    else{
      // the table context.actions contains a list of types for each
      // action name. We need to check if there is already such a type 
      // in the list. If so -- error, otherwise -- add
      if (gstcInTypesL(ActType, Types)){
	ThrowMF("Double declaration of action %t\n", ActName);
      }
      else{
	Types=ATappend(Types,(ATerm)ActType);
      }
    }
    ATtablePut(context.actions,(ATerm)ActName,(ATerm)Types);
  }
 finally:
  return Result;
}

static ATbool gstcReadInProcsAndInit (ATermList Procs, ATermAppl Init){
  ATbool Result=ATtrue;
  for(;!ATisEmpty(Procs);Procs=ATgetNext(Procs)){
    ATermAppl Proc=ATAgetFirst(Procs);
    ATermAppl ProcName=ATAgetArgument(ATAgetArgument(Proc,0),0);
    
    if(ATLtableGet(context.actions, (ATerm)ProcName)){
      ThrowMF("Declaration of both process and action %t\n", ProcName);
    }	

    ATermList ProcType=ATLgetArgument(ATAgetArgument(Proc,0),1);
    ATermList Types=ATLtableGet(context.processes,(ATerm)ProcName);
    if(!Types){
      Types=ATmakeList1((ATerm)ProcType);
    }
    else{
      // the table context.processes contains a list of types for each
      // process name. We need to check if there is already such a type 
      // in the list. If so -- error, otherwise -- add
      if (gstcInTypesL(ProcType, Types)){
	ThrowMF("Double declaration of process %t\n", ProcName);
      }
      else{
	Types=ATappend(Types,(ATerm)ProcType);
      }
    }
    ATtablePut(context.processes,(ATerm)ProcName,(ATerm)Types);
    ATtablePut(body.proc_pars,(ATerm)ATAgetArgument(Proc,0),(ATerm)ATLgetArgument(Proc,1));
    ATtablePut(body.proc_bodies,(ATerm)ATAgetArgument(Proc,0),(ATerm)ATAgetArgument(Proc,2));
    gsDebugMsg("Read-in Proc Name %t, Types %t\n",ProcName,Types);    
  }
  ATtablePut(body.proc_pars,(ATerm)INIT_KEY,(ATerm)ATmakeList0());
  ATtablePut(body.proc_bodies,(ATerm)INIT_KEY,(ATerm)Init);
 finally:
  return Result;
} 

static ATermList gstcWriteProcs(void){
  ATermList Result=ATmakeList0();
  for(ATermList ProcVars=ATtableKeys(body.proc_pars);!ATisEmpty(ProcVars);ProcVars=ATgetNext(ProcVars)){
    ATermAppl ProcVar=ATAgetFirst(ProcVars);
    if(ProcVar==INIT_KEY) continue;
    Result=ATinsert(Result,(ATerm)gsMakeProcEqn(ProcVar,
						ATLtableGet(body.proc_pars,(ATerm)ProcVar),
						ATAtableGet(body.proc_bodies,(ATerm)ProcVar)
						)
		    );
  }
  return ATreverse(Result);
}

static ATbool gstcCheckNamesData(void){
  ATbool Result=ATtrue;
  ATermTable Vars=ATtableCreate(63,50);
  for(ATermList Eqns=body.equations;!ATisEmpty(Eqns);Eqns=ATgetNext(Eqns)){
    ATermAppl Eqn=ATAgetFirst(Eqns);
    Vars=gstcAddVars2Table(Vars,ATLgetArgument(Eqn,0));
    if(!Vars){ThrowF;}
    ATermAppl Cond=ATAgetArgument(Eqn,1);
    if(!gsIsNil(Cond) && !gstcCheckNamesD(Vars,Cond)){ThrowF;}
    if(!gstcCheckNamesD(Vars,ATAgetArgument(Eqn,2))){ThrowF;}
    if(!gstcCheckNamesD(Vars,ATAgetArgument(Eqn,3))){ThrowF;}
  }
 finally:
  ATtableDestroy(Vars);
  return Result;
}

static ATbool gstcCheckNamesProc(void){
  ATbool Result=ATtrue;
  ATermTable Vars=ATtableCreate(63,50);
  for(ATermList ProcVars=ATtableKeys(body.proc_pars);!ATisEmpty(ProcVars);ProcVars=ATgetNext(ProcVars)){
    ATermAppl ProcVar=ATAgetFirst(ProcVars);
    Vars=gstcAddVars2Table(Vars,ATLtableGet(body.proc_pars,(ATerm)ProcVar));
    if(!Vars){ThrowF;}
    if(!gstcCheckNamesP(Vars,ATAtableGet(body.proc_bodies,(ATerm)ProcVar))){ATtableDestroy(Vars);ThrowF;}
  } 
 finally:
  ATtableDestroy(Vars);
  return Result;
}

static ATbool gstcTransformActProc(void){
  ATbool Result=ATtrue;
  for(ATermList ProcVars=ATtableKeys(body.proc_pars);!ATisEmpty(ProcVars);ProcVars=ATgetNext(ProcVars)){
    ATermAppl ProcVar=ATAgetFirst(ProcVars);
    ATermAppl ProcTerm=ATAtableGet(body.proc_bodies,(ATerm)ProcVar);
    ProcTerm=gstcTraverseActProc(ProcTerm);
    if(!ProcTerm){ThrowF;}
    ATtablePut(body.proc_bodies,(ATerm)ProcVar,(ATerm)ProcTerm);
  }
 finally:
  return Result;
}

static ATbool gstcTransformVarConst(void){
  ATbool Result=ATtrue;
  ATermTable Vars=ATtableCreate(63,50);
  
  //data terms in equations
  {
    ATermList NewEqns=ATmakeList0();
    for(ATermList Eqns=body.equations;!ATisEmpty(Eqns);Eqns=ATgetNext(Eqns)){
      ATermAppl Eqn=ATAgetFirst(Eqns);
      ATermList VarList=ATLgetArgument(Eqn,0);
      Vars=gstcAddVars2Table(Vars,VarList);
      if(!Vars){ThrowF;}
      ATermAppl Cond=ATAgetArgument(Eqn,1);
      if(!gsIsNil(Cond) && !(Cond=gstcTraverseVarConstD(Vars,Cond))){ThrowF;}
      ATermAppl Left=ATAgetArgument(Eqn,2);
      if(!(Left=gstcTraverseVarConstD(Vars,Left))){ThrowF;}
      ATermAppl Right=ATAgetArgument(Eqn,3);
      if(!(Right=gstcTraverseVarConstD(Vars,Right))){ThrowF;}
      NewEqns=ATinsert(NewEqns,(ATerm)gsMakeDataEqn(VarList,Cond,Left,Right));
    }
    body.equations=ATreverse(NewEqns);
  }
  
  //data terms in processes and init
  for(ATermList ProcVars=ATtableKeys(body.proc_pars);!ATisEmpty(ProcVars);ProcVars=ATgetNext(ProcVars)){
    ATermAppl ProcVar=ATAgetFirst(ProcVars);
    ATtableReset(Vars);
    Vars=gstcAddVars2Table(Vars,ATLtableGet(body.proc_pars,(ATerm)ProcVar));
    if(!Vars){ThrowF;}
    ATermAppl NewProcTerm=gstcTraverseVarConstP(Vars,ATAtableGet(body.proc_bodies,(ATerm)ProcVar));
    if(!NewProcTerm){ThrowF;}
    ATtablePut(body.proc_bodies,(ATerm)ProcVar,(ATerm)NewProcTerm);
  } 
 finally:
  ATtableDestroy(Vars);
  return Result;
}

// ======== Auxiliary functions
static inline ATbool gstcInTypesA(ATermAppl Type, ATermList Types){
  for(;!ATisEmpty(Types);Types=ATgetNext(Types))
    if(gstcEqTypesA(ATAgetFirst(Types),Type)) return ATtrue;
  return ATfalse;
}

static ATbool gstcEqTypesA(ATermAppl Type1, ATermAppl Type2){
  return ATisEqual(Type1, Type2);
}

static inline ATbool gstcInTypesL(ATermList Type, ATermList Types){
  for(;!ATisEmpty(Types);Types=ATgetNext(Types))
    if(gstcEqTypesL(ATLgetFirst(Types),Type)) return ATtrue;
  return ATfalse;
}

static ATbool gstcEqTypesL(ATermList Type1, ATermList Type2){
  return ATisEqual(Type1, Type2);
}

static ATbool gstcReadInSortStruct(ATermAppl SortExpr){
  ATbool Result=ATtrue;

  if(gsIsSortId(SortExpr)) return ATtrue;

  if(gsIsSortList(SortExpr) || gsIsSortSet(SortExpr) || gsIsSortBag(SortExpr))
    return gstcReadInSortStruct(ATAgetArgument(SortExpr,0));

  if(gsIsSortArrowProd(SortExpr)){
    if(!gstcReadInSortStruct(ATAgetArgument(SortExpr,1))) return ATfalse;
    for(ATermList Sorts=ATLgetArgument(SortExpr,0);!ATisEmpty(Sorts);Sorts=ATgetNext(Sorts)){
      if(!gstcReadInSortStruct(ATAgetFirst(Sorts))) return ATfalse;
    }
    return ATtrue;
  }
  
  if(gsIsSortStruct(SortExpr)){
    for(ATermList Constrs=ATLgetArgument(SortExpr,0);!ATisEmpty(Constrs);Constrs=ATgetNext(Constrs)){
      ATermAppl Constr=ATAgetFirst(Constrs);
    
      // recognizer -- if present -- a function from SortExpr to Bool
      ATermAppl Name=ATAgetArgument(Constr,2);
      if(!gsIsNil(Name) && 
	 !gstcAddFunction(Name,gsMakeSortArrowProd(ATmakeList1((ATerm)Constr),gsMakeSortExprBool()),"recognizer")) {ThrowF;}
      
      // constructor type and projections
      ATermList Projs=ATLgetArgument(Constr,1);
      Name=ATAgetArgument(Constr,0);
      if(ATisEmpty(Projs)){
	if(!gstcAddConstant(Name,Constr,"constructor constant")){ThrowF;}
	else continue;
      }
      
      ATermList ConstructorType=ATmakeList0();
      for(;!ATisEmpty(Projs);Projs=ATgetNext(Projs)){
	ATermAppl Proj=ATAgetFirst(Projs);
	ATermAppl ProjSort=ATAgetArgument(Proj,1);
	
	// not to forget, recursive call for ProjSort ;-)
	if(!gstcReadInSortStruct(ProjSort)) {ThrowF;}

	if(!gsIsNil(Proj) &&
	   !gstcAddFunction(ATAgetArgument(Proj,0),gsMakeSortArrowProd(ATmakeList1((ATerm)Constr),ProjSort),"projection")) {ThrowF;}
	ConstructorType=ATinsert(ConstructorType,(ATerm)ProjSort);
      }
      if(!gstcAddFunction(Name,gsMakeSortArrowProd(ATreverse(ConstructorType),Constr),"constructor")) {ThrowF;}
    }
    return ATtrue;
  }
  
  assert(0);
 finally:
  return Result;
} 

static ATbool gstcAddConstant(ATermAppl Name, ATermAppl Sort, const char* msg){
  ATbool Result=ATtrue;

  if(ATAtableGet(context.constants, (ATerm)Name) || ATLtableGet(context.functions, (ATerm)Name)){
    ThrowMF("Double declaration of %s %t\n", msg, Name);
  }
  
  ATtablePut(context.constants, (ATerm)Name, (ATerm)Sort);
 finally:
  return Result;
}

static ATbool gstcAddFunction(ATermAppl Name, ATermAppl Sort, const char *msg){
  ATbool Result=ATtrue;

  if(ATAtableGet(context.constants, (ATerm)Name)){
    ThrowMF("Double declaration of constant and %s %t\n", msg, Name);
  }

  ATermList Types=ATLtableGet(context.functions, (ATerm)Name);
  // the table context.functions contains a list of types for each
  // function name. We need to check if there is already such a type 
  // in the list. If so -- error, otherwise -- add
  if (Types && gstcInTypesA(Sort, Types)){
    ThrowMF("Double declaration of %s %t\n", msg, Name);
  }
  else{
    if (!Types) Types=ATmakeList0();
    Types=ATappend(Types,(ATerm)Sort);
    ATtablePut(context.functions,(ATerm)Name,(ATerm)Types);
  }
  gsDebugMsg("Read-in %s %t Type %t\n",msg,Name,Types);    
 finally:
  return Result;
}

static ATbool gstcCheckNamesP(ATermTable Vars, ATermAppl ProcTerm){
  ATbool Result=ATtrue;
 finally:
  return Result;
}

static ATbool gstcCheckNamesD(ATermTable Vars, ATermAppl DataTerm){
  ATbool Result=ATtrue;
 finally:
  return Result;
}
    
static ATermTable gstcAddVars2Table(ATermTable Vars, ATermList VarDecls){
  ATbool Result=ATtrue;

  for(;!ATisEmpty(VarDecls);VarDecls=ATgetNext(VarDecls)){
    ATermAppl VarDecl=ATAgetFirst(VarDecls);
    ATermAppl VarName=ATAgetArgument(VarDecl,0);
    ATermAppl VarType=ATAgetArgument(VarDecl,1);
    // if already defined -- replace (other option -- warning)
    // if variable name is a constant name -- it has more priority (other options -- warning, error)
    ATtablePut(Vars, (ATerm)VarName, (ATerm)VarType);
  } 
 finally:
  if (!Result) {
    ATtableDestroy(Vars);
    return NULL;
  }
  else
    return Vars;
}

static ATbool gstcMatchRewrActProc(ATermAppl ProcTerm){
  gsDebugMsg("matching %t, %d\n",ProcTerm, gsIsActionProcess(ProcTerm)); 
  if(gsIsActionProcess(ProcTerm)) return ATtrue;
  else return ATfalse;
}

//typedef struct make_list_filter_int {
// int n;
// ATbool length_is_n(ATerm List);
//
///*{
//  assert(ATgetType(List)==AT_LIST);
//  return (ATgetLength(List)==n);
//  }*/
//} Make_List_Filter_Int;

static ATermAppl gstcRewrActProc(ATermAppl ProcTerm){
  ATermAppl Result=NULL;
  ATermAppl Name=ATAgetArgument(ProcTerm,0);
  ATermList ParList;

  ATbool action;

  if((ParList=ATLtableGet(context.actions,(ATerm)Name))){
    action=ATtrue;
  }
  else{
    if((ParList=ATLtableGet(context.processes,(ATerm)Name))){
      action=ATfalse;
    }
    else{
      ThrowM("Action or process %t not declared\n", Name);
    }
  }
  assert(!ATisEmpty(ParList));
  
  int nFactPars=ATgetLength(ATLgetArgument(ProcTerm,1));
  char *msg=(action)?"action":"process";
  
  //filter the list of lists ParList to keep only the lists of lenth nFactPars
  {
    ATermList NewParList=ATmakeList0();
    for(;!ATisEmpty(ParList);ParList=ATgetNext(ParList)){
      ATermList Par=ATLgetFirst(ParList);
      if(ATgetLength(Par)==nFactPars) NewParList=ATinsert(NewParList,(ATerm)Par);
    }
    ParList=ATreverse(NewParList);
  }
 
  if(ATisEmpty(ParList)){
    ThrowM("No %s %t with %d parameter is declared (while typechecking %t)\n", msg, Name, nFactPars, ProcTerm);     
  }
  else{
    if(ATgetLength(ParList)==1){
      Result=gstcMakeActionOrProc(action,Name,ATLgetFirst(ParList),ATLgetArgument(ProcTerm,1));
    }
    else{
      Result=gstcMakeActionOrProc(action,Name,ATmakeList1((ATerm)gsMakeUnknown()),ATLgetArgument(ProcTerm,1));
      // here later can be [Unknown],ParList
    }
  }
  gsDebugMsg("recognized %s %t\n",msg,Result);    
 finally:
  return Result;
}

static inline ATermAppl gstcMakeActionOrProc(ATbool action, ATermAppl Name, ATermList FormParList, ATermList FactParList){
  return (action)?gsMakeAction(gsMakeActId(Name,FormParList),FactParList)
    :gsMakeProcess(gsMakeProcVarId(Name,FormParList),FactParList);
}

static ATermAppl gstcTraverseActProc(ATermAppl ProcTerm){
  // if maches -- apply
  if(gstcMatchRewrActProc(ProcTerm))
    return gstcRewrActProc(ProcTerm);
  
  // otherwise apply to args
  AFun ProcSymbol=ATgetAFun(ProcTerm);
  int n = ATgetArity(ProcSymbol);
  DECLA(ATerm,args,n);              // ATerm* args = alloca(n*sizeof(ATerm));
  for (int i=0;i<n;i++)
    args[i] = ATgetArgument(ProcTerm,i);
  
  // For conditions don't apply to the first parameter
  for (int i=gsIsCond(ProcTerm)?1:0;i<n;i++){
    if(ATgetType(args[i])==AT_APPL){
      args[i]=(ATerm)gstcTraverseActProc((ATermAppl)args[i]);
      if(!args[i]) return NULL;
    } 
  }
  return ATmakeApplArray(ProcSymbol,args);
}

static ATermAppl gstcTraverseVarConstP(ATermTable Vars, ATermAppl ProcTerm){
  ATermAppl Result=NULL;
  AFun ProcSymbol=ATgetAFun(ProcTerm);
  int n = ATgetArity(ProcSymbol);
  if(n==0) return ProcTerm;

  if(gsIsAction(ProcTerm) || gsIsProcess(ProcTerm)){
    ATermList NewPars=ATmakeList0();
    for(ATermList Pars=ATLgetArgument(ProcTerm,1);!ATisEmpty(Pars);Pars=ATgetNext(Pars)){
      ATermAppl NewPar=gstcTraverseVarConstD(Vars,ATAgetFirst(Pars));
      if(!NewPar) {throw;}
      NewPars=ATinsert(NewPars,(ATerm)NewPar);
    }
    return ATsetArgument(ProcTerm,(ATerm)ATreverse(NewPars),1);
  }

  if(gsIsRestrict(ProcTerm) || gsIsHide(ProcTerm) || 
     gsIsRename(ProcTerm) || gsIsComm(ProcTerm) || gsIsAllow(ProcTerm)){
    ATermAppl NewProc=gstcTraverseVarConstP(Vars,ATAgetArgument(ProcTerm,1));
    if(!NewProc) {throw;}
    return ATsetArgument(ProcTerm,(ATerm)NewProc,1);
  }

  if(gsIsSync(ProcTerm) || gsIsSeq(ProcTerm) || gsIsBInit(ProcTerm) ||
     gsIsMerge(ProcTerm) || gsIsLMerge(ProcTerm) || gsIsChoice(ProcTerm)){
    ATermAppl NewLeft=gstcTraverseVarConstP(Vars,ATAgetArgument(ProcTerm,0));
    if(!NewLeft) {throw;}
    ATermAppl NewRight=gstcTraverseVarConstP(Vars,ATAgetArgument(ProcTerm,1));
    if(!NewRight) {throw;}
    return ATsetArgument(ATsetArgument(ProcTerm,(ATerm)NewLeft,0),(ATerm)NewRight,1);
  }

  if(gsIsAtTime(ProcTerm)){
    ATermAppl NewProc=gstcTraverseVarConstP(Vars,ATAgetArgument(ProcTerm,0));
    if(!NewProc) {throw;}
    ATermAppl NewTime=gstcTraverseVarConstD(Vars,ATAgetArgument(ProcTerm,1));
    if(!NewTime) {throw;}
    return gsMakeAtTime(NewProc,NewTime);
  }

  if(gsIsCond(ProcTerm)){
    ATermAppl NewCond=gstcTraverseVarConstD(Vars,ATAgetArgument(ProcTerm,0));
    if(!NewCond) {throw;}
    ATermAppl NewLeft=gstcTraverseVarConstP(Vars,ATAgetArgument(ProcTerm,1));
    if(!NewLeft) {throw;}
    ATermAppl NewRight=gstcTraverseVarConstP(Vars,ATAgetArgument(ProcTerm,2));
    if(!NewRight) {throw;}
    return gsMakeCond(NewCond,NewLeft,NewRight);
  }

  if(gsIsSum(ProcTerm)){
    ATermTable NewVars=gstcAddVars2Table(Vars,ATLgetArgument(ProcTerm,0));
    if(!NewVars) {throw;}
    ATermAppl NewProc=gstcTraverseVarConstP(NewVars,ATAgetArgument(ProcTerm,1));
    if(!NewProc) {throw;}
    return ATsetArgument(ProcTerm,(ATerm)NewProc,1);
  }
  
  assert(0);
 finally:
  return Result;
}

static ATermAppl gstcTraverseVarConstD(ATermTable Vars, ATermAppl DataTerm){
  ATermAppl Result=NULL;
 
  if(gsIsNumber(DataTerm)) return DataTerm;

  if(gsIsSetBagComp(DataTerm)){
    ATermTable NewVars=gstcAddVars2Table(Vars,ATmakeList1((ATerm)ATAgetArgument(DataTerm,0)));
    if(!NewVars) {throw;}
    ATermAppl NewData=gstcTraverseVarConstD(NewVars,ATAgetArgument(DataTerm,1));
    if(!NewData) {throw;}
    return ATsetArgument(DataTerm,(ATerm)NewData,1);
  }

  if(gsIsForall(DataTerm) || gsIsExists(DataTerm) || gsIsLambda(DataTerm)){
    ATermTable NewVars=gstcAddVars2Table(Vars,ATLgetArgument(DataTerm,0));
    if(!NewVars) {throw;}
    ATermAppl NewData=gstcTraverseVarConstD(NewVars,ATAgetArgument(DataTerm,1));
    if(!NewData) {throw;}
    return ATsetArgument(DataTerm,(ATerm)NewData,1);
  }
  
  if(gsIsWhr(DataTerm)){
    ATermList WhereVarList=ATmakeList0();
    for(ATermList WhereList=ATLgetArgument(DataTerm,1);!ATisEmpty(WhereList);WhereList=ATgetNext(WhereList)){
      ATermAppl WhereElem=ATAgetFirst(WhereList);
      WhereVarList=ATinsert(WhereVarList, (ATerm)gsMakeDataVarId(ATAgetArgument(WhereElem,0),gsMakeUnknown()));
    }
    ATermTable NewVars=gstcAddVars2Table(Vars,ATreverse(WhereVarList));
    if(!NewVars) {throw;}
    ATermAppl NewData=gstcTraverseVarConstD(NewVars,ATAgetArgument(DataTerm,0));
    if(!NewData) {throw;}
    return ATsetArgument(DataTerm,(ATerm)NewData,0);
  }

  if(gsIsListEnum(DataTerm) || gsIsSetEnum(DataTerm)){
    ATermList NewData=gstcTraverseVarConstL(Vars,ATLgetArgument(DataTerm,0));
    if(!NewData) {throw;}
    return ATsetArgument(DataTerm,(ATerm)NewData,0);
  }  

  if(gsIsBagEnum(DataTerm)){
    ATermList NewData=gstcTraverseVarConstLL(Vars,ATLgetArgument(DataTerm,0));
    if(!NewData) {throw;}
    return ATsetArgument(DataTerm,(ATerm)NewData,0);
  }  

  if(gsIsDataApplProd(DataTerm)){
    ATermAppl NewData=gstcTraverseVarConstDN(ATgetLength(ATLgetArgument(DataTerm,1)),Vars,ATAgetArgument(DataTerm,0));
    if(!NewData) {throw;}
    ATermList NewDatas=gstcTraverseVarConstL(Vars,ATLgetArgument(DataTerm,1));
    if(!NewDatas) {throw;}
    return gsMakeDataApplProd(NewData,NewDatas);
  }  

  if(gsIsDataVarIdOpId(DataTerm)){
    ATermAppl Name=ATAgetArgument(DataTerm,0);
    ATermAppl Type=ATAtableGet(Vars,(ATerm)Name);
    if(Type) return gsMakeDataVarId(Name,Type);
    
    if((Type=ATAtableGet(context.constants,(ATerm)Name))) return gsMakeOpId(Name,Type);
    
    ATermList ParList=ATLtableGet(context.functions,(ATerm)Name);
    if(ParList && ATgetLength(ParList)==1) return gsMakeDataVarId(Name,ATAgetFirst(ParList));
    else{
      gsDebugMsg("Unknown Op %t\n",DataTerm);    
      return gsMakeOpId(Name,gsMakeUnknown());
    }
  }  
  assert(0);
 finally:
  return Result;
}
    
static ATermAppl gstcTraverseVarConstDN(int nFactPars, ATermTable Vars, ATermAppl DataTerm){
  if(gsIsDataVarIdOpId(DataTerm)){
    ATermAppl Name=ATAgetArgument(DataTerm,0);
    ATermAppl Type=ATAtableGet(Vars,(ATerm)Name);
    if(Type) return gsMakeDataVarId(Name,Type);
    
    if(!nFactPars){
      if((Type=ATAtableGet(context.constants,(ATerm)Name))) return gsMakeOpId(Name,Type);
      else{
	gsDebugMsg("Unknown Op %t\n",DataTerm);
	return gsMakeOpId(Name,Type);
      }
    }
    
    ATermList ParList=ATLtableGet(context.functions,(ATerm)Name);
    // filter ParList keeping only functions A_0#...#A_nFactPars->A
    if(ParList){
      ATermList NewParList=ATmakeList0();
      for(;!ATisEmpty(ParList);ParList=ATgetNext(ParList)){
	ATermAppl Par=ATAgetFirst(ParList);
	if(gsIsSortArrowProd(Par) && ATgetLength(ATLgetArgument(Par,0))==nFactPars) NewParList=ATinsert(NewParList,(ATerm)Par);
      }
      ParList=ATreverse(NewParList);
    }
    
    if(ParList && ATgetLength(ParList)==1) return gsMakeDataVarId(Name,ATAgetFirst(ParList));
    else{
      gsDebugMsg("Unknown Op %t\n",DataTerm);    
      return gsMakeOpId(Name,gsMakeUnknown());
    }
  }
  else return gstcTraverseVarConstD(Vars, DataTerm);
}


static ATermList gstcTraverseVarConstL(ATermTable Vars, ATermList DataTermList){
  ATermList Result=NULL;
  ATermList NewDataTermList=ATmakeList0();

  for(;!ATisEmpty(DataTermList);DataTermList=ATgetNext(DataTermList)){
    ATermAppl NewDataTerm=gstcTraverseVarConstD(Vars, ATAgetFirst(DataTermList));
    if(!NewDataTerm) {throw;}
    NewDataTermList=ATinsert(NewDataTermList,(ATerm)NewDataTerm);
  }
  return ATreverse(NewDataTermList);
 finally:
  return Result;
}

static ATermList gstcTraverseVarConstLL(ATermTable Vars, ATermList DataTermList2){
  ATermList Result=NULL;
  ATermList NewDataTermList2=ATmakeList0();

  for(;!ATisEmpty(DataTermList2);DataTermList2=ATgetNext(DataTermList2)){
    ATermAppl DataTerm2=ATAgetFirst(DataTermList2);
    ATermAppl NewDataTerm0=gstcTraverseVarConstD(Vars, ATAgetArgument(DataTerm2,0));
    if(!NewDataTerm0) {throw;}
    ATermAppl NewDataTerm1=gstcTraverseVarConstD(Vars, ATAgetArgument(DataTerm2,1));
    if(!NewDataTerm1) {throw;}
    NewDataTermList2=ATinsert(NewDataTermList2,(ATerm)gsMakeBagEnumElt(NewDataTerm0,NewDataTerm1));
  }
  return ATreverse(NewDataTermList2);
 finally:
  return Result;
}
