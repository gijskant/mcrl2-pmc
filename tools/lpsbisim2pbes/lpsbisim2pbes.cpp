// Author(s): Wieger Wesselink, Simon Janssen, Tim Willemse
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lpsbisim2pbes.cpp

#include "boost.hpp" // precompiled headers

#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include "mcrl2/pbes/normalize.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/bisimulation.h"
#include "mcrl2/utilities/tool.h"
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/exception.h"

using namespace mcrl2;
using namespace mcrl2::lps;
using namespace mcrl2::pbes_system;
using utilities::command_line_parser;
using utilities::interface_description;
using utilities::make_mandatory_argument;
using utilities::tools::tool;
using namespace mcrl2::log;

/// \brief Base class for tools that take a file as input.
class input_input_tool: public tool
{
  protected:
    /// The first input file name
    std::string m_input_filename1;

    /// The second input file name
    std::string m_input_filename2;

    /// \brief Checks if the number of positional options is OK.
    /// \param parser A command line parser
    void check_positional_options(const command_line_parser& parser)
    {
      if (parser.options.find("help") != parser.options.end())
      {
        return;
      }
      if (parser.arguments.size() < 2)
      {
        parser.error("wrong number of file arguments");
      }
    }

    /// \brief Returns the synopsis of the tool
    /// \return The string "[OPTION]... INFILE1 INFILE2\n"
    std::string synopsis() const
    {
      return "[OPTION]... INFILE1 INFILE2\n";
    }

    /// \brief Parse non-standard options
    /// \param parser A command line parser
    void parse_options(const command_line_parser& parser)
    {
      m_input_filename1 = parser.arguments[0];
      m_input_filename2 = parser.arguments[1];
    }

  public:
    /// \brief Constructor.
    input_input_tool(const std::string& name,
                     const std::string& author,
                     const std::string& what_is,
                     const std::string& tool_description
                    )
      : tool(name, author, what_is, tool_description)
    {
    }

    /// \brief Returns a const reference to the first input filename.
    const std::string& input_filename1() const
    {
      return m_input_filename1;
    }

    /// \brief Returns a reference to the first input filename.
    std::string& input_filename1()
    {
      return m_input_filename1;
    }

    /// \brief Returns a const reference to the second input filename.
    const std::string& input_filename2() const
    {
      return m_input_filename2;
    }

    /// \brief Returns a reference to the second input filename.
    std::string& input_filename2()
    {
      return m_input_filename2;
    }
};

enum bisimulation_type
{
  strong_bisim,
  weak_bisim,
  branching_bisim,
  branching_sim
};

bisimulation_type parse_bisimulation_type(const std::string& type)
{
  if (type == "strong-bisim"        ) return strong_bisim;
  else if (type == "weak-bisim"     ) return weak_bisim;  
  else if (type == "branching-bisim") return branching_bisim;
  else if (type == "branching-sim"  ) return branching_sim;
  throw mcrl2::runtime_error(std::string("unknown bisimulation type ") + type + "!");
  return strong_bisim;
}

std::string print_bisimulation_type(int type)
{
  switch (type)
  {
    case strong_bisim:
      return "strong bisimulation";
    case weak_bisim:
      return "weak bisimulation";
    case branching_bisim:
      return "branching bisimulation";
    case branching_sim:
      return "branching simulation equivalence";
  }
  return "unknown type";
}

class lpsbisim2pbes_tool: public input_input_tool
{
  protected:
    /// \brief The output file name
    std::string m_output_filename;

    /// \brief The type of bisimulation
    bisimulation_type m_bisimulation_type;

    /// \brief If true the result is normalized
    bool normalize;

    /// \brief Returns the synopsis of the tool.
    /// \return The string "[OPTION]... INFILE1 INFILE2 [OUTFILE]\n"
    std::string synopsis() const
    {
      return "[OPTION]... INFILE1 INFILE2 [OUTFILE]\n";
    }

    /// \brief Parse non-standard options
    /// \param parser A command line parser
    void parse_options(const command_line_parser& parser)
    {
      input_input_tool::parse_options(parser);
      if (2 < parser.arguments.size())
      {
        m_output_filename = parser.arguments[2];
      }
      normalize = parser.options.count("normalize") > 0;

      m_bisimulation_type = parse_bisimulation_type(parser.option_argument("bisimulation"));
    }

    void add_options(interface_description& desc) /*< One can add command line
                     options by overriding the virtual function `add_options`. >*/
    {
      desc.add_option("normalize", "normalize the result", 'n');
      desc.add_option("bisimulation", make_mandatory_argument("NAME"),
                      "generate a PBES for the bisimulation type NAME:\n"
                      "'strong-bisim' for strong bisimilarity,\n"
                      "'weak-bisim' for weak bisimilarity,'\n"
                      "'branching-bisim' for branching bisimilarity,\n"
                      "'branching-sim' for branching simulation equivalence"
                      ,'b'
                     );
    }

  public:
    lpsbisim2pbes_tool()
      : input_input_tool(
        "lpsbisim2pbes",
        "Wieger Wesselink; Tim Willemse and Bas Ploeger",
        "computes a bisimulation relation between two LPSs",
        "Reads two files containing an LPS, and computes a PBES that expresses "
        "bisimulation between the two. If OUTFILE is not present, standard "
        "output is used.\n"
      )
    {}

    /// \brief Returns a const reference to the output filename.
    const std::string& output_filename() const
    {
      return m_output_filename;
    }

    /// \brief Returns a reference to the output filename.
    std::string& output_filename()
    {
      return m_output_filename;
    }

    bool run()
    {
      specification M;
      specification S;

      mCRL2log(verbose) << "lpsbisim2pbes parameters:" << std::endl;
      mCRL2log(verbose) << "  input file 1 :         " << input_filename1() << std::endl;
      mCRL2log(verbose) << "  input file 2 :         " << input_filename2() << std::endl;
      mCRL2log(verbose) << "  output file  :         " << output_filename() << std::endl;
      mCRL2log(verbose) << "  bisimulation :         " << print_bisimulation_type(m_bisimulation_type) << std::endl;
      mCRL2log(verbose) << "  normalize    :         " << normalize << std::endl;

      M.load(input_filename1());
      S.load(input_filename2());
      pbes<> result;
      switch (m_bisimulation_type)
      {
        case strong_bisim:
          result = strong_bisimulation(M, S);
          break;
        case weak_bisim:
          result = weak_bisimulation(M, S);
          break;
        case branching_bisim:
          result = branching_bisimulation(M, S);
          break;
        case branching_sim:
          result = branching_simulation_equivalence(M, S);
          break;
      }
      if (normalize)
      {
        pbes_system::normalize(result);
      }
      result.save(output_filename());

      return true;
    }
};

int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)
  lpsbisim2pbes_tool tool;
  return tool.execute(argc, argv);
}

