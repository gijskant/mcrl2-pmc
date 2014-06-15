// Author(s): Wieger Wesselink, Jan Friso Groote. Based on the aterm library by Paul Klint and others.
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
#include <functional>

#include "boost/type_traits/is_base_of.hpp"
#include "mcrl2/atermpp/detail/aterm.h"

/// \brief The main namespace for the aterm++ library.
namespace atermpp
{

typedef void(*term_callback)(const aterm&);

void add_creation_hook(const function_symbol&, term_callback);
void add_deletion_hook(const function_symbol&, term_callback);

class aterm
{
  public:
    template < typename T >
    friend class term_appl;

    template < typename T >
    friend class term_list;

    friend void detail::free_term_aux(const detail::_aterm* t, const detail::_aterm*& terms_to_be_removed);

    friend void detail::initialise_aterm_administration();

    template <class Term, class Iter, class ATermConverter>
    friend const detail::_aterm *detail::make_list_backward(Iter first, Iter last, const ATermConverter &convert_to_aterm);

    template <class Term, class Iter, class ATermConverter>
    friend const detail::_aterm *detail::make_list_forward(Iter first, Iter last, const ATermConverter &convert_to_aterm);

    friend const detail::_aterm* detail::address(const aterm &t);
  protected:
    const detail::_aterm* m_term;

    static const detail::_aterm *undefined_aterm();
    static const detail::_aterm *empty_aterm_list();

    inline size_t decrease_reference_count() const
    {
      assert(m_term!=NULL);
      assert(m_term->reference_count()>0);
      m_term->decrease_reference_count();
      return m_term->reference_count();
    }

    template <bool CHECK>
    void increase_reference_count() const
    {
      assert(m_term!=NULL);
      if (CHECK) assert(m_term->reference_count()>0);
      m_term->increase_reference_count();
    }

    void copy_term(const aterm &t)
    {
      t.increase_reference_count<true>();
      decrease_reference_count();
      m_term=t.m_term;
    }

    // An aterm has a function symbol, which can also be an AS_EMPTY_LIST,
    // AS_INT and AS_LIST.
    const function_symbol &function() const
    {
      return m_term->function();
    }

  public: // Should be protected, but this cannot yet be done due to a problem
          // in the compiling rewriter.
    aterm (const detail::_aterm *t):m_term(t)
    {
      // Note that reference_count can be 0, as this term can just be constructed,
      // and is now handed over to become a real aterm.
      increase_reference_count<false>();
    }

  public:

    /// \brief Default constructor
    aterm():m_term(undefined_aterm())
    {
      increase_reference_count<false>();
    }

    /// \brief Copy constructor
    aterm(const aterm &t):m_term(t.m_term)
    {
      increase_reference_count<true>();
    }

    /// \brief Assignment operator.
    /// \param t a term to be assigned.
    aterm &operator=(const aterm &t)
    {
      copy_term(t);
      return *this;
    }

    /// \brief Move assignment operator.
    /// \param t a term to be assigned.
    aterm& operator=(aterm &&t)
    {
      swap(t);
      return *this;
    }

    /// \brief Destructor.
    ~aterm ()
    {
      decrease_reference_count();
    }

    /// \brief Returns whether this term is a term_appl.
    /// \details This function has constant complexity.
    ///          It is defined as !type_is_int() && !type_is_list().
    /// \return True iff term is an term_appl.
    bool type_is_appl() const
    {
      return !type_is_int() && !type_is_list();
    }

    /// \brief Returns whether this term has the internal structure of an aterm_int.
    /// \details This function has constant complexity.
    /// \return True iff term is an term_int.
    bool type_is_int() const
    {
      return m_term->function()==detail::function_adm.AS_INT;
    }

    /// \brief Returns whether this term has the structure of an term_list
    /// \details This function has constant complexity.
    /// \return True iff term is an term_list.
    bool type_is_list() const
    {
      const function_symbol &f=m_term->function();
      return f==detail::function_adm.AS_LIST|| f==detail::function_adm.AS_EMPTY_LIST;
    }

    /// \brief Equality function on two aterms.
    /// \details Terms are stored in a maximally shared way. This
    ///         means that this equality operator can be calculated
    ///         in constant time.
    /// \param t A term to which the current term is compared.
    /// \return true iff t is equal to the current term.
    bool operator ==(const aterm &t) const
    {
      assert(m_term->reference_count()>0);
      assert(t.m_term->reference_count()>0);
      return m_term==t.m_term;
    }

    /// \brief Inequality operator on two aterms.
    /// \details See note at the == operator. This operator requires constant time.
    /// \param t A term to which the current term is compared.
    /// \return false iff t is equal to the current term.
    bool operator !=(const aterm &t) const
    {
      assert(m_term->reference_count()>0);
      assert(t.m_term->reference_count()>0);
      return m_term!=t.m_term;
    }

    /// \brief Comparison operator for two aterms.
    /// \details This operator requires constant time. It compares
    ///         the addresses where terms are stored. That means
    ///         that the outcome of this operator is only stable
    ///         as long as aterms are not garbage collected.
    /// \param t A term to which the current term is compared.
    /// \return True iff the current term is smaller than the argument.
    bool operator <(const aterm &t) const
    {
      assert(m_term->reference_count()>0);
      assert(t.m_term->reference_count()>0);
      return m_term<t.m_term;
    }

