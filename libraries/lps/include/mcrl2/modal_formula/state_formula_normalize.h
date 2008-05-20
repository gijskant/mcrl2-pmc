// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING).
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/state_formula_normalize.h
/// \brief Add your file description here.

#ifndef MCRL2_MODAL_STATE_FORMULA_NORMALIZE_H
#define MCRL2_MODAL_STATE_FORMULA_NORMALIZE_H

#include "mcrl2/modal_formula/state_formula_builder.h"

namespace mcrl2 {

namespace modal {

/// \internal
/// Puts a logical not around state variables named X
struct state_variable_negation
{
  core::identifier_string X;
  
  state_variable_negation(core::identifier_string X_)
    : X(X_)
  {}
  
  aterm_appl operator()(aterm_appl t) const
  {
    using namespace modal::state_frm;

    if (is_var(t) && (name(t) == X))
    {
      return not_(t);
    }
    else
    {
      return t;
    }
  }
};

/// \internal
/// The function normalize brings a state formula into positive normal form,
/// i.e. a formula without any occurrences of ! or =>.
inline
state_formula normalize(state_formula f)
{
  using namespace modal::state_frm;

  if (is_not(f))
  {
    f = arg(f); // remove the not
    if (is_data(f)) {
      return data::data_expr::not_(f);
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
    } else if (is_var(f)) {
      throw std::runtime_error(std::string("normalize error: illegal argument ") + f.to_string());
    } else if (is_mu(f)) {
      return nu(name(f), ass(f), arg(normalize(not_(f.substitute(state_variable_negation(name(f)))))));
    } else if (is_nu(f)) {
      return mu(name(f), ass(f), arg(normalize(not_(f.substitute(state_variable_negation(name(f)))))));
    }
  }
  else // !is_not(f)
  {
    if (is_data(f)) {
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
    } else if (is_var(f)) {
      return f;
    } else if (is_mu(f)) {
      return mu(name(f), ass(f), normalize(arg(f)));
    } else if (is_nu(f)) {
      return nu(name(f), ass(f), normalize(arg(f)));
    }
  }
  throw std::runtime_error(std::string("normalize error: unknown argument ") + f.to_string());
  return state_formula();
}

} // namespace modal

} // namespace mcrl2

#endif // MCRL2_MODAL_STATE_FORMULA_NORMALIZE_H
