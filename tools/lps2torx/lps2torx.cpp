// Author(s): Muck van Weerdenburg
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lps2torx.cpp

#define NAME "lps2torx"
#define AUTHOR "Muck van Weerdenburg"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <getopt.h>
#include <aterm2.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include "mcrl2/core/struct.h"
#include "mcrl2/core/print.h"
#include "mcrl2/lps/nextstate.h"
#include "mcrl2/data/enum.h"
#include "mcrl2/data/rewrite.h"
#include "mcrl2/lps/dataelm.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "mcrl2/utilities/version_info.h"

using namespace ::mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2::core::detail;
using namespace mcrl2::data;
using namespace std;

#define is_tau(x) ATisEmpty((ATermList) ATgetArgument(x,0))
          
void print_torx_action(ostream &os, ATermAppl mact)
{
  if ( is_tau(mact) )
  {
    os << "tau";
  } else {
    ATermAppl act = (ATermAppl) ATgetFirst((ATermList) ATgetArgument(mact,0));
    PrintPart_CXX(cout,ATgetArgument(act,0), ppDefault);
    ATermList dl = (ATermList) ATgetArgument(act,1);
    for (; !ATisEmpty(dl); dl=ATgetNext(dl))
    {
      cout << "!";
      PrintPart_CXX(cout,ATgetFirst(dl), ppDefault);
    }
  }
}
  
typedef struct {
  int action;
  int state;
} index_pair;

class torx_data
{
  private:
    ATermIndexedSet stateactions;
    ATermTable state_indices;
    AFun fun_trip;
    unsigned int num_indices;

    ATerm triple(ATerm one, ATerm two, ATerm three)
    {
      return (ATerm) ATmakeAppl3(fun_trip,one,two,three);
    }

    ATerm third(ATerm trip)
    {
      return ATgetArgument((ATermAppl) trip,2);
    }

  public:
    torx_data(unsigned int initial_size)
    {
      stateactions = ATindexedSetCreate(initial_size,50);
      state_indices = ATtableCreate(initial_size,50);
      fun_trip = ATmakeAFun("@trip@",2,ATfalse);
      ATprotectAFun(fun_trip);
      num_indices = 0;
    }

    ~torx_data()
    {
      ATunprotectAFun(fun_trip);
      ATtableDestroy(state_indices);
      ATindexedSetDestroy(stateactions);
    }

    index_pair add_action_state(ATerm from, ATerm action, ATerm to)
    {
      ATbool is_new;
      index_pair p;

      p.action = ATindexedSetPut(stateactions,triple(from,action,to),&is_new);
      if ( is_new == ATtrue )
      {
        num_indices = num_indices + 1;
      }

      ATerm i;
      if ( (i = ATtableGet(state_indices,to)) == NULL )
      {
        ATtablePut(state_indices,to,(ATerm) ATmakeInt(p.action));
        p.state = p.action;
      } else {
        p.state = ATgetInt((ATermInt) i);
      }

      return p;
    }

    ATerm get_state(unsigned int index)
    {
      if ( index < num_indices )
      {
        return third(ATindexedSetGetElem(stateactions,index));
      } else {
        return NULL;
      }
    }
};

static void print_help_suggestion(FILE *f, char *Name)
{
  fprintf(f,"Try '%s --help' for more information.\n",Name);
}

static void print_help(FILE *f, char *Name)
{
  fprintf(f,
    "Usage: %s [OPTION]... [INFILE]\n"
    "Provide a TorX explorer interface to the LPS in INFILE. If INFILE is not\n"
    "supplied, stdin is used.\n"
    "\n"
    "The LPS can be explored using TorX as described in torx_explorer(5).\n"
    "\n"
    "Options:\n"
    "  -h, --help              display this help message and terminate\n"
    "      --version           display version information and terminate\n"
    "  -q, --quiet             do not display any unrequested information\n"
    "  -v, --verbose           display concise intermediate messages\n"
    "  -d, --debug             display detailed intermediate messages\n"
    "  -f, --freevar           do not replace free variables in the LPS with dummy\n"
    "                          values\n"
    "  -y, --dummy             replace free variables in the LPS with dummy values\n"
    "                          (default)\n"
    "  -u, --unused-data       do not remove unused parts of the data specification\n"
    "  -c, --vector            store state in a vector (fastest, default)\n"
    "  -r, --tree              store state in a tree (for memory efficiency)\n"
    "  -RNAME, --rewriter=NAME use rewriter NAME (default 'jitty')\n"
    "\n"
    "Report bugs at <http://www.mcrl2.org/issuetracker>.\n"
    , Name);
}

