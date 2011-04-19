// Author(s): Jeroen van der Wulp, Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/print.h
/// \brief Provides utilities for pretty printing.

#ifndef MCRL2_DATA_PRINT_H
#define MCRL2_DATA_PRINT_H

#include <iostream>
#include <sstream>
#include <list>
#include <iterator>

#include "boost/utility/enable_if.hpp"

#include "mcrl2/atermpp/container_utility.h"
#include "mcrl2/core/print.h"
#include "mcrl2/core/identifier_string.h"
#include "mcrl2/data/detail/precedence.h"
#include "mcrl2/data/standard_utility.h"
#include "mcrl2/data/list.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/find.h"
#include "mcrl2/data/multiple_possible_sorts.h"
#include "mcrl2/data/unknown_sort.h"
#include "mcrl2/data/standard.h"
#include "mcrl2/data/bag.h"
#include "mcrl2/data/bool.h"
#include "mcrl2/data/list.h"
#include "mcrl2/data/set.h"
#include "mcrl2/data/real.h"
#include "mcrl2/data/set_identifier_generator.h"
#include "mcrl2/data/find.h"
#include "mcrl2/data/detail/print_utility.h"
#include "mcrl2/exception.h"

namespace atermpp
{

template<>
struct aterm_traits<std::pair<mcrl2::data::variable, mcrl2::data::variable> >
{
  static void protect(const std::pair<mcrl2::data::variable, mcrl2::data::variable>& t)
  {
    t.first.protect();
    t.second.protect();
  }
  static void unprotect(const std::pair<mcrl2::data::variable, mcrl2::data::variable>& t)
  {
    t.first.unprotect();
    t.second.unprotect();
  }
  static void mark(const std::pair<mcrl2::data::variable, mcrl2::data::variable>& t)
  {
    t.first.mark();
    t.second.mark();
  }
};

namespace detail
{

// This is here to make the std::list container work with the pp container overload.
template < typename T >
struct is_container_impl< std::list< T > >
{
  typedef boost::true_type type;
};

} // namespace detail
} // namespace atermpp

namespace mcrl2
{

namespace data
{

/// \brief Pretty prints the contents of a container
/// \param[in] c a container with data or sort expressions
template <typename Container>
inline std::string pp(Container const& c, typename atermpp::detail::enable_if_container< Container >::type* = 0)
{
  std::string result;

  if (c.begin() != c.end())
  {
    result.append(mcrl2::core::pp(*c.begin()));

    for (typename Container::const_iterator i = ++(c.begin()); i != c.end(); ++i)
    {
      result.append(", ").append(mcrl2::core::pp(*i));
    }
  }

  return result;
}

/// \brief Pretty prints a data and sort expressions
/// \param[in] c A data or sort expression
inline std::string pp(atermpp::aterm_appl const& c)
{
  return core::pp(c);
}

/// \brief Pretty prints a data and sort expressions
/// \param[in] c A data or sort expression.
template < typename Expression >
inline std::string pp(atermpp::term_list< Expression > const& c)
{
  return core::pp(c);
}

namespace detail
{


template <typename Derived>
struct printer: public data::add_traverser_sort_expressions<core::detail::printer, Derived>
{
  typedef data::add_traverser_sort_expressions<core::detail::printer, Derived> super;

  using super::enter;
  using super::leave;
  using super::operator();
  using core::detail::printer<Derived>::print_sorts;

  Derived& derived()
  {
    return static_cast<Derived&>(*this);
  }

  core::identifier_string generate_identifier(const std::string& prefix, const data_expression& context) const
  {
    data::set_identifier_generator generator;
    std::set<variable> variables = data::find_variables(context);
    for (std::set<variable>::iterator i = variables.begin(); i != variables.end(); ++i)
    {
      generator.add_identifier(i->name());
    }
    return generator(prefix);
  }
  
  template <typename Container>
  void print_container(const Container& container,
                       int container_precedence = -1,
                       const std::string& separator = ", ",
                       const std::string& open_bracket = "(",
                       const std::string& close_bracket = ")"
                      )
  {
    for (typename Container::const_iterator i = container.begin(); i != container.end(); ++i)
    {
      if (i != container.begin())
      {
        derived().print(separator);
      }
      bool print_brackets = (container.size() > 1) && (data::detail::precedence(*i) < container_precedence);
      if (print_brackets)
      {
        derived().print(open_bracket);
      }
      derived()(*i);
      if (print_brackets)
      {
        derived().print(close_bracket);
      }
    }
  }

