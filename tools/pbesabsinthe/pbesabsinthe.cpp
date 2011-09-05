// Author(s): Wieger Wesselink, Simon Janssen, Tim Willemse
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbesabsint.cpp

#include "boost.hpp" // precompiled headers

#include <iostream>
#include <string>
#include "mcrl2/exception.h"
#include "mcrl2/utilities/input_output_tool.h"
#include "mcrl2/utilities/mcrl2_gui_tool.h"
#include "mcrl2/utilities/text_utility.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/absinthe.h"
#include "mcrl2/pbes/rewriter.h"
#include "mcrl2/atermpp/aterm_init.h"

using namespace mcrl2;
using namespace mcrl2::log;
using namespace mcrl2::pbes_system;
using namespace mcrl2::core;
using namespace mcrl2::utilities;
using namespace mcrl2::utilities::tools;

class pbes_absinthe_tool: public input_output_tool
{
  protected:
    typedef input_output_tool super;

    std::string m_abstraction_file;
    bool m_print_used_function_symbols;
    bool m_enable_logging;

    /// The transformation strategies of the tool.
    enum approximation_strategy
    {
      as_over,
      as_under
    };

    approximation_strategy m_strategy;

    /// Sets the output format.
    /// \param format An output format.
    void set_approximation_strategy(const std::string& strategy)
    {
      if (strategy == "over")
      {
        m_strategy = as_over;
      }
      else if (strategy == "under")
      {
        m_strategy = as_under;
      }
      else
      {
        throw std::runtime_error("unknown approximation strategy specified (got `" + strategy + "')");
      }
    }

    void parse_options(const command_line_parser& parser)
    {
      super::parse_options(parser);
      set_approximation_strategy(parser.option_argument("strategy"));
      m_abstraction_file = parser.option_argument("abstraction-file");
      m_print_used_function_symbols = parser.options.count("used-function-symbols") > 0;
      m_enable_logging = parser.options.count("enable-logging") > 0;
    }

    void add_options(interface_description& desc)
    {
      super::add_options(desc);

      desc.add_option("abstraction-file",
                       make_optional_argument("FILE", ""),
                       "use the abstraction specification in FILE. ",
                       'a');

      desc.add_option("strategy",
                       make_mandatory_argument("NAME"),
                       "use the approximation strategy NAME:\n"
                       "  'over'  for an over-approximation,\n"
                       "  'under' for an under-approximation\n",
                       's');
      desc.add_option("used-function-symbols", "print used function symbols", 'u');
      desc.add_option("enable-logging", "print absinthe specific log messages", 'l');
    }

  public:
    pbes_absinthe_tool()
      : super(
        "pbesabsint",
        "Wieger Wesselink; Maciek Gazda and Tim Willemse",
        "apply data domain abstracion to a PBES",
        "Reads a file containing a PBES, and applies abstraction to it's data domain, based on a\n"
        "user defined mappings. If OUTFILE is not present, standard output is used. If INFILE is not\n"
        "present, standard input is used."
      )
    {}

    bool run()
    {
      mCRL2log(verbose) << "pbesabsint parameters:    " << std::endl;
      mCRL2log(verbose) << "  input file:             " << m_input_filename << std::endl;
      mCRL2log(verbose) << "  output file:            " << m_output_filename << std::endl;
      mCRL2log(verbose) << "  abstraction file:       " << m_abstraction_file << std::endl;
      mCRL2log(verbose) << "  approximation strategy: " << (m_strategy == as_over ? "over" : "under")  << std::endl;

      // load the pbes
      pbes<> p;
      p.load(m_input_filename);

      if (m_print_used_function_symbols)
      {
        pbes_system::detail::print_used_function_symbols(p);
      }

      std::string abstraction_text;
      if (!m_abstraction_file.empty())
      {
        abstraction_text = utilities::read_text(m_abstraction_file);
      }

      bool over_approximation = m_strategy == as_over;

      absinthe_algorithm algorithm;
      if (m_enable_logging)
      {
        algorithm.enable_logging();
      }
      algorithm.run(p, abstraction_text, over_approximation);

      // save the result
      p.save(m_output_filename);

      return true;
    }

};

class pbes_absint_gui_tool: public mcrl2_gui_tool<pbes_absinthe_tool>
{
  public:
    pbes_absint_gui_tool()
    {
      m_gui_options["data"] = create_filepicker_widget("Text Files (*.txt)|*.txt|mCRL2 files (*.mcrl2)|*.mcrl2|All Files (*.*)|*.*");
      std::vector<std::string> values;
      values.clear();
      values.push_back("over");
      values.push_back("under");
      // TODO: finish the GUI tool
    }
};

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)
  pbes_absint_gui_tool tool;
  return tool.execute(argc, argv);
}
