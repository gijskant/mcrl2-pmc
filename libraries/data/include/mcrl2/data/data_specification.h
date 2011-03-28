// Author(s): Jeroen Keiren, Jeroen van der Wulp, Jan Friso Groote
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/data_specification.h
/// \brief The class data_specification.

#ifndef MCRL2_DATA_DATA_SPECIFICATION_H
#define MCRL2_DATA_DATA_SPECIFICATION_H

#include <algorithm>
#include <functional>
#include <vector>

#include "boost/bind.hpp"
#include "boost/iterator/transform_iterator.hpp"
#include "boost/range/iterator_range.hpp"

#include "mcrl2/atermpp/aterm_appl.h"
#include "mcrl2/atermpp/map.h"
#include "mcrl2/atermpp/table.h"
#include "mcrl2/atermpp/set.h"

// utilities
#include "mcrl2/atermpp/container_utility.h"
#include "mcrl2/data/utility.h"
#include "mcrl2/data/translate_user_notation.h"

// data expressions
#include "mcrl2/data/data_expression.h"
#include "mcrl2/data/function_symbol.h"
#include "mcrl2/data/data_equation.h"
#include "mcrl2/data/assignment.h"
#include "mcrl2/data/where_clause.h"
#include "mcrl2/data/function_update.h"
#include "mcrl2/data/detail/normalize_sorts_fwd.h"

// sorts
#include "mcrl2/data/sort_expression.h"
#include "mcrl2/data/container_sort.h"
#include "mcrl2/data/structured_sort.h"
#include "mcrl2/data/alias.h"
#include "mcrl2/data/standard.h"

// standard data types
#include "mcrl2/data/bool.h"
#include "mcrl2/data/pos.h"
#include "mcrl2/data/nat.h"
#include "mcrl2/data/int.h"
#include "mcrl2/data/real.h"
#include "mcrl2/data/list.h"
#include "mcrl2/data/set.h"
#include "mcrl2/data/bag.h"

namespace mcrl2
{

namespace data
{

// prototype
class data_specification;

/// \cond INTERNAL_DOCS
namespace detail
{
  data_equation translate_user_notation_data_equation(const data_equation& x);
} // namespace detail

/* sort_expression normalize_sorts(const sort_expression& x,
                                const data_specification& data_spec);

class data_specification; */

// prototype, find.h included at the end of this file to prevent circular dependencies.
template < typename Container >
std::set<sort_expression> find_sort_expressions(Container const& container);

/// \cond INTERNAL_DOCS
namespace detail
{
template < typename Container, typename Sequence >
void insert(Container& container, Sequence sequence)
{
  container.insert(sequence.begin(), sequence.end());
}

atermpp::aterm_appl data_specification_to_aterm_data_spec(const data_specification&);
}
/// \endcond

/// \brief data specification.

class data_specification
{
  protected:

    /// \brief map from sort expression to constructors
    typedef atermpp::multimap< sort_expression, data::function_symbol > sort_to_symbol_map;

    /// \brief map from basic_sort (names) to sort expression
    typedef atermpp::multimap< basic_sort, sort_expression >           ltr_aliases_map;

  private:

    /// \cond INTERNAL_DOCS
    /// \brief projects a pair of sort and a constructor to the latter
    struct symbol_projection : public std::unary_function< sort_to_symbol_map::value_type const, data::function_symbol >
    {
      /// \brief Application to constant pair
      data::function_symbol const& operator()(sort_to_symbol_map::value_type const& v) const
      {
        return v.second;
      }
    };

    struct convert_to_alias : public std::unary_function< ltr_aliases_map::value_type const, alias >
    {
      /// \brief Application to constant pair
      alias operator()(ltr_aliases_map::value_type const& e) const
      {
        return alias(e.first,e.second);
      }
    };

    /// \endcond

  public:

    /// \brief iterator range over list of sort expressions
    typedef boost::iterator_range< atermpp::set< sort_expression >::iterator >                    sorts_range;
    /// \brief iterator range over constant list of sort expressions
    typedef boost::iterator_range< atermpp::set< sort_expression >::const_iterator >              sorts_const_range;

    /// \brief iterator range over list of sort expressions
    typedef boost::iterator_range< atermpp::set< variable >::iterator >                           variable_range;
    /// \brief iterator range over constant list of sort expressions
    typedef boost::iterator_range< atermpp::set< variable >::const_iterator >                     variable_const_range;

    /// \brief iterator over aliases (objects of type function_symbol)
    typedef boost::transform_iterator< convert_to_alias, ltr_aliases_map::iterator >          aliases_iterator;
    /// \brief const iterator over aliases (objects of type function_symbol)
    typedef boost::transform_iterator< convert_to_alias, ltr_aliases_map::const_iterator >    aliases_const_iterator;
    /// \brief iterator range over list of aliases
    typedef boost::iterator_range< aliases_iterator >                                             aliases_range;
    /// \brief iterator range over constant list of aliases
    typedef boost::iterator_range< aliases_const_iterator >                                       aliases_const_range;

    /// \brief iterator over constructors (objects of type function_symbol)
    typedef boost::transform_iterator< symbol_projection, sort_to_symbol_map::iterator >          constructors_iterator;
    /// \brief const iterator over constructors (objects of type function_symbol)
    typedef boost::transform_iterator< symbol_projection, sort_to_symbol_map::const_iterator >    constructors_const_iterator;
    /// \brief iterator range over constructors (objects of type function_symbol)
    typedef boost::iterator_range< constructors_iterator >                                        constructors_range;
    /// \brief const iterator range over constructors (objects of type function_symbol)
    typedef boost::iterator_range< constructors_const_iterator >                                  constructors_const_range;