  template <typename Container>
  void print_list(const Container& container,
                  const std::string& opener = "(",
                  const std::string& closer = ")",
                  const std::string& separator = ", "
                 )
  {
    if (container.empty())
    {
      return;
    }
    derived().print(opener);
    for (typename Container::const_iterator i = container.begin(); i != container.end(); ++i)
    {
      if (i != container.begin())
      {
        derived().print(separator);
      }
      derived()(*i);
    }
    derived().print(closer);
  }

  bool is_abstraction_application(const application& x) const
  {
    //std::cout << "\n<abstraction>" << pp(x) << " " << pp(x.head()) << " " << std::boolalpha << is_abstraction(x.head()) << std::endl;
    return is_abstraction(x.head());
  }

  bool is_cons_list(data_expression x) const
  {
    while (sort_list::is_cons_application(x))
    {
      x = sort_list::tail(x);
    }
    return sort_list::is_nil_function_symbol(x);
  }

  bool is_snoc_list(data_expression x) const
  {
    while (sort_list::is_snoc_application(x))
    {
      x = sort_list::rtail(x);
    }
    return sort_list::is_nil_function_symbol(x);
  }

  bool is_fset_cons_list(data_expression x)
  {
    while (sort_fset::is_fset_cons_application(x))
    {
      x = sort_fset::tail(x);
    }
    return sort_fset::is_fset_empty_function_symbol(x);
  }

  bool is_fset_insert_list(data_expression x)
  {
    while (sort_fset::is_fsetinsert_application(x))
    {
      x = sort_fset::right(x);
    }
    return sort_fset::is_fset_empty_function_symbol(x);
  }

  bool is_fbag_cons_list(data_expression x)
  {
    while (sort_fbag::is_fbag_cons_application(x))
    {
      x = sort_fbag::tail(x);
    }
    return sort_fbag::is_fbag_empty_function_symbol(x);
  }

  bool is_fbag_insert_list(data_expression x)
  {
    while (sort_fbag::is_fbaginsert_application(x))
    {
      x = sort_fbag::arg1(x);
    }
    return sort_fbag::is_fbag_empty_function_symbol(x);
  }

  bool is_fbag_cinsert_list(data_expression x)
  {
    while (sort_fbag::is_fbagcinsert_application(x))
    {
      x = sort_fbag::arg1(x);
    }
    return sort_fbag::is_fbag_empty_function_symbol(x);
  }

  bool is_numeric_cast(const data_expression& x)
  {
    return data::sort_nat::is_pos2nat_application(x)
           || data::sort_int::is_pos2int_application(x)
           || data::sort_real::is_pos2real_application(x)
           || data::sort_int::is_nat2int_application(x)
           || data::sort_real::is_nat2real_application(x)
           || data::sort_real::is_int2real_application(x)
           ;
  }

  bool is_numeric_constant(const application& x)
  {
    return data::sort_pos::is_c1_function_symbol(x.head())
           || data::sort_nat::is_c0_function_symbol(x.head())
           || data::sort_pos::is_cdub_application(x)
           || data::sort_nat::is_cnat_application(x)
           ;
  }

  bool is_fset_true(data_expression x)
  {
    return sort_bool::is_true_function_symbol(sort_set::left(x));
  }

  bool is_fset_false(data_expression x)
  {
    return sort_bool::is_false_function_symbol(sort_set::left(x));
  }

  bool is_fset_lambda(data_expression x)
  {
    return is_lambda(sort_set::left(x)) && sort_fset::is_fset_empty_function_symbol(sort_set::right(x));
  }

  bool is_fbag_zero(const data_expression& x)
  {
    return sort_bag::is_zero_function_function_symbol(sort_bag::left(x));
  }

  bool is_fbag_one(const data_expression& x)
  {
    return sort_bag::is_one_function_function_symbol(sort_bag::left(x));
  }

  bool is_fbag_lambda(data_expression x)
  {
    return is_lambda(sort_bag::left(x)) && sort_fbag::is_fbag_empty_function_symbol(sort_bag::right(x));
  }

  void print_cons_list(data_expression x)
  {
    data_expression_vector arguments;
    while (sort_list::is_cons_application(x))
    {
      arguments.push_back(sort_list::head(x));
      x = sort_list::tail(x);
    }
    derived().print("[");
    print_container(arguments, 6);
    derived().print("]");
  }

  void print_snoc_list(data_expression x)
  {
    data_expression_vector arguments;
    while (sort_list::is_snoc_application(x))
    {
      arguments.insert(arguments.begin(), sort_list::rhead(x));
      x = sort_list::rtail(x);
    }
    derived().print("[");
    print_container(arguments, 7);
    derived().print("]");
  }

