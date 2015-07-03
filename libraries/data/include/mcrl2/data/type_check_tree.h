// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/type_check_tree.h
/// \brief add your file description here.

#ifndef MCRL2_DATA_TYPE_CHECK_TREE_H
#define MCRL2_DATA_TYPE_CHECK_TREE_H

#include <iostream>
#include <memory>
#include <sstream>
#include "mcrl2/data/type_checker.h"
#include "mcrl2/data/parse.h"
#include "mcrl2/data/untyped_sort_variable.h"

namespace mcrl2 {

namespace data {

// Represents an arbitrary sort
inline
const basic_sort& any_sort()
{
  static const basic_sort t("@any_sort");
  return t;
}

template <typename Container>
std::string print_node_vector(const std::string& name, const Container& nodes)
{
  std::ostringstream out;
  out << name << "(";
  for (auto i = nodes.begin(); i != nodes.end(); ++i)
  {
    if (i != nodes.begin())
    {
      out << ", ";
    }
    out << (*i)->print();
  }
  out << ")";
  return out.str();
}

template <typename Container>
std::string print_vector(const std::string& name, const Container& nodes)
{
  std::ostringstream out;
  out << name << "(";
  for (auto i = nodes.begin(); i != nodes.end(); ++i)
  {
    if (i != nodes.begin())
    {
      out << ", ";
    }
    out << *i;
  }
  out << ")";
  return out.str();
}

struct type_check_node;

typedef std::shared_ptr<type_check_node> type_check_node_ptr;

struct type_check_context
{
  const std::map<core::identifier_string, sort_expression_list>& system_constants;
  const std::map<core::identifier_string, function_sort_list>& system_functions;
  const std::map<core::identifier_string, sort_expression>& user_constants;
  const std::map<core::identifier_string, function_sort_list>& user_functions;
  std::map<core::identifier_string, std::vector<sort_expression> > declared_variables;
  // const std::map<core::identifier_string, sort_expression>& allowed_variables;

  std::map<untyped_sort_variable, sort_expression> sort_variables;
  std::size_t sort_variable_index;

  type_check_context(
                     const std::map<core::identifier_string, sort_expression_list>& system_constants_,
                     const std::map<core::identifier_string, function_sort_list>& system_functions_,
                     const std::map<core::identifier_string, sort_expression>& user_constants_,
                     const std::map<core::identifier_string, function_sort_list>& user_functions_,
                     const std::map<core::identifier_string, std::vector<sort_expression> >& declared_variables_
                    )
    : system_constants(system_constants_),
      system_functions(system_functions_),
      user_constants(user_constants_),
      user_functions(user_functions_),
      declared_variables(declared_variables_),
      sort_variable_index(0)
  { }

  type_check_context(const type_checker& checker, const std::map<core::identifier_string, std::vector<sort_expression> >& declared_variables_)
    : system_constants(checker.system_constants()),
      system_functions(checker.system_functions()),
      user_constants(checker.user_constants()),
      user_functions(checker.user_functions()),
      declared_variables(declared_variables_),
      sort_variable_index(0)
  {}

  // Returns the system defined constants and the user defined constants matching with name
  std::pair<sort_expression_list, sort_expression_list> find_matching_constants(const std::string& name) const;

  // Returns the system defined functions and the user defined functions matching with (name, arity)
  std::pair<function_sort_list, function_sort_list> find_matching_functions(const std::string& name, std::size_t arity) const;

  // Returns the system defined functions and the user defined functions matching with name
  std::pair<function_sort_list, function_sort_list> find_matching_functions(const std::string& name) const;

  // Returns the variables matching with name
  std::vector<sort_expression> find_matching_variables(const std::string& name) const;

  untyped_sort_variable create_sort_variable()
  {
    untyped_sort_variable result(sort_variable_index++);
    sort_variables[result] = untyped_sort();
    return result;
  }

  void add_context_variable(const variable& v)
  {
    declared_variables[v.name()].push_back(v.sort());
  }

  void add_context_variables(const variable_list& variables)
  {
    for (const variable& v: variables)
    {
      add_context_variable(v);
    }
  }

  void remove_context_variable(const variable& v)
  {
    auto i = declared_variables.find(v.name());
    if (i->second.size() <= 1)
    {
      declared_variables.erase(i);
    }
    else
    {
      i->second.pop_back();
    }
  }

