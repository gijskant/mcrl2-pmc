// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/core/parser_utility.h
/// \brief add your file description here.

#ifndef MCRL2_CORE_PARSER_UTILITY_H
#define MCRL2_CORE_PARSER_UTILITY_H

#include "mcrl2/core/dparser.h"

namespace mcrl2 {

namespace core {

/// \brief Calls the function f on each node in the parse tree with x as root
template <typename Function>
void foreach_parse_node(const parse_node& x, Function f)
{
  if (x)
  {
    f(x);
    for (int i = 0; i < x.child_count(); i++)
    {
      foreach_parse_node(x.child(i), f);
    }
  }
}

/// \brief Checks if a node is the binary operation op
struct is_binary_operator
{
  const parser_table& table;
  std::string op;

  is_binary_operator(const parser_table& table_, const std::string& op_)
    : table(table_),
      op(op_)
  {}

  bool operator()(const parse_node& x)
  {
    if (!x)
    {
      return false;
    }
    bool result = true;
    if (x.child_count() != 3)
    {
      result = false;
    }
    else
    {
      if (table.symbol_name(x.child(1).symbol()) != op)
      {
        result = false;
      }
    }
    return result;
  }
};

/// \brief Checks if a node is the binary operation '&&'
struct is_and: public is_binary_operator
{
  is_and(const parser_table& table_)
    : is_binary_operator(table_, "&&")
  {}
};

/// \brief Checks if a node is the binary operation '||'
struct is_or: public is_binary_operator
{
  is_or(const parser_table& table_)
    : is_binary_operator(table_, "||")
  {}
};

/// \brief Checks if a node is the merge operation '||'
struct is_merge: public is_binary_operator
{
  is_merge(const parser_table& table_)
    : is_binary_operator(table_, "||")
  {}
};

/// \brief Checks if a node is the left merge operation '||_'
struct is_left_merge: public is_binary_operator
{
  is_left_merge(const parser_table& table_)
    : is_binary_operator(table_, "||_")
  {}
};

/// \brief Checks if a node is of type 'x && (y || z)'
struct is_and_or
{
  const parser_table& table;

  is_and_or(const parser_table& table_)
    : table(table_)
  {}

  bool operator()(const parse_node& x)
  {
    bool result = true;
    if (!is_and(table)(x))
    {
      result = false;
    }
    else
    {
      // check if the types match
      parse_node left = x.child(0);
      parse_node right = x.child(2);
      if (x.symbol() != left.symbol())
      {
        result = false;
      }
      if (x.symbol() != right.symbol())
      {
        result = false;
      }
      if (!is_or(table)(right))
      {
        result = false;
      }
    }
    return result;
  }
};

/// \brief Checks if a node is of type 'x ||_ (y || z)'
struct is_left_merge_merge
{
  const parser_table& table;

  is_left_merge_merge(const parser_table& table_)
    : table(table_)
  {}

  bool operator()(const parse_node& x)
  {
    bool result = true;
    if (!is_left_merge(table)(x))
    {
      result = false;
    }
    else
    {
      // check if the types match
      parse_node left = x.child(0);
      parse_node right = x.child(2);
      if (x.symbol() != left.symbol())
      {
        result = false;
      }
      if (x.symbol() != right.symbol())
      {
        result = false;
      }
      if (!is_merge(table)(right))
      {
        result = false;
      }
    }
    return result;
  }
};

struct warn_and_or
{
  const parser_table& table;

  warn_and_or(const parser_table& table_)
    : table(table_)
  {}

  void operator()(const parse_node& x)
  {
    if (is_and_or(table)(x))
    {
      std::cout << "Warning: the expression of the form 'x && y || z' on location " << x.line() << ":" << x.column() << " may be parsed differently than before" << std::endl;
    }
  }
};

struct warn_left_merge_merge
{
  const parser_table& table;

  warn_left_merge_merge(const parser_table& table_)
    : table(table_)
  {}

  void operator()(const parse_node& x)
  {
    if (is_left_merge_merge(table)(x))
    {
      std::cout << "Warning: the expression of the form 'x ||_ y || z' on location " << x.line() << ":" << x.column() << " may be parsed differently than before" << std::endl;
    }
  }
};

} // namespace core

} // namespace mcrl2

#endif // MCRL2_CORE_PARSER_UTILITY_H