  void print_fset_cons_list(data_expression x)
  {
    data_expression_vector arguments;
    while (sort_fset::is_fset_cons_application(x))
    {
      arguments.push_back(sort_fset::head(x));
      x = sort_fset::tail(x);
    }
    derived().print("{");
    print_container(arguments, 6);
    derived().print("}");
  }

  void print_fset_insert_list(data_expression x)
  {
    data_expression_vector arguments;
    while (sort_fset::is_fsetinsert_application(x))
    {
      arguments.push_back(sort_fset::left(x));
      x = sort_fset::right(x);
    }
    derived().print("{");
    print_container(arguments, 6);
    derived().print("}");
  }
  
  void print_fbag_zero(const data_expression& x)
  {
//std::cout << "\n<fbag_zero>" << core::pp(x) << " " << x << std::endl;
    // TODO: check if this is the correct way to handle this case
    if (sort_fbag::is_fbag_empty_function_symbol(sort_bag::right(x)))
    {
      derived().print("{}");
    }
    else
    {
      derived().print("{");
      derived()(sort_bag::right(x));
      derived().print("}");
    }
  } 
  
  void print_fbag_one(const data_expression& x)
  {
//std::cout << "\n<fbag_one>" << core::pp(x) << " " << x << std::endl;
    sort_expression s = function_sort(sort_bag::left(x).sort()).domain().front(); // the sort of the bag elements
    core::identifier_string name = generate_identifier("x", x);
    variable var(name, s);
    data_expression body = number(sort_nat::nat(), "1");
    if (!sort_fbag::is_fbag_empty_function_symbol(sort_bag::right(x)))
    {
      body = sort_nat::swap_zero(body, sort_bag::bagcount(s, var, sort_bag::bagfbag(s, sort_bag::right(x))));
    }
    derived().print("{ ");
    print_sorts() = true;
    derived()(var);
    print_sorts() = false;
    derived().print(" | ");
    derived()(body);
    derived().print(" }");
  } 
  
  void print_fbag_lambda(data_expression x)
  {
//std::cout << "\n<fbag_lambda>" << core::pp(x) << " " << x << std::endl;
    sort_expression s = function_sort(sort_bag::left(x).sort()).domain().front(); // the sort of the bag elements
    core::identifier_string name = generate_identifier("x", x);
    variable var(name, s);
    data::lambda left(sort_set::left(x));
    data_expression body = left.body();
    if (!sort_fbag::is_fbag_empty_function_symbol(sort_bag::right(x)))
    {
      body = sort_nat::swap_zero(body, sort_bag::bagcount(s, var, sort_bag::bagfbag(s, sort_bag::right(x))));
    }
    derived().print("{ ");
    print_sorts() = true;
    derived()(left.variables());
    print_sorts() = false;
    derived().print(" | ");
    derived()(body);
    derived().print(" }");
  }   
  
  void print_fbag_default(data_expression x)
  {
//std::cout << "\n<fbag_default>" << core::pp(x) << " " << x << std::endl;
    data_expression right = sort_set::right(x);
    sort_expression s = function_sort(sort_bag::left(x).sort()).domain().front();
    core::identifier_string name = generate_identifier("x", x);
    variable var(name, s);
    data_expression body = sort_bag::left(x)(var);
    if (!sort_fbag::is_fbag_empty_function_symbol(sort_bag::right(x)))
    {
      body = sort_nat::swap_zero(body, sort_bag::bagcount(s, var, sort_bag::bagfbag(s, sort_bag::right(x))));
    }
    derived().print("{ ");
    print_sorts() = true;
    derived()(var);
    print_sorts() = false;
    derived().print(" | ");
    derived()(body);
    derived().print(" }");
  }  

  void print_fbag_cons_list(data_expression x)
  {
    atermpp::vector<std::pair<data_expression, data_expression> > arguments;
    while (sort_fbag::is_fbag_cons_application(x))
    {
      arguments.push_back(std::make_pair(sort_fbag::head(x), sort_fbag::headcount(x)));
      x = sort_fbag::tail(x);
    }
    print_list(arguments, "{", "}");
  }

  void print_fbag_insert_list(data_expression x)
  {
    atermpp::vector<std::pair<data_expression, data_expression> > arguments;
    while (sort_fbag::is_fbaginsert_application(x))
    {
      arguments.push_back(std::make_pair(sort_fbag::arg1(x), sort_nat::cnat(sort_fbag::arg2(x))));
      x = sort_fbag::arg3(x);
    }
    print_list(arguments, "{", "}");
  }

  void print_fbag_cinsert_list(data_expression x)
  {
    atermpp::vector<std::pair<data_expression, data_expression> > arguments;
    while (sort_fbag::is_fbagcinsert_application(x))
    {
      arguments.push_back(std::make_pair(sort_fbag::arg1(x), sort_fbag::arg2(x)));
      x = sort_fbag::arg3(x);
    }
    print_list(arguments, "{", "}");
  }

