// Author(s): Jan Friso Groote
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl22lps.cpp
/// \brief Add your file description here.

#define NAME "mcrl22lps"
#define AUTHOR "Jan Friso Groote"

#include <cassert>
#include <stdbool.h>
#include <getopt.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <fstream>
#include "aterm2.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/lin_types.h"
#include "mcrl2/lps/lin_std.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "mcrl2/core/struct.h"
#include "mcrl2/core/detail/parse.h"
#include "mcrl2/core/detail/typecheck.h"
#include "mcrl2/core/detail/data_implementation.h"
#include "mcrl2/core/alpha.h"
#include "mcrl2/utilities/version_info.h"

#define INFILEEXT ".mcrl2"
#define OUTFILEEXT ".lps"

#ifdef __cplusplus
using namespace ::mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2::core::detail;
using namespace::mcrl2;
#endif

//Functions used by the main program
static t_lin_options parse_command_line(int argc, char *argv[]);
static ATermAppl linearise_file(t_lin_options &lin_options);
static char const* lin_method_to_string(t_lin_method lin_method);
static void PrintMoreInfo(char *Name);
static void PrintHelp(char *Name);

// Squadt protocol interface and utility pseudo-library
#ifdef ENABLE_SQUADT_CONNECTIVITY
#include "mcrl2/utilities/squadt_interface.h"

class squadt_interactor : public mcrl2::utilities::squadt::mcrl2_tool_interface {

  private:

    static const char* mcrl2_file_for_input; ///< file containing an LTS that can be imported using the LTS library
    static const char* lps_file_for_output;  ///< file containing an LTS that can be imported using the LTS library

    static const char* option_linearisation_method;
    static const char* option_final_cluster;
    static const char* option_no_intermediate_cluster;
    static const char* option_no_alpha;
    static const char* option_newstate;
    static const char* option_binary;
    static const char* option_statenames;
    static const char* option_no_rewrite;
    static const char* option_no_freevars;
    static const char* option_no_sumelm;
    static const char* option_no_deltaelm;
    static const char* option_end_phase;
    static const char* option_add_delta;

  private:

    boost::shared_ptr < tipi::datatype::enumeration > linearisation_method_enumeration;

    boost::shared_ptr < tipi::datatype::enumeration > linearisation_phase_enumeration;

  private:

    /** \brief compiles a t_lin_options instance from a configuration */
    bool extract_task_options(tipi::configuration const& c, t_lin_options&) const;

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

const char* squadt_interactor::mcrl2_file_for_input = "mcrl2_in";
const char* squadt_interactor::lps_file_for_output  = "lps_out";

const char* squadt_interactor::option_linearisation_method     = "linearisation_method";
const char* squadt_interactor::option_final_cluster            = "final_cluster";
const char* squadt_interactor::option_no_intermediate_cluster  = "no_intermediate_cluster";
const char* squadt_interactor::option_no_alpha                 = "no_alpha";
const char* squadt_interactor::option_newstate                 = "newstate";
const char* squadt_interactor::option_binary                   = "binary";
const char* squadt_interactor::option_statenames               = "statenames";
const char* squadt_interactor::option_no_rewrite               = "no_rewrite";
const char* squadt_interactor::option_no_freevars              = "no_freevars"; 
const char* squadt_interactor::option_no_sumelm                = "no_sumelm";
const char* squadt_interactor::option_no_deltaelm              = "no_dataelm";
const char* squadt_interactor::option_end_phase                = "end_phase";
const char* squadt_interactor::option_add_delta                = "add_delta";

squadt_interactor::squadt_interactor() {
  linearisation_method_enumeration.reset(new tipi::datatype::enumeration("stack"));

  *linearisation_method_enumeration % "regular" % "regular2" % "expansion";

  linearisation_phase_enumeration.reset(new tipi::datatype::enumeration("all"));

  *linearisation_phase_enumeration % "parsing" % "type-checking" % "alphabet_reduction" % "data_implementation";
}

void squadt_interactor::set_capabilities(tipi::tool::capabilities& c) const {
  c.add_input_configuration(mcrl2_file_for_input, tipi::mime_type("mcrl2", tipi::mime_type::text),
                                                            tipi::tool::category::transformation);
}

void squadt_interactor::user_interactive_configuration(tipi::configuration& c) {
  using namespace tipi;
  using namespace tipi::layout;
  using namespace tipi::datatype;
  using namespace tipi::layout::elements;

  std::string infilename = c.get_input(mcrl2_file_for_input).get_location();

  // Set defaults for options
  if (!c.option_exists(option_final_cluster)) {
    c.add_option(option_final_cluster).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_no_intermediate_cluster)) {
    c.add_option(option_no_intermediate_cluster).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_no_alpha)) {
    c.add_option(option_no_alpha).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_newstate)) {
    c.add_option(option_newstate).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_binary)) {
    c.add_option(option_binary).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_statenames)) {
    c.add_option(option_statenames).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_no_rewrite)) {
    c.add_option(option_no_rewrite).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_no_freevars)) {
    c.add_option(option_no_freevars).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_no_sumelm)) {
    c.add_option(option_no_sumelm).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_no_deltaelm)) {
    c.add_option(option_no_deltaelm).set_argument_value< 0, tipi::datatype::boolean >(false);
  }
  if (!c.option_exists(option_add_delta)) {
    c.add_option(option_add_delta).set_argument_value< 0, tipi::datatype::boolean >(false);
  }

  /* Create display */
  tipi::layout::tool_display d;

  // Helper for linearisation method selection
  mcrl2::utilities::squadt::radio_button_helper < t_lin_method > method_selector(d);

  // Helper for end phase selection
  mcrl2::utilities::squadt::radio_button_helper < t_phase >      phase_selector(d); 

  layout::vertical_box& m = d.create< vertical_box >().set_default_margins(margins(0,5,0,5));

  m.append(d.create< label >().set_text("Method: ")).
    append(d.create< horizontal_box >().
        append(method_selector.associate(lmStack, "Stack")).
        append(method_selector.associate(lmRegular, "Regular", true)).
        append(method_selector.associate(lmRegular2, "Regular2")));

  checkbox& clusterintermediate = d.create< checkbox >().set_status(!c.get_option_argument< bool >(option_no_intermediate_cluster));
  checkbox& clusterfinal        = d.create< checkbox >().set_status(c.get_option_argument< bool >(option_final_cluster));
  checkbox& newstate            = d.create< checkbox >().set_status(c.get_option_argument< bool >(option_newstate));
  checkbox& binary              = d.create< checkbox >().set_status(c.get_option_argument< bool >(option_binary));
  checkbox& statenames          = d.create< checkbox >().set_status(c.get_option_argument< bool >(option_statenames));
  checkbox& add_delta           = d.create< checkbox >().set_status(c.get_option_argument< bool >(option_add_delta));
  checkbox& rewrite             = d.create< checkbox >().set_status(!c.get_option_argument< bool >(option_no_rewrite));
  checkbox& alpha               = d.create< checkbox >().set_status(!c.get_option_argument< bool >(option_no_alpha));
  checkbox& sumelm              = d.create< checkbox >().set_status(!c.get_option_argument< bool >(option_no_sumelm));
  checkbox& deltaelm            = d.create< checkbox >().set_status(!c.get_option_argument< bool >(option_no_deltaelm));
  checkbox& freevars            = d.create< checkbox >().set_status(!c.get_option_argument< bool >(option_no_freevars));

  // two columns to select the linearisation options of the tool
  m.append(d.create< label >().set_text(" ")).
    append(d.create< horizontal_box >().
        append(d.create< vertical_box >().set_default_alignment(layout::right).
            append(clusterintermediate.set_label("Intermediate clustering")).
            append(clusterfinal.set_label("Final clustering")).
            append(newstate.set_label("Use enumerations for state variables")).
            append(binary.set_label("Encode enumerations by booleans")).
            append(statenames.set_label("Derive state names from specification")).
            append(add_delta.set_label("Add delta summands"))).
        append(d.create< vertical_box >().set_default_alignment(layout::left).
            append(rewrite.set_label("Use rewriting")).
            append(alpha.set_label("Apply alphabet axioms")).
            append(sumelm.set_label("Apply sum elimination")).
            append(deltaelm.set_label("Apply delta elimination")).
            append(freevars.set_label("Generate free variables"))));

  // Determine which phases the linearizer will go through. Default is all.
  m.append(d.create< label >().set_text(" ")).
    append(d.create< label >().set_text("Stop after: ")).
    append(d.create< vertical_box >().
        append(phase_selector.associate(phParse, "Parsing")).
        append(phase_selector.associate(phTypeCheck, "Type checking")).
        append(phase_selector.associate(phDataImpl, "Data implementation")).
        append(phase_selector.associate(phAlphaRed, "Alphabet reduction")).
        append(phase_selector.associate(phNone, "Linearisation", true)));

  // Add okay button
  button& okay_button = d.create< button >().set_label("OK");

  m.append(d.create< label >().set_text(" ")).
    append(okay_button, layout::right);

  // Set default values for options if the configuration specifies them
  if (c.option_exists(option_end_phase)) {
    phase_selector.set_selection(static_cast < t_phase > (
        c.get_option_argument< size_t >(option_end_phase, 0)));
  }
  if (c.option_exists(option_linearisation_method)) {
    method_selector.set_selection(static_cast < t_lin_method > (
        c.get_option_argument< size_t >(option_linearisation_method, 0)));
  }

  send_display_layout(d.set_manager(m));

  /* Wait for the OK button to be pressed */
  okay_button.await_change();

  // Update configuration
  if (!c.output_exists(lps_file_for_output)) {
    c.add_output(lps_file_for_output, tipi::mime_type("lps", tipi::mime_type::application), c.get_output_name(".lps"));
  }

  c.add_option(option_linearisation_method).
          append_argument(linearisation_method_enumeration, method_selector.get_selection());

  c.add_option(option_end_phase).
          append_argument(linearisation_phase_enumeration, phase_selector.get_selection());

  if (phase_selector.get_selection() != phNone) { // file will not be produced
    c.remove_output(lps_file_for_output);
  }

  using tipi::datatype::boolean;

  c.get_option(option_final_cluster).set_argument_value< 0, boolean >(clusterfinal.get_status());
  c.get_option(option_no_intermediate_cluster).set_argument_value< 0, boolean >(!clusterintermediate.get_status());
  c.get_option(option_no_alpha).set_argument_value< 0, boolean >(!alpha.get_status());
  c.get_option(option_newstate).set_argument_value< 0, boolean >(newstate.get_status());
  c.get_option(option_binary).set_argument_value< 0, boolean >(binary.get_status());
  c.get_option(option_statenames).set_argument_value< 0, boolean >(statenames.get_status());
  c.get_option(option_no_rewrite).set_argument_value< 0, boolean >(!rewrite.get_status());
  c.get_option(option_no_freevars).set_argument_value< 0, boolean >(!freevars.get_status());
  c.get_option(option_no_sumelm).set_argument_value< 0, boolean >(!sumelm.get_status());
  c.get_option(option_no_deltaelm).set_argument_value< 0, boolean >(!deltaelm.get_status());
  c.get_option(option_add_delta).set_argument_value< 0, boolean >(add_delta.get_status());

  send_clear_display();
}

