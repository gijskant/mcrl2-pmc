// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/precedence.h
/// \brief add your file description here.

#ifndef MCRL2_DATA_PRECEDENCE_H
#define MCRL2_DATA_PRECEDENCE_H

#include "mcrl2/core/detail/precedence.h"
#include "mcrl2/data/bag.h"
#include "mcrl2/data/bool.h"
#include "mcrl2/data/int.h"
#include "mcrl2/data/list.h"
#include "mcrl2/data/nat.h"
#include "mcrl2/data/set.h"
#include "mcrl2/data/real.h"

namespace mcrl2 {

namespace data {

using namespace core::detail::precedences;

namespace detail {

  inline
  bool is_plus(const application& x)
  {
    return sort_int::is_plus_application(x) ||
           sort_nat::is_plus_application(x) ||
           sort_pos::is_plus_application(x) ||
           sort_real::is_plus_application(x);
  }

  inline
  bool is_minus(const application& x)
  {
    return sort_int::is_minus_application(x) ||
           sort_real::is_minus_application(x);
  }

  inline
  bool is_mod(const application& x)
  {
    return sort_int::is_mod_application(x) ||
           sort_nat::is_mod_application(x);
  }

  inline
  bool is_div(const application& x)
  {
    return sort_int::is_div_application(x) ||
           sort_nat::is_div_application(x);
  }

  inline
  bool is_divmod(const application& x)
  {
    return sort_nat::is_divmod_application(x);
  }

  inline
  bool is_divides(const application& x)
  {
    return sort_real::is_divides_application(x);
  }

  inline
  bool is_implies(const application& x)
  {
    return sort_bool::is_implies_application(x);
  }

  inline
  bool is_set_union(const application& x)
  {
    return sort_set::is_setunion_application(x);
  }

  inline
  bool is_set_difference(const application& x)
  {
    return sort_set::is_setdifference_application(x);
  }

  inline
  bool is_bag_join(const application& x)
  {
    return sort_bag::is_bagjoin_application(x);
  }

  inline
  bool is_bag_difference(const application& x)
  {
    return sort_bag::is_bagdifference_application(x);
  }

  inline
  bool is_and(const application& x)
  {
    return sort_bool::is_and_application(x);
  }

  inline
  bool is_or(const application& x)
  {
    return sort_bool::is_or_application(x);
  }

  inline
  bool is_equal_to(const application& x)
  {
    return data::is_equal_to_application(x);
  }

  inline
  bool is_not_equal_to(const application& x)
  {
    return data::is_not_equal_to_application(x);
  }

  inline
  bool is_less(const application& x)
  {
    return data::is_less_application(x);
  }

  inline
  bool is_less_equal(const application& x)
  {
    return data::is_less_equal_application(x);
  }

  inline
  bool is_greater(const application& x)
  {
    return data::is_greater_application(x);
  }

  inline
  bool is_greater_equal(const application& x)
  {
    return data::is_greater_equal_application(x);
  }

  inline
  bool is_in(const application& x)
  {
    return sort_list::is_in_application(x);
  }

  inline
  bool is_times(const application& x)
  {
    return sort_int::is_times_application(x);
  }

  inline
  bool is_element_at(const application& x)
  {
    return sort_list::is_element_at_application(x);
  }

  inline
  bool is_set_intersection(const application& x)
  {
    return sort_set::is_setintersection_application(x);
  }

  inline
  bool is_bag_intersection(const application& x)
  {
    return sort_bag::is_bagintersect_application(x);
  }

  inline
  bool is_concat(const application& x)
  {
    return sort_list::is_concat_application(x);
  }

  inline
  bool is_cons_list(data_expression x)
  {
    while (sort_list::is_cons_application(x))
    {
      x = sort_list::tail(x);
    }
    return sort_list::is_nil_function_symbol(x);
  }

  inline
  bool is_snoc_list(data_expression x)
  {
    while (sort_list::is_snoc_application(x))
    {
      x = sort_list::rtail(x);
    }
    return sort_list::is_nil_function_symbol(x);
  }

  inline
  bool is_cons(const application& x)
  {
    return sort_list::is_cons_application(x) && !is_cons_list(x);
  }

