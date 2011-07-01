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
#include "mcrl2/data/precedence.h"
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
#include "mcrl2/data/normalize_sorts.h"
#include "mcrl2/data/detail/print_utility.h"
#include "mcrl2/data/detail/data_expression_with_variables.h"
#include "mcrl2/exception.h"

namespace atermpp
{

namespace detail
{

// This is here to make the std::list container work with the pp container overload.
template < typename T >
struct is_container_impl<std::list<T> >
{
  typedef boost::true_type type;
};

} // namespace detail
} // namespace atermpp

namespace mcrl2
{

namespace data
{

namespace detail
{

// Needed for argument dependent lookup (?)
using namespace core::detail::precedences;

inline
void check_pp(const std::string& s1, const std::string& s2, const std::string& s3)
{
  if (s1 != s2)
  {
    std::clog << "<pp>   " << s1 << std::endl;
    std::clog << "<print>" << s2 << std::endl;
    std::clog << "<aterm>" << s3 << std::endl;
    throw std::runtime_error("not equal");
  }
}

} // namespace detail

#ifdef MCRL2_ENABLE_CHECK_PP
#define MCRL2_CHECK_PP(s1, s2, s3) data::detail::check_pp(s1, s2, s3);
#else
#define MCRL2_CHECK_PP(s1, s2, s3)
#endif

// forward declaration
template <typename T> std::string print(const T& t);

/// \brief Pretty prints a term
template <typename T>
std::string pp(const T& t, typename atermpp::detail::disable_if_container<T>::type* = 0)
{
  MCRL2_CHECK_PP(core::pp(t), data::print(t), t.to_string());
  return core::pp(t);
}

/// \brief Pretty prints the contents of a container
/// \param[in] c a container with data or sort expressions
template <typename Container>
std::string pp(Container const& c, typename atermpp::detail::enable_if_container<Container>::type* = 0)
{
  std::string result;

  if (c.begin() != c.end())
  {
    result.append(data::pp(*c.begin()));

    for (typename Container::const_iterator i = ++(c.begin()); i != c.end(); ++i)
    {
      result.append(", ").append(data::pp(*i));
    }
  }

  MCRL2_CHECK_PP(result, data::print(c), "unknown");
  return result;
}

inline
std::string pp(const data_specification& x)
{
  return core::pp(data::detail::data_specification_to_aterm_data_spec(x));
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
  using core::detail::printer<Derived>::print_list;

  Derived& derived()
  {
    return static_cast<Derived&>(*this);
  }

  bool is_infix_operation(const application& x)
  {
    if (x.arguments().size() != 2)
    {
      return false;
    }
    core::identifier_string name = function_symbol(x.head()).name();
    return
      (name == data::sort_bool::implies_name())          ||
      (name == data::sort_bool::and_name())          ||
      (name == data::sort_bool::or_name())           ||
      (name == data::detail::equal_symbol())           ||
      (name == data::detail::not_equal_symbol())          ||
      (name == data::detail::less_symbol())           ||
      (name == data::detail::less_equal_symbol())          ||
      (name == data::detail::greater_symbol())           ||
      (name == data::detail::greater_equal_symbol())          ||
      (name == data::sort_list::in_name())        ||
      (name == data::sort_list::cons_name())         ||
      (name == data::sort_list::snoc_name())         ||
      (name == data::sort_list::concat_name())       ||
      (name == data::sort_real::plus_name())          ||
      (name == data::sort_real::minus_name())         ||
      (name == data::sort_set::setunion_name())     ||
      (name == data::sort_set::setdifference_name())      ||
      (name == data::sort_bag::bagjoin_name())      ||
      (name == data::sort_bag::bagdifference_name())      ||
      (name == data::sort_int::div_name())          ||
      (name == data::sort_int::mod_name())          ||
      (name == data::sort_real::divides_name())       ||
      (name == data::sort_int::times_name())         ||
      (name == data::sort_list::element_at_name())        ||
      (name == data::sort_set::setintersection_name()) ||
      (name == data::sort_bag::bagintersect_name());
  }

  void print_sort(const application& x)
  {
    std::cout << "<value>" << core::pp(x) << " " << x << " ";
    if (is_numeric_value(x))
    {
      std::cout << "<numeric value>";
    }
    else if (sort_bool::is_bool(x.sort()))
    {
      std::cout << "<bool>";
    }
    else if (sort_pos::is_pos(x.sort()))
    {
      std::cout << "<pos>";
    }
    else if (sort_nat::is_nat(x.sort()))
    {   
      std::cout << "<nat>";
    }
    else if (sort_int::is_int(x.sort()))
    {   
      std::cout << "<int>";
    }
    else if (sort_real::is_real(x.sort()))
    {
      std::cout << "<real>";
    }
    else if (sort_list::is_list(x.sort()))
    {
      std::cout << "<list>";
    }
    else if (sort_set::is_set(x.sort()))
    {
      std::cout << "<set>";
    }
    else if (sort_fset::is_fset(x.sort()))
    {
      std::cout << "<fset>";
    }
    else if (sort_bag::is_bag(x.sort()))
    {   
      std::cout << "<bag>";
    }
    else if (sort_fbag::is_fbag(x.sort()))
    {
      std::cout << "<fbag>";
    }
    else if (is_function_update_application(x))
    {
      std::cout << "<function_update>";
    }
    else if (is_abstraction_application(x))
    {
      std::cout << "<abstraction>";
    }
    else // function application
    {
      std::cout << "<other>";
    }
    std::cout << std::endl;
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
      bool print_brackets = (container.size() > 1) && (precedence(*i) < container_precedence);
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

  template <typename T>
  void print_expression(const T& x, int prec = 5)
  {
    bool print_parens = (precedence(x) < prec);
    if (print_parens)
    {
      derived().print("(");
    }
    derived()(x);
    if (print_parens)
    {
      derived().print(")");
    }
  }

  template <typename T>
  void print_binary_operation(const T& x, const std::string& op)
  {
    print_expression(x.left(),
                     precedence(x)
                    );
    derived().print(op);
    print_expression(x.right(), precedence(x));
  }

  // implements special handling for expressions like 'x && y || z', in
  // which different operators have the same precedence
  template <typename T>
  void print_binary_operation_special(const T& x, const std::string& op)
  {
    data_expression left = x.left();
    data_expression right = x.right();
    int prec = precedence(x);
    print_expression(left, (prec == precedence(left) && x.head() != application(left).head()) ? prec + 1 : prec);
    derived().print(op);
    print_expression(right, (prec == precedence(right) && x.head() != application(right).head()) ? prec + 1 : prec);
  }

  template <typename Variable>
  void print_variable(const Variable& x, bool print_sort = false)
  {
    derived()(x);
    if (print_sort)
    {
      derived().print(": ");
      derived()(x.sort());
    }
  }
  
  template <typename Container>
  void print_variables(const Container& container,
                       bool print_sorts = true,
                       bool join_sorts = true,
                       const std::string& opener = "(",
                       const std::string& closer = ")",
                       const std::string& separator = ", "
                      )
  {
    typedef typename Container::const_iterator iterator;

    iterator first = container.begin();
    iterator last = container.end();
    if (first == last)
    {
      return;
    }

    derived().print(opener);
    
    while (first != last)
    {
      // print the elements of the interval [first, i)
      if (first != container.begin())
      {
        derived().print(separator);
      }

      if (print_sorts && join_sorts)
      {
        // determine a consecutive interval [first, i) with elements of the same sorts
        iterator i = first;
        do
        {
          ++i;
        }
        while (i != last && i->sort() == first->sort());

        for (iterator j = first; j != i; ++j)
        {
          if (j != first)
          {
            derived().print(",");
          }
          derived()(*j);
        }

        // print the sort
        if (print_sorts)
        {
          derived().print(": ");
          derived()(first->sort());
        }

        // update first
        first = i;
      }
      else
      {
        derived()(*first);

        // print the sort
        if (print_sorts)
        {
          derived().print(": ");
          derived()(first->sort());
        }
        
        // update first
        ++first;
      }
    }
    derived().print(closer);
  }

  template <typename Container>
  void print_assignments(const Container& container,
                         bool print_lhs = true,
                         const std::string& opener = "",
                         const std::string& closer = "",
                         const std::string& separator = ", ",
                         const std::string& assignment_symbol = " = "
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
      if (print_lhs)
      {
        derived()(i->lhs());
        derived().print(assignment_symbol);
      }
      derived()(i->rhs());
    }
    derived().print(closer);
  }

  template <typename T>
  void print_condition(const T& condition, const std::string& arrow = "  ->  ", int precedence = 3)
  {
    if (!sort_bool::is_true_function_symbol(condition))
    {
      print_expression(condition, precedence);
      derived().print(arrow);
    }
  }

  void print_time(const data_expression& t)
  {
    print_expression(t, 13);
  }

  template <typename Container>
  void print_sort_list(const Container& container,
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
      bool print_brackets = is_function_sort(*i);
      if (print_brackets)
      {
        derived().print("(");
      }
      derived()(*i);
      if (print_brackets)
      {
        derived().print(")");
      }
    }
    derived().print(closer);
  }

