// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lpssumelm.cpp

#define NAME "lpssumelm"
#define AUTHOR "Jeroen Keiren"

//C++
#include <exception>
#include <cstdio>

//Aterms
#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "mcrl2/utilities/command_line_interface.h" // must come after mcrl2/core/messaging.h

//LPS Framework
#include <mcrl2/lps/sumelm.h>

using namespace std;
using namespace ::mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2;

struct tool_options {
  std::string input_file; ///< Name of the file to read input from
  std::string output_file; ///< Name of the file to write output to (or stdout)
};
  
//Squadt connectivity
#ifdef ENABLE_SQUADT_CONNECTIVITY
#include <mcrl2/utilities/squadt_interface.h>

//Forward declaration because do_sumelm() is called within squadt_interactor class
int do_sumelm(const tool_options& options);

class squadt_interactor: public mcrl2::utilities::squadt::mcrl2_tool_interface
{
  private:

    static const char*  lps_file_for_input;  ///< file containing an LPS that can be imported
    static const char*  lps_file_for_output; ///< file used to write the output to

  public:
    /** \brief configures tool capabilities */
    void set_capabilities(tipi::tool::capabilities&) const;

    /** \brief queries the user via SQuADT if needed to obtain configuration information */
    void user_interactive_configuration(tipi::configuration&);

    /** \brief check an existing configuration object to see if it is usable */
    bool check_configuration(tipi::configuration const&) const;

    /** \brief performs the task specified by a configuration */
    bool perform_task(tipi::configuration&);
};

const char* squadt_interactor::lps_file_for_input  = "lps_in";
const char* squadt_interactor::lps_file_for_output = "lps_out";

void squadt_interactor::set_capabilities(tipi::tool::capabilities& capabilities) const
{
  // The tool has only one main input combination
  gsDebugMsg("squadt_interactor: Setting capabilities\n");
  capabilities.add_input_configuration(lps_file_for_input, tipi::mime_type("lps", tipi::mime_type::application), tipi::tool::category::transformation);
}

void squadt_interactor::user_interactive_configuration(tipi::configuration& configuration)
{
  gsDebugMsg("squadt_interactor: User interactive configuration\n");

  if (configuration.is_fresh()) {
    if (!configuration.output_exists(lps_file_for_output)) {
      configuration.add_output(lps_file_for_output, tipi::mime_type("lps", tipi::mime_type::application), configuration.get_output_name(".lps"));
    }
  }
}

bool squadt_interactor::check_configuration(tipi::configuration const& configuration) const
{
  gsDebugMsg("squadt_interactor: Checking configuration\n");
  // Check if everything present
  return (configuration.input_exists(lps_file_for_input) &&
          configuration.output_exists(lps_file_for_output)
         );
}

bool squadt_interactor::perform_task(tipi::configuration& configuration)
{
  gsDebugMsg("squadt_interactor: Performing task\n");
  tool_options options;
  options.input_file = configuration.get_input(lps_file_for_input).get_location();
  options.output_file = configuration.get_output(lps_file_for_output).get_location();

  gsDebugMsg("Calling do_sumelm through SQuADT, with input: %s and output: %s\n", options.input_file.c_str(), options.output_file.c_str());
  return (do_sumelm(options)==0);
}

#endif //ENABLE_SQUADT_CONNECTIVITY

///Reads a specification from input_file, 
///applies sum elimination to it and writes the result to output_file.
int do_sumelm(const tool_options& options)
{
  lps::specification lps_specification;
    
  lps_specification.load(options.input_file);

  // Untime lps_specification and save the output to a binary file
  lps::specification new_spec = lps::sumelm(lps_specification);

  gsDebugMsg("Sum elimination completed, saving to %s\n", options.output_file.c_str());
  new_spec.save(options.output_file);

  return 0;
}

///Parses command line and sets settings from command line switches
tool_options parse_command_line(int ac, char** av) {
  interface_description clinterface(av[0], NAME, AUTHOR, "[OPTION]... [INFILE [OUTFILE]]\n"
                             "Remove superfluous summations from the linear process specification (LPS) in "
                             "INFILE and write the result to OUTFILE. If INFILE is not present, stdin is used. "
                             "If OUTFILE is not present, stdout is used.");

  command_line_parser parser(clinterface, ac, av);

  tool_options t_options;

  if (2 < parser.arguments.size()) {
    parser.error("too many file arguments");
  }
  else {
    if (0 < parser.arguments.size()) {
      t_options.input_file = parser.arguments[0];
    }
    if (1 < parser.arguments.size()) {
      t_options.output_file = parser.arguments[1];
    }
  }

  return t_options;
}

int main(int argc, char** argv)
{
  MCRL2_ATERM_INIT(argc, argv)

  try {
#ifdef ENABLE_SQUADT_CONNECTIVITY
    if (mcrl2::utilities::squadt::interactor< squadt_interactor >::free_activation(argc, argv)) {
      return EXIT_SUCCESS;
    }
#endif

    return do_sumelm(parse_command_line(argc, argv));
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return EXIT_FAILURE;
}