  inline
  bool is_snoc(const application& x)
  {
    return sort_list::is_snoc_application(x) && !is_snoc_list(x);
  }

} // namespace detail

inline
int precedence(const application& x)
{
  // TODO: this is unexpected, what to do???
  if (sort_real::is_creal_application(x))
  {
    return precedence(sort_real::numerator(x));
  }

  else if (detail::is_implies(x))
  {
    return 1;
  }
  else if (   detail::is_and(x)
           || detail::is_or(x)
          )
  {
    return 2;
  }
  else if (detail::is_equal_to(x) ||
           detail::is_not_equal_to(x)
          )
  {
    return 3;
  }
  else if (   detail::is_less(x)
           || detail::is_less_equal(x)
           || detail::is_greater(x)
           || detail::is_greater_equal(x)
           || detail::is_in(x)
          )
  {
    return 4;
  }
  else if (detail::is_cons(x))
  {
    return 5;
  }
  else if (detail::is_snoc(x))
  {
    return 6;
  }
  else if (detail::is_concat(x))
  {
    return 7;
  }
  else if (   detail::is_plus(x)
           || detail::is_minus(x)
           || detail::is_set_union(x)
           || detail::is_set_difference(x)
           || detail::is_bag_join(x)
           || detail::is_bag_difference(x)
          )
  {
    return 8;
  }
  else if (   detail::is_div(x)
           || detail::is_mod(x)
           || detail::is_divmod(x)
           || detail::is_divides(x)
          )
  {
    return 9;
  }
  else if (   detail::is_times(x)
           || detail::is_element_at(x)
           || detail::is_set_intersection(x)
           || detail::is_bag_intersection(x)
          )
  {
    return 10;
  }
  return max_precedence;
}

inline
int infix_precedence_left(const application& x)
{
  // TODO: this is unexpected, what to do???
  if (sort_real::is_creal_application(x))
  {
    return infix_precedence_left(sort_real::numerator(x));
  }

  else if (detail::is_implies(x))
  {
    return 2;
  }
  else if (   detail::is_and(x)
           || detail::is_or(x)
          )
  {
    return 3;
  }
  else if (detail::is_equal_to(x) ||
           detail::is_not_equal_to(x)
          )
  {
    return 4;
  }
  else if (   detail::is_less(x)
           || detail::is_less_equal(x)
           || detail::is_greater(x)
           || detail::is_greater_equal(x)
           || detail::is_in(x)
          )
  {
    return 5;
  }
  else if (detail::is_cons(x))
  {
    return 8;
  }
  else if (detail::is_snoc(x))
  {
    return 6;
  }
  else if (detail::is_concat(x))
  {
    return 7;
  }
  else if (   detail::is_plus(x)
           || detail::is_minus(x)
           || detail::is_set_union(x)
           || detail::is_set_difference(x)
           || detail::is_bag_join(x)
           || detail::is_bag_difference(x)
          )
  {
    return 8;
  }
  else if (   detail::is_div(x)
           || detail::is_mod(x)
           || detail::is_divides(x)
          )
  {
    return 9;
  }
  else if (   detail::is_times(x)
           || detail::is_element_at(x)
           || detail::is_set_intersection(x)
           || detail::is_bag_intersection(x)
          )
  {
    return 10;
  }
  return max_precedence;
}

inline
int infix_precedence_right(const application& x)
{
  // TODO: this is unexpected, what to do???
  if (sort_real::is_creal_application(x))
  {
    return infix_precedence_right(sort_real::numerator(x));
  }

  else if (detail::is_implies(x))
  {
    return 1;
  }
  else if (   detail::is_and(x)
           || detail::is_or(x)
          )
  {
    return 2;
  }
  else if (detail::is_equal_to(x) ||
           detail::is_not_equal_to(x)
          )
  {
    return 3;
  }
  else if (   detail::is_less(x)
           || detail::is_less_equal(x)
           || detail::is_greater(x)
           || detail::is_greater_equal(x)
           || detail::is_in(x)
          )
  {
    return 5;
  }
  else if (detail::is_cons(x))
  {
    return 5;
  }
  else if (detail::is_snoc(x))
  {
    return 8;
  }
  else if (detail::is_concat(x))
  {
    return 8;
  }
  else if (   detail::is_plus(x)
           || detail::is_minus(x)
           || detail::is_set_union(x)
           || detail::is_set_difference(x)
           || detail::is_bag_join(x)
           || detail::is_bag_difference(x)
          )
  {
    return 9;
  }
  else if (   detail::is_div(x)
           || detail::is_mod(x)
           || detail::is_divides(x)
          )
  {
    return 10;
  }
  else if (   detail::is_times(x)
           || detail::is_element_at(x)
           || detail::is_set_intersection(x)
           || detail::is_bag_intersection(x)
          )
  {
    return 11;
  }
  return max_precedence;
}

inline
int precedence(const data_expression& x)
{
  if (is_application(x))
  {
    return precedence(application(x));
  }
  return max_precedence;
}

inline
int infix_precedence_left(const data_expression& x)
{
  if (is_application(x))
  {
    return infix_precedence_left(application(x));
  }
  return max_precedence;
}

inline
int infix_precedence_right(const data_expression& x)
{
  if (is_application(x))
  {
    return infix_precedence_right(application(x));
  }
  return max_precedence;
}

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_PRECEDENCE_H
