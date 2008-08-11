// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/modal_formula/parse.h
/// \brief add your file description here.

#ifndef MCRL2_MODAL_FORMULA_PARSE_H
#define MCRL2_MODAL_FORMULA_PARSE_H

#include <iostream>
#include "mcrl2/core/parse.h"
#include "mcrl2/modal_formula/state_formula.h"

namespace mcrl2 {

namespace modal {

  inline
  std::istream& operator>>(std::istream& from, state_formula& f)
  {
    f = state_formula(core::parse_state_frm(from));
    return from;
  }

} // namespace modal

} // namespace mcrl2

#endif // MCRL2_MODAL_FORMULA_PARSE_H
