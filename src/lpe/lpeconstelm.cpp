// TODO
// BRACKETS VERWIJDEREN IN BOOST OM DE OPTIES!!!!!
// GOED MAKEN VAN LAYOUT (UITLIJNING)

//C++
#include <iostream>
#include <vector>
#include <set>
#include <stdbool.h>
#include <string>
#include <iterator>
#include <fstream>

//Boost
#include <boost/program_options.hpp>

//mCRL2
#include <aterm2.h>
#include "atermpp/aterm.h"
#include "mcrl2/mcrl2_visitor.h"
#include "mcrl2/specification.h"
#include "mcrl2/predefined_symbols.h"
#include "mcrl2/sort.h"

#include "libgsrewrite.h"
#include "gsfunc.h"
#include "gslowlevel.h"

using namespace std;
using namespace mcrl2;
using namespace atermpp;

//Maximum number of proces parameters: 4294967295 [@AMD 1700+, Linux-FC4, 256 MB RAM]

using namespace std;
using namespace mcrl2;
using namespace atermpp;

namespace po = boost::program_options;
po::variables_map vm;

//Constanten
string version = "Version 0.2";

//Global variables
data_expression_list          vinit; //init vector
data_assignment_list          newstatevector; // newstate vector
data_assignment_list          tvector;
data_assignment_list          sv;
data_assignment_list          ainit;
int                           n;    //number of process parameters


bool compare(data_expression x, data_expression y, data_equation_list equations)
{
  ATermAppl x1 = rewrite(x.to_ATermAppl(), gsMakeDataEqnSpec(equations.to_ATermList()));
  ATermAppl y1 = rewrite(y.to_ATermAppl(), gsMakeDataEqnSpec(equations.to_ATermList()));
  return atermpp::aterm(x1) == atermpp::aterm(y1);
}

data_assignment_list nextstate(data_assignment_list currentstate, data_assignment_list assignments, data_equation_list equations)
{
  data_assignment_list out;
  
  data_assignment_list::iterator i = currentstate.begin();
  while( i!= currentstate.end()){
    data_assignment_list::iterator j = assignments.begin(); 
    while (j != assignments.end()){
      if (i->lhs() == j->lhs()){
        data_expression z = j->rhs();
        data_assignment_list::iterator k = currentstate.begin();
        while (k != currentstate.end()){
          z = z.substitute(*k);
          k++;
        };
        out = append(
                out, 
                data_assignment( 
                  i->lhs(), 
                  data_expression(
                    aterm ( 
                      rewrite(
                        z.to_ATermAppl(), 
                        gsMakeDataEqnSpec(
                          equations.to_ATermList()
                        )  
                      )
                    ) 
                  )
                )
              );
      } else {
        out = append(out,*i);
      }
    j++;
    } 
    i++;
  }
  
 return out; 
}

data_expression_list rhsl(data_assignment_list x)
{
  data_expression_list y;
  data_assignment_list::iterator i = x.begin();
  while (i != x.end()) {
    y = append(y, i->rhs() );
    i++;
  };

  return y;
}

data_variable_list lhsl(data_assignment_list x)
{
  data_variable_list y;
  data_assignment_list::iterator i = x.begin();
  while (i != x.end()) {
    y = append(y, i->lhs() );
    i++;
  };

  return y;
}

bool eval_cond(data_expression datexpr, data_assignment_list statevector, data_equation_list equations, set<int> S){

  bool b;
  set<int>::iterator i;
  data_assignment_list conditionvector; 
  
  //
  //ORDE N^3
  //
  
  // TODO:
  //   Beter: end() -> begin() /\ push_front gebruiken
  //   Element_at = N
  //   Append = N
  //   begin-> end = N

  
  i = S.begin();
  while (i != S.end()) {
    conditionvector = append(conditionvector, data_assignment(  element_at(lhsl(statevector), *i) , element_at( rhsl(statevector), *i  )));
    i++;
  };

  //assert(false);
    
  data_assignment_list::iterator j = conditionvector.begin();
  while (j != conditionvector.end() ){;
    datexpr = datexpr.substitute(*j);
    j++;
  };

  //cout << conditionvector.pp() << endl;
  
  // 
  // !!!!!!! data_expression(gsMakeOpIdFalse()) !!!!!! DIRECTE AANROEP UIT GSFUNC
  //

  b = !compare(data_expression(gsMakeOpIdFalse()), datexpr, equations);

  //cout << conditionvector.pp() << endl << datexpr.pp() << endl << b << endl << endl;

  return b;
}

