// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING).
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/atermpp/aterm_appl.h
/// \brief The term_appl class represents function application.

#ifndef MCRL2_ATERMPP_ATERM_APPL_H
#define MCRL2_ATERMPP_ATERM_APPL_H

#include <cassert>
#include <vector>
#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_list.h"
#include "mcrl2/atermpp/function_symbol.h"
#include "mcrl2/atermpp/aterm_appl_iterator.h"

/// Namespace containing all ATerm++ functionality
namespace atermpp
{
  /// A term that models function application. The template argument
  /// denotes the type of the arguments.
  ///
  template <typename Term>
  class term_appl: public aterm_base
  {
    friend class aterm_string;

    private:
      // prevent accidental usage of operator[], since this maps to the
      // built-in C++ operator[](ATermAppl, int)
      Term operator[](unsigned int i) const
      {
        return Term();
      }

    protected:
      const ATermAppl appl() const
      {
        return reinterpret_cast<const ATermAppl>(m_term);
      }

    public:
      /// The type of object, T stored in the term_appl.
      ///
      typedef Term value_type;

      /// Pointer to T.
      ///
      typedef Term* pointer;

      /// Reference to T.
      ///
      typedef Term& reference;

      /// Const reference to T.
      ///
      typedef const Term const_reference;

      /// An unsigned integral type.                                      
      ///
      typedef size_t size_type;

      /// A signed integral type.                                         
      ///
      typedef ptrdiff_t difference_type;

      /// Iterator used to iterate through an term_appl.                      
      ///
      typedef term_appl_iterator<Term> iterator;

      /// Const iterator used to iterate through an term_appl.                
      ///
      typedef term_appl_iterator<Term> const_iterator;

      /// Default constructor.
      ///
      term_appl()
      {}

      /// Constructor.
      ///
      term_appl(ATerm term)
        : aterm_base(term)
      {
        assert(type() == AT_APPL);
      }

      /// Constructor.
      ///
      term_appl(ATermAppl term)
        : aterm_base(term)
      {}

      /// Constructor.
      ///
      term_appl(function_symbol sym, term_list<Term> args)
        : aterm_base(ATmakeApplList(sym, args))
      {}

      /// Allow construction from an aterm. The aterm must be of the right type.
      ///
      term_appl(aterm t)
        : aterm_base(t)
      {}

      /// Constructor.
      ///
      template <typename Iter>
      term_appl(function_symbol sym, Iter first, Iter last)
      {
        std::vector<ATerm> arguments;
        for (Iter i = first; i != last; ++i)
        {
          arguments.push_back(aterm_traits<typename std::iterator_traits<Iter>::value_type>::term(*i));
        }
        m_term = ATmakeApplArray(sym, &(arguments.front()));
      }

      /// Constructor.
      ///
      term_appl(function_symbol sym)
        : aterm_base(ATmakeAppl0(sym))
      {
      }

#include "mcrl2/atermpp/aterm_appl_constructor.h" // additional constructors generated by preprocessor

      /// Conversion to ATermAppl.
      ///
      operator ATermAppl() const
      {
        return reinterpret_cast<ATermAppl>(m_term);
      }

      /// Assignment operator.
      ///
      term_appl<Term>& operator=(aterm_base t)
      {
        assert(t.type() == AT_APPL);
        m_term = aterm_traits<aterm_base>::term(t);
        return *this;
      }

      /// Assignment operator.
      ///
      term_appl<Term>& operator=(ATermAppl t)
      {
        m_term = reinterpret_cast<ATerm>(t);
        return *this;
      }

      /// Returns the size of the term_list.
      ///
      size_type size() const
      { return ATgetArity(ATgetAFun(appl())); }

      /// Returns an iterator pointing to the end of the term_list.     
      ///
      const_iterator begin() const
      {
        return const_iterator(((ATerm *)(m_term) + ARG_OFFSET));
      } 

