// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/builder.h
/// \brief add your file description here.

#ifndef MCRL2_MODAL_FORMULA_BUILDER_H
#define MCRL2_MODAL_FORMULA_BUILDER_H

#include "mcrl2/modal_formula/action_formula.h"
#include "mcrl2/modal_formula/regular_formula.h"
#include "mcrl2/modal_formula/state_formula.h"
#include "mcrl2/data/builder.h"
#include "mcrl2/lps/builder.h"

namespace mcrl2 {

namespace action_formulas {

//--- start generated action_formulas::add_sort_expressions code ---//
  template <template <class> class Builder, class Derived>
  struct add_sort_expressions: public Builder<Derived>
  {
    typedef Builder<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    action_formulas::action_formula operator()(const action_formulas::true_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    action_formulas::action_formula operator()(const action_formulas::false_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    action_formulas::action_formula operator()(const action_formulas::not_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::not_(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::and_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::and_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::or_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::or_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::imp& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::imp(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::forall& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::forall(static_cast<Derived&>(*this)(x.variables()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::exists& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::exists(static_cast<Derived&>(*this)(x.variables()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::at& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::at(static_cast<Derived&>(*this)(x.operand()), static_cast<Derived&>(*this)(x.time_stamp()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::action_formula& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result;
      if (data::is_data_expression(x)) { result = static_cast<Derived&>(*this)(data::data_expression(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_true(x)) { result = static_cast<Derived&>(*this)(action_formulas::true_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_false(x)) { result = static_cast<Derived&>(*this)(action_formulas::false_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_not(x)) { result = static_cast<Derived&>(*this)(action_formulas::not_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_and(x)) { result = static_cast<Derived&>(*this)(action_formulas::and_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_or(x)) { result = static_cast<Derived&>(*this)(action_formulas::or_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_imp(x)) { result = static_cast<Derived&>(*this)(action_formulas::imp(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_forall(x)) { result = static_cast<Derived&>(*this)(action_formulas::forall(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_exists(x)) { result = static_cast<Derived&>(*this)(action_formulas::exists(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_at(x)) { result = static_cast<Derived&>(*this)(action_formulas::at(atermpp::aterm_appl(x))); }
      else if (lps::is_multi_action(x)) { lps::multi_action y = x; static_cast<Derived&>(*this)(y); result = y; }
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
  };

  /// \brief Builder class
  template <typename Derived>
  struct sort_expression_builder: public add_sort_expressions<lps::sort_expression_builder, Derived>
  {
    typedef add_sort_expressions<lps::sort_expression_builder, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated action_formulas::add_sort_expressions code ---//

//--- start generated action_formulas::add_data_expressions code ---//
  template <template <class> class Builder, class Derived>
  struct add_data_expressions: public Builder<Derived>
  {
    typedef Builder<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    action_formulas::action_formula operator()(const action_formulas::true_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    action_formulas::action_formula operator()(const action_formulas::false_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    action_formulas::action_formula operator()(const action_formulas::not_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::not_(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::and_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::and_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::or_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::or_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::imp& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::imp(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::forall& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::forall(x.variables(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::exists& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::exists(x.variables(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::at& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::at(static_cast<Derived&>(*this)(x.operand()), static_cast<Derived&>(*this)(x.time_stamp()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::action_formula& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result;
      if (data::is_data_expression(x)) { result = static_cast<Derived&>(*this)(data::data_expression(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_true(x)) { result = static_cast<Derived&>(*this)(action_formulas::true_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_false(x)) { result = static_cast<Derived&>(*this)(action_formulas::false_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_not(x)) { result = static_cast<Derived&>(*this)(action_formulas::not_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_and(x)) { result = static_cast<Derived&>(*this)(action_formulas::and_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_or(x)) { result = static_cast<Derived&>(*this)(action_formulas::or_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_imp(x)) { result = static_cast<Derived&>(*this)(action_formulas::imp(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_forall(x)) { result = static_cast<Derived&>(*this)(action_formulas::forall(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_exists(x)) { result = static_cast<Derived&>(*this)(action_formulas::exists(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_at(x)) { result = static_cast<Derived&>(*this)(action_formulas::at(atermpp::aterm_appl(x))); }
      else if (lps::is_multi_action(x)) { lps::multi_action y = x; static_cast<Derived&>(*this)(y); result = y; }
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
  };

  /// \brief Builder class
  template <typename Derived>
  struct data_expression_builder: public add_data_expressions<lps::data_expression_builder, Derived>
  {
    typedef add_data_expressions<lps::data_expression_builder, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated action_formulas::add_data_expressions code ---//

//--- start generated action_formulas::add_action_formula_expressions code ---//
  template <template <class> class Builder, class Derived>
  struct add_action_formula_expressions: public Builder<Derived>
  {
    typedef Builder<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    action_formulas::action_formula operator()(const action_formulas::true_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    action_formulas::action_formula operator()(const action_formulas::false_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    action_formulas::action_formula operator()(const action_formulas::not_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::not_(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::and_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::and_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::or_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::or_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::imp& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::imp(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::forall& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::forall(x.variables(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::exists& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::exists(x.variables(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::at& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result = action_formulas::at(static_cast<Derived&>(*this)(x.operand()), x.time_stamp());
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    action_formulas::action_formula operator()(const action_formulas::action_formula& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      action_formulas::action_formula result;
      if (data::is_data_expression(x)) { result = static_cast<Derived&>(*this)(data::data_expression(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_true(x)) { result = static_cast<Derived&>(*this)(action_formulas::true_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_false(x)) { result = static_cast<Derived&>(*this)(action_formulas::false_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_not(x)) { result = static_cast<Derived&>(*this)(action_formulas::not_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_and(x)) { result = static_cast<Derived&>(*this)(action_formulas::and_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_or(x)) { result = static_cast<Derived&>(*this)(action_formulas::or_(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_imp(x)) { result = static_cast<Derived&>(*this)(action_formulas::imp(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_forall(x)) { result = static_cast<Derived&>(*this)(action_formulas::forall(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_exists(x)) { result = static_cast<Derived&>(*this)(action_formulas::exists(atermpp::aterm_appl(x))); }
      else if (action_formulas::is_at(x)) { result = static_cast<Derived&>(*this)(action_formulas::at(atermpp::aterm_appl(x))); }
      else if (lps::is_multi_action(x)) { lps::multi_action y = x; static_cast<Derived&>(*this)(y); result = y; }
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
  };

  /// \brief Builder class
  template <typename Derived>
  struct action_formula_builder: public add_action_formula_expressions<core::builder, Derived>
  {
    typedef add_action_formula_expressions<core::builder, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated action_formulas::add_action_formula_expressions code ---//

} // namespace action_formulas

namespace regular_formulas {

//--- start generated regular_formulas::add_sort_expressions code ---//
  template <template <class> class Builder, class Derived>
  struct add_sort_expressions: public Builder<Derived>
  {
    typedef Builder<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    regular_formulas::regular_formula operator()(const regular_formulas::nil& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::seq& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::seq(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::alt& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::alt(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::trans& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::trans(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::trans_or_nil& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::trans_or_nil(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::regular_formula& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result;
      if (action_formulas::is_action_formula(x)) { result = static_cast<Derived&>(*this)(action_formulas::action_formula(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_nil(x)) { result = static_cast<Derived&>(*this)(regular_formulas::nil(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_seq(x)) { result = static_cast<Derived&>(*this)(regular_formulas::seq(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_alt(x)) { result = static_cast<Derived&>(*this)(regular_formulas::alt(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_trans(x)) { result = static_cast<Derived&>(*this)(regular_formulas::trans(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_trans_or_nil(x)) { result = static_cast<Derived&>(*this)(regular_formulas::trans_or_nil(atermpp::aterm_appl(x))); }
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
  };

  /// \brief Builder class
  template <typename Derived>
  struct sort_expression_builder: public add_sort_expressions<action_formulas::sort_expression_builder, Derived>
  {
    typedef add_sort_expressions<action_formulas::sort_expression_builder, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated regular_formulas::add_sort_expressions code ---//

//--- start generated regular_formulas::add_data_expressions code ---//
  template <template <class> class Builder, class Derived>
  struct add_data_expressions: public Builder<Derived>
  {
    typedef Builder<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    regular_formulas::regular_formula operator()(const regular_formulas::nil& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::seq& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::seq(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::alt& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::alt(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::trans& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::trans(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::trans_or_nil& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::trans_or_nil(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::regular_formula& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result;
      if (action_formulas::is_action_formula(x)) { result = static_cast<Derived&>(*this)(action_formulas::action_formula(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_nil(x)) { result = static_cast<Derived&>(*this)(regular_formulas::nil(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_seq(x)) { result = static_cast<Derived&>(*this)(regular_formulas::seq(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_alt(x)) { result = static_cast<Derived&>(*this)(regular_formulas::alt(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_trans(x)) { result = static_cast<Derived&>(*this)(regular_formulas::trans(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_trans_or_nil(x)) { result = static_cast<Derived&>(*this)(regular_formulas::trans_or_nil(atermpp::aterm_appl(x))); }
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
  };

  /// \brief Builder class
  template <typename Derived>
  struct data_expression_builder: public add_data_expressions<action_formulas::data_expression_builder, Derived>
  {
    typedef add_data_expressions<action_formulas::data_expression_builder, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated regular_formulas::add_data_expressions code ---//

//--- start generated regular_formulas::add_regular_formula_expressions code ---//
  template <template <class> class Builder, class Derived>
  struct add_regular_formula_expressions: public Builder<Derived>
  {
    typedef Builder<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    regular_formulas::regular_formula operator()(const regular_formulas::nil& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::seq& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::seq(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::alt& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::alt(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::trans& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::trans(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::trans_or_nil& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result = regular_formulas::trans_or_nil(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    regular_formulas::regular_formula operator()(const regular_formulas::regular_formula& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      regular_formulas::regular_formula result;
      if (action_formulas::is_action_formula(x)) { result = static_cast<Derived&>(*this)(action_formulas::action_formula(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_nil(x)) { result = static_cast<Derived&>(*this)(regular_formulas::nil(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_seq(x)) { result = static_cast<Derived&>(*this)(regular_formulas::seq(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_alt(x)) { result = static_cast<Derived&>(*this)(regular_formulas::alt(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_trans(x)) { result = static_cast<Derived&>(*this)(regular_formulas::trans(atermpp::aterm_appl(x))); }
      else if (regular_formulas::is_trans_or_nil(x)) { result = static_cast<Derived&>(*this)(regular_formulas::trans_or_nil(atermpp::aterm_appl(x))); }
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
  };

  /// \brief Builder class
  template <typename Derived>
  struct regular_formula_builder: public add_regular_formula_expressions<core::builder, Derived>
  {
    typedef add_regular_formula_expressions<core::builder, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated regular_formulas::add_regular_formula_expressions code ---//

} // namespace regular_formulas

namespace state_formulas {

//--- start generated state_formulas::add_sort_expressions code ---//
  template <template <class> class Builder, class Derived>
  struct add_sort_expressions: public Builder<Derived>
  {
    typedef Builder<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    state_formulas::state_formula operator()(const state_formulas::true_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::false_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::not_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::not_(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::and_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::and_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::or_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::or_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::imp& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::imp(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::forall& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::forall(static_cast<Derived&>(*this)(x.variables()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::exists& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::exists(static_cast<Derived&>(*this)(x.variables()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::must& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::must(static_cast<Derived&>(*this)(x.formula()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::may& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::may(static_cast<Derived&>(*this)(x.formula()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::yaled& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::yaled_timed& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::yaled_timed(static_cast<Derived&>(*this)(x.time_stamp()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::delay& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::delay_timed& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::delay_timed(static_cast<Derived&>(*this)(x.time_stamp()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::variable& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::variable(x.name(), static_cast<Derived&>(*this)(x.arguments()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::nu& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::nu(x.name(), static_cast<Derived&>(*this)(x.assignments()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::mu& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::mu(x.name(), static_cast<Derived&>(*this)(x.assignments()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::state_formula& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result;
      if (data::is_data_expression(x)) { result = static_cast<Derived&>(*this)(data::data_expression(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_true(x)) { result = static_cast<Derived&>(*this)(state_formulas::true_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_false(x)) { result = static_cast<Derived&>(*this)(state_formulas::false_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_not(x)) { result = static_cast<Derived&>(*this)(state_formulas::not_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_and(x)) { result = static_cast<Derived&>(*this)(state_formulas::and_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_or(x)) { result = static_cast<Derived&>(*this)(state_formulas::or_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_imp(x)) { result = static_cast<Derived&>(*this)(state_formulas::imp(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_forall(x)) { result = static_cast<Derived&>(*this)(state_formulas::forall(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_exists(x)) { result = static_cast<Derived&>(*this)(state_formulas::exists(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_must(x)) { result = static_cast<Derived&>(*this)(state_formulas::must(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_may(x)) { result = static_cast<Derived&>(*this)(state_formulas::may(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_yaled(x)) { result = static_cast<Derived&>(*this)(state_formulas::yaled(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_yaled_timed(x)) { result = static_cast<Derived&>(*this)(state_formulas::yaled_timed(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_delay(x)) { result = static_cast<Derived&>(*this)(state_formulas::delay(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_delay_timed(x)) { result = static_cast<Derived&>(*this)(state_formulas::delay_timed(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_variable(x)) { result = static_cast<Derived&>(*this)(state_formulas::variable(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_nu(x)) { result = static_cast<Derived&>(*this)(state_formulas::nu(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_mu(x)) { result = static_cast<Derived&>(*this)(state_formulas::mu(atermpp::aterm_appl(x))); }
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
  };

  /// \brief Builder class
  template <typename Derived>
  struct sort_expression_builder: public add_sort_expressions<regular_formulas::sort_expression_builder, Derived>
  {
    typedef add_sort_expressions<regular_formulas::sort_expression_builder, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated state_formulas::add_sort_expressions code ---//

//--- start generated state_formulas::add_data_expressions code ---//
  template <template <class> class Builder, class Derived>
  struct add_data_expressions: public Builder<Derived>
  {
    typedef Builder<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    state_formulas::state_formula operator()(const state_formulas::true_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::false_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::not_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::not_(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::and_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::and_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::or_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::or_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::imp& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::imp(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::forall& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::forall(x.variables(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::exists& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::exists(x.variables(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::must& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::must(static_cast<Derived&>(*this)(x.formula()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::may& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::may(static_cast<Derived&>(*this)(x.formula()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::yaled& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::yaled_timed& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::yaled_timed(static_cast<Derived&>(*this)(x.time_stamp()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::delay& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::delay_timed& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::delay_timed(static_cast<Derived&>(*this)(x.time_stamp()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::variable& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::variable(x.name(), static_cast<Derived&>(*this)(x.arguments()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::nu& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::nu(x.name(), static_cast<Derived&>(*this)(x.assignments()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::mu& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::mu(x.name(), static_cast<Derived&>(*this)(x.assignments()), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::state_formula& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result;
      if (data::is_data_expression(x)) { result = static_cast<Derived&>(*this)(data::data_expression(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_true(x)) { result = static_cast<Derived&>(*this)(state_formulas::true_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_false(x)) { result = static_cast<Derived&>(*this)(state_formulas::false_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_not(x)) { result = static_cast<Derived&>(*this)(state_formulas::not_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_and(x)) { result = static_cast<Derived&>(*this)(state_formulas::and_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_or(x)) { result = static_cast<Derived&>(*this)(state_formulas::or_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_imp(x)) { result = static_cast<Derived&>(*this)(state_formulas::imp(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_forall(x)) { result = static_cast<Derived&>(*this)(state_formulas::forall(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_exists(x)) { result = static_cast<Derived&>(*this)(state_formulas::exists(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_must(x)) { result = static_cast<Derived&>(*this)(state_formulas::must(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_may(x)) { result = static_cast<Derived&>(*this)(state_formulas::may(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_yaled(x)) { result = static_cast<Derived&>(*this)(state_formulas::yaled(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_yaled_timed(x)) { result = static_cast<Derived&>(*this)(state_formulas::yaled_timed(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_delay(x)) { result = static_cast<Derived&>(*this)(state_formulas::delay(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_delay_timed(x)) { result = static_cast<Derived&>(*this)(state_formulas::delay_timed(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_variable(x)) { result = static_cast<Derived&>(*this)(state_formulas::variable(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_nu(x)) { result = static_cast<Derived&>(*this)(state_formulas::nu(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_mu(x)) { result = static_cast<Derived&>(*this)(state_formulas::mu(atermpp::aterm_appl(x))); }
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
  };

  /// \brief Builder class
  template <typename Derived>
  struct data_expression_builder: public add_data_expressions<regular_formulas::data_expression_builder, Derived>
  {
    typedef add_data_expressions<regular_formulas::data_expression_builder, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated state_formulas::add_data_expressions code ---//

//--- start generated state_formulas::add_state_formula_expressions code ---//
  template <template <class> class Builder, class Derived>
  struct add_state_formula_expressions: public Builder<Derived>
  {
    typedef Builder<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    state_formulas::state_formula operator()(const state_formulas::true_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::false_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::not_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::not_(static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::and_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::and_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::or_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::or_(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::imp& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::imp(static_cast<Derived&>(*this)(x.left()), static_cast<Derived&>(*this)(x.right()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::forall& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::forall(x.variables(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::exists& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::exists(x.variables(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::must& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::must(x.formula(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::may& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::may(x.formula(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::yaled& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::yaled_timed& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::delay& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::delay_timed& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::variable& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
      return x;
    }
    
    state_formulas::state_formula operator()(const state_formulas::nu& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::nu(x.name(), x.assignments(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::mu& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result = state_formulas::mu(x.name(), x.assignments(), static_cast<Derived&>(*this)(x.operand()));
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
    state_formulas::state_formula operator()(const state_formulas::state_formula& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      state_formulas::state_formula result;
      if (data::is_data_expression(x)) { result = static_cast<Derived&>(*this)(data::data_expression(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_true(x)) { result = static_cast<Derived&>(*this)(state_formulas::true_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_false(x)) { result = static_cast<Derived&>(*this)(state_formulas::false_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_not(x)) { result = static_cast<Derived&>(*this)(state_formulas::not_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_and(x)) { result = static_cast<Derived&>(*this)(state_formulas::and_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_or(x)) { result = static_cast<Derived&>(*this)(state_formulas::or_(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_imp(x)) { result = static_cast<Derived&>(*this)(state_formulas::imp(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_forall(x)) { result = static_cast<Derived&>(*this)(state_formulas::forall(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_exists(x)) { result = static_cast<Derived&>(*this)(state_formulas::exists(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_must(x)) { result = static_cast<Derived&>(*this)(state_formulas::must(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_may(x)) { result = static_cast<Derived&>(*this)(state_formulas::may(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_yaled(x)) { result = static_cast<Derived&>(*this)(state_formulas::yaled(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_yaled_timed(x)) { result = static_cast<Derived&>(*this)(state_formulas::yaled_timed(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_delay(x)) { result = static_cast<Derived&>(*this)(state_formulas::delay(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_delay_timed(x)) { result = static_cast<Derived&>(*this)(state_formulas::delay_timed(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_variable(x)) { result = static_cast<Derived&>(*this)(state_formulas::variable(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_nu(x)) { result = static_cast<Derived&>(*this)(state_formulas::nu(atermpp::aterm_appl(x))); }
      else if (state_formulas::is_mu(x)) { result = static_cast<Derived&>(*this)(state_formulas::mu(atermpp::aterm_appl(x))); }
      static_cast<Derived&>(*this).leave(x);
      return result;
    }
    
  };

  /// \brief Builder class
  template <typename Derived>
  struct state_formula_builder: public add_state_formula_expressions<core::builder, Derived>
  {
    typedef add_state_formula_expressions<core::builder, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated state_formulas::add_state_formula_expressions code ---//

} // namespace state_formulas

} // namespace mcrl2

#endif // MCRL2_MODAL_FORMULA_BUILDER_H
