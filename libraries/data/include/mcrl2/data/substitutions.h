// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/substitutions.h
/// \brief add your file description here.

#ifndef MCRL2_DATA_SUBSTITUTIONS_H
#define MCRL2_DATA_SUBSTITUTIONS_H

#include <functional>
#include "mcrl2/data/print.h"
#include "mcrl2/data/replace.h"
#include "mcrl2/data/data_expression.h"

namespace mcrl2 {

namespace data {

/// \brief Substitution that maps a sort expression to a sort expression.
struct sort_expression_assignment: public std::unary_function<sort_expression, sort_expression>
{
  typedef sort_expression variable_type;
  typedef sort_expression expression_type;
  
  sort_expression lhs;
  sort_expression rhs;

  sort_expression_assignment(const sort_expression& lhs_, const sort_expression& rhs_)
  : lhs(lhs_),
    rhs(rhs_)
  {}

  sort_expression operator()(const sort_expression& x)
  {
    if (x == lhs) {
      return rhs;
    }
    return x;
  }
};

/// \brief Substitution that maps a data expression to a data expression
struct data_expression_assignment: public std::unary_function<data_expression, data_expression>
{
  typedef data_expression variable_type;
  typedef data_expression expression_type;
  
  data_expression lhs;
  data_expression rhs;

  data_expression_assignment(const data_expression& lhs_, const data_expression& rhs_)
  : lhs(lhs_),
    rhs(rhs_)
  {}

  data_expression operator()(const data_expression& x)
  {
    if (x == lhs) {
      return rhs;
    }
    return x;
  }
};

/// \brief Substitution that maps data variables to data expressions. The substitution is stored as an
/// assignment_list.
struct assignment_sequence_substitution : public std::unary_function<variable, data_expression>
{
  typedef variable variable_type;
  typedef data_expression expression_type;

  assignment_list assignments;

  assignment_sequence_substitution(assignment_list assignments_)
    : assignments(assignments_)
  {}

  data_expression operator()(const variable& v) const
  {
    for (assignment_list::const_iterator i = assignments.begin(); i != assignments.end(); ++i)
    {
      if (i->lhs() == v)
      {
        return i->rhs();
      }
    }
    return v;
  }
};

/// \brief Generic substitution function. The substitution is stored as a sequence
/// of variables and a sequence of expressions.
template <typename VariableContainer, typename ExpressionContainer>
struct sequence_sequence_substitution: public std::unary_function<typename VariableContainer::value_type, typename ExpressionContainer::value_type>
{
  /// \brief type used to represent variables
  typedef typename VariableContainer::value_type variable_type;

  /// \brief type used to represent expressions
  typedef typename ExpressionContainer::value_type  expression_type;

  const VariableContainer& variables;
  const ExpressionContainer& expressions;

  sequence_sequence_substitution(const VariableContainer& variables_, const ExpressionContainer& expressions_)
    : variables(variables_),
      expressions(expressions_)
  {
    assert(variables.size() == expressions.size());
  }

  expression_type operator()(const variable_type& v) const
  {
    typename VariableContainer::const_iterator i = variables.begin();
    typename ExpressionContainer::const_iterator j = expressions.begin();

    for (; i != variables.end(); ++i, ++j)
    {
      if (*i == v)
      {
        return *j;
      }
    }
    return v;
  }

  template <typename Expression>
  expression_type operator()(const Expression&) const
  {
    throw std::runtime_error("data::sequence_sequence_substitution::operator(const Expression&) is a deprecated interface!");
    return data_expression();
  }
};

/// \brief Utility function for creating a sequence_sequence_substitution.
template <typename VariableContainer, typename ExpressionContainer>
sequence_sequence_substitution<VariableContainer, ExpressionContainer>
make_sequence_sequence_substitution(const VariableContainer& vc, const ExpressionContainer& ec)
{
  return sequence_sequence_substitution<VariableContainer, ExpressionContainer>(vc, ec);
}

/// \brief Generic substitution function. The substitution is stored as a sequence
/// of pairs of variables and expressions.
template <typename Container>
struct pair_sequence_substitution: public std::unary_function<typename Container::value_type::first_type, typename Container::value_type::second_type>
{
  /// \brief type used to represent variables
  typedef typename Container::value_type::first_type variable_type;