    /// \brief iterator over mappings (objects of type function_symbol)
    typedef boost::transform_iterator< symbol_projection, sort_to_symbol_map::iterator >          mappings_iterator;
    /// \brief const iterator over mappings (objects of type function_symbol)
    typedef boost::transform_iterator< symbol_projection, sort_to_symbol_map::const_iterator >    mappings_const_iterator;
    /// \brief iterator range over mappings (objects of type function_symbol)
    typedef boost::iterator_range< mappings_iterator >                                            mappings_range;
    /// \brief const iterator range over mappings (objects of type function_symbol)
    typedef boost::iterator_range< mappings_const_iterator >                                      mappings_const_range;

    /// \brief iterator range over list of data equations (objects of type data_equation)
    typedef boost::iterator_range< atermpp::set< data_equation >::iterator >                      equations_range;
    /// \brief iterator range over constant list of data equations (objects of type data_equation)
    typedef boost::iterator_range< atermpp::set< data_equation >::const_iterator >                equations_const_range;

  private:

    friend atermpp::aterm_appl detail::data_specification_to_aterm_data_spec(const data_specification&);

    ///\brief Builds a specification from aterm
    void build_from_aterm(const atermpp::aterm_appl& t);

    // The function below recalculates m_normalised_aliases, such that
    // it forms a confluent terminating rewriting system using which
    // sorts can be normalised.
    void reconstruct_m_normalised_aliases() const;

    // The function below checks whether there is an alias loop, e.g. aliases
    // of the form A=B; B=A; or more complex A=B->C; B=Set(D); D=List(A); Loops
    // through structured sorts are allowed. If a loop is detected, an exception
    // is thrown.
    void check_for_alias_loop(
      const sort_expression s,
      std::set < sort_expression > sorts_already_seen,
      const bool toplevel=true) const;

  protected:

    /// \brief The variable data_specification_is_type_checked indicates
    /// whether a typechecked data specification is used to construct the
    /// data specification, or not. If not, the data specification is stored
    /// in non_type_checked_data_spec. It is only returned using the function
    /// data_specification_to_aterm_data_spec. Other functions to access the
    /// data structures cannot be used. If the specification is type
    /// checked, non_type_checked_data_spec is not used, but the specification
    /// is put into all the other data structures. The function declare_data_specification_to_be_type_checked
    /// sets the boolean data_specification_is_type_checked and takes care that all
    /// data types are properly constructed.

    bool m_data_specification_is_type_checked;
    atermpp::aterm_appl m_non_typed_checked_data_spec;

    /// \brief This boolean indicates whether the variables
    /// m_normalised_constructors, m_mappings, m_equations, m_normalised_sorts,
    /// m_normalised_aliases.
    mutable bool m_normalised_data_is_up_to_date;

    /// \brief The basic sorts and structured sorts in the specification.
    atermpp::set< sort_expression >     m_sorts;

    /// \brief The sorts that occur in the context of this data specification.
    /// The normalised sorts, constructors, mappings and equations are complete
    /// with respect to these sorts.
    mutable atermpp::set< sort_expression >     m_sorts_in_context;

    /// \brief The basic sorts and structured sorts in the specification.
    ltr_aliases_map                     m_aliases;

    /// \brief A mapping of sort expressions to the constructors corresponding to that sort.
    sort_to_symbol_map                  m_constructors;

    /// \brief The mappings of the specification.
    sort_to_symbol_map                  m_mappings;

    /// \brief The equations of the specification.
    atermpp::set< data_equation >       m_equations;

    /// \brief Set containing all the sorts, including the system defined ones.
    mutable atermpp::set< sort_expression >         m_normalised_sorts;

    /// \brief Set containing all constructors, including the system defined ones.
    /// The types in these constructors are normalised.
    mutable sort_to_symbol_map         m_normalised_constructors;

    /// \brief Set containing system defined all mappings, including the system defined ones.
    /// The types in these mappings are normalised.
    mutable sort_to_symbol_map         m_normalised_mappings;
    //
    /// \brief Table containing all equations, including the system defined ones.
    ///        The sorts in these equations are normalised.
    mutable atermpp::set< data_equation >           m_normalised_equations;

    /// \brief Table containing how sorts should be mapped to normalised sorts.
    // sort_normaliser               m_sort_normaliser;
    mutable atermpp::map< sort_expression, sort_expression > m_normalised_aliases;

    void data_is_not_necessarily_normalised_anymore() const
    {
      m_normalised_data_is_up_to_date=false;
    }

  protected:

    /// \brief Adds a sort to this specification, and marks it as system
    ///        defined
    ///
    /// \param[in] s A sort expression.
    /// \pre s does not yet occur in this specification.
    /// \post is_system_defined(s) = true
    /// \note this operation does not invalidate iterators of sorts_const_range
    void add_system_defined_sort(const sort_expression& s) const
    {
      sort_expression normalised(normalize_sorts(s,*this));
      if (!is_function_sort(normalised))
      {
        m_normalised_sorts.insert(normalised);
      }
    }

    /// \brief Adds a constructor to this specification, and marks it as
    ///        system defined.
    ///
    /// \param[in] f A function symbol.
    /// \pre f does not yet occur in this specification.
    /// \post is_system_defined(f) == true
    /// \note this operation does not invalidate iterators of constructors_const_range
    inline
    void add_system_defined_constructor(const function_symbol& f) const
    {
      add_function(m_normalised_constructors,normalize_sorts(f,*this));
    }

    /// \brief Adds a mapping to this specification, and marks it as system
    ///        defined.
    ///
    /// \param[in] f A function symbol.
    /// \pre f does not yet occur in this specification.
    /// \post is_system_defined(f) == true
    /// \note this operation does not invalidate iterators of mappings_const_range
    void add_system_defined_mapping(const function_symbol& f) const
    {
      add_function(m_normalised_mappings,normalize_sorts(f,*this));
    }

