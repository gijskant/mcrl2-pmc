#ifndef ITERATORS_H
#define ITERATORS_H

namespace squadt {
  namespace detail {

    /**
     * \brief Interface class for allowing limited external iteration over private members that support STL iterators
     *
     * The value type of the container are assumed to be a pointer type, (or
     * otherwise be dereferencable). The dereference operator of the iterator
     * results in the dereferenced value rather than the pointer.
     *
     * @param T the type of the container type which elements should be iterated
     * @param S the type of an element
     *
     * \attention Works only under the assumption that the list is not changed during iteration.
     **/
    template < typename T, typename S >
    class constant_indirect_iterator {

      private:

        /** \brief The actual iterator */
        typename T::const_iterator iterator;

        /** \brief The past-end-of-sequence value */
        typename T::const_iterator end;

      public:

        /** \brief Constructor */
        constant_indirect_iterator(const T&);

        /** \brief Whether the iterator has moved past the end of the sequence */
        inline bool valid();

        /** \brief Advances to the next element */
        inline void operator++();

        /** \brief Get the element that is currently referenced */
        inline S* operator*() const;
    };

    /**
     * @param c the container with the elements over which to iterate
     **/
    template < typename T, typename S >
    inline constant_indirect_iterator< T, S >::constant_indirect_iterator(const T& c) : iterator(c.begin()), end(c.end()) {
    }
 
    template < typename T, typename S >
    inline bool constant_indirect_iterator< T, S >::valid() {
      return (iterator != end);
    }
 
    template < typename T, typename S >
    inline void constant_indirect_iterator< T, S >::operator++() {
      ++iterator;
    }
 
    template < typename T, typename S >
    inline S* constant_indirect_iterator< T, S >::operator*() const {
      return ((*iterator).get());
    }
  }
}

#endif