  bool is_abstraction_application(const application& x) const
  {
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
    while (sort_fset::is_fset_cons_application(x) || sort_fset::is_fsetinsert_application(x))
    {
      if (sort_fset::is_fset_cons_application(x))
      {
        x = sort_fset::tail(x);
      }
      else // if (sort_fset::is_fsetinsert_application(x))
      {
        x = sort_fset::right(x);
      }
    }
    return sort_fset::is_fset_empty_function_symbol(x);
  }

  /// \brief Returns true if x is a list composed of cons, insert and cinsert applications.
  bool is_fbag_cons_list(data_expression x)
  {
    while (sort_fbag::is_fbag_cons_application(x) || sort_fbag::is_fbaginsert_application(x) || sort_fbag::is_fbagcinsert_application(x))
    {
      if (sort_fbag::is_fbag_cons_application(x))
      {
        x = sort_fbag::tail(x);
      }
      else
      {
        x = sort_fbag::arg3(x);
      }
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

  bool is_numeric_value(const application& x)
  {
    return is_numeric_expression(x);
  }

  bool is_numeric_expression(const application& x)
  {
    return    sort_pos::is_pos(x.sort()) 
           || sort_nat::is_nat(x.sort())
           || sort_int::is_int(x.sort())
           || sort_real::is_real(x.sort());
  }

  bool is_standard_sort(const sort_expression& x)
  {
    return    sort_pos::is_pos(x) 
           || sort_bool::is_bool(x)
           || sort_nat::is_nat(x)
           || sort_int::is_int(x)
           || sort_real::is_real(x);
  }

  bool is_fset_true(data_expression x)
  {
    return sort_set::is_true_function_function_symbol(sort_set::left(x));
  }

  bool is_fset_false(data_expression x)
  {
    return sort_set::is_false_function_function_symbol(sort_set::left(x));
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
    while (sort_fset::is_fset_cons_application(x) || sort_fset::is_fsetinsert_application(x))
    {
      if (sort_fset::is_fset_cons_application(x))
      {
        arguments.push_back(sort_fset::head(x));
        x = sort_fset::tail(x);
      }
      else // if (sort_fset::is_fsetinsert_application(x))
      {
        arguments.push_back(sort_fset::left(x));
        x = sort_fset::right(x);
      }
    }
    derived().print("{");
    print_container(arguments, 6);
    derived().print("}");
  }
 
  void print_fbag_zero(const data_expression& x)
  {
    // TODO: check if this is the correct way to handle this case
    data_expression y = sort_bag::right(x);
    if (sort_fbag::is_fbag_empty_function_symbol(y))
    {
      derived().print("{}");
    }
    else if (data::is_variable(y))
    {
      derived().print("@bagfbag(");
      derived()(variable(y).name());
      derived().print(")");
    }
    else
    {
      derived().print("{");
      derived()(y);
      derived().print("}");
    }
  } 
  
  void print_fbag_one(const data_expression& x)
  {
    sort_expression s = function_sort(sort_bag::left(x).sort()).domain().front(); // the sort of the bag elements
    core::identifier_string name = generate_identifier("x", x);
    variable var(name, s);
    data_expression body = number(sort_nat::nat(), "1");
    if (!sort_fbag::is_fbag_empty_function_symbol(sort_bag::right(x)))
    {
      body = sort_nat::swap_zero(body, sort_bag::bagcount(s, var, sort_bag::bagfbag(s, sort_bag::right(x))));
    }
    derived().print("{ ");
    print_variable(var, true);
    derived().print(" | ");
    derived()(body);
    derived().print(" }");
  } 
  
  void print_fbag_lambda(data_expression x)
  {
    sort_expression s = function_sort(sort_bag::left(x).sort()).domain().front(); // the sort of the bag elements
    core::identifier_string name = generate_identifier("x", x);
    variable var(name, s);
    data::lambda left(sort_bag::left(x));
    data_expression body = left.body();
    if (!sort_fbag::is_fbag_empty_function_symbol(sort_bag::right(x)))
    {
      body = sort_nat::swap_zero(body, sort_bag::bagcount(s, var, sort_bag::bagfbag(s, sort_bag::right(x))));
    }
    derived().print("{ ");
    print_variables(left.variables(), true, true, "", "", ", ");
    derived().print(" | ");
    derived()(body);
    derived().print(" }");
  }   
  
  void print_fbag_default(data_expression x)
  {
    // std::cout << "<fbag_default>" << core::pp(x) << " " << core::pp(sort_bag::left(x)) << " " << sort_bag::left(x) << std::endl;
    //if (sort_bag::is_false_function_function_symbol(sort_bag::left(x)))
    if (false)
    {
      derived().print("@bagfbag(");
      derived()(sort_set::right(x));
      derived().print(")");
    }
    else
    {     
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
      print_variable(var, true);
      derived().print(" | ");
      derived()(body);
      derived().print(" }");
    }
  }  

  void print_fbag_cons_list(data_expression x)
  {
    atermpp::vector<std::pair<data_expression, data_expression> > arguments;
    while (sort_fbag::is_fbag_cons_application(x) || sort_fbag::is_fbaginsert_application(x) || sort_fbag::is_fbagcinsert_application(x))
    {
      if (sort_fbag::is_fbag_cons_application(x))
      {
        arguments.push_back(std::make_pair(sort_fbag::head(x), sort_fbag::headcount(x)));
        x = sort_fbag::tail(x);
      }
      else if (sort_fbag::is_fbaginsert_application(x))
      {
        arguments.push_back(std::make_pair(sort_fbag::arg1(x), sort_nat::cnat(sort_fbag::arg2(x))));
        x = sort_fbag::arg3(x);
      }
      else // if (sort_fbag::is_fbagcinsert_application(x))
      {
        arguments.push_back(std::make_pair(sort_fbag::arg1(x), sort_fbag::arg2(x)));
        x = sort_fbag::arg3(x);
      }
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
    print_variables(left.variables(), true, true, "", "", ", ");
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
        sort_expression s = function_sort(sort_set::left(x).sort()).domain().front();
        core::identifier_string name = generate_identifier("x", x);
        variable var(name, s);
        data_expression body(sort_set::left(x)(var));
        derived().print("{ ");
        print_variable(var, true);
        derived().print(" | ");
        derived()(body);
        derived().print(" }");         
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
        print_variable(var, true);
        derived().print(" | ");
        derived()(body);
        derived().print(" }");         
    }
  }  

  template <typename Abstraction>
  void print_abstraction(const Abstraction& x, const std::string& op)
  {
    derived().enter(x);
    derived().print(op + " ");
    print_variables(x.variables(), true, true, "", "", ", ");   
    derived().print(". ");
    derived()(x.body());
    derived().leave(x);
  }

  void print_function_application(const application& x)
  {
    if (is_infix_operation(x))
    {
      data_expression_list::const_iterator i = x.arguments().begin();
      core::identifier_string name = function_symbol(x.head()).name();
      data_expression left = *i++;
      data_expression right = *i;
      print_expression(left, infix_precedence_left(left));
      derived().print(" ");
      derived()(x.head());
      derived().print(" ");
      print_expression(right, infix_precedence_right(right));
      return;
    }
    
    // print the head
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

    // print the arguments
    print_parentheses = x.arguments().size() > 0;
    if (is_function_symbol(x.head()) && x.arguments().size() == 1)
    {
      std::string name(function_symbol(x.head()).name());
      if (name == "!" || name == "#")
      {
        print_parentheses = precedence(x.arguments().front()) < max_precedence;
      }
    }
    if (print_parentheses)
    {
      derived().print("(");
    }
    print_container(x.arguments());
    if (print_parentheses)
    {
      derived().print(")");
    }   
  }

  // N.B. This is interpreted as the bag element 'x.first: x.second'
  void operator()(const std::pair<data_expression, data_expression>& x)
  {
    derived()(x.first);
    derived().print(": ");
    derived()(x.second);
  }

  // TODO: this code should be generated!
  void operator()(const data::container_type& x)
  {
    static_cast<Derived&>(*this).enter(x);
    if (data::is_list_container(x))
    {
      static_cast<Derived&>(*this)(data::list_container(atermpp::aterm_appl(x)));
    }
    else if (data::is_set_container(x))
    {
      static_cast<Derived&>(*this)(data::set_container(atermpp::aterm_appl(x)));
    }
    else if (data::is_bag_container(x))
    {
      static_cast<Derived&>(*this)(data::bag_container(atermpp::aterm_appl(x)));
    }
    else if (data::is_fset_container(x))
    {
      static_cast<Derived&>(*this)(data::fset_container(atermpp::aterm_appl(x)));
    }
    else if (data::is_fbag_container(x))
    {
      static_cast<Derived&>(*this)(data::fbag_container(atermpp::aterm_appl(x)));
    }
    static_cast<Derived&>(*this).leave(x);
  }

  void operator()(const data::assignment& x)
  {
    derived().enter(x);
    derived()(x.lhs());
    derived().print(" = ");
    derived()(x.rhs());
    derived().leave(x);
  }

//  // assignment lists have their own notation
//  void operator()(const data::assignment_list& x)
//  {
//    derived().enter(x);
//    print_assignments(x, true, "[", "]", ", ", ":=");
//    derived().leave(x);
//  }

  // variable lists have their own notation
  void operator()(const data::variable_list& x)
  {
    derived().enter(x);
    print_list(x, "[", "]", ", ", true);
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
    if (x.name() != no_identifier())
    {
      derived()(x.name());
      derived().print(": ");
    }
    derived()(x.sort());
    derived().leave(x);
  }

  void operator()(const data::structured_sort_constructor& x)
  {
    derived().enter(x);
    derived()(x.name());
    print_list(x.arguments(), "(", ")", ", ");
    if (x.recogniser() != data::no_identifier())
    {
      derived().print("?");
      derived()(x.recogniser());
    }
    derived().leave(x);
  }

  void operator()(const data::alias& x)
  {
    derived().enter(x);
    derived()(x.name());
    derived().print(" = ");
    derived()(x.reference());
    derived().leave(x);
  }

  void operator()(const data::list_container& x)
  {
    derived().enter(x);
    derived().print("List");
    derived().leave(x);
  }

  void operator()(const data::set_container& x)
  {
    derived().enter(x);
    derived().print("Set");
    derived().leave(x);
  }

  void operator()(const data::bag_container& x)
  {
    derived().enter(x);
    derived().print("Bag");
    derived().leave(x);
  }

  void operator()(const data::fset_container& x)
  {
    derived().enter(x);
    derived().print("@FSet");
    derived().leave(x);
  }

  void operator()(const data::fbag_container& x)
  {
    derived().enter(x);
    derived().print("@FBag");
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
    derived().print("(");
    derived()(x.element_sort());
    derived().print(")");
    derived().leave(x);
  }

  void operator()(const data::structured_sort& x)
  {
    derived().enter(x);
    print_list(x.constructors(), "struct ", "", " | ");
    derived().leave(x);
  }

  void operator()(const data::function_sort& x)
  {
    derived().enter(x);
    print_sort_list(x.domain(), "", " -> ", " # ");
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
    else if (sort_fset::is_fset_empty_function_symbol(x))
    {
      derived().print("{}");
    }
    else
    {
      derived().print(x.name());
    }    
    derived().leave(x);
  }

  void operator()(const data::application& y)
  {
    // TODO: this copy is done because of the "numeric reconstruction", which
    // will hopefully be removed in the future.
    data::application x = y;
      
#ifdef MCRL2_DEBUG_PRINT
    print_sort(x);
#endif
    derived().enter(x);

    //-------------------------------------------------------------------//
    //                            numeric values
    //-------------------------------------------------------------------//
    if (is_numeric_value(x))
    {
//std::cout << "<reconstruct>" << core::pp(x) << " -> ";
      data_expression z = detail::reconstruct_numeric_expression(x);
      if (is_function_symbol(z))
      {
        derived().print(function_symbol(z).name());
        return;
      }
      else
      {
        x = z;
      }
    }

    // TODO: can these be moved to int/pos/nat/real?
    if (is_numeric_cast(x))
    {
      // ignore numeric casts like Pos2Nat
      derived()(x.arguments().front());
    }

    //-------------------------------------------------------------------//
    //                            bool
    //-------------------------------------------------------------------//
    else if (sort_bool::is_bool(x.sort()))
    {
      if (data::is_equal_to_application(x))
      {
        print_binary_operation(x, " == ");
      }
      else if (data::is_not_equal_to_application(x))
      {
        print_binary_operation(x, " != ");
      }     
      else if (data::is_less_application(x))
      {
        print_binary_operation(x, " < ");
      }
      else if (data::is_less_equal_application(x))
      {
        print_binary_operation(x, " <= ");
      }
      else if (data::is_greater_application(x))
      {
        print_binary_operation(x, " > ");
      }
      else if (data::is_greater_equal_application(x))
      {
        print_binary_operation(x, " >= ");
      }
      else if (sort_bool::is_implies_application(x))
      {
        print_binary_operation(x, " => ");
      }
      else if (sort_bool::is_and_application(x))
      {
        print_binary_operation_special(x, " && ");
      }
      else if (sort_bool::is_or_application(x))
      {
        print_binary_operation_special(x, " || ");
      }
      else if (sort_list::is_in_application(x))
      {
        print_binary_operation(x, " in ");
      }
      else if (sort_list::is_element_at_application(x))
      {
        print_binary_operation(x, " . ");
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
      if (   sort_pos::is_plus_application(x)
          || sort_nat::is_plus_application(x)
         )
      {
        print_binary_operation(x, " + ");
      }
      else if (sort_pos::is_times_application(x))
      {
        print_binary_operation(x, " * ");
      }
      else
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
        print_binary_operation(x, " + ");
      }
      else if (sort_nat::is_times_application(x))
      {
        print_binary_operation(x, " * ");
      }
      else if (sort_nat::is_div_application(x))
      {
        // TODO: make a proper binary operation of div
        print_expression(sort_nat::arg1(x), precedence(x));
        derived().print(" div ");
        print_expression(sort_nat::arg2(x), precedence(x));
      }     
      else if (   sort_nat::is_mod_application(x)
               || sort_int::is_mod_application(x)
              )
      {
        // TODO: make a proper binary operation of mod
        print_expression(sort_nat::arg1(x), precedence(x));
        derived().print(" mod ");
        print_expression(sort_nat::arg2(x), precedence(x));
      }
      else if (sort_nat::is_exp_application(x))
      {
        // print_binary_operation(x, " ^ ");
        print_function_application(x);
      }
      else if (sort_nat::is_cnat_application(x))
      {
        derived()(sort_nat::arg(x));
      }
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
        print_binary_operation(x, " + ");
      }
      else if (sort_int::is_minus_application(x))
      {
        print_binary_operation(x, " - ");
      }
      else if (sort_int::is_times_application(x))
      {
        print_binary_operation(x, " * ");
      }
      else if (sort_int::is_div_application(x))
      {
        // TODO: make a proper binary operation of div
        print_expression(sort_int::arg1(x), precedence(x));
        derived().print(" div ");
        print_expression(sort_int::arg2(x), precedence(x));
      }
      else if (sort_int::is_cint_application(x))
      {
        derived()(sort_int::arg(x));
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
        print_binary_operation(x, " + ");
      }
      else if (sort_real::is_minus_application(x))
      {
        print_binary_operation(x, " - ");
      }
      else if (sort_real::is_times_application(x))
      {
        print_binary_operation(x, " * ");
      }
      else if (sort_real::is_divides_application(x))
      {
        print_binary_operation(x, " / ");
      }
      else if (sort_real::is_creal_application(x)) // TODO: fix this!!!
      {
        derived()(sort_real::numerator(x));
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
        print_binary_operation(x, " ++ ");
      }
      else if (sort_list::is_cons_application(x))
      {
        if (is_cons_list(x))
        {
          print_cons_list(x);
        }
        else
        {
          print_binary_operation(x, " |> ");
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
          print_binary_operation(x, " <| ");
        }
      }
      else if (sort_list::is_list_enumeration_application(x))
      {
        derived().print("[");
        print_container(x.arguments(), precedence(x));
        derived().print("]");
      }
      else
      {
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
        print_binary_operation(x, " + ");
      }
      else if (sort_set::is_setintersection_application(x))
      {
        print_binary_operation(x, " * ");
      }
      else if (sort_set::is_setdifference_application(x))
      {
        print_binary_operation(x, " - ");
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
        print_variable(var, true);
        derived().print(" | ");
        derived()(body);
        derived().print(" }");   
      }
      else if (sort_set::is_setfset_application(x))
      {
        data_expression y = sort_set::arg(x);
        if (sort_fset::is_fset_empty_function_symbol(y))
        {
          derived().print("{}");
        }
        else if (data::is_variable(y))
        {
          derived().print("@setfset(");
          derived()(variable(y).name());
          derived().print(")");
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
      if (is_fset_cons_list(x))
      {
        print_fset_cons_list(x);
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
        print_binary_operation(x, " + ");
      }
      else if (sort_bag::is_bagintersect_application(x))
      {
        print_binary_operation(x, " * ");
      }
      else if (sort_bag::is_bagdifference_application(x))
      {
        print_binary_operation(x, " - ");
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
        data_expression y = sort_bag::arg(x);
        if (sort_fbag::is_fbag_empty_function_symbol(y))
        {
          derived().print("{}");
        }
        else if (data::is_variable(y))
        {
          derived().print("@bagfbag(");
          derived()(variable(y).name());
          derived().print(")");
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
      if (is_fbag_cons_list(x))
      {
        print_fbag_cons_list(x);
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
      print_function_application(x);
    }
    derived().leave(x);
  }

  void operator()(const data::where_clause& x)
  {
    derived().enter(x);
    derived()(x.body());
    derived().print(" whr ");
    assignment_expression_list declarations = x.declarations();
    for (assignment_expression_list::const_iterator i = declarations.begin(); i != declarations.end(); ++i)
    {
      if (i != declarations.begin())
      {
        derived().print(", ");
      }
      derived()(i->lhs());
      derived().print(" = ");
      derived()(i->rhs());
    }
    derived().print(" end");
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
    print_condition(x.condition());
    derived()(x.lhs());
    derived().print("  =  ");
    derived()(x.rhs());
    derived().leave(x);
  }

  // Container contains elements of type T such that t.sort() is a sort_expression.
  template <typename Container>
  void print_function_declarations(const Container& container,
                                   const std::string& opener = "(",
                                   const std::string& closer = ")",
                                   const std::string& separator = ", "
                                  )
  {
    typedef typename Container::value_type T;
    
    // print nothing if the container is empty
    if (container.empty())
    {
      return;
    }

    typename Container::const_iterator first = container.begin();
    typename Container::const_iterator last = container.end();

    derived().print(opener);   

    while (first != last)
    {
      if (first != container.begin())
      {
        derived().print(separator);
      }

      typename Container::const_iterator i = first;
      do
      {
        ++i;
      }
      while (i != last && first->sort() == i->sort());

      print_list(std::vector<T>(first, i), "", "", ",");
      derived().print(": ");
      derived()(first->sort());
      
      first = i;
    }
    derived().print(closer);
  }

  // Container contains elements of type T such that t.sort() is a sort_expression.
  template <typename Container>
  void print_function_declarations_maximally_shared(const Container& container,
                          const std::string& opener = "(",
                          const std::string& closer = ")",
                          const std::string& separator = ", "
                         )
  {
    typedef typename Container::value_type T;
    
    // print nothing if the container is empty
    if (container.empty())
    {
      return;
    }

    // sort_map[s] will contain all elements t of container with t.sort() == s.
    std::map<sort_expression, std::vector<T> > sort_map;

    // sorts will contain all sort expressions s that appear as a key in sort_map,
    // in the order they are encountered in container
    std::vector<sort_expression> sorts;

    for (typename Container::const_iterator i = container.begin(); i != container.end(); ++i)
    {
      if (sort_map.find(i->sort()) == sort_map.end())
      {
        sorts.push_back(i->sort());
      }
      sort_map[i->sort()].push_back(*i);
    }

    // do the actual printing
    derived().print(opener);   
    for (std::vector<sort_expression>::iterator i = sorts.begin(); i != sorts.end(); ++i)
    {
      if (i != sorts.begin())
      {
        derived().print(separator);
      }
      const std::vector<T>& v = sort_map[*i];
      print_list(v, "", "", ",");
      derived().print(": ");
      derived()(*i);
    }
    derived().print(closer);
  }

  // Adds variables v and function symbols f to variable_map and function_symbol_names respectively.
  void update_mappings(const data_equation& eqn,
                       std::vector<variable>& variables,
                       std::map<core::identifier_string, variable>& variable_map,
                       std::set<core::identifier_string>& function_symbol_names
                      )
  {
    std::set<function_symbol> f = data::find_function_symbols(eqn);
    for (std::set<function_symbol>::iterator i = f.begin(); i != f.end(); ++i)
    {
      function_symbol_names.insert(i->name());
    }
    const variable_list& v = eqn.variables();
    for (variable_list::const_iterator i = v.begin(); i != v.end(); ++i)
    {
      std::pair<std::map<core::identifier_string, variable>::iterator, bool> k = variable_map.insert(std::make_pair(i->name(), *i));
      if (k.second) // new variable encountered
      {
        variables.push_back(*i);
      }
    }
  }

  bool has_conflict(const data_equation& eqn,
                    const std::map<core::identifier_string, variable>& variable_map
                   )
  {
    const variable_list& v = eqn.variables();
    for (variable_list::const_iterator i = v.begin(); i != v.end(); ++i)
    {
      std::map<core::identifier_string, variable>::const_iterator j = variable_map.find(i->name());
      if (j != variable_map.end() && *i != j->second)
      {
        return true;
      }
    }
    return false;
  }

  /// \brief Searches in the range of equations [first, last) for the first equation
  /// that conflicts with one of the previous equations. We say that equation e1 conflicts
  /// with another equation e2 if their declared variables contain a variable with the same
  /// name and a different sort, or if a declared variable in e1 has the same name as a
  /// function symbol appearing in equation e2.
  template <typename Iter>
  Iter find_conflicting_equation(Iter first, Iter last, std::vector<variable>& variables)
  {
    std::map<core::identifier_string, variable> variable_map;
    std::set<core::identifier_string> function_symbol_names;
    for (Iter i = first; i != last; ++i)
    {
      if (has_conflict(*i, variable_map))
      {
        return i;
      }
      else
      {
        update_mappings(*i, variables, variable_map, function_symbol_names);
      }
    }
    return last; // no conflict found
  }

  template <typename Container>
  bool has_non_standard_sorts(const Container& c)
  {
    for (typename Container::const_iterator i = c.begin(); i != c.end(); ++i)
    {
      if (!is_standard_sort(*i))
      {
        return true;
      }
    }
    return false;
  }

  template <typename AliasContainer, typename SortContainer>
  void print_sort_declarations(const AliasContainer& aliases,
                               const SortContainer& sorts,
                               const std::string& opener = "(",
                               const std::string& closer = ")",
                               const std::string& separator = ", "
                              )
  {
    if (aliases.empty() && sorts.empty())
    {
      return;
    }
    bool first_element = true;
    derived().print(opener);

    // print aliases
    for (typename AliasContainer::const_iterator i = aliases.begin(); i != aliases.end(); ++i)
    {
      if (!first_element)
      {
        derived().print(separator);
      }
      derived()(*i);
      first_element = false;
    }

    // print sorts
    for (typename SortContainer::const_iterator i = sorts.begin(); i != sorts.end(); ++i)
    {
      if (!first_element)
      {
        derived().print(separator);
      }
      derived()(*i);
      first_element = false;
    }

    derived().print(closer);
  }

  template <typename Container>
  void print_equations(const Container& equations,
                       const data_specification& data_spec,
                       const std::string& opener = "(",
                       const std::string& closer = ")",
                       const std::string& separator = ", "
                      )
  {
    typename Container::const_iterator first = equations.begin();
    typename Container::const_iterator last = equations.end();
      
    Container normalized_equations = equations;
    data::normalize_sorts(normalized_equations, data_spec);

    while (first != last)
    {
      std::vector<variable> variables;
      typename Container::const_iterator i = find_conflicting_equation(first, last, variables);
      print_function_declarations_maximally_shared(variables, "var  ", ";\n", ";\n     ");

      // N.B. We print normalized equations instead of user defined equations.
      // print_list(std::vector<data_equation>(first, i), opener, closer, separator);
      typename Container::const_iterator first1 = normalized_equations.begin() + (first - equations.begin());
      typename Container::const_iterator i1 = normalized_equations.begin() + (i - equations.begin());
      print_list(std::vector<data_equation>(first1, i1), opener, closer, separator);

      first = i;
    }
  }

  void operator()(const data::data_specification& x)
  {
    derived().enter(x);
    print_sort_declarations(x.user_defined_aliases(), x.user_defined_sorts(), "sort ", ";\n\n", ";\n     ");
    print_function_declarations(x.user_defined_constructors(), "cons ",";\n\n", ";\n     ");
    print_function_declarations(x.user_defined_mappings(), "map  ",";\n\n", ";\n     ");
    print_equations(x.user_defined_equations(), x, "eqn  ", ";\n\n", ";\n     ");
    derived().leave(x);
  }

  void operator()(const data::data_expression_with_variables& x)
  {
    derived().enter(x);
    derived()(static_cast<const data::data_expression&>(x));
    derived().print(" ");
    derived()(x.variables());
    derived().leave(x);   
  }

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