  void print_fset_true(data_expression x)
  {
    derived().print("!{");
    // TODO: compute the complement of the set
    derived()(sort_set::right(x));
    derived().print("}");
  }
  
  void print_fset_false(data_expression x)
  {
    derived().print("{");
    derived()(sort_set::right(x));
    derived().print("}");   
  }
  
  void print_fset_lambda(data_expression x)
  {
    sort_expression s = function_sort(sort_set::left(x).sort()).domain().front(); // the sort of the set elements
    data::lambda left(sort_set::left(x));
    derived().print("{ ");
    print_sorts() = true;
    derived()(left.variables());
    print_sorts() = false;
    derived().print(" | ");
    derived()(left.body());
    derived().print(" }");
  }
  
  void print_fset_default(data_expression x)
  {
    data_expression right = sort_set::right(x);
    // TODO: check if this is the correct way to handle this case
    if (sort_fset::is_fset_empty_function_symbol(right))
    {
      derived().print("{}");
    }
    else
    {
      sort_expression s = function_sort(sort_set::left(x).sort()).domain().front();
      core::identifier_string name = generate_identifier("x", x);
      variable var(name, s);
      data_expression lhs(sort_set::left(x)(var));
      data_expression rhs(sort_set::setin(s, var, sort_set::setfset(s, right)));
      data_expression body = not_equal_to(lhs, rhs);
      derived().print("{ ");
      print_sorts() = true;
      derived()(var);
      print_sorts() = false;
      derived().print(" | ");
      derived()(body);
      derived().print(" }");
    }
  }  

  void print_abstraction(const abstraction& x, const std::string& op)
  {
    derived().enter(x);
    derived().print(op + " ");
    derived().print_sorts() = true;
    print_container(x.variables());
    derived().print_sorts() = false;
    derived().print(". ");
    derived()(x.body());
    derived().leave(x);
  }

  void print_numeric_constant(data_expression x)
  {
    x = detail::reconstruct_numeric_expression(x);
    derived().print(function_symbol(x).name());
  }

  void print_function_application(const application& x)
  {
    //std::cout << "\n<function application>" << core::pp(x) << " " << x.arguments().size() << "\n";
    bool print_parentheses = is_abstraction(x.head());
    if (print_parentheses)
    {
      derived().print("(");
    }
    derived()(x.head());
    if (print_parentheses)
    {
      derived().print(")");
    }
    derived().print("(");
    print_container(x.arguments());
    derived().print(")");
  }

  // N.B. This is interpreted as the bag element 'x.first: x.second'
  void operator()(const std::pair<data_expression, data_expression>& x)
  {
    derived()(x.first);
    derived().print(": ");
    derived()(x.second);
  }

  void operator()(const data::container_type& x)
  {
    // skip
  }

  void operator()(const data::assignment& x)
  {
    derived().enter(x);
    derived()(x.lhs());
    derived().print(":=");
    derived()(x.rhs());
    derived().leave(x);
  }

  void operator()(const data::identifier_assignment& x)
  {
    derived().enter(x);
    derived()(x.lhs());
    derived().print(":=");
    derived()(x.rhs());
    derived().leave(x);
  }

  void operator()(const data::set_or_bag_comprehension_binder& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::set_comprehension_binder& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::bag_comprehension_binder& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::forall_binder& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::exists_binder& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::lambda_binder& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::structured_sort_constructor_argument& x)
  {
    derived().enter(x);
    derived()(x.name());
    derived()(x.sort());
    derived().leave(x);
  }

  void operator()(const data::structured_sort_constructor& x)
  {
    derived().enter(x);
    derived()(x.name());
    derived()(x.arguments());
    derived()(x.recogniser());
    derived().leave(x);
  }

  void operator()(const data::list_container& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::set_container& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::bag_container& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::fset_container& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::fbag_container& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::basic_sort& x)
  {
    derived().enter(x);
    derived()(x.name());
    derived().leave(x);
  }

  void operator()(const data::container_sort& x)
  {
    derived().enter(x);
    derived()(x.container_name());
    derived()(x.element_sort());
    derived().leave(x);
  }

  void operator()(const data::structured_sort& x)
  {
    derived().enter(x);
    derived()(x.constructors());
    derived().leave(x);
  }

  void operator()(const data::function_sort& x)
  {
    derived().enter(x);
    derived()(x.domain());
    derived().print(" -> ");
    derived()(x.codomain());
    derived().leave(x);
  }

