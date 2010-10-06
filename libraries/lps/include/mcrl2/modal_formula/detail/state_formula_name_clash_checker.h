// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/detail/state_formula_name_clash_checker.h
/// \brief add your file description here.

#ifndef MCRL2_MODAL_FORMULA_DETAIL_STATE_FORMULA_NAME_CLASH_CHECKER_H
#define MCRL2_MODAL_FORMULA_DETAIL_STATE_FORMULA_NAME_CLASH_CHECKER_H

#include <vector>
#include "mcrl2/core/identifier_generator.h"
#include "mcrl2/modal_formula/traverser.h"
#include "mcrl2/exception.h"

namespace mcrl2 {

namespace state_formulas {

namespace detail {

  /// \brief Traverser that checks for name clashes in nested mu's/nu's.
  class state_formula_name_clash_checker: public state_formulas::traverser<state_formula_name_clash_checker>
  {
    public:
      typedef state_formulas::traverser<state_formula_name_clash_checker> super;
      typedef core::term_traits<state_formula> tr;

      using super::operator();
      using super::enter;
      using super::leave;
     
      /// \brief For generating fresh variables.
      core::number_postfix_generator m_generator;
      
      /// \brief The stack of names.
      std::vector<core::identifier_string> m_name_stack;

      /// \brief Pops the stack
      void pop()
      {
        m_name_stack.pop_back();
      }
      
      /// \brief Pushes name on the stack.
      void push(const core::identifier_string& name)
      {     
        if (std::find(m_name_stack.begin(), m_name_stack.end(), name) != m_name_stack.end())
        {
          throw mcrl2::runtime_error("nested propositional variable " + std::string(name) + " clashes");
        }
        m_name_stack.push_back(name);
      }    

      void enter(const mu& x)
      {
        push(x.name());
      }

      void leave(const mu& x)
      {
        pop();
      }

      void enter(const nu& x)
      {
        push(x.name());
      }

      void leave(const nu& x)
      {
        pop();
      }
  };

  /// \brief Throws a mcrl2::runtime_exception if the formula contains name clashes
  inline
  void check_name_clashes(const state_formula& f)
  {
    state_formula_name_clash_checker checker;
    //checker(f);
  }

} // namespace detail

} // namespace state_formulas

} // namespace mcrl2

#endif // MCRL2_MODAL_FORMULA_DETAIL_STATE_FORMULA_NAME_CLASH_CHECKER_H
