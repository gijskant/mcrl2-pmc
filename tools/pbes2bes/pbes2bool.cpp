// Author(s): Jan Friso Groote
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbes2bool.cpp
/// \brief Add your file description here.

// ======================================================================
//
// file          : pbes2bool
// date          : 15-04-2007
// version       : 0.1.3
//
// author(s)     : Alexander van Dam <avandam@damdonk.nl>
//                 Jan Friso Groote <J.F.Groote@tue.nl>
//
// ======================================================================


#define NAME "pbes2bool"
#define AUTHOR "Jan Friso Groote"

//C++
#include <ostream>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>

#include <sstream>

//Boost
#include <boost/program_options.hpp>

//MCRL2-specific
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/version_info.h"
#include "mcrl2/utilities/aterm_ext.h"

//LPS-Framework
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/utility.h"
#include "mcrl2/data/data_operators.h"
#include "mcrl2/data/sort_expression.h"
#include "mcrl2/pbes/pbes2bool.h"

//ATERM-specific
#include "mcrl2/atermpp/substitute.h"
#include "mcrl2/atermpp/utility.h"
#include "mcrl2/atermpp/indexed_set.h"
#include "mcrl2/atermpp/table.h"
#include "_aterm.h"

//Tool-specific
// #include "pbes_rewrite_jfg.h"
// #include "sort_functions.h"
#include "bes.h"

using namespace std;
using namespace lps;
using namespace mcrl2::utilities;
using namespace mcrl2::core;
using bes::bes_expression;

using atermpp::make_substitution;

namespace po = boost::program_options;

//Function declarations used by main program
//------------------------------------------
static t_tool_options parse_command_line(int argc, char** argv);
//Post: The command line options are parsed.
//      The program has aborted with a suitable error code, if:
//    - errors were encounterd
//    - non-standard behaviour was requested (help, version)
//Ret:  The parsed command line options

// SQuADT protocol interface
#ifdef ENABLE_SQUADT_CONNECTIVITY
#include <mcrl2/utilities/squadt_interface.h>

class squadt_interactor : public mcrl2::utilities::squadt::mcrl2_tool_interface {

  private:

    static const char*  pbes_file_for_input;             ///< file containing an LPS
    static const char*  bes_file_for_output;             ///< file used to write the output to
    static const char*  counter_example_file_for_output; ///< file used to write the output to

    enum bes_output_format {
      none,
      vasy,
      cwi
    };

    static const char* option_transformation_strategy;
    static const char* option_rewrite_strategy;
    static const char* option_selected_output_format;
    static const char* option_precompile;
    static const char* option_counter;
    static const char* option_hash_table;
    static const char* option_tree;
    static const char* option_unused_data;

  private:

    boost::shared_ptr < tipi::datatype::enumeration > transformation_strategy_enumeration;
    boost::shared_ptr < tipi::datatype::enumeration > output_format_enumeration;

  public:

    /** \brief constructor */
    squadt_interactor();

    /** \brief configures tool capabilities */
    void set_capabilities(tipi::tool::capabilities&) const;

    /** \brief queries the user via SQuADT if needed to obtain configuration information */
    void user_interactive_configuration(tipi::configuration&);

    /** \brief check an existing configuration object to see if it is usable */
    bool check_configuration(tipi::configuration const&) const;

    /** \brief performs the task specified by a configuration */
    bool perform_task(tipi::configuration&);
};

const char* squadt_interactor::pbes_file_for_input             = "pbes_in";
const char* squadt_interactor::bes_file_for_output             = "bes_out";
const char* squadt_interactor::counter_example_file_for_output = "counter_example";

const char* squadt_interactor::option_transformation_strategy = "transformation_strategy";
const char* squadt_interactor::option_selected_output_format  = "selected_output_format";
const char* squadt_interactor::option_rewrite_strategy        = "rewrite_strategy";
const char* squadt_interactor::option_precompile              = "precompile";
const char* squadt_interactor::option_counter                 = "counter";
const char* squadt_interactor::option_hash_table              = "hash_table";
const char* squadt_interactor::option_tree                    = "tree";
const char* squadt_interactor::option_unused_data             = "unused_data";

squadt_interactor::squadt_interactor() {
  transformation_strategy_enumeration.reset(new tipi::datatype::enumeration("lazy"));

  *transformation_strategy_enumeration % "optimize" % "on-the-fly" % "on-the-fly-with-fixpoints";

  output_format_enumeration.reset(new tipi::datatype::enumeration("none"));

  *output_format_enumeration % "vasy" % "cwi";
}