  void remove_context_variables(const variable_list& variables)
  {
    for (const variable& v: variables) // N.B. the order of insertion and removal should not matter
    {
      remove_context_variable(v);
    }
  }
};

struct type_check_constraint;
typedef std::shared_ptr<type_check_constraint> constraint_ptr;
constraint_ptr expand_constraint(constraint_ptr p);

struct type_check_constraint
{
  virtual std::string print() const = 0;
};

struct no_constraint: public type_check_constraint
{
  std::string print() const
  {
    return "no_constraint()";
  }
};

// The sort of the corresponding data expression should be equal to 'sort'.
struct is_sort_constraint: public type_check_constraint
{
  sort_expression s1;
  sort_expression s2;

  is_sort_constraint(const sort_expression& s1_, const sort_expression& s2_)
    : s1(s1_), s2(s2_)
  {}

  std::string print() const
  {
    return "is_sort(" + data::pp(s1) + ", " + data::pp(s2) + ")";
  }
};

inline
constraint_ptr make_is_sort_constraint(const sort_expression& s1, const sort_expression& s2)
{
  return constraint_ptr(new is_sort_constraint(s1, s2));
}

// The sort variable s1 should be a subsort of s2.
struct subsort_constraint: public type_check_constraint
{
  sort_expression s1;
  sort_expression s2;

  subsort_constraint(const sort_expression& s1_, const sort_expression& s2_)
    : s1(s1_), s2(s2_)
  {}

  std::string print() const
  {
    return "subsort(" + data::pp(s1) + ", " + data::pp(s2) + ")";
  }
};

inline
constraint_ptr make_subsort_constraint(const sort_expression& s1, const sort_expression& s2)
{
  return constraint_ptr(new subsort_constraint(s1, s2));
}

struct or_constraint: public type_check_constraint
{
  std::vector<constraint_ptr> alternatives;

  or_constraint(const std::vector<constraint_ptr>& alternatives_)
    : alternatives(alternatives_)
  {}

  std::string print() const
  {
    return print_node_vector("or", alternatives);
  }
};

inline
constraint_ptr make_or_constraint(const std::vector<constraint_ptr>& alternatives)
{
  std::vector<constraint_ptr> v;
  for (constraint_ptr p: alternatives)
  {
    no_constraint* x_no = dynamic_cast<no_constraint*>(p.get());
    if (x_no)
    {
      continue;
    }
    or_constraint* x_or = dynamic_cast<or_constraint*>(p.get());
    if (x_or)
    {
      v.insert(v.end(), x_or->alternatives.begin(), x_or->alternatives.end());
      continue;
    }
    v.push_back(p);
  }
  if (v.size() == 0)
  {
    return constraint_ptr(new no_constraint());
  }
  else if (alternatives.size() == 1)
  {
    return v.front();
  }
  return constraint_ptr(new or_constraint(v));
}

struct and_constraint: public type_check_constraint
{
  std::vector<constraint_ptr> alternatives;

  and_constraint(const std::vector<constraint_ptr>& alternatives_)
    : alternatives(alternatives_)
  {}

  std::string print() const
  {
    return print_node_vector("and", alternatives);
  }
};

struct false_constraint: public type_check_constraint
{
  std::string message;

  false_constraint(const std::string& message_)
    : message(message_)
  {}

  std::string print() const
  {
    return "false(" + message + ")";
  }
};

inline
constraint_ptr make_and_constraint(const std::vector<constraint_ptr>& alternatives)
{
  std::vector<constraint_ptr> v;
  for (constraint_ptr p: alternatives)
  {
    no_constraint* x_no = dynamic_cast<no_constraint*>(p.get());
    if (x_no)
    {
      continue;
    }
    and_constraint* x_and = dynamic_cast<and_constraint*>(p.get());
    if (x_and)
    {
      v.insert(v.end(), x_and->alternatives.begin(), x_and->alternatives.end());
      continue;
    }
    v.push_back(p);
  }
  if (v.size() == 0)
  {
    return constraint_ptr(new no_constraint());
  }
  else if (alternatives.size() == 1)
  {
    return v.front();
  }
  return constraint_ptr(new and_constraint(v));
}

struct type_check_node
{
  std::vector<type_check_node_ptr> children;
  constraint_ptr constraint;
  sort_expression sort;