  void operator()(const data::unknown_sort& x)
  {
    derived().enter(x);
    derived().leave(x);
  }

  void operator()(const data::multiple_possible_sorts& x)
  {
    derived().enter(x);
    derived()(x.sorts());
    derived().leave(x);
  }

  void operator()(const data::identifier& x)
  {
    derived().enter(x);
    derived()(x.name());
    derived().leave(x);
  }

  void operator()(const data::variable& x)
  {
    derived().enter(x);
    derived()(x.name());
    if (derived().print_sorts())
    {
      derived().print(": ");
      derived()(x.sort());
    }
    derived().leave(x);
  }

  void operator()(const data::function_symbol& x)
  {
    derived().enter(x);
    if (sort_nat::is_c0_function_symbol(x))
    {
      derived().print("0");
    }
    else if (sort_pos::is_c1_function_symbol(x))
    {
      derived().print("1");
    }
    else if (sort_fbag::is_fbag_empty_function_symbol(x))
    {
      derived().print("{}");
    }
    else
    {
      derived().print(x.name());
    }    
//    derived()(x.sort());
    derived().leave(x);
  }

  void operator()(const data::application& x)
  {
    derived().enter(x);

    //-------------------------------------------------------------------//
    //                            numeric values
    //-------------------------------------------------------------------//
    // TODO: can these be moved to int/pos/nat/real?
    if (is_numeric_cast(x))
    {
      // ignore numeric casts like Pos2Nat
      derived()(x.arguments().front());
    }
    else if (is_numeric_constant(x))
    {
      print_numeric_constant(x);
    }

    //-------------------------------------------------------------------//
    //                            bool
    //-------------------------------------------------------------------//
    else if (sort_bool::is_bool(x.sort()))
    {
      if (data::is_equal_to_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " == ");
      }
      else if (data::is_not_equal_to_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " != ");
      }     
      else if (data::is_less_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " < ");
      }
      else if (data::is_less_equal_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " <= ");
      }
      else if (data::is_greater_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " > ");
      }
      else if (data::is_greater_equal_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " >= ");
      }
      else if (sort_bool::is_implies_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " => ");
      }
      else if (sort_bool::is_and_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " && ");
      }
      else if (sort_bool::is_or_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " || ");
      }
      else if (sort_list::is_in_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " in ");
      }
      else if (sort_list::is_element_at_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " . ");
      }
      else
      {
        print_function_application(x);
      }
    }

    //-------------------------------------------------------------------//
    //                            pos
    //-------------------------------------------------------------------//
    else if (sort_pos::is_pos(x.sort()))
    {   
      if (sort_pos::is_plus_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " + ");
      }
      {
        print_function_application(x);
      }
    }

    //-------------------------------------------------------------------//
    //                            nat
    //-------------------------------------------------------------------//
    else if (sort_nat::is_nat(x.sort()))
    {   
      if (sort_nat::is_plus_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " + ");
      }
      else if (sort_nat::is_times_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " * ");
      }
      else if (sort_nat::is_div_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " / ");
      }     
      else if (sort_nat::is_mod_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " mod ");
      }
      else if (sort_nat::is_exp_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " ^ ");
      }