int main(int argc, char **argv)
{
  MCRL2_ATERM_INIT(argc, argv)

  FILE *SpecStream;
  ATermAppl Spec;
  #define sopts "hqvdfyucrR:"
  struct option lopts[] = {
    { "help",            no_argument,       NULL, 'h' },
    { "version",         no_argument,       NULL, 0   },
    { "quiet",           no_argument,       NULL, 'q' },
    { "verbose",         no_argument,       NULL, 'v' },
    { "debug",           no_argument,       NULL, 'd' },
    { "freevar",         no_argument,       NULL, 'f' },
    { "dummy",           no_argument,       NULL, 'y' },
    { "unused-data",     no_argument,       NULL, 'u' },
    { "vector",          no_argument,       NULL, 'c' },
    { "tree",            no_argument,       NULL, 'r' },
    { "rewriter",        required_argument, NULL, 'R' },
    { 0, 0, 0, 0 }
  };

  bool quiet = false;
  bool verbose = false;
  bool debug = false;
  RewriteStrategy strat = GS_REWR_JITTY;
  bool usedummies = true;
  bool removeunused = true;
  int stateformat = GS_STATE_VECTOR;
  int opt;
  while ( (opt = getopt_long(argc,argv,sopts,lopts,NULL)) != -1 )
  {
    switch ( opt )
    {
      case 'h':
        print_help(stdout, argv[0]);
        return 0;
      case 0:
        print_version_information(NAME, AUTHOR);
        return 0;
      case 'q':
        quiet = true;
        break;
      case 'v':
        verbose = true;
        break;
      case 'd':
        debug = true;
        break;
      case 'f':
        usedummies = false;
        break;
      case 'y':
        usedummies = true;
        break;
      case 'u':
        removeunused = false;
        break;
      case 'c':
        stateformat = GS_STATE_VECTOR;
        break;
      case 'r':
        stateformat = GS_STATE_TREE;
        break;
      case 'R':
        strat = RewriteStrategyFromString(optarg);
        if ( strat == GS_REWR_INVALID )
        {
          gsErrorMsg("invalid rewrite strategy '%s'\n",optarg);
          return 1;
        }
        break;
      default:
        break;
    }
  }
  if ( quiet && verbose )
  {
    gsErrorMsg("options -q/--quiet and -v/--verbose cannot be used together\n");
    return 1;
  }
  if ( quiet && debug )
  {
    gsErrorMsg("options -q/--quiet and -d/--debug cannot be used together\n");
    return 1;
  }
  if ( quiet )
    gsSetQuietMsg();
  if ( verbose )
    gsSetVerboseMsg();
  if ( debug )
    gsSetDebugMsg();
  
  if ( argc-optind > 1 )
  {
    print_help_suggestion(stderr,argv[0]);
    return 1;
  }

  char *SpecFileName;
  if ( argc-optind == 1 )
  {
    SpecFileName = argv[optind];
    if ( (SpecStream = fopen(SpecFileName, "rb")) == NULL )
    {
      gsErrorMsg("could not open input file '%s' for reading: ", SpecFileName);
      perror(NULL);
      return 1;
    }
  } else {
    SpecFileName = NULL;
    SpecStream = stdin;
  }
  if ( SpecStream == stdin )
    gsVerboseMsg("reading LPS from stdin\n");
  else
    gsVerboseMsg("reading LPS from '%s'\n", SpecFileName);
  Spec = (ATermAppl) ATreadFromFile(SpecStream);
  if ( Spec == NULL )
  {
    if ( SpecStream == stdin )
      gsErrorMsg("could not read LPS from stdin\n");
    else
      gsErrorMsg("could not read LPS from '%s'\n", SpecFileName);
    return 1;
  }
  assert(Spec != NULL);
  if (!gsIsSpecV1(Spec)) {
    if ( SpecStream == stdin )
      gsErrorMsg("stdin does not contain an LPS\n");
    else
      gsErrorMsg("'%s' does not contain an LPS\n", SpecFileName);
    return 1;
  }
  assert(gsIsSpecV1(Spec));

  if ( removeunused )
  {
    gsVerboseMsg("removing unused parts of the data specification.\n");
    Spec = removeUnusedData(Spec);
  }

  gsVerboseMsg("initialising...\n");
  torx_data td(10000);

  NextState *nstate = createNextState(
    Spec,
    !usedummies,
    stateformat,
    createEnumerator(
      data_specification(ATAgetArgument(Spec,0)),
      createRewriter(data_specification(ATAgetArgument(Spec,0)),strat),
      true
    ),
    true
  );

  ATerm initial_state = nstate->getInitialState();

  ATerm dummy_action = (ATerm) ATmakeAppl0(ATmakeAFun("@dummy_action@",0,ATfalse));
  td.add_action_state(initial_state,dummy_action,initial_state);

  gsVerboseMsg("generating state space...\n");

  NextStateGenerator *nsgen = NULL;
  bool err = false;
  bool notdone = true;
  while ( notdone && !cin.eof() )
  {
    string s;

    cin >> s;
    if ( s.size() != 1 )
    {
	    cout << "A_ERROR UnknownCommand: unknown or unimplemented command '" << s << "'" << endl;
	    continue;
    }

    switch ( s[0] )
    {
      case 'r': // Reset
        // R event TAB solved TAB preds TAB freevars TAB identical
        cout << "R 0\t1\t\t\t" << endl;
        break;
      case 'e': // Expand
        {
        int index;
        ATerm state;
        
        cin >> index;
        state = td.get_state( index );
	if ( state == NULL )
	{
		cout << "E0 value " << index << " not valid" << endl;
		break;
	}
    
        cout << "EB" << endl;
        nsgen = nstate->getNextStates(state,nsgen);
        ATermAppl Transition;
        ATerm NewState;
        while ( nsgen->next(&Transition,&NewState) )
        {
          index_pair p;
    
          p = td.add_action_state(state,(ATerm) Transition,NewState);

          // Ee event TAB visible TAB solved TAB label TAB preds TAB freevars TAB identical
          cout << "Ee " << p.action << "\t" << (is_tau(Transition)?0:1) << "\t1\t";
          print_torx_action(cout,Transition);
          cout << "\t\t\t";
          if ( p.action != p.state )
          {
            cout << p.state;
          }
          cout << endl;
        }
        cout << "EE" << endl;

        if ( nsgen->errorOccurred() )
        {
          err = true;
          notdone = false;
        }
        break;
        }
      case 'q': // Quit
        cout << "Q" << endl;
        notdone = false;
        break;
      default:
	cout << "A_ERROR UnknownCommand: unknown or unimplemented command '" << s << "'" << endl;
        break;
    }
  }
  delete nsgen;
  delete nstate;
}
