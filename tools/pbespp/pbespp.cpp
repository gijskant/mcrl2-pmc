// Author(s): Aad Mathijssen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbespp.cpp

#include "boost.hpp" // precompiled headers

#define NAME "pbespp"
#define AUTHOR "Aad Mathijssen"

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <cassert>

#include "mcrl2/core/detail/struct.h"
#include "mcrl2/core/detail/aterm_io.h"
#include "mcrl2/core/print.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/utilities/input_output_tool.h"
#include "mcrl2/pbes/pbes.h"

using namespace mcrl2::utilities::tools;
using namespace mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2;

//local declarations

class pbespp_tool: public input_output_tool
{
  public:
    pbespp_tool()
      : input_output_tool(NAME, AUTHOR,
          "pretty print a PBES",
          "Print the PBES in INFILE to OUTFILE in a human readable format. If OUTFILE "
          "is not present, stdout is used. If INFILE is not present, stdin is used."
        ),
        format(ppDefault)
    {}

    bool run()
    {
      print_specification();
      return true;
    }

  protected:
    t_pp_format  format;

    void add_options(interface_description& desc)
    {
      input_output_tool::add_options(desc);
      desc.add_option("format", make_mandatory_argument("FORMAT"),
        "print the PBES in the specified FORMAT:\n"
        "  'default' for a PBES specification (default),\n"
        "  'debug' for 'default' with the exceptions that data expressions are printed in prefix notation using identifiers from the internal format, and each data equation is put in a separate data equation section,\n"
        "  'internal' for a textual ATerm representation of the internal format, or\n"
        "  'internal-debug' for 'internal' with an indented layout", 'f');
    }

    void parse_options(const command_line_parser& parser)
    {
      input_output_tool::parse_options(parser);
      if (parser.options.count("format")) {
        std::string str_format(parser.option_argument("format"));
        if (str_format == "internal") {
          format = ppInternal;
        } else if (str_format == "internal-debug") {
          format = ppInternalDebug;
        } else if (str_format == "debug") {
          format = ppDebug;
        } else if (str_format != "default") {
          parser.error("option -f/--format has illegal argument '" + str_format + "'");
        }
      }
    }

  private:
    void print_specification()
    {
      std::string str_in  = (input_filename().empty())?"stdin":("'" + input_filename() + "'");
      std::string str_out = (output_filename().empty())?"stdout":("'" + output_filename() + "'");

      pbes_system::pbes<> pbes_specification;
      pbes_specification.load(input_filename());

      if (format != ppDebug)
      {
        pbes_specification.data() = mcrl2::data::remove_all_system_defined(pbes_specification.data());
      }

      ATermAppl spec = pbes_to_aterm(pbes_specification);

      //open output file for writing or set to stdout
      FILE *output_stream    = NULL;
      if (output_filename().empty()) {
        output_stream = stdout;
        gsDebugMsg("output to stdout.\n");
      } else {
        output_stream = fopen(output_filename().c_str(), "w");
        if (output_stream == NULL) {
          std::string err_msg(strerror(errno));
          if (err_msg.length() > 0 && err_msg[err_msg.length()-1] == '\n') {
            err_msg.replace(err_msg.length()-1, 1, "");
          }
          throw mcrl2::runtime_error("could not open output file " + str_out + " for writing (" + err_msg + ")");
        }
        gsDebugMsg("output file %s is opened for writing.\n", str_out.c_str());
      }
      assert(output_stream != NULL);
      //print spec to output_stream
      gsVerboseMsg("printing PBES from %s to %s in the %s format\n",
        str_in.c_str(), str_out.c_str(), pp_format_to_string(format).c_str());
      //pretty print spec to output_stream
      PrintPart_C(output_stream, (ATerm) spec, format);
      if (output_stream != stdout) {
        fclose(output_stream);
      }
    }
};

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)
  return pbespp_tool().execute(argc, argv);
}

