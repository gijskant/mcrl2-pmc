// Author(s): Jeroen Keiren
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file lpsdecluster.cpp
/// \brief Add your file description here.

// ======================================================================
//
// Copyright (c) 2006 TU/e
//
// ----------------------------------------------------------------------
//
// file          : lpsdecluster 
// date          : 22-12-2006
// version       : 0.5
//
// author(s)     : Jeroen Keiren <j.j.a.keiren@student.tue.nl>
//
// ======================================================================

#define NAME "lpsdecluster"
#define AUTHOR "Jeroen Keiren"

//C++
#include <exception>
#include <cstdio>

//Aterms
#include <mcrl2/atermpp/aterm.h>

#include <mcrl2/lps/decluster.h>

#include "mcrl2/core/messaging.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "mcrl2/utilities/command_line_interface.h" // must come after mcrl2/core/messaging.h

using namespace std;
using namespace ::mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2;

///////////////////////////////////////////////////////////////////////////////
/// \brief structure that holds all options available for the tool.
///
struct tool_options {
  std::string input_file; ///< Name of the file to read input from
  std::string output_file; ///< Name of the file to write output to (or stdout)
  bool finite_only; ///< Only decluster finite sorts
  RewriteStrategy strategy; ///< Rewrite strategy to use, default jitty
};

//Squadt connectivity
#ifdef ENABLE_SQUADT_CONNECTIVITY
#include <mcrl2/utilities/squadt_interface.h>

//Forward declaration because do_decluster() is called within squadt_interactor class
int do_decluster(const tool_options& options);

class squadt_interactor: public mcrl2::utilities::squadt::mcrl2_tool_interface
{
  private:

    static const char*  lps_file_for_input;  ///< file containing an LPS that can be imported
    static const char*  lps_file_for_output; ///< file used to write the output to