bool squadt_interactor::check_configuration(tipi::configuration const& c) const {
  bool result = true;

  result |= c.input_exists(mcrl2_file_for_input);
  result |= c.output_exists(lps_file_for_output);
  result |= c.option_exists(option_linearisation_method);

  return (result);
}

bool squadt_interactor::extract_task_options(tipi::configuration const& c, t_lin_options& task_options) const {
  bool result = true;

  if (c.input_exists(mcrl2_file_for_input)) {
    task_options.infilename = c.get_input(mcrl2_file_for_input).get_location();
  }
  else {
    send_error("Configuration does not contain an input object\n");

    result = false;
  }

  if (c.output_exists(lps_file_for_output) ) {
    task_options.outfilename = c.get_output(lps_file_for_output).get_location();
  }
  else {
    send_error("Configuration does not contain an output object\n");

    result = false;
  }

  if (c.option_exists(option_linearisation_method)) {
    task_options.lin_method = static_cast < t_lin_method > (boost::any_cast < size_t > (c.get_option_argument(option_linearisation_method, 0)));
  }
  else {
    send_error("Configuration does not contain a linearisation method\n");

    result = false;
  } 

  task_options.final_cluster           = c.get_option_argument< bool >(option_final_cluster);
  task_options.no_intermediate_cluster = c.get_option_argument< bool >(option_no_intermediate_cluster);
  task_options.noalpha             = c.get_option_argument< bool >(option_no_alpha);
  task_options.newstate                = c.get_option_argument< bool >(option_newstate);
  task_options.binary                  = c.get_option_argument< bool >(option_binary);
  task_options.statenames              = c.get_option_argument< bool >(option_statenames);
  task_options.norewrite               = c.get_option_argument< bool >(option_no_rewrite);
  task_options.nofreevars              = c.get_option_argument< bool >(option_no_freevars);
  task_options.nosumelm                = c.get_option_argument< bool >(option_no_sumelm);
  task_options.nodeltaelimination      = c.get_option_argument< bool >(option_no_deltaelm);
  task_options.add_delta               = c.get_option_argument< bool >(option_add_delta);
  
  task_options.end_phase = static_cast < t_phase > (boost::any_cast < size_t > (c.get_option_argument(option_end_phase, 0)));

  task_options.check_only = (task_options.end_phase != phNone);

  return (result);
}