  /// \brief type used to represent expressions
  typedef typename Container::value_type::second_type expression_type;

  const Container& container;

  pair_sequence_substitution(const Container& container_)
    : container(container_)
  {}

  expression_type operator()(const variable_type& v) const
  {
    for (typename Container::const_iterator i = container.begin(); i != container.end(); ++i)
    {
      if (i->first == v)
      {
        return i->second;
      }
    }
    return v;
  }

  template <typename Expression>
  expression_type operator()(const Expression&) const
  {
    throw std::runtime_error("data::pair_sequence_substitution::operator(const Expression&) is a deprecated interface!");
    return data_expression();
  }
};

/// \brief Utility function for creating a pair_sequence_substitution.
template <typename Container>
pair_sequence_substitution<Container>
make_pair_sequence_substitution(const Container& c)
{
  return pair_sequence_substitution<Container>(c);
}

/// \brief Generic substitution function. The substitution is stored as a mapping
/// of variables to expressions.
template <typename AssociativeContainer>
struct map_substitution : public std::unary_function<typename AssociativeContainer::key_type, typename AssociativeContainer::mapped_type>
{
  typedef typename AssociativeContainer::key_type variable_type;
  typedef typename AssociativeContainer::mapped_type expression_type;

  const AssociativeContainer& m_map;

  map_substitution(const AssociativeContainer& m)
    : m_map(m)
  { }

  expression_type operator()(const variable_type& v) const
  {
    typename AssociativeContainer::const_iterator i = m_map.find(v);
    if (i == m_map.end())
    {
      return v;
    }
    else
    {
      return i->second;
    }
    // N.B. This does not work!
    // return i == m_map.end() ? v : i->second;
  }

  template <typename Expression>
  expression_type operator()(const Expression&) const
  {
    throw std::runtime_error("data::map_substitution::operator(const Expression&) is a deprecated interface!");
    return data_expression();
  }
};

/// \brief Utility function for creating a map_substitution.
template <typename AssociativeContainer>
map_substitution<AssociativeContainer>
make_map_substitution(const AssociativeContainer& m)
{
  return map_substitution<AssociativeContainer>(m);
}

/// \brief Generic substitution function. The substitution is stored as a mapping
/// of variables to expressions. The substitution is mutable, meaning that substitutions
/// to variables can be added and removed as follows:
///\verbatim
/// mutable_map_substitution sigma;
/// mutable_map_substitution::variable_type v;
/// mutable_map_substitution::expression_type e;
/// sigma[v] = e; // add the assignment of e to v
/// sigma[v] = v; // remove the assignment to v
///\endverbatim
template <typename AssociativeContainer = atermpp::map<variable,data_expression> >
class mutable_map_substitution : public std::unary_function<typename AssociativeContainer::key_type, typename AssociativeContainer::mapped_type>
{
protected:
  AssociativeContainer m_map;

public:
  typedef typename AssociativeContainer::key_type variable_type;
  typedef typename AssociativeContainer::mapped_type expression_type;
  typedef typename AssociativeContainer::const_iterator const_iterator;
  typedef typename AssociativeContainer::iterator iterator;
  
  /// \brief Friend functions that collect some details of a substitution,
  /// needed for rewriting in internal format, as well as alpha-conversion.
  template<typename AssociativeContainer1, typename UnaryOperator>
  friend mutable_map_substitution< typename atermpp::map<typename AssociativeContainer1::key_type, atermpp::aterm_appl> >
  apply(const mutable_map_substitution< AssociativeContainer1 >& sigma, UnaryOperator f);

  template<typename AssociativeContainer1>
  friend atermpp::set<core::identifier_string> get_identifiers(const mutable_map_substitution< AssociativeContainer1 >& sigma);


  /// \brief Wrapper class for internal storage and substitution updates using operator()
  struct assignment
  {
    typename AssociativeContainer::key_type m_variable;
    AssociativeContainer&                   m_map;

    /// \brief Constructor.
    ///
    /// \param[in] v a variable.
    /// \param[in] m a mapping of variables to expressions.
    assignment(typename AssociativeContainer::key_type v, AssociativeContainer& m) :
      m_variable(v), m_map(m)
    { }

    template <typename AssignableToExpression>
    void operator=(AssignableToExpression const& e)
    {
      mCRL2log(log::debug2, "substitutions") << "Setting " << data::pp(m_variable) << " := " << e << std::endl;
      if (e != m_variable)
      {
        m_map[m_variable] = e;
      }
      else
      {
        m_map.erase(m_variable);
      }
    }
  };

  mutable_map_substitution()
  {}

  mutable_map_substitution(const AssociativeContainer& m)
    : m_map(m)
  {}

  template <typename VariableContainer, typename ExpressionContainer>
  mutable_map_substitution(VariableContainer const& vc, ExpressionContainer const& ec)
  {
    BOOST_ASSERT(vc.size() == ec.size());

    typename ExpressionContainer::const_iterator j = ec.begin();
    for (typename VariableContainer::const_iterator i = vc.begin(); i != vc.end(); ++i, ++j)
    {
      m_map[*i] = *j;
    }
  }

  expression_type operator()(const variable_type& v) const
  {
    typename AssociativeContainer::const_iterator i = m_map.find(v);
    expression_type result;
    if (i == m_map.end())
    {
      result = expression_type(v);
    }
    else
    {
      result = i->second;
    }
    mCRL2log(log::debug2, "substitutions") << "sigma(" << v <<") = " << result << std::endl;
    return result;
    // N.B. This does not work!
    // return i == m_map.end() ? v : i->second;
  }

  assignment operator[](variable_type const& v)
  {
    return assignment(v, this->m_map);
  }

  /// \brief Resets the substitution by letting every variable yield itself. Cf. clear() of a map.
  ///        
  void clear() 
  {
    m_map.clear();
  }

  template <typename Substitution>
  bool operator==(const Substitution&) const
  {
    return false;
  }

  mutable_map_substitution & operator=(const mutable_map_substitution& other)
  {
    m_map=other.m_map;
    return *this;
  }

  /// \brief Returns an iterator pointing to the beginning of the sequence of assignments
  /// TODO: should become protected
  const_iterator begin() const
  {
    return m_map.begin();
  }

  /// \brief Returns an iterator pointing past the end of the sequence of assignments
  /// TODO: should become protected
  const_iterator end() const
  {
    return m_map.end();
  }

  /// \brief Returns an iterator pointing to the beginning of the sequence of assignments
  /// TODO: should become protected
  iterator begin()
  {
    return this->m_map.begin();
  }

  /// \brief Returns an iterator pointing past the end of the sequence of assignments
  /// TODO: should become protected
  iterator end()
  {
    return this->m_map.end();
  }

  /// \brief Returns true if the substitution is empty
  bool empty()
  {
    return m_map.empty();
  }

  std::string to_string() const
  {
    std::stringstream result;
    result << "[";
    for (const_iterator i = begin(); i != end(); ++i)
    {
      result << (i == begin() ? "" : "; ") << data::pp(i->first) << ":" << data::pp(i->first.sort()) << " := " << data::pp(i->second);
    }
    result << "]";
    return result.str();
  }

protected:

  /// \brief Returns an iterator that references the expression associated with v or is equal to m_map.end()
  iterator find(variable_type const& v)
  {
    return this->m_map.find(v);
  }

  /// \brief Returns an iterator that references the expression associated with v or is equal to m_map.end()
  const_iterator find(variable_type const& v) const
  {
    return m_map.find(v);
  }

};

/// \brief Utility function for creating a mutable_map_substitution.
template <typename VariableContainer, typename ExpressionContainer, typename MapContainer>
mutable_map_substitution<MapContainer>
make_mutable_map_substitution(const VariableContainer& vc, const ExpressionContainer& ec)
{
  return mutable_map_substitution<MapContainer>(vc, ec);
}

template <typename VariableContainer, typename ExpressionContainer>
mutable_map_substitution<std::map<typename VariableContainer::value_type, typename ExpressionContainer::value_type> >
make_mutable_map_substitution(const VariableContainer& vc, const ExpressionContainer& ec)
{
  return mutable_map_substitution<std::map<typename VariableContainer::value_type, typename ExpressionContainer::value_type> >(vc, ec);
}

namespace detail
{
  struct variable_to_index : public std::unary_function< data::variable, size_t >
  {
    variable_to_index()
    {}