void print_const(specification spec , set< int > S)
{  

  LPE lpe = spec.lpe();

  set< int >::iterator i;
  data_assignment_list sub = spec.init_assignments();
  data_assignment_list result;
  
  i = S.begin();
  while (i != S.end()) {
    result = append(result, data_assignment(  element_at(lhsl(sub), *i) , element_at( rhsl(sub), *i  )));
    i++;
  };

  //Display constants
  cout << result.pp() << endl;
  
  return;
}

void constelm(string filename, int option)
{
  specification spec;
  if (!spec.load(filename))
  {
    cerr << "could not read " << filename << endl;
    return;
  }
  LPE lpe = spec.lpe();

  n = lpe.process_parameters().size();
  sv = spec.init_assignments();
  ainit = spec.init_assignments();
  newstatevector = sv;  

  
  set< int > V; 
  set< int > S;

  for(int j=0; j <= (n-1) ; j++){
    V.insert(j);
  };
  
  set< int > D;
  //cout << lpe.summands().size() << endl;;

  ////
  // Als V <  S dan zijn er variabele process parameters gevonden
  ////


  while(S.size()!=V.size()) {
    sv = newstatevector;
    S = V;
    V.clear();

    set< int > S_dummy;        
    
    for(summand_list::iterator s_current = lpe.summands().begin(); s_current != lpe.summands().end(); ++s_current){ 
      if (eval_cond(s_current->condition(), sv, spec.equations(), S)){
        
        data_assignment_list ass_nextstate = s_current->assignments();
        tvector = nextstate(sv, ass_nextstate, spec.equations() );
      
        set< int >::iterator j = S.begin();
        while (j != S.end()){
          if (element_at(rhsl(ainit), *j) != (element_at(rhsl(tvector), *j)  ) ){
            S_dummy.insert(*j);
            newstatevector = replace(newstatevector, element_at(tvector, *j), *j);
            
          };
          j++;
        };
        cout << sv.pp() << " -- " << tvector.pp() << " -- " << newstatevector.pp() << endl; 
     };
         
    }; 
    
    set_difference(S.begin(), S.end(), S_dummy.begin(), S_dummy.end(), inserter(V, V.begin()));

    cout << endl;
  }; 
  //cout << V.size() << endl; 

  print_const(spec , S);

  return;
}

int main(int ac, char* av[])
{
      string filename;
      int opt = 0;

      try {
        po::options_description desc;
        desc.add_options()
            ("help,h",      "display this help")
            ("version,v",   "display version information")
            ("monitor,m",   "display progress information")
            ("nosingleton", "do not remove sorts consisting of a single element")
            ("nocondition", "do not use conditions during elimination (faster)")
        ;
	
	po::options_description hidden("Hidden options");
	hidden.add_options()
	    ("INFILE", po::value<string>(), "input file" )
	;
	
	po::options_description cmdline_options;
	cmdline_options.add(desc).add(hidden);
	
	po::options_description visible("Allowed options");
	visible.add(desc);
	
	po::positional_options_description p;
	p.add("INFILE", -1);
	
	po::variables_map vm;
        po::store(po::command_line_parser(ac, av).
                  options(cmdline_options).positional(p).run(), vm);
        po::notify(vm);
        
        if (vm.count("help") || ac == 1) {
            cerr << "Usage: "<< av[0] << " [OPTION]... INFILE\n";
            cerr << "Remove constant process parameters from the LPE in INFILE, and write the result" << endl;
            cerr << "to stdout." << endl;
            cerr << endl;
            cerr << desc;
            return 0;
        }
        
        if (vm.count("version")) {
	        cerr << version << endl;
	        return 0;
	      }

        if (vm.count("monitor")) {
          //cerr << "Displaying progress" << endl;
          opt = 1;
	      }

        if (vm.count("nosingleton")) {
          //cerr << "Active: no removal of process parameters which have sorts of cardinatilty one" << endl;
          opt = 2;
	      }

        if (vm.count("nocondition")) {
          //cerr << "Active: All conditions are true" << endl;
          opt = 3;
	      }

        if (vm.count("INFILE"))
        {
          filename = vm["INFILE"].as<string>();
	      }
        //set< int > S = constelm(filename, opt);
        constelm(filename, opt);      
	
    }
    catch(exception& e)
    {
        cerr << e.what() << "\n";
        return 1;
    }    
    return 0;
}

