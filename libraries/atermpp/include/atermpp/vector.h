// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file atermpp/vector.h
/// \brief Add your file description here.

#ifndef MCRL2_ATERMPP_VECTOR_H
#define MCRL2_ATERMPP_VECTOR_H

#include <memory>
#include <vector>
#include "atermpp/aterm_traits.h"
#include "protaterm.h"          // IProtectedATerm

namespace atermpp {

template <class T, class Allocator = std::allocator<T> >
class vector: public std::vector<T, Allocator>, IProtectedATerm
{
  public:
    vector()
    {
      ATprotectProtectedATerm(this);
    }

    explicit vector(const Allocator& a)
      : std::vector<T, Allocator>(a)
    {
      ATprotectProtectedATerm(this);
    }

    explicit vector(typename std::vector<T, Allocator>::size_type count)
      : std::vector<T, Allocator>(count)
    {
      ATprotectProtectedATerm(this);
    }
    
    vector(typename std::vector<T, Allocator>::size_type count, const T& val)
      : std::vector<T, Allocator>(count, val)
    {
      ATprotectProtectedATerm(this);
    }

    vector(typename std::vector<T, Allocator>::size_type count, const T& val, const Allocator& a)
      : std::vector<T, Allocator>(count, val, a)
    {
      ATprotectProtectedATerm(this);
    }

    vector(const vector& right)
      : std::vector<T, Allocator>(right)
    {
      ATprotectProtectedATerm(this);
    }

    template<class InIt>
        vector(InIt first, InIt last)
      : std::vector<T, Allocator>(first, last)
    {
      ATprotectProtectedATerm(this);
    }

    template<class InIt>
        vector(InIt first, InIt last, const Allocator& a)
      : std::vector<T, Allocator>(first, last, a)
    {
      ATprotectProtectedATerm(this);
    }

    ~vector()
    {
      ATunprotectProtectedATerm(this);
    }

    void ATprotectTerms()
    {
#ifdef ATERM_DEBUG_PROTECTION
std::cout << "atermpp::vector.ATprotectTerms() : protecting " << vector<T>::size() << " elements" << std::endl;
#endif // ATERM_DEBUG_PROTECTION
      for (typename std::vector<T, Allocator>::iterator i = std::vector<T, Allocator>::begin(); i != std::vector<T, Allocator>::end(); ++i)
      {
        aterm_traits<T>::mark(*i);
      }
    }
};

} // namespace atermpp

#endif // MCRL2_ATERMPP_VECTOR_H
