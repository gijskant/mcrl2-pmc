// Author(s): Wieger Wesselink, Simon Janssen, Tim Willemse
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbesabstract.cpp

#include "boost.hpp" // precompiled headers

#include <iostream>
#include <string>
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/abstract.h"
#include "mcrl2/pbes/detail/pbes_parameter_map.h"
#include "mcrl2/utilities/input_output_tool.h"

using namespace mcrl2;
using namespace mcrl2::pbes_system;
using namespace mcrl2::core;
using namespace mcrl2::utilities;
using utilities::tools::input_output_tool;

class pbes_abstract_tool: public input_output_tool
{
  protected:
    typedef input_output_tool super;

    std::string m_parameter_selection;
    bool m_value_true;

    void parse_options(const command_line_parser& parser)
    {
      super::parse_options(parser);
      if (parser.options.count("select") > 0)
      {
        m_parameter_selection = parser.option_argument("select");
        boost::trim(m_parameter_selection);
      }
      m_value_true = parser.option_argument_as<bool>("abstraction-value");
    }

    void add_options(interface_description& desc)
    {
      super::add_options(desc);
      desc.add_option("select",
                      make_optional_argument("NUM", ""),
                      "select the PBES parameters that are allowed to be abstracted. Wild cards are allowed\n"
                      "\n"
                      "Examples:\n\n"
                      "          X1(b:Bool,c:Bool);X2(b:Bool)\n"
                      "          X1(*:Bool);X2(*:Bool)\n"
                      "          *(*:Bool)",
                      'f');
      desc.add_option("abstraction-value",
                      make_optional_argument("NUM", "1"),
                      "the abstraction value 0 (false) or 1 (true)",
                      'a');
    }

  public:
    pbes_abstract_tool()
      : super(
        "pbesabstract",
        "Wieger Wesselink; Tom Haenen and Tim Willemse",
        "Tool for abstracting parameters of a PBES",
        "Reads a file containing a PBES. For the variables that are indicated to be"
        "abstracted, the smallest boolean subexpression is found which is then replaced"
        "by false (default) or true.\n"
        "  If the solution of a PBES after application of pbesabstract with"
        "the value false is true, then the original PBES has solution true. Similarly"
        "if the solution of a PBES after application of pbesabstract with the value\n"
        "true is false, then the original PBES has solution false. After abstracting from certain"
        "variables, pbesparelm can be used to remove these variables from the PBES hopefully, "
        "but not necessarily, reducing the number of BES variables required to solve the PBES.\n\n"
        "If OUTFILE is not present, standard output is used."
        "If INFILE is not present, standard input is used."
        "\n\n"
      ),
      m_value_true(true)
    {}

    bool run()
    {
      mCRL2log(verbose) << "pbesabstract parameters:" << std::endl;
      mCRL2log(verbose) << "  input file:         " << m_input_filename << std::endl;
      mCRL2log(verbose) << "  output file:        " << m_output_filename << std::endl;
      mCRL2log(verbose) << "  parameters:         " << m_parameter_selection << std::endl;

      // load the pbes
      pbes<> p;
      p.load(m_input_filename);

      // run the algorithm
      pbes_abstract_algorithm algorithm;
      pbes_system::detail::pbes_parameter_map parameter_map = pbes_system::detail::parse_pbes_parameter_map(p, m_parameter_selection);
      algorithm.run(p, parameter_map, m_value_true);

      // save the result
      p.save(m_output_filename);

      return true;
    }
};

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)
  pbes_abstract_tool tool;
  return tool.execute(argc, argv);
}
