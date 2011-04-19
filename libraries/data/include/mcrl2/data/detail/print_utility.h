// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/detail/print_utility.h
/// \brief add your file description here.

#ifndef MCRL2_DATA_DETAIL_PRINT_UTILITY_H
#define MCRL2_DATA_DETAIL_PRINT_UTILITY_H

#ifdef MCRL2_USE_BOOST_EXPRESSIVE
#include <boost/xpressive/xpressive.hpp>
#endif
#include "mcrl2/atermpp/aterm_access.h"
#include "mcrl2/core/text_utility.h"
#include "mcrl2/data/bool.h"
#include "mcrl2/data/pos.h"
#include "mcrl2/data/nat.h"
#include "mcrl2/data/int.h"
#include "mcrl2/data/real.h"
#include "mcrl2/data/list.h"
#include "mcrl2/data/set.h"
#include "mcrl2/data/bag.h"
#include "mcrl2/data/standard.h"
#include "mcrl2/data/standard_utility.h"
#include "mcrl2/data/function_update.h"
#include "mcrl2/data/lambda.h"

namespace mcrl2 {

namespace data {

namespace detail {

/// \pre BoolExpr is a boolean expression, SortExpr is of type Pos, Nat, Int or
//     Real.
/// \return if(BoolExpr, 1, 0) of sort SortExpr
inline
data::data_expression bool_to_numeric(data::data_expression const& e, data::sort_expression const& s)
{
  // TODO Maybe enforce that SortExpr is a PNIR sort
  return data::if_(e, data::function_symbol("1", s), data::function_symbol("0", s));
}

inline
data_expression reconstruct_pos_mult(const data_expression& x, std::vector<char>& result)
{ 
  if (data::sort_pos::is_c1_function_symbol(x))
  {
    //x is 1; return result
    return data::function_symbol(data::detail::vector_number_to_string(result), data::sort_pos::pos());
  }
  else if (data::sort_pos::is_cdub_application(x))
  {
    //x is of the form cDub(b,p); return (result*2)*v(p) + result*v(b)
    data_expression bool_arg = sort_pos::bit(x);
    data_expression pos_arg = sort_pos::number(x);
    data::detail::decimal_number_multiply_by_two(result);
    pos_arg = reconstruct_pos_mult(pos_arg, result);
    if (data::sort_bool::is_false_function_symbol(bool_arg))
    {
      //result*v(b) = 0
      return pos_arg;
    }
    else if (data::sort_bool::is_true_function_symbol(bool_arg))
    {
      //result*v(b) = result
      return data::sort_real::plus(pos_arg,
                                   data::function_symbol(data::detail::vector_number_to_string(result), data::sort_pos::pos()));
    }
    else if (data::detail::vector_number_to_string(result) == "1")
    {
      //result*v(b) = v(b)
      return data::sort_real::plus(pos_arg, bool_to_numeric(bool_arg, data::sort_nat::nat()));
    }
    else
    {
      //result*v(b)
      return data::sort_real::plus(pos_arg,
                                   data::sort_real::times(data::function_symbol(data::detail::vector_number_to_string(result), data::sort_nat::nat()),
                                   bool_to_numeric(bool_arg, data::sort_nat::nat())));
    }
  }
  else
  {
    //x is not a Pos constructor
    if (data::detail::vector_number_to_string(result) == "1")
    {
      return x;
    }
    else
    {
      return data::sort_real::times(data::function_symbol(data::detail::vector_number_to_string(result), data::sort_pos::pos()), x);
    }
  }
}

inline
data::data_expression reconstruct_numeric_expression(data::data_expression x)
{
  if (data::sort_pos::is_c1_function_symbol(x) || data::sort_pos::is_cdub_application(x))
  {
    if (data::sort_pos::is_positive_constant(x))
    {
      std::string positive_value(data::sort_pos::positive_constant_as_string(x));
      x = data::function_symbol(positive_value, data::sort_pos::pos());
    }
    else
    {
      std::vector<char> number = data::detail::string_to_vector_number("1");
      x = reconstruct_pos_mult(x, number);
    }
  }
  else if (data::sort_nat::is_c0_function_symbol(x))
  {
    x = data::function_symbol("0", data::sort_nat::nat());
  }
  else if ((sort_nat::is_cnat_application(x) || sort_nat::is_pos2nat_application(x)) && sort_pos::is_pos(sort_nat::arg(x).sort()))
  {
    data_expression value = reconstruct_numeric_expression(sort_nat::arg(x));
    x = data::sort_nat::pos2nat(value);
    if (is_function_symbol(value))
    {
      core::identifier_string name = function_symbol(value).name();
      if (core::is_numeric_string(name))
      {
        x = data::function_symbol(name, data::sort_nat::nat());
      }
    }
  }
  else if (data::sort_nat::is_cpair_application(x))
  {
  }
  else if (data::sort_int::is_cneg_application(x))
  {
    x = data::sort_int::negate(sort_int::arg(x));
  }
  else if ( (data::sort_int::is_cint_application(x) || data::sort_int::is_nat2int_application(x))
            && data::sort_nat::is_nat(sort_int::arg(x).sort())
          )
  {
    data_expression value = sort_int::arg(x);
    value = reconstruct_numeric_expression(value);
    x = data::sort_int::nat2int(data::data_expression(value));
    if (is_function_symbol(value))
    {
      core::identifier_string name = function_symbol(value).name();
      if (core::is_numeric_string(name))
      {
        x = data::function_symbol(name, data::sort_int::int_());
      }
    }
  }
  else if (data::sort_real::is_int2real_application(x) && data::sort_int::is_int(sort_real::arg(x).sort()))
  {
    data_expression value = reconstruct_numeric_expression(sort_real::arg(x));
    x = data::sort_real::int2real(value);
    if (is_function_symbol(value))
    {
      core::identifier_string name = atermpp::arg1(value);
      if (core::is_numeric_string(name))
      {
        x = data::function_symbol(name, data::sort_real::real_());
      }
    }
  }
  else if (data::sort_real::is_creal_application(x))
  {
    data_expression numerator = reconstruct_numeric_expression(sort_real::numerator(x));
    data_expression denominator = reconstruct_numeric_expression(sort_real::denominator(x));
    if (denominator == data::function_symbol("1", data::sort_pos::pos()))
    {
      x = data::sort_real::int2real(data::data_expression(numerator));
      if (is_function_symbol(numerator))
      {
        core::identifier_string name = function_symbol(numerator).name();
        if (core::is_numeric_string(name))
        {
          x = data::function_symbol(name, data::sort_real::real_());
        }
      }
    }
    else
    {
      x = data::sort_real::divides(data::data_expression(numerator),
                                   data::sort_int::pos2int(data::data_expression(denominator)));
      if (is_function_symbol(denominator))
      {
        core::identifier_string name = function_symbol(denominator).name();
        if (core::is_numeric_string(name))
        {
          x = data::sort_real::divides(data::data_expression(numerator),
                                       data::function_symbol(name, data::sort_int::int_())
                                      );
        }
      }
    }
  }
  else if (data::sort_real::is_redfracwhr_application(x))
  {
    x = data::sort_real::plus(sort_real::int2real(sort_real::arg2(x)),
                              sort_real::divides(sort_real::arg3(x),
                              sort_real::arg1(x))
                             );
  }
  return x;
}

} // namespace detail

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_DETAIL_PRINT_UTILITY_H
