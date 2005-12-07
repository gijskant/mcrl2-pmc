// ======================================================================
//
// Copyright (c) 2004, 2005 Wieger Wesselink
//
// ----------------------------------------------------------------------
//
// file          : atermpp/aterm_make_match.h
// date          : 25-10-2005
// version       : 1.0
//
// author(s)     : Wieger Wesselink  <J.W.Wesselink@tue.nl>
//
// ======================================================================

///////////////////////////////////////////////////////////////////////////////
/// \file atermpp/aterm_make_match.h
/// This file contains implementations for aterm_make and aterm_match.

#ifndef ATERM_MAKE_MATCH_H
#define ATERM_MAKE_MATCH_H

#include "atermpp/aterm.h"

namespace atermpp {

  //---------------------------------------------------------//
  //         conversion between pointer and reference
  //         (needed for aterm_make/aterm_match)
  //---------------------------------------------------------//
  template <typename T>
  struct aterm_conversion
  {
    typedef T  value_type;
    typedef T* pointer;
  };
  
  // specialization for aterm
  template <>
  struct aterm_conversion<atermpp::aterm>
  {
    typedef ATerm  value_type;
    typedef ATerm* pointer;
  };

  //---------------------------------------------------------//
  //             aterm_ptr
  //---------------------------------------------------------//

  template <typename T>
  typename aterm_conversion<T>::value_type aterm_ptr(T& t)
  {
    return t;
  }

  inline
  ATerm aterm_ptr(atermpp::aterm& t)
  {
    return t;
  }
  
  inline
  ATerm aterm_ptr(const atermpp::aterm& t)
  {
    return t;
  }

  //---------------------------------------------------------//
  //             aterm_ptr_ref
  //---------------------------------------------------------//

  template <typename T>
  typename aterm_conversion<T>::pointer aterm_ptr_ref(T& t)
  {
    return &t;
  }
  
  inline
  ATerm* aterm_ptr_ref(atermpp::aterm& t)
  {
    return &t.term();
  }
  
  inline
  const ATerm* aterm_ptr_ref(const atermpp::aterm& t)
  {
    return &t.term();
  }

} // namespace atermpp

namespace atermpp
{
  //---------------------------------------------------------//
  //             aterm_make / aterm_match
  //---------------------------------------------------------//
  
  inline
  aterm make_term(const std::string& pattern)
  {
    return aterm(ATreadFromString(const_cast<char*>(pattern.c_str())));
  }
  
#include "atermpp/aterm_make.h"    // generated by preprocessor
#include "atermpp/aterm_match.h"   // generated by preprocessor

} // namespace atermpp

#endif // ATERM_MAKE_MATCH_H
