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
#define AUTHOR "Aad Mathijssen and Jeroen Keiren"

#include <string>
#include <iostream>
#include <fstream>

#include "mcrl2/utilities/logger.h"
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/utilities/input_output_tool.h"
#include "mcrl2/utilities/pbes_input_tool.h"
#include "mcrl2/utilities/mcrl2_gui_tool.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/io.h"

using namespace mcrl2::log;
using namespace mcrl2::utilities::tools;
using namespace mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2;

//local declarations

class pbespp_tool: public pbes_input_tool<input_output_tool>
{
  private:
    typedef pbes_input_tool<input_output_tool> super;

  public:
    pbespp_tool()
      : super(NAME, AUTHOR,
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
                      "  'internal' for a textual ATerm representation of the internal format", 'f');
    }

    void parse_options(const command_line_parser& parser)
    {
      super::parse_options(parser);
      if (parser.options.count("format"))
      {
        std::string str_format(parser.option_argument("format"));
        if (str_format == "internal")
        {
          format = ppInternal;
        }
        else if (str_format != "default")
        {
          parser.error("option -f/--format has illegal argument '" + str_format + "'");
        }
      }
    }

  private:
    void print_specification()
    {
      pbes_system::pbes<> pbes_specification;
      load_pbes(pbes_specification, input_filename(), pbes_input_format());

      mCRL2log(verbose) << "printing PBES from "
                        << (input_filename().empty()?"standard input":input_filename())
                        << " to " << (output_filename().empty()?"standard output":output_filename())
                        << " in the " << pp_format_to_string(format) << " format" << std::endl;

      if (output_filename().empty())
      {
        if (format == ppInternal)
        {
          std::cout << pbes_system::pbes_to_aterm(pbes_specification);
        }
        else
        {
          std::cout << pbes_system::pp(pbes_specification);
        }
      }
      else
      {
        std::ofstream output_stream(output_filename().c_str());
        if (output_stream.is_open())
        {
          if (format == ppInternal)
          {
            output_stream << pbes_system::pbes_to_aterm(pbes_specification);
          }
          else
          {
            output_stream << pbes_system::pp(pbes_specification);
          }
          output_stream.close();
        }
        else
        {
          throw mcrl2::runtime_error("could not open output file " + output_filename() + " for writing");
        }
      }
    }
};

class pbespp_gui_tool: public mcrl2_gui_tool<pbespp_tool>
{
  public:
    pbespp_gui_tool()
    {

      std::vector<std::string> values;

      values.clear();
      values.push_back("default");
      values.push_back("debug");
      values.push_back("internal");
      values.push_back("internal-debug");
      m_gui_options["format"] = create_radiobox_widget(values);
    }
};

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)
  return pbespp_gui_tool().execute(argc, argv);
}