void squadt_interactor::set_capabilities(tipi::tool::capabilities& c) const {
  c.add_input_configuration(pbes_file_for_input, tipi::mime_type("pbes", tipi::mime_type::application), tipi::tool::category::transformation);
}

void squadt_interactor::user_interactive_configuration(tipi::configuration& c) {
  using namespace tipi;
  using namespace tipi::layout;
  using namespace tipi::layout::elements;

  if (!c.option_exists(option_precompile)) {
    c.add_option(option_precompile).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_counter)) {
    c.add_option(option_counter).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_hash_table)) {
    c.add_option(option_hash_table).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_tree)) {
    c.add_option(option_tree).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_unused_data)) {
    c.add_option(option_unused_data).set_argument_value< 0, tipi::datatype::boolean >(true);
  }
  if (!c.option_exists(option_rewrite_strategy)) {
    c.add_option(option_rewrite_strategy).append_argument(mcrl2::utilities::squadt::rewrite_strategy_enumeration, 0);
  }
  if (!c.option_exists(option_transformation_strategy)) {
    c.add_option(option_transformation_strategy).append_argument(transformation_strategy_enumeration, lazy);
  }
  if (!c.option_exists(option_selected_output_format)) {
    c.add_option(option_selected_output_format).append_argument(output_format_enumeration, none);
  }

  /* Create display */
  tipi::layout::tool_display d;

  // Helper for format selection
  mcrl2::utilities::squadt::radio_button_helper < bes_output_format > format_selector(d);

  // Helper for strategy selection
  mcrl2::utilities::squadt::radio_button_helper < RewriteStrategy > rewrite_strategy_selector(d);

  // Helper for strategy selection
  mcrl2::utilities::squadt::radio_button_helper < transformation_strategy > strategy_selector(d);

  layout::vertical_box& m = d.create< vertical_box >().set_default_margins(margins(0,5,0,5));

  checkbox& precompile(d.create< checkbox >().set_status(c.get_option_argument< bool >(option_precompile)));
  checkbox& counter(d.create< checkbox >().set_status(c.get_option_argument< bool >(option_counter)));
  checkbox& hash_table(d.create< checkbox >().set_status(c.get_option_argument< bool >(option_hash_table)));
  checkbox& tree(d.create< checkbox >().set_status(c.get_option_argument< bool >(option_tree)));
  checkbox& unused_data(d.create< checkbox >().set_status(c.get_option_argument< bool >(option_unused_data)));

  m.append(d.create< label >().set_text("Rewrite strategy")).
    append(d.create< horizontal_box >().
                append(rewrite_strategy_selector.associate(GS_REWR_INNER, "Inner")).
                append(rewrite_strategy_selector.associate(GS_REWR_INNERC, "Innerc")).
                append(rewrite_strategy_selector.associate(GS_REWR_JITTY, "Jitty")).
                append(rewrite_strategy_selector.associate(GS_REWR_JITTYC, "Jittyc")),
          margins(0,5,0,5)).
    append(d.create< label >().set_text("Output format : ")).
    append(d.create< horizontal_box >().
                append(format_selector.associate(none, "none")).
                append(format_selector.associate(vasy, "vasy")).
                append(format_selector.associate(cwi, "cwi")),
          margins(0,5,0,5)).
    append(d.create< label >().set_text("Strategy to generate a BES from a PBES: "), margins(8,5,0,5)).
    append(d.create< vertical_box >().
        append(strategy_selector.associate(lazy, "0: without optimisation")).
        append(strategy_selector.associate(optimize, "1: forward substitution of true/false")).
        append(strategy_selector.associate(on_the_fly, "2: full substitution of true/false")).
        append(strategy_selector.associate(on_the_fly_with_fixed_points, "3: full substitution and cycle detection")),
          margins(0,5,8,5)).
    append(d.create< horizontal_box >().
        append(d.create< vertical_box >().
            append(precompile.set_label("precompile for faster rewriting")).
            append(counter.set_label("produce a counter example")).
            append(unused_data.set_label("remove unused data"))).
        append(d.create< vertical_box >().
            append(hash_table.set_label("use hash tables and translation to BDDs")).
            append(tree.set_label("store state in a tree (memory efficiency)"))));

  button& okay_button = d.create< button >().set_label("OK");

  m.append(d.create< label >().set_text(" ")).
    append(okay_button, layout::right);

  /// Copy values from options specified in the configuration
  if (c.option_exists(option_transformation_strategy)) {
    strategy_selector.set_selection(static_cast < transformation_strategy > (
        c.get_option_argument< size_t >(option_transformation_strategy, 0)));
  }
  if (c.option_exists(option_selected_output_format)) {
    format_selector.set_selection(static_cast < bes_output_format > (
        c.get_option_argument< size_t >(option_selected_output_format, 0)));
  }
  if (c.option_exists(option_rewrite_strategy)) {
    rewrite_strategy_selector.set_selection(static_cast < RewriteStrategy > (
        c.get_option_argument< size_t >(option_rewrite_strategy, 0)));
  }

  send_display_layout(d.set_manager(m));

  /* Wait until the ok button was pressed */
  okay_button.await_change();

  c.get_option(option_transformation_strategy).replace_argument(0, transformation_strategy_enumeration,
                                static_cast < transformation_strategy > (strategy_selector.get_selection()));
  c.get_option(option_selected_output_format).replace_argument(0, output_format_enumeration,
                                static_cast < bes_output_format > (format_selector.get_selection()));
  c.get_option(option_rewrite_strategy).replace_argument(0, mcrl2::utilities::squadt::rewrite_strategy_enumeration,
                                static_cast < RewriteStrategy > (rewrite_strategy_selector.get_selection()));

  if (c.get_option_argument< size_t >(option_selected_output_format)!=none)
  {
    /* Add output file to the configuration */
    if (c.output_exists(bes_file_for_output)) {
      tipi::object& output_file = c.get_output(bes_file_for_output);
   
      output_file.set_location(c.get_output_name(".txt"));
    }
    else {
      c.add_output(bes_file_for_output, tipi::mime_type("txt", tipi::mime_type::application), 
                   c.get_output_name(".txt"));
    }
  }

  c.get_option(option_precompile).set_argument_value< 0, boolean >(precompile.get_status());
  c.get_option(option_counter).set_argument_value< 0, boolean >(counter.get_status());
  c.get_option(option_hash_table).set_argument_value< 0, boolean >(hash_table.get_status());
  c.get_option(option_tree).set_argument_value< 0, boolean >(tree.get_status());
  c.get_option(option_unused_data).set_argument_value< 0, boolean >(unused_data.get_status());

  send_clear_display();
}

