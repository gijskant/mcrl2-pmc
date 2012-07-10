// #define MCRL2_PBES_TRANSLATE_DEBUG
// #define MCRL2_EQUAL_MULTI_ACTIONS_DEBUG

// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/pbes_translate.h
/// \brief The pbes_translate algorithm.

#ifndef MCRL2_PBES_PBES_TRANSLATE_H
#define MCRL2_PBES_PBES_TRANSLATE_H

#include <iterator>
#include <sstream>
#include <algorithm>
#include "mcrl2/atermpp/detail/aterm_list_utility.h"
#include "mcrl2/core/detail/print_utility.h"
#include "mcrl2/data/standard_utility.h"
#include "mcrl2/data/detail/data_utility.h"
#include "mcrl2/data/detail/find.h"
#include "mcrl2/data/replace.h"
#include "mcrl2/data/set_identifier_generator.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/lps/detail/make_timed_lps.h"
#include "mcrl2/lps/replace.h"
#include "mcrl2/modal_formula/replace.h"
#include "mcrl2/modal_formula/monotonicity.h"
#include "mcrl2/modal_formula/state_formula.h"
#include "mcrl2/modal_formula/state_formula_rename.h"
#include "mcrl2/modal_formula/traverser.h"
#include "mcrl2/modal_formula/preprocess_state_formula.h"
#include "mcrl2/modal_formula/detail/state_variable_negator.h"
#include "mcrl2/modal_formula/detail/action_formula_accessors.h"
#include "mcrl2/modal_formula/detail/state_formula_accessors.h"
#include "mcrl2/pbes/monotonicity.h"
#include "mcrl2/pbes/normalize.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/replace.h"
#include "mcrl2/pbes/detail/pbes_translate_impl.h"
#include "mcrl2/pbes/detail/lps2pbes_indenter.h"

namespace mcrl2
{

namespace pbes_system
{

/// \cond INTERNAL_DOCS
//
/// \brief Concatenates two sequences of PBES equations
/// \param p A sequence of PBES equations
/// \param q A sequence of PBES equations
/// \return The concatenation result
inline
atermpp::vector<pbes_equation> operator+(const atermpp::vector<pbes_equation>& p, const atermpp::vector<pbes_equation>& q)
{
  atermpp::vector<pbes_equation> result(p);
  result.insert(result.end(), q.begin(), q.end());
  return result;
}
/// \endcond

/// \cond INTERNAL_DOCS
//
/// \brief Appends a PBES equation to a sequence of PBES equations
/// \param p A sequence of PBES equations
/// \param e A PBES equation
/// \return The append result
inline
atermpp::vector<pbes_equation> operator+(const atermpp::vector<pbes_equation>& p, const pbes_equation& e)
{
  atermpp::vector<pbes_equation> result(p);
  result.push_back(e);
  return result;
}
/// \endcond

/// \cond INTERNAL_DOCS
namespace detail
{

inline
std::string myprint(const atermpp::vector<pbes_equation>& v)
{
  std::ostringstream out;
  out << "[";
  for (atermpp::vector<pbes_equation>::const_iterator i = v.begin(); i != v.end(); ++i)
  {
    out << "\n  " << pbes_system::pp(i->symbol()) << " " << pbes_system::pp(i->variable()) << " = " << pbes_system::pp(i->formula());
  }
  out << "\n]";
  return out.str();
}

/// \brief Generates fresh variables with names that do not appear in the given context.
/// Caveat: the implementation is very inefficient.
/// \param update_context If true, then generated names are added to the context
inline
data::variable_list make_fresh_variables(const data::variable_list& variables, data::set_identifier_generator& id_generator, bool add_to_context = true)
{
  data::variable_vector result;
  for (data::variable_list::const_iterator i = variables.begin(); i != variables.end(); ++i)
  {
    core::identifier_string name =  id_generator(std::string(i->name()));
    result.push_back(data::variable(name, i->sort()));
    if (!add_to_context)
    {
      id_generator.remove_identifier(name);
    }
  }
  return atermpp::convert<data::variable_list>(result);
}

data::variable_list Par(const core::identifier_string& X, const data::variable_list& l, const state_formulas::state_formula& x);

struct par_traverser: public state_formulas::state_formula_traverser<par_traverser>
{
  typedef state_formulas::state_formula_traverser<par_traverser> super;
  using super::enter;
  using super::leave;
  using super::operator();

#if BOOST_MSVC
#include "mcrl2/core/detail/traverser_msvc.inc.h"
#endif

  const core::identifier_string& X;
  const data::variable_list& l;
  atermpp::vector<data::variable_list> result_stack;

  par_traverser(const core::identifier_string& X_, const data::variable_list& l_)
    : X(X_), l(l_)
  {}

  void push(const data::variable_list& x)
  {
    result_stack.push_back(x);
  }

  const data::variable_list& top() const
  {
    return result_stack.back();
  }

  data::variable_list pop()
  {
    data::variable_list result = top();
    result_stack.pop_back();
    return result;
  }

  // join the two topmost elements on the stack
  void join()
  {
    data::variable_list right = pop();
    data::variable_list left = pop();
    push(left + right);
  }

  void leave(const data::data_expression& x)
  {
    push(data::variable_list());
  }

  void leave(const state_formulas::true_& x)
  {
    push(data::variable_list());
  }

  void leave(const state_formulas::false_& x)
  {
    push(data::variable_list());
  }

  void leave(const state_formulas::not_& x)
  {
    // skip
  }

  void leave(const state_formulas::and_& x)
  {
    join();
  }

  void leave(const state_formulas::or_& x)
  {
    join();
  }

  void leave(const state_formulas::imp& x)
  {
    join();
  }

  void operator()(const state_formulas::forall& x)
  {
    push(Par(X, l + x.variables(), x.body()));
  }

  void operator()(const state_formulas::exists& x)
  {
    push(Par(X, l + x.variables(), x.body()));
  }

  void leave(const state_formulas::must& x)
  {
    // skip
  }

  void leave(const state_formulas::may& x)
  {
    // skip
  }

  void leave(const state_formulas::yaled& x)
  {
    push(data::variable_list());
  }

  void leave(const state_formulas::yaled_timed& x)
  {
    push(data::variable_list());
  }

  void leave(const state_formulas::delay& x)
  {
    push(data::variable_list());
  }

  void leave(const state_formulas::delay_timed& x)
  {
    push(data::variable_list());
  }

