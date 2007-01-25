// ======================================================================
//
// Copyright (c) 2004, 2005 TU/e
//
// ----------------------------------------------------------------------
//
// file          : lpeparelm 
// date          : 15-11-2005
// version       : 0.5
//
// author(s)     : Frank Stappers  <f.p.m.stappers@student.tue.nl>
//
// ======================================================================

#if BOOST_BUILD_PCH_ENABLED
# ifdef ENABLE_SQUADT_CONNECTIVITY
#  include <utility/squadt_utility.h>
#  include "lpe/specification.h"
# else
#  include "specification.h"
# endif
#else
# include "lpe/specification.h"
#endif

//C++
#include <cstdio>

//Boost
#include <boost/program_options.hpp>

//Lowlevel library for messages
#include <libprint_c.h>

//mCRL2
#include "atermpp/aterm.h"

class lpeParElm {

  private:

    std::string                         p_inputfile;
    std::string                         p_outputfile;
    std::vector< lpe::data_assignment > p_initAssignments;
    std::set< lpe::data_variable >      p_S;                // <-inhert process parameters
    std::set< lpe::data_variable >      p_usedVars;
    bool                                p_verbose;
    bool                                p_debug;
    lpe::specification                  p_spec;
 
    //Only used by getDataVarIDs  
    std::set< lpe::data_variable >   p_foundVariables;       

  private:

    void getDatVarRec(atermpp::aterm_appl t);

    std::set< lpe::data_variable > getDataVarIDs(atermpp::aterm_appl input);

    template <typename Term>
    inline atermpp::term_list<Term> vectorToList(std::vector<Term> y);

    template <typename Term>
    inline atermpp::term_list<Term> setToList(std::set<Term> y);

  public:

    void setVerbose(bool b);
    void setDebug(bool b);
    void setSaveFile(std::string const& x);
    bool loadFile(std::string const& filename);
    bool readStream();
    void writeStream(lpe::specification newSpec);
    void filter();
    void output();
    std::string getVersion();
};

// Squadt protocol interface and utility pseudo-library
#ifdef ENABLE_SQUADT_CONNECTIVITY
#include <utility/squadt_utility.h>

class squadt_interactor : public squadt_tool_interface {

  private:

    static const char*  lpd_file_for_input;  ///< file containing an LPE that can be imported
    static const char*  lpd_file_for_output; ///< file used to write the output to

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

const char* squadt_interactor::lpd_file_for_input  = "lpd_in";
const char* squadt_interactor::lpd_file_for_output = "lpd_out";

void squadt_interactor::set_capabilities(sip::tool::capabilities& c) const {
  c.add_input_combination(lpd_file_for_input, sip::mime_type("lpe"), sip::tool::category::transformation);
}

void squadt_interactor::user_interactive_configuration(sip::configuration& c) {
  if (c.is_fresh()) {
    c.add_output(lpd_file_for_output, sip::mime_type("lpe"), c.get_output_name(".lpe"));
  }
}

bool squadt_interactor::check_configuration(sip::configuration const& c) const {
  bool result = true;

  result &= c.input_exists(lpd_file_for_input);
  result &= c.output_exists(lpd_file_for_output);

  return (result);
}

bool squadt_interactor::perform_task(sip::configuration& c) {
  lpeParElm parelm;

  std::string input_file_name  = c.get_input(lpd_file_for_input).get_location();
  std::string output_file_name = c.get_output(lpd_file_for_output).get_location();

  if (parelm.loadFile(input_file_name)) {
    parelm.setSaveFile(output_file_name);

    parelm.filter();
    parelm.output(); 

    return (true);
  }
  else {
    send_error("Invalid input, incorrect format?");
  }

  return (false);
}
#endif

using namespace atermpp;

using atermpp::aterm;
using atermpp::aterm_appl;
using atermpp::aterm_list;

const char* version = "0.5.1";

void lpeParElm::getDatVarRec(aterm_appl t) {
  if(gsIsDataVarId(t)){
    p_foundVariables.insert(t);
  };

  for(aterm_appl::iterator i = t.begin(); i!= t.end();i++) {
    getDatVarRec(aterm_appl(*i));
  } 
} 
  
// Returns a vector in which each element is a AtermsAppl (DataVarID)  
//
inline std::set< lpe::data_variable > lpeParElm::getDataVarIDs(aterm_appl input) {
  p_foundVariables.clear();
  getDatVarRec(input);

  return p_foundVariables;
}
  
// template for changing a vector into a list
//  
template <typename Term>
inline term_list<Term> lpeParElm::vectorToList(std::vector<Term> y) { 
  term_list<Term> result;

  for(typename std::vector<Term>::iterator i = y.begin(); i != y.end() ; i++) { 
      result = push_front(result,*i); 
  }

  return atermpp::reverse(result); 
} 

// template for changing a set into a list
//  
template <typename Term>
inline term_list<Term> lpeParElm::setToList(std::set<Term> y) { 
  term_list<Term> result;

  for(typename std::set<Term>::iterator i = y.begin(); i != y.end() ; i++) { 
    result = push_front(result,*i); 
  }

  return atermpp::reverse(result); 
} 
  
// Sets verbose option
// Note: Has to be set
//
inline void lpeParElm::setVerbose(bool b) {
  if (b)
  {
    gsSetVerboseMsg();
  }
  p_verbose = b;
}

// Sets verbose option
// Note: Has to be set
//  
inline void lpeParElm::setDebug(bool b) {
  if (b)
  {
    gsSetDebugMsg();
  }
  p_debug = b;
}

// Set output file
//  
inline void lpeParElm::setSaveFile(std::string const& x) {
  p_outputfile = x;
}

// Loads an LPE from file
// returns true if succeeds
//  
inline bool lpeParElm::loadFile(std::string const& filename) {
  p_inputfile = filename;
  if (!p_spec.load(p_inputfile)){
    gsErrorMsg("lpeparelm: error: could not read input file '%s'\n", filename.c_str());
    return false;      
  };
  return true;   
}

// Reads an LPE from stdin
// returns true if succeeds
//  
inline bool lpeParElm::readStream() {
  ATermAppl z = (ATermAppl) ATreadFromFile(stdin);
  if (z == NULL){
    gsErrorMsg("lpeparelm: Could not read LPE from stdin\n");
    return false;
  };
  if (!gsIsSpecV1(z)){
    gsErrorMsg("lpeparelm: Stdin does not contain an LPE\n");
    return false;
  }

  p_spec = lpe::specification(z);
  return true;
}

// Writes file to stdout
//
void lpeParElm::writeStream(lpe::specification newSpec)  {
  assert(gsIsSpecV1((ATermAppl) newSpec));
  ATwriteToBinaryFile(aterm(newSpec) , stdout);
}

// The lpeparelm filter
//  
void lpeParElm::filter() {
  using namespace lpe;

  LPE                      lpe       = p_spec.lpe();

  std::vector< aterm_appl >          foundParameters;
  std::set< data_variable >     T;
  std::set< data_variable >     foundVariables;
  

  // Searching for process parameters:
  // In a condition:     c_i(d,e_i) for some i \in I
  // In an argument:     f_i(d,e_i) for some i \in I
  // In a timecondition: t_i(d,e_i) for some i \in I
  //
  if(p_verbose){
    gsVerboseMsg("lpeparelm: Searching for used process parameters: ");
  }
  
  int counter = 0;
  int n = lpe.summands().size();
  for(lpe::summand_list::iterator currentSummand = lpe.summands().begin(); currentSummand != lpe.summands().end(); currentSummand++){ 
    
    gsDebugMsg("Summand: %d/%d\n", ++counter, n);
    gsVerboseMsg(".");
    
    //Condition
    //
    foundVariables = getDataVarIDs(aterm_appl(currentSummand->condition()));
    gsDebugMsg("%s\n", pp(currentSummand->condition()).c_str());
    gsDebugMsg("\033[36m%s\033[0m\n", setToList(foundVariables).to_string().c_str());
    for(std::set< lpe::data_variable>::iterator i = foundVariables.begin(); i != foundVariables.end(); i++){
       p_usedVars.insert(data_variable(*i));
    }

    //Time
    //
    if (currentSummand->has_time()){
      foundVariables = getDataVarIDs(aterm_appl(currentSummand->time()));
      gsDebugMsg("%s\n", pp(currentSummand->time()).c_str());
      gsDebugMsg("\033[39m%s\033[0m\n", setToList(foundVariables).to_string().c_str());
      for(std::set< lpe::data_variable >::iterator i = foundVariables.begin(); i != foundVariables.end(); i++){
         p_usedVars.insert(data_variable(*i));
      };
    }
   
    
    //Actions
    //
    for(lpe::action_list::iterator i = currentSummand->actions().begin(); i != currentSummand->actions().end(); i++){
      for(lpe::data_expression_list::iterator j = i->arguments().begin(); j != i->arguments().end(); j++){
        foundVariables = getDataVarIDs(aterm_appl(*j));
        gsDebugMsg("%s\n", i->to_string().c_str());
        gsDebugMsg("\033[31m%s\033[0m\n", setToList(foundVariables).to_string().c_str());  
        for(std::set< lpe::data_variable >::iterator k = foundVariables.begin(); k != foundVariables.end(); k++){
	         p_usedVars.insert(*k);
	      };
      };  
    };
    gsDebugMsg("\033[32m%s\033[0m\n", setToList(p_usedVars).to_string().c_str());
  }
  gsVerboseMsg("\n");
  
  // Needed all process parameters which are not marked have to be eliminated
  //
  int cycle = 0;
  if(p_verbose){
    gsVerboseMsg("lpeparelm: Searching for dependent process parameters\n");
  } 
  bool reset = true;
  while (reset){
    gsVerboseMsg("lpeparelm:   Cycle %d: ", ++cycle);
    reset = false;
    //counter = 0; 
    for(summand_list::iterator currentSummand = lpe.summands().begin(); currentSummand != lpe.summands().end(); currentSummand++){
      gsDebugMsg("Summand :%d/%d\n", ++counter, n);    
      gsVerboseMsg(".");
      for(data_assignment_list::iterator i = currentSummand->assignments().begin(); i !=  currentSummand->assignments().end() ;i++){
        if (p_usedVars.find(i->lhs()) != p_usedVars.end()){
          foundVariables = getDataVarIDs(aterm_appl(i->rhs()));
          gsDebugMsg("%s\n", pp(i->rhs()).c_str());
          unsigned int  z = p_usedVars.size();
          for(std::set< lpe::data_variable >::iterator k = foundVariables.begin(); k != foundVariables.end(); k++){
	          p_usedVars.insert(*k);
	          gsDebugMsg("%s\n", pp(*k).c_str());
	        }
	        gsDebugMsg("%d----%d\n", z, p_usedVars.size());
          if (p_usedVars.size() != z){
            reset = true;
            gsDebugMsg("\033[39m Match added: %s\033[0m\n", i->lhs().to_string().c_str()); 
          };  
        }
      }
    }
    gsVerboseMsg("\n");
    gsDebugMsg("%s\n", setToList(p_usedVars).to_string().c_str());
  }

  for(data_variable_list::iterator di = lpe.process_parameters().begin(); di != lpe.process_parameters().end() ; di++){
    T.insert(*di);	  
  };
  set_difference(T.begin(), T.end(),  p_usedVars.begin(),  p_usedVars.end(), inserter(p_S, p_S.begin()));

  //if (p_S.size() ==0){
  //  assert(test());
  //}

  if (p_verbose) {
    gsVerboseMsg("lpeparelm: Number of removed process parameters: %d\n", p_S.size());
    if (p_S.size() !=0){
      gsVerboseMsg("lpeparelm:   [ ");
      for(std::set< lpe::data_variable >::iterator i = p_S.begin(); i != (--p_S.end()); i++){
        gsVerboseMsg("%s, ", i->unquoted_name().c_str());
      }
      gsVerboseMsg("%s ]\n", (*(--p_S.end())).unquoted_name().c_str());
    }
  }// else {  
  // gsVerboseMsg("Number of removed process parameters : %d\n", p_S.size();
  //}
}

inline void lpeParElm::output() {
  using namespace lpe;

  LPE lpe = p_spec.lpe();
  summand_list rebuild_summandlist;
  data_variable_list rebuild_process_parameters;
  data_expression_list rebuild_data_expression_pars;
  
  for(data_variable_list::iterator i = lpe.process_parameters().begin() ; i != lpe.process_parameters().end() ; i++){
    if (p_usedVars.find(*i) != p_usedVars.end()){
      rebuild_process_parameters = push_front(rebuild_process_parameters, *i);
    }
  }

  //Remove process parameters in summands which are not used
  //
  summand_list summands = lpe.summands();
  for(lpe::summand_list::iterator i = summands.begin(); i != summands.end(); i++){
    data_assignment_list rebuild_assignments;
    for(lpe::data_assignment_list::iterator j = i->assignments().begin(); j != i->assignments().end();  j++){
      bool b = false;
      for(std::set< lpe::data_variable >::iterator k = p_S.begin(); k != p_S.end() ;k++){
        b = b || (*k == j->lhs());        
      }
      if (!b) {
        rebuild_assignments = push_front(rebuild_assignments , *j);
      } 
    }  
    //Construct the new summation_variable_list
    //
    data_variable_list rebuild_summation_variables;
    for(data_variable_list::iterator j = i->summation_variables().begin(); j != i->summation_variables().end(); j++ ){
      bool b = false;
      for(std::set<lpe::data_variable>::iterator k = p_S.begin(); k != p_S.end(); k++){
        b = b || (*k == *j);
      }
      if (!b){
        rebuild_summation_variables = push_front(rebuild_summation_variables, *j);
      }    
    }
    //LPE_summand(data_variable_list summation_variables, data_expression condition, 
    //          bool delta, action_list actions, data_expression time, 
    //          data_assignment_list assignments);
    LPE_summand rebuild_summand; 
   
    rebuild_summand = LPE_summand(atermpp::reverse(rebuild_summation_variables), i->condition(),
      i->is_delta(), i->actions(), i-> time(), atermpp::reverse(rebuild_assignments));  
    rebuild_summandlist = push_front(rebuild_summandlist, rebuild_summand);
  }
  
  //construct new specfication
  LPE rebuild_lpe;
  rebuild_lpe = LPE(
    lpe.free_variables(), 
    atermpp::reverse(rebuild_process_parameters), 
    atermpp::reverse(rebuild_summandlist)
  );

  data_variable_list               rebuild_initial_variables;
  data_expression_list             rebuild_initial_state;
  data_expression_list::iterator   j = p_spec.initial_state().begin();

  
  for(data_variable_list::iterator i = p_spec.initial_variables().begin() ; i != p_spec.initial_variables().end() ; i++){
    if (p_usedVars.find(*i) != p_usedVars.end()){
      rebuild_initial_variables = push_front(rebuild_initial_variables, *i);
      rebuild_initial_state = push_front(rebuild_initial_state, *j);
    }
    j++;
  }
  
  // Rebuild spec
  //
  //specification(sort_list sorts, function_list constructors, 
  //            function_list mappings, data_equation_list equations, 
  //            action_label_list action_labels, LPE lpe, 
  //            data_variable_list initial_free_variables, 
  //            data_variable_list initial_variables, 
  //            data_expression_list initial_state);
  //
  specification rebuild_spec;
  rebuild_spec = specification(
    p_spec.data(), 
    p_spec.action_labels(), 
    rebuild_lpe,
    p_spec.initial_free_variables(), 
    atermpp::reverse(rebuild_initial_variables),
    atermpp::reverse(rebuild_initial_state)
  );
  
  assert(gsIsSpecV1((ATermAppl) rebuild_spec));
 
  if (p_outputfile.size() == 0){
    //if(!p_verbose){
    //  assert(!p_verbose);
      writeStream(rebuild_spec);
    //};
  } else {
    if(!rebuild_spec.save(p_outputfile)){
       gsErrorMsg("lpeparelm: Unsuccessfully written outputfile: %s\n", p_outputfile.c_str());
    };
  } 
}
  
inline std::string lpeParElm::getVersion() {
  return (version);
}


void parse_command_line(int ac, char** av, lpeParElm& parelm) {
  namespace po = boost::program_options;

  po::options_description description;

  /* Name of the file to read input from (or standard input: "-") */
  std::vector < std::string > file_names;

  description.add_options()
    ("verbose,v", "turn on the display of short intermediate messages")
    ("debug,d",   "turn on the display of detailed intermediate messages")
    ("version",   "display version information")
    ("help,h",    "display this help")
  ;
        
  po::options_description hidden("Hidden options");

  hidden.add_options()
     ("file_names", po::value< std::vector< std::string> >(), "input/output files")
  ;
        
  po::options_description cmdline_options;
  cmdline_options.add(description).add(hidden);
        
  po::options_description visible("Allowed options");
  visible.add(description);
        
  po::positional_options_description p;
  p.add("file_names", -1);

  po::variables_map vm;
  po::store(po::command_line_parser(ac, av).options(cmdline_options).positional(p).run(), vm);
  po::notify(vm);
     
  if (vm.count("help")) {
    std::cerr << "Usage: "<< av[0] << " [OPTION]... [INFILE [OUTFILE]] \n"
              << "Removes unused parameters from the LPD read from standard input or INFILE." << std::endl
              << "By default the result is written to standard output, and otherwise to OUTFILE." << std::endl
              << std::endl << description;

    exit (0);
  }
        
  if (vm.count("version")) {
    std::cerr << version << " (revision " << REVISION << ")" << std::endl;

    exit (0);
  }

  parelm.setVerbose(vm.count("verbose") > 0);
  parelm.setDebug(vm.count("debug") > 0);

  if (vm.count("file_names")){
    file_names = vm["file_names"].as< std::vector< std::string > >();
  }

  if (file_names.size() == 0){
    /* Read from standard input */
    if (!parelm.readStream()) {
      exit (1);
    }
  }
  else if (2 < file_names.size()) {
    std::cerr << "lpeparelm: Specify only INPUT and/or OUTPUT file (too many arguments)."<< std::endl;

    exit (0);
  }
  else {
    if (!parelm.loadFile(file_names[0])) {
      exit (1);
    }

    if (file_names.size() == 2) {
      parelm.setSaveFile(file_names[1]);
    }
  }
}

int main(int argc, char** argv) {
  ATerm     bottom;

  ATinit(argc,argv,&bottom);

  gsEnableConstructorFunctions();
  
#ifdef ENABLE_SQUADT_CONNECTIVITY
  squadt_interactor c;

  if (!c.try_interaction(argc, argv)) {
#endif
    lpeParElm parelm;

    parse_command_line(argc,argv,parelm);

    parelm.filter();
    parelm.output(); 
#ifdef ENABLE_SQUADT_CONNECTIVITY
  }
#endif

  return (0);
}
