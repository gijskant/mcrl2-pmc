// ======================================================================
//
// Copyright (c) 2006 TU/e
//
// ----------------------------------------------------------------------
//
// file          : lpesumelm 
// date          : 31-10-2006
// version       : 0.3
//
// author(s)     : Jeroen Keiren <j.j.a.keiren@student.tue.nl>
//
// ======================================================================

//C++
#include <exception>
#include <cstdio>

//Boost
#include <boost/program_options.hpp>

//Lowlevel library for gsErrorMsg
#include <libprint_c.h>

//Aterms
#include <atermpp/aterm.h>
#include <atermpp/aterm_list.h>
#include <atermpp/algorithm.h>

//LPE Framework
#include <lpe/lpe.h>
#include <lpe/specification.h>
#include <lpe/data_functional.h>
#include <lpe/data_init.h>

//Squadt connectivity
#ifdef ENABLE_SQUADT_CONNECTIVITY
#include <squadt_utility.h>
#endif

using namespace std;
using namespace atermpp;
using namespace lpe;
using namespace lpe::data_init;

namespace po = boost::program_options;

#define VERSION "0.3"

std::string input_file; ///< Name of the file to read input from
std::string output_file; ///< Name of the file to write output to (or stdout)

#ifdef ENABLE_SQUADT_CONNECTIVITY
//Forward declaration because do_sumelm() is called within squadt_lpesumelm class
int do_sumelm(const std::string input_file_name, const std::string output_file_name);

class squadt_lpesumelm: public squadt_tool_interface
{
  private:
    enum input_files {
      lpd_file_for_input, ///< file containing an lpd that can be imported
      lpd_file_for_output ///< file used to write output to
    };

  public:
    /** \brief configures tool capabilities */
    void set_capabilities(sip::tool::capabilities&) const;

    /** \brief queries the user via SQuADT if needed to obtain configuration information */
    void user_interactive_configuration(sip::configuration&);

    /** \brief check an existing configuration object to see if it is usable */
    bool check_configuration(sip::configuration const&) const;

    /** \brief performs the task specified by a configuration */
    bool perform_task(sip::configuration&);
};

void squadt_lpesumelm::set_capabilities(sip::tool::capabilities& capabilities) const
{
  // The tool has only one main input combination
  gsDebugMsg("squadt_lpesumelm: Setting capabilities\n");
  capabilities.add_input_combination(lpd_file_for_input, sip::mime_type("lpe"), sip::tool::category::transformation);
}

void squadt_lpesumelm::user_interactive_configuration(sip::configuration& configuration)
{
  gsDebugMsg("squadt_lpesumelm: User interactive configuration\n");
  configuration.add_output(lpd_file_for_output, sip::mime_type("lpe"), configuration.get_output_name(".lpe"));
}

bool squadt_lpesumelm::check_configuration(sip::configuration const& configuration) const
{
  gsDebugMsg("squadt_lpesumelm: Checking configuration\n");
// Check if everything present (see lpe2lts)
  return (configuration.object_exists(lpd_file_for_input) &&
          configuration.object_exists(lpd_file_for_output)
         );
}

bool squadt_lpesumelm::perform_task(sip::configuration& configuration)
{
  gsDebugMsg("squadt_lpesumelm: Performing task\n");
  std::string in_file, out_file;
  in_file = configuration.get_object(lpd_file_for_input)->get_location();
  out_file = configuration.get_object(lpd_file_for_output)->get_location();

  gsDebugMsg("Calling do_sumelm through SQuADT, with input: %s and output: %s\n", in_file.c_str(), out_file.c_str());
  return (do_sumelm(in_file, out_file)==0);
}

#endif //ENABLE_SQUADT_CONNECTIVITY

////////////////////////////////////////////////////////////////
// Auxiliary functions used to support untime operation
// Some of these might be interesting to get into the LPE library

