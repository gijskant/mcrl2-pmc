// Author(s): Alexander van Dam, Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/pbes2bes.h

#ifndef MCRL2_PBES_PBES2BES_H
#define MCRL2_PBES_PBES2BES_H

#include <set>
#include <string>
#include <vector>
#include "mcrl2/atermpp/indexed_set.h"
#include "mcrl2/atermpp/table.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/data/data.h"
#include "mcrl2/data/data_variable_replace.h"
#include "mcrl2/data/sort_utility.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/utility.h"
#include "mcrl2/pbes/propositional_variable_replace.h"
#include "pbes_rewrite.h"

namespace mcrl2 {

namespace pbes_system {

struct t_instantiations {
  data::data_variable_list finite_var;     // List of all finite variables
  data::data_variable_list infinite_var;   // List of all infinite variables
  data::data_expression_list finite_exp;   // List of all finite expressions
  data::data_expression_list infinite_exp; // List of all infinite expressions

  void protect()
  {
    finite_var.protect();
    infinite_var.protect();
    finite_exp.protect();
    infinite_exp.protect();
  }

  void unprotect()
  {
    finite_var.unprotect();
    infinite_var.unprotect();
    finite_exp.unprotect();
    infinite_exp.unprotect();
  }
  void mark()
  {
    finite_var.mark();
    infinite_var.mark();
    finite_exp.mark();
    infinite_exp.mark();
  }
};

//function sort_names
//-------------------
/// \brief Sort the equation system with respect to the order of predicate variables in the original equation system
inline
atermpp::vector<pbes_equation> sort_names(std::vector< core::identifier_string > names_order, atermpp::vector<pbes_equation> to_sort)
{
  atermpp::vector<pbes_equation> result;
  if (names_order.size() == 1)
  {
    result = to_sort;
  }
  else
  {
    for (std::vector< core::identifier_string >::iterator on = names_order.begin(); on != names_order.end(); on++)
    {
      atermpp::vector<pbes_equation> todo;
      for (atermpp::vector<pbes_equation>::iterator eqi = to_sort.begin(); eqi != to_sort.end(); eqi++)
      {
        std::string name = eqi->variable().name();
        name = name.substr(0, name.find("@"));
        std::string ons = *on;
        if (name == ons)
        {
          result.push_back(*eqi);
        }
        else
        {
          todo.push_back(*eqi);
        }
      }
      to_sort = todo;
    }
  }

  return result;
}

//function create_propvar_name
//----------------------------
/// \brief Create a new propositional variable name
inline
core::identifier_string create_propvar_name(core::identifier_string propvar_name, data::data_expression_list del)
{
  std::string propvar_name_current = propvar_name;
  if (!del.empty())
  {
    for (data::data_expression_list::iterator del_i = del.begin(); del_i != del.end(); del_i++)
    {
      if (is_data_operation(*del_i))
      { //If p is a OpId
        propvar_name_current += "@";
        propvar_name_current += pp(*del_i);
      }
      else if (is_data_variable(*del_i))
      { // If p is a freevar
        core::gsErrorMsg("The propositional varaible contains a variable of finite sort.\n");
        core::gsErrorMsg("Can not handle variables of finite sort when creating a propositional variable name.\n");
        core::gsErrorMsg("Computation aborted.\n");
        std::cout << "Problematic Term: " << pp(*del_i) << std::endl;
        throw std::runtime_error("exit!");
      }
      else if (is_data_application(*del_i))
      { // If p is a data application
        propvar_name_current += "@";
        propvar_name_current += pp(*del_i);
      }
      else
      {
        core::gsErrorMsg("Can not rewrite the name of the propositional_variable\n");
        std::cout << "Problematic Term: " << pp(*del_i) << std::endl;
        throw std::runtime_error("exit!");
      }
    }
  }

  return propvar_name_current;
}

//function create_naive_propositional_variable_instantiation
//----------------------------------------------------------
/// \brief Create a new propositional variable instantiation with instantiated values and infinite variables
propositional_variable_instantiation create_naive_propositional_variable_instantiation(propositional_variable_instantiation propvarinst, atermpp::table *enumerated_sorts)
{
  data::data_expression_list finite_expression;
  data::data_expression_list infinite_expression;

  for (data::data_expression_list::iterator p = propvarinst.parameters().begin(); p != propvarinst.parameters().end(); p++)
  {
    if (enumerated_sorts->get(p->sort()) != NULL)
    { //sort is finite
      if (is_data_operation(*p))
      { // If p is a correct data operation
        finite_expression = push_back(finite_expression, *p);
      }
      else if (is_data_variable(*p))
      { // If p is a freevar
        core::gsErrorMsg("The propositional varaible contains a variable of finite sort.\n");
        core::gsErrorMsg("Can not handle variables of finite sort when creating a propositional variable name.\n");
        core::gsErrorMsg("Computation aborted.\n");
        std::cout << "Problematic Term: " << pp(*p) << std::endl;
        throw std::runtime_error("exit!");
      }
    }
    else
    { //sort is infinite
      infinite_expression = push_back(infinite_expression, *p);
    }
  }

  return propositional_variable_instantiation(create_propvar_name(propvarinst.name(), finite_expression), infinite_expression);
}

//function do_lazy_algorithm
//--------------------------
/// \brief Create a BES, using the lazy approach
inline
pbes<> do_lazy_algorithm(pbes<> pbes_spec)
{
  // Instantiate free variables in the system
  if (!pbes_spec.instantiate_free_variables())
  {
    core::gsErrorMsg("Instantiatiation of free variables failed!\n");
    throw std::runtime_error("exit!");
  }
  core::gsVerboseMsg("Using lazy approach...\n");

  propositional_variable_instantiation initial_state = pbes_spec.initial_state();
  atermpp::vector<pbes_equation> eqsys = pbes_spec.equations();
  data_specification data = pbes_spec.data();

  propositional_variable_instantiation new_initial_state;
  atermpp::vector<pbes_equation> new_equation_system;

  atermpp::set< propositional_variable_instantiation > states_todo;
  atermpp::set< propositional_variable_instantiation >::iterator current_state_it;

  int nr_of_equations = 0;
  data::data_variable_list empty_data_variable_list;
  data::data_expression_list empty_data_expression_list;

  Rewriter *rewriter = createRewriter(data);

  atermpp::table pbes_equations(2*eqsys.size(), 50);  // (propvarname, pbes_equation)
  atermpp::indexed_set states_done(10000, 50);    // (propvarinst)

  std::vector< core::identifier_string > names_order;

  core::gsVerboseMsg("Retrieving PBES equations from equation system...\n");
  for (atermpp::vector<pbes_equation>::iterator eqi = eqsys.begin(); eqi != eqsys.end(); eqi++)
  {
    pbes_equations.put(eqi->variable().name(), *eqi);
    names_order.push_back(eqi->variable().name());
  }

  states_todo.insert(initial_state);

  core::gsVerboseMsg("Computing BES...\n");
  while (states_todo.size() != 0)
  {
    // Get the first element of the set
    current_state_it = states_todo.begin();
    propositional_variable_instantiation current_state = *current_state_it;
    states_todo.erase(current_state);

    // Get equation which belongs to the current propvarinst and their needed parts
    pbes_equation current_pbeq = pbes_equation(pbes_equations.get(current_state.name()));
    propositional_variable current_variable = current_pbeq.variable();
    pbes_expression current_pbes_expression = current_pbeq.formula();

    // Create new propositional variable and propositional variable instantiation
    core::identifier_string new_propvar_name = create_propvar_name(current_variable.name(), current_state.parameters());
    propositional_variable new_variable = propositional_variable(new_propvar_name, empty_data_variable_list);
    propositional_variable_instantiation new_propvarinst = propositional_variable_instantiation(new_propvar_name, empty_data_expression_list);

    // Add the new instantiation to the table
    states_done.put(new_propvarinst);

    // Replace all occurrences in the right hand side and rewrite the expression
    pbes_expression new_pbes_expression;
    new_pbes_expression = data::replace_data_variable_sequence(current_pbes_expression, current_variable.parameters(), current_state.parameters());
    new_pbes_expression = pbes_expression_rewrite(new_pbes_expression, data, rewriter);

    propositional_variable_instantiation_list oldpropvarinst_list;
    propositional_variable_instantiation_list newpropvarinst_list;

    // Get all propvarinst of the rhs
    std::set< propositional_variable_instantiation > propvarinst_set = find_propositional_variable_instantiations(new_pbes_expression);

    for (std::set< propositional_variable_instantiation >::iterator pvi = propvarinst_set.begin(); pvi != propvarinst_set.end(); pvi++)
    {
      propositional_variable_instantiation temp_pvi = propositional_variable_instantiation(create_propvar_name(pvi->name(), pvi->parameters()), empty_data_expression_list);
      oldpropvarinst_list = push_front(oldpropvarinst_list, *pvi);
      if (states_done.index(temp_pvi) < 0)
      {
        states_todo.insert(*pvi);
      }
      newpropvarinst_list = push_front(newpropvarinst_list, temp_pvi);
    }

    // Replace the propvarinsts with the new ones
    new_pbes_expression = replace_propositional_variable_sequence(new_pbes_expression,  oldpropvarinst_list, newpropvarinst_list);

    // Create resulting pbes_equation and add it to equation system
    new_equation_system.push_back(pbes_equation(current_pbeq.symbol(), new_variable, new_pbes_expression));

    if (++nr_of_equations % 1000 == 0)
      core::gsVerboseMsg("At equation %d\n", nr_of_equations);
  }

  core::gsVerboseMsg("Sorting result...\n");
  new_equation_system = sort_names(names_order, new_equation_system);

  // Rewrite initial state
  new_initial_state = propositional_variable_instantiation(create_propvar_name(initial_state.name(), initial_state.parameters()), empty_data_expression_list);

  pbes<> result = pbes<>(data, new_equation_system, new_initial_state);

  return result;
}

//function do_finite_algorithm
//---------------------------
/// \brief Create a PBES without finite data sorts, using the finite approach
inline
pbes<> do_finite_algorithm(pbes<> pbes_spec)
{
  // Instantiate free variables in the system
  if (!pbes_spec.instantiate_free_variables())
  {
    core::gsErrorMsg("Instantiatiation of free variables failed!\n");
    throw std::runtime_error("exit!");
  }
  core::gsVerboseMsg("Using finite approach...\n");

  propositional_variable_instantiation initial_state = pbes_spec.initial_state();
  atermpp::vector<pbes_equation> eqsys = pbes_spec.equations();
  data_specification data = pbes_spec.data();

  atermpp::vector<pbes_equation> result_eqsys;        // resulting equation system
  int nr_of_equations = 0;          // Nr of equations computed
  Rewriter *rewriter = createRewriter(data);  // Data rewriter

  // Empty data::data_variable_list and data::data_expression_list
  data::data_variable_list empty_data_variable_list;
  data::data_expression_list empty_data_expression_list;

  atermpp::table sort_enumerations(10,50);

  //Populate sort_enumerations with all enumerations for the finite sorts of the system
  core::gsVerboseMsg("Enumerating finite data sorts...\n");
  for (atermpp::vector<pbes_equation>::iterator eq_i = eqsys.begin(); eq_i != eqsys.end(); eq_i++)
  {
    data::data_variable_list parameters = eq_i->variable().parameters();
    for (data::data_variable_list::iterator p = parameters.begin(); p != parameters.end(); p++)
    {
      sort_expression current_sort = p->sort();
      if (sort_enumerations.get(current_sort) == NULL)
      {
        if (check_finite(data.constructors(), current_sort))
        {
          data::data_expression_list enumerations_from_sort = enumerate_constructors(data.constructors(), current_sort);
          sort_enumerations.put(current_sort, enumerations_from_sort);
        }
      }
    }
  }

  core::gsVerboseMsg("Computing PBES without finite data sorts...\n");
  for (atermpp::vector<pbes_equation>::iterator eq_i = eqsys.begin(); eq_i != eqsys.end(); eq_i++)
  {
    pbes_equation equation = *eq_i;

    propositional_variable propvar = equation.variable();
    pbes_expression formula = equation.formula();

    core::identifier_string propvar_name = propvar.name();
    data::data_variable_list propvar_parameters = propvar.parameters();


    atermpp::vector< t_instantiations > instantiation_list;
    t_instantiations current_values;    // Current results

    // Add empty instantiation to the list
    instantiation_list.push_back(current_values);

    std::string propvar_name_string = propvar_name;
    core::gsVerboseMsg("Creating all possible instantiations for propositional variable %s...\n", propvar_name_string.c_str());

    for (data::data_variable_list::iterator p = propvar_parameters.begin(); p != propvar_parameters.end(); p++)
    {
      atermpp::vector< t_instantiations > intermediate_instantiation_list;
      if (sort_enumerations.get(p->sort()) == NULL)
      { // The sort is infinite
        for (atermpp::vector< t_instantiations >::iterator inst_i = instantiation_list.begin(); inst_i != instantiation_list.end(); inst_i++)
        {
          current_values = *inst_i;
          current_values.infinite_var = push_back(current_values.infinite_var, *p);

          intermediate_instantiation_list.push_back(current_values);
        }
      }
      else
      { // The sort is finite
        current_values.finite_var = push_back(current_values.finite_var, *p);
        data::data_expression_list enumerations = sort_enumerations.get(p->sort());

        for (atermpp::vector< t_instantiations >::iterator inst_i = instantiation_list.begin(); inst_i != instantiation_list.end(); inst_i++)
        {
          for (data::data_expression_list::iterator e = enumerations.begin(); e != enumerations.end(); e++)
          {
            current_values.finite_exp = push_back(inst_i->finite_exp, *e);
            intermediate_instantiation_list.push_back(current_values);
          }
        }
      }
      instantiation_list = intermediate_instantiation_list;
    }

    core::gsVerboseMsg("Computing Boolean equations for each instantiation of propositional variable %s...\n", propvar_name_string.c_str());

    for (atermpp::vector< t_instantiations >::iterator inst_i = instantiation_list.begin(); inst_i != instantiation_list.end(); inst_i++)
    {
      propositional_variable propvar_current = propositional_variable(create_propvar_name(propvar_name, inst_i->finite_exp), inst_i->infinite_var);

      pbes_expression current_expression; // Current expression

      // Substitute all instantiated variables and rewrite the rhs as far as possible.
      current_expression = data::replace_data_variable_sequence(formula, inst_i->finite_var, inst_i->finite_exp);
      current_expression = pbes_expression_rewrite(current_expression, data, rewriter);

      propositional_variable_instantiation_list oldpropvarinst_list;
      propositional_variable_instantiation_list newpropvarinst_list;

      // Get all propvarinst of the rhs
      std::set< propositional_variable_instantiation > propvarinst_set = find_propositional_variable_instantiations(current_expression);

      for (std::set< propositional_variable_instantiation >::iterator pvi = propvarinst_set.begin(); pvi != propvarinst_set.end(); pvi++)
      {
        oldpropvarinst_list = push_front(oldpropvarinst_list, *pvi);
        propositional_variable_instantiation newpropvarinst = create_naive_propositional_variable_instantiation(*pvi, &sort_enumerations);
        newpropvarinst_list = push_front(newpropvarinst_list, newpropvarinst);
      }

      current_expression = replace_propositional_variable_sequence(current_expression, oldpropvarinst_list, newpropvarinst_list);
      result_eqsys.push_back(pbes_equation(eq_i->symbol(), propvar_current, current_expression));

      if (++nr_of_equations % 1000 == 0)
        core::gsVerboseMsg("At Boolean equation %d\n", nr_of_equations);

    }
  }

  core::gsVerboseMsg("Instantiation process finished.\nNumber of Boolean equations computed: %d\n", nr_of_equations);

  // rewrite initial state
  propositional_variable_instantiation new_initial_state = create_naive_propositional_variable_instantiation(initial_state, &sort_enumerations);

  pbes<> result = pbes<>(data, result_eqsys, new_initial_state);

  delete rewriter;

  return result;
}

} // namespace pbes_system

} // namespace mcrl2

// Specify how the ATerms in t_instantiations need to be protected using a traits class
namespace atermpp {
  using mcrl2::pbes_system::t_instantiations;

  template<>
  struct aterm_traits<t_instantiations>
  {
    static void protect(t_instantiations t) { t.protect(); }
    static void unprotect(t_instantiations t) { t.unprotect(); }
    static void mark(t_instantiations t) { t.mark(); }
  };
} // namespace atermpp

#endif // MCRL2_PBES_PBES2BES_H
