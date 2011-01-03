// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/state_formula_normalize.h
/// \brief Add your file description here.

#ifndef MCRL2_MODAL_STATE_FORMULA_NORMALIZE_H
#define MCRL2_MODAL_STATE_FORMULA_NORMALIZE_H

#include "mcrl2/modal_formula/state_formula.h"
#include "mcrl2/modal_formula/detail/state_variable_negator.h"
#include "mcrl2/data/bool.h"

namespace mcrl2 {

namespace state_formulas {

/// \brief Brings a state formula into positive normal form,
/// i.e. a formula without any occurrences of ! or =>.
/// \param f A modal formula
/// \return The normalized formula
inline
state_formula normalize(state_formula f)
{
  using namespace accessors;
  
  if (is_not(f))
  {
    f = arg(f); // remove the not
    if (data::is_data_expression(f)) {
      return data::sort_bool::not_(f);
    } else if (is_true(f)) {
      return false_();
    } else if (is_false(f)) {
      return true_();
    } else if (is_not(f)) {
      return normalize(arg(f));
    } else if (is_and(f)) {
      return or_(normalize(not_(left(f))), normalize(not_(right(f))));
    } else if (is_or(f)) {
      return and_(normalize(not_(left(f))), normalize(not_(right(f))));
    } else if (is_imp(f)) {
      return and_(normalize(left(f)), normalize(not_(right(f))));
    } else if (is_forall(f)) {
      return exists(var(f), normalize(not_(arg(f))));
    } else if (is_exists(f)) {
      return forall(var(f), normalize(not_(arg(f))));
    } else if (is_must(f)) {
      return may(act(f), normalize(not_(arg(f))));
    } else if (is_may(f)) {
      return must(act(f), normalize(not_(arg(f))));
    } else if (is_yaled_timed(f)) {
      return delay_timed(time(f));
    } else if (is_yaled(f)) {
      return delay();
    } else if (is_delay_timed(f)) {
      return yaled_timed(time(f));
    } else if (is_delay(f)) {
      return yaled();
    } else if (is_variable(f)) {
      throw mcrl2::runtime_error(std::string("normalize error: illegal argument ") + f.to_string());
    } else if (is_mu(f)) {
      return nu(name(f), ass(f), arg(normalize(not_(detail::negate_propositional_variable(name(f), f)))));
    } else if (is_nu(f)) {
      return mu(name(f), ass(f), arg(normalize(not_(detail::negate_propositional_variable(name(f), f)))));
    }
  }
  else // !is_not(f)
  {
    if (data::is_data_expression(f)) {
      return f;
    } else if (is_true(f)) {
      return f;
    } else if (is_false(f)) {
      return f;
    //} else if (is_not(f)) {
    // ;
    } else if (is_and(f)) {
      return and_(normalize(left(f)), normalize(right(f)));
    } else if (is_or(f)) {
      return or_(normalize(left(f)), normalize(right(f)));
    } else if (is_imp(f)) {
      return or_(normalize(left(f)), normalize(not_(right(f))));
    } else if (is_forall(f)) {
      return forall(var(f), normalize(arg(f)));
    } else if (is_exists(f)) {
      return exists(var(f), normalize(arg(f)));
    } else if (is_must(f)) {
      return must(act(f), normalize(arg(f)));
    } else if (is_may(f)) {
      return may(act(f), normalize(arg(f)));
    } else if (is_yaled_timed(f)) {
      return f;
    } else if (is_yaled(f)) {
      return f;
    } else if (is_delay_timed(f)) {
      return f;
    } else if (is_delay(f)) {
      return f;
    } else if (is_variable(f)) {
      return f;
    } else if (is_mu(f)) {
      return mu(name(f), ass(f), normalize(arg(f)));
    } else if (is_nu(f)) {
      return nu(name(f), ass(f), normalize(arg(f)));
    }
  }
  throw mcrl2::runtime_error(std::string("normalize error: unknown argument ") + f.to_string());
  return state_formula();
}

} // namespace state_formulas

} // namespace mcrl2

#endif // MCRL2_MODAL_STATE_FORMULA_NORMALIZE_H
