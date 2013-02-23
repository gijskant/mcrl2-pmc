// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/set_identifier_generator.h
/// \brief The classes set_identifier_generator and multiset_identifier_generator.

#ifndef MCRL2_DATA_SET_IDENTIFIER_GENERATOR_H
#define MCRL2_DATA_SET_IDENTIFIER_GENERATOR_H

#include "mcrl2/data/identifier_generator.h"

namespace mcrl2
{

namespace data
{

/// \brief Identifier generator that stores the identifiers of the
/// context in a set.
/// Using the operator()() and operator()(std::string) fresh
/// identifiers can be generated that do not appear in the
/// context.
class set_identifier_generator: public identifier_generator<>
{
  protected:
    /// \brief The context of the identifier generator.
    std::set<core::identifier_string> m_identifiers;

  public:
    /// \brief Constructor.
    set_identifier_generator()
    {}

    /// \brief Clears the context.
    void clear_context()
    {
      m_identifiers.clear();
      m_generator = utilities::number_postfix_generator();
    }

    /// \brief Adds the identifier s to the context.
    /// \param s A
    void add_identifier(const core::identifier_string& s)
    {
      m_identifiers.insert(s);
    }

    /// \brief Removes one occurrence of the identifier s from the context.
    /// \param s A
    void remove_identifier(const core::identifier_string& s)
    {
      m_identifiers.erase(s);
    }

    /// \brief Returns true if the identifier s appears in the context.
    /// \param s A
    /// \return True if the identifier s appears in the context.
    bool has_identifier(const core::identifier_string& s) const
    {
      return m_identifiers.find(s) != m_identifiers.end();
    }

    /// \brief Returns the context.
    /// \return The context.
    const std::set<core::identifier_string>& context() const
    {
      return m_identifiers;
    }
};

/// \brief Identifier generator that stores the identifiers of the
/// context in a multiset.
/// If an identifier occurs multiple times,
/// multiple calls to remove_from_context are required to
/// remove it.
/// Using the operator()() and operator()(std::string) fresh
/// identifiers can be generated that do not appear in the
/// context.
class multiset_identifier_generator: public identifier_generator<>
{
  protected:
    /// \brief The context of the identifier generator.
    std::multiset<core::identifier_string> m_identifiers;

  public:
    /// \brief Constructor.
    multiset_identifier_generator()
    {}

    /// \brief Clears the context.
    void clear_context()
    {
      m_identifiers.clear();
      m_generator = utilities::number_postfix_generator();
    }

    /// \brief Adds the identifier s to the context.
    /// \param s A
    void add_identifier(const core::identifier_string& s)
    {
      m_identifiers.insert(s);
    }

    /// \brief Removes one occurrence of the identifier s from the context.
    /// \param s A
    void remove_identifier(const core::identifier_string& s)
    {
      std::multiset<core::identifier_string>::iterator i = m_identifiers.find(s);
      if (i != m_identifiers.end())
      {
        m_identifiers.erase(i);
      }
    }

    /// \brief Returns true if the identifier s appears in the context.
    /// \param s A
    /// \return True if the identifier s appears in the context.
    bool has_identifier(const core::identifier_string& s) const
    {
      return m_identifiers.find(s) != m_identifiers.end();
    }

    /// \brief Returns the context.
    /// \return The context.
    const std::multiset<core::identifier_string>& context() const
    {
      return m_identifiers;
    }
};

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_SET_IDENTIFIER_GENERATOR_H