  type_check_node()
    : constraint(constraint_ptr(new no_constraint())), sort(undefined_sort_expression())
  {}

  type_check_node(const std::vector<type_check_node_ptr>& children_)
    : children(children_), constraint(constraint_ptr(new no_constraint())), sort(undefined_sort_expression())
  {}

  // Adds a value to the 'sort' attribute
  // Sets the constraints that apply to this node to 'constraint'
  virtual void set_constraint(type_check_context& context)
  {}

  void set_children_constraints(type_check_context& context)
  {
    for (type_check_node_ptr child: children)
    {
      child->set_constraint(context);
    }
  }

  // Throws an exception if the node violates a well typedness rule
  virtual void check_well_typedness(const type_check_context& context)
  {}

  virtual std::string print() const = 0;

  void expand_constraints()
  {
    for (type_check_node_ptr child: children)
    {
      child->expand_constraints();
    }
    constraint = expand_constraint(constraint);
  }
};

struct id_node: public type_check_node
{
  std::string value;

  id_node(const std::string& value_)
    : value(value_)
  {}

  void set_constraint(type_check_context& context)
  {
    sort = context.create_sort_variable();

    std::vector<constraint_ptr> alternatives;

    // it is a variable
    std::vector<sort_expression> variable_sorts = context.find_matching_variables(value);
    if (variable_sorts.size() == 1)
    {
      alternatives.push_back(constraint_ptr(new is_sort_constraint(sort, variable_sorts.front())));
    }

    // it is a constant
    std::pair<sort_expression_list, sort_expression_list> p = context.find_matching_constants(value);
    for (const sort_expression& s: p.first + p.second)
    {
      alternatives.push_back(constraint_ptr(new is_sort_constraint(sort, s)));
    }

    // it is a function
    std::pair<function_sort_list, function_sort_list> q = context.find_matching_functions(value);
    for (const function_sort& s: q.first + q.second)
    {
      alternatives.push_back(constraint_ptr(new is_sort_constraint(sort, s)));
    }

    if (alternatives.empty())
    {
      constraint = constraint_ptr(new false_constraint("The id " + value + " is not declared!"));
    }
    else
    {
      constraint = make_or_constraint(alternatives);
    }
  }

  std::string print() const
  {
    return "id(" + value + ")";
  }
};

struct number_node: public type_check_node
{
  std::string value;

  number_node(const std::string& value_)
    : value(value_)
  {}

  void set_constraint(type_check_context& context)
  {
    sort = context.create_sort_variable();
    if (detail::is_pos(value))
    {
      constraint = make_subsort_constraint(sort_pos::pos(), sort);
    }
    else if (detail::is_nat(value))
    {
      constraint = make_subsort_constraint(sort_nat::nat(), sort);
    }
    else
    {
      throw mcrl2::runtime_error("unknown numeric string " + value);
    }
  }

  std::string print() const
  {
    return "number(" + value + ")";
  }
};

struct constant_node: public type_check_node
{
  std::string name;

  constant_node(const std::string& name_)
    : name(name_)
  {}

  void set_constraint(type_check_context& context)
  {
    sort = context.create_sort_variable();
    std::pair<sort_expression_list, sort_expression_list> p = context.find_matching_constants(name);
    std::vector<constraint_ptr> alternatives;
    for (const sort_expression& s: p.first + p.second)
    {
      alternatives.push_back(make_subsort_constraint(s, sort));
    }
    constraint = make_or_constraint(alternatives);
    set_children_constraints(context);
  }

  std::string print() const
  {
    return "constant(" + name + ")";
  }
};

struct true_node: public constant_node
{
  true_node()
    : constant_node("true")
  { }
};

struct false_node: public constant_node
{
  false_node()
    : constant_node("false")
  { }
};

struct empty_list_node: public constant_node
{
  empty_list_node()
    : constant_node("[]")
  { }

  void set_constraint(type_check_context& context)
  {
    untyped_sort_variable element_sort = context.create_sort_variable();
    sort = sort_list::list(element_sort);
    constraint = make_subsort_constraint(any_sort(), element_sort);
  }
};

struct empty_set_node: public constant_node
{
  empty_set_node()
    : constant_node("{}")
  { }

