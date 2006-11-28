%{
#include "fsmparser.h"
#include "fsmlexer.cpp"

// Global variables

LTS* fsmparserlts = NULL;

ATermList stateVector;
ATermList valueTable;
ATermList stateId;
ATermList typeValues;
ATermAppl typeId;

std::vector< State* > states;

AFun const_ATtypeid;
AFun const_ATparmid;
AFun const_ATvalue;
AFun const_ATstate;
AFun const_ATparam;

// Function declarations

void fsmerror(const char* c);
char* intToCString(int i);
%}

%union {
  ATermAppl aterm;
  int number;
}

%start fsm_file

%token EOLN SECSEP LPAR RPAR FANIN FANOUT NODENR ARROW
%token <number> NUMBER
%token <aterm> ID QUOTED
%type  <aterm> type_name type_name1 action

%%

fsm_file : 
	params
	  { 
	    valueTable = ATreverse( valueTable );
	    stateId = ATreverse( stateId );
	    fsmparserlts->setStateVectorSpec( stateId )
	  }
	SECSEP EOLN
	states
	SECSEP EOLN transitions
	;

// --------- Section containing the state vector ----------

params : 
	/* empty */
	|
	params param EOLN
	;

param :
	ID
	cardinality type_def
	  {
	    stateId = ATinsert( stateId, (ATerm)ATmakeAppl2( const_ATparmid,
	      (ATerm)$1, (ATerm)typeId ) )
	  }
	|
	FANIN cardinality type_def_ignore
	|
	FANOUT cardinality type_def_ignore
	|
	NODENR cardinality type_def_ignore
	;

cardinality :
	LPAR NUMBER RPAR
	;
	
type_def : 
	type_name
	  { 
	    typeValues = ATempty
	  }
	type_values
	  { 
	    typeValues = ATreverse( typeValues );
	    typeId = ATmakeAppl2( const_ATtypeid, (ATerm)$1, (ATerm)typeValues );
	    valueTable = ATinsert( valueTable, (ATerm)typeValues )
	  }
	;

type_name :
	/* empty */
	  { $$ = ATmakeAppl0( ATmakeAFun( "", 0, ATfalse ) ) }
	|
	type_name1
	  { $$ = $1 }
	| 
	type_name ARROW type_name1
	  {
	    std::string result = static_cast<std::string> ( ATwriteToString( (ATerm)$1 ) )
	      + "->" + static_cast<std::string> ( ATwriteToString( (ATerm)$3 ) );
	    $$ = ATmakeAppl0( ATmakeAFun( result.c_str(), 0, ATfalse ) )
	  }
	;

type_name1 :
	ID
	  { $$ = $1 }
	|
	LPAR type_name RPAR
	  {
	    std::string result = "(" + static_cast<std::string> ( ATwriteToString(
	      (ATerm)$2) ) + ")";
	    $$ = ATmakeAppl0( ATmakeAFun( result.c_str(), 0, ATfalse ) )
	  }
	;


type_values :
	/* empty */
	|
	type_values type_value
	;

type_value :
	QUOTED
	  { typeValues = ATinsert( typeValues, (ATerm)ATmakeAppl2(
	      const_ATvalue, (ATerm)$1, (ATerm)ATmakeInt( ATgetLength( typeValues ) ) ) ) }
	;

type_def_ignore : 
	type_name_ignore
	type_values_ignore
	;

type_name_ignore :
	/* empty */
	| type_name_ignore1
	| type_name_ignore ARROW type_name_ignore1
	;

type_name_ignore1 :
	ID
	| LPAR type_name_ignore RPAR
	;

type_values_ignore :
	/* empty */
	| type_values_ignore type_value_ignore
	;

type_value_ignore :
	QUOTED
	;

// ----------- Section containing the states ---------

states :
	/* empty */
	|
	states
	state
	  {
	    stateVector = ATreverse( stateVector ); 
	    State* s = new State( stateVector );
	    fsmparserlts->addState( s );
	    states.push_back( s );
	    if ( states.size() == 1 ) 
	      fsmparserlts->setInitialState( s );
	    stateVector = ATempty
	  }
	EOLN
	;

state :
	/* empty */
	|
	state NUMBER
	  { 
	    int paramNo = ATgetLength( stateVector );
	    if ( paramNo < ATgetLength( valueTable ) )
	    {
	      stateVector = ATinsert( stateVector, ATelementAt(
		(ATermList)ATelementAt( valueTable, paramNo ), $2 ) );
	    }
	  }
	;

// ---------- Section containing the transitions ----------

transitions:
	/* empty */
	|
	transitions transition
	EOLN
	;

transition:
	NUMBER NUMBER action
	  {
	    State* frState = states[$1-1];
	    State* toState = states[$2-1];
	    Transition* t = new Transition( frState, toState, (ATerm)$3 );
	    fsmparserlts->addTransition( t );
	    if ( $1 != $2 )
	    {
	      frState->addOutTransition( t );
	      toState->addInTransition( t );
	    }
	    else
	    {
	      frState->addLoop( t );
	    }
	  }
	;

action :
	/* empty */
	  { $$ = ATmakeAppl0( ATmakeAFun( "", 0, ATfalse ) ) }
	|
	QUOTED
	  { $$ = $1 }
	;

%%

int fsmwrap()
{
  return 1;
}

void fsmerror(const char *str)
{
  throw std::string( "Parse error: " + std::string(str) + " token \"" + std::string(fsmtext) +
    "\" at line " + std::string(intToCString( lineNo )) + " position " +
    std::string(intToCString( posNo )) );
}
 
void parseFSMfile( std::string fileName, LTS* const lts )
{
  // reset the lexer position variables
  lineNo=1;
  posNo=1;
  
  FILE* infile = fopen(fileName.c_str(),"r");
  if ( infile == NULL )
    throw std::string( "Cannot open file for reading:\n" + fileName );
  else
  {
    // INITIALISE
    fsmparserlts = lts;
    fsmrestart( infile );
    
    const_ATtypeid = ATmakeAFun( "TypeId", 2, ATfalse );
    ATprotectAFun( const_ATtypeid );
    const_ATparmid = ATmakeAFun( "ParamId", 2, ATfalse );
    ATprotectAFun( const_ATparmid );
    const_ATvalue = ATmakeAFun( "Value", 2, ATfalse );
    ATprotectAFun( const_ATvalue );
    const_ATstate = ATmakeAFun( "State", 2, ATfalse );
    ATprotectAFun( const_ATstate );
    const_ATparam = ATmakeAFun( "Param", 2, ATfalse );
    ATprotectAFun( const_ATparam );
    stateVector = ATempty;
    ATprotectList( &stateVector );
    valueTable = ATempty;
    ATprotectList( &valueTable );
    stateId = ATempty;
    ATprotectList( &stateId );
    typeValues = NULL;
    ATprotectList( &typeValues );
    typeId = NULL;
    ATprotectAppl( &typeId );
    

    // PARSE
    fsmparse();
    
    // CLEAN UP
    ATunprotectAFun( const_ATtypeid );
    ATunprotectAFun( const_ATparmid );
    ATunprotectAFun( const_ATvalue );
    ATunprotectAFun( const_ATstate );
    ATunprotectAFun( const_ATparam );
    ATunprotectList( &stateVector );
    ATunprotectList( &valueTable );
    ATunprotectList( &stateId );
    ATunprotectList( &typeValues );
    ATunprotectAppl( &typeId );
    
    states.clear();
    fsmparserlts = NULL;
  }
} 

char* intToCString( int i )
{
    std::ostringstream oss;
    oss << i;
    return (char*)oss.str().c_str();
}