bool squadt_interactor::check_configuration(tipi::configuration const& c) const {
  bool result = true;

  result &= c.input_exists(pbes_file_for_input);
  result &= c.option_exists(option_transformation_strategy);
  result &= c.option_exists(option_selected_output_format);
  if (result && (c.get_option_argument< size_t >(option_selected_output_format)!=none))
  { /* only check for the existence of an outputfile if the output format does
       not equal none */
    result &= c.output_exists(bes_file_for_output);
  }

  return (result);
}

bool squadt_interactor::perform_task(tipi::configuration& c) {
  // static std::string strategies[] = { "lazy", "fly" };
  static std::string formats[]    = { "none", "vasy", "cwi" };

  t_tool_options tool_options;

  tool_options.opt_precompile_pbes           = c.get_option_argument< bool >(option_precompile);
  tool_options.opt_construct_counter_example = c.get_option_argument< bool >(option_counter);
  tool_options.opt_store_as_tree             = c.get_option_argument< bool >(option_tree);
  tool_options.opt_data_elm                  = c.get_option_argument< bool >(option_unused_data);;
  tool_options.opt_use_hashtables            = c.get_option_argument< bool >(option_hash_table);;
  tool_options.rewrite_strategy              = static_cast < RewriteStrategy > (
                        c.get_option_argument< size_t >(option_rewrite_strategy, 0));

  if (tool_options.opt_construct_counter_example && !c.output_exists(counter_example_file_for_output)) {
    tool_options.opt_counter_example_file = c.get_output_name(".txt").c_str();

    c.add_output(counter_example_file_for_output, tipi::mime_type("txt", tipi::mime_type::text), 
                 tool_options.opt_counter_example_file);
  }

  tool_options.opt_outputformat = formats[c.get_option_argument< size_t >(option_selected_output_format)];

  tool_options.opt_strategy = static_cast < transformation_strategy > (
                        c.get_option_argument< size_t >(option_transformation_strategy, 0));

  tool_options.infilename       = c.get_input(pbes_file_for_input).get_location();

  if (c.output_exists(bes_file_for_output)) {
    tool_options.outfilename = c.get_output(bes_file_for_output).get_location();
  }

  send_clear_display();

  bool result = process(tool_options);
 
  return (result);
}
#endif