    static const char*  option_finite_only;
    static const char*  option_rewrite_strategy;

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

const char* squadt_interactor::option_finite_only      = "finite_only";
const char* squadt_interactor::option_rewrite_strategy = "rewrite_strategy";

void squadt_interactor::set_capabilities(tipi::tool::capabilities& capabilities) const
{
  // The tool has only one main input combination
  capabilities.add_input_configuration(lps_file_for_input, tipi::mime_type("lps", tipi::mime_type::application), tipi::tool::category::transformation);
}

void squadt_interactor::user_interactive_configuration(tipi::configuration& configuration)
{
  using namespace tipi;
  using namespace tipi::layout;
  using namespace tipi::datatype;
  using namespace tipi::layout::elements;

  /* Set defaults where the supplied configuration does not have values */
  if (!configuration.output_exists(lps_file_for_output)) {
    configuration.add_output(lps_file_for_output, tipi::mime_type("lps", tipi::mime_type::application), configuration.get_output_name(".lps"));
  }
  if (!configuration.option_exists(option_rewrite_strategy)) {
    configuration.add_option(option_rewrite_strategy).append_argument(mcrl2::utilities::squadt::rewrite_strategy_enumeration, 0);
  }
  if (!configuration.option_exists(option_finite_only)) {
    configuration.add_option(option_finite_only).
       set_argument_value< 0, tipi::datatype::boolean >(true, false);
  }

  /* Create display */
  tipi::layout::tool_display d;

  // Helper for strategy selection
  mcrl2::utilities::squadt::radio_button_helper < RewriteStrategy > strategy_selector(d);

  layout::vertical_box& m = d.create< vertical_box >();

  m.append(d.create< label >().set_text("Rewrite strategy")).
    append(d.create< horizontal_box >().
                append(strategy_selector.associate(GS_REWR_INNER, "Inner")).
                append(strategy_selector.associate(GS_REWR_INNERC, "Innerc")).
                append(strategy_selector.associate(GS_REWR_JITTY, "Jitty")).
                append(strategy_selector.associate(GS_REWR_JITTYC, "Jittyc")));

  /* Prepare user interaction */
  checkbox& finite_only = d.create< checkbox >().set_status(configuration.get_option_argument< bool >(option_finite_only));
  
  m.append(d.create< label >().set_text(" ")).
    append(finite_only.set_label("Only decluster variables of finite sorts"), layout::left);

  button& okay_button = d.create< button >().set_label("OK");

  m.append(d.create< label >().set_text(" ")).
    append(okay_button, layout::right);

  if (configuration.option_exists(option_rewrite_strategy)) {
    strategy_selector.set_selection(static_cast < RewriteStrategy > (
        configuration.get_option_argument< size_t >(option_rewrite_strategy, 0)));
  }
  
  send_display_layout(d.set_manager(m));

  okay_button.await_change();
  
  /* Update configuration */
  configuration.get_option(option_finite_only).
     set_argument_value< 0, tipi::datatype::boolean >(finite_only.get_status());

  configuration.get_option(option_rewrite_strategy).replace_argument(0, mcrl2::utilities::squadt::rewrite_strategy_enumeration, strategy_selector.get_selection());
}

bool squadt_interactor::check_configuration(tipi::configuration const& configuration) const
{
  bool result = true;
  result |= configuration.input_exists(lps_file_for_input);
  result |= configuration.output_exists(lps_file_for_output);
  result |= configuration.option_exists(option_rewrite_strategy);

  return result;
}

bool squadt_interactor::perform_task(tipi::configuration& configuration)
{
  using namespace tipi;
  using namespace tipi::layout;
  using namespace tipi::datatype;
  using namespace tipi::layout::elements;

  tool_options options;
  options.input_file = configuration.get_input(lps_file_for_input).get_location();
  options.output_file = configuration.get_output(lps_file_for_output).get_location();
  options.finite_only = configuration.option_exists(option_finite_only);
  options.strategy = static_cast < RewriteStrategy > (boost::any_cast < size_t > (configuration.get_option_argument(option_rewrite_strategy, 0)));

  /* Create display */
  tipi::layout::tool_display d;

  send_display_layout(d.set_manager(d.create< vertical_box >().
                append(d.create< label >().set_text("Declustering in progress"), layout::left)));

  //Perform declustering
  bool result = do_decluster(options) == 0;

  send_display_layout(d.set_manager(d.create< vertical_box >().
                append(d.create< label >().set_text(std::string("Declustering ") + ((result) ? "succeeded" : "failed")), layout::left)));

  return result;
}

#endif //ENABLE_SQUADT_CONNECTIVITY


///Reads a specification from input_file, 
///applies declustering to it and writes the result to output_file.
int do_decluster(const tool_options& options)
{
  lps::specification lps_specification;

  try
  {
    lps_specification.load(options.input_file);
    Rewriter* r = createRewriter(lps_specification.data(), options.strategy);

    lps::specification result = lps::decluster(lps_specification, *r, options.finite_only);

    // decluster lps_specification and save the output to a binary file
    if (!result.save(options.output_file, true)) 
    {
      // An error occurred when saving
      gsErrorMsg("Could not save to '%s'\n", options.output_file.c_str());
      return (1);
    }
  }
  catch (std::exception& e)
  {
    gsErrorMsg("lpsdecluster: Unable to load LPS from `%s'\n", options.input_file.c_str());
    return (1);
  }

  return 0;
}

///Parses command line and sets settings from command line switches
tool_options parse_command_line(int ac, char** av) {
  interface_description clinterface(av[0], NAME, AUTHOR, " [OPTION]... [INFILE [OUTFILE]]\n"
                              "Instantiate the summation variables of the linear process specification (LPS)\n"
                              "in INFILE and write the result to OUTFILE. If INFILE is not present, stdin is\n"
                              "used. If OUTFILE is not present, stdout is used.\n");

  clinterface.add_option("finite", "only instantiate variables whose sorts are finite", 'f');

  clinterface.add_rewriting_options();

  command_line_parser parser(clinterface, ac, av);

  tool_options t_options = { "-", "-", (0 < parser.options.count("finite")), GS_REWR_JITTY };

  if (0 < parser.unmatched.size()) {
    t_options.input_file = parser.unmatched[0];
  }
  if (1 < parser.unmatched.size()) {
    t_options.output_file = parser.unmatched[1];
  }
  if (2 < parser.unmatched.size()) {
    clinterface.throw_exception("too many file arguments");
  }

  t_options.strategy = RewriteStrategyFromString(parser.option_argument("rewriter").c_str());

  return t_options;
}

int main(int argc, char** argv) {
  MCRL2_ATERM_INIT(argc, argv)

  try {
#ifdef ENABLE_SQUADT_CONNECTIVITY
    if (mcrl2::utilities::squadt::interactor< squadt_interactor >::free_activation(argc, argv)) {
      return 0;
    }
#endif

    return do_decluster(parse_command_line(argc,argv));
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return EXIT_FAILURE;
}