//      else if (sort_list::is_count_application(x))
//      {
//        derived().print("#");
//        derived()(x.arguments().front());
//      }
//      else if (sort_bag::is_bagcount_application(x))
//      {
//        derived().print("#");
//        derived()(x.arguments().front());
//      }
//      else if (sort_fbag::is_fbagcount_application(x))
//      {
//        derived().print("#");
//        derived()(x.arguments().front());
//      }
      else
      {
        print_function_application(x);
      }
    }

    //-------------------------------------------------------------------//
    //                            int
    //-------------------------------------------------------------------//
    else if (sort_int::is_int(x.sort()))
    {   
      if (sort_int::is_negate_application(x))
      {
        derived().print("-");
        derived()(x.arguments().front());
      }
      else if (sort_int::is_plus_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " + ");
      }
      else if (sort_int::is_minus_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " - ");
      }
      else if (sort_int::is_times_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " * ");
      }
      else if (sort_int::is_div_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " / ");
      }
      else
      {
        print_function_application(x);
      }
    }

    //-------------------------------------------------------------------//
    //                            real
    //-------------------------------------------------------------------//
    else if (sort_real::is_real(x.sort()))
    {
      if (sort_real::is_negate_application(x))
      {
        derived().print("-");
        derived()(x.arguments().front());
      }
      else if (sort_real::is_plus_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " + ");
      }
      else if (sort_real::is_minus_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " - ");
      }
      else if (sort_real::is_times_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " * ");
      }
      else if (sort_real::is_divides_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " / ");
      }
      else
      {
        print_function_application(x);
      }
    }

    //-------------------------------------------------------------------//
    //                            list
    //-------------------------------------------------------------------//
    else if (sort_list::is_list(x.sort()))
    {
      if (sort_list::is_concat_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " ++ ");
      }
      else if (sort_list::is_cons_application(x))
      {
        if (is_cons_list(x))
        {
          print_cons_list(x);
        }
        else
        {
          print_container(x.arguments(), data::detail::precedence(x), " |> ");
        }
      }
      else if (sort_list::is_snoc_application(x))
      {
        if (is_snoc_list(x))
        {
          print_snoc_list(x);
        }
        else
        {
          print_container(x.arguments(), data::detail::precedence(x), " <| ");
        }
      }
      else
      {
        // TODO: handle ListEnum
        print_function_application(x);
      }
    }

    //-------------------------------------------------------------------//
    //                            set
    //-------------------------------------------------------------------//
    else if (sort_set::is_set(x.sort()))
    {
      if (sort_set::is_setcomplement_application(x))
      {
        derived().print("!");
        derived()(x.arguments().front());
      }
      else if (sort_set::is_setunion_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " + ");
      }
      else if (sort_set::is_setintersection_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " * ");
      }
      else if (sort_set::is_setdifference_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " - ");
      }
      else if (sort_set::is_setconstructor_application(x))
      {
        if (is_fset_true(x))
        {
          print_fset_true(x);
        }
        else if (is_fset_false(x))
        {
          print_fset_false(x);
        }
        else if (is_fset_lambda(x))
        {
          print_fset_lambda(x);
        }
        else
        {
          print_fset_default(x);
        }
      }
      else if (sort_set::is_setcomprehension_application(x))
      {
        sort_expression s = function_sort(sort_set::arg(x).sort()).domain().front();
        core::identifier_string name = generate_identifier("x", x);
        variable var(name, s);
        data_expression body(sort_set::arg(x)(var));
        derived().print("{ ");
        derived()(var);
        derived().print(" | ");
        derived()(body);
        derived().print(" }");   
      }
      else if (sort_set::is_setfset_application(x))
      {
        //std::cout << "\n<setfset>" << core::pp(x) << " " << x << std::endl;
        data_expression y = sort_set::arg(x);
        if (sort_fset::is_fset_empty_function_symbol(y))
        {
          derived().print("{}");
        }
        else
        {
          derived()(y);
        }
      }
      else
      {
        print_function_application(x);
      }
    }

    //-------------------------------------------------------------------//
    //                            fset
    //-------------------------------------------------------------------//
    else if (sort_fset::is_fset(x.sort()))
    {
      if (sort_fset::is_fset_cons_application(x))
      {
        if (is_fset_cons_list(x))
        {
          print_fset_cons_list(x);
        }
        else
        {
          // TODO: can this case occur?
          throw mcrl2::runtime_error("unexpected case in fset " + x.to_string());
        }
      }
      else if (sort_fset::is_fsetinsert_application(x))
      {
        if (is_fset_insert_list(x))
        {
          print_fset_insert_list(x);
        }
        else
        {
          // TODO: can this case occur?
          throw mcrl2::runtime_error("unexpected case in fset " + x.to_string());
        }
      }
      else
      {
        print_function_application(x);
      }
    }

    //-------------------------------------------------------------------//
    //                            bag
    //-------------------------------------------------------------------//
    else if (sort_bag::is_bag(x.sort()))
    {   
      if (sort_bag::is_bagjoin_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " + ");
      }
      else if (sort_bag::is_bagintersect_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " * ");
      }
      else if (sort_bag::is_bagdifference_application(x))
      {
        print_container(x.arguments(), data::detail::precedence(x), " - ");
      }

      else if (sort_bag::is_bagconstructor_application(x))
      {
        if (is_fbag_zero(x))
        {
          print_fbag_zero(x);
        }
        else if (is_fbag_one(x))
        {
          print_fbag_one(x);
        }
        else if (is_fbag_lambda(x))
        {
          print_fbag_lambda(x);
        }
        else
        {
          print_fbag_default(x);
        }
      }
      else if (sort_bag::is_bagcomprehension_application(x))
      {
        sort_expression s = function_sort(sort_bag::arg(x).sort()).domain().front();
        core::identifier_string name = generate_identifier("x", x);
        variable var(name, s);
        data_expression body(sort_bag::arg(x)(var));
        derived().print("{ ");
        derived()(var);
        derived().print(" | ");
        derived()(body);
        derived().print(" }");   
      }
      else if (sort_bag::is_bagfbag_application(x))
      {
        //std::cout << "\n<bagfbag>" << core::pp(x) << " " << x << std::endl;
        data_expression y = sort_bag::arg(x);
        if (sort_fbag::is_fbag_empty_function_symbol(y))
        {
          derived().print("{}");
        }
        else
        {
          derived()(y);
        }
      }
      else
      {
        print_function_application(x);
      }
    }

    //-------------------------------------------------------------------//
    //                            fbag
    //-------------------------------------------------------------------//
    else if (sort_fbag::is_fbag(x.sort()))
    {
      if (sort_fbag::is_fbag_cons_application(x))
      {
        if (is_fbag_cons_list(x))
        {
          print_fbag_cons_list(x);
        }
        else
        {
          // TODO: can this case occur?
          throw mcrl2::runtime_error("unexpected case in fbag " + x.to_string());
        }
      }
      else if (sort_fbag::is_fbaginsert_application(x))
      {
        if (is_fbag_insert_list(x))
        {
          print_fbag_insert_list(x);
        }
        else
        {
          // TODO: can this case occur?
          throw mcrl2::runtime_error("unexpected case in fbag " + x.to_string());
        }
      }
      else if (sort_fbag::is_fbagcinsert_application(x))
      {
        if (is_fbag_cinsert_list(x))
        {
          print_fbag_cinsert_list(x);
        }
        else
        {
          // TODO: can this case occur?
          throw mcrl2::runtime_error("unexpected case in fbag " + x.to_string());
        }
      }
      else
      {
        print_function_application(x);
      }
    }

    //-------------------------------------------------------------------//
    //                            function update
    //-------------------------------------------------------------------//
    else if (is_function_update_application(x))
    {
      //std::cout << "\n<function_update>" << pp(x) << " " << x << std::endl;
      data_expression x1 = data::arg1(x);
      data_expression x2 = data::arg2(x);
      data_expression x3 = data::arg3(x);
      bool print_parentheses = is_abstraction(x1);
      if (print_parentheses)
      {
        derived().print("(");
      }
      derived()(x1);
      if (print_parentheses)
      {
        derived().print(")");
      }
      derived().print("[");
      derived()(x2);
      derived().print(" -> ");
      derived()(x3);
      derived().print("]");
    }

    //-------------------------------------------------------------------//
    //                            abstraction
    //-------------------------------------------------------------------//
    else if (is_abstraction_application(x))
    {
      if (x.arguments().size() > 0) {
        derived().print("(");         
      }
      derived()(x.head());
      if (x.arguments().size() > 0)
      {
        derived().print(")(");
      }
      print_container(x.arguments());
      if (x.arguments().size() > 0)
      {
        derived().print(")");
      }
    }
    
    //-------------------------------------------------------------------//
    //                            function application
    //-------------------------------------------------------------------//
    else
    {
      std::cout << "\n<error: unknown application>" << core::pp(x) << "\n";
      print_function_application(x);
    }
    derived().leave(x);
  }

  void operator()(const data::where_clause& x)
  {
    derived().enter(x);
    derived()(x.body());
    derived()(x.declarations());
    derived().leave(x);
  }

  void operator()(const data::forall& x)
  {
    print_abstraction(x, "forall");
  }

  void operator()(const data::exists& x)
  {
    print_abstraction(x, "exists");
  }

  void operator()(const data::lambda& x)
  {
    print_abstraction(x, "lambda");
  }

  void operator()(const data::data_equation& x)
  {
    derived().enter(x);
    derived()(x.variables());
    derived()(x.condition());
    derived()(x.lhs());
    derived()(x.rhs());
    derived().leave(x);
  }