  void set_constraint(type_check_context& context)
  {
    untyped_sort_variable element_sort = context.create_sort_variable();
    sort = sort_set::set_(element_sort);
    constraint = make_subsort_constraint(any_sort(), element_sort);
  }
};

struct empty_bag_node: public constant_node
{
  empty_bag_node()
    : constant_node("{:}")
  { }

  void set_constraint(type_check_context& context)
  {
    untyped_sort_variable element_sort = context.create_sort_variable();
    sort = sort_bag::bag(element_sort);
    constraint = make_subsort_constraint(any_sort(), element_sort);
  }
};

struct list_enumeration_node: public type_check_node
{
  list_enumeration_node(const std::vector<type_check_node_ptr>& children)
    : type_check_node(children)
  {}

  void set_constraint(type_check_context& context)
  {
    set_children_constraints(context);
    untyped_sort_variable element_sort = context.create_sort_variable();
    sort = sort_list::list(element_sort);
    std::vector<constraint_ptr> alternatives;
    for (type_check_node_ptr child: children)
    {
      alternatives.push_back(make_subsort_constraint(element_sort, child->sort));
    }
    constraint = make_and_constraint(alternatives);
  }

  std::string print() const
  {
    return print_node_vector("list_enumeration", children);
  }
};

struct bag_enumeration_node: public type_check_node
{
  bag_enumeration_node(const std::vector<type_check_node_ptr>& children)
    : type_check_node(children)
  {}

  void set_constraint(type_check_context& context)
  {
    set_children_constraints(context);
    untyped_sort_variable element_sort = context.create_sort_variable();
    sort = sort_bag::bag(element_sort);
    std::vector<constraint_ptr> alternatives;
    for (std::size_t i = 0; i < children.size(); i++)
    {
      if (i % 2 == 0)
      {
        alternatives.push_back(make_subsort_constraint(element_sort, children[i]->sort));
      }
      else
      {
        alternatives.push_back(make_subsort_constraint(sort_nat::nat(), children[i]->sort));
      }
    }
    constraint = make_and_constraint(alternatives);
  }

  std::string print() const
  {
    return print_node_vector("bag_enumeration", children);
  }
};

struct set_enumeration_node: public type_check_node
{
  set_enumeration_node(const std::vector<type_check_node_ptr>& children)
    : type_check_node(children)
  {}

  void set_constraint(type_check_context& context)
  {
    set_children_constraints(context);
    untyped_sort_variable element_sort = context.create_sort_variable();
    sort = sort_set::set_(element_sort);
    std::vector<constraint_ptr> alternatives;
    for (type_check_node_ptr child: children)
    {
      alternatives.push_back(make_subsort_constraint(element_sort, child->sort));
    }
    constraint = make_and_constraint(alternatives);
  }

  std::string print() const
  {
    return print_node_vector("set_enumeration", children);
  }
};

struct bag_or_set_enumeration_node: public type_check_node
{
  variable v;

  bag_or_set_enumeration_node(const variable& v_, type_check_node_ptr x)
    : v(v_)
  {
    children.push_back(x);
  }

  void set_constraint(type_check_context& context)
  {
    context.add_context_variable(v);
    set_children_constraints(context);
    sort = context.create_sort_variable();
    auto element_sort = v.sort();
    constraint_ptr set_constraint = make_and_constraint({
      constraint_ptr(new is_sort_constraint(sort, sort_set::set_(element_sort))),
      constraint_ptr(new is_sort_constraint(children.front()->sort, sort_bool::bool_()))
    });
    constraint_ptr bag_constraint = make_and_constraint({
        constraint_ptr(new is_sort_constraint(sort, sort_bag::bag(element_sort))),
        make_subsort_constraint(sort_nat::nat(), children.front()->sort)
       });
    constraint = make_or_constraint({ set_constraint, bag_constraint });
    context.remove_context_variable(v);
  }

  std::string print() const
  {
    return print_node_vector("bag_or_set_enumeration", children);
  }
};

struct function_update_node: public type_check_node
{
  function_update_node(type_check_node_ptr x1, type_check_node_ptr x2, type_check_node_ptr x3)
  {
    children.push_back(x1);
    children.push_back(x2);
    children.push_back(x3);
  }