///Used to assist in occurs_in function.
struct compare_data_variable
{
  aterm v;

  compare_data_variable(data_variable v_)
    : v(aterm_appl(v_))
  {}
  
  bool operator()(aterm t) const
  {
    return v == t;
  }
};

///pre: true
///ret: data_variable v occurs in d.
template <typename data_type>
bool occurs_in(data_type d, data_variable v)
{
  return find_if(aterm_appl(d), compare_data_variable(v)) != aterm();
}

///pre: is_var(t)
///ret: The data_variable embedded in t
inline
data_variable get_var(data_expression t)
{
  assert(is_data_variable(t));
  data_variable result = data_variable(ATermAppl(t));
  return result;
}


///pre: is_and(t) || is_equal_to(t)
///ret: lefthandside of t
inline
data_expression lhs(data_expression t)
{
  assert(is_and(t) || is_equal_to(t));
  return data_expression(ATAgetArgument(ATAgetArgument(t, 0), 1));  
}

///pre: is_and(t) || is_equal_to(t)
///ret: righthandside of t
inline
data_expression rhs(data_expression t)
{
  assert(is_and(t) || is_equal_to(t));
  return data_expression(ATAgetArgument(t, 1));
}

///pre: is_equal_to(t); t is of form a == b
///ret: b == a
inline
data_expression swap_equality(data_expression t)
{
  assert(is_equal_to(t));
  return lpe::equal_to(rhs(t), lhs(t));
}

///Apply substitution to the righthand sides of the assignments in dl
data_assignment_list substitute_rhs(const data_assignment_list& dl, const data_assignment& substitution)
{
  data_assignment_list result;

  for(data_assignment_list::iterator i = dl.begin(); i != dl.end(); ++i)
  {
    data_expression rhs = i->rhs();
    rhs = rhs.substitute(substitution);
    result = push_front(result, data_assignment(i->lhs(), rhs));
  }

  return result;
}

////////////////////////////////////////////////////////////
// Functions for sumelm operations
//

///Apply a simple form of sum elimination in the case a summation
///variable does not occur within the summand at all
lpe::LPE_summand remove_unused_variables(const lpe::LPE_summand& summand)
{
  lpe::LPE_summand new_summand;
  // New summation variable list, all variables in this list occur in other terms in the summand.
  lpe::data_variable_list new_summation_variables;

  for(lpe::data_variable_list::iterator i = summand.summation_variables().begin(); i != summand.summation_variables().end(); ++i)
  {
    data_variable v = *i;
   
    if (!summand.is_delta())
    { 
      //Check whether variable occurs in other terms of summand
      //If variable occurs leave the variable, so add variable to new list
      if (occurs_in(summand.condition(), v) || occurs_in(summand.actions(), v)
          || occurs_in(summand.time(), v) || occurs_in(summand.assignments(), v))
      {
        new_summation_variables = push_front(new_summation_variables, v);
      }
      //else remove the variable, i.e. do not add it to the new list (skip)
    }
  }

  new_summation_variables = reverse(new_summation_variables);

  new_summand = set_summation_variables(summand, new_summation_variables);

  return new_summand;
}

///Take a specification and apply sum elimination to its summands
lpe::specification remove_unused_variables_(const lpe::specification& specification)
{
  lpe::LPE lpe = specification.lpe();
  lpe::specification new_specification;
  lpe::summand_list new_summand_list = lpe.summands();

  // Traverse the summand list, and apply sum elimination to its summands,
  // whilst constructing a new summand list in the process.
  new_summand_list = atermpp::apply(new_summand_list, remove_unused_variables);

  new_specification = set_lpe(specification, set_summands(lpe, new_summand_list));

  return new_specification;
}