//Main Program
//------------
int main(int argc, char** argv)
{
  MCRL2_ATERM_INIT(argc, argv)

#ifdef ENABLE_SQUADT_CONNECTIVITY
  if (mcrl2::utilities::squadt::interactor< squadt_interactor >::free_activation(argc, argv)) {
    return 0;
  }
#endif

  return process(parse_command_line(argc, argv));
}

//function parse_command_line
//---------------------------
t_tool_options parse_command_line(int argc, char** argv)
{
  t_tool_options tool_options;
  string opt_outputformat;
  string opt_strategy;
  bool opt_precompile_pbes=false;
  bool opt_use_hashtables=false;
  bool opt_construct_counter_example=false;
  bool opt_store_as_tree=false;
  bool opt_data_elm=true;

  string opt_rewriter;
  vector< string > file_names;

  po::options_description desc;

  desc.add_options()
      ("strategy,s",  po::value<string>(&opt_strategy)->default_value("0"), "use strategy arg (default '0');\n"
       "The following strategies are available:\n"
       "0) Compute all boolean equations which can be reached from the initial state, without"
       "any optimization (default). This is is the most data efficient option per generated equation.\n"
       "1) Optimize by immediately substituting the the right hand sides for already investigated"
       "variables that are true or false when generating a expression. This is as memory efficient as 0.\n"
       "2) In addition to 1, also substitute variables that are true or false into an already generated"
       "right hand sides. This can mean that certain variables become unreachable (e.g. X0 in X0 && X1,"
       "when X1 becomes false, assuming X0 does not occur elsewhere. It will be maintained which variables"
       "have become unreachable as these do not have to be investigated. Depending on the PBES, this can"
       "reduce the size of the generated BES substantially, but requires a larger memory footstamp.\n"
       "3) In addition to 2, investigate for generated variables whether they occur on a loop, such that"
       "they can be set to true or false, depending on the fixed point symbol. This can increase the time"
       "needed to generate an equation substantially")
      ("rewriter,R", po::value<string>(&opt_rewriter)->default_value("jitty"), "indicate the rewriter to be used. Options are:\n"
       "inner   interpreting innermost rewriter,\n"
       "jitty   interpreting just in time rewriter (default),\n"
       "innerc  compiling innermost rewriter (not for Windows),\n"
       "jittyc  compiling just in time rewriter (fastest, not for Windows).\n")
      ("counter,c", "Print at the end a tree labelled with instantiations of the left hand side of"
       "equations. This tree is an indication of how pbes2bool came to the validity/invalidity of the PBES.")
      ("precompile,P", "Precompile the pbes for faster rewriting. Does not work when the toolset is compiled in debug mode")
      ("hashtables,H", "Use hashtables when substituting in bes equations, and translate internal expressions to binary decision diagrams (discouraged, due to heavy performance penalties).")
      ("output,o",  po::value<string>(&opt_outputformat)->default_value("none"), "use outputformat arg (default 'none');\n"
               "available outputformats are none, vasy and cwi")
      ("tree,r", "store state in a tree (for memory efficiency)")
      ("unused-data,u","do not remove unused parts of the data specification")
      ("verbose,v",  "turn on the display of short intermediate gsMessages")
      ("debug,d",    "turn on the display of detailed intermediate gsMessages")
      ("version",    "display version information")
      ("help,h",    "display this help")
      ;

  po::options_description hidden("Hidden options");
  hidden.add_options()
      ("file_names",  po::value< vector< string > >(), "input/output files")
      ;

  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);

  po::options_description visible("Allowed options");
  visible.add(desc);

  po::positional_options_description p;
  p.add("file_names", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    cout << "Usage: " << argv[0] << " [OPTION]... [INFILE [OUTFILE]]" << endl;
    cout << "Solves PBES from INFILE, or writes an equivalent BES to OUTFILE. If INFILE is" << endl;
    cout << "not present, stdin is used. If OUTFILE is not present, stdout is used." << endl;
    cout << endl;
    cout << "Options:" << endl;
    cout << desc;
    cout << endl;
    cout << "Report bugs at <http://www.mcrl2.org/issuetracker>." << endl;

    exit(0);
  }
 
  if (vm.count("precompile"))
  { opt_precompile_pbes=true;
  }

  if (vm.count("hashtables"))
  { opt_use_hashtables=true;
  }

  if (vm.count("counter"))
  { opt_construct_counter_example=true;
  }
 
  if (vm.count("tree"))
  { opt_store_as_tree=true;
  }

  if (vm.count("unused-data"))
  { opt_data_elm=false;
  }

  if (vm.count("version"))
  {
    print_version_information(NAME, AUTHOR);
    exit(0);
  }

  if (vm.count("debug"))
  {
    gsSetDebugMsg();
  }

  if (vm.count("verbose"))
  {
    gsSetVerboseMsg();
  }

  if (vm.count("output")) // Output format
  {
    opt_outputformat = vm["output"].as< string >();
    if (!((opt_outputformat == "none") || (opt_outputformat == "vasy") || (opt_outputformat == "cwi")))
    {
      gsErrorMsg("Unknown outputformat specified. Available outputformats are none, vasy and cwi\n");
      exit(1);
    }
  }

  if (vm.count("strategy")) // Bes solving strategy (currently only one available)
  {
    opt_strategy = vm["strategy"].as< string >();
    if (!(opt_strategy == "0" || opt_strategy == "1" || opt_strategy == "2" || opt_strategy == "3" ))
    {
      gsErrorMsg("Unknown strategy specified. The available strategies are 0, 1, 2, and 3\n");
      exit(1);
    }
  }

  if (vm.count("rewriter")) // Select the rewiter to be used
  { 
    opt_rewriter = vm["rewriter"].as< string >();
    if (!(opt_rewriter == "inner") &&
        !(opt_rewriter == "jitty") &&
        !(opt_rewriter == "innerc") &&
        !(opt_rewriter == "jittyc"))
    { 
      gsErrorMsg("Unknown rewriter specified. Available rewriters are inner, jitty, innerc and jittyc\n");
      exit(1);
    }

        
  }
  
  if (vm.count("file_names"))
  {
    file_names = vm["file_names"].as< vector< string > >();
  }

  string infilename;
  string outfilename;
  if (file_names.size() == 0)
  {
    // Read from and write to stdin
    infilename = "-";
    outfilename = "-";
  }
  else if ( 2 < file_names.size())
  {
    cerr << NAME << ": Too many arguments" << endl;
    exit(1);
  }
  else
  {
    infilename = file_names[0];
    if (file_names.size() == 2)
    {
      outfilename = file_names[1];
    }
    else
    {
      outfilename = "-";
    }
  }
  
  tool_options.infilename = infilename;
  tool_options.outfilename = outfilename;
  
  tool_options.opt_outputformat = opt_outputformat;

  if (opt_strategy=="0")
  { tool_options.opt_strategy=lazy;
  }
  else if (opt_strategy=="1")
  { tool_options.opt_strategy=optimize;
  }
  else if (opt_strategy=="2")
  { tool_options.opt_strategy=on_the_fly;
  }
  else if (opt_strategy=="3")
  { tool_options.opt_strategy=on_the_fly_with_fixed_points;
  }
  else assert(0); // Unknown strategy. Should not occur here, as there is a check for this above.
  
  if (opt_rewriter=="inner")
  { tool_options.rewrite_strategy=GS_REWR_INNER;
  }
  else if (opt_rewriter=="jitty")
  { tool_options.rewrite_strategy=GS_REWR_JITTY;
  }
  else if (opt_rewriter=="innerc")
  { tool_options.rewrite_strategy=GS_REWR_INNERC;
  }
  else if (opt_rewriter=="jittyc")
  { tool_options.rewrite_strategy=GS_REWR_JITTYC;
  }
  else assert(0); // Unknown rewriter specified. Should have been caught above.

  tool_options.opt_precompile_pbes=opt_precompile_pbes;
  tool_options.opt_construct_counter_example=opt_construct_counter_example;
  tool_options.opt_store_as_tree=opt_store_as_tree;
  tool_options.opt_data_elm=opt_data_elm;
  tool_options.opt_use_hashtables=opt_use_hashtables;
  
  return tool_options;
}
