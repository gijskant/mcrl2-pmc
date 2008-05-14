// Author(s): Muck van Weerdenburg
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2i.cpp

#define NAME "mcrl2i"
#define AUTHOR "Muck van Weerdenburg"

#include <iostream>
#include <sstream>
#include <string>
#include <cctype>

#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <aterm2.h>

#include "mcrl2/data/rewrite.h"
#include "mcrl2/data/enum.h"
#include "mcrl2/core/struct.h"
#include "mcrl2/core/print.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/core/detail/parse.h"
#include "mcrl2/core/detail/typecheck.h"
#include <mcrl2/core/detail/data_implementation.h>
#include <mcrl2/core/detail/data_reconstruct.h>
#include "mcrl2/lps/specification.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "mcrl2/utilities/command_line_interface.h" // after messaging.h and rewrite.h

using namespace std;
using namespace mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2::core::detail;
using namespace mcrl2::lps;

char help_message[] = "At the prompt any mCRL2 data expression can be given. This term will be "
                      "rewritten to normal form and printed. Also, one can assign values to declared "
                      "variables by writing x := v, for variable x and value v. These variables can "
                      "then be used in expressions. Besides this, the prompt accepts the following "
                      "commands (where VARLISt is of the form x,y,...: S; v,w,...: T):\n"
                      "  :h[elp]                         print this help message\n"
                      "  :q[uit]                         quit\n"
                      "  :t[ype] EXPRESSION              print type of EXPRESSION\n"
                      "  :v[ar] VARLIST                  declare variables in VARLIST\n"
                      "  :r[ewriter] STRATEGY            use STRATEGY for rewriting\n"
                      "  :s[solve] <VARLIST> EXPRESSION  give all valuations of the variables in\n"
                      "                                  VARLIST that satisfy EXPRESSION\n";

static Rewriter *rewr;
static Enumerator *e;
static ATermTable variables;
static ATermTable assignments;

static void updated_specification(specification spec)
{
  gsVerboseMsg("reinitialising rewriter and enumerator...\n");
  RewriteStrategy strat = rewr->getStrategy();
  delete e;
  delete rewr;
  rewr = createRewriter(spec.data(), strat);
  e = createEnumerator(spec.data(),rewr);
}

static bool refresh_specification(specification old_spec, specification new_spec)
{
  if ( old_spec == new_spec )
  {
    return false;
  }

  updated_specification(new_spec);

  return true;
}

static void clear_rewr_substs(ATermList vars)
{
  if ( vars == NULL )
  {
    vars = ATtableKeys(assignments);
  }
  for (; !ATisEmpty(vars); vars=ATgetNext(vars))
  {
    rewr->clearSubstitution(ATAgetFirst(vars));
  }
}

static void reset_rewr_substs()
{
  ATermList vars = ATtableKeys(assignments);
  for (; !ATisEmpty(vars); vars=ATgetNext(vars))
  {
    ATermAppl var = ATAgetFirst(vars);
    rewr->setSubstitution(var,ATtableGet(assignments,(ATerm) var));
  }
}

static string trim_spaces(const string &str)
{
  // function from http://www.codeproject.com/vcpp/stl/stdstringtrim.asp
  string s(str);
  string::size_type pos = s.find_last_not_of(' ');
  if(pos != string::npos)
  {
    s.erase(pos + 1);
    pos = s.find_first_not_of(' ');
    if (pos != string::npos)
      s.erase(0, pos);
  } else
    s.erase(s.begin(), s.end());

  return s;
}