bool squadt_interactor::perform_task(tipi::configuration& c) {
  using namespace boost;
  using namespace tipi;
  using namespace tipi::layout;
  using namespace tipi::datatype;
  using namespace tipi::layout::elements;

  bool result = true;

  t_lin_options task_options;
  
  // Extract configuration
  extract_task_options(c, task_options);

  /* Create display */
  tipi::layout::tool_display d;

  label& message = d.create< label >();
 
  d.set_manager(d.create< vertical_box >().
                        append(message.set_text("Linearisation in progress"), layout::left));

  send_display_layout(d);

  // Perform linearisation
  ATermAppl linearisation_result = linearise_file(task_options);

  if (linearisation_result == 0) {
    message.set_text("Linearisation in failed");

    result = false;
  }
  else if (task_options.check_only) {
    message.set_text(str(format("%s contains a well-formed mCRL2 specification.") % task_options.infilename));
  }
  else {
    //store the result
    FILE *outstream = fopen(task_options.outfilename.c_str(), "wb");

    if (outstream != 0) {
      ATwriteToSAFFile((ATerm) linearisation_result, outstream);

      fclose(outstream);
    }
    else {
      send_error(str(format("cannot open output file '%s'\n") % task_options.outfilename));

      result = false;
    }

    if (result) {
      message.set_text("Linearisation finished");
    }
  }

  send_display_layout(d);

  return (result);
}

#endif

using namespace std;

