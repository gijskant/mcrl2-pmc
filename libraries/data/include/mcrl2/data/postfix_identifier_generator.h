// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/postfix_identifier_generator.h
/// \brief The class postfix_identifier_generator.

#ifndef MCRL2_DATA_POSTFIX_IDENTIFIER_GENERATOR_H
#define MCRL2_DATA_POSTFIX_IDENTIFIER_GENERATOR_H

#include "mcrl2/data/set_identifier_generator.h"

namespace mcrl2 {

namespace data {

/// Uses the given postfix as a hint for generating identifiers.
class postfix_identifier_generator: public set_identifier_generator
{
  protected:
    std::string m_postfix;

  public:
    /// Constructor.
    ///
    postfix_identifier_generator(std::string postfix)
      : m_postfix(postfix)
    {}

    /// Returns a unique identifier, with the given hint as prefix.
    /// The returned identifier is added to the context.
    virtual core::identifier_string operator()(const std::string& hint)
    {
      core::identifier_string id(hint);
      int index = 0;
      while (has_identifier(id))
      {   
        std::string name = str(boost::format(hint + m_postfix + "%02d") % index++);
        id = core::identifier_string(name);
      }
      add_to_context(id);
      return id;
    }   
};

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_POSTFIX_IDENTIFIER_GENERATOR_H
