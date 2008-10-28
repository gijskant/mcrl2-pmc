// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/atermpp/aterm.h
/// \brief The aterm class.

#ifndef MCRL2_ATERMPP_ATERM_H
#define MCRL2_ATERMPP_ATERM_H

#include <string> 
#include <iostream>
#include <cassert>
#include "aterm2.h"
#include "mcrl2/atermpp/aterm_traits.h"

namespace atermpp
{
  /// Base class for aterm.
  ///
  class aterm_base
  {
    template <typename T>
    friend struct aterm_traits;
    
    protected:
      /// The wrapped ATerm.
      ATerm m_term;
  
      /// \return A const reference to the wrapped ATerm.
      const ATerm& term() const
      { return m_term; }
      
      /// \return A reference to the wrapped ATerm.
      ATerm& term()
      { return m_term; }

    public:
      /// Constructor.
      ///
      aterm_base()
        : m_term(0)
      {}
      
      /// Constructor.
      ///
      /// \param term A term.
      aterm_base(ATerm term)
        : m_term(term)
      {
      }

      /// Constructor.
      ///
      /// \param term A term containing a list.
      aterm_base(ATermList term)
        : m_term(reinterpret_cast<ATerm>(term))
      {}
  
      /// Constructor.
      ///
      /// \param term A term containing an integer.
      aterm_base(ATermInt term)
        : m_term(reinterpret_cast<ATerm>(term))
      {
      }
  
      /// Constructor.
      ///
      /// \param term A term containing a real.
      aterm_base(ATermReal term)
        : m_term(reinterpret_cast<ATerm>(term))
      {
      }
  
      /// Constructor.
      ///
      /// \param term A term containing a blob.
      aterm_base(ATermBlob term)
        : m_term(reinterpret_cast<ATerm>(term))
      {
      }
  
      /// Constructor.
      ///
      /// \param term A term containing a function application.
      aterm_base(ATermAppl term)
        : m_term(reinterpret_cast<ATerm>(term))
      {
      }

      /// Constructor.
      ///
      /// \param s A string.
      aterm_base(const std::string& s)
        : m_term(ATmake(const_cast<char*>(s.c_str())))
      {}

      /// Protect the aterm.
      /// Protects the aterm from being freed at garbage collection.
      ///
      void protect()
      {
        ATprotect(&m_term);
      }

      /// Unprotect the aterm.
      /// Releases protection of the aterm which has previously been protected through a
      /// call to protect.
      ///
      void unprotect()
      {
        ATunprotect(&m_term);
      }

      /// Mark the aterm for not being garbage collected.
      ///
      void mark()
      {
        ATmarkTerm(m_term);
      }

      /// Return the type of term.
      /// Result is one of AT_APPL, AT_INT,
      /// AT_REAL, AT_LIST, AT_PLACEHOLDER, or AT_BLOB.
      ///
      /// \return The type of the term.
      int type() const
      { return ATgetType(m_term); }
      
      /// Writes the term to a string.
      ///
      /// \return A string representation of the term.
      std::string to_string() const
      { return std::string(ATwriteToString(m_term)); }
  };
 
  /// \cond INTERNAL_DOCS
  template <>                           
  struct aterm_traits<aterm_base>
  {                                     
    typedef ATerm aterm_type;           
    static void protect(aterm_base t)        { t.protect(); }
    static void unprotect(aterm_base t)      { t.unprotect(); }
    static void mark(aterm_base t)           { t.mark(); }
    static ATerm term(aterm_base t)          { return t.term(); }
    static ATerm* ptr(aterm_base& t)         { return &t.term(); }
  };
  /// \endcond

  /// Returns true if x has the default value of an aterm. In the ATerm Library
  /// this value is given by ATfalse.
  /// \param x A term.
  /// \return True if the value of the term is ATfalse.
  inline
  bool operator!(const aterm_base& x)
  {
    return ATisEqual(aterm_traits<aterm_base>::term(x), ATfalse);
  }