      /// Returns a const_iterator pointing to the beginning of the term_list.
      ///
      const_iterator end() const
      {
        return const_iterator(((ATerm *)(m_term) + ARG_OFFSET + size()));
      }
  
      /// Returns the largest possible size of the term_list.
      ///
      size_type max_size() const
      { return (std::numeric_limits<unsigned long>::max)(); }

      /// true if the list's size is 0.
      ///
      bool empty() const
      { return size() == 0; }

      /// Get the function symbol (function_symbol) of the application.
      ///
      function_symbol function() const
      {
        return function_symbol(ATgetAFun(appl()));
      }
 
      /// Returns true if the term is quoted.
      /// 
      bool is_quoted() const
      {
        return function().is_quoted();
      }

      /// Returns the i-th argument.
      /// 
      Term operator()(unsigned int i) const
      {
        return Term(ATgetArgument(appl(), i));
      }
  
      /// Returns a copy of the term with the i-th child replaced by t.
      /// DEPRECATED!
      ///
      term_appl<Term> set_argument(Term t, unsigned int i)
      {
        return ATsetArgument(appl(), t, i);
      }

      /// Get the i-th argument of the application.
      /// DEPRECATED!
      ///
      aterm argument(unsigned int i) const
      {
        return aterm(ATgetArgument(appl(), i));
      }

      /// Get the list of arguments of the application.
      /// DEPRECATED!
      ///
      term_list<Term> argument_list() const
      {
        return term_list<Term>(ATgetArguments(appl()));
      }
  };

  /// A term_appl with children of type aterm.
  ///
  typedef term_appl<aterm> aterm_appl;

  /// \cond INTERNAL_DOCS
  template <typename Term>
  struct aterm_traits<term_appl<Term> >
  {
    typedef ATermAppl aterm_type;
    static void protect(term_appl<Term> t)   { t.protect(); }
    static void unprotect(term_appl<Term> t) { t.unprotect(); }
    static void mark(term_appl<Term> t)      { t.mark(); }
    static ATerm term(term_appl<Term> t)     { return t.term(); }
    static ATerm* ptr(term_appl<Term>& t)    { return &t.term(); }
  };
  /// \endcond

  /// Equality operator.
  ///
  template <typename Term>
  bool operator==(const term_appl<Term>& x, const term_appl<Term>& y)
  {
    return ATisEqual(aterm_traits<term_appl<Term> >::term(x), aterm_traits<term_appl<Term> >::term(y)) == ATtrue;
  }
  
  /// Equality operator.
  ///
  template <typename Term>
  bool operator==(const term_appl<Term>& x, ATermAppl y)
  {
    return ATisEqual(aterm_traits<term_appl<Term> >::term(x), y) == ATtrue;
  }
  
  /// Equality operator.
  ///
  template <typename Term>
  bool operator==(ATermAppl x, const term_appl<Term>& y)
  {
    return ATisEqual(x, aterm_traits<term_appl<Term> >::term(y)) == ATtrue;
  }

  /// Inequality operator.
  ///
  template <typename Term>
  bool operator!=(const term_appl<Term>& x, const term_appl<Term>& y)
  {
    return ATisEqual(aterm_traits<term_appl<Term> >::term(x), aterm_traits<term_appl<Term> >::term(y)) == ATfalse;
  }
  
  /// Inequality operator.
  ///
  template <typename Term>
  bool operator!=(const term_appl<Term>& x, ATermAppl y)
  {
    return ATisEqual(aterm_traits<term_appl<Term> >::term(x), y) == ATfalse;
  }
  
  /// Inequality operator.
  ///
  template <typename Term>
  bool operator!=(ATermAppl x, const term_appl<Term>& y)
  {
    return ATisEqual(x, aterm_traits<term_appl<Term> >::term(y)) == ATfalse;
  }

} // namespace atermpp

#endif // MCRL2_ATERMPP_ATERM_APPL_H
