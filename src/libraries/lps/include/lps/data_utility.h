///////////////////////////////////////////////////////////////////////////////
/// \file data_utility.h

#ifndef LPS_DATA_UTILITY_H
#define LPS_DATA_UTILITY_H

#include <string>
#include <utility>
#include <set>
#include <vector>
#include <iterator>
#include <algorithm>

#include "boost/format.hpp"
#include "lps/data.h"
#include "lps/sort.h"
#include "lps/identifier_string.h"
#include "atermpp/algorithm.h"
#include "atermpp/aterm.h"
#include "atermpp/set.h"
#include "atermpp/utility.h"

namespace lps {

using atermpp::aterm;
using atermpp::aterm_traits;

/// Test if a term is an identifier.
struct is_identifier
{
  bool operator()(aterm t) const
  {
    return t.type() == AT_APPL && aterm_appl(t).size() == 0;
  }
};

/// Returns the set of all identifiers occurring in the term t.
template <typename Term>
std::set<identifier_string> identifiers(Term t)
{
  std::set<identifier_string> result;
  find_all_if(aterm_traits<Term>::term(t), is_identifier(), std::inserter(result, result.end()));
  return result;
}

/// Returns the set of all identifiers occurring in the term t.
template <typename Term>
std::set<std::string> identifier_strings(Term t)
{
  std::set<identifier_string> s = identifiers(t);
  std::set<std::string> result;
  for (std::set<identifier_string>::iterator i = s.begin(); i != s.end(); ++i)
    result.insert(*i);
  return result;
}

/// Returns the names of the variables in t.
inline
std::vector<std::string> variable_strings(data_variable_list t)
{
  std::vector<std::string> result;
  for (data_variable_list::iterator i = t.begin(); i != t.end(); ++i)
    result.push_back(i->name());
  return result;
}

/// Returns a copy of t, but with a common postfix added to each variable name,
/// and such that the new names do not appear in context.
///
inline
data_variable_list fresh_variables(data_variable_list t, const std::set<std::string>& context, std::string postfix_format = "_%02d")
{
  std::vector<std::string> ids = variable_strings(t);
  std::string postfix;
  for (int i = 0; ; i++)
  {
    postfix = str(boost::format(postfix_format) % i);
    std::vector<std::string>::iterator j = ids.begin();
    for ( ; j != ids.end(); j++)
    {
      if (context.find(*j + postfix) != context.end())
        break;
    }
    if (j == ids.end()) // success!
      break;
  }
  data_variable_list result;
  for (data_variable_list::iterator k = t.begin(); k != t.end(); ++k)
  {
    identifier_string name(std::string(k->name()) + postfix);
    result = push_front(result, data_variable(gsMakeDataVarId(name, k->sort())));
  }
  return atermpp::reverse(result);
}

/// Returns an identifier that doesn't appear in the term context.
template <typename Term>
identifier_string fresh_identifier(std::string hint, Term context)
{
  std::set<identifier_string> ids = identifiers(context);
  identifier_string s(hint);
  int index = 0;
  while (ids.find(s) != ids.end())
  {   
    std::string name = str(boost::format(hint + "%02d") % index++);
    s = identifier_string(name);
  }
  return s;
}

/// Returns a variable that doesn't appear in context.
template <typename Term>
data_variable fresh_variable(std::string hint, Term context, lps::sort s = sort_expr::real())
{
  identifier_string id = fresh_identifier(hint, context);
  return data_variable(gsMakeDataVarId(id, s));
}

/// Returns all data variables that occur in the term t.
template <typename Term>
std::set<data_variable> find_variables(Term t)
{
  // find all data variables in t
  std::set<data_variable> variables;
  atermpp::find_all_if(t, is_data_variable, std::inserter(variables, variables.end()));
  return variables;
}

/// Returns all names data variables that occur in the term t.
template <typename Term>
std::set<std::string> find_variable_names(Term t)
{
  // find all data variables in t
  std::set<data_variable> variables;
  atermpp::find_all_if(t, is_data_variable, std::inserter(variables, variables.end()));

  std::set<std::string> result;
  for (std::set<data_variable>::iterator j = variables.begin(); j != variables.end(); ++j)
  {
    result.insert(j->name());
  }
  return result;
}

/// Fresh variable generator that generates data variables with
/// names that do not appear in the given context.
class fresh_variable_generator
{
  protected:
    atermpp::set<identifier_string> m_identifiers;
    std::string m_hint;                  // used as a hint for operator()()
    lps::sort m_sort;                    // used for operator()()

  public:
    fresh_variable_generator()
     : m_hint("t"), m_sort(sort_expr::real())
    { }

    template <typename Term>
    fresh_variable_generator(Term context, std::string hint = "t", lps::sort s = sort_expr::real())
    {
      m_identifiers = identifiers(context);
      m_hint = hint;
      m_sort = s;
    }

    /// Set a new hint.
    void set_hint(std::string hint)
    {
      m_hint = hint;
    }

    /// Returns the current hint.
    std::string hint() const
    {
      return m_hint;
    }

    /// Set a new context.
    template <typename Term>
    void set_context(Term context)
    {
      m_identifiers = identifiers(context);
    }

    /// Set a new sort.
    void set_sort(lps::sort s)
    {
      m_sort = s;
    }

    /// Returns the current sort.
    lps::sort sort() const
    {
      return m_sort;
    }

    /// Add term t to the context.
    template <typename Term>
    void add_to_context(Term t)
    {
      std::set<identifier_string> ids = identifiers(t);
      std::copy(ids.begin(), ids.end(), std::inserter(m_identifiers, m_identifiers.end()));
    }

    /// Returns a unique variable of the given sort, with the given hint as prefix.
    /// The returned variable is added to the context.
    data_variable operator()()
    {
      identifier_string id(m_hint);
      int index = 0;
      while (m_identifiers.find(id) != m_identifiers.end())
      {   
        std::string name = str(boost::format(m_hint + "%02d") % index++);
        id = identifier_string(name);
      }
      m_identifiers.insert(id);
      return data_variable(gsMakeDataVarId(id, m_sort));
    }

    /// Returns a unique variable with the same sort as the variable v, and with
    /// the same prefix. The returned variable is added to the context.
    data_variable operator()(data_variable v)
    {
      std::string hint = v.name();
      identifier_string id(hint);
      int index = 0;
      while (m_identifiers.find(id) != m_identifiers.end())
      {   
        std::string name = str(boost::format(hint + "%02d") % index++);
        id = identifier_string(name);
      }
      m_identifiers.insert(id);
      return data_variable(gsMakeDataVarId(id, v.sort()));
    }
};

/// Returns a variable list that doesn't contain terms that appear in context.
template <typename Term>
data_variable_list fresh_variable_list(unsigned int size, Term context, std::string hint, lps::sort s = sort_expr::real())
{
  data_variable_list result;
  fresh_variable_generator generator(context, hint, s);
  for (unsigned int i = 0; i < size; i++)
  {
    result = push_front(result, generator());
  }
  return data_variable_list(atermpp::reverse(result));
}

} // namespace lps

#endif // LPS_DATA_UTILITY_H
