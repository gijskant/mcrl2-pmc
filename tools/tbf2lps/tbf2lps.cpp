// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file tbf2lps.cpp

#include "boost.hpp" // precompiled headers

#define NAME "tbf2lps"
#define AUTHOR "Muck van Weerdenburg"

#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include "mcrl2/aterm/aterm2.h"
#include "mcrl2/aterm/aterm_ext.h"
#include "lpstrans.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/input_output_tool.h"
#include "mcrl2/utilities/mcrl2_gui_tool.h"
#include "mcrl2/exception.h"

using namespace mcrl2::utilities;
using namespace mcrl2::utilities::tools;
using namespace mcrl2::core;
using namespace aterm;

class tbf2lps_tool: public input_output_tool
{
    typedef input_output_tool super;

  public:
    tbf2lps_tool()
      : super(NAME, AUTHOR,
              "convert a muCRL LPE to an mCRL2 LPS",
              "Read mCRL LPS from INFILE, convert it to a mCRL2 LPS and save the result to "
              "OUTFILE. If OUTFILE is not present, stdout is used. If INFILE is not present, "
              "stdin is used. To use stdin and save the output to a file, use '-' for INFILE.\n"
              "\n"
              "This conversion assumes that Bool is the boolean sort with "
              "constructors T and F. Additionally, the following "
              "conversions on the data specification will be applied:\n"
              "  constructors T, F: -> Bool are replaced by true and false,\n"
              "  mapping and: Bool # Bool -> Bool is replaced by ||, and\n"
              "  mapping eq: S # S -> Bool is replaced by == for each sort S"
             ),
      m_not_convert_mappings(true)
    {}

    bool run()
    {
      ATermAppl mcrl_spec;

      if (input_filename().empty())
      {
        mCRL2log(verbose) << "reading mCRL LPS from stdin..." << std::endl;

        mcrl_spec = (ATermAppl) ATreadFromFile(stdin);

        if (mcrl_spec == 0)
        {
          throw mcrl2::runtime_error("could not read mCRL LPS from '" + input_filename() + "'");
        }
        if (!is_mCRL_spec(mcrl_spec))
        {
          throw mcrl2::runtime_error("stdin does not contain an mCRL LPS");
        }
      }
      else
      {
        mCRL2log(verbose) << "reading mCRL LPS from '" <<  input_filename() << "'..." << std::endl;

        FILE* in_stream = fopen(input_filename().c_str(), "rb");

        if (in_stream == 0)
        {
          throw mcrl2::runtime_error("could not open input file '" + input_filename() + "' for reading");
        }

        mcrl_spec = (ATermAppl) ATreadFromFile(in_stream);

        fclose(in_stream);

        if (mcrl_spec == 0)
        {
          throw mcrl2::runtime_error("could not read mCRL LPS from '" + input_filename() + "'");
        }
        if (!is_mCRL_spec(mcrl_spec))
        {
          throw mcrl2::runtime_error("'" + input_filename() + "' does not contain an mCRL LPS");
        }
      }

      ATprotectAppl(&mcrl_spec);
      assert(is_mCRL_spec(mcrl_spec));

      ATermAppl spec = translate(mcrl_spec,true,m_not_convert_mappings);
      ATprotectAppl(&spec);

      if (output_filename().empty())
      {
        mCRL2log(verbose) << "writing mCRL2 LPS to stdout..." << std::endl;

        ATwriteToSAFFile((ATerm) spec, stdout);
      }
      else
      {
        mCRL2log(verbose) << "writing mCRL2 LPS to '" <<  output_filename() << "'..." << std::endl;

        FILE* outstream = fopen(output_filename().c_str(), "wb");

        if (outstream == NULL)
        {
          throw mcrl2::runtime_error("cannot open output file '" + output_filename() + "'");
        }

        ATwriteToSAFFile((ATerm) spec,outstream);

        fclose(outstream);
      }
      return true;
    }

  protected:
    bool m_not_convert_mappings;

    void add_options(interface_description& desc)
    {
      input_output_tool::add_options(desc);
      desc.add_option("no-conv-map",
                      "do not apply conversion of mappings and equations", 'n');
    }

    void parse_options(const command_line_parser& parser)
    {
      input_output_tool::parse_options(parser);
      m_not_convert_mappings = parser.options.count("no-conv-map") == 0;
    }
};

class tbf2lps_gui_tool: public mcrl2_gui_tool<tbf2lps_tool>
{
  public:
    tbf2lps_gui_tool()
    {
      m_gui_options["no-conv-map"] = create_checkbox_widget();
    }
};

int main(int argc, char** argv)
{
  MCRL2_ATERM_INIT(argc, argv)
  return tbf2lps_gui_tool().execute(argc, argv);
}