  void leave(const state_formulas::variable& x)
  {
    push(data::variable_list());
  }

  void operator()(const state_formulas::nu& x)
  {
    if (x.name() == X)
    {
      push(l);
    }
    else
    {
      push(Par(X, l + data::left_hand_sides(x.assignments()), x.operand()));
    }
  }

  void operator()(const state_formulas::mu& x)
  {
    if (x.name() == X)
    {
      push(l);
    }
    else
    {
      push(Par(X, l + data::left_hand_sides(x.assignments()), x.operand()));
    }
  }
};

inline
data::variable_list Par(const core::identifier_string& X, const data::variable_list& l, const state_formulas::state_formula& x)
{
  par_traverser f(X, l);
  f(x);
  return f.top();
}

pbes_expression Sat(const lps::multi_action& a, const action_formulas::action_formula& x);

template <typename Derived>
struct sat_traverser: public action_formulas::action_formula_traverser<Derived>
{
  typedef action_formulas::action_formula_traverser<Derived> super;
  using super::enter;
  using super::leave;
  using super::operator();

#if BOOST_MSVC
#include "mcrl2/core/detail/traverser_msvc.inc.h"
#endif

  const lps::multi_action& a;
  atermpp::vector<pbes_expression> result_stack;

  sat_traverser(const lps::multi_action& a_)
    : a(a_)
  {}

  Derived& derived()
  {
    return static_cast<Derived&>(*this);
  }

  void push(const pbes_expression& x)
  {
    result_stack.push_back(x);
  }

  const pbes_expression& top() const
  {
    return result_stack.back();
  }

  pbes_expression pop()
  {
    pbes_expression result = top();
    result_stack.pop_back();
    return result;
  }

  void leave(const data::data_expression& x)
  {
    push(x);
  }

  void leave(const lps::multi_action& x)
  {
    push(lps::equal_multi_actions(a, x));
  }

  void leave(const action_formulas::true_& x)
  {
    push(true_());
  }

  void leave(const action_formulas::false_& x)
  {
    push(false_());
  }

  void operator()(const action_formulas::not_& x)
  {
    push(not_(Sat(a, x.operand())));
  }

  void leave(const action_formulas::and_& x)
  {
    pbes_expression right = pop();
    pbes_expression left = pop();
    push(and_(left, right));
  }

  void leave(const action_formulas::or_& x)
  {
    pbes_expression right = pop();
    pbes_expression left = pop();
    push(or_(left, right));
  }

  void leave(const action_formulas::imp& x)
  {
    pbes_expression right = pop();
    pbes_expression left = pop();
    push(imp(left, right));
  }

  void operator()(const action_formulas::forall& x)
  {
    data::set_identifier_generator id_generator;
    id_generator.add_identifiers(data::detail::variable_names(lps::find_variables(a)));
    id_generator.add_identifiers(data::detail::variable_names(action_formulas::find_variables(x)));
    data::variable_list y = pbes_system::detail::make_fresh_variables(x.variables(), id_generator, false);
    action_formulas::action_formula alpha = x.body();
    push(forall(y, Sat(a, action_formulas::replace_free_variables(alpha, data::make_sequence_sequence_substitution(x.variables(), y)))));
  }

  void operator()(const action_formulas::exists& x)
  {
    data::set_identifier_generator id_generator;
    id_generator.add_identifiers(data::detail::variable_names(lps::find_variables(a)));
    id_generator.add_identifiers(data::detail::variable_names(action_formulas::find_variables(x)));
    data::variable_list y = pbes_system::detail::make_fresh_variables(x.variables(), id_generator, false);
    action_formulas::action_formula alpha = x.body();
    push(exists(y, Sat(a, action_formulas::replace_free_variables(alpha, data::make_sequence_sequence_substitution(x.variables(), y)))));
  }

  void operator()(const action_formulas::at& x)
  {
    data::data_expression t = a.time();
    action_formulas::action_formula alpha = x.operand();
    data::data_expression u = x.time_stamp();
    push(and_(Sat(a, alpha), data::equal_to(t, u)));
  }
};

template <template <class> class Traverser>
struct apply_sat_traverser: public Traverser<apply_sat_traverser<Traverser> >
{
  typedef Traverser<apply_sat_traverser<Traverser> > super;
  using super::enter;
  using super::leave;
  using super::operator();
  using super::top;

  apply_sat_traverser(const lps::multi_action& a)
    : super(a)
  {}

#ifdef BOOST_MSVC
#include "mcrl2/core/detail/traverser_msvc.inc.h"
#endif
};

inline
pbes_expression Sat(const lps::multi_action& a, const action_formulas::action_formula& x)
{
  apply_sat_traverser<sat_traverser> f(a);
  f(x);
  return f.top();
}

} // namespace detail
/// \endcond

/// \brief Abstract algorithm class for translating a state formula and a specification to a pbes.
class pbes_translate_algorithm
{
  protected:
    /// \brief The Par function of the translation
    /// \param x A
    /// \param l A sequence of data variables
    /// \param f A modal formula
    /// \return The function result
    data::variable_list Par(const core::identifier_string& x, const data::variable_list& l, const state_formulas::state_formula& f)
    {
#ifdef MCRL2_PBES_TRANSLATE_PAR_DEBUG
      std::cerr << "\n<Par>(" << core::pp(x) << ", " << data::pp(l) << ", " << state_formulas::pp(f) << ") = " << data::pp(result) << std::endl;
#endif
      return detail::Par(x, l, f);
    }

  public:
    /// \brief Constructor.
    pbes_translate_algorithm()
    {}
};

/// \brief Algorithm for translating a state formula and a timed specification to a pbes.
class pbes_translate_algorithm_timed: public pbes_translate_algorithm
{
  protected:

    /// \brief The \p sat_top function of the translation
    /// \param a A timed multi-action
    /// \param b An action formula
    /// \return The function result
    pbes_expression sat_top(const lps::multi_action& x, const action_formulas::action_formula& b)
    {
      return detail::Sat(x, b);
    }