static bool parse_var_decl(string decl, ATermList *varlist, specification &spec)
{
    string::size_type semi_pos = decl.find(':');
    if ( semi_pos == string::npos )
    {
      gsErrorMsg("invalid type declaration '%s'\n",decl.c_str());
      return false;
    }

    stringstream ss(decl.substr(semi_pos+1));
    
    ATermAppl sort = parse_sort_expr(ss);
    if ( sort == NULL )
      return false;

    gsDebugMsg("parsed: %T\n",sort);

    ATermAppl reconstructed_spec = reconstruct_spec(spec);
    sort = type_check_sort_expr(sort,reconstructed_spec);
    if ( sort == NULL )
      return false;

    gsDebugMsg("type checked: %T\n",sort);

    sort = implement_data_sort_expr(sort,reconstructed_spec);
    if ( sort == NULL )
      return false;

    if ( refresh_specification(spec,specification(reconstructed_spec)) )
    {
      spec = specification(reconstructed_spec);
    }

    gsDebugMsg("data implemented: %T\n",sort);

    string names = decl.substr(0,semi_pos) + ',';
    string::size_type i;
    while ( ( i = names.find(',') ) != string::npos )
    {
      *varlist = ATinsert(*varlist,(ATerm) gsMakeDataVarId(gsString2ATermAppl(trim_spaces(names.substr(0,i)).c_str()),sort));
      names = names.substr(i+1);
    }

    return true;
}

static bool parse_var_decl_list(string decl_list, ATermList *varlist, specification &spec)
{
  decl_list += ';';
  string::size_type pos;
  while ( (pos = decl_list.find(';')) != string::npos )
  {
    string decl(decl_list.substr(0,pos));
    decl_list = decl_list.substr(pos+1);

    if ( !parse_var_decl(decl,varlist,spec) )
      return false;
  }

  return true;
}

static ATermList parse_varlist_from_string(string &s, specification &spec)
{
  string::size_type start = s.find('<');
  if ( start == string::npos )
  {
    gsErrorMsg("opening bracket for variable list not found\n");
    return NULL;
  }
  string::size_type end = s.find('>',start);
  if ( end == string::npos )
  {
    gsErrorMsg("closing bracket for variable list not found\n");
    return NULL;
  }
  string varlist = s.substr(start+1,end-start-1);
  s = s.substr(end+1);

  ATermList result = ATmakeList0();
  if ( !parse_var_decl_list(varlist,&result,spec) )
    return NULL;
    
  gsDebugMsg("variable list: %T\n",result);

  return result;
}

static ATermAppl parse_term(string &term_string, specification &spec, ATermList vars = NULL)
{
  stringstream ss(term_string);

  ATermAppl term = parse_data_expr(ss);
  if ( term == NULL )
    return NULL;

  gsDebugMsg("parsed: %T\n",term);

  ATermList save_assignments = ATmakeList0();
  if ( vars != NULL )
  {
    for (ATermList l=vars; !ATisEmpty(l); l=ATgetNext(l))
    {
      ATermAppl var = ATAgetFirst(l);
      if ( ATtableGet(variables,ATgetArgument(var,0)) != NULL )
      {
        ATerm old_val = rewr->getSubstitution(var);
        rewr->clearSubstitution(var);
        save_assignments = ATinsert(save_assignments,(ATerm) gsMakeSubst((ATerm) var,old_val));
      }
      ATtablePut(variables,ATgetArgument(var,0),ATgetArgument(var,1));
    }
  }

  // type checking and data implementation of a data expression use a
  // specification before data implementation.
  ATermAppl reconstructed_spec = reconstruct_spec(spec);
  term = type_check_data_expr(term,NULL,reconstructed_spec,variables);
  if ( vars != NULL )
  {
    for (ATermList l=vars; !ATisEmpty(l); l=ATgetNext(l))
    {
      ATtableRemove(variables,ATgetArgument(ATAgetFirst(l),0));
    }
    for (; !ATisEmpty(save_assignments); save_assignments=ATgetNext(save_assignments))
    {
      ATermAppl subst = ATAgetFirst(save_assignments);
      ATermAppl var = ATAgetArgument(subst,0);
      rewr->setSubstitution(var,ATgetArgument(subst,1));
      ATtablePut(variables,ATgetArgument(var,0),ATgetArgument(var,1));
    }
  }
  if ( term == NULL )
    return NULL;

  gsDebugMsg("type checked: %T\n",term);

  term = implement_data_data_expr(term,reconstructed_spec);
  if ( refresh_specification(spec,specification(reconstructed_spec)) )
  {
    spec = specification(reconstructed_spec);
  }
  if ( term == NULL )
    return NULL;

  gsDebugMsg("data implemented: %T\n",term);

  return term;
}

