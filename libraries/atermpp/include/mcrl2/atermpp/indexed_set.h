// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/atermpp/indexed_set.h
/// \brief Add your file description here.

#ifndef MCRL2_ATERMPP_INDEXED_SET_H
#define MCRL2_ATERMPP_INDEXED_SET_H

#include <boost/shared_ptr.hpp>
#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_list.h"

namespace atermpp
{
  struct indexed_set_deleter
  {
    void operator()(ATermIndexedSet s)
    {
      ATindexedSetDestroy(s);
    }
  }; 
  
  //---------------------------------------------------------//
  //                     indexed_set
  //---------------------------------------------------------//
  class indexed_set
  {
   protected:
     boost::shared_ptr<_ATermTable> m_set;

   public:
      /// Create a new indexed_set.
      ///
      indexed_set(unsigned int initial_size = 100, unsigned int max_load_pct = 75)
        : m_set(ATindexedSetCreate(initial_size, max_load_pct), indexed_set_deleter())
      {}
      
      /// Clear the hash table in the set.
      /// This function clears the hash table in the set, but does not release the memory.
      /// Using indexed_set_reset instead of indexed_set_destroy is preferable when indexed sets of
      /// approximately the same size are being used.
      ///
      void reset()
      {
        ATindexedSetReset(m_set.get());
      }
      
      /// Enter elem into the set.
      /// This functions enters elem into the set. If elem was already in the set the previously
      /// assigned index of elem is returned, and new is set to false. If elem did not yet occur in set a
      /// new number is assigned, and new is set to true.  This number can either be the number of an
      /// element that has been removed, or, if such a number is not available, the lowest non used number
      /// is assigned to elem and returned. The lowest number that is used is 0.
      ///
      std::pair<long, bool> put(aterm elem)
      {
        ATbool b;
        long l = ATindexedSetPut(m_set.get(), elem, &b);
        return std::make_pair(l, b == ATtrue);
      }
      
      /// Find the index of elem in set.
      /// The index assigned to elem is returned, except when elem is not in the set, in
      /// which case the return value is a negative number.
      ///
      long index(aterm elem)
      {
        return ATindexedSetGetIndex(m_set.get(), elem);
      }
      
      /// Retrieve the element at index in set.
      /// This function must be invoked with a valid index and it returns the elem assigned
      /// to this index. If it is invoked with an invalid index, effects are not predictable.
      ///
      aterm get(long index)
      {
        return ATindexedSetGetElem(m_set.get(), index);
      }
      
      /// Remove elem from set.
      /// The elem is removed from the indexed set, and if a number was assigned to elem,
      /// it is freed to be reassigned to an element, that may be put into the set at some later instance.
      ///
      void remove(aterm elem)
      {
        ATindexedSetRemove(m_set.get(), elem);
      }
      
      /// Retrieve all elements in set.
      /// A list with all valid elements stored in the indexed set is returned.  The list is
      /// ordered from element with index 0 onwards.
      ///
      aterm_list elements()
      {
        return aterm_list(ATindexedSetElements(m_set.get()));
      }
  };

} // namespace atermpp

#endif // MCRL2_ATERMPP_INDEXED_SET_H
