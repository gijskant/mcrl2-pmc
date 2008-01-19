// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file free_variables.h
/// \brief Add your file description here.

#ifndef MCRL2_LPS_DETAIL_FREE_VARIABLES_H
#define MCRL2_LPS_DETAIL_FREE_VARIABLES_H

#include <algorithm>
#include <set>
#include "mcrl2/data/data.h"
#include "mcrl2/lps/summand.h"
#include "mcrl2/data/detail/free_variables.h"
#include "mcrl2/atermpp/algorithm.h"

namespace mcrl2 {

namespace lps {

namespace detail {

/// \brief Collects the free variables that occur in the specification and writes them to dest.
template <typename OutputIterator>
void collect_free_variables(const summand& s, const std::set<data_variable>& bound_variables, OutputIterator dest)
{
  atermpp::for_each(s, mcrl2::data::detail::make_data_variable_collector(bound_variables.begin(),
                                                            bound_variables.end(),
                                                            s.summation_variables().begin(),
                                                            s.summation_variables().end(),
                                                            dest
                                                           ));  
}

} // namespace detail

} // namespace lps

} // namespace mcrl2

#endif // MCRL2_LPS_DETAIL_FREE_VARIABLES_H
