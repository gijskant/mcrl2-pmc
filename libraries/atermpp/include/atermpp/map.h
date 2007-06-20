// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file atermpp/map.h
/// \brief Add your file description here.

#ifndef MCRL2_ATERMPP_MAP_H
#define MCRL2_ATERMPP_MAP_H

#include <functional>
#include <memory>
#include <map>
#include "atermpp/aterm_traits.h"
#include "protaterm.h"          // IProtectedATerm

namespace atermpp {

template<class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key,T> > >
class map: public std::map<Key, T, Compare, Allocator>, IProtectedATerm
{
  public:
    map()
    {
      ATprotectProtectedATerm(this);
    }

    explicit map(const Compare& comp)
      : std::map<Key, T, Compare, Allocator>(comp)
    {
      ATprotectProtectedATerm(this);
    }

    map(const Compare& comp, const Allocator& a)
      : std::map<Key, T, Compare, Allocator>(comp, a)
    {
      ATprotectProtectedATerm(this);
    }

    map(const map& right)
      : std::map<Key, T, Compare, Allocator>(right)
    {
      ATprotectProtectedATerm(this);
    }

    template<class InIt>
    map(InIt first, InIt last)
      : std::map<Key, T, Compare, Allocator>(first, last)
    {
      ATprotectProtectedATerm(this);
    }

    template<class InIt>
    map(InIt first, InIt last, const Compare& comp)
      : std::map<Key, T, Compare, Allocator>(first, last, comp)
    {
      ATprotectProtectedATerm(this);
    }

    template<class InIt>
    map(InIt first, InIt last, const Compare& comp, const Allocator& a)
      : std::map<Key, T, Compare, Allocator>(first, last, comp, a)
    {
      ATprotectProtectedATerm(this);
    }
   
    ~map()
    {
      ATunprotectProtectedATerm(this);
    }

    void ATprotectTerms()
    {
#ifdef ATERM_DEBUG_PROTECTION
std::cout << "atermpp::map.ATprotectTerms() : protecting " << std::map<Key, T, Compare, Allocator>::size() << " elements" << std::endl;
#endif // ATERM_DEBUG_PROTECTION
      for (typename std::map<Key, T, Compare, Allocator>::iterator i = std::map<Key, T, Compare, Allocator>::begin(); i != std::map<Key, T, Compare, Allocator>::end(); ++i)
      {
        aterm_traits<Key>::mark(i->first);
        aterm_traits<T>::mark(i->second);
      }
    }
};

template<class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key,T> > >
class multimap: public std::multimap<Key, T, Compare, Allocator>, IProtectedATerm
{
  public:
    multimap()
    {
      ATprotectProtectedATerm(this);
    }

    explicit multimap(const Compare& comp)
      : std::multimap<Key, T, Compare, Allocator>(comp)
    {
      ATprotectProtectedATerm(this);
    }

    multimap(const Compare& comp, const Allocator& a)
      : std::multimap<Key, T, Compare, Allocator>(comp, a)
    {
      ATprotectProtectedATerm(this);
    }

    multimap(const multimap& right)
      : std::multimap<Key, T, Compare, Allocator>(right)
    {
      ATprotectProtectedATerm(this);
    }

    template<class InIt>
    multimap(InIt first, InIt last)
      : std::multimap<Key, T, Compare, Allocator>(first, last)
    {
      ATprotectProtectedATerm(this);
    }

    template<class InIt>
    multimap(InIt first, InIt last, const Compare& comp)
      : std::multimap<Key, T, Compare, Allocator>(first, last, comp)
    {
      ATprotectProtectedATerm(this);
    }

    template<class InIt>
    multimap(InIt first, InIt last, const Compare& comp, const Allocator& a)
      : std::multimap<Key, T, Compare, Allocator>(first, last, comp, a)
    {
      ATprotectProtectedATerm(this);
    }
   
    ~multimap()
    {
      ATunprotectProtectedATerm(this);
    }

    void ATprotectTerms()
    {
#ifdef ATERM_DEBUG_PROTECTION
std::cout << "atermpp::multimap.ATprotectTerms() : protecting " << multimap<Key, T, Compare, Allocator>::size() << " elements" << std::endl;
#endif // ATERM_DEBUG_PROTECTION
      for (typename std::multimap<Key, T, Compare, Allocator>::iterator i = std::multimap<Key, T, Compare, Allocator>::begin(); i != std::multimap<Key, T, Compare, Allocator>::end(); ++i)
      {
        aterm_traits<Key>::mark(i->first);
        aterm_traits<T>::mark(i->second);
      }
    }
};

} // namespace atermpp

#endif // MCRL2_ATERMPP_MAP_H
