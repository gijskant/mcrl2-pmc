// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/untyped_multi_action.h
/// \brief add your file description here.

#ifndef MCRL2_LPS_UNTYPED_MULTI_ACTION_H
#define MCRL2_LPS_UNTYPED_MULTI_ACTION_H

#include "mcrl2/lps/untyped_action.h"

namespace mcrl2 {

namespace lps {

//--- start generated class untyped_multi_action ---//
/// \brief An untyped multi-action
class untyped_multi_action: public atermpp::aterm_appl
{
  public:
    /// \brief Default constructor.
    untyped_multi_action()
      : atermpp::aterm_appl(core::detail::constructUntypedMultAct())
    {}

    /// \brief Constructor.
    /// \param term A term
    untyped_multi_action(const atermpp::aterm& term)
      : atermpp::aterm_appl(term)
    {
      assert(core::detail::check_term_UntypedMultAct(*this));
    }

    /// \brief Constructor.
    untyped_multi_action(const untyped_action_list& actions)
      : atermpp::aterm_appl(core::detail::gsMakeUntypedMultAct(actions))
    {}

    const untyped_action_list& actions() const
    {
      return atermpp::aterm_cast<const untyped_action_list>(atermpp::list_arg1(*this));
    }
};

/// \brief list of untyped_multi_actions
typedef atermpp::term_list<untyped_multi_action> untyped_multi_action_list;

/// \brief vector of untyped_multi_actions
typedef std::vector<untyped_multi_action>    untyped_multi_action_vector;

//--- end generated class untyped_multi_action ---//

} // namespace lps

} // namespace mcrl2

#endif // MCRL2_LPS_UNTYPED_MULTI_ACTION_H