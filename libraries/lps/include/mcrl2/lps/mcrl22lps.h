// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/mcrl22lps.h
/// \brief The mcrl22lps algorithm.

#ifndef MCRL2_LPS_MCRL22LPS_H
#define MCRL2_LPS_MCRL22LPS_H

#include <string>
#include <sstream>
#include "mcrl2/lps/lin_std.h"
#include "mcrl2/lps/lin_types.h"
#include "mcrl2/lps/detail/algorithms.h"

namespace mcrl2 {

namespace lps {

  /// Generates a linearized process specification from a specification in text.
  inline
  specification mcrl22lps(const std::string& spec, t_lin_options options = t_lin_options())
  {   
    using namespace lps::detail;

    lin_std_initialize_global_variables();
    
    // the lineariser expects data from a stream...
    std::stringstream spec_stream;
    spec_stream << spec;

    ATermAppl result = detail::parse_specification(spec_stream);
    result           = detail::type_check_specification(result);
    result           = detail::alpha_reduce(result);
    result           = detail::implement_data_specification(result);
    return linearise(result, options);
  }

} // namespace lps

} // namespace mcrl2

#endif // MCRL2_LPS_MCRL22LPS_H
