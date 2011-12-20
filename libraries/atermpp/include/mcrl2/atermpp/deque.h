// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/atermpp/deque.h
/// \brief Protected deque container.

#ifndef MCRL2_ATERMPP_DEQUE_H
#define MCRL2_ATERMPP_DEQUE_H

#include <memory>
#include <deque>
#include "mcrl2/aterm/protaterm.h"          // IProtectedATerm
#include "mcrl2/atermpp/aterm_traits.h"

namespace atermpp
{

/// \brief Protected deque container.
template <class T, class Allocator = std::allocator<T> >
class deque: public std::deque<T, Allocator>, IProtectedATerm
{
  public:
    /// \brief Constructor.
    deque()
      : IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param a An allocator.
    explicit deque(const Allocator& a)
      : std::deque<T, Allocator>(a),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param count A positive number.
    explicit deque(typename std::deque<T,Allocator>::size_type count)
      : std::deque<T, Allocator>(count),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param count A positive number.
    /// \param val A value.
    deque(typename std::deque<T,Allocator>::size_type count, const T& val)
      : std::deque<T, Allocator>(count, val),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param count A positive number.
    /// \param val A value.
    /// \param a An allocator.
    deque(typename std::deque<T,Allocator>::size_type count, const T& val, const Allocator& a)
      : std::deque<T, Allocator>(count, val, a),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param right A deque.
    deque(const deque& right)
      : std::deque<T, Allocator>(right),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param first The start of a range of elements.
    /// \param last The end of a range of elements.
    template<class InIt>
    deque(InIt first, InIt last)
      : std::deque<T, Allocator>(first, last),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param first The start of a range of elements.
    /// \param last The end of a range of elements.
    /// \param a An allocator.
    template<class InIt>
    deque(InIt first, InIt last, const Allocator& a)
      : std::deque<T, Allocator>(first, last, a),
        IProtectedATerm(this)
    {
    }

    /// Destructor.
    ~deque()
    {
    }

    /// \brief Protects the elements from being garbage collected.
    void ATmarkTerms()
    {
#ifdef ATERM_DEBUG_PROTECTION
      std::cout << "atermpp::deque.ATmarkTerms() : protecting " << deque<T>::size() << " elements" << std::endl;
#endif // ATERM_DEBUG_PROTECTION
      for (typename std::deque<T, Allocator>::iterator i = std::deque<T, Allocator>::begin(); i != std::deque<T, Allocator>::end(); ++i)
      {
        aterm_traits<T>::mark(*i);
      }
    }
};

} // namespace atermpp

#endif // MCRL2_ATERMPP_DEQUE_H