  /// Writes a string representation of the aterm t to the stream out.
  /// \param out An output stream.
  /// \param t A term.
  /// \return The stream to which the string representation has been written.
  inline
  std::ostream& operator<<(std::ostream& out, const aterm_base& t)
  {
    return out << t.to_string();
  }

  ///////////////////////////////////////////////////////////////////////////////
  // aterm
  /// \brief Represents a generic term.
  ///
  class aterm: public aterm_base
  {
    public:
      /// Constructor.
      ///
      aterm() {}

      /// Constructor.
      ///
      /// \param term A term.
      aterm(aterm_base term)
        : aterm_base(term)
      { }

      /// Constructor.
      ///
      /// \param term A term.
      aterm(ATerm term)
        : aterm_base(term)
      { }

      /// Constructor.
      ///
      /// \param term A term containing a list.
      aterm(ATermList term)
        : aterm_base(term)
      { }
  
      /// Constructor.
      ///
      /// \param term A term containing an integer.
      aterm(ATermInt term)
        : aterm_base(term)
      { }
  
      /// Constructor.
      ///
      /// \param term A term containing a real.
      aterm(ATermReal term)
        : aterm_base(term)
      { }
  
      /// Constructor.
      ///
      /// \param term A term containing a blob.
      aterm(ATermBlob term)
        : aterm_base(term)
      { }
  
      /// Constructor.
      ///
      /// \param term A term containing a function application.
      aterm(ATermAppl term)
        : aterm_base(term)
      { }

      /// Constructor.
      ///
      /// \param s A string.
      aterm(const std::string& s)
        : aterm_base(s)
      { }

      /// Conversion operator.
      ///
      operator ATerm() const
      { return m_term; } 
  }; 

  /// \cond INTERNAL_DOCS
  template <>                           
  struct aterm_traits<aterm>            
  {                                     
    typedef ATerm aterm_type;           
    static void protect(aterm t)        { t.protect(); }
    static void unprotect(aterm t)      { t.unprotect(); }
    static void mark(aterm t)           { t.mark(); }
    static ATerm term(aterm t)          { return t.term(); }
    static ATerm* ptr(aterm& t)         { return &t.term(); }
  };
  /// \endcond

  /// Read an aterm from string.
  /// This function parses a character string into an aterm.
  ///
  /// \param s A string representation of a term.
  /// \return The term corresponding to the string.
  inline
  aterm read_from_string(const std::string& s)
  {
    return ATreadFromString(s.c_str());
  }
  
  /// Read a aterm from a string in baf format.
  /// This function decodes a baf character string into an aterm.
  ///
  /// \param s A string representation of a term in baf format.
  /// \param size The size of the string.
  /// \return The term corresponding to the string.
  inline
  aterm read_from_binary_string(const std::string& s, unsigned int size)
  {
    return ATreadFromBinaryString(reinterpret_cast<const unsigned char *>(s.c_str()), size);
  }
  
  /// Read a aterm from a string in taf format.
  /// This function decodes a taf character string into an aterm.
  ///
  /// \param s A string.
  /// \param size A positive number.
  /// \return The term read from string.
  inline
  aterm read_from_shared_string(const std::string& s, unsigned int size)
  {
    return ATreadFromSharedString(s.c_str(), size);
  }
  
  /// Read an aterm from named binary or text file.
  /// This function reads an aterm file filename. A test is performed to see if the file
  /// is in baf, taf, or plain text. "-" is standard input's filename.
  ///
  /// \param name The name of a file.
  /// \return A term that was read from a file.
  inline
  aterm read_from_named_file(const std::string& name)
  {
    return ATreadFromNamedFile(name.c_str());
  }

  /// Writes term t to file named filename in textual format.
  /// This function writes aterm t in textual representation to file filename. "-" is
  /// standard output's filename.
  ///
  /// \param t A term.
  /// \param filename The name of a file.
  /// \return True if the operation succeeded.
  inline
  bool write_to_named_text_file(aterm t, const std::string& filename)
  {
    return ATwriteToNamedTextFile(t, filename.c_str()) == ATtrue;
  }

