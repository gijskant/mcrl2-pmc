// Author(s): Gijs Kant
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/mes2pbes.h
/// \brief Transform modal equation systems to PBES.

#ifndef MCRL2_PBES_MES2PBES_H_
#define MCRL2_PBES_MES2PBES_H_

#include "mcrl2/utilities/logger.h"

namespace mcrl2 {

namespace state_formulas {

pbes_system::pbes mes2pbes(const data::data_specification& data, const modal_equation_system& x);

pbes_system::pbes mes2pbes_approximate(const data::data_specification& data, const modal_equation_system& x);

// \brief Visitor for transforming a modal equation system to a PBES.
struct mes2pbes_builder
{
  data::data_specification m_data;
public:

  mes2pbes_builder(const data::data_specification& data) :
    m_data(data)
  { }

  pbes_system::pbes_expression operator()(const true_& x)
  {
    return pbes_system::true_();
  }

  pbes_system::pbes_expression operator()(const false_& x)
  {
    return pbes_system::false_();
  }

  pbes_system::pbes_expression operator()(const exists& x)
  {
    return pbes_system::exists(x.variables(), (*this)(x.body()));
  }

  pbes_system::pbes_expression operator()(const forall& x)
  {
    return pbes_system::forall(x.variables(), (*this)(x.body()));
  }

  pbes_system::pbes_expression operator()(const imp& x)
  {
    return pbes_system::imp((*this)(x.left()), (*this)(x.right()));
  }

  pbes_system::pbes_expression operator()(const or_& x)
  {
    return pbes_system::or_((*this)(x.left()), (*this)(x.right()));
  }

  pbes_system::pbes_expression operator()(const and_& x)
  {
    return pbes_system::and_((*this)(x.left()), (*this)(x.right()));
  }

  pbes_system::pbes_expression operator()(const not_& x)
  {
    return pbes_system::not_((*this)(x.operand()));
  }

  pbes_system::pbes_expression operator()(const variable& x)
  {
    return pbes_system::propositional_variable_instantiation(x.name(), x.arguments());
  }

  pbes_system::pbes_expression operator()(const state_formulas::state_formula& x)
  {
    pbes_system::pbes_expression result;
    if (data::is_data_expression(x))
    {
      result = atermpp::down_cast<data::data_expression>(x);
    }
    else if (state_formulas::is_true(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::true_>(x));
    }
    else if (state_formulas::is_false(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::false_>(x));
    }
    else if (state_formulas::is_not(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::not_>(x));
    }
    else if (state_formulas::is_and(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::and_>(x));
    }
    else if (state_formulas::is_or(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::or_>(x));
    }
    else if (state_formulas::is_imp(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::imp>(x));
    }
    else if (state_formulas::is_forall(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::forall>(x));
    }
    else if (state_formulas::is_exists(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::exists>(x));
    }
    else if (state_formulas::is_must(x))
    {
      throw std::runtime_error("No modal operator allowed in equations.");
    }
    else if (state_formulas::is_may(x))
    {
      throw std::runtime_error("No modal operator allowed in equations.");
    }
    else if (state_formulas::is_yaled(x))
    {
      throw std::runtime_error("No time operator allowed in equations.");
    }
    else if (state_formulas::is_yaled_timed(x))
    {
      throw std::runtime_error("No time operator allowed in equations.");
    }
    else if (state_formulas::is_delay(x))
    {
      throw std::runtime_error("No time operator allowed in equations.");
    }
    else if (state_formulas::is_delay_timed(x))
    {
      throw std::runtime_error("No time operator allowed in equations.");
    }
    else if (state_formulas::is_variable(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::variable>(x));
    }
    else if (state_formulas::is_nu(x))
    {
      throw std::runtime_error("No fixpoint operator allowed in equations.");
    }
    else if (state_formulas::is_mu(x))
    {
      throw std::runtime_error("No fixpoint operator allowed in equations.");
    }
    return result;
  }

  pbes_system::pbes operator()(const modal_equation_system& x)
  {
    std::vector<pbes_system::pbes_equation> equations;
    for(auto e : x.equations())
    {
      pbes_system::pbes_equation eq = (*this)(e);
      equations.push_back(eq);
    }
    return pbes_system::pbes(m_data, equations,
        atermpp::down_cast<pbes_system::propositional_variable_instantiation>((*this)(x.initial_state()))
        );
  }

  pbes_system::fixpoint_symbol operator()(const fixpoint_symbol& x)
  {
    return pbes_system::fixpoint_symbol(x);
  }

  pbes_system::pbes_equation operator()(const modal_equation& x)
  {
    return pbes_system::pbes_equation(
        (*this)(x.symbol()),
        pbes_system::propositional_variable(x.name(), x.parameters()),
        (*this)(x.formula())
    );
  }

};

pbes_system::pbes mes2pbes(const data::data_specification& data, const modal_equation_system& x)
{
  mes2pbes_builder f(data);
  return f(x);
}

// \brief Visitor for transforming a modal equation system to a PBES.
struct mes2pbes_approximation_builder
{
  data::data_specification m_data;
public:

  mes2pbes_approximation_builder(const data::data_specification& data) :
    m_data(data)
  { }

  pbes_system::pbes_expression operator()(const true_& x, bool t)
  {
    return t ? pbes_system::true_() : pbes_system::false_();
  }

  pbes_system::pbes_expression operator()(const false_& x, bool t)
  {
    return t ? pbes_system::false_() : pbes_system::true_();
  }

  pbes_system::pbes_expression operator()(const exists& x, bool t)
  {
    if (t)
    {
      return pbes_system::exists(x.variables(), (*this)(x.body(), t));
    }
    else
    {
      return pbes_system::forall(x.variables(), (*this)(x.body(), t));
    }
  }

