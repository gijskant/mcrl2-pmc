// Author(s): Aad Mathijssen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file bestranslate.cpp

#include "boost.hpp" // precompiled headers

#include <string>
#include <iostream>
#include <fstream>

#include "mcrl2/utilities/logger.h"
#include "mcrl2/atermpp/aterm_init.h"
#include "mcrl2/utilities/input_output_tool.h"
#include "mcrl2/bes/boolean_equation_system.h"
#include "mcrl2/bes/io.h"
#include "mcrl2/bes/pg_parse.h"
#include "mcrl2/core/text_utility.h"

using namespace mcrl2::utilities::tools;
using namespace mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2;

namespace mcrl2
{
namespace bes
{

/// \brief Guess output file format based on filename
bes_output_format guess_file_format(std::string const& filename)
{
  std::string extension = *(core::split(filename, ".").rbegin());

  bes_output_format result;
  if (extension == "bes")
  {
    result = bes_output_bes;
  }
  else if (extension == "cwi")
  {
    result = bes_output_cwi;
  }
  else if (extension == "gm")
  {
    result = bes_output_pgsolver;
  }
  else
  {
    throw mcrl2::runtime_error("Unknown extension `." + extension + "' occurred.");
  }

  return result;
}

/// \brief Load BES from input_filename. This guesses the file type of
///        input_filename based on the extension.
template <typename Container>
inline
void load_bes(boolean_equation_system<Container>& bes, std::string const& input_filename)
{
  bes_output_format format = guess_file_format(input_filename);
  std::ifstream input; // Cannot declare in switch

  switch (format)
  {
    case bes_output_bes:
      bes.load(input_filename);
      break;
    case bes_output_cwi:
      throw mcrl2::runtime_error("Reading BES from cwi format is not supported");
      break;
    case bes_output_pgsolver:
      input.open(input_filename.c_str());
      parse_pgsolver(input, bes);
      break;
    default:
      throw mcrl2::runtime_error("Trying to read BES from unsupported format");
  }
}

/// \brief Save BES to output_filename. The type is guessed based upon the
///        extension of output_filename
template <typename Container>
inline
void save_bes(boolean_equation_system<Container> const& bes, std::string const& output_filename)
{
  bes_output_format format = guess_file_format(output_filename);
  save_bes(bes, output_filename, format);
}

}
}

class bestranslate_tool: public input_output_tool
{
  private:
    typedef input_output_tool super;

  public:
    bestranslate_tool()
      : super("bestranslate", "Jeroen Keiren",
              "translate a BES between various formats",
              "Translate BES in INFILE to OUTFILE converting between the formats"
              "specified in the filename. If OUTFILE is not present, stdout is"
              "used. If INFILE is not present, stdin is used."
             )
    {}

    bool run()
    {
      using namespace mcrl2::bes;
      boolean_equation_system<> bes;
      load_bes(bes, input_filename());
      save_bes(bes, output_filename());
      return true;
    }
};


int main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)
  return bestranslate_tool().execute(argc, argv);
}

