// ======================================================================
//
// Copyright (c) 2004, 2005 Wieger Wesselink
//
// ----------------------------------------------------------------------
//
// file          : atermpp/aterm_appl.h
// date          : 25-10-2005
// version       : 1.0
//
// author(s)     : Wieger Wesselink  <J.W.Wesselink@tue.nl>
//
// ======================================================================

///////////////////////////////////////////////////////////////////////////////
/// \file aterm_appl.h
/// Contains the definition of the aterm_appl class.

#ifndef ATERM_APPL_H
#define ATERM_APPL_H

#include <cassert>
#include "atermpp/aterm.h"
#include "atermpp/detail/aterm_conversion.h"
#include "atermpp/aterm_list.h"
#include "atermpp/function_symbol.h"

namespace atermpp
{
  //---------------------------------------------------------//
  //                     aterm_appl
  //---------------------------------------------------------//
  class aterm_appl: public aterm
  {
    public:
      aterm_appl()
      {}
  
      aterm_appl(ATerm t)
        : aterm(t)
      {
        assert(type() == AT_APPL);
      }

      aterm_appl(ATermAppl a)
        : aterm(a)
      {}

      template <typename Term>
      aterm_appl(function_symbol sym, term_list<Term> args)
        : aterm(ATmakeApplList(sym, args))
      {}

      /// Allow construction from an aterm. The aterm must be of the right type.
      ///
      aterm_appl(aterm t)
        : aterm(t)
      {}

#include "atermpp/aterm_appl_constructor.h" // additional constructors generated by preprocessor

      /// Get the function symbol (function_symbol) of the application.
      ///
      function_symbol function() const
      {
        return function_symbol(ATgetAFun(void2appl(m_term)));
      }
  
      bool is_quoted() const
      {
        return function().is_quoted();
      }
  
      /// Get the i-th argument of the application.
      ///
      aterm argument(unsigned int i) const
      {
        return aterm(ATgetArgument(void2appl(m_term), i));
      }
  
      /// Get the list of arguments of the application.
      ///
      aterm_list argument_list() const
      {
        return aterm_list(ATgetArguments(void2appl(m_term)));
      }

      /// Conversion to ATermAppl.
      ///
      operator ATermAppl() const
      {
        return void2appl(m_term);
      }
  };

////  /// Set the i-th argument of an application to term.
//  /// This function returns a copy of appl with argument i replaced by term.
//  ///
//  inline
//  aterm_appl set_appl_argument(aterm_appl appl, unsigned int i, aterm term)
//  {
//    return ATsetArgument(appl, term, i);
//  }   
//
////  /// Returns a quoted string.
//  ///
//  inline
//  aterm_appl quoted_string(std::string s)
//  {
//    // TODO: this should be done more efficiently!
//    return aterm_appl("\"" + s + "\"");
//  }

  /// INTERNAL ONLY
  inline
  ATerm aterm_ptr(aterm_appl& t)
  {
    return t;
  }
  
  /// INTERNAL ONLY
  inline
  ATerm aterm_ptr(const aterm_appl& t)
  {
    return t;
  }

   template <>
   class aterm_protect_traits<aterm_appl>
   {
     public:
       static void protect(aterm_appl t)
       {
#ifdef ATERM_DEBUG_PROTECTION
std::cout << "aterm_protect_traits<aterm_appl>::protect() " << t << std::endl;
#endif // ATERM_DEBUG_PROTECTION
         t.protect();
       }

       static void unprotect(aterm_appl t)
       {
#ifdef ATERM_DEBUG_PROTECTION
std::cout << "aterm_protect_traits<aterm_appl>::unprotect() " << t << std::endl;
#endif // ATERM_DEBUG_PROTECTION
         t.unprotect();
       }

       static void mark(aterm_appl t)
       {
#ifdef ATERM_DEBUG_PROTECTION
std::cout << "aterm_protect_traits<aterm_appl>::mark() " << t << std::endl;
#endif // ATERM_DEBUG_PROTECTION
         t.mark();
       }
   };

} // namespace atermpp

#endif // ATERM_APPL_H
