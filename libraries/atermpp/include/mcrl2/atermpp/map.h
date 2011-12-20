// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/atermpp/map.h
/// \brief Protected map and multimap containers.

#ifndef MCRL2_ATERMPP_MAP_H
#define MCRL2_ATERMPP_MAP_H

#include <functional>
#include <memory>
#include <map>
#include "mcrl2/aterm/protaterm.h"          // IProtectedATerm
#include "mcrl2/atermpp/aterm_traits.h"


namespace atermpp
{

/// \brief Protected map container.
template<class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key,T> > >
class map: public std::map<Key, T, Compare, Allocator>, IProtectedATerm
{
  public:
    /// \brief Constructor.
    map()
      : IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param comp A compare function.
    explicit map(const Compare& comp)
      : std::map<Key, T, Compare, Allocator>(comp),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param comp A compare function.
    /// \param a An allocator.
    map(const Compare& comp, const Allocator& a)
      : std::map<Key, T, Compare, Allocator>(comp, a),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param right A map.
    map(const map& right)
      : std::map<Key, T, Compare, Allocator>(right),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param first The start of a range of map elements.
    /// \param last The end of a range of map elements.
    template<class InIt>
    map(InIt first, InIt last)
      : std::map<Key, T, Compare, Allocator>(first, last),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param first The start of a range of map elements.
    /// \param last The end of a range of map elements.
    /// \param comp A compare function.
    template<class InIt>
    map(InIt first, InIt last, const Compare& comp)
      : std::map<Key, T, Compare, Allocator>(first, last, comp),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param first The start of a range of map elements.
    /// \param last The end of a range of map elements.
    /// \param comp A compare function.
    /// \param a An allocator.
    template<class InIt>
    map(InIt first, InIt last, const Compare& comp, const Allocator& a)
      : std::map<Key, T, Compare, Allocator>(first, last, comp, a),
        IProtectedATerm(this)
    {
    }

    /// Destructor.
    virtual ~map()
    {
    }

    /// \brief Protects the elements from being garbage collected.
    void ATmarkTerms()
    {
#ifdef ATERM_DEBUG_PROTECTION
      std::cout << "atermpp::map.ATmarkTerms() : protecting " << std::map<Key, T, Compare, Allocator>::size() << " elements" << std::endl;
#endif // ATERM_DEBUG_PROTECTION
      for (typename std::map<Key, T, Compare, Allocator>::iterator i = std::map<Key, T, Compare, Allocator>::begin(); i != std::map<Key, T, Compare, Allocator>::end(); ++i)
      {
        aterm_traits<Key>::mark(i->first);
        aterm_traits<T>::mark(i->second);
      }
    }
};

/// \brief Protected multimap container.
template<class Key, class T, class Compare = std::less<Key>, class Allocator = std::allocator<std::pair<const Key,T> > >
class multimap: public std::multimap<Key, T, Compare, Allocator>, IProtectedATerm
{
  public:
    /// \brief Constructor.
    multimap()
      : IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param comp A compare function.
    explicit multimap(const Compare& comp)
      : std::multimap<Key, T, Compare, Allocator>(comp),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param comp A compare function.
    /// \param a An allocator.
    multimap(const Compare& comp, const Allocator& a)
      : std::multimap<Key, T, Compare, Allocator>(comp, a),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param right A map.
    multimap(const multimap& right)
      : std::multimap<Key, T, Compare, Allocator>(right),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param first The start of a range of map elements.
    /// \param last The end of a range of map elements.
    template<class InIt>
    multimap(InIt first, InIt last)
      : std::multimap<Key, T, Compare, Allocator>(first, last),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param first The start of a range of map elements.
    /// \param last The end of a range of map elements.
    /// \param comp A compare function.
    template<class InIt>
    multimap(InIt first, InIt last, const Compare& comp)
      : std::multimap<Key, T, Compare, Allocator>(first, last, comp),
        IProtectedATerm(this)
    {
    }

    /// \brief Constructor.
    /// \param first The start of a range of map elements.
    /// \param last The end of a range of map elements.
    /// \param comp A compare function.
    /// \param a An allocator.
    template<class InIt>
    multimap(InIt first, InIt last, const Compare& comp, const Allocator& a)
      : std::multimap<Key, T, Compare, Allocator>(first, last, comp, a),
        IProtectedATerm(this)
    {
    }

    /// Destructor.
    virtual ~multimap()
    {
    }

    /// \brief Protects the elements from being garbage collected.
    void ATmarkTerms()
    {
#ifdef ATERM_DEBUG_PROTECTION
      std::cout << "atermpp::multimap.ATmarkTerms() : protecting " << multimap<Key, T, Compare, Allocator>::size() << " elements" << std::endl;
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
