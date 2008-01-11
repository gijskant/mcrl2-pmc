// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/atermpp/aterm_list_iterator.h
/// \brief Iterator for term_list.

#ifndef MCRL2_ATERMPP_ATERM_LIST_ITERATOR_H
#define MCRL2_ATERMPP_ATERM_LIST_ITERATOR_H

#include <boost/iterator/iterator_facade.hpp>
#include "mcrl2/atermpp/aterm.h"

namespace atermpp {

  // needed for conversion of the return type of ATgetFirst
  /// \internal
  template <class T>
  struct term_list_iterator_traits
  {
    typedef ATerm value_type;
  };

/// \brief Iterator for term_list.
///
template <typename Value>
class term_list_iterator: public boost::iterator_facade<
        term_list_iterator<Value>,         // Derived
        const Value,                       // Value
        boost::forward_traversal_tag,      // CategoryOrTraversal
        const Value                        // Reference
    >
{
 public:
    /// Constructor.
    ///
    term_list_iterator()
      : m_list(ATempty)
    {}

    /// Constructor.
    ///
    term_list_iterator(ATermList l)
      : m_list(l)
    {}

 private:
    friend class boost::iterator_core_access;

    /// \internal
    bool equal(term_list_iterator const& other) const
    { return this->m_list == other.m_list; }

    /// \internal
    const Value dereference() const
    {
      return Value(typename term_list_iterator_traits<Value>::value_type(ATgetFirst(m_list)));
    }

    /// \internal
    void increment()
    { m_list = ATgetNext(m_list); }

    ATermList m_list;
};

} // namespace atermpp

#endif // MCRL2_ATERMPP_ATERM_LIST_ITERATOR_H
