//  Copyright 2007 Simona Orzan. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ./gauss.h

#include <cstdio>
#include <string>
#include <iostream> 
#include <sstream> 
#include <fstream> 

#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/pbes_expression.h"
#include "librewrite.h" // rewriter
#include "prover/bdd_prover.h" // prover
#include "print/messaging.h"

#define PREDVAR_MARK '$'

using namespace lps;


atermpp::vector<pbes_equation> solve_pbes(pbes<> pbes_spec, bool interactive, int bound, std::string solver, std::string rewriter);

pbes_equation solve_equation(pbes_equation e, 
			     bool interactive, int bound, BDD_Prover* p);

pbes_expression substitute(pbes_expression expr, 
			   propositional_variable X, pbes_expression solX);

pbes_expression update_expression(pbes_expression e, atermpp::vector<pbes_equation> es_solution);

pbes_expression rewrite_pbes_expression(pbes_expression e, BDD_Prover* p);


void solve_equation_interactive(propositional_variable X, 
				pbes_expression defX, 
				pbes_expression approx);