static void declare_variables(string &vars, specification &spec)
{
  ATermList varlist = ATmakeList0();
  if ( ! parse_var_decl_list(vars,&varlist,spec) )
    return;

  for (; !ATisEmpty(varlist); varlist=ATgetNext(varlist))
  {
    ATermAppl var = ATAgetFirst(varlist);
    ATtablePut(variables,ATgetArgument(var,0),ATgetArgument(var,1));
  }
}

struct tool_options_type {
  std::string     infilename;
  RewriteStrategy strategy;
};

static tool_options_type parse_command_line(int ac, char** av) {
  interface_description clinterface(av[0], NAME, AUTHOR, "[OPTION]... [INFILE]\n"
    "Evaluate expressions using the data specification of the LPS in INFILE via a\n"
    "text-based interface.\n"
    "\n" + std::string(help_message));

  clinterface.add_rewriting_options();

  command_line_parser parser(clinterface, ac, av);

  tool_options_type options;

  options.strategy = parser.option_argument_as< RewriteStrategy >("rewriter");

  if (0 < parser.arguments.size()) {
    options.infilename = parser.arguments[0];
  }
  if (1 < parser.arguments.size()) {
    parser.error("too many file arguments");
  }

  return options;
}

int main(int argc, char **argv)
{
  MCRL2_ATERM_INIT(argc, argv)

  try {
    tool_options_type options(parse_command_line(argc, argv));

    ATermAppl raw_specification;

    if (options.infilename.empty()) {
      gsVerboseMsg("reading LPS from stdin\n");

      raw_specification = (ATermAppl) ATreadFromFile(stdin);

      if (raw_specification == 0) {
        throw std::runtime_error("could not read LPS from '" + options.infilename + "'");
      }
      if (!gsIsSpecV1(raw_specification)) {
        throw std::runtime_error("stdin does not contain an LPS");
      }
    }
    else {
      gsVerboseMsg("reading LPS from '%s'\n", options.infilename.c_str());

      FILE *in_stream = fopen(options.infilename.c_str(), "rb");

      if (in_stream == 0) {
        throw std::runtime_error("could not open input file '" + options.infilename + "' for reading");
      }

      raw_specification = (ATermAppl) ATreadFromFile(in_stream);

      fclose(in_stream);

      if (raw_specification == 0) {
        throw std::runtime_error("could not read LPS from '" + options.infilename + "'");
      }
      if (!gsIsSpecV1(raw_specification)) {
        throw std::runtime_error("'" + options.infilename + "' does not contain an LPS");
      }
    }

    specification spec(raw_specification);

    gsMessage("mCRL2 interpreter (type :h for help)\n");
 
    rewr = createRewriter(spec.data(), options.strategy);
    e = createEnumerator(spec.data(),rewr);
    variables = ATtableCreate(50,50);
    assignments = ATtableCreate(50,50);
 
    bool notdone = true;
    while ( notdone )
    {
      while ( true )
      {
        string s;
 
        (cout << "? ").flush();
        getline(cin, s);
        if ( (s.length() > 0) && (s[s.length()-1] == '\r') )
        { // remove CR
          s.resize(s.length()-1);
        }
        
        if ( cin.eof() )
        {
          cout << endl;
          notdone = false;
          break;
        }
 
        if ( s.substr(0,1) == ":" )
        {
          s = s.substr(1);
          if ( (s == "q") || (s == "quit") )
          {
            if ( cin.eof() )
              cout << endl;  
            notdone = false;
            break;
          } else if ( (s == "h") || (s == "help") )
          {
            cout << help_message;
          } else if ( (s.substr(0,2) == "r ") || (s.substr(0,9) == "rewriter ") ) 
          {
            if ( s.substr(0,2) == "r " )
              s = s.substr(2);
            else
              s = s.substr(9);
            options.strategy = RewriteStrategyFromString(s.c_str());
            if ( options.strategy == GS_REWR_INVALID )
            {
              gsErrorMsg("invalid rewrite strategy '%s', ignoring command\n",s.c_str());
            } else {
              updated_specification(spec);
            }
          } else if ( (s.substr(0,2) == "t ") || (s.substr(0,5) == "type ") )
          {
            if ( s.substr(0,2) == "t " )
              s = s.substr(2);
            else
              s = s.substr(5);
            ATermAppl term = parse_term(s,spec);
            if ( term != NULL )
            {
              gsprintf("%P\n",gsGetSort(term));
            }
          } else if ( (s.substr(0,2) == "v ") || (s.substr(0,4) == "var ") )
          {
            if ( s.substr(0,2) == "v " )
              s = s.substr(2);
            else
              s = s.substr(4);
            declare_variables(s,spec);
          } else if ( (s.substr(0,2) == "s ") || (s.substr(0,6) == "solve ") )
          {
            if ( s.substr(0,2) == "s " )
              s = s.substr(2);
            else
              s = s.substr(6);
            ATermList vars = parse_varlist_from_string(s,spec);
            if ( vars == NULL )
              break;
            ATermAppl term = parse_term(s,spec,vars);
            if ( term != NULL )
            {
              if ( gsGetSort(term) != gsMakeSortExprBool() )
              {
                gsErrorMsg("expression is not of sort Bool\n");
                break;
              }
              static EnumeratorSolutions *sols = NULL;
              clear_rewr_substs(vars);
              sols = e->findSolutions(vars,rewr->toRewriteFormat(term),false,sols);
              ATermList sol;
              while ( sols->next(&sol) )
              {
                gsprintf("[");
                for (; !ATisEmpty(sol); sol=ATgetNext(sol))
                {
                  ATermAppl subst = ATAgetFirst(sol);
                  rewr->setSubstitution(ATAgetArgument(subst,0),ATgetArgument(subst,1));
                  gsprintf("%P := %P",ATAgetArgument(subst,0),rewr->fromRewriteFormat(ATgetArgument(subst,1)));
                  if ( !ATisEmpty(ATgetNext(sol)) )
                  {
                    gsprintf(", ");
                  }
                }
                gsprintf("] gives %P\n",rewr->rewrite(term));
              }
              for (; !ATisEmpty(vars); vars=ATgetNext(vars))
              {
                rewr->clearSubstitution(ATAgetFirst(vars));
              }
              reset_rewr_substs();
            }
          } else {
            cout << "unknown command (try ':h' for help)" << endl;
          }
        } else {
          string::size_type assign_pos = s.find(":=");
          ATermAppl var = NULL;
          if ( assign_pos != string::npos )
          {
            ATermAppl var_name = gsString2ATermAppl(trim_spaces(s.substr(0,assign_pos)).c_str());
            ATermAppl var_sort;
            if ( (var_sort = (ATermAppl) ATtableGet(variables,(ATerm) var_name)) == NULL )
            {
              gsErrorMsg("variable '%T' is not declared\n",var_name);
              break;
            }
            var = gsMakeDataVarId(var_name,var_sort);
            s = s.substr(assign_pos+2);
          }
          ATermAppl term = parse_term(s,spec);
          if ( term != NULL )
          {
            term = rewr->rewrite(term);
            if ( var == NULL )
            {
              gsprintf("%P\n",term);
            } else {
              rewr->setSubstitution(var,rewr->toRewriteFormat(term));
              ATtablePut(assignments,(ATerm) var,rewr->toRewriteFormat(term));
            }
          }
        }
      }
    }
 
    delete rewr;
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
