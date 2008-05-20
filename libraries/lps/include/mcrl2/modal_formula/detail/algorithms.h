// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING).
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file algorithms.h
/// \brief Add your file description here.

#ifndef MCRL2_MODAL_FORMULA_DETAIL_ALGORITHMS_H
#define MCRL2_MODAL_FORMULA_DETAIL_ALGORITHMS_H

#include <stdexcept>
#include <sstream>
#include <climits>
#include <iostream>
#include <sstream>
#include <fstream>
#include "aterm2.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/core/detail/parse.h"
#include "mcrl2/core/detail/typecheck.h"
#include "mcrl2/core/detail/data_implementation.h"
#include "mcrl2/core/detail/data_reconstruct.h"
#include "mcrl2/core/detail/regfrmtrans.h"
#include "mcrl2/modal_formula/state_formula.h"
#include "mcrl2/lps/specification.h"

namespace mcrl2 {

namespace modal {

namespace detail {

  inline
  ATermAppl parse_state_formula(std::istream& from)
  {
    ATermAppl result = core::detail::parse_state_frm(from);
    if (result == NULL)
      throw std::runtime_error("parse error in parse_state_frm()");
    return result;
  }
  
  /// type check a state formula against spec,
  /// spec is a lps specification before data implementation, or
  /// a pbes specification before data implementation, or
  /// a data specification before data implementation.
  inline
  ATermAppl type_check_state_formula(ATermAppl formula, ATermAppl spec)
  {
    ATermAppl result = core::detail::type_check_state_frm(formula, spec);
    if (result == NULL)
      throw std::runtime_error("type check error");
    return result;
  }
  
  /// implement sorts and data expressions in formula,
  /// using the data from spec. 
  /// spec is a lps specification before data implementation, or
  /// a pbes specification before data implementation, or
  /// a data specification before data implementation.
  inline
  ATermAppl implement_data_state_formula(ATermAppl formula, ATermAppl& spec)
  {
    ATermAppl result = core::detail::implement_data_state_frm(formula, spec);
    if (result == NULL)
      throw std::runtime_error("data implementation error");
    return result;
  }
  
  inline
  state_formula translate_regular_formula(ATermAppl formula)
  {
    ATermAppl result = core::detail::translate_reg_frms(formula);
    if (result == NULL)
      throw std::runtime_error("formula translation error");
    return result;
  }

  // spec may be updated as the data implementation of the state formula
  // may cause internal names to change.
  inline
  state_formula mcf2statefrm(const std::string& formula_text, lps::specification& spec)
  {
    std::stringstream formula_stream;
    formula_stream << formula_text;
    ATermAppl f = parse_state_formula(formula_stream);
    ATermAppl reconstructed_spec = mcrl2::core::detail::reconstruct_spec(spec);
    f = type_check_state_formula(f, reconstructed_spec);
    f = implement_data_state_formula(f, reconstructed_spec);
    f = translate_regular_formula(f);
    spec = lps::specification(reconstructed_spec);
    return f;
  }

} // namespace detail

} // namespace modal

} // namespace mcrl2

#endif // MCRL2_MODAL_FORMULA_DETAIL_ALGORITHMS_H