    /// \brief Adds an equation to this specification, and marks it as system
    ///        defined.
    ///
    /// \param[in] e An equation.
    /// \pre e does not yet occur in this specification.
    /// \post is_system_defined(f) == true
    /// \note this operation does not invalidate iterators of equations_const_range
    void add_system_defined_equation(const data_equation& e) const
    {
      m_normalised_equations.insert(normalize_sorts(e,*this));
    }

    /// \brief Adds constructors, mappings and equations for a structured sort
    ///        to this specification, and marks them as system defined.
    ///
    /// \param[in] sort A sort expression that is representing the structured sort.
    void insert_mappings_constructors_for_structured_sort(const structured_sort& sort) const
    {
      add_system_defined_sort(normalize_sorts(sort,*this));

      structured_sort s_sort(sort);
      function_symbol_vector f(s_sort.constructor_functions(sort));
      std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
      f = s_sort.projection_functions(sort);
      std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
      f = s_sort.recogniser_functions(sort);
      std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));

      data_equation_vector e(s_sort.constructor_equations(sort));
      std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));
      e = s_sort.projection_equations(sort);
      std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));
      e = s_sort.recogniser_equations(sort);
      std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));
    }

    void remove_function(sort_to_symbol_map& container, const function_symbol& f)
    {
      boost::iterator_range< sort_to_symbol_map::iterator > r(container.equal_range(f.sort().target_sort()));

      sort_to_symbol_map::iterator i =
        std::find(r.begin(), r.end(), sort_to_symbol_map::value_type(f.sort().target_sort(), f));

      if (i != container.end())
      {
        container.erase(i);
      }
    }

    void add_function(sort_to_symbol_map& container, const function_symbol& f) const
    {
      sort_expression index_sort(f.sort().target_sort());
      constructors_const_range relevant_range(container.equal_range(index_sort));
      if (std::find(relevant_range.begin(), relevant_range.end(), f) == relevant_range.end())
      {
        container.insert(std::make_pair(index_sort, f));
      }
    }

    void add_function(sort_to_symbol_map& container, const function_symbol& f)
    {
      sort_expression index_sort(f.sort().target_sort());
      constructors_const_range relevant_range(container.equal_range(index_sort));
      if (std::find(relevant_range.begin(), relevant_range.end(), f) == relevant_range.end())
      {
        container.insert(std::make_pair(index_sort, f));
      }
    }

    void add_standard_mappings_and_equations(sort_expression const& sort) const
    {
      function_symbol_vector symbols(standard_generate_functions_code(normalize_sorts(sort,*this)));

      for (function_symbol_vector::const_iterator i = symbols.begin(); i != symbols.end(); ++i)
      {
        add_function(m_normalised_mappings,*i);
      }

      data_equation_vector equations(standard_generate_equations_code(sort));

      for (data_equation_vector::const_iterator i = equations.begin(); i != equations.end(); ++i)
      {
        add_system_defined_equation(*i);
      }
    }

  public:

    ///\brief Default constructor. Generate a data specification that contains
    ///       only booleans.
    data_specification()
      : m_data_specification_is_type_checked(true),
        m_normalised_data_is_up_to_date(false)
    {}

    ///\brief Constructor from an aterm.
    /// \param[in] t a term adhering to the internal format.
    data_specification(const atermpp::aterm_appl& t)
      :
      m_data_specification_is_type_checked(false),
      m_normalised_data_is_up_to_date(false)
    {
      m_non_typed_checked_data_spec=t;
    }

    ///\brief Constructor
    template < typename SortsRange, typename AliasesRange, typename ConstructorsRange,
             typename MappingsRange, typename EquationsRange >
    data_specification(const SortsRange& sorts,
                       const AliasesRange& aliases,
                       const ConstructorsRange& constructors,
                       const MappingsRange& mappings,
                       const EquationsRange& equations)
      : m_data_specification_is_type_checked(true),
        m_normalised_data_is_up_to_date(false)
    {
      std::for_each(sorts.begin(), sorts.end(),
                    boost::bind(&data_specification::add_sort, this, _1));
      std::for_each(constructors.begin(), constructors.end(),
                    boost::bind(&data_specification::add_constructor, this, _1));
      std::for_each(mappings.begin(), mappings.end(),
                    boost::bind(&data_specification::add_mapping, this, _1));
      std::for_each(equations.begin(), equations.end(),
                    boost::bind(&data_specification::add_equation, this, _1));
    }

    /// \brief Indicates that the data specification is type checked.
    /// \details This builds up internal data structures and allows
    ///  access to the data specification using all the utility functions.
    void declare_data_specification_to_be_type_checked()
    {
      assert(!m_data_specification_is_type_checked); //A data specification can only be declared
      //type checked once.
      m_data_specification_is_type_checked=true;
      build_from_aterm(m_non_typed_checked_data_spec);
      m_non_typed_checked_data_spec=atermpp::aterm_appl();
    }


    /// \brief Gets all sort declarations including those that are system defined.
    ///
    /// \details The time complexity of this operation is constant, except when
    ///      the data specification has been changed, in which case it can be that
    ///      it must be normalised again. This operation is linear in the size of
    ///      the specification.
    /// \return The sort declarations of this specification.
    inline
    sorts_const_range sorts() const
    {
      assert(m_data_specification_is_type_checked);
      normalise_specification_if_required();
      return sorts_const_range(m_normalised_sorts);
    }

    /// \brief Gets all sorts defined by a user (excluding the system defined sorts).
    ///
    /// \details The time complexity of this operation is constant.
    /// \return The user defined sort declaration.
    inline
    sorts_const_range user_defined_sorts() const
    {
      assert(m_data_specification_is_type_checked);
      return sorts_const_range(m_sorts);
    }

    /// \brief Gets all constructors including those that are system defined.
    /// \details The time complexity is the same as for sorts().
    /// \return All constructors in this specification, including those for
    /// structured sorts.
    inline
    constructors_const_range constructors() const
    {
      assert(m_data_specification_is_type_checked);
      normalise_specification_if_required();
      return constructors_const_range(m_normalised_constructors);
    }

    /// \brief Gets the constructors defined by the user, excluding those that
    /// are system defined.
    /// \details The time complexity for this operation is constant.
    inline
    constructors_const_range user_defined_constructors() const
    {
      assert(m_data_specification_is_type_checked);
      return constructors_const_range(m_constructors);
    }

    /// \brief Gets all constructors of a sort including those that are system defined.
    ///
    /// \details The time complexity is the same as for sorts().
    /// \param[in] s A sort expression.
    /// \return The constructors for sort s in this specification.
    inline
    constructors_const_range constructors(const sort_expression& s) const
    {
      assert(m_data_specification_is_type_checked);
      normalise_specification_if_required();
      return constructors_const_range(m_normalised_constructors.equal_range(normalize_sorts(s,*this)));
    }

    /// \brief Gets all mappings in this specification including those that are system defined.
    ///
    /// \brief The time complexity is the same as for sorts().
    /// \return All mappings in this specification, including recognisers and
    /// projection functions from structured sorts.
    inline
    mappings_const_range mappings() const
    {
      assert(m_data_specification_is_type_checked);
      normalise_specification_if_required();
      return mappings_const_range(m_normalised_mappings);
    }

    /// \brief Gets all user defined mappings in this specification.
    ///
    /// \brief The time complexity is constant.
    /// \return All mappings in this specification, including recognisers and
    /// projection functions from structured sorts.
    inline
    mappings_const_range user_defined_mappings() const
    {
      assert(m_data_specification_is_type_checked);
      return mappings_const_range(m_mappings);
    }

    /// \brief Gets all mappings of a sort including those that are system defined
    ///
    /// \param[in] s A sort expression.
    /// \return All mappings in this specification, for which s occurs as a
    /// right-hand side of the mapping's sort.
    inline
    mappings_const_range mappings(const sort_expression& s) const
    {
      assert(m_data_specification_is_type_checked);
      normalise_specification_if_required();
      return mappings_const_range(m_normalised_mappings.equal_range(normalize_sorts(s,*this)));
    }

    /// \brief Gets all equations in this specification including those that are system defined
    ///
    /// \details The time complexity of this operation is the same as that for sort().
    /// \return All equations in this specification, including those for
    ///  structured sorts.
    inline
    equations_const_range equations() const
    {
      assert(m_data_specification_is_type_checked);
      normalise_specification_if_required();
      return equations_const_range(m_normalised_equations);
    }

    /// \brief Gets all user defined equations.
    ///
    /// \details The time complexity of this operation is constant.
    /// \return All equations in this specification, including those for
    ///  structured sorts.
    inline
    equations_const_range user_defined_equations() const
    {
      assert(m_data_specification_is_type_checked);
      return equations_const_range(m_equations);
    }

    /// \brief Gets a normalisation mapping that maps each sort to its unique normalised sort
    /// \details When in a specification sort aliases are used, like sort A=B or
    ///    sort Tree=struct leaf | node(Tree,Tree) then there are different representations
    ///    for each sort. The normalisation mapping maps each sort to a unique representant.
    ///    Moreover, it is this unique sort that it provides in internal mappings.
    const atermpp::map< sort_expression, sort_expression > &sort_alias_map() const
    {
      assert(m_data_specification_is_type_checked);
      normalise_specification_if_required();
      return m_normalised_aliases;
    }

    /// \brief Gets the user defined aliases.
    /// \details The time complexity is constant.
    inline
    ltr_aliases_map user_defined_aliases() const
    {
      assert(m_data_specification_is_type_checked);
      return m_aliases;
    }

    /// \brief Return the user defined context sorts of the current specification.
    /// \details Time complexity is constant.
    sorts_const_range context_sorts() const
    {
      return m_sorts_in_context;
    }

    /// \brief Adds a sort to this specification
    ///
    /// \param[in] s A sort expression.
    /// \note this operation does not invalidate iterators of sorts_const_range
    void add_sort(const sort_expression& s)
    {
      assert(m_data_specification_is_type_checked);
      if (m_sorts.insert(s).second)
      {
        data_is_not_necessarily_normalised_anymore();
      }
    }

    /// \brief Adds an alias (new name for a sort) to this specification
    ///
    /// \param[in] a an alias
    /// \pre !search_sort(s.name()) || is_alias(s.name()) || constructors(s.name()).empty()
    /// \note this operation does not invalidate iterators of aliases_const_range
    /// \post is_alias(s.name()) && normalize_sorts(s.name(),*this) = normalize_sorts(s.reference(),*this)
    void add_alias(alias const& a)
    {
      assert(m_data_specification_is_type_checked);
      m_aliases.insert(std::pair<basic_sort, sort_expression>(a.name(),a.reference()));
      data_is_not_necessarily_normalised_anymore();
    }

    /// \brief Adds a constructor to this specification
    ///
    /// \param[in] f A function symbol.
    /// \pre a mapping f does not yet occur in this specification.
    /// \note this operation does not invalidate iterators of constructors_const_range
    void add_constructor(const function_symbol& f)
    {
      assert(m_data_specification_is_type_checked);
      add_function(m_constructors, f);
      data_is_not_necessarily_normalised_anymore();
    }

    /// \brief Adds a mapping to this specification
    ///
    /// \param[in] f A function symbol.
    /// \pre a constructor f does not yet occur in this specification.
    /// \note this operation does not invalidate iterators of mappings_const_range
    void add_mapping(const function_symbol& f)
    {
      assert(m_data_specification_is_type_checked);
      add_function(m_mappings, f);
      data_is_not_necessarily_normalised_anymore();
    }

    /// \brief Adds an equation to this specification
    ///
    /// \param[in] e An equation.
    /// \pre e does not yet occur in this specification.
    /// \note this operation does not invalidate iterators of equations_const_range
    void add_equation(const data_equation& e)
    {
      assert(m_data_specification_is_type_checked);
      m_equations.insert(detail::translate_user_notation_data_equation(e));
      data_is_not_necessarily_normalised_anymore();
    }

    ///\brief Adds the sort s to the context sorts
    /// \param[in] s a sort expression. It is
    /// added to m_sorts_in_context. For this sort standard functions are generated
    /// automatically (if, <,<=,==,!=,>=,>) and if the sort is a standard sort,
    /// the necessary constructors, mappings and equations are added to the data type.
    void add_context_sort(const sort_expression& s) const
    {
      if (m_sorts_in_context.insert(s).second)
      {
        if (is_container_sort(s))
        {
          add_context_sort(container_sort(s).element_sort());
        }
        data_is_not_necessarily_normalised_anymore();
      }
    }

    ///\brief Adds the sorts in c to the context sorts
    /// \param[in] c a container of sort expressions. These are
    /// added to m_sorts_in_context. For these sorts standard functions are generated
    /// automatically (if, <,<=,==,!=,>=,>) and if the sorts are standard sorts,
    /// the necessary constructors, mappings and equations are added to the data type.
    template <typename Container>
    void add_context_sorts(const Container& c, typename atermpp::detail::enable_if_container<Container>::type* = 0) const
    {
      std::for_each(c.begin(), c.end(),
                    boost::bind(&data_specification::add_context_sort, this, _1));
    }

  private:

    ///\brief Normalises the sorts in the data specification
    ///\details See \ref normalise_sorts on arbitrary objects for a more detailed description.
    /// All sorts in the constructors, mappings and equations are normalised.
    void normalise_sorts() const
    {
      // Normalise the sorts of the constructors.
      assert(m_data_specification_is_type_checked);
      m_normalised_sorts.clear();
      m_normalised_constructors.clear();
      m_normalised_mappings.clear();
      m_normalised_equations.clear();
      std::set < sort_expression > sorts_already_added_to_m_normalised_sorts;
      reconstruct_m_normalised_aliases();
      for (atermpp::set< sort_expression >::const_iterator i=m_sorts.begin();
           i!=m_sorts.end(); ++i)
      {
        add_system_defined_sort(*i);
        import_system_defined_sort(*i,sorts_already_added_to_m_normalised_sorts);
      }

      for (atermpp::set< sort_expression >::const_iterator i=m_sorts_in_context.begin();
           i!=m_sorts_in_context.end(); ++i)
      {
        import_system_defined_sort(*i,sorts_already_added_to_m_normalised_sorts);
      }

      std::set< sort_expression > dependent_sorts;
      dependent_sorts.insert(sort_bool::bool_());

      // constructors
      detail::insert(dependent_sorts, make_sort_range(constructors_const_range(m_constructors)));

      // mappings
      detail::insert(dependent_sorts, make_sort_range(mappings_const_range(m_mappings)));

      // equations
      for (atermpp::set< data_equation >::const_iterator r(m_equations.begin()); r != m_equations.end(); ++r)
      {
        // make function sort in case of constants to add the corresponding sort as needed
        detail::insert(dependent_sorts, find_sort_expressions(*r));
      }

      // aliases, with both left and right hand sides.
      for (ltr_aliases_map::const_iterator i=m_aliases.begin();
           i!=m_aliases.end(); ++i)
      {
        dependent_sorts.insert(i->first);
        detail::insert(dependent_sorts,find_sort_expressions(i->second));
      }

      for (atermpp::set< sort_expression >::const_iterator i=dependent_sorts.begin();
           i!=dependent_sorts.end(); ++i)
      {
        add_system_defined_sort(*i);
        import_system_defined_sort(*i,sorts_already_added_to_m_normalised_sorts);
      }


      for (ltr_aliases_map ::const_iterator i=m_aliases.begin();
           i!=m_aliases.end(); ++i)
      {
        add_system_defined_sort(i->first);
        add_system_defined_sort(i->second);
        import_system_defined_sort(i->first,sorts_already_added_to_m_normalised_sorts);
        import_system_defined_sort(i->second,sorts_already_added_to_m_normalised_sorts);
      }

      // sort_to_symbol_map new_constructors;
      for (sort_to_symbol_map::const_iterator i=m_constructors.begin();
           i!=m_constructors.end(); ++i)
      {
        const sort_expression normalised_sort=normalize_sorts(i->first,*this);
        const function_symbol normalised_constructor=normalize_sorts(i->second,*this);

        mappings_const_range range(m_normalised_constructors.equal_range(normalised_constructor.sort().target_sort()));
        if (std::find(range.begin(), range.end(), normalised_constructor) == range.end())
        {
          m_normalised_constructors.insert(std::pair<sort_expression, function_symbol>
                                           (normalised_sort,normalised_constructor));
        }
        add_system_defined_sort(normalised_sort);
      }

      // Normalise the sorts of the mappings.
      for (sort_to_symbol_map::const_iterator i=m_mappings.begin();
           i!=m_mappings.end(); ++i)
      {
        const sort_expression normalised_sort=normalize_sorts(i->first,*this);
        const function_symbol normalised_mapping=normalize_sorts(i->second,*this);

        mappings_const_range range(m_normalised_mappings.equal_range(normalised_mapping.sort().target_sort()));
        if (std::find(range.begin(), range.end(), normalised_mapping) == range.end())
        {
          m_normalised_mappings.insert((std::pair<sort_expression, function_symbol>
                                        (normalised_sort,normalised_mapping)));
        }
        add_system_defined_sort(normalised_sort);
      }

      // Normalise the sorts of the expressions and variables in equations.
      for (atermpp::set< data_equation >::const_iterator i=m_equations.begin();
           i!=m_equations.end(); ++i)
      {
        add_system_defined_equation(*i);
      }
    }

    /// \brief
    /// \details
    void normalise_specification_if_required() const
    {
      if (!m_normalised_data_is_up_to_date)
      {
        m_normalised_data_is_up_to_date=true;
        normalise_sorts();
      }
    }

    ///\brief Adds the system defined sorts in a sequence.
    ///       The second argument is used to check which sorts are added, to prevent
    ///       useless repetitions of additions of sorts.
    /// The function normalise_sorts imports for the given sort_expression sort all sorts, constructors,
    /// mappings and equations that belong to this sort to the `normalised' sets in this
    /// data type. E.g. for the sort Nat of natural numbers, it is required that Pos
    /// (positive numbers) are defined.
    void import_system_defined_sort(
      sort_expression const& sort,
      std::set <sort_expression> &sorts_already_added_to_m_normalised_sorts) const
    {
      assert(m_data_specification_is_type_checked);
      // First check whether sort has already been added. If yes, we can skip this step.
      if (sorts_already_added_to_m_normalised_sorts.count(sort)>0)
      {
        return;
      }
      sorts_already_added_to_m_normalised_sorts.insert(sort);

      // add sorts, constructors, mappings and equations
      if (sort == sort_bool::bool_())
      {
        // Add bool to the specification
        add_system_defined_sort(sort_bool::bool_());

        function_symbol_vector f(sort_bool::bool_generate_constructors_code());
        std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
        f = sort_bool::bool_generate_functions_code();
        std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
        data_equation_vector e(sort_bool::bool_generate_equations_code());
        std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));
      }
      else if (sort == sort_real::real_())
      {
        // Add Real to the specification
        add_system_defined_sort(sort_real::real_());

        function_symbol_vector f(sort_real::real_generate_constructors_code());
        std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
        f = sort_real::real_generate_functions_code();
        std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
        data_equation_vector e(sort_real::real_generate_equations_code());
        std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));

        import_system_defined_sort(sort_int::int_(),sorts_already_added_to_m_normalised_sorts);
        // A full definition of Int is required
        // as the rewrite rules of Real rely on it.
      }
      else if (sort == sort_int::int_())
      {
        // Add Int to the specification
        add_system_defined_sort(sort_int::int_());

        function_symbol_vector f(sort_int::int_generate_constructors_code());
        std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
        f = sort_int::int_generate_functions_code();
        std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
        data_equation_vector e(sort_int::int_generate_equations_code());
        std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));

        import_system_defined_sort(sort_nat::nat(),sorts_already_added_to_m_normalised_sorts);
        // See above, Int requires Nat.
      }
      else if (sort == sort_nat::nat())
      {
        // Add Nat to the specification
        add_system_defined_sort(sort_nat::natpair());
        add_system_defined_sort(sort_nat::nat());

        function_symbol_vector f(sort_nat::nat_generate_constructors_code());
        std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
        f = sort_nat::nat_generate_functions_code();
        std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
        data_equation_vector e(sort_nat::nat_generate_equations_code());
        std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));

        import_system_defined_sort(sort_pos::pos(),sorts_already_added_to_m_normalised_sorts);  // See above, Nat requires Pos.
      }
      else if (sort == sort_pos::pos())
      {
        // Add Pos to the specification
        add_system_defined_sort(sort_pos::pos());

        function_symbol_vector f(sort_pos::pos_generate_constructors_code());
        std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
        f = sort_pos::pos_generate_functions_code();
        std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
        data_equation_vector e(sort_pos::pos_generate_equations_code());
        std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));
      }
      else if (is_function_sort(sort))
      {
        const sort_expression t=function_sort(sort).codomain();
        import_system_defined_sort(t,sorts_already_added_to_m_normalised_sorts);
        const sort_expression_list& l=function_sort(sort).domain();
        for (sort_expression_list::const_iterator i=l.begin(); i!=l.end(); ++i)
        {
          import_system_defined_sort(*i,sorts_already_added_to_m_normalised_sorts);
        }
        if (l.size()==1)
        {
          data_equation_vector e(function_update_generate_equations_code(l.front(),t));
          std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));
        }
      }
      else if (is_container_sort(sort))
      {
        sort_expression element_sort(container_sort(sort).element_sort());
        // Import the element sort (which may be a complex sort also).
        import_system_defined_sort(element_sort,sorts_already_added_to_m_normalised_sorts);
        if (sort_list::is_list(sort))
        {
          import_system_defined_sort(sort_nat::nat(),sorts_already_added_to_m_normalised_sorts); // Required for lists.

          // Add a list to the specification.
          add_system_defined_sort(sort);

          function_symbol_vector f(sort_list::list_generate_constructors_code(element_sort));
          std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
          f = sort_list::list_generate_functions_code(element_sort);
          std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
          data_equation_vector e(sort_list::list_generate_equations_code(element_sort));
          std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));
        }
        else if (sort_set::is_set(sort)||sort_fset::is_fset(sort))
        {
          // Add the function sort element_sort->Bool to the specification
          // const sort_expression_list l(element_sort);
          import_system_defined_sort(function_sort(push_front(sort_expression_list(),element_sort),sort_bool::bool_()),
                                     sorts_already_added_to_m_normalised_sorts);

          // Add a set to the specification.
          add_system_defined_sort(sort_set::set_(element_sort));
          function_symbol_vector f(sort_set::set_generate_constructors_code(element_sort));
          std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
          f = sort_set::set_generate_functions_code(element_sort);
          std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
          data_equation_vector e(sort_set::set_generate_equations_code(element_sort));
          std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));

          // Add also the finite set specification.
          add_system_defined_sort(sort_fset::fset(element_sort));
          f = sort_fset::fset_generate_constructors_code(element_sort);
          std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
          f = sort_fset::fset_generate_functions_code(element_sort);
          std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
          e = sort_fset::fset_generate_equations_code(element_sort);
          std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));
        }
        else if (sort_bag::is_bag(sort)||sort_fbag::is_fbag(sort))
        {
          // Add the sorts Nat and set_(element_sort) to the specification.
          import_system_defined_sort(sort_nat::nat(),sorts_already_added_to_m_normalised_sorts); // Required for bags.
          import_system_defined_sort(sort_set::set_(element_sort),sorts_already_added_to_m_normalised_sorts);

          // Add the function sort element_sort->Nat to the specification
          import_system_defined_sort(function_sort(push_front(sort_expression_list(),element_sort),sort_nat::nat()),
                                     sorts_already_added_to_m_normalised_sorts);

          // Add a bag to the specification.
          add_system_defined_sort(sort_bag::bag(element_sort));
          function_symbol_vector f(sort_bag::bag_generate_constructors_code(element_sort));
          std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
          f = sort_bag::bag_generate_functions_code(element_sort);
          std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
          data_equation_vector e(sort_bag::bag_generate_equations_code(element_sort));
          std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));

          // Also add a finite bag to the specification
          add_system_defined_sort(sort_fbag::fbag(element_sort));
          f = sort_fbag::fbag_generate_constructors_code(element_sort);
          std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_constructor, this, _1));
          f = sort_fbag::fbag_generate_functions_code(element_sort);
          std::for_each(f.begin(), f.end(), boost::bind(&data_specification::add_system_defined_mapping, this, _1));
          e = sort_fbag::fbag_generate_equations_code(element_sort);
          std::for_each(e.begin(), e.end(), boost::bind(&data_specification::add_system_defined_equation, this, _1));
        }
      }
      else if (is_structured_sort(sort))
      {
        insert_mappings_constructors_for_structured_sort(sort);
      }
      const sort_expression normalised_sort=normalize_sorts(sort,*this);
      add_standard_mappings_and_equations(normalised_sort);
    }

  public:

    /* /// \brief returns a sort expression that is represented by an alias.
    /// \details For a sort expression e, it is figured out which sort
    /// it represents. Concretely: if e is no alias, e is returned. If
    /// e=f is an alias and f is a basic sort, then the sort expression represented by e is
    /// returned. If e=C where C is a container sort, function sort or structured sort
    /// C is returned. The aliases in C are not unfolded.

    sort_expression unalias(const basic_sort &e) const
    {
      assert(m_data_specification_is_type_checked);
      const ltr_aliases_map::const_iterator i=m_aliases.find(e);
      if (i==m_aliases.end())  // Not found
      {
        return e;
      }
      if (is_basic_sort(i->second))
      { return unalias(i->second);
      }
      // *i is a more complex type;
      return i->second;
    } */

    /// \brief Removes sort from specification.
    /// Note that this also removes aliases for the sort but does not remove
    /// constructors, mappings and equations.
    /// \param[in] s A sort expression.
    /// \post s does not occur in this specification.
    /// \note this operation does not invalidate iterators of sorts_const_range,
    /// only if they point to the element that is removed
    void remove_sort(const sort_expression& s)
    {
      assert(m_data_specification_is_type_checked);
      m_sorts.erase(s);
      m_normalised_sorts.erase(normalize_sorts(s,*this));
    }

    /// \brief Removes alias from specification.
    /// \post !search_sort(a.name()) && !is_alias(a.name())
    void remove_alias(alias const& a)
    {
      assert(m_data_specification_is_type_checked);
      m_sorts.erase(a.name());
      m_aliases.erase(a.name());
      data_is_not_necessarily_normalised_anymore();
    }

    /// \brief Removes constructor from specification.
    ///
    /// Note that this does not remove equations containing the constructor.
    /// \param[in] f A constructor.
    /// \pre f occurs in the specification as constructor.
    /// \post f does not occur as constructor.
    /// \note this operation does not invalidate iterators of constructors_const_range,
    /// only if they point to the element that is removed
    void remove_constructor(const function_symbol& f)
    {
      assert(m_data_specification_is_type_checked);
      remove_function(m_normalised_constructors,normalize_sorts(f,*this));
      remove_function(m_constructors,f);
    }

    /// \brief Removes mapping from specification.
    ///
    /// Note that this does not remove equations in which the mapping occurs.
    /// \param[in] f A function.
    /// \post f does not occur as constructor.
    /// \note this operation does not invalidate iterators of mappings_const_range,
    /// only if they point to the element that is removed
    void remove_mapping(const function_symbol& f)
    {
      assert(m_data_specification_is_type_checked);
      remove_function(m_mappings,f);
      remove_function(m_normalised_mappings,normalize_sorts(f,*this));
    }

    /// \brief Removes equation from specification.
    ///
    /// \param[in] e An equation.
    /// \post e is removed from this specification.
    /// \note this operation does not invalidate iterators of equations_const_range,
    /// only if they point to the element that is removed
    void remove_equation(const data_equation& e)
    {
      assert(m_data_specification_is_type_checked);
      const data_equation e1=data::detail::translate_user_notation_data_equation(e);
      m_equations.erase(e1);
      m_normalised_equations.erase(normalize_sorts(e1,*this));
    }

    /// \brief Checks whether two sort expressions represent the same sort
    ///
    /// \param[in] s1 A sort expression
    /// \param[in] s2 A sort expression
    bool equal_sorts(sort_expression const& s1, sort_expression const& s2) const
    {
      assert(m_data_specification_is_type_checked);
      normalise_specification_if_required();
      const sort_expression normalised_sort1=normalize_sorts(s1,*this);
      const sort_expression normalised_sort2=normalize_sorts(s2,*this);
      return (normalised_sort1 == normalised_sort2);
    }

    /// \brief Checks whether a sort is certainly finite.
    ///
    /// \param[in] s A sort expression
    /// \return true if s can be determined to be finite,
    ///      false otherwise.
    bool is_certainly_finite(const sort_expression& s) const;

    /// \brief Checks whether a sort is a constructor sort
    ///
    /// \param[in] s A sort expression
    /// \return true if s is a constructor sort
    bool is_constructor_sort(const sort_expression& s) const
    {
      assert(m_data_specification_is_type_checked);
      normalise_specification_if_required();
      const sort_expression normalised_sort=normalize_sorts(s,*this);
      return !is_function_sort(normalised_sort) && !constructors(normalised_sort).empty();
    }

    /// \brief Returns true if
    /// <ul>
    /// <li>the domain and range sorts of constructors are contained in the list of sorts</li>
    /// <li>the domain and range sorts of mappings are contained in the list of sorts</li>
    /// </ul>
    /// \return True if the data specification is well typed.
    bool is_well_typed() const;

    bool operator==(const data_specification& other) const
    {
      if (!m_data_specification_is_type_checked)
      {
        if (other.m_data_specification_is_type_checked)
        {
          return false;
        }
        return m_non_typed_checked_data_spec==other.m_non_typed_checked_data_spec;
      }
      normalise_specification_if_required();
      other.normalise_specification_if_required();
      return
        // m_sorts_in_context == other.m_sorts_in_context &&
        other.m_data_specification_is_type_checked &&
        m_non_typed_checked_data_spec == other.m_non_typed_checked_data_spec &&
        m_normalised_sorts == other.m_normalised_sorts &&
        m_normalised_constructors == other.m_normalised_constructors &&
        m_normalised_mappings == other.m_normalised_mappings &&
        m_normalised_equations == other.m_normalised_equations;
    }

    data_specification& operator=(const data_specification& other)
    {
      m_data_specification_is_type_checked=other.m_data_specification_is_type_checked;
      m_non_typed_checked_data_spec=other.m_non_typed_checked_data_spec;
      m_normalised_data_is_up_to_date=other.m_normalised_data_is_up_to_date;
      m_sorts=other.m_sorts;
      m_sorts_in_context=other.m_sorts_in_context;
      m_aliases=other.m_aliases;
      m_constructors=other.m_constructors;
      m_mappings=other.m_mappings;
      m_equations=other.m_equations;
      m_normalised_sorts=other.m_normalised_sorts;
      m_normalised_mappings=other.m_normalised_mappings;
      m_normalised_constructors=other.m_normalised_constructors;
      m_normalised_equations=other.m_normalised_equations;
      m_normalised_aliases=other.m_normalised_aliases;
      return *this;
    }

}; // class data_specification


