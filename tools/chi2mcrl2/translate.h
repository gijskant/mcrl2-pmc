// Author(s): Frank Stappers
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file translate.h

#ifndef TRANSLATE_H
#define TRANSLATE_H

#include <cstring>
#include <cmath>
#include <iostream>
#include "mcrl2/aterm/aterm2.h"
#include "mcrl2/core/messaging.h"
#include <cstdio>
#include <vector>
#include <map>
#include <sstream>
#include <list>
#include <set>
#include <stack>

#define RPV RecProcessVariable
#define RVT RecVariableType
#define RAT RecActionTransition
#define RS RecStreams
#define RSP RecStreamPos
#define RPI RecParenthesisInfo
#define RPVS RecProcessVectors
#define RPC RecProcessChannels
#define RC  RecChannel

// XXX Remove
using namespace aterm;

struct t_options
{
  std::string infilename;
  std::string outfilename;
  bool no_statepar;
};

typedef struct
{
  std::string Name;
  std::string Type;
  std::string InitValue;
} RecProcessVariable;

typedef struct
{
  std::string Name;
  std::string Type;
  std::string HashCount;
} RecProcessChannels;

typedef struct
{
  std::string Name;
  std::string Type;
} RecVariableType;

typedef struct
{
  size_t state;
  int holdsForState;
} RecStreams;

typedef struct
{
  int stream;
  size_t position;
} RecStreamPos;

//Information per parenthesis
typedef struct
{
  std::set<size_t> endstates;
  bool looped;
  bool guardedloop;
  size_t  begin_state;
  size_t  end_state;
  std::set<int>  streams;
  bool parallel;
  bool alternative;

} RecParenthesisInfo;

//Information per transition
typedef struct
{
  size_t state;
  int stream;
  int originates_from_stream;
  bool terminate;
  size_t parenthesis_level;
  bool looped_state;
  bool guardedloop;
  std::string guard;
  std::string action;
  std::map<std::string, std::string> vectorUpdate; // First:  Identifier Variable
  // Second: Expression
  size_t nextstate;
} RecActionTransition;

typedef struct
{
  std::vector<RPV> DeclarationVariables;
  std::vector<RPV> SpecificationVariables;
  std::vector<RPC> DeclarationChannels;
  size_t NumberOfStreams;
} RecProcessVectors;

typedef struct
{
  std::string send_end;
  std::string recv_end;
  std::string Type;
} RecChannel;

template <class T>
inline std::string to_string(const T& t)
{
  std::stringstream ss;
  ss << t;
  return ss.str();
}

template < >
inline std::string to_string(const std::string& t)
{
  return t;
}

class CAsttransform
{
  public:
    CAsttransform()
      : parenthesis_level(0),
        terminate(true),
        state(ATERM_NON_EXISTING_POSITION),
        next_state(ATERM_NON_EXISTING_POSITION),
        loop(false),
        guardedloop(false),
        guardedStarBeginState(0),
        stream_number(0),
        originates_from_stream(0),
        alternative(false),
        parallel(false),
        no_statepar(false)
    {}

    bool translator(ATermAppl ast);
    bool set_options(t_options options);
    std::string getResult();
  private:
    std::string manipulateProcess(ATermAppl input);
    std::string mcrl2_result;
    bool StrcmpIsFun(const char* str, ATermAppl aterm);
    std::string variable_prefix; //prefix stores the name of the process globally
    size_t parenthesis_level;
    std::pair<std:: vector<RVT>, std::vector<RPC> > manipulateDeclaredProcessDefinition(ATermAppl input);
    std::vector<RVT> manipulateDeclaredProcessVariables(ATermList input);

    std::pair<std::vector<RPV>, std::vector<RPC> > manipulateProcessSpecification(ATermAppl input);
    std::vector<RPV> manipulateProcessVariableDeclarations(ATermList input);

    std::vector<std::string> getVariablesNamesFromList(ATermList input);
    std::vector<RPC> manipulateDeclaredProcessChannels(ATermList input);

    std::string manipulateExpression(ATermAppl input);
    void manipulateStatements(ATermAppl input);
    std::map<std::string, std::string> manipulateAssignmentStat(ATermList input_id, ATermList input_exp);
    std::vector<std::string> getExpressionsFromList(ATermList input);
    bool onlyIdentifiersInExpression(ATermList input);
    std::map<int, std::set<int> > affectedStreamMap;

    std::string manipulateExplicitTemplates(ATermList input);
    std::vector<RPV> ProcessVariableMap;
    std::vector<RPC> ProcessChannelMap;

    std::string processType(ATermAppl input);

    std::vector<RAT> transitionSystem;

    bool terminate;  //terminate per parenthesis level
    size_t state;
    size_t next_state;

    //Begin_state: used to deterime the beginstates per parenthesis level
    std::map<size_t, size_t> begin_state; //first:  parenthesis level
    //second: begin state
    std::map<size_t, size_t> end_state;   //first:  parenthesis level
    //second: end state

    std::map<size_t, std::set<size_t> > endstates_per_parenthesis_level;
    std::map<size_t, std::vector<RPI>  > info_per_parenthesis_level_per_parenthesis;

    size_t determineEndState(std::set<size_t> org_set, size_t lvl);
    bool loop;  //Variable to indicate if a parenthesis_level is looped
    bool guardedloop; //Variable to indicatie if a parenthesis_level is a guarded loop
    std::string guardedStarExpression; //Variable that contains the guard of *>
    size_t guardedStarBeginState; //Variable to indicatie the state where the *> starts
    bool transitionexists(RAT transition, std::vector<RAT> transitionvector);

    int stream_number; //Variable used for indicate the current stream ( Parallel )
    int originates_from_stream;  //Variable to indicate which stream started the current stream (Parrallel)
    std::set<int> streams_per_parenthesis_level; // Set that contains the streams for a certain parenthesis_level
    std::set<int> all_streams; //Set of streams for a certain parenthesis_level

    bool alternative; //Variable used to indicate if a parenthesis_level is alternative
    bool parallel; //Variable used to indicate if a parenthesis_level is parallel

    void manipulateModel(ATermAppl input);
    // std::vector<int> bypass;
    std::vector<RPV> manipulateModelSpecification(ATermAppl input);
    void manipulateModelStatements(ATermAppl input);

    std::string initialisation;
    std::map<std::string, RPVS> ProcessForInstantation;

    //Variable that constructs paramteres for a Chi Process
    std::vector<ATerm> ChiDeclParameters;

    //Variable that constructs paramters for an entire Chi model
    std::map<std::string, std::vector<ATerm> > Chi_interfaces;

    //These channels will form define the multi-actions for mCRL2-specification
    // STRING [ChannelID] x INT [HASH] -> RC
    std::map<std::pair < std::string, int >, RC> Channels;

    std::map<std::string, int > instantation_counter;

    std::map<ATermAppl, std::string> structset;
    std::string printStructset(ATermAppl input);
    std::string initialValueVariable(std::string Type);

    std::string processValue(ATermAppl input);

    std::string prefixmCRL2spec;

    std::set<std::string> takeTypes;
    std::set<std::string> dropTypes;
    std::set<std::string> union_setTypes;
    std::set<std::string> intersection_setTypes;
    std::set<std::string> sub_setTypes;
    std::set<std::string> dif_setTypes;
    std::set<std::string> equal_setTypes;

    std::set<std::string> InstantiatedHashedChannels;

    std::set<std::string> DeclaredTypesForChannels;

    bool TypeChecking(ATermAppl arg1, ATermAppl arg2);

    //OPTIONS
    bool no_statepar;

}
;


#endif
