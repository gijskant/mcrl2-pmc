// Author(s): Alexander van Dam
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file pbes_io.h
/// \brief Add your file description here.

#ifndef MCRL2_PBES_IO_H
#define MCRL2_PBES_IO_H

#include <cstdio>
#include <exception>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <sstream>
#include <string>
#include "mcrl2/atermpp/indexed_set.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/data/data.h"
#include "mcrl2/pbes/pbes.h"

namespace mcrl2 {

namespace pbes_system {

//function convert_rhs_to_cwi
//---------------------------
/// \brief Convert a PBES expression to the format of the right hand side used by the CWI
inline
std::string convert_rhs_to_cwi(pbes_expression p, atermpp::indexed_set *variables)
{
  std::string result;
  if (pbes_expr::is_pbes_true(p))
    // PBESTrue => T
    result = "T";
  else if (pbes_expr::is_pbes_false(p))
    // PBESFalse => F
    result = "F";
  else if (pbes_expr::is_pbes_and(p))
  {
    //PBESAnd(a,b) => (a & b)
    std::string left = convert_rhs_to_cwi(accessors::left(p), variables);
    std::string right = convert_rhs_to_cwi(accessors::right(p), variables);
    result = "(" + left + "&" + right + ")";
  }
  else if (pbes_expr::is_pbes_or(p))
  {
    //PBESOr(a,b) => (a | b)
    std::string left = convert_rhs_to_cwi(accessors::left(p), variables);
    std::string right = convert_rhs_to_cwi(accessors::right(p), variables);
    result = "(" + left + "|" + right + ")";
  }
  else if (pbes_expr::is_propositional_variable_instantiation(p))
  {
    // PropVar => <Int>
    propositional_variable_instantiation propvarinst = propositional_variable_instantiation(p);
    data::data_variable_list empty;
    propositional_variable propvar = propositional_variable(propvarinst.name(), empty);
    long variable = variables->index(propvar);
    if (variable < 0)
    {
      core::gsErrorMsg("Error: The BES is not closed. Write to cwi-format failed.\n");
      throw std::runtime_error("exit!");
    }
    else
    {
      std::stringstream variable_stream;
      variable_stream << variable;
      result = variable_stream.str();
    }
  }
  else
  {
    core::gsErrorMsg("The used equation system is not a BES. Could not save this in CWI-format.\n");
    throw std::runtime_error("exit!");
  }
  return result;
}

//function save_pbes_in_cwi_format
//--------------------------------
/// \brief Convert the PBES to the format used at the CWI for their BES solvers
inline
void save_pbes_in_cwi_format(pbes<> pbes_spec, std::string outfilename)
{
  // Use an indexed set to keep track of the variables and their cwi-representations
  atermpp::vector<pbes_equation> eqsys = pbes_spec.equations();
  atermpp::indexed_set *variables = new atermpp::indexed_set(2*eqsys.size(), 50);
  
  // Fill the indexed set
  for (atermpp::vector<pbes_equation>::iterator eq = eqsys.begin(); eq != eqsys.end(); eq++)
  {
    variables->put(eq->variable());
  } // because pbes is closed, all variables are done at this point

  std::vector< std::string > result;
  for (atermpp::vector<pbes_equation>::iterator eq = eqsys.begin(); eq != eqsys.end(); eq++)
  {
    // fixpoint:
    //  mu => min
    //  nu => max
    std::string fp;
    (eq->symbol().is_mu())?fp = "min":fp = "max";

    // variable:
    //  Integer representation of the propositional variable of the left hand side
    std::string variable;
    std::stringstream variable_stream;
    variable_stream << variables->index(eq->variable());
    variable = variable_stream.str();

    // Convert the right hand side of the equation to the CWI format
    std::string rhs = convert_rhs_to_cwi(eq->formula(), variables);
    std::string equation = fp + " " + variable + " = " + rhs + "\n";
    result.push_back(equation);
  }

  if (outfilename != "-")
  {
    std::ofstream outputfile;
    outputfile.open(outfilename.c_str(), std::ios::trunc);
  
    for (std::vector< std::string >::iterator res = result.begin(); res != result.end(); res++)
    {
      if (outputfile.is_open())
      {
        outputfile << *res;
      }
      else
      {
        core::gsErrorMsg("Could not save PBES to %s\n", outfilename.c_str());
        throw std::runtime_error("exit!");
      }
    }
    outputfile.close();
  }
  else
  {
    for (std::vector< std::string >::iterator res = result.begin(); res != result.end(); res++)
    {
      std::cout << *res ;
    }
  }
}

//function load_pbes
//------------------
/// \brief Load a PBES.
inline
pbes<> load_pbes(std::string infilename)
{
  pbes<> pbes_spec;
  if (infilename == "-")
  {
    try
    {
      pbes_spec.load("-");
    }
    catch (std::runtime_error e)
    {
      core::gsErrorMsg("Cannot open PBES from stdin\n");
      throw std::runtime_error("exit!");
    }
  }
  else
  {
    try
    {
      pbes_spec.load(infilename);
    }
    catch (std::runtime_error e)
    {
      core::gsErrorMsg("Cannot open PBES from '%s'\n", infilename.c_str());
      throw std::runtime_error("exit!");
    }
  }
  return pbes_spec;
}

//function save_pbes
//------------------
/// \brief Save a PBES in the format specified.
inline
void save_pbes(pbes<> pbes_spec, std::string outfilename, std::string outputformat)
{
  // Check if the result is a BES or a PBES
  bool is_bes = true;
  if ( (!pbes_spec.is_bes()) && (!pbes_spec.initial_state().parameters().empty() ) )
  {
    is_bes = false;
    core::gsVerboseMsg("The result is a PBES.\n");
  }
  else
  {
    core::gsVerboseMsg("The result is a BES.\n");
  }
  
  if (outputformat == "internal")
  { // In internal format
    core::gsVerboseMsg("Saving result in internal format...\n");
    if (!pbes_spec.save(outfilename, false))
    {
      core::gsErrorMsg("Could not save PBES to %s\n", outfilename.c_str());
      throw std::runtime_error("exit!");
    }
  }
  else if (outputformat == "binary")
  { // in binary format
    core::gsVerboseMsg("Saving result in binary format...\n");
    if (!pbes_spec.save(outfilename, true))
    {
      core::gsErrorMsg("Could not save PBES to %s\n", outfilename.c_str());
      throw std::runtime_error("exit!");
    }
  }
  else if (outputformat == "cwi")
  { // in CWI format only if the result is a BES, otherwise Binary
    core::gsVerboseMsg("Saving result in CWI format...\n");
    if (is_bes)
    {
      save_pbes_in_cwi_format(pbes_spec, outfilename);
    }
    else
    {
      core::gsWarningMsg("Saving in CWI format not possible. Saving result in binary format.\n");
      if (!pbes_spec.save(outfilename, true))
      {
        core::gsErrorMsg("Could not save PBES to %s\n", outfilename.c_str());
      throw std::runtime_error("exit!");
      }
    }
  }
}

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_IO_H
