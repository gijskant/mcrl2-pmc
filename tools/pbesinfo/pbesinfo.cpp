// Author(s): Alexander van Dam
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
#define VERSION "July 2007"
#define AUTHOR "Alexander van Dam"

//C++
#include <cstdio>
#include <exception>
#include <iostream>
#include <string>

//Boost
#include <boost/program_options.hpp>

//MCRL-specific
#include "mcrl2/pbes/pbes.h"
#include "print/messaging.h"

using namespace std;
using namespace lps;
using namespace ::mcrl2::utilities;

namespace po = boost::program_options;

typedef struct{
	bool opt_full;
} t_tool_options;

// Name of the file to read input from
string file_name;

t_tool_options parse_command_line(int argc, char** argv)
{
	po::options_description desc;
	t_tool_options tool_options;
	bool opt_full = false;

	desc.add_options()
			("help,h",		"display this help")
			("verbose,v",	"turn on the display of short intermediate messages")
			("debug,d",		"turn on the display of detailed intermediate messages")
			("version",		"display version information")
			("full,f",		"display full information on the PBES")
			;
	
	po::options_description hidden("Hidden options");
	hidden.add_options()
			("INFILE",		po::value<string>(), "input file")
			;
	
	po::options_description cmdline_options;
	cmdline_options.add(desc).add(hidden);
	
	po::options_description visible("Allowed options");
	visible.add(desc);
	
	po::positional_options_description p;
	p.add("INFILE", -1);
	
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
	po::notify(vm);
	
	if (vm.count("help"))
	{
		cerr << "Usage: " << argv[0] << " [OPTION]... [INFILE]" << endl;
		cerr << "Print basic information on the PBES in INFILE." << endl;
		cerr << endl;
		cerr << desc;
		
		exit(0);
	}
	
	if (vm.count("version"))
	{
		cerr << "pbesinfo " << VERSION <<  " (revision " << REVISION << ")" << endl;
		
		exit(0);
	}

	if (vm.count("full"))
	{
		opt_full = true;
	}
	
	if (vm.count("debug"))
	{
		gsSetDebugMsg();	
	}
	
	if (vm.count("verbose"))
	{
		gsSetVerboseMsg();
	}
	
	file_name = (0 < vm.count("INFILE")) ? vm["INFILE"].as<string>() : "-";	

	tool_options.opt_full = opt_full;
	return tool_options;
}

int main(int argc, char** argv)
{
	ATerm bottom;
	ATinit(argc, argv, &bottom);

	gsEnableConstructorFunctions();

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

  try
  {
	  pbes_specification.load(file_name);

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
		cout << "Input read from '" << ((file_name == "-") ? "standard input" : file_name) << "'" << endl << endl;
		
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
			cout << *i;
			if (mu_done == size_mu)
				cout << ")";
			else
				cout << ", ";
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
			cout << *i;
			if (nu_done == size_nu)
				cout << ")";
			else
				cout << ", ";
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
						cout << " -> " << pp(lps::sort(sort_bool));
				}
				cout << endl;
				nr_predvar++;
			}
		}
	}
  catch (std::runtime_error e)
  {
    std::cerr << e.what() << std::endl;
  }

	return 0;
}