/*
  template <typename Container>
  void print_declarations(const Container& container,
                          const std::string& separator = ", ",
                          const std::string& opener = "(",
                          const std::string& closer = ")"
                         )
  {
    if (container.empty())
    {
      return;
    }
    derived().print(opener);
    sort_expression last_sort;
    for (typename Container::const_iterator i = container.begin(); i != container.end(); ++i)
    {
      if (i != container.begin())
      {
        derived().print(separator);
      }
      derived()(*i);
      if (i->sort() != last_sort)
      {
        derived().print(": ");
        derived()(i->sort());
      }
    }
    derived().print(closer);
  }

  template <typename Container>
  void print_equations(const Container& container,
                       const std::string& separator = ", ",
                       const std::string& opener = "(",
                       const std::string& closer = ")"
                      )
  {
    if (container.empty())
    {
      return;
    }
    derived().print(opener);
    sort_expression last_sort;
    for (typename Container::const_iterator i = container.begin(); i != container.end(); ++i)
    {
      if (i != container.begin())
      {
        derived().print(separator);
      }
      derived()(*i);
      if (i->sort() != last_sort)
      {
        derived().print(": ");
        derived()(i->sort());
      }
    }
    derived().print(closer);
  }

  template <typename Container>
  void print_equations(const Container& container,
                       const std::string& opener = "(",
                       const std::string& closer = ")",
                       const std::string& separator = ", "
                      )
  {
    if (container.empty())
    {
      return;
    }
    derived().print(opener);
    for (typename Container::const_iterator i = container.begin(); i != container.end(); ++i)
    {
      if (i != container.begin())
      {
        derived().print(separator);
      }
      derived()(*i);
    }
    derived().print(closer);
  }

  void print_variables(const std::map<core::identifier_string> variable>& variable_map,
                       const std::string& opener = "(",
                       const std::string& closer = ")",
                       const std::string& separator = ", "
                      )
  {
    if (container.empty())
    {
      return;
    }
    derived().print(opener);
    std::map<std::string, std::std<std::string> > variable_strings;
    for (std::map<core::identifier_string> variable>::const_iterator i = variable_map.begin(); i != variable_map.end(); ++i)
    {
      std::string sort_name = derived()(i->second.sort());
      variable_strings[].insert(i->first);
    }
    for (std::map<std::string, std::std<std::string> >::iterator i = variable_strings.begin(); i != variable_strings.end(); ++i)
    {
      derived().print(core::string_join(i->second, ", "));
      derived().print(": ");
    }
    for (typename Container::const_iterator i = container.begin(); i != container.end(); ++i)
    {
      if (i != container.begin())
      {
        derived().print(separator);
      }
      derived()(*i);
    }
    derived().print(closer);
  }

  /// \brief Searches in the range of equations [first, last) for the first equation
  /// that conflicts with one of the previous equations. We say that equation e1 conflicts
  /// with another equation e2 if their declared variables contain a variable with the same
  /// name and a different sort, or if a declared variable in e1 has the same name as a
  /// function symbol appearing in equation e2.
  template <typename Iter>
  Iter find_conflicting_equation(Iter first, Iter last, std::map<core::identifier_string, variable>& variable_map, std::set<core::identifier>& function_symbol_names)
  {
    std::set<data::function_symbol> f = data::find_function_symbols(*first);
    for (std::set<data::function_symbol>::iterator i = f.begin(); i != f.end(); ++i)
    {
      function_symbol_names.insert(i->name());
    }
    const variable_list& v = first->variables();
    for (variable_list::const_iterator i = v.begin(); i != v.end(); ++i)
    {
      variable_map[i->name()] = *i;
    }

    for (Iter i = ++first; i != last; ++i)
    {
      const variable_list& v = i->variables();
      for (variable_list::const_iterator j = v.begin(); j != v.end(); ++j)
      {
        std::map<core::identifier_string> variable>::const_iterator k = variable_map.find(j->name());
        if (k != variable_map.end() && *j != k->second)
        {
          return i;
        }
      }
      std::set<data::function_symbol> f = data::find_function_symbols(*i);
      for (std::set<data::function_symbol>::iterator j = f.begin(); j != f.end(); ++j)
      {
        function_symbol_names.insert(j->name());
      }
      for (variable_list::const_iterator j = v.begin(); j != v.end(); ++j)
      {
        variable_map[j->name()] = *j;
      }
    }
    return last; // no conflict found
  }

  void operator()(const data::data_specification& x)
  {
    derived().enter(x);
    print_list(x.sorts(), "sort ", "\n", ";\n    ");
    print_declarations(x.constructors(), "cons ", ";\n", ";\n     ");
    print_declarations(x.mappings(), "map ", ";\n", ";\n     ");
    atermpp::vector<data_equation>::const_iterator first = x.equations().begin();
    atermpp::vector<data_equation>::const_iterator last = x.equations().end();
    atermpp::vector<data_equation>::const_iterator i = first;
    while (i != last)
    {
      std::map<core::identifier_string> variable> variable_map;
      std::set<core::identifier> function_symbol_names;
      i = find_conflicting_equation(i, last, variable_map, function_symbol_names);
      print_variables(variable_map);
      print_equations(x.equations(), "eqn ", ";\n", ";\n     ");
    }
    derived().leave(x);
  }
*/

};

} // namespace detail

/// \brief Prints the object t to a stream.
template <typename T>
void print(const T& t, std::ostream& out)
{
  core::detail::apply_printer<data::detail::printer> printer(out);
  printer(t);
}

/// \brief Returns a string representation of the object t.
template <typename T>
std::string print(const T& t)
{
  std::ostringstream out;
  data::print(t, out);
  return out.str();
}

} // namespace data

} // namespace mcrl2

#endif

