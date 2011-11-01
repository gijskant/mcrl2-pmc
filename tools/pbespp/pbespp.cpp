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
#include "mcrl2/pbes/tools.h"

using namespace mcrl2::log;
using namespace mcrl2::utilities::tools;
using namespace mcrl2::utilities;
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
      format(core::ppDefault)
    {}

    bool run()
    {
      pbespp(input_filename(),
             output_filename(),
             pbes_input_format(),
             format
            );
      return true;
    }

  protected:
    core::t_pp_format  format;

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
          format = core::ppInternal;
        }
        else if (str_format != "default")
        {
          parser.error("option -f/--format has illegal argument '" + str_format + "'");
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