  std::string print() const
  {
    return print_node_vector("function_update", children);
  }
};

struct application_node: public type_check_node
{
  std::size_t arity;

  application_node(type_check_node_ptr head, const std::vector<type_check_node_ptr>& arguments)
    : arity(arguments.size())
  {
    children.push_back(head);
    for (auto arg: arguments)
    {
      children.push_back(arg);
    }
  }

  void set_constraint(type_check_context& context)
  {
    set_children_constraints(context);
    untyped_sort_variable codomain = context.create_sort_variable();
    std::vector<sort_expression> domain;
    for (std::size_t i = 0; i < arity; i++)
    {
      domain.push_back(children[i+1]->sort);
    }
    sort = codomain;

    std::vector<constraint_ptr> alternatives;
    alternatives.push_back(constraint_ptr(new is_sort_constraint(children[0]->sort, function_sort(sort_expression_list(domain.begin(), domain.end()), codomain))));

    // add missing constraints for if application
    // TODO: handle this is a more structured way
    if (children[0]->print() == "id(if)")
    {
      alternatives.push_back(make_subsort_constraint(codomain, children[1]->sort));
      alternatives.push_back(make_subsort_constraint(codomain, children[2]->sort));
    }

    constraint = make_and_constraint(alternatives);
  }

  std::string print() const
  {
    return print_node_vector("application", children);
  }
};

struct unary_operator_node: public type_check_node
{
  std::string name;

  unary_operator_node(const std::string& name_, type_check_node_ptr arg)
    : type_check_node({ arg }), name(name_)
  {}

  void set_constraint(type_check_context& context)
  {
    sort = context.create_sort_variable();
    std::pair<function_sort_list, function_sort_list> p = context.find_matching_functions(name, 1);
    std::vector<constraint_ptr> alternatives;
    for (const function_sort& s: p.first + p.second)
    {
      auto i = s.domain().begin();
      const sort_expression& first = *i;
      alternatives.push_back(make_and_constraint({
          make_subsort_constraint(children[0]->sort, first),
          make_subsort_constraint(s.codomain(), sort)
         })
      );
    }
    constraint = make_or_constraint(alternatives);
    set_children_constraints(context);
  }


  std::string print() const
  {
    std::ostringstream out;
    out << "unary_operator(" << name << ", " << children.front()->print() << ")";
    return out.str();
  }
};

struct forall_node: public type_check_node
{
  variable_list variables;

  forall_node(const variable_list& variables_, type_check_node_ptr arg)
    : type_check_node({ arg }), variables(variables_)
  {}

  void set_constraint(type_check_context& context)
  {
    context.add_context_variables(variables);
    sort = context.create_sort_variable();
    constraint = constraint_ptr(new is_sort_constraint(sort, sort_bool::bool_()));
    set_children_constraints(context);
    context.remove_context_variables(variables);
  }

  std::string print() const
  {
    std::ostringstream out;
    out << "forall(" << data::pp(variables) << ". " << children.front()->print() << ")";
    return out.str();
  }
};

struct exists_node: public type_check_node
{
  variable_list variables;

  exists_node(const variable_list& variables_, type_check_node_ptr arg)
    : type_check_node({ arg }), variables(variables_)
  {}

  void set_constraint(type_check_context& context)
  {
    context.add_context_variables(variables);
    sort = context.create_sort_variable();
    constraint = constraint_ptr(new is_sort_constraint(sort, sort_bool::bool_()));
    set_children_constraints(context);
    context.remove_context_variables(variables);
  }

  std::string print() const
  {
    std::ostringstream out;
    out << "exists(" << data::pp(variables) << ". " << children.front()->print() << ")";
    return out.str();
  }
};

struct lambda_node: public unary_operator_node
{
  variable_list variables;

  lambda_node(const variable_list& variables_, type_check_node_ptr arg)
    : unary_operator_node("lambda", arg), variables(variables_)
  {}

  void set_constraint(type_check_context& context)
  {
    context.add_context_variables(variables);
    set_children_constraints(context);
    sort = context.create_sort_variable();
    constraint = constraint_ptr(new subsort_constraint(any_sort(), sort));
    context.remove_context_variables(variables);
  }

