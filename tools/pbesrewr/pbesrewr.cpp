// Author(s): Jan Friso Groote, Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbesrewr.cpp

#include "boost.hpp" // precompiled headers

#include <iostream>
#include <string>
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/pbes/tools.h"
#include "mcrl2/utilities/input_output_tool.h"
#include "mcrl2/utilities/rewriter_tool.h"
#include "mcrl2/utilities/pbes_rewriter_tool.h"
#include "mcrl2/utilities/mcrl2_gui_tool.h"

using namespace mcrl2;
using namespace mcrl2::log;
using utilities::tools::input_output_tool;
using utilities::tools::rewriter_tool;
using utilities::tools::pbes_rewriter_tool;
using namespace mcrl2::utilities::tools;
using namespace mcrl2::utilities;

class pbes_rewriter : public pbes_rewriter_tool<rewriter_tool<input_output_tool> >
{
  protected:
    typedef pbes_rewriter_tool<rewriter_tool<input_output_tool> > super;

    bool m_skip_data;

    void parse_options(const command_line_parser& parser)
    {
      super::parse_options(parser);
      m_skip_data = parser.options.count("skip-data") > 0;
    }

    void add_options(interface_description& desc)
    {
      super::add_options(desc);
      desc.add_option("skip-data", "do not rewrite data expressions", 's');
    }

  public:
    pbes_rewriter()
      : super(
        "pbesrewr",
        "Jan Friso Groote and Wieger Wesselink",
        "rewrite and simplify a PBES",
        "Rewrite the PBES in INFILE, remove quantified variables and write the resulting PBES to OUTFILE. "
        "If INFILE is not present, stdin is used. If OUTFILE is not present, stdout is used."
      ),
      m_skip_data(false)
    {}

    bool run()
    {
      using namespace pbes_system;
      using namespace utilities;

      mCRL2log(verbose) << "pbesrewr parameters:" << std::endl;
      mCRL2log(verbose) << "  input file:         " << m_input_filename << std::endl;
      mCRL2log(verbose) << "  output file:        " << m_output_filename << std::endl;
      mCRL2log(verbose) << "  pbes rewriter:      " << m_pbes_rewriter_type << std::endl;

      pbesrewr(input_filename(),
               rewrite_strategy(),
               rewriter_type(),
               m_skip_data
             );
      return true;
    }

};

class pbes_rewriter_gui: public mcrl2_gui_tool<pbes_rewriter>
{
  public:
    pbes_rewriter_gui()
    {

      std::vector<std::string> values;

      values.clear();
      values.push_back("simplify");
      values.push_back("quantifier-all");
      values.push_back("quantifier-finite");
      values.push_back("pfnf");
      m_gui_options["pbes-rewriter"] = create_radiobox_widget(values);

      add_rewriter_widget();
    }
};
int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  return pbes_rewriter_gui().execute(argc, argv);
}