static t_lin_options parse_command_line(int argc, char *argv[])
{ 
  //declarations for getopt
  t_lin_options lin_options;
  bool lm_chosen = false;
  #define ShortOptions   "0123cnrwbaofep:hqvdmgD"
  #define VersionOption  0x1
  struct option LongOptions[] = {
    { "stack",       no_argument,       NULL, '0' },
    { "regular",     no_argument,       NULL, '1' },
    { "regular2",    no_argument,       NULL, '2' },
    { "cluster",     no_argument,       NULL, 'c' },
    { "no-cluster",  no_argument,       NULL, 'n' },
    { "no-alpha",    no_argument,       NULL, 'r' },
    { "newstate",    no_argument,       NULL, 'w' },
    { "binary",      no_argument,       NULL, 'b' },
    { "statenames",  no_argument,       NULL, 'a' },
    { "no-rewrite",  no_argument,       NULL, 'o' },
    { "no-freevars", no_argument,       NULL, 'f' },
    { "no-sumelm",   no_argument,       NULL, 'm' },
    { "no-deltaelm", no_argument,       NULL, 'g' },
    { "check-only",  no_argument,       NULL, 'e' },
    { "end-phase",   required_argument, NULL, 'p' },
    { "help",        no_argument,       NULL, 'h' },
    { "version",     no_argument,       NULL, VersionOption },
    { "quiet",       no_argument,       NULL, 'q' },
    { "verbose",     no_argument,       NULL, 'v' },
    { "debug",       no_argument,       NULL, 'd' },
    { "delta",       no_argument,       NULL, 'D' },
    { 0, 0, 0, 0 }
  };
  int Option;
  //parse options
  Option = getopt_long(argc, argv, ShortOptions, LongOptions, NULL);
  while (Option != -1) {
    switch (Option){
      case '0': /* stack */
        if (lm_chosen && lin_options.lin_method != lmStack) {
          gsErrorMsg("only one method of linearisation is allowed\n");
          exit(EXIT_FAILURE);
        }
        lm_chosen = true;
        lin_options.lin_method = lmStack;
        break;
      case '1': /* regular */
        if (lm_chosen && lin_options.lin_method != lmRegular) {
          gsErrorMsg("only one method of linearisation is allowed\n");
          exit(EXIT_FAILURE);
        }
        lm_chosen = true;
        lin_options.lin_method = lmRegular;
        break;
      case '2': /* regular2 */
        if (lm_chosen && lin_options.lin_method != lmRegular2) {
          gsErrorMsg("only one method of linearisation is allowed\n");
          exit(EXIT_FAILURE);
        }
        lm_chosen = true;
        lin_options.lin_method = lmRegular2;
        break;
      case 'c': /* cluster */ 
        lin_options.final_cluster = true;
        break;
      case 'n': /* no-cluster */
        lin_options.no_intermediate_cluster = true;
        break;
      case 'r': /* no-alpha */
        lin_options.noalpha = true;
        break;
      case 'w': /* newstate */ 
        lin_options.newstate = true;
        break;
      case 'b': /* binary */ 
        lin_options.binary = true;
        break;
      case 'a': /* statenames */ 
        lin_options.statenames = true;
        break;
      case 'o': /* no-rewrite */ 
        lin_options.norewrite = true;
        break;
      case 'f': /* nofreevars */
        lin_options.nofreevars = true;
        break;
      case 'e': /* check-only */
        lin_options.check_only = true;
        lin_options.end_phase = phTypeCheck;
        break;
      case 'm': /* no-sumelm */
        lin_options.nosumelm = true;
        break;
      case 'g': /* no-deltaelm */
        lin_options.nodeltaelimination = true;
        break;
      case 'D': /* delta */
        lin_options.add_delta = true;
        break;
      case 'p': /* end-phase */
        if (strcmp(optarg, "pa") == 0) {
          lin_options.end_phase = phParse;
        } else if (strcmp(optarg, "tc") == 0) {
          lin_options.end_phase = phTypeCheck;
        } else if (strcmp(optarg, "ar") == 0) {
          lin_options.end_phase = phAlphaRed;
        } else if (strcmp(optarg, "di") == 0) {
          lin_options.end_phase = phDataImpl;
        } else {
          gsErrorMsg("option -p has illegal argument '%s'\n", optarg);
          exit(EXIT_FAILURE);
        }
        break;
      case 'h': /* help */
        PrintHelp(argv[0]);
        exit(EXIT_SUCCESS);
      case VersionOption: /* version */
        print_version_information(NAME, AUTHOR);
        exit(EXIT_SUCCESS);
      case 'q': /* quiet */
        gsSetQuietMsg();
        break;
      case 'v': /* verbose */
        gsSetVerboseMsg();
        break;
      case 'd': /* debug */
        gsSetDebugMsg();
        break;
      case '?':
      default:
        PrintMoreInfo(argv[0]); 
        exit(EXIT_FAILURE);
    } 
    Option = getopt_long(argc, argv, ShortOptions, LongOptions, NULL);
  }
  //check for dangerous and illegal option combinations
  if (lin_options.newstate && lin_options.lin_method == lmStack) {
    gsErrorMsg("option -w can only be used with -1 or -2\n");
    exit(EXIT_FAILURE);
  }
  if (lin_options.check_only && (lin_options.end_phase != phTypeCheck)) {
    gsErrorMsg("options -e and -p may not be used in conjunction\n");
    exit(EXIT_FAILURE);
  }
  if (lin_options.noalpha && (lin_options.end_phase == phAlphaRed)) {
    gsErrorMsg("options -r and -p ar may not be used in conjunction\n");
    exit(EXIT_FAILURE);
  }
  int noargc; //non-option argument count
  noargc = argc - optind;
  if (noargc > 2) {
    fprintf(stderr, "%s: too many arguments\n", NAME);
    PrintMoreInfo(argv[0]);
    exit(EXIT_FAILURE);
  } else {
    //noargc >= 0 && noargc <= 2
    if (noargc > 0) {
      lin_options.infilename = argv[optind];
    }
    if (noargc == 2) {
      lin_options.outfilename = argv[optind + 1];
    }
  }
  return lin_options;
}

