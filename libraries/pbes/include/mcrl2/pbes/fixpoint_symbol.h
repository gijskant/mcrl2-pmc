// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/fixpoint_symbol.h
/// \brief The class fixpoint_symbol.

#ifndef MCRL2_PBES_FIXPOINT_SYMBOL_H
#define MCRL2_PBES_FIXPOINT_SYMBOL_H

#include <cassert>
#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_list.h"
#include "mcrl2/core/detail/constructors.h"
#include "mcrl2/core/detail/soundness_checks.h"
#include "mcrl2/core/detail/function_symbols.h"
#include "mcrl2/data/data_specification.h"

namespace mcrl2
{

namespace pbes_system
{

//--- start generated class fixpoint_symbol ---//
/// \brief A fixpoint symbol
class fixpoint_symbol: public atermpp::aterm_appl
{
  public:
    /// \brief Default constructor.
    fixpoint_symbol()
      : atermpp::aterm_appl(core::detail::constructFixPoint())
    {}

    /// \brief Constructor.
    /// \param term A term
    explicit fixpoint_symbol(const atermpp::aterm& term)
      : atermpp::aterm_appl(term)
    {
      assert(core::detail::check_rule_FixPoint(*this));
    }
//--- start user section fixpoint_symbol ---//
    /// \brief Returns the mu symbol.
    /// \return The mu symbol.
    static fixpoint_symbol mu()
    {
      return fixpoint_symbol(atermpp::aterm_appl(core::detail::function_symbol_Mu()));
    }

    /// \brief Returns the nu symbol.
    /// \return The nu symbol.
    static fixpoint_symbol nu()
    {
      return fixpoint_symbol(atermpp::aterm_appl(core::detail::function_symbol_Nu()));
    }

    /// \brief Returns true if the symbol is mu.
    /// \return True if the symbol is mu.
    bool is_mu() const
    {
      return function() == core::detail::function_symbols::Mu;
    }

    /// \brief Returns true if the symbol is nu.
    /// \return True if the symbol is nu.
    bool is_nu() const
    {
      return function() == core::detail::function_symbols::Nu;
    }
//--- end user section fixpoint_symbol ---//
};

/// \brief list of fixpoint_symbols
typedef atermpp::term_list<fixpoint_symbol> fixpoint_symbol_list;

/// \brief vector of fixpoint_symbols
typedef std::vector<fixpoint_symbol>    fixpoint_symbol_vector;

// prototype declaration
std::string pp(const fixpoint_symbol& x);

/// \brief Outputs the object to a stream
/// \param out An output stream
/// \return The output stream
inline
std::ostream& operator<<(std::ostream& out, const fixpoint_symbol& x)
{
  return out << pbes_system::pp(x);
}

/// \brief swap overload
inline void swap(fixpoint_symbol& t1, fixpoint_symbol& t2)
{
  t1.swap(t2);
}
//--- end generated class fixpoint_symbol ---//

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_FIXPOINT_SYMBOL_H