  std::string print() const
  {
    std::ostringstream out;
    out << "lambda(" << data::pp(variables) << ". " << children.front()->print() << ")";
    return out.str();
  }
};

struct binary_operator_node: public type_check_node
{
  std::string name;

  binary_operator_node(const std::string& name_, type_check_node_ptr left, type_check_node_ptr right)
    : type_check_node({ left, right }), name(name_)
  {}

  void set_constraint(type_check_context& context)
  {
    set_children_constraints(context);
    sort = context.create_sort_variable();
    std::pair<function_sort_list, function_sort_list> p = context.find_matching_functions(name, 2);
    std::vector<constraint_ptr> alternatives;
    for (const function_sort& s: p.first + p.second)
    {
      auto i = s.domain().begin();
      const sort_expression& left = *i++;
      const sort_expression& right = *i;
      alternatives.push_back(make_and_constraint({
          make_subsort_constraint(children[0]->sort, left),
          make_subsort_constraint(children[1]->sort, right),
          make_subsort_constraint(s.codomain(), sort)
         })
      );
    }
    constraint = make_or_constraint(alternatives);
  }

  std::string print() const
  {
    std::ostringstream out;
    out << "binary_operator(" << name << ", " << children[0]->print() << ", " << children[1]->print() << ")";
    return out.str();
  }
};

struct where_clause_node: public type_check_node
{
  std::vector<std::string> variable_names;

  where_clause_node(type_check_node_ptr body, const std::vector<std::pair<std::string, type_check_node_ptr> >& assignments)
  {
    children.push_back(body);
    variable_vector v;
    for (const std::pair<std::string, type_check_node_ptr>& a: assignments)
    {
      variable_names.push_back(a.first);
      children.push_back(a.second);
    }
  }

  void set_constraint(type_check_context& context)
  {
    variable_list variables;
    for (const std::string& name: variable_names)
    {
      variables.push_front(variable(name, any_sort()));
    }
    context.add_context_variables(variables);
    set_children_constraints(context);
    sort = context.create_sort_variable();

    std::vector<constraint_ptr> constraints = { make_subsort_constraint(any_sort(), sort) };
    for (type_check_node_ptr child: children)
    {
      constraints.push_back(make_subsort_constraint(any_sort(), child->sort));
    }
    constraint = make_and_constraint(constraints);
    context.remove_context_variables(variables);
  }

  std::string print() const
  {
    std::ostringstream out;
    return print_node_vector("where_clause", children);
    return out.str();
  }
};

struct type_check_tree_generator: public data_expression_actions
{
  type_check_tree_generator(const core::parser& parser_)
    : data_expression_actions(parser_)
  {}

  std::vector<type_check_node_ptr> parse_DataExprList(const core::parse_node& node) const
  {
    return parse_vector<type_check_node_ptr>(node, "DataExpr", [&](const core::parse_node& node) { return parse_DataExpr(node); });
  }

  std::vector<type_check_node_ptr> parse_BagEnumEltList(const core::parse_node& node) const
  {
    return parse_DataExprList(node);
  }

  std::pair<std::string, type_check_node_ptr> parse_Assignment(const core::parse_node& node) const
  {
    return std::make_pair(parse_Id(node.child(0)), parse_DataExpr(node.child(2)));
  }

  std::vector<std::pair<std::string, type_check_node_ptr> > parse_AssignmentList(const core::parse_node& node) const
  {
    return parse_vector<std::pair<std::string, type_check_node_ptr> >(node, "Assignment", [&](const core::parse_node& node) { return parse_Assignment(node); });
  }