    size_t operator()(const data::variable& v) const
    {
      size_t result = v.name().function();
      return result;
    }
  };
}

/// \brief Generic substitution function.
/// \details This substitution assumes a function variable -> size_t, that, for
///          each variable gives a unique index. The substitutions are stored
///          internally as a vector, mapping size_t to expression.
///          Provided that, given a variable, its index can be computed in O(1)
///          time, insertion is O(1) amortized, and lookup is O(1).
///          Memory required is O(n) where n is the largest index used.
template <typename VariableType = data::variable, typename ExpressionSequence = atermpp::vector<data_expression> >
class mutable_indexed_substitution : public std::unary_function<VariableType, typename ExpressionSequence::value_type>
{
protected:
  /// \brief Internal storage for substitutions.
  /// Required to be a container with random access through [] operator.
  ExpressionSequence m_container;

public:

  /// \brief Friend function to get all identifiers in the substitution
  template<typename VariableType1, typename ExpressionSequence1>
  friend atermpp::set<core::identifier_string> get_identifiers(const mutable_indexed_substitution< VariableType1, ExpressionSequence1 >& sigma);

  /// \brief Friend function that applies a function to all right hand sides of the substitution.
  template<typename VariableType1, typename ExpressionSequence1, typename UnaryOperator>
  friend mutable_indexed_substitution<VariableType1, atermpp::vector<atermpp::aterm_appl> >
  apply(const mutable_indexed_substitution<VariableType1, ExpressionSequence1 >& sigma, UnaryOperator f);

  /// \brief Type of variables
  typedef VariableType variable_type;

  /// \brief Type of expressions
  typedef typename ExpressionSequence::value_type expression_type;

  /// \brief Default constructor
  mutable_indexed_substitution()
  {}

  /// \brief Copy constructor
  mutable_indexed_substitution(const ExpressionSequence& c)
    : m_container(c)
  {}

  /// \brief Wrapper class for internal storage and substitution updates using operator()
  struct assignment
  {
    variable_type        m_variable;
    ExpressionSequence&  m_container;

    /// \brief Constructor.
    ///
    /// \param[in] v a variable.
    /// \param[in] c a container of expressions.
    assignment(variable_type v, ExpressionSequence& c) :
      m_variable(v), m_container(c)
    { }

    /// \brief Actual assignment
    template <typename AssignableToExpression>
    void operator=(AssignableToExpression const& e)
    {
      mCRL2log(log::debug2, "substitutions") << "Setting " << data::pp(m_variable) << " := " << e << std::endl;

      size_t i = detail::variable_to_index()(m_variable);

      // Resize container if needed
      if(i >= m_container.size())
      {
        m_container.resize(i+1, expression_type());
      }

      // update substitution
      if (e != m_variable)
      {
        m_container[i] = e;
      }
      else
      {
        m_container[i] = expression_type();
      }
    }
  };

  /// \brief Application operator; applies substitution to v.
  expression_type operator()(const variable_type& v) const
  {
    size_t i = detail::variable_to_index()(v);
    expression_type result;
    if(i < m_container.size() && m_container[i] != expression_type())
    {
      result = m_container[i];
    }
    else
    {
      result = expression_type(v);
    }
    mCRL2log(log::debug2, "substitutions") << "sigma(" << v <<") = " << result << std::endl;
    return result;
  }

  /// \brief Index operator.
  assignment operator[](variable_type const& v)
  {
    return assignment(v, this->m_container);
  }

  /// \brief Clear substitutions.
  void clear()
  {
    m_container.assign(m_container.size(), expression_type());
  }

  /// \brief Compare substitutions
  template <typename Substitution>
  bool operator==(const Substitution&) const
  {
    return false;
  }

  /// \brief Assignment operator
  mutable_indexed_substitution & operator=(const mutable_indexed_substitution& other)
  {
    m_container=other.m_container;
    return *this;
  }

