// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ltscompare.cpp

#define NAME "ltscompare"
#define AUTHOR "Muck van Weerdenburg"

#include <string>
#include "aterm2.h"
#include "mcrl2/core/detail/struct.h"
#include "mcrl2/lts/liblts.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "mcrl2/utilities/command_line_interface.h" // after messaging.h and rewrite.h

using namespace std;
using namespace mcrl2::lts;
using namespace mcrl2::utilities;
using namespace mcrl2::core;

static const char *equivalent_string(lts_equivalence eq)
{
  switch ( eq )
  {
    case lts_eq_bisim:
      return "strongly bisimilar";
    case lts_eq_branching_bisim:
      return "branching bisimilar";
    case lts_eq_trace:
      return "trace equivalent";
    case lts_eq_weak_trace:
      return "weak trace equivalent";
    default:
      return "equivalent";
  }
}

static void print_formats(FILE *f)
{
  fprintf(f,
    "The following formats are accepted by " NAME ":\n"
    "\n"
    "  format  ext.  description                       remarks\n"
    "  -----------------------------------------------------------\n"
    "  aut     .aut  Aldebaran format (CADP)\n"
#ifdef MCRL2_BCG
    "  bcg     .bcg  Binary Coded Graph format (CADP)\n"
#endif
    "  mcrl    .svc  mCRL SVC format\n"
    "  mcrl2   .svc  mCRL2 SVC format                  default\n"
    "\n"
    );
}

struct t_tool_options {
  std::string     name_for_first;
  std::string     name_for_second;
  lts_type        format_for_first;
  lts_type        format_for_second;
  lts_equivalence equivalence;
  lts_eq_options  eq_opts;
};

t_tool_options parse_command_line(int ac, char** av) {
  interface_description clinterface(av[0], NAME, AUTHOR, "[OPTION]... [INFILE1] INFILE2\n",
    "Determine whether or not the labelled transition systems (LTSs) in INFILE1 and INFILE2 are the same modulo some equivalence. "
    "If INFILE1 is not supplied, stdin is used.\n"
    "\n"
    "The input formats are determined by the contents of INFILE1 and INFILE2. "
    "Options --in1 and --in2 can be used to force the input format of INFILE1 and INFILE2, respectively. "
    "A list of supported formats can be requested using the option --formats.");

  clinterface.
    add_option("formats", "list accepted formats", 'f').
    add_option("lps", make_mandatory_argument("FILE"),
      "use FILE as the LPS from which the input LTS was generated; this is "
      "needed to store the correct parameter names of states when saving "
      "in fsm format and to convert non-mCRL2 LTSs to a mCRL2 LTS", 'l').
    add_option("in1", make_mandatory_argument("FORMAT"),
      "use FORMAT as the format for INFILE1 (or stdin)", 'i').
    add_option("in2", make_mandatory_argument("FORMAT"),
      "use FORMAT as the format for INFILE2", 'j').
    add_option("equivalence", make_mandatory_argument("NAME"),
      "use equivalence NAME:\n"
      "  '" + lts::string_for_equivalence(lts_eq_bisim) + "' for "
            + lts::name_of_equivalence(lts_eq_bisim) + " (default), or\n"
      "  '" + lts::string_for_equivalence(lts_eq_branching_bisim) + "' for "
            + lts::name_of_equivalence(lts_eq_branching_bisim) + ", or\n"
      "  '" + lts::string_for_equivalence(lts_eq_trace) + "' for "
            + lts::name_of_equivalence(lts_eq_trace) + ", or\n"
      "  '" + lts::string_for_equivalence(lts_eq_weak_trace) + "' for " 
            + lts::name_of_equivalence(lts_eq_weak_trace)
      , 'e').
    add_option("tau", make_mandatory_argument("ACTNAMES"),
      "consider actions with a name in the comma separated list ACTNAMES to "
      "be internal (tau) actions in addition to those defined as such by "
      "the input");

  command_line_parser parser(clinterface, ac, av);

  t_tool_options tool_options;

  tool_options.equivalence = lts_eq_bisim;

  if (parser.options.count("formats")) {
    print_formats(stderr);
    exit(EXIT_SUCCESS);
  }

  if (parser.options.count("equivalence")) {

    tool_options.equivalence = lts::parse_equivalence(
        parser.option_argument("equivalence"));
    
    if (tool_options.equivalence != lts_eq_bisim &&
        tool_options.equivalence != lts_eq_branching_bisim &&
        tool_options.equivalence != lts_eq_trace &&
        tool_options.equivalence != lts_eq_weak_trace)
    {
      parser.error("option -e/--equivalence has illegal argument '" + 
          parser.option_argument("equivalence") + "'");
    }
  }

  if (parser.options.count("tau")) {
    lts_reduce_add_tau_actions(tool_options.eq_opts, parser.option_argument("tau"));
  }

  if (parser.arguments.size() == 0) {
    parser.error("need at least one file argument");
  }
  else if (2 < parser.arguments.size()) {
    parser.error("too many file arguments");
  }
  else {
    if (0 < parser.arguments.size()) {
      tool_options.name_for_first  = parser.arguments[0];
    }
    if (1 < parser.arguments.size()) {
      tool_options.name_for_second  = parser.arguments[1];
    }
  }

  if (parser.options.count("in1")) {
    if (1 < parser.options.count("in1")) {
      std::cerr << "warning: multiple input formats specified for first LTS; can only use one\n";
    }

    tool_options.format_for_first = lts::parse_format(parser.option_argument("in1"));

    if (tool_options.format_for_first == lts_none) {
      std::cerr << "warning: format '" << parser.option_argument("in1") <<
                   "' is not recognised; option ignored" << std::endl;
    }
  }
  else if (!tool_options.name_for_first.empty()) {
    tool_options.format_for_first = lts::guess_format(tool_options.name_for_first);
  }
  if (parser.options.count("in2")) {
    if (1 < parser.options.count("in2")) {
      std::cerr << "warning: multiple input formats specified for second LTS; can only use one\n";
    }

    tool_options.format_for_second = lts::parse_format(parser.option_argument("in2"));

    if (tool_options.format_for_second == lts_none) {
      std::cerr << "warning: format '" << parser.option_argument("in2") <<
                   "' is not recognised; option ignored" << std::endl;
    }
  }
  else {
    tool_options.format_for_second = lts::guess_format(tool_options.name_for_second);
  }

  return tool_options;
}

