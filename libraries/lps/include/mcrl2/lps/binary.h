// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file binary.h
/// \brief The binary algorithm.

#ifndef MCRL2_LPS_BINARY_H
#define MCRL2_LPS_BINARY_H

#include "mcrl2/lps/specification.h"
#include "mcrl2/data/rewrite.h"

namespace mcrl2 {

namespace lps {

specification binary(const specification& s, Rewriter& r);

} // namespace lps

} // namespace mcrl2

#endif // MCRL2_LPS_BINARY_H
