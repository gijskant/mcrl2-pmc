// Author(s): Wieger Wesselink, Simon Janssen, Tim Willemse
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbesconstelm.cpp

#include "boost.hpp" // precompiled headers

// #define MCRL2_PBES_CONSTELM_DEBUG
// #define MCRL2_PBES_EXPRESSION_BUILDER_DEBUG

#include <iostream>
#include <string>
#include "mcrl2/utilities/pbes_rewriter_tool.h"
#include "mcrl2/data/identifier_generator.h"
#include "mcrl2/data/enumerator.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/constelm.h"
#include "mcrl2/pbes/rewriter.h"

using namespace mcrl2;
using namespace mcrl2::pbes_system;
using namespace mcrl2::core;
using namespace mcrl2::utilities;
using utilities::tools::pbes_rewriter_tool;

class pbes_constelm_tool: public pbes_rewriter_tool
{
  protected:
    bool m_compute_conditions;

    void parse_options(const command_line_parser& parser)
    {
    	pbes_rewriter_tool::parse_options(parser);
      m_compute_conditions = parser.options.count("compute-conditions") > 0;
    }

    void add_options(interface_description& desc)
    {
    	pbes_rewriter_tool::add_options(desc);
      desc.add_option("compute-conditions", "compute propagation conditions", 'c');
    }

  public:
    pbes_constelm_tool()
      : pbes_rewriter_tool(
          "pbesconstelm",
          "Wieger Wesselink",
          "Reads a file containing a pbes, and applies constant parameter elimination to it. If OUTFILE "
          "is not present, standard output is used. If INFILE is not present, standard input is used."
        )
    {}

    bool run()
    {
      if (mcrl2::core::gsVerbose)
      {
        std::cout << "pbesconstelm parameters:" << std::endl;
        std::cout << "  input file:         " << m_input_filename << std::endl;
        std::cout << "  output file:        " << m_output_filename << std::endl;
        std::cout << "  compute conditions: " << std::boolalpha << m_compute_conditions << std::endl;
      }
    
      // load the pbes
      pbes<> p;
      p.load(m_input_filename);
      
      // data rewriter
      data::rewriter datar = create_rewriter(p.data());

      // pbes rewriter
      switch (rewriter_type())
      {
        case simplify:
        {
          typedef simplifying_rewriter<pbes_system::pbes_expression, data::rewriter> my_pbes_rewriter;
          my_pbes_rewriter pbesr(datar);    
          pbes_constelm_algorithm<pbes_system::pbes_expression, data::rewriter, my_pbes_rewriter> algorithm(datar, pbesr);
          data::number_postfix_generator name_generator("UNIQUE_PREFIX");
          algorithm.run(p, m_compute_conditions);
          break;
        }
        case quantifier_all:
        case quantifier_finite:
        {
          typedef pbes_system::enumerate_quantifiers_rewriter<pbes_system::pbes_expression, data::rewriter_with_variables, data::data_enumerator<> > my_pbes_rewriter;
          bool enumerate_infinite_sorts = (rewriter_type() == quantifier_all);
          data::number_postfix_generator name_generator("UNIQUE_PREFIX");
          data::data_enumerator<> datae(p.data(), datar, name_generator);
          data::rewriter_with_variables datarv(datar);
          my_pbes_rewriter pbesr(datarv, datae, enumerate_infinite_sorts);
          pbes_constelm_algorithm<pbes_system::pbes_expression, data::rewriter, my_pbes_rewriter> algorithm(datar, pbesr);
          algorithm.run(p, m_compute_conditions);
          break;
        }
        default:
        { }
      }     
      
      // save the result
      p.save(m_output_filename);
      
      return true;
    }
};

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)
  pbes_constelm_tool tool;
  return tool.execute(argc, argv);
}