    /// \brief The \p RHS function of the translation
    /// \param f0 A modal formula
    /// \param f A modal formula
    /// \param lps A linear process
    /// \param T A data variable
    /// \param id_generator A set of strings that may not be used for naming a fresh variable
    /// \return The function result
    pbes_expression RHS(
      state_formulas::state_formula f0,
      state_formulas::state_formula f,
      const lps::linear_process& lps,
      data::variable T,
      data::set_identifier_generator& id_generator
     )
    {
#ifdef MCRL2_PBES_TRANSLATE_DEBUG
      std::cerr << "\n" << lps2pbes_indent() << "<RHS timed>" << state_formulas::pp(f) << std::flush;
      lps2pbes_increase_indent();
#endif
      namespace z = pbes_expr_optimized;
      using namespace pbes_system::accessors;
      namespace s = state_formulas;
      namespace a = state_formulas::detail::accessors;
      namespace d = data;
      using atermpp::detail::operator+;

      pbes_expression result;

      if (!s::is_not(f))
      {
        if (data::is_data_expression(f))
        {
          result = pbes_expression(f);
        }
        else if (s::is_true(f))
        {
          result = z::true_();
        }
        else if (s::is_false(f))
        {
          result = z::false_();
        }
        else if (s::is_and(f))
        {
          result = z::and_(RHS(f0, a::left(f), lps, T, id_generator), RHS(f0, a::right(f), lps, T, id_generator));
        }
        else if (s::is_or(f))
        {
          result = z::or_(RHS(f0, a::left(f), lps, T, id_generator), RHS(f0, a::right(f), lps, T, id_generator));
        }
        else if (s::is_imp(f))
        {
          // TODO: generalize
          // result = imp(RHS(f0, a::left(f), lps, T, id_generator), RHS(f0, a::right(f), lps, T, id_generator));
          result = z::or_(RHS(f0, s::not_(a::left(f)), lps, T, id_generator), RHS(f0, a::right(f), lps, T, id_generator));
        }
        else if (s::is_forall(f))
        {
          id_generator.add_identifiers(data::detail::variable_names(data::find_variables(a::var(f))));
          result = pbes_expr::forall(a::var(f), RHS(f0, a::arg(f), lps, T, id_generator));
        }
        else if (s::is_exists(f))
        {
          id_generator.add_identifiers(data::detail::variable_names(data::find_variables(a::var(f))));
          result = pbes_expr::exists(a::var(f), RHS(f0, a::arg(f), lps, T, id_generator));
        }
        else if (s::is_must(f))
        {
          atermpp::vector<pbes_expression> v;
          action_formulas::action_formula alpha = a::act(f);
          state_formulas::state_formula phi = a::arg(f);
          const lps::action_summand_vector& asv = lps.action_summands();
          for (lps::action_summand_vector::const_iterator i = asv.begin(); i != asv.end(); ++i)
          {
            data::data_expression ci(i->condition());
            lps::multi_action ai = i->multi_action();
            data::assignment_list gi = i->assignments();
            //data::variable_list xp(lps.process_parameters());
            data::variable_list yi(i->summation_variables());

            pbes_expression rhs = RHS(f0, phi, lps, T, id_generator);
//std::cout << "\n" << core::detail::print_set(id_generator, "id_generator") << std::endl;
            data::variable_list y = pbes_system::detail::make_fresh_variables(yi, id_generator);
//std::cout << "\n" << core::detail::print_list(yi, data::stream_printer(), "yi") << std::endl;
//std::cout << "\n" << core::detail::print_list(y, data::stream_printer(), "y") << std::endl;
            ci = data::replace_free_variables(ci, data::make_sequence_sequence_substitution(yi, y));
            lps::replace_free_variables(ai, data::make_sequence_sequence_substitution(yi, y));
            gi = data::replace_free_variables(gi, data::make_sequence_sequence_substitution(yi, y));
            data::data_expression ti = ai.time();

            pbes_expression p1 = sat_top(ai, alpha);
            pbes_expression p2 = ci;
            pbes_expression p3 = d::greater(ti, T);

            // N.B. The order of these two substitutions is important!
            rhs = pbes_system::replace_free_variables(rhs, data::assignment_sequence_substitution(gi));
            rhs = pbes_system::replace_free_variables(rhs, data::assignment(T, ti));

            pbes_expression p = pbes_expr::forall(y, imp(and_(and_(p1, p2), p3), rhs));
            v.push_back(p);
          }
          result = z::join_and(v.begin(), v.end());
        }
        else if (s::is_may(f))
        {
          atermpp::vector<pbes_expression> v;
          action_formulas::action_formula alpha = a::act(f);
          state_formulas::state_formula phi = a::arg(f);
          const lps::action_summand_vector& asv=lps.action_summands();
          for (lps::action_summand_vector::const_iterator i = asv.begin(); i != asv.end(); ++i)
          {
            data::data_expression ci(i->condition());
            lps::multi_action ai = i->multi_action();
            data::assignment_list gi = i->assignments();
            //data::variable_list xp(lps.process_parameters());
            data::variable_list yi(i->summation_variables());

            pbes_expression rhs = RHS(f0, phi, lps, T, id_generator);
            data::variable_list y = pbes_system::detail::make_fresh_variables(yi, id_generator);
            ci = data::replace_free_variables(ci, data::make_sequence_sequence_substitution(yi, y));
            lps::replace_free_variables(ai, data::make_sequence_sequence_substitution(yi, y));
            gi = data::replace_free_variables(gi, data::make_sequence_sequence_substitution(yi, y));
            data::data_expression ti = ai.time();

            pbes_expression p1 = sat_top(ai, alpha);
            pbes_expression p2 = ci;
            pbes_expression p3 = d::greater(ti, T);

            // N.B. The order of these two substitutions is important!
            rhs = pbes_system::replace_free_variables(rhs, data::assignment_sequence_substitution(gi));
            rhs = pbes_system::replace_free_variables(rhs, data::assignment(T, ti));

            pbes_expression p = pbes_expr::exists(y, and_(and_(and_(p1, p2), p3), rhs));
            v.push_back(p);
          }
          result = z::join_or(v.begin(), v.end());
        }
        else if (s::is_delay_timed(f))
        {
          data::data_expression t = a::time(f);
          atermpp::vector<pbes_expression> v;
          const lps::action_summand_vector& asv=lps.action_summands();
          for (lps::action_summand_vector::const_iterator i = asv.begin(); i != asv.end(); ++i)
          {
            data::data_expression ck(i->condition());
            data::data_expression tk(i->multi_action().time());
            data::variable_list yk = i->summation_variables();
            pbes_expression p = pbes_expr::exists(yk, and_(ck, d::less_equal(t, tk)));
            v.push_back(p);
          }
          const lps::deadlock_summand_vector& dsv=lps.deadlock_summands();
          for (lps::deadlock_summand_vector::const_iterator i = dsv.begin(); i != dsv.end(); ++i)
          {
            data::data_expression ck(i->condition());
            data::data_expression tk(i->deadlock().time());
            data::variable_list yk = i->summation_variables();
            pbes_expression p = pbes_expr::exists(yk, and_(ck, d::less_equal(t, tk)));
            v.push_back(p);
          }
          result = z::or_(z::join_or(v.begin(), v.end()), d::less_equal(t, T));
        }
        else if (s::is_yaled_timed(f))
        {
          data::data_expression t = a::time(f);
          atermpp::vector<pbes_expression> v;
          const lps::action_summand_vector& asv=lps.action_summands();
          for (lps::action_summand_vector::const_iterator i = asv.begin(); i != asv.end(); ++i)
          {
            data::data_expression ck(i->condition());
            data::data_expression tk(i->multi_action().time());
            data::variable_list yk = i->summation_variables();
            pbes_expression p = pbes_expr::forall(yk, or_(data::sort_bool::not_(ck), d::greater(t, tk)));
            v.push_back(p);
          }
          const lps::deadlock_summand_vector& dsv=lps.deadlock_summands();
          for (lps::deadlock_summand_vector::const_iterator i = dsv.begin(); i != dsv.end(); ++i)
          {
            data::data_expression ck(i->condition());
            data::data_expression tk(i->deadlock().time());
            data::variable_list yk = i->summation_variables();
            pbes_expression p = pbes_expr::forall(yk, or_(data::sort_bool::not_(ck), d::greater(t, tk)));
            v.push_back(p);
          }
          result = z::and_(z::join_or(v.begin(), v.end()), d::greater(t, T));
        }
        else if (s::is_variable(f))
        {
          core::identifier_string X = a::name(f);
          data::data_expression_list d = a::param(f);
          data::variable_list xp = lps.process_parameters();
          result = propositional_variable_instantiation(X, T + d + xp + Par(X, data::variable_list(), f0));
        }
        else if (s::is_mu(f) || (s::is_nu(f)))
        {
          core::identifier_string X = a::name(f);
          data::data_expression_list d = detail::mu_expressions(f);
          data::variable_list xp = lps.process_parameters();
          result = propositional_variable_instantiation(X, T + d + xp + Par(X, data::variable_list(), f0));
        }
        else
        {
          throw mcrl2::runtime_error(std::string("RHS[timed] error: unknown state formula ") + state_formulas::pp(f));
        }
      }
      else // the formula is a negation
      {
        f = a::arg(f);
        if (data::is_data_expression(f))
        {
          result = pbes_expression(data::sort_bool::not_(f));
        }
        else if (s::is_true(f))
        {
          result = z::false_();
        }
        else if (s::is_false(f))
        {
          result = z::true_();
        }
        else if (s::is_not(f))
        {
          result = RHS(f0, a::arg(f), lps, T, id_generator);
        }
        else if (s::is_and(f))
        {
          result = z::or_(RHS(f0, s::not_(a::left(f)), lps, T, id_generator), RHS(f0, s::not_(a::right(f)), lps, T, id_generator));
        }
        else if (s::is_or(f))
        {
          result = z::and_(RHS(f0, s::not_(a::left(f)), lps, T, id_generator), RHS(f0, s::not_(a::right(f)), lps, T, id_generator));
        }
        else if (s::is_imp(f))
        {
          result = z::and_(RHS(f0, a::left(f), lps, T, id_generator), RHS(f0, s::not_(a::right(f)), lps, T, id_generator));
        }
        else if (s::is_forall(f))
        {
          id_generator.add_identifiers(data::detail::variable_names(data::find_variables(a::var(f))));
          result = pbes_expr::exists(a::var(f), RHS(f0, s::not_(a::arg(f)), lps, T, id_generator));
        }
        else if (s::is_exists(f))
        {
          id_generator.add_identifiers(data::detail::variable_names(data::find_variables(a::var(f))));
          result = pbes_expr::forall(a::var(f), RHS(f0, s::not_(a::arg(f)), lps, T, id_generator));
        }
        else if (s::is_must(f))
        {
          action_formulas::action_formula alpha = a::act(f);
          state_formulas::state_formula phi = a::arg(f);
          result = RHS(f0, s::may(alpha, s::not_(phi)), lps, T, id_generator);
        }
        else if (s::is_may(f))
        {
          action_formulas::action_formula alpha = a::act(f);
          state_formulas::state_formula phi = a::arg(f);
          result = RHS(f0, s::must(alpha, s::not_(phi)), lps, T, id_generator);
        }
        else if (s::is_delay_timed(f))
        {
          data::data_expression t = a::time(f);
          result = RHS(f0, s::yaled_timed(t), lps, T, id_generator);
        }
        else if (s::is_yaled_timed(f))
        {
          data::data_expression t = a::time(f);
          result = RHS(f0, s::delay_timed(t), lps, T, id_generator);
        }
        else if (s::is_variable(f))
        {
          result = z::not_(RHS(f0, f, lps, T, id_generator));
        }
        else if (s::is_mu(f) || (s::is_nu(f)))
        {
          core::identifier_string X = a::name(f);
          data::assignment_list xf = a::ass(f);
          state_formulas::state_formula phi = a::arg(f);
          if (s::is_mu(f))
          {
            result = RHS(f0, s::mu(X, xf, s::not_(phi)), lps, T, id_generator);
          }
          else
          {
            result = RHS(f0, s::nu(X, xf, s::not_(phi)), lps, T, id_generator);
          }
        }
        else
        {
          throw mcrl2::runtime_error(std::string("RHS[timed] error: unknown state formula ") + state_formulas::pp(f));
        }
      }
#ifdef MCRL2_PBES_TRANSLATE_DEBUG
      lps2pbes_decrease_indent();
      std::cerr << "\n" << lps2pbes_indent() << "<RHSresult>" << pbes_system::pp(result) << std::flush;
#endif
      return result;
    }