/// \brief Finds a mapping in a data specification.
/// \param data A data specification
/// \param s A string
/// \return The found mapping

inline
function_symbol find_mapping(data_specification const& data, std::string const& s)
{
  data_specification::mappings_const_range r(data.mappings());

  data_specification::mappings_const_range::const_iterator i = std::find_if(r.begin(), r.end(), detail::function_symbol_has_name(s));
  return (i == r.end()) ? function_symbol() : *i;
}

/// \brief Finds a constructor in a data specification.
/// \param data A data specification
/// \param s A string
/// \return The found constructor

inline
function_symbol find_constructor(data_specification const& data, std::string const& s)
{
  data_specification::constructors_const_range r(data.constructors());
  data_specification::constructors_const_range::const_iterator i = std::find_if(r.begin(), r.end(), detail::function_symbol_has_name(s));
  return (i == r.end()) ? function_symbol() : *i;
}

/// \brief Finds a sort in a data specification.
/// \param data A data specification
/// \param s A string
/// \return The found sort

inline
sort_expression find_sort(data_specification const& data, std::string const& s)
{
  data_specification::sorts_const_range r(data.sorts());
  data_specification::sorts_const_range::const_iterator i = std::find_if(r.begin(), r.end(), detail::sort_has_name(s));
  return (i == r.end()) ? sort_expression() : *i;
}

