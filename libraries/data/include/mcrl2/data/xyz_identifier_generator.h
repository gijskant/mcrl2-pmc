// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/xyz_identifier_generator.h
/// \brief Class that generates identifiers in the range X, Y, Z, X0, Y0, Z0, X1, ...

#ifndef MCRL2_DATA_XYZ_IDENTIFIER_GENERATOR_H
#define MCRL2_DATA_XYZ_IDENTIFIER_GENERATOR_H

#include "mcrl2/data/set_identifier_generator.h"

namespace mcrl2 {

namespace data {

/// A class that generates identifiers. A context is maintained
/// containing already used identifiers. The context is stored
/// in a multiset. If an identifier occurs multiple times,
/// multiple calls to remove_from_context are required to
/// remove it.
///
/// Using the operator()() and operator()(std::string) fresh
/// identifiers can be generated that do not appear in the
/// context.
class xyz_identifier_generator: public multiset_identifier_generator
{
  protected:
    int m_index; // index of last generated identifier
    char m_char; // character of last generated identifier

    /// \brief Returns the next name in the range X, Y, Z, X0, Y0, Z0, X1, ...
    ///
    std::string next()
    {
      switch (m_char) {
        case 'X' : {
          m_char = 'Y';
          break;
        }
        case 'Y' : {
          m_char = 'Z';
          break;
        }
        case 'Z' : {
          m_char = 'X';
          m_index++;
          break;
        }
      }
      return m_index < 0 ? std::string(1, m_char) : str(boost::format("%1%%2%") % m_char % m_index);
    }

  public:
    /// Constructor.
    ///
    xyz_identifier_generator()
     : m_index(-2), m_char('Z')
    {}

    /// Constructor.
    ///
    template <typename Term>
    xyz_identifier_generator(Term t)
     : m_index(-2), m_char('Z')
    {
      add_to_context(t);
    }
    
    /// \brief Returns hint if it isn't in the context yet. Else the next available
    /// identifier in the range X, Y, Z, X0, Y0, Z0, X1, ... is returned.
    /// The returned variable is added to the context.
    ///
    core::identifier_string operator()(const std::string& hint)
    {
      core::identifier_string result(hint);

      if (m_identifiers.find(hint) != m_identifiers.end())
      {
        m_index = -2;
        m_char = 'Z';
        do {
          result = core::identifier_string(next());
        } while (m_identifiers.find(result) != m_identifiers.end());
      }

      add_identifier(result);
      return result;
    }
};

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_XYZ_IDENTIFIER_GENERATOR_H