  pbes_system::pbes_expression operator()(const forall& x, bool t)
  {
    if (t)
    {
      return pbes_system::forall(x.variables(), (*this)(x.body(), t));
    }
    else
    {
      return pbes_system::exists(x.variables(), (*this)(x.body(), t));
    }
  }

  // FIXME
  pbes_system::pbes_expression operator()(const imp& x, bool t)
  {
    if (t)
    {
      return pbes_system::imp((*this)(x.left(), t), (*this)(x.right(), t));
    }
    else
    {
      return pbes_system::and_((*this)(x.left(), !t), (*this)(x.right(), t));
    }
  }

  pbes_system::pbes_expression operator()(const or_& x, bool t)
  {
    if (t)
    {
      return pbes_system::or_((*this)(x.left(), t), (*this)(x.right(), t));
    }
    else
    {
      return pbes_system::and_((*this)(x.left(), t), (*this)(x.right(), t));
    }
  }

  pbes_system::pbes_expression operator()(const and_& x, bool t)
  {
    if (t)
    {
      return pbes_system::and_((*this)(x.left(), t), (*this)(x.right(), t));
    }
    else
    {
      return pbes_system::or_((*this)(x.left(), t), (*this)(x.right(), t));
    }
  }

  pbes_system::pbes_expression operator()(const not_& x, bool t)
  {
    return (*this)(x.operand(), !t);
  }

  pbes_system::pbes_expression operator()(const variable& x, bool t)
  {
    core::identifier_string name(((std::string)x.name()) + (t ? "_T" : "_F"));
    return pbes_system::propositional_variable_instantiation(name, x.arguments());
  }

  pbes_system::pbes_expression operator()(const must& x, bool t)
  {
    return t ? (*this)(x.operand(), t) : pbes_system::false_();
  }

  pbes_system::pbes_expression operator()(const may& x, bool t)
  {
    return t ? pbes_system::false_() : (*this)(x.operand(), t);
  }

  pbes_system::pbes_expression operator()(const state_formulas::state_formula& x, bool t)
  {
    pbes_system::pbes_expression result;
    if (data::is_data_expression(x))
    {
      result = atermpp::down_cast<data::data_expression>(x);
    }
    else if (state_formulas::is_true(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::true_>(x), t);
    }
    else if (state_formulas::is_false(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::false_>(x), t);
    }
    else if (state_formulas::is_not(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::not_>(x), t);
    }
    else if (state_formulas::is_and(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::and_>(x), t);
    }
    else if (state_formulas::is_or(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::or_>(x), t);
    }
    else if (state_formulas::is_imp(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::imp>(x), t);
    }
    else if (state_formulas::is_forall(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::forall>(x), t);
    }
    else if (state_formulas::is_exists(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::exists>(x), t);
    }
    else if (state_formulas::is_must(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::must>(x), t);
    }
    else if (state_formulas::is_may(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::may>(x), t);
    }
    else if (state_formulas::is_yaled(x))
    {
      throw std::runtime_error("No time operator allowed in equations.");
    }
    else if (state_formulas::is_yaled_timed(x))
    {
      throw std::runtime_error("No time operator allowed in equations.");
    }
    else if (state_formulas::is_delay(x))
    {
      throw std::runtime_error("No time operator allowed in equations.");
    }
    else if (state_formulas::is_delay_timed(x))
    {
      throw std::runtime_error("No time operator allowed in equations.");
    }
    else if (state_formulas::is_variable(x))
    {
      result = (*this)(atermpp::down_cast<state_formulas::variable>(x), t);
    }
    else if (state_formulas::is_nu(x))
    {
      throw std::runtime_error("No fixpoint operator allowed in equations.");
    }
    else if (state_formulas::is_mu(x))
    {
      throw std::runtime_error("No fixpoint operator allowed in equations.");
    }
    return result;
  }

  pbes_system::pbes operator()(const modal_equation_system& x, bool t)
  {
    std::vector<pbes_system::pbes_equation> equations;
    for(auto e : x.equations())
    {
      pbes_system::pbes_equation eq = (*this)(e, true);
      equations.push_back(eq);
    }
    for(auto e : x.equations())
    {
      pbes_system::pbes_equation eq = (*this)(e, false);
      equations.push_back(eq);
    }
    return pbes_system::pbes(m_data, equations,
        atermpp::down_cast<pbes_system::propositional_variable_instantiation>((*this)(x.initial_state(), t))
        );
  }

  pbes_system::fixpoint_symbol operator()(const fixpoint_symbol& x, bool t)
  {
    if (t)
    {
      return pbes_system::fixpoint_symbol(x);
    }
    else
    {

      return pbes_system::fixpoint_symbol((x.is_nu()) ? fixpoint_symbol::mu() : fixpoint_symbol::nu());
    }
  }

  pbes_system::pbes_equation operator()(const modal_equation& x, bool t)
  {
    core::identifier_string name(((std::string)x.name()) + (t ? "_T" : "_F"));
    return pbes_system::pbes_equation(
        (*this)(x.symbol(), t),
        pbes_system::propositional_variable(name, x.parameters()),
        (*this)(x.formula(), t)
    );
  }

};

pbes_system::pbes mes2pbes_approximate(const data::data_specification& data, const modal_equation_system& x, bool underapproximate)
{
  mes2pbes_approximation_builder f(data);
  return f(x, underapproximate);
}

} // namespace state_formulas

} // namespace mcrl2

#endif /* MCRL2_PBES_MES2PBES_H_ */