/// \brief Gets all equations with a data expression as head
/// on one of its sides.
///
/// \param[in] specification A data specification.
/// \param[in] d A data expression.
/// \return All equations with d as head in one of its sides.

inline
data_equation_vector find_equations(data_specification const& specification, const data_expression& d)
{
  data_equation_vector result;
  data_specification::equations_const_range equations(specification.equations());
  for (data_specification::equations_const_range::const_iterator i = equations.begin(); i != equations.end(); ++i)
  {
    if (i->lhs() == d || i->rhs() == d)
    {
      result.push_back(*i);
    }
    else if (is_application(i->lhs()))
    {
      if (static_cast<application>(i->lhs()).head() == d)
      {
        result.push_back(*i);
      }
    }
    else if (is_application(i->rhs()))
    {
      if (static_cast<application>(i->rhs()).head() == d)
      {
        result.push_back(*i);
      }
    }
  }
  return result;
}

} // namespace data

} // namespace mcrl2

#ifndef MCRL2_DATA_FIND_H
#include "mcrl2/data/find.h"
#endif

#ifndef MCRL2_DATA_TRANSLATE_USER_NOTATION_H
#include "mcrl2/data/translate_user_notation.h"
#endif

#include "mcrl2/data/normalize_sorts.h"

#endif // MCRL2_DATA_DATA_SPECIFICATION_H