int process(t_tool_options const & tool_options) {
  lts l1,l2;

  if ( tool_options.name_for_first.empty() ) {
    gsVerboseMsg("reading first LTS from stdin...\n");

    if ( !l1.read_from(std::cin, tool_options.format_for_first) ) {
      throw mcrl2::runtime_error("cannot read LTS from stdin\nretry with -v/--verbose for more information");
    }
  } else {
    gsVerboseMsg("reading first LTS from '%s'...\n", tool_options.name_for_first.c_str());

    if ( !l1.read_from(tool_options.name_for_first, tool_options.format_for_first) ) {
      bool failed = true; 
      if ( tool_options.format_for_first == lts_none ) { // XXX really do this? 
        gsVerboseMsg("reading failed; trying to force format by extension...\n"); 
        lts_type guessedtype = lts::guess_format(tool_options.name_for_first); 
        if ( (guessedtype != lts_none) && l1.read_from(tool_options.name_for_first,guessedtype) ) 
        { 
          failed = false; 
        } 
      } 
      if ( failed ) {
        throw mcrl2::runtime_error("cannot read LTS from file '" + tool_options.name_for_first + "'\nretry with -v/--verbose for more information");
      }
    }
  }

  gsVerboseMsg("reading second LTS from '%s'...\n", tool_options.name_for_second.c_str());

  if ( !l2.read_from(tool_options.name_for_second, tool_options.format_for_second) ) {
    bool failed = true; 
    if ( tool_options.format_for_second == lts_none ) { // XXX really do this? 
      gsVerboseMsg("reading failed; trying to force format by extension...\n"); 
      lts_type guessedtype = lts::guess_format(tool_options.name_for_second); 
      if ( (guessedtype != lts_none) && l2.read_from(tool_options.name_for_second,guessedtype) ) 
      { 
        failed = false; 
      } 
    } 
    if ( failed ) {
      throw mcrl2::runtime_error("cannot read LTS from file '" + tool_options.name_for_second + "'\nretry with -v/--verbose for more information");
    }
  }

  gsVerboseMsg("comparing LTSs (modulo %s)...\n", lts::name_of_equivalence(tool_options.equivalence).c_str());

  bool result = l1.compare(l2,tool_options.equivalence,tool_options.eq_opts);

  gsMessage("LTSs are %s%s\n", ((result) ? "" : "not "),
                         equivalent_string(tool_options.equivalence));

  return (result) ? 0 : 2;
}

int main(int argc, char **argv) {
  MCRL2_ATERM_INIT(argc, argv)

  try {
#ifdef ENABLE_SQUADT_CONNECTIVITY
    if (mcrl2::utilities::squadt::interactor< squadt_interactor >::free_activation(argc, argv)) {
      return EXIT_SUCCESS;
    }
#endif

    return process(parse_command_line(argc, argv));
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return EXIT_FAILURE;
}