  /// \brief Returns true if the substitution is empty
  bool empty()
  {
    for(typename ExpressionSequence::const_iterator i = m_container.begin(); i != m_container.end(); ++i)
    {
      if(*i != expression_type())
      {
        return false;
      }
    }
    return true;
  }

protected:
  /// \brief size of the wrapped container
  size_t size() const
  {
    return m_container.size();
  }

  /// \brief resize the wrapped container
  void resize(const size_t n)
  {
    m_container.resize(n);
  }

  /// \brief set position i of the wrapped container to e
  void set(const size_t i, const expression_type& e)
  {
    mCRL2log(log::debug2, "substitutions") << "Setting " << static_cast<atermpp::function_symbol>(i).name() << " := " << e << std::endl;
    m_container[i] = e;
  }

  /// \brief get the element at position i of the wrapped container
  expression_type get(const size_t i) const
  {
    assert(i < m_container.size());
    return m_container[i];
  }

public:
  /// \brief string representation of the substitution
  std::string to_string() const
  {
    std::stringstream result;
    bool first = true;
    result << "[";
    for (size_t i = 0; i < size(); ++i)
    {
      if(get(i) != expression_type())
      {
        if(first)
        {
          first = false;
        }
        else
        {
          result << "; ";
        }
        result << core::identifier_string(static_cast<atermpp::function_symbol>(i).name()) << " := " << data::pp(get(i));
      }
    }
    result << "]";
    return result.str();
  }

};

/// \brief Function to get all identifiers in the substitution
/// \deprecated
template<typename VariableType, typename ExpressionSequence>
atermpp::set<core::identifier_string> get_identifiers(const mutable_indexed_substitution<VariableType, ExpressionSequence >& sigma)
{
  typedef typename ExpressionSequence::value_type expression_type;

  atermpp::set<core::identifier_string> result;
  for(size_t i = 0; i < sigma.size(); ++i)
  {
    if(sigma.get(i) != expression_type())
    {
      result.insert(core::identifier_string(static_cast<atermpp::function_symbol>(i).name()));
      find_all_if(sigma.get(i),core::is_identifier_string,std::inserter(result,result.begin()));
    }
  }
  return result;
}

/// \deprecated
/// Provided for use with the rewriters in internal format
template<typename AssociativeContainer>
atermpp::set<core::identifier_string> get_identifiers(const mutable_map_substitution< AssociativeContainer >& sigma)
{
  atermpp::set<core::identifier_string> result;
  for(typename mutable_map_substitution< AssociativeContainer >::const_iterator i = sigma.begin(); i != sigma.end(); ++i)
  {
    result.insert(i->first.name());
    find_all_if(i->second,core::is_identifier_string,std::inserter(result,result.begin()));
  }
  return result;
}

/// \deprecated
/// Provided for use with the rewriters in internal format
/// \brief Friend function that applies a function to all right hand sides of the substitution.
template<typename VariableType, typename ExpressionSequence, typename UnaryOperator>
mutable_indexed_substitution<VariableType, atermpp::vector<atermpp::aterm_appl> >
apply(const mutable_indexed_substitution<VariableType, ExpressionSequence >& sigma, UnaryOperator f)
{
  mutable_indexed_substitution<VariableType, atermpp::vector<atermpp::aterm_appl> > result;
  result.resize(sigma.size());
  for(size_t i = 0; i < sigma.size(); ++i)
  {
    if(sigma.get(i) != data_expression())
    {
      result.set(i, f(sigma.get(i)));
    }
  }
  return result;
}

/// \deprecated
/// Provided for use with the rewriters in internal format
template<typename AssociativeContainer, typename UnaryOperator>
mutable_map_substitution< atermpp::map<typename AssociativeContainer::key_type, atermpp::aterm_appl > >
apply(const mutable_map_substitution< AssociativeContainer >& sigma, UnaryOperator f)
{
  mutable_map_substitution< atermpp::map<typename AssociativeContainer::key_type, atermpp::aterm_appl > > result;
  for(typename mutable_map_substitution< AssociativeContainer >::const_iterator i = sigma.begin(); i != sigma.end(); ++i)
  {
    result[i->first] = f(i->second);
  }
  return result;
}

/// \brief An adapter that makes an arbitrary substitution function mutable.
template <typename Substitution>
class mutable_substitution_composer: public std::unary_function<typename Substitution::variable_type, typename Substitution::expression_type>
{
  public:
    /// \brief type used to represent variables
    typedef typename Substitution::variable_type variable_type;

