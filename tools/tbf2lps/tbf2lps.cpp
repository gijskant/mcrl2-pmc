// Author(s): Muck van Weerdenburg
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file tbf2lps.cpp

#define NAME "tbf2lps"
#define AUTHOR "Muck van Weerdenburg"

#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <aterm2.h>
#include "mcrl2/core/struct.h"
#include "lpstrans.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "mcrl2/utilities/command_line_interface.h" // after messaging.h and rewrite.h

using namespace ::mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2;

struct tool_options_type {
  bool            convert_funcs;
  bool            convert_bools; 
  std::string     infilename;
  std::string     outfilename;
};

tool_options_type parse_command_line(int ac, char** av) {
  interface_description clinterface(av[0], NAME, AUTHOR, "[OPTION]... [INFILE [OUTFILE]]\n"
    "Read mCRL LPS from INFILE, convert it to a mCRL2 LPS and save the result to "
    "OUTFILE. If OUTFILE is not present, stdout is used. If INFILE is not present, "
    "stdin is used. To use stdin and save the output to a file, use '-' for INFILE.\n"
    "\n"
    "The following conversions on the data specification will be applied:\n"
    "- constructors T, F: -> Bool are replaced by true and false,\n"
    "- mappings and, or: Bool # Bool -> Bool are replaced by && and ||, and\n"
    "- mapping eq: S # S -> Bool is replaced by == for each sort S");

  clinterface.
    add_option("no-conv-map",
      "do not apply the conversion of mappings and, or and eq", 'n').
    add_option("no-conv-cons",
      "do not apply the conversion of constructors T and F; note that this conversion is "
      "really needed for the toolset to know what true and false are (e.g. simulation "
      "and state space generation will not be possible)");

  command_line_parser parser(clinterface, ac, av);

  tool_options_type options;

  options.convert_funcs = parser.options.count("no-conv-map") == 0;
  options.convert_bools = parser.options.count("no-conv-cons") == 0;

  if (0 < parser.arguments.size()) {
    options.infilename = parser.arguments[0];
  }
  if (1 < parser.arguments.size()) {
    options.outfilename = parser.arguments[0];
  }
  if (2 < parser.arguments.size()) {
    parser.error("too many file arguments");
  }

  return options;
}

int main(int argc, char **argv)
{
  MCRL2_ATERM_INIT(argc, argv)

  try {
    tool_options_type options(parse_command_line(argc, argv));

    ATermAppl mcrl_spec;

    if (options.infilename.empty()) {
      gsVerboseMsg("reading mCRL LPS from stdin...\n");

      mcrl_spec = (ATermAppl) ATreadFromFile(stdin);

      if (mcrl_spec == 0) {
        throw std::runtime_error("could not read mCRL LPS from '" + options.infilename + "'");
      }
      if (!is_mCRL_spec(mcrl_spec)) {
        throw std::runtime_error("stdin does not contain an mCRL LPS");
      }
    }
    else {
      gsVerboseMsg("reading mCRL LPS from '%s'...\n", options.infilename.c_str());

      FILE *in_stream = fopen(options.infilename.c_str(), "rb");

      if (in_stream == 0) {
        throw std::runtime_error("could not open input file '" + options.infilename + "' for reading");
      }

      mcrl_spec = (ATermAppl) ATreadFromFile(in_stream);

      fclose(in_stream);

      if (mcrl_spec == 0) {
        throw std::runtime_error("could not read mCRL LPS from '" + options.infilename + "'");
      }
      if (!is_mCRL_spec(mcrl_spec)) {
        throw std::runtime_error("'" + options.infilename + "' does not contain an mCRL LPS");
      }
    }

    ATprotectAppl(&mcrl_spec);
    assert(is_mCRL_spec(mcrl_spec));

    ATermAppl spec = translate(mcrl_spec,options.convert_bools,options.convert_funcs);
    ATprotectAppl(&spec);

    if (options.outfilename.empty()) {
      gsVerboseMsg("writing mCRL2 LPS to stdout...\n");

      ATwriteToSAFFile((ATerm) mcrl_spec, stdout);
    }
    else {
      gsVerboseMsg("writing mCRL2 LPS to '%s'...\n", options.outfilename.c_str());

      FILE *outstream = fopen(options.outfilename.c_str(), "wb");

      if (outstream == NULL) {
        throw std::runtime_error("cannot open output file '" + options.outfilename + "'");
      }

      ATwriteToSAFFile((ATerm) spec,outstream);

      fclose(outstream);
    }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return EXIT_FAILURE;
}