    /// \brief The \p E function of the translation
    /// \param f0 A modal formula
    /// \param f A modal formula
    /// \param lps A linear process
    /// \param T A data variable
    /// \return The function result
    atermpp::vector<pbes_equation> E(state_formulas::state_formula f0,
                                     state_formulas::state_formula f,
                                     const lps::linear_process& lps,
                                     data::variable T)
    {
#ifdef MCRL2_PBES_TRANSLATE_DEBUG
      std::cerr << "\n" << lps2pbes_indent() << "<E timed>" << state_formulas::pp(f) << std::flush;
      lps2pbes_increase_indent();
#endif
      using namespace state_formulas::detail::accessors;
      namespace s = state_formulas;
      using namespace data;
      atermpp::vector<pbes_equation> result;

      if (!s::is_not(f))
      {
        if (data::is_data_expression(f))
        {
          // do nothing
        }
        else if (s::is_true(f))
        {
          // do nothing
        }
        else if (s::is_false(f))
        {
          // do nothing
        }
        else if (s::is_and(f))
        {
          result = E(f0, left(f), lps, T) + E(f0, right(f), lps, T);
        }
        else if (s::is_or(f))
        {
          result = E(f0, left(f), lps, T) + E(f0, right(f), lps, T);
        }
        else if (s::is_imp(f))
        {
          result = E(f0, s::not_(left(f)), lps, T) + E(f0, right(f), lps, T);
        }
        else if (s::is_forall(f))
        {
          result = E(f0, arg(f), lps, T);
        }
        else if (s::is_exists(f))
        {
          result = E(f0, arg(f), lps, T);
        }
        else if (s::is_must(f))
        {
          result = E(f0, arg(f), lps, T);
        }
        else if (s::is_may(f))
        {
          result = E(f0, arg(f), lps, T);
        }
        else if (s::is_variable(f))
        {
          // do nothing
        }
        else if (s::is_mu(f) || (s::is_nu(f)))
        {
          core::identifier_string X = name(f);
          data::variable_list xf = detail::mu_variables(f);
          data::variable_list xp = lps.process_parameters();
          state_formulas::state_formula g = arg(f);
          fixpoint_symbol sigma = s::is_mu(f) ? fixpoint_symbol::mu() : fixpoint_symbol::nu();
          propositional_variable v(X, T + xf + xp + Par(X, data::variable_list(), f0));
          data::set_identifier_generator id_generator;
          pbes_expression expr = RHS(f0, g, lps, T, id_generator);
          pbes_equation e(sigma, v, expr);
          result = atermpp::vector<pbes_equation>() + e + E(f0, g, lps, T);
        }
        else if (s::is_yaled_timed(f))
        {
          // do nothing
        }
        else if (s::is_delay_timed(f))
        {
          // do nothing
        }
        else
        {
          throw mcrl2::runtime_error(std::string("E[timed] error: unknown state formula ") + state_formulas::pp(f));
        }
      }
      else // the formula is a negation
      {
        f = arg(f);
        if (data::is_data_expression(f))
        {
          // do nothing
        }
        else if (s::is_true(f))
        {
          // do nothing
        }
        else if (s::is_false(f))
        {
          // do nothing
        }
        else if (s::is_not(f))
        {
          result = E(f0, arg(f), lps, T);
        }
        else if (s::is_and(f))
        {
          result = E(f0, s::not_(left(f)), lps, T) + E(f0, s::not_(right(f)), lps, T);
        }
        else if (s::is_or(f))
        {
          result = E(f0, s::not_(left(f)), lps, T) + E(f0, s::not_(right(f)), lps, T);
        }
        else if (s::is_imp(f))
        {
          result = E(f0, left(f), lps, T) + E(f0, s::not_(right(f)), lps, T);
        }
        else if (s::is_forall(f))
        {
          result = E(f0, s::not_(arg(f)), lps, T);
        }
        else if (s::is_exists(f))
        {
          result = E(f0, s::not_(arg(f)), lps, T);
        }
        else if (s::is_must(f))
        {
          result = E(f0, s::not_(arg(f)), lps, T);
        }
        else if (s::is_may(f))
        {
          result = E(f0, s::not_(arg(f)), lps, T);
        }
        else if (s::is_variable(f))
        {
          // do nothing
        }
        else if (s::is_mu(f) || (s::is_nu(f)))
        {
          core::identifier_string X = name(f);
          data::variable_list xf = detail::mu_variables(f);
          data::variable_list xp = lps.process_parameters();
          fixpoint_symbol sigma = s::is_mu(f) ? fixpoint_symbol::nu() : fixpoint_symbol::mu();
          propositional_variable v(X, T + xf + xp + Par(X, data::variable_list(), f0));
          state_formulas::state_formula g = s::not_(arg(f));
          g = state_formulas::detail::negate_propositional_variable(v.name(), g);
          data::set_identifier_generator id_generator;
          pbes_expression expr = RHS(f0, g, lps, T, id_generator);
          pbes_equation e(sigma, v, expr);
          result = atermpp::vector<pbes_equation>() + e + E(f0, g, lps, T);
        }
        else if (s::is_yaled_timed(f))
        {
          // do nothing
        }
        else if (s::is_delay_timed(f))
        {
          // do nothing
        }
        else
        {
          throw mcrl2::runtime_error(std::string("E[timed] error: unknown state formula ") + state_formulas::pp(f));
        }
      }
#ifdef MCRL2_PBES_TRANSLATE_DEBUG
      lps2pbes_decrease_indent();
      std::cerr << "\n" << lps2pbes_indent() << "<Eresult>" << detail::myprint(result) << std::flush;
#endif
      return result;
    }

