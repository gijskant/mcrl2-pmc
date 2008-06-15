// Author(s): Aad Mathijssen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ./txt2pbes.cpp
/// \brief Parse a textual description of a PBES.

#define NAME "txt2pbes"
#define AUTHOR "Aad Mathijssen"

//C++
#include <cstdio>
#include <fstream>
#include <string>

//mCRL2 specific
#include "mcrl2/core/parse.h"
#include "mcrl2/core/typecheck.h"
#include "mcrl2/core/data_implementation.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/command_line_interface.h" // must come after mcrl2/core/messaging.h
#include "mcrl2/pbes/pbes.h"

using namespace mcrl2::core;
using namespace mcrl2::utilities;

struct t_tool_options
{
  std::string infilename;
  std::string outfilename;

  t_tool_options() :
    infilename(""),
    outfilename("")
  {}
};

t_tool_options parse_command_line(int ac, char **av)
{
  interface_description clinterface(av[0], NAME, AUTHOR, "[OPTION]... [INFILE [OUTFILE]]\n",
    "Parse the textual description of a PBES from INFILE and write it to OUTFILE. "
    "If INFILE is not present, stdin is used. If OUTFILE is not present, stdout is used.\n\n"
    "The textual description should adhere to the BNF syntax description at "
    "<http://mcrl2.org/wiki/index.php/PBES_syntax>.",
    "It is not possible to use overloading of propositional variables, "
    "i.e. it is not possible to declare two propositional variables with the same name, "
    "but with a different type."
  );
  command_line_parser parser(clinterface, ac, av);
  t_tool_options tool_options;
  if (2 < parser.arguments.size()) {
    parser.error("too many file arguments");
  }
  if (0 < parser.arguments.size()) {
    tool_options.infilename = parser.arguments[0];
  }
  if (1 < parser.arguments.size()) {
    tool_options.outfilename = parser.arguments[1];
  }
  return tool_options;
}

int main(int argc, char** argv)
{
  MCRL2_ATERM_INIT(argc, argv)

  try {
    //parse command line
    t_tool_options tool_options = parse_command_line(argc, argv);
    ATermAppl result = NULL;
    //parse specification
    if (tool_options.infilename.empty()) {
      //parse specification from stdin
      gsVerboseMsg("parsing input from stdin...\n");
      result = parse_pbes_spec(std::cin);
    } else {
      //parse specification from infilename
      std::ifstream instream(tool_options.infilename.c_str(), std::ifstream::in|std::ifstream::binary);
      if (!instream.is_open()) {
        throw mcrl2::runtime_error("cannot open input file '" + tool_options.infilename + "'");
      }
      gsVerboseMsg("parsing input file '%s'...\n", tool_options.infilename.c_str());
      result = parse_pbes_spec(instream);
      instream.close();
    }
    if (result == NULL) {
      throw mcrl2::runtime_error("parsing failed");
    }
    //type check the result
    gsVerboseMsg("type checking...\n");
    result = type_check_pbes_spec(result);
    if (result == NULL) {
      throw mcrl2::runtime_error("type checking failed");
    }
    //implement standard data types and type constructors on the result
    gsVerboseMsg("implementing standard data types and type constructors...\n");
    result = implement_data_pbes_spec(result);
    if (result == NULL) {
      throw mcrl2::runtime_error("data implementation failed");
    }
    //store the result
    mcrl2::pbes_system::pbes<> p(result);
    p.save(tool_options.outfilename);
    return EXIT_SUCCESS;
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
