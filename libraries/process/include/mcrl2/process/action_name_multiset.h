// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/process/action_name_multiset.h
/// \brief add your file description here.

#ifndef MCRL2_PROCESS_ACTION_NAME_MULTISET_H
#define MCRL2_PROCESS_ACTION_NAME_MULTISET_H

#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/core/identifier_string.h"
#include "mcrl2/core/detail/struct.h"
#include "mcrl2/core/detail/constructors.h"

namespace mcrl2 {

namespace process {

  //<MultActName>  ::= MultActName(<String>+)
  /// \brief Multiset of action names
  class action_name_multiset: public atermpp::aterm_appl
  {
    public:
      /// \brief Constructor.
      action_name_multiset()
        : atermpp::aterm_appl(core::detail::constructMultActName())
      {}

      /// \brief Constructor.
      /// \param term A term
      action_name_multiset(atermpp::aterm_appl term)
        : atermpp::aterm_appl(term)
      {
        assert(core::detail::check_term_MultActName(m_term));
      }

      /// \brief Constructor.
      action_name_multiset(core::identifier_string_list names)
        : atermpp::aterm_appl(core::detail::gsMakeMultActName(names))
      {}

      /// \brief Returns the names of the multi-action
      /// \return The names of the multi-action
      core::identifier_string_list names() const
      {
        using namespace atermpp;
        // return list_arg1(arg1(*this));
        return list_arg1(*this);
      }
  };

  /// \brief Read-only singly linked list of action_name_multiset expressions
  typedef atermpp::term_list<action_name_multiset> action_name_multiset_list;

} // namespace process

} // namespace mcrl2

#endif // MCRL2_PROCESS_ACTION_NAME_MULTISET_H