  /// Writes term t to file named filename in Binary aterm Format (baf).
  ///
  /// \param t A term.
  /// \param filename The name of a file.
  /// \return True if the operation succeeded. 
  inline
  bool write_to_named_binary_file(aterm t, const std::string& filename)
  {
    return ATwriteToNamedBinaryFile(t, filename.c_str()) == ATtrue;
  }

  /// Writes term t to file named filename in Streamable aterm Format (saf).
  ///
  /// \param t A term.
  /// \param filename The name of a file.
  /// \return True if the operation succeeded.
  inline
  bool write_to_named_saf_file(aterm t, const std::string& filename)
  {
    return ATwriteToNamedSAFFile(t, filename.c_str()) == ATtrue;
  }

  /// Annotate a term with a labeled annotation.
  /// Creates a version of t that is annotated with annotation and labeled by
  /// label.
  ///
  /// \param t A term.
  /// \param label A label.
  /// \param annotation An annotation.
  /// \return A term with an added annotation.
  inline
  aterm set_annotation(aterm t, aterm label, aterm annotation)
  {
    return ATsetAnnotation(t, label, annotation);
  }

  /// Retrieves annotation of t with label label.
  /// This function can be used to retrieve a specific annotation of a term. If t has
  /// no annotations, or no annotation labeled with label exists, `aterm()` is returned. Otherwise the
  /// annotation is returned.
  ///
  /// \param t A term.
  /// \param label A label.
  /// \return The annotation with the given label.
  inline
  aterm get_annotation(aterm t, aterm label)
  {
    return ATgetAnnotation(t, label);
  }

  /// Remove a specific annotation from a term.
  /// This function returns a version of t which has its annotation with label label
  /// removed. If t has no annotations, or no annotation labeled with label exists, t itself is returned.
  ///
  /// \param t A term.
  /// \param label A label.
  /// \return The term with a removed annotation.
  inline
  aterm remove_annotation(aterm t, aterm label)
  {
    return ATremoveAnnotation(t, label);
  }

  /// Equality operator.
  /// \param x A term.
  /// \param y A term.
  /// \return True if the terms are equal.
  inline
  bool operator==(const aterm& x, const aterm& y)
  {
    return ATisEqual(x, y) == ATtrue;
  }
  
  /// Equality operator.
  /// \param x A term.
  /// \param y A term.
  /// \return True if the terms are equal.
  inline
  bool operator==(const aterm& x, ATerm y)
  {
    return ATisEqual(x, y) == ATtrue;
  }
  
  /// Equality operator.
  /// \param x A term.
  /// \param y A term.
  /// \return True if the terms are equal.
  inline
  bool operator==(const ATerm& x, aterm y)
  {
    return ATisEqual(x, y) == ATtrue;
  }

  /// Inequality operator.
  /// \param x A term.
  /// \param y A term.
  /// \return True if the terms are not equal.
  inline
  bool operator!=(const aterm& x, const aterm& y)
  {
    return ATisEqual(x, y) == ATfalse;
  }
  
  /// Inequality operator.
  /// \param x A term.
  /// \param y A term.
  /// \return True if the terms are not equal.
  inline
  bool operator!=(const aterm& x, ATerm y)
  {
    return ATisEqual(x, y) == ATfalse;
  }
  
  /// Inequality operator.
  /// \param x A term.
  /// \param y A term.
  /// \return True if the terms are not equal.
  inline
  bool operator!=(const ATerm& x, aterm y)
  {
    return ATisEqual(x, y) == ATfalse;
  }

} // namespace atermpp

#include "mcrl2/atermpp/aterm_make_match.h"
#include "mcrl2/atermpp/aterm_init.h"

#endif // MCRL2_ATERMPP_ATERM_H