    /// \brief Comparison operator for two aterms.
    /// \details This operator requires constant time. See note at the operator <.
    /// \param t A term to which the current term is compared.
    /// \return True iff the current term is larger than the argument.
    bool operator >(const aterm &t) const
    {
      assert(m_term->reference_count()>0);
      assert(t.m_term->reference_count()>0);
      return m_term>t.m_term;
    }

    /// \brief Comparison operator for two aterms.
    /// \details This operator requires constant time. See note at the operator <.
    /// \param t A term to which the current term is compared.
    /// \return True iff the current term is smaller or equal than the argument.
    bool operator <=(const aterm &t) const
    {
      assert(m_term->reference_count()>0);
      assert(t.m_term->reference_count()>0);
      return m_term<=t.m_term;
    }

    /// \brief Comparison operator for two aterms.
    /// \details This operator requires constant time. See note at the operator <.
    /// \param t A term to which the current term is compared.
    /// \return True iff the current term is larger or equalthan the argument.
    bool operator >=(const aterm &t) const
    {
      assert(m_term->reference_count()>0);
      assert(t.m_term->reference_count()>0);
      return m_term>=t.m_term;
    }

    /// \brief Returns true if this term is not equal to the term assigned by
    ///        the default constructor of aterms, term_appl<T>'s and aterm_int.
    /// \details The default constructor of a term_list<T> is the empty list, on which
    ///          the operator defined yields true. This operation is more efficient
    ///          than comparing the current term with an aterm(), term_appl<T>() or an
    ///          aterm_int().
    /// \return A boolean indicating whether this term equals the default constructor.
    bool defined() const
    {
      assert(m_term->reference_count()>0);
      return this->function()!=detail::function_adm.AS_DEFAULT;
    }

    /// \brief Swaps this term with its argument.
    /// \details This operation is more efficient than exchanging terms by an assignment,
    ///          as swapping does not require to change the protection of terms.
    /// \param t The term with which this term is swapped.
    void swap(aterm &t)
    {
      assert(m_term->reference_count()>0);
      assert(t.m_term->reference_count()>0);
      using std::swap;
      swap(m_term,t.m_term);
    }
};

/// \brief A cheap cast from one aterm based type to another
/// \details When casting one aterm based type into another, generally
///          a new aterm is constructed, and the old one is destroyed.
///          This can cause undesired overhead, for instance due to
///          increasing and decreasing of reference counts. This
///          cast changes the type, without changing the aterm itself.
///          It can only be used if the input and output types inherit
///          from aterms, and contain no additional information than a
///          single aterm.
/// \param   t A term of a type inheriting from an aterm.
/// \return  A term of type ATERM_TYPE_OUT.
template <class ATERM_TYPE_OUT>
const ATERM_TYPE_OUT &aterm_cast(const aterm &t)
{
  static_assert(boost::is_base_of<aterm, ATERM_TYPE_OUT>::value,"Aterm_cast must be applied on a term derived from aterm");
  static_assert(sizeof(ATERM_TYPE_OUT)==sizeof(aterm),"Aterm_cast cannot be applied types derived from aterms where extra fields are added");
#ifndef NDEBUG
  // This assignment checks whether e has the right type;
  ATERM_TYPE_OUT t0=(ATERM_TYPE_OUT &)t;
  // assert(t0); // Prevent warning for not using t;
#endif
  return (ATERM_TYPE_OUT &)t;
}

/// \brief Send the term in textual form to the ostream.
std::ostream& operator<<(std::ostream& out, const aterm& t);

} // namespace atermpp


namespace std
{

/// \brief Swaps two aterms.
/// \details This operation is more efficient than exchanging terms by an assignment,
///          as swapping does not require to change the protection of terms.
///          In order to be used in the standard containers, the declaration must
///          be preceded by an empty template declaration. This swap function is
///          not used for classes that derive from the aterm class. A specific
///          swap function must be provided for derived classes.
/// \param t1 The first term
/// \param t2 The second term

template <>
inline void swap(atermpp::aterm &t1, atermpp::aterm &t2)
{
  t1.swap(t2);
}

/// \brief specialization of the standard std::hash function.
template<>
struct hash<atermpp::aterm>
{
  std::size_t operator()(const atermpp::aterm& t) const
  {
    return std::hash<atermpp::detail::_aterm*>()(const_cast<atermpp::detail::_aterm*>(atermpp::detail::address(t)));
  }
};

/// \brief specialization of the standard std::hash function.
template<>
struct hash<std::pair<atermpp::aterm,atermpp::aterm> >
{
  std::size_t operator()(const std::pair<atermpp::aterm, atermpp::aterm>& x) const
  {
    // The hashing function below is taken from boost (http://www.boost.org/doc/libs/1_35_0/doc/html/boost/hash_combine_id241013.html).
    size_t seed=std::hash<atermpp::aterm>()(x.first);
    return std::hash<atermpp::aterm>()(x.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
};

} // namespace std


#include "mcrl2/atermpp/detail/aterm_implementation.h"

#endif // MCRL2_ATERMPP_ATERM_H