  public:
    /// \brief Constructor.
    pbes_translate_algorithm_timed()
    {}

    /// \brief Runs the translation algorithm
    /// \param formula A modal formula
    /// \param spec A linear process specification
    /// \return The result of the translation
    pbes<> run(const state_formulas::state_formula& formula, const lps::specification& spec)
    {
      using namespace state_formulas::detail::accessors;
      using atermpp::detail::operator+;
      namespace s = state_formulas;

      lps::linear_process lps = spec.process();

      // resolve name conflicts and wrap the formula in a mu or nu if needed
      state_formulas::state_formula f = state_formulas::preprocess_state_formula(formula, spec);

      // make sure the lps is timed
      std::set<core::identifier_string> id_generator = lps::find_identifiers(spec);
      std::set<core::identifier_string> fcontext = state_formulas::find_identifiers(f);
      id_generator.insert(fcontext.begin(), fcontext.end());

      data::variable T = fresh_variable(id_generator, data::sort_real::real_(), "T");
      id_generator.insert(T.name());
      lps::detail::make_timed_lps(lps, id_generator);

      // compute the equations
      atermpp::vector<pbes_equation> e = E(f, f, lps, T);

      // compute initial state
      assert(e.size() > 0);
      pbes_equation e1 = e.front();
      core::identifier_string Xe(e1.variable().name());
      assert(s::is_mu(f) || s::is_nu(f));
      core::identifier_string Xf = name(f);
      data::data_expression_list fi = detail::mu_expressions(f);
      data::data_expression_list pi = spec.initial_process().state(spec.process().process_parameters());
      propositional_variable_instantiation init(Xe, data::sort_real::real_(0) + fi + pi + Par(Xf, data::variable_list(), f));

      pbes<> result(spec.data(), e, spec.global_variables(), init);
      pbes_system::normalize(result);
      assert(pbes_system::is_normalized(result));
      assert(result.is_closed());
      complete_data_specification(result);
      return result;
    }
};

/// \brief Algorithm for translating a state formula and an untimed specification to a pbes.
class pbes_translate_algorithm_untimed_base: public pbes_translate_algorithm
{
  protected:

    /// \brief The \p sat_top function of the translation
    /// \param x A sequence of actions
    /// \param b An action formula
    /// \return The function result
    pbes_expression sat_top(const lps::multi_action& x, const action_formulas::action_formula& b)
    {
      return detail::Sat(x, b);
    }
};

/// \brief Algorithm for translating a state formula and an untimed specification to a pbes.
class pbes_translate_algorithm_untimed: public pbes_translate_algorithm_untimed_base
{
  protected:
    /// \brief The \p RHS function of the translation
    /// \param f0 A modal formula
    /// \param f A modal formula
    /// \param lps A linear process
    /// \param id_generator A set of strings that may not be used for naming a fresh variable
    /// \return The function result
    pbes_expression RHS(state_formulas::state_formula f0,
                        state_formulas::state_formula f,
                        const lps::linear_process& lps,
                        data::set_identifier_generator& id_generator)
    {
#ifdef MCRL2_PBES_TRANSLATE_DEBUG
      std::cerr << "\n" << lps2pbes_indent() << "<RHS-untimed>" << state_formulas::pp(f) << std::flush;
      lps2pbes_increase_indent();
#endif
      namespace z = pbes_expr_optimized;
      using namespace data::detail;
      using namespace accessors;
      using atermpp::detail::operator+;
      namespace s = state_formulas;
      namespace a = state_formulas::detail::accessors;

      pbes_expression result;

      if (!s::is_not(f))
      {
        if (data::is_data_expression(f))
        {
          result = pbes_expression(f);
        }
        else if (s::is_true(f))
        {
          result = z::true_();
        }
        else if (s::is_false(f))
        {
          result = z::false_();
        }
        else if (s::is_and(f))
        {
          result = z::and_(RHS(f0, a::left(f), lps, id_generator), RHS(f0, a::right(f), lps, id_generator));
        }
        else if (s::is_or(f))
        {
          result = z::or_(RHS(f0, a::left(f), lps, id_generator), RHS(f0, a::right(f), lps, id_generator));
        }
        else if (s::is_imp(f))
        {
          // DANGEROUS! result = imp(RHS(f0, a::left(f), lps, id_generator), RHS(f0, a::right(f), lps, id_generator));
          result = z::or_(RHS(f0, s::not_(a::left(f)), lps, id_generator), RHS(f0, a::right(f), lps, id_generator));
        }
        else if (s::is_forall(f))
        {
          id_generator.add_identifiers(data::detail::variable_names(data::find_variables(a::var(f))));
          result = pbes_expr::forall(a::var(f), RHS(f0, a::arg(f), lps, id_generator));
        }
        else if (s::is_exists(f))
        {
          id_generator.add_identifiers(data::detail::variable_names(data::find_variables(a::var(f))));
          result = pbes_expr::exists(a::var(f), RHS(f0, a::arg(f), lps, id_generator));
        }
        else if (s::is_must(f))
        {
          atermpp::vector<pbes_expression> v;
          action_formulas::action_formula alpha(a::act(f));
          state_formulas::state_formula phi(a::arg(f));
          const lps::action_summand_vector& asv=lps.action_summands();
          for (lps::action_summand_vector::const_iterator i = asv.begin(); i != asv.end(); ++i)
          {
            data::data_expression ci(i->condition());
            lps::action_list ai(i->multi_action().actions());
            data::assignment_list gi = i->assignments();
            //data::variable_list xp(lps.process_parameters());
            data::variable_list yi(i->summation_variables());

            pbes_expression rhs = RHS(f0, phi, lps, id_generator);
            data::variable_list y = pbes_system::detail::make_fresh_variables(yi, id_generator);
            ci = data::replace_free_variables(ci, data::make_sequence_sequence_substitution(yi, y));
            ai = lps::replace_free_variables(ai, data::make_sequence_sequence_substitution(yi, y));
            gi = data::replace_free_variables(gi, data::make_sequence_sequence_substitution(yi, y));
            pbes_expression p1 = sat_top(ai, alpha);
            pbes_expression p2 = ci;
            rhs = pbes_system::replace_free_variables(rhs, data::assignment_sequence_substitution(gi));

            pbes_expression p = pbes_expr::forall(y, imp(and_(p1, p2), rhs));
            v.push_back(p);
          }
          result = z::join_and(v.begin(), v.end());
        }
        else if (s::is_may(f))
        {
          atermpp::vector<pbes_expression> v;
          action_formulas::action_formula alpha(a::act(f));
          state_formulas::state_formula phi(a::arg(f));
          const lps::action_summand_vector& asv=lps.action_summands();
          for (lps::action_summand_vector::const_iterator i = asv.begin(); i != asv.end(); ++i)
          {
            data::data_expression ci(i->condition());
            lps::action_list ai(i->multi_action().actions());
            data::assignment_list gi = i->assignments();
            //data::variable_list xp(lps.process_parameters());
            data::variable_list yi(i->summation_variables());

            pbes_expression rhs = RHS(f0, phi, lps, id_generator);
            data::variable_list y = pbes_system::detail::make_fresh_variables(yi, id_generator);
            ci = data::replace_free_variables(ci, data::make_sequence_sequence_substitution(yi, y));
            ai = lps::replace_free_variables(ai, data::make_sequence_sequence_substitution(yi, y));
            gi = data::replace_free_variables(gi, data::make_sequence_sequence_substitution(yi, y));
            pbes_expression p1 = sat_top(ai, alpha);
            pbes_expression p2 = ci;
            rhs = pbes_system::replace_free_variables(rhs, data::assignment_sequence_substitution(gi));

            pbes_expression p = pbes_expr::exists(y, and_(and_(p1, p2), rhs));
            v.push_back(p);
          }
          result = z::join_or(v.begin(), v.end());
        }
        else if (s::is_variable(f))
        {
          core::identifier_string X = a::name(f);
          data::data_expression_list d = a::param(f);
          data::variable_list xp = lps.process_parameters();
          result = propositional_variable_instantiation(X, d + xp + Par(X, data::variable_list(), f0));
        }
        else if (s::is_mu(f) || (s::is_nu(f)))
        {
          core::identifier_string X = a::name(f);
          data::data_expression_list d = detail::mu_expressions(f);
          data::variable_list xp = lps.process_parameters();
          result = propositional_variable_instantiation(X, d + xp + Par(X, data::variable_list(), f0));
        }
        else
        {
          throw mcrl2::runtime_error(std::string("RHS[untimed] error: unknown state formula ") + state_formulas::pp(f));
        }
      }
      else // the formula is a negation
      {
        f = a::arg(f);
        if (data::is_data_expression(f))
        {
          result = pbes_expression(data::sort_bool::not_(f));
        }
        else if (s::is_true(f))
        {
          result = z::false_();
        }
        else if (s::is_false(f))
        {
          result = z::true_();
        }
        else if (s::is_not(f))
        {
          result = RHS(f0, a::arg(f), lps, id_generator);
        }
        else if (s::is_and(f))
        {
          result = z::or_(RHS(f0, s::not_(a::left(f)), lps, id_generator), RHS(f0, s::not_(a::right(f)), lps, id_generator));
        }
        else if (s::is_or(f))
        {
          result = z::and_(RHS(f0, s::not_(a::left(f)), lps, id_generator), RHS(f0, s::not_(a::right(f)), lps, id_generator));
        }
        else if (s::is_imp(f))
        {
          result = z::and_(RHS(f0, a::left(f), lps, id_generator), RHS(f0, s::not_(a::right(f)), lps, id_generator));
        }
        else if (s::is_forall(f))
        {
          id_generator.add_identifiers(data::detail::variable_names(data::find_variables(a::var(f))));
          result = pbes_expr::exists(a::var(f), RHS(f0, s::not_(a::arg(f)), lps, id_generator));
        }
        else if (s::is_exists(f))
        {
          id_generator.add_identifiers(data::detail::variable_names(data::find_variables(a::var(f))));
          result = pbes_expr::forall(a::var(f), RHS(f0, s::not_(a::arg(f)), lps, id_generator));
        }
        else if (s::is_must(f))
        {
          action_formulas::action_formula alpha = a::act(f);
          state_formulas::state_formula phi = a::arg(f);
          result = RHS(f0, s::may(alpha, s::not_(phi)), lps, id_generator);
        }
        else if (s::is_may(f))
        {
          action_formulas::action_formula alpha = a::act(f);
          state_formulas::state_formula phi = a::arg(f);
          result = RHS(f0, s::must(alpha, s::not_(phi)), lps, id_generator);
        }
        else if (s::is_delay(f))
        {
          result = RHS(f0, s::yaled(), lps, id_generator);
        }
        else if (s::is_yaled(f))
        {
          result = RHS(f0, s::delay(), lps, id_generator);
        }
        else if (s::is_variable(f))
        {
          result = z::not_(RHS(f0, f, lps, id_generator));
        }
        else if (s::is_mu(f) || (s::is_nu(f)))
        {
          core::identifier_string X = a::name(f);
          data::assignment_list xf = a::ass(f);
          state_formulas::state_formula phi = a::arg(f);
          if (s::is_mu(f))
          {
            result = RHS(f0, s::mu(X, xf, s::not_(phi)), lps, id_generator);
          }
          else
          {
            result = RHS(f0, s::nu(X, xf, s::not_(phi)), lps, id_generator);
          }
        }
        else
        {
          throw mcrl2::runtime_error(std::string("RHS[untimed] error: unknown state formula ") + state_formulas::pp(f));
        }
      }
#ifdef MCRL2_PBES_TRANSLATE_DEBUG
      lps2pbes_decrease_indent();
      std::cerr << "\n" << lps2pbes_indent() << "<RHSresult>" << pbes_system::pp(result) << std::flush;
#endif
      return result;
    }