    /// \brief type used to represent expressions
    typedef typename Substitution::expression_type expression_type;

    /// \brief Wrapper class for internal storage and substitution updates using operator()
    typedef typename mutable_map_substitution<atermpp::map<variable_type, expression_type> >::assignment assignment;

    /// \brief The type of the wrapped substitution
    typedef Substitution substitution_type;

  protected:
    /// \brief The wrapped substitution
    const Substitution& f_;

    /// \brief An additional mutable substitution
    mutable_map_substitution<atermpp::map<variable_type, expression_type> > g_;

  public:
    /// \brief Constructor
    mutable_substitution_composer(const Substitution& f)
      : f_(f)
    {}

    /// \brief Apply on single single variable expression
    /// \param[in] v the variable for which to give the associated expression
    /// \return expression equivalent to <|s|>(<|e|>), or a reference to such an expression
    expression_type operator()(variable_type const& v) const
    {
      return data::replace_free_variables(f_(v), g_);
    }

    template <typename Expression>
    expression_type operator()(const Expression&) const
    {
      throw std::runtime_error("data::mutable_substitution_composer::operator(const Expression&) is a deprecated interface!");
      return data_expression();
    }

    assignment operator[](variable_type const& v)
    {
      return g_[v];
    }

    /// \brief Returns the wrapped substitution
    /// \return The wrapped substitution
    const substitution_type& substitution() const
    {
      return f_;
    }
};

/// \brief Specialization for mutable_map_substitution.
template <typename AssociativeContainer>
class mutable_substitution_composer<mutable_map_substitution<AssociativeContainer> >: public std::unary_function<typename AssociativeContainer::key_type, typename AssociativeContainer::mapped_type>
{
  public:
    /// \brief The type of the wrapped substitution
    typedef mutable_map_substitution<AssociativeContainer> substitution_type;

    /// \brief type used to represent variables
    typedef typename substitution_type::variable_type variable_type;

    /// \brief type used to represent expressions
    typedef typename substitution_type::expression_type expression_type;

    /// \brief Wrapper class for internal storage and substitution updates using operator()
    typedef typename substitution_type::assignment assignment;

  protected:
    /// \brief object on which substitution manipulations are performed
    mutable_map_substitution<AssociativeContainer>& g_;

  public:

    /// \brief Constructor with mutable substitution object
    /// \param[in,out] g underlying substitution object
    mutable_substitution_composer(mutable_map_substitution<AssociativeContainer>& g)
      : g_(g)
    {}

    /// \brief Apply on single single variable expression
    /// \param[in] v the variable for which to give the associated expression
    /// \return expression equivalent to <|s|>(<|e|>), or a reference to such an expression
    expression_type operator()(variable_type const& v) const
    {
      return g_(v);
    }

    template <typename Expression>
    expression_type operator()(const Expression&) const
    {
      throw std::runtime_error("data::mutable_substitution_composer<mutable_map_substitution<AssociativeContainer> >::operator(const Expression&) is a deprecated interface!");
      return data_expression();
    }

    assignment operator[](variable_type const& v)
    {
      return g_[v];
    }

    /// \brief Returns the wrapped substitution
    /// \return The wrapped substitution
    const substitution_type& substitution() const
    {
      return g_;
    }
};

/// \brief Returns a string representation of the map, for example [a := 3, b := true].
/// \param sigma a substitution.
/// \return A string representation of the map.
template <typename Substitution>
std::string print_substitution(const Substitution& sigma)
{
  return sigma.to_string();
}

/// \brief Returns a string representation of the map, for example [a := 3, b := true].
/// \param[in] sigma a constant reference to an object of a mutable_substitution_composer instance
/// \return A string representation of the map.
template <typename Substitution>
std::string print_substitution(const mutable_substitution_composer<Substitution>& sigma)
{
  return to_string(sigma.substitution());
}

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_SUBSTITUTIONS_H