ATermAppl linearise_file(t_lin_options &lin_options)
{
  ATermAppl result = NULL;
  //parse specification
  if (lin_options.infilename == "") {
    //parse specification from stdin
    gsVerboseMsg("parsing input from stdin...\n");
    result = parse_proc_spec(cin);
  } else {
    //parse specification from infilename
    ifstream instream(lin_options.infilename.c_str(), ifstream::in|ifstream::binary);
    if (!instream.is_open()) {
      gsErrorMsg("cannot open input file '%s'\n", lin_options.infilename.c_str());
      return NULL;
    }
    gsVerboseMsg("parsing input file '%s'...\n", lin_options.infilename.c_str());
    result = parse_proc_spec(instream);
    instream.close();
  }
  if (result == NULL) 
  {
    gsErrorMsg("parsing failed\n");
    return NULL;
  }

  if (lin_options.end_phase == phParse) {
    return result;
  }
  //type check the result
  gsVerboseMsg("type checking...\n");
  result = type_check_proc_spec(result);
  if (result == NULL) 
  {
    gsErrorMsg("type checking failed\n");
    return NULL;
  }
  if (lin_options.end_phase == phTypeCheck) {
    return result;
  }
  //perform alphabet reductions 
  if (!lin_options.noalpha) {
    gsVerboseMsg("performing alphabet reductions...\n");
    result = gsAlpha(result);
    if (result == NULL)
    {
      gsErrorMsg("alphabet reductions failed\n");
      return NULL;
    }
    if (lin_options.end_phase == phAlphaRed) {
      return result;
    }
  }
  //implement standard data types and type constructors on the result
  gsVerboseMsg("implementing standard data types and type constructors...\n");
  result = implement_data_proc_spec(result);
  if (result == NULL) 
  {
    gsErrorMsg("data implementation failed\n");
    return NULL;
  }
  if (lin_options.end_phase == phDataImpl) {
    return result;
  }
  //linearise the result
  gsVerboseMsg("linearising processes using the %s method\n", lin_method_to_string(lin_options.lin_method));

  result = linearise_std(result, lin_options);
  if (result == NULL) 
  {
    gsErrorMsg("linearisation failed\n");
    return NULL;
  }
  return result; 
}

inline char const* lin_method_to_string(t_lin_method lin_method)
{
  static const char* method[] = {"stack","regular","regular2"};

  return (method[lin_method]);
}

void PrintMoreInfo(char *Name)
{
  fprintf(stderr, "Try `%s --help' for more information.\n", Name);
}

