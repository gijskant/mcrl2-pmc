// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/sort_identifier.h
/// \brief Class sort_identifier.

#ifndef MCRL2_DATA_SORT_ID_H
#define MCRL2_DATA_SORT_ID_H

#include "mcrl2/data/sort_expression.h"
#include "mcrl2/core/identifier_string.h"

namespace mcrl2 {

namespace data {

class sort_identifier;

/// \brief singly linked list of sorts
///
typedef atermpp::term_list<sort_identifier> sort_identifier_list;

/// \brief sort id.
///
//<SortId>       ::= SortId(<String>)
class sort_identifier: public sort_expression
{
  public:
    /// Constructor.
    ///
    sort_identifier()
      : sort_expression(core::detail::constructSortId())
    {}

    /// Constructor.
    ///
    /// \param t A term containing a sort identifier.
    sort_identifier(ATermAppl t)
      : sort_expression(t)
    {
      assert(core::detail::check_rule_SortId(m_term));
    }

    /// Constructor.
    ///
    /// \param t A term containing a sort identifier.
    sort_identifier(atermpp::aterm_appl t)
      : sort_expression(t)
    {
      assert(core::detail::check_rule_SortId(m_term));
    }

    /// Constructor.
    ///
    /// \param name The name of a sort identifier.
    sort_identifier(core::identifier_string name)
      : sort_expression(core::detail::gsMakeSortId(name))
    {
      assert(core::detail::check_rule_SortId(m_term));
    }

    /// Constructor.
    ///
    /// \param s A string representation of a sort identifier.
    sort_identifier(std::string s)
      : sort_expression(core::detail::gsMakeSortId(core::detail::gsString2ATermAppl(s.c_str())))
    {}

    /// Returns the name of the sort identifier.
    ///
    /// \return The name of the sort identifier.
    core::identifier_string name() const
    {
      return atermpp::arg1(*this);
    }
    
    /// \overload
    ///
    /// \return Always returns false.
    bool is_arrow() const
    {
      return false;
    }
};

/// \brief Returns true if the term t is a sort identifier.
inline
bool is_sort_identifier(atermpp::aterm_appl t)
{
  return core::detail::gsIsSortId(t);
}

} // namespace data

} // namespace mcrl2

/// \cond INTERNAL_DOCS
MCRL2_ATERM_TRAITS_SPECIALIZATION(mcrl2::data::sort_identifier)
/// \endcond

#endif // MCRL2_DATA_SORT_ID_H
