// Author(s): Alexander van Dam
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbesinfo.cpp
/// \brief Add your file description here.

// ======================================================================
//
// file          : pbesinfo 
// date          : 11-04-2007
// version       : 0.1.0
//
// author(s)     : Alexander van Dam <avandam@damdonk.nl>
//
// ======================================================================

#define NAME "pbesinfo"
#define AUTHOR "Alexander van Dam"

//C++
#include <cstdio>
#include <exception>
#include <iostream>
#include <string>

//MCRL2-specific
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/utilities/aterm_ext.h"
#include "mcrl2/utilities/command_line_interface.h" // must come after mcrl2/core/messaging.h

using namespace std;
using namespace mcrl2::utilities;
using namespace mcrl2::core;
using namespace mcrl2::data;
using namespace mcrl2::lps;
using namespace mcrl2::pbes_system;

struct t_tool_options {
  bool opt_full;
  string file_name; // Name of the file to read input from
};

t_tool_options parse_command_line(int ac, char** av) {
  interface_description clinterface(av[0], NAME, AUTHOR, "[OPTION]... [INFILE]\n",
                           "Print basic information on the PBES in INFILE. If INFILE is not present, stdin\n"
                           "is used.");

  clinterface.add_option("full", "display the predicate variables and their signature", 'f');

  command_line_parser parser(clinterface, ac, av);

  t_tool_options tool_options = { (0 < parser.options.count("full")), "-" };

  if (0 < parser.arguments.size()) {
    tool_options.file_name = parser.arguments[0];
  }
  if (1 < parser.arguments.size()) {
    parser.error("too many file arguments");
  }

  return tool_options;
}

int main(int argc, char** argv)
{
  MCRL2_ATERM_INIT(argc, argv)

  try {
    t_tool_options tool_options = parse_command_line(argc, argv);

     pbes<> pbes_specification;

     /// If PBES can be loaded from file_name, then
     /// - Show if PBES is closed and if it is well formed
     ///	- Show number of equations
     /// - Show number of mu's / nu's.
     /// - Show which predicate variables have mu's and which predicate variables have nu's
     /// - Show predicate variables and their type
     /// else 
     /// - Give error

     pbes_specification.load(tool_options.file_name);

     // Get PBES equations. Makes a lot of function calls more readable.
     atermpp::vector<pbes_equation> eqsys;
     eqsys = pbes_specification.equations();
     
     bool pbes_well_formed = pbes_specification.is_well_typed();
     bool pbes_closed = pbes_specification.is_closed();

     // Vectors for storing intermediate results
     vector<identifier_string> predvar_mu;
     vector<identifier_string> predvar_nu;
     vector<propositional_variable> predvar_data;
     		
     // Integers for showing totals
     int mu = 0;
     int nu = 0;
     int fp_errors = 0;
     
     for (atermpp::vector<pbes_equation>::iterator fp_i = eqsys.begin(); fp_i != eqsys.end(); fp_i++)
     {
        // - Store data_variables
        predvar_data.push_back(fp_i->variable());
     	
        // Check on mu or nu
        if (fp_i->symbol().is_mu())
        {
          // If fp is mu:
          // - Increase #mu's
          // - Store predicate variable in mu-list and common list
          // - Store data_variables
          mu++;
          if (tool_options.opt_full)
            predvar_mu.push_back(fp_i->variable().name());
        }
        else if (fp_i->symbol().is_nu())
        {
        // If fp is nu:
        // - Increase #nu's
        // - Store predicate variable in nu-list and common list
          nu++;
          if (tool_options.opt_full)
            predvar_nu.push_back(fp_i->variable().name());
        }
        else
        {
          fp_errors++;
        }
     }
     		
     // Show file from which PBES was read
     cout << "Input read from '" << ((tool_options.file_name == "-") ? "standard input" : tool_options.file_name) << "'" << endl << endl;
     
     // Check if errors occurred in reading PBEs
     if (fp_errors != 0)
     {
     	cerr << "WARNING: Reading number of mu's and nu's had errors. Results may be incorrect" << endl;
     }
     
     // Show if PBES is closed and well formed
     cout << "The PBES is " << (pbes_closed ? "" : "not ") << "closed and " << (pbes_well_formed ? "" : "not ") << "well formed" << endl;
     
     // Show number of equations
     cout << "Number of equations: " << eqsys.size() << endl;
     
     // Show number of mu's with the predicate variables from the mu's
     cout << "Number of mu's:      " << mu;
     int size_mu = predvar_mu.size();	
     int mu_done = 1;
     if (size_mu > 0)
       cout << "   (";
     for (vector<identifier_string>::iterator i = predvar_mu.begin(); i != predvar_mu.end(); i++)
     {
     	cout << *i << ((mu_done == size_mu) ? ")" : ", ");
     	mu_done++;
     }
     cout << endl;
     		
     // Show number of nu's with the predicate variables from the nu's
     cout << "Number of nu's:      " << nu;
     int size_nu = predvar_nu.size();	
     int nu_done = 1;
     if (size_nu > 0)
     	cout << "   (";
     for (vector<identifier_string>::iterator i = predvar_nu.begin(); i != predvar_nu.end(); i++)
     {
     	cout << *i << ((mu_done == size_mu) ? ")" : ", ");
     	nu_done++;
     }
     cout << endl;
     
     // Show binding variables with their signature
     if (tool_options.opt_full)
     {
     	int nr_predvar = 1;
     	string sort_bool = "Bool";
     	for (vector<propositional_variable>::iterator pv_i = predvar_data.begin(); pv_i != predvar_data.end(); pv_i++)
     	{
     	  int bv_size = pv_i->parameters().size();
     	  int nr_sorts = 1;
     	  if (nr_predvar == 1)
     	    cout << "Predicate variables: " << pv_i->name() << " :: ";
     	  else
     	    cout << "                     " << pv_i->name() << " :: ";
     	  for (term_list<data_variable>::iterator dv_i = pv_i->parameters().begin(); dv_i != pv_i->parameters().end(); dv_i++)
     	  {
     	    cout << pp(dv_i->sort());
     	    if (nr_sorts < bv_size)
     	    {
     	      cout << " x ";
     	      nr_sorts++;
     	    }
     	    else
     	      cout << " -> " << pp(sort_expression(sort_bool));
     	    }
     	  cout << endl;
     	  nr_predvar++;
     	}
     }
  }
  catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }

  return EXIT_FAILURE;
}