//Recursively apply sum elimination on a summand. 
//We build up a list of substitutions that need to be made in substitutions
//the caller of this function needs to apply sumstitutions to the summand
//once we exit recursion
//working_condition is a parameter that we use to split up the problem,
//at the first call of this function working_condition == summand->condition()
//should hold.
//The new condition is built up on the return path of the recursion, so
//the last exit of the recursion is the new condition of the summand.
//
//Note that filtering the summation variables should (for now) be done in the calling
//function, by applying remove_unused_variables_ on the result, because that is a little
//more efficient.
//!!!INTERNAL USE ONLY!!!
data_expression recursive_substitute_equalities(const LPE_summand& summand,
                                                data_expression working_condition,
                                                data_assignment_list& substitutions)
{
  // In all cases not explicitly handled we return the original working_condition
  data_expression result = working_condition;

  if (is_and(working_condition))
  {
    //Recursively apply sum elimination on lhs and rhs
    //Note that recursive application provides for progress because lhs and rhs split the working condition.
    data_expression a,b;
    a = recursive_substitute_equalities(summand, lhs(working_condition), substitutions);
    b = recursive_substitute_equalities(summand, rhs(working_condition), substitutions);
    result = and_(a,b);
  }
  
  else if (is_equal_to(working_condition))
  {
    //Check if rhs is a variable, if so, swap lhs and rhs, so that the following code
    //is always the same.
    if (!is_data_variable(lhs(working_condition)) && is_data_variable(rhs(working_condition)))
    {
      working_condition = swap_equality(working_condition);
    }

    //If lhs is a variable, check if it occurs in the summation variables, if so
    //apply substitution lhs := rhs in actions, time and assignments.
    //substitution in condition is accounted for on returnpath of recursion,
    //substitution in summation_variables is done in calling function.
    if (is_data_variable(lhs(working_condition)))
    {
      data_variable_list lhs_subst;
      for (data_assignment_list::iterator i = substitutions.begin(); i != substitutions.end(); ++i)
      {
        lhs_subst = push_front(lhs_subst, i->lhs());
      }

      //According to sum elimination lemma the variable that is being substituted can not occur in its replacement.
      if (!occurs_in(lhs_subst, get_var(lhs(working_condition))) && occurs_in(summand.summation_variables(), get_var(lhs(working_condition))) && !occurs_in(rhs(working_condition), get_var(lhs(working_condition))))
      {
        data_assignment substitution = data_assignment(get_var(lhs(working_condition)), rhs(working_condition));
 
        // First apply substitution to righthandside of other substitutions,
        // then add new substitution.
        substitutions = substitute_rhs(substitutions, substitution);
        substitutions = push_front(substitutions, substitution);
        result = true_();
      }
    }

  }
  
  return result;
}

///Apply sum elimination on the summand consisting of
///summation_variables, condition, actions, time and assignments.
///This checks the following:
///X(..) = sum d. d=e -> a(..) . X(..)
///and returns X(..) = e -> a(..) . X(..)
lpe::LPE_summand substitute_equalities(const lpe::LPE_summand& summand)
{
  lpe::LPE_summand new_summand = summand;

  //Apply elimination and store result
  lpe::data_assignment_list substitutions;
  lpe::data_expression new_condition = recursive_substitute_equalities(new_summand, new_summand.condition(), substitutions);

  //Apply the substitutions that were returned from the recursive call
  new_condition = new_condition.substitute(assignment_list_substitution(substitutions));
  new_summand = LPE_summand(new_summand.summation_variables(),
                            new_condition.substitute(assignment_list_substitution(substitutions)),
                            new_summand.is_delta(),
                            new_summand.actions().substitute(assignment_list_substitution(substitutions)),
                            new_summand.time().substitute(assignment_list_substitution(substitutions)),
                            new_summand.assignments().substitute(assignment_list_substitution(substitutions)));
  //Take the summand with substitution, and remove the summation variables that are now not needed
  new_summand = remove_unused_variables(new_summand);
  return new_summand;
}


