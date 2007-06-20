// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file atermpp/aterm_blob.h
/// \brief Add your file description here.

#ifndef MCRL2_ATERMPP_ATERM_BLOB_H
#define MCRL2_ATERMPP_ATERM_BLOB_H

#include "atermpp/aterm.h"

namespace atermpp
{
  //---------------------------------------------------------//
  //                     aterm_blob
  //---------------------------------------------------------//
  class aterm_blob: public aterm_base
  {
   public:
      aterm_blob()
      {}

      aterm_blob(ATermBlob t)
        : aterm_base(t)
      {}

      aterm_blob(ATerm t)
        : aterm_base(t)
      {
        assert(type() == AT_BLOB);
      }

      /// Allow construction from an aterm. The aterm must be of the right type.
      ///
      aterm_blob(aterm t)
        : aterm_base(t)
      {
        assert(type() == AT_BLOB);
      }

      /// Build a Binary Large OBject given size (in bytes) and data.
      /// This function can be used to create an aterm of type blob, holding the data
      /// pointed to by data. No copy of this data area is made, so the user should allocate this himself.
      ///    Note:  due to the internal representation of a blob, size cannot exceed 224 in the current
      /// implementation. This limits the size of the data area to 16 Mb.
      ///
      aterm_blob(unsigned int size, void* data)
        : aterm_base(ATmakeBlob(size, data))
      {}

      aterm_blob& operator=(aterm_base t)
      {
        assert(t.type() == AT_BLOB);
        m_term = aterm_traits<aterm_base>::term(t);
        return *this;
      }

      /// Get the data section of the blob.
      ///
      void* data()
      {
        return ATgetBlobData(void2blob(m_term));
      }

      /// Get the size (in bytes) of the blob.
      ///
      unsigned int size() const
      {
        return ATgetBlobSize(void2blob(m_term));
      }
  };

  template <>
  struct aterm_traits<aterm_blob>
  {
    typedef ATermBlob aterm_type;
    static void protect(aterm_blob t)   { t.protect(); }
    static void unprotect(aterm_blob t) { t.unprotect(); }
    static void mark(aterm_blob t)      { t.mark(); }
    static ATerm term(aterm_blob t)     { return t.term(); }
    static ATerm* ptr(aterm_blob t)     { return &t.term(); }
  };

} // namespace atermpp

#endif // MCRL2_ATERMPP_ATERM_BLOB_H