void PrintHelp(char *Name)
{
  fprintf(stdout,
    "Usage: %s [OPTION]... [INFILE [OUTFILE]]\n"
    "Linearises the mCRL2 specification in INFILE and writes the resulting LPS to\n"
    "OUTFILE. If OUTFILE is not present, stdout is used. If INFILE is not present,\n"
    "stdin is used.\n"
    "\n"
    "Options:\n"
    "  -0, --stack           the LPS is generated using stack datatypes;\n"
    "                        useful when -1 and -2 do not work\n"
    "  -1, --regular         if the specification is regular, the LPS is generated\n"
    "                        in regular form (default)\n"
    "  -2, --regular2        a variant of regular that uses more data variables;\n"
    "                        sometimes successful when -1 leads to non-termination\n"
    "  -c, --cluster         all actions in the final LPS are clustered\n"
    "  -n, --no-cluster      the actions in intermediate LPSs are not clustered\n"
    "                        (default behaviour is that intermediate LPSs are\n"
    "                        clustered and the final LPS is not clustered)\n"
    "  -r, --no-alpha        alphabet reductions are not applied\n"
    "  -w, --newstate        state variables are encoded using enumerated types\n"
    "                        (requires -1 or -2); without -w numbers are used\n"
    "  -b, --binary          when clustering use binary case functions instead of\n"
    "                        n-ary; in the presence of -w, state variables are\n"
    "                        encoded by a vector of boolean variables\n"
    "  -a, --statenames      the names of state variables are derived from the\n"
    "                        specification\n"
    "  -o, --no-rewrite      do not rewrite data terms while linearising;\n"
    "                        useful when the rewrite system does not terminate\n"
    "  -f, --no-freevars     instantiate don't care values with arbitrary constants,\n"
    "                        instead of modelling them by free variables\n"
    "  -m, --no-sumelm       avoid applying sum elimination in parallel composition\n"
    "  -g, --no-deltaelm     avoid removing spurious delta summands\n"
    "  -D, --delta           add a true->delta summands to each state in each process;\n"
    "                        these delta's subsume all other conditional timed delta's,\n"
    "                        effectively reducing the number of delta summands drastically\n"
    "                        in the resulting linear process; speeds up linearisation\n"
    "  -e, --check-only      check syntax and static semantics; do not linearise\n"
    "  -pPHASE, --end-phase=PHASE\n"
    "                        stop linearisation after phase PHASE and output the\n"
    "                        result; PHASE can be 'pa' (parse), 'tc' (type check),\n"
    "                        'ar' (alphabet reduction) or 'di' (data implementation)\n"
    "  -h, --help            display this help and terminate\n"
    "      --version         display version information and terminate\n"
    "  -q, --quiet           do not display warning messages\n"
    "  -v, --verbose         display concise intermediate messages\n"
    "  -d, --debug           display detailed intermediate messages\n"
    "\n"
    "Report bugs at <http://www.mcrl2.org/issuetracker>.\n"
    , Name);
}

// Main 

int main(int argc, char *argv[])
{
  MCRL2_ATERM_INIT(argc, argv)

#ifdef ENABLE_SQUADT_CONNECTIVITY
  if (!mcrl2::utilities::squadt::interactor< squadt_interactor >::free_activation(argc, argv)) {
#endif
    t_lin_options lin_options = parse_command_line(argc, argv);
    //linearise infilename with options lin_options
    ATermAppl result = linearise_file(lin_options);
    if (result == NULL) {
      return EXIT_FAILURE;
    }
    //initialise spec to check if it well-typed
    if (lin_options.end_phase == phNone) {
      lps::specification spec(result);
    }
    if (lin_options.check_only) {
      if (lin_options.infilename == "") {
        fprintf(stdout, "stdin");
      } else {
        fprintf(stdout, "The file '%s'", lin_options.infilename.c_str());
      }
      fprintf(stdout, " contains a well-formed mCRL2 specification.\n");
      return EXIT_SUCCESS;
    } else {
      //store the result
      if (lin_options.outfilename == "") {
        gsVerboseMsg("saving result to stdout...\n");
        ATwriteToSAFFile((ATerm) result, stdout);
      } else { //outfilename != NULL
        //open output filename
        FILE *outstream = fopen(lin_options.outfilename.c_str(), "wb");
        if (outstream == NULL) {
          gsErrorMsg("cannot open output file '%s'\n", lin_options.outfilename.c_str());
          return EXIT_FAILURE;
        }
        gsVerboseMsg("saving result to '%s'...\n", lin_options.outfilename.c_str());
        ATwriteToSAFFile((ATerm) result, outstream);
        fclose(outstream);
      }
    }
#ifdef ENABLE_SQUADT_CONNECTIVITY
  }
#endif

  return EXIT_SUCCESS;
}