    /// \brief The \p E function of the translation
    /// \param f0 A modal formula
    /// \param f A modal formula
    /// \param lps A linear process
    /// \return The function result
    atermpp::vector<pbes_equation> E(
      state_formulas::state_formula f0,
      state_formulas::state_formula f,
      const lps::linear_process& lps)
    {
#ifdef MCRL2_PBES_TRANSLATE_DEBUG
      std::cerr << "\n" << lps2pbes_indent() << "<E-untimed>" << state_formulas::pp(f) << std::flush;
      lps2pbes_increase_indent();
#endif
      using namespace state_formulas::detail::accessors;
      namespace s = state_formulas;
      using namespace data;
      //using state_formulas::is_variable;
      atermpp::vector<pbes_equation> result;

      if (!s::is_not(f))
      {
        if (data::is_data_expression(f))
        {
          // do nothing
        }
        else if (s::is_true(f))
        {
          // do nothing
        }
        else if (s::is_false(f))
        {
          // do nothing
        }
        else if (s::is_and(f))
        {
          result = E(f0, left(f), lps) + E(f0, right(f), lps);
        }
        else if (s::is_or(f))
        {
          result = E(f0, left(f), lps) + E(f0, right(f), lps);
        }
        else if (s::is_imp(f))
        {
          result = E(f0, s::not_(left(f)), lps) + E(f0, right(f), lps);
        }
        else if (s::is_forall(f))
        {
          result = E(f0, arg(f), lps);
        }
        else if (s::is_exists(f))
        {
          result = E(f0, arg(f), lps);
        }
        else if (s::is_must(f))
        {
          result = E(f0, arg(f), lps);
        }
        else if (s::is_may(f))
        {
          result = E(f0, arg(f), lps);
        }
        else if (s::is_variable(f))
        {
          // do nothing
        }
        else if (s::is_mu(f) || (s::is_nu(f)))
        {
          core::identifier_string X = name(f);
          data::variable_list xf = detail::mu_variables(f);
          data::variable_list xp = lps.process_parameters();
          state_formulas::state_formula g = arg(f);
          fixpoint_symbol sigma = s::is_mu(f) ? fixpoint_symbol::mu() : fixpoint_symbol::nu();
          propositional_variable v(X, xf + xp + Par(X, data::variable_list(), f0));
          data::set_identifier_generator id_generator;
          pbes_expression expr = RHS(f0, g, lps, id_generator);
          pbes_equation e(sigma, v, expr);
          result = atermpp::vector<pbes_equation>() + e + E(f0, g, lps);
        }
        else if (s::is_yaled(f))
        {
          // do nothing
        }
        else if (s::is_delay(f))
        {
          // do nothing
        }
        else
        {
          throw mcrl2::runtime_error(std::string("E[untimed] error: unknown state formula ") + state_formulas::pp(f));
        }
      }
      else // the formula is a negation
      {
        f = arg(f);
        if (data::is_data_expression(f))
        {
          // do nothing
        }
        else if (s::is_true(f))
        {
          // do nothing
        }
        else if (s::is_false(f))
        {
          // do nothing
        }
        else if (s::is_not(f))
        {
          result = E(f0, arg(f), lps);
        }
        else if (s::is_and(f))
        {
          result = E(f0, s::not_(left(f)), lps) + E(f0, s::not_(right(f)), lps);
        }
        else if (s::is_or(f))
        {
          result = E(f0, s::not_(left(f)), lps) + E(f0, s::not_(right(f)), lps);
        }
        else if (s::is_imp(f))
        {
          result = E(f0, left(f), lps) + E(f0, s::not_(right(f)), lps);
        }
        else if (s::is_forall(f))
        {
          result = E(f0, s::not_(arg(f)), lps);
        }
        else if (s::is_exists(f))
        {
          result = E(f0, s::not_(arg(f)), lps);
        }
        else if (s::is_must(f))
        {
          result = E(f0, s::not_(arg(f)), lps);
        }
        else if (s::is_may(f))
        {
          result = E(f0, s::not_(arg(f)), lps);
        }
        else if (s::is_variable(f))
        {
          // do nothing
        }
        else if (s::is_mu(f) || (s::is_nu(f)))
        {
          core::identifier_string X = name(f);
          data::variable_list xf = detail::mu_variables(f);
          data::variable_list xp = lps.process_parameters();
          fixpoint_symbol sigma = s::is_mu(f) ? fixpoint_symbol::nu() : fixpoint_symbol::mu();
          propositional_variable v(X, xf + xp + Par(X, data::variable_list(), f0));
          state_formulas::state_formula g = s::not_(arg(f));
          g = state_formulas::detail::negate_propositional_variable(v.name(), g);
          data::set_identifier_generator id_generator;
          pbes_expression expr = RHS(f0, g, lps, id_generator);
          pbes_equation e(sigma, v, expr);
          result = atermpp::vector<pbes_equation>() + e + E(f0, g, lps);
        }
        else if (s::is_yaled_timed(f))
        {
          // do nothing
        }
        else if (s::is_delay_timed(f))
        {
          // do nothing
        }
        else
        {
          throw mcrl2::runtime_error(std::string("E[untimed] error: unknown state formula ") + state_formulas::pp(f));
        }
      }
#ifdef MCRL2_PBES_TRANSLATE_DEBUG
      lps2pbes_decrease_indent();
      std::cerr << "\n" << lps2pbes_indent() << "<Eresult>" << detail::myprint(result) << std::flush;
#endif
      return result;
    }