  type_check_node_ptr parse_DataExpr(const core::parse_node& node) const
  {
    assert(symbol_name(node) == "DataExpr");
    if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "Id")) { return type_check_node_ptr(new id_node(parse_Id(node.child(0)))); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "Number")) { return type_check_node_ptr(new number_node(parse_Number(node.child(0)))); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "true")) { return type_check_node_ptr(new true_node()); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "false")) { return type_check_node_ptr(new false_node()); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "[") && (symbol_name(node.child(1)) == "]")) { return type_check_node_ptr(new empty_list_node()); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "{") && (symbol_name(node.child(1)) == "}")) { return type_check_node_ptr(new empty_set_node()); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "{") && (symbol_name(node.child(1)) == ":") && (symbol_name(node.child(2)) == "}")) { return type_check_node_ptr(new empty_bag_node()); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "[") && (symbol_name(node.child(1)) == "DataExprList") && (symbol_name(node.child(2)) == "]")) { return type_check_node_ptr(new list_enumeration_node(parse_DataExprList(node.child(1)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "{") && (symbol_name(node.child(1)) == "BagEnumEltList") && (symbol_name(node.child(2)) == "}")) { return type_check_node_ptr(new bag_enumeration_node(parse_BagEnumEltList(node.child(1)))); }
    else if ((node.child_count() == 5) && (symbol_name(node.child(0)) == "{") && (symbol_name(node.child(1)) == "VarDecl") && (symbol_name(node.child(2)) == "|") && (symbol_name(node.child(3)) == "DataExpr") && (symbol_name(node.child(4)) == "}")) { return type_check_node_ptr(new bag_or_set_enumeration_node(parse_VarDecl(node.child(1)), parse_DataExpr(node.child(3)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "{") && (symbol_name(node.child(1)) == "DataExprList") && (symbol_name(node.child(2)) == "}")) { return type_check_node_ptr(new set_enumeration_node(parse_DataExprList(node.child(1)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "(") && (symbol_name(node.child(1)) == "DataExpr") && (symbol_name(node.child(2)) == ")")) { return parse_DataExpr(node.child(1)); }
    else if ((node.child_count() == 6) && (symbol_name(node.child(0)) == "DataExpr") && (symbol_name(node.child(1)) == "[") && (symbol_name(node.child(2)) == "DataExpr") && (symbol_name(node.child(3)) == "->") && (symbol_name(node.child(4)) == "DataExpr") && (symbol_name(node.child(5)) == "]")) { return type_check_node_ptr(new function_update_node(parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)), parse_DataExpr(node.child(4)))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "DataExpr") && (symbol_name(node.child(1)) == "(") && (symbol_name(node.child(2)) == "DataExprList") && (symbol_name(node.child(3)) == ")")) { return type_check_node_ptr(new application_node(parse_DataExpr(node.child(0)), parse_DataExprList(node.child(2)))); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "!") && (symbol_name(node.child(1)) == "DataExpr")) { return type_check_node_ptr(new unary_operator_node("!", parse_DataExpr(node.child(1)))); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "-") && (symbol_name(node.child(1)) == "DataExpr")) { return type_check_node_ptr(new unary_operator_node("-", parse_DataExpr(node.child(1)))); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "#") && (symbol_name(node.child(1)) == "DataExpr")) { return type_check_node_ptr(new unary_operator_node("#", parse_DataExpr(node.child(1)))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "forall") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "DataExpr")) { return type_check_node_ptr(new forall_node(parse_VarsDeclList(node.child(1)), parse_DataExpr(node.child(3)))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "exists") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "DataExpr")) { return type_check_node_ptr(new exists_node(parse_VarsDeclList(node.child(1)), parse_DataExpr(node.child(3)))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "lambda") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "DataExpr")) { return type_check_node_ptr(new lambda_node(parse_VarsDeclList(node.child(1)), parse_DataExpr(node.child(3)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "=>" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("=>" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "&&" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("&&" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "||" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("||" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "==" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("==" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "!=" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("!=" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "<"  ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("<"  , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "<=" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("<=" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == ">=" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node(">=" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == ">"  ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node(">"  , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "in" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("in" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "|>" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("|>" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "<|" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("<|" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "++" ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("++" , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "+"  ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("+"  , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "-"  ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("-"  , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "/"  ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("/"  , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "div") && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("div", parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "mod") && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("mod", parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "*"  ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("*"  , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "DataExpr") && (node.child(1).string() == "."  ) && (symbol_name(node.child(2)) == "DataExpr")) { return type_check_node_ptr(new binary_operator_node("."  , parse_DataExpr(node.child(0)), parse_DataExpr(node.child(2)))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "DataExpr") && (symbol_name(node.child(1)) == "whr") && (symbol_name(node.child(2)) == "AssignmentList") && (symbol_name(node.child(3)) == "end")) { return type_check_node_ptr(new where_clause_node({ parse_DataExpr(node.child(0)) }, parse_AssignmentList(node.child(2)))); }
    throw core::parse_node_unexpected_exception(m_parser, node);
  }
};

inline
std::pair<sort_expression_list, sort_expression_list> type_check_context::find_matching_constants(const std::string& name) const
{
  sort_expression_list system_result;
  sort_expression_list user_result;
  auto i = system_constants.find(core::identifier_string(name));
  if (i != system_constants.end())
  {
    system_result = i->second;
  }
  auto j = user_constants.find(core::identifier_string(name));
  if (j != user_constants.end())
  {
    user_result = atermpp::make_list<sort_expression>(j->second);
  }
  return std::make_pair(system_result, user_result);
}

inline
function_sort_list filter_sorts(const function_sort_list& sorts, std::size_t arity)
{
  std::vector<function_sort> result;
  for (const function_sort& sort: sorts)
  {
    if (sort.domain().size() == arity)
    {
      result.push_back(sort);
    }
  }
  return function_sort_list(result.begin(), result.end());
}

inline
std::pair<function_sort_list, function_sort_list> type_check_context::find_matching_functions(const std::string& name, std::size_t arity) const
{
  function_sort_list system_result;
  function_sort_list user_result;
  auto i = system_functions.find(core::identifier_string(name));
  if (i != system_functions.end())
  {
    system_result = filter_sorts(i->second, arity);
  }
  auto j = user_functions.find(core::identifier_string(name));
  if (j != user_functions.end())
  {
    user_result = filter_sorts(j->second, arity);
  }
  return std::make_pair(system_result, user_result);
}

inline
std::pair<function_sort_list, function_sort_list> type_check_context::find_matching_functions(const std::string& name) const
{
  function_sort_list system_result;
  function_sort_list user_result;
  auto i = system_functions.find(core::identifier_string(name));
  if (i != system_functions.end())
  {
    system_result = i->second;
  }
  auto j = user_functions.find(core::identifier_string(name));
  if (j != user_functions.end())
  {
    user_result = j->second;
  }
  return std::make_pair(system_result, user_result);
}

inline
std::vector<sort_expression> type_check_context::find_matching_variables(const std::string& name) const
{
  std::vector<sort_expression> result;
  auto i = declared_variables.find(core::identifier_string(name));
  if (i != declared_variables.end())
  {
    result.push_back(i->second.back());
  }
  return result;
}

inline
void print_node(type_check_node_ptr node)
{
  std::cout << "\n-----------------------------" << std::endl;
  std::cout << "node = " << node->print() << std::endl;
  std::cout << "sort = " << node->sort << std::endl;
  std::cout << "constraint = " << node->constraint->print() << std::endl;
  for (type_check_node_ptr child: node->children)
  {
    print_node(child);
  }
}

// TODO: This design is ugly, but for the moment it seems the easiest solution to modify
// the constraint tree
inline
constraint_ptr expand_constraint(constraint_ptr p)
{
  {
    or_constraint* x = dynamic_cast<or_constraint*>(p.get());
    if (x)
    {
      for (constraint_ptr& q: x->alternatives)
      {
        q = expand_constraint(q);
      }
      return p;
    }
  }
  {
    and_constraint* x = dynamic_cast<and_constraint*>(p.get());
    if (x)
    {
      for (constraint_ptr& q: x->alternatives)
      {
        q = expand_constraint(q);
      }
      return p;
    }
  }
  {
    subsort_constraint* x = dynamic_cast<subsort_constraint*>(p.get());
    if (x)
    {
      const sort_expression& s1 = x->s1;
      const sort_expression& s2 = x->s2;
      if (s1 == sort_bool::bool_())
      {
        return make_is_sort_constraint(s2, s1);
      }
      if (is_container_sort(s1) && is_container_sort(s2))
      {
        const container_sort& c1 = atermpp::down_cast<container_sort>(s1);
        const container_sort& c2 = atermpp::down_cast<container_sort>(s2);
        if (c1.container_name() == c2.container_name())
        {
          return make_subsort_constraint(c1.element_sort(), c2.element_sort());
        }
      }
      return p;
    }
  }
  return p;
}

} // namespace data

} // namespace mcrl2

#endif // MCRL2_DATA_TYPE_CHECK_TREE_H