///Take an lpe specification, apply equality sum elimination to it,
///and return an lpe specification
lpe::specification substitute_equalities_(const lpe::specification& specification)
{
  lpe::LPE lpe = specification.lpe();
  lpe::specification new_specification;
  lpe::summand_list new_summand_list = lpe.summands();

  // Apply sum elimination on each of the summands in the summand list.
  new_summand_list = atermpp::apply(new_summand_list, substitute_equalities);
  new_specification = set_lpe(specification, set_summands(lpe, new_summand_list));
  return new_specification;
}

///Returns an LPE specification in which the timed arguments have been rewritten
lpe::specification sumelm(const lpe::specification& specification) 
{
  gsVerboseMsg("Applying sum elimination...\n");

  lpe::specification new_specification = specification;
  new_specification = substitute_equalities_(new_specification); // new_specification used for future concerns, possibly disabling substitute_equalities_
  new_specification = remove_unused_variables_(new_specification);
  return new_specification;
}

///Reads a specification from input_file, 
///applies sum elimination to it and writes the result to output_file.
int do_sumelm(const std::string input_file_name, const std::string output_file_name)
{
  lpe::specification lpe_specification;
  if (lpe_specification.load(input_file_name)) {
    // Untime lpe_specification and save the output to a binary file
    lpe::specification new_spec = sumelm(lpe_specification);

    gsDebugMsg("Sum elimination completed, saving to %s\n", output_file_name.c_str());
    bool success = new_spec.save(output_file_name, true);

    if (!success) 
    {
      // An error occurred when saving
      gsErrorMsg("Could not save to '%s'\n", output_file_name.c_str());
      return (1);
    }
  }
  else {
    gsErrorMsg("lpesumelm: Unable to load LPE from `%s'\n", input_file_name.c_str());
    return (1);
  }

  return 0;
}

///Parses command line and sets settings from command line switches
void parse_command_line(int ac, char** av) {
  po::options_description desc;

  desc.add_options()
      ("help,h",      "display this help")
      ("verbose,v",   "turn on the display of short intermediate messages")
      ("debug,d",    "turn on the display of detailed intermediate messages")
      ("version",     "display version information")
  ;
  po::options_description hidden("Hidden options");
  hidden.add_options()
      ("INFILE", po::value< string >(), "input file")
      ("OUTFILE", po::value< string >(), "output file")
  ;
  po::options_description cmdline_options;
  cmdline_options.add(desc).add(hidden);
      
  po::options_description visible("Allowed options");
  visible.add(desc);

  po::positional_options_description p;
  p.add("INFILE", 1);
  p.add("OUTFILE", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(ac, av).
    options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);
  
  if (vm.count("help")) {
    cerr << "Usage: "<< av[0] << " [OPTION]... [INFILE] [OUTFILE]" << endl;
    cerr << "Apply sum elimination on the LPE in INFILE and store the result to OUTFILE" << endl;

    cerr << endl;
    cerr << desc;

    exit (0);
  }
      
  if (vm.count("version")) {
    cerr << "lpesumelm " << VERSION << " (revision " << REVISION << ")" << endl;

    exit (0);
  }

  if (vm.count("debug")) {
    gsSetDebugMsg();
  }

  if (vm.count("verbose")) {
    gsSetVerboseMsg();
  }

  input_file = (0 < vm.count("INFILE")) ? vm["INFILE"].as< string >() : "-";
  output_file = (0 < vm.count("OUTFILE")) ? vm["OUTFILE"].as< string >() : "-";
}

int main(int ac, char** av) {
  ATerm bot;
  ATinit(ac, av, &bot);
  gsEnableConstructorFunctions();

#ifdef ENABLE_SQUADT_CONNECTIVITY
  gsDebugMsg("Squadt connectivity enabled\n");
  squadt_lpesumelm sl;
  if (sl.try_interaction(ac, av)) {
    return 0;
  }
#endif

  parse_command_line(ac,av);
  return do_sumelm(input_file, output_file);
}