  public:
    /// \brief Constructor.
    pbes_translate_algorithm_untimed()
    {}

    /// \brief Runs the translation algorithm
    /// \param formula A modal formula
    /// \param spec A linear process specification
    /// \return The result of the translation
    pbes<> run(const state_formulas::state_formula& formula, const lps::specification& spec)
    {
      using namespace state_formulas::detail::accessors;
      using atermpp::detail::operator+;
      lps::linear_process lps = spec.process();

      if (!state_formulas::is_monotonous(formula))
      {
        throw mcrl2::runtime_error(std::string("lps2pbes error: the formula ") + state_formulas::pp(formula) + " is not monotonous!");
      }

      // resolve name conflicts and wrap the formula in a mu or nu if needed
      state_formulas::state_formula f = state_formulas::preprocess_state_formula(formula, spec);

#ifdef MCRL2_PBES_TRANSLATE_DEBUG
      std::cerr << "<preprocessed formula>" << state_formulas::pp(f) << std::endl;
#endif

      // compute the equations
      atermpp::vector<pbes_equation> e = E(f, f, lps);

      // compute the initial state
      assert(e.size() > 0);
      pbes_equation e1 = e.front();
      core::identifier_string Xe(e1.variable().name());
      assert(is_mu(f) || is_nu(f));
      core::identifier_string Xf = name(f);
      data::data_expression_list fi = detail::mu_expressions(f);
      data::data_expression_list pi = spec.initial_process().state(spec.process().process_parameters());
      propositional_variable_instantiation init(Xe, fi + pi + Par(Xf, data::variable_list(), f));

      pbes<> result = pbes<>(spec.data(), e, spec.global_variables(), init);
      assert(is_monotonous(result));
      pbes_system::normalize(result);
      assert(pbes_system::is_normalized(result));
      assert(result.is_closed());
      complete_data_specification(result);
      return result;
    }
};

/// \brief Translates a state_formulas::state_formula and a lps::specification to a pbes. If the pbes evaluates
/// to true, the formula holds for the lps::specification.
/// \param formula A modal formula
/// \param spec A linear process specification
/// \param timed determines whether the timed or untimed variant of the algorithm is chosen
/// \return The resulting pbes
inline pbes<> pbes_translate(const state_formulas::state_formula& formula, const lps::specification& spec, bool timed = false)
{
  if ((formula.has_time() || spec.process().has_time()) && !timed)
  {
    mCRL2log(log::warning) << "Switch to timed translation because formula has " << (formula.has_time()?"":"no ") << "time, and process has " << (spec.process().has_time()?"":"no ") << "time" << std::endl;
    timed = true;
  }

  if (timed)
  {
    pbes_translate_algorithm_timed algorithm;
    return algorithm.run(formula, spec);
  }
  else
  {
    pbes_translate_algorithm_untimed algorithm;
    return algorithm.run(formula, spec);
  }
}

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_PBES_TRANSLATE_H
