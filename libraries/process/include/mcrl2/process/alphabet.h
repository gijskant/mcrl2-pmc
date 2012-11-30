// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/process/alphabet.h
/// \brief add your file description here.

#ifndef MCRL2_PROCESS_ALPHABET_H
#define MCRL2_PROCESS_ALPHABET_H

#include <algorithm>
#include <iterator>
#include <iostream>
#include <sstream>
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>
#include "mcrl2/data/substitutions.h"
#include "mcrl2/process/detail/allow_set.h"
#include "mcrl2/process/find.h"
#include "mcrl2/process/replace.h"
#include "mcrl2/process/builder.h"
#include "mcrl2/process/traverser.h"
#include "mcrl2/process/utility.h"
#include "mcrl2/utilities/logger.h"

namespace mcrl2 {

namespace process {

multi_action_name_set alphabet(const process_expression& x, const atermpp::vector<process_equation>& equations);

namespace detail {

inline
bool contains_tau(const multi_action_name_set& A)
{
  multi_action_name tau;
  return A.find(tau) != A.end();
}

inline
process_expression make_sync(const process_expression x, const process_expression& y)
{
  if (is_delta(x) && is_delta(y))
  {
    return delta();
  }
  return sync(x, y);
}

inline
process_expression make_merge(const process_expression x, const process_expression& y)
{
  if (is_delta(x) && is_delta(y))
  {
    return delta();
  }
  return merge(x, y);
}

inline
process_expression make_left_merge(const process_expression x, const process_expression& y)
{
  if (is_delta(x) && is_delta(y))
  {
    return delta();
  }
  return left_merge(x, y);
}

inline
process_expression make_allow(const multi_action_name_set& A, const process_expression& x)
{
  if (A.empty())
  {
    return delta();
  }

  // convert A to an action_name_multiset_list B
  atermpp::vector<action_name_multiset> v;
  for (multi_action_name_set::const_iterator i = A.begin(); i != A.end(); ++i)
  {
    const multi_action_name& alpha = *i;
    if (!i->empty()) // exclude tau
    {
      v.push_back(action_name_multiset(core::identifier_string_list(alpha.begin(), alpha.end())));
    }
  }
  action_name_multiset_list B(v.begin(), v.end());

  return B.empty() ? x : allow(B, x);
}

inline
process_expression make_comm(const communication_expression_list& C, const process_expression& x)
{
  if (C.empty())
  {
    return x;
  }
  else
  {
    return comm(C, x);
  }
}

struct alphabet_node
{
  multi_action_name_set alphabet;

  alphabet_node(const multi_action_name_set& alphabet_)
    : alphabet(alphabet_)
  {}
};

inline
std::ostream& operator<<(std::ostream& out, const alphabet_node& x)
{
  return out << "alphabet = " << lps::pp(x.alphabet);
}

/// \brief Traverser that computes the alphabet of process expressions
template <typename Derived, typename Node = alphabet_node>
struct alphabet_traverser: public process_expression_traverser<Derived>
{
  typedef process_expression_traverser<Derived> super;
  using super::enter;
  using super::leave;
  using super::operator();

#if BOOST_MSVC
#include "mcrl2/core/detail/traverser_msvc.inc.h"
#endif

  Derived& derived()
  {
    return static_cast<Derived&>(*this);
  }

  const atermpp::vector<process_equation>& equations;
  std::set<process_identifier>& W;
  atermpp::vector<Node> node_stack;

  alphabet_traverser(const atermpp::vector<process_equation>& equations_, std::set<process_identifier>& W_)
    : equations(equations_), W(W_)
  {}

  // Push a node to node_stack
  void push(const Node& node)
  {
    mCRL2log(log::debug1) << "<push> A = " << lps::pp(node.alphabet) << std::endl;
    node_stack.push_back(node);
  }

  // Push A to node_stack
  void push(const multi_action_name_set& A)
  {
    push(Node(A));
  }

  // Pop the top element of node_stack and return it
  Node pop()
  {
    Node result = node_stack.back();
    mCRL2log(log::debug1) << "<pop> A = " << lps::pp(result.alphabet) << std::endl;
    node_stack.pop_back();
    return result;
  }

  // Return the top element of node_stack
  Node& top()
  {
    return node_stack.back();
  }

  // Return the top element of node_stack
  const Node& top() const
  {
    return node_stack.back();
  }

  // Pops two elements A1 and A2 from the stack, and pushes back union(A1, A2)
  void join()
  {
    Node right = pop();
    Node left = pop();
    push(set_union(left.alphabet, right.alphabet));
  }

  // Pops two elements A1 and A2 from the stack, and pushes back union(A1, A2, A1 | A2)
  void join_merge()
  {
    Node right = pop();
    Node left = pop();
    push(alphabet_operations::merge(left.alphabet, right.alphabet));
  }

  // Pops two elements A1 and A2 from the stack, and pushes back A1 | A2
  void join_sync()
  {
    Node right = pop();
    Node left = pop();
    push(alphabet_operations::sync(left.alphabet, right.alphabet));
  }

  void leave(const lps::action& x)
  {
    multi_action_name alpha;
    alpha.insert(x.label().name());
    multi_action_name_set A;
    A.insert(alpha);
    push(A);
  }

  void leave(const process::process_instance& x)
  {
    if (W.find(x.identifier()) == W.end())
    {
      W.insert(x.identifier());
      const process_equation& eqn = find_equation(equations, x.identifier());
      derived()(eqn.expression());
      W.erase(x.identifier());
    }
    else
    {
      push(multi_action_name_set());
    }
  }

  void leave(const process::process_instance_assignment& x)
  {
    if (W.find(x.identifier()) == W.end())
    {
      W.insert(x.identifier());
      const process_equation& eqn = find_equation(equations, x.identifier());
      derived()(eqn.expression());
      W.erase(x.identifier());
    }
    else
    {
      push(multi_action_name_set());
    }
  }

  void leave(const process::delta& x)
  {
    push(multi_action_name_set());
  }

  void leave(const process::tau& x)
  {
    multi_action_name_set A;
    A.insert(multi_action_name()); // A = { tau }
    push(A);
  }

  void leave(const process::sum& x)
  {
  }

  void leave(const process::block& x)
  {
    top().alphabet = alphabet_operations::block(x.block_set(), top().alphabet);
  }

  void leave(const process::hide& x)
  {
    top().alphabet = alphabet_operations::hide(x.hide_set(), top().alphabet);
  }

  void leave(const process::rename& x)
  {
    top().alphabet = alphabet_operations::rename(x.rename_set(), top().alphabet);
  }

  void leave(const process::comm& x)
  {
    top().alphabet = alphabet_operations::comm(x.comm_set(), top().alphabet);
  }

  void leave(const process::allow& x)
  {
    top().alphabet = alphabet_operations::allow(x.allow_set(), top().alphabet);
  }

  void leave(const process::sync& x)
  {
    join_sync();
  }

  void leave(const process::at& x)
  {
  }

  void leave(const process::seq& x)
  {
    join();
  }

  void leave(const process::if_then& x)
  {
  }

  void leave(const process::if_then_else& x)
  {
    join();
  }

  void leave(const process::bounded_init& x)
  {
    join();
  }

  void leave(const process::merge& x)
  {
    join_merge();
  }

  void leave(const process::left_merge& x)
  {
    join_merge();
  }

  void leave(const process::choice& x)
  {
    join();
  }
};

struct apply_alphabet_traverser: public alphabet_traverser<apply_alphabet_traverser>
{
  typedef alphabet_traverser<apply_alphabet_traverser> super;
  using super::enter;
  using super::leave;
  using super::operator();
  using super::node_stack;

#if BOOST_MSVC
#include "mcrl2/core/detail/traverser_msvc.inc.h"
#endif

  apply_alphabet_traverser(const atermpp::vector<process_equation>& equations, std::set<process_identifier>& W)
    : super(equations, W)
  {}
};

inline
alphabet_node alphabet(const process_expression& x, const atermpp::vector<process_equation>& equations, std::set<process_identifier>& W)
{
  apply_alphabet_traverser f(equations, W);
  f(x);
  return f.node_stack.back();
}

struct push_allow_node: public alphabet_node
{
  push_allow_node(const multi_action_name_set& alphabet, const process_expression& expression = process_expression(), boost::logic::tribool needs_allow = boost::logic::indeterminate)
    : alphabet_node(alphabet), m_expression(expression), m_needs_allow(needs_allow)
  {}

  std::string print(const allow_set& A) const
  {
    std::ostringstream out;
    if (m_needs_allow)
    {
      out << "allow(" << lps::pp(set_intersection(alphabet, A.A, A.A_includes_subsets)) << ", " << process::pp(m_expression) << ")";
    }
    else if (!m_needs_allow)
    {
      out << process::pp(m_expression);
    }
    else
    {
      out << "allow?(" << lps::pp(set_intersection(alphabet, A.A, A.A_includes_subsets)) << ", " << process::pp(m_expression) << ")";
    }
    return out.str();
  }

  void finish(const atermpp::vector<process_equation>& equations, const allow_set& A)
  {
    // alphabet = process::alphabet(m_expression, equations);
    if (boost::logic::indeterminate(m_needs_allow))
    {
      std::size_t alphabet_size = alphabet.size();
      alphabet = A.intersect(alphabet);
      m_needs_allow = alphabet.size() < alphabet_size;
    }

    if (m_needs_allow)
    {
      if (alphabet.size() == 1 && contains_tau(alphabet)) // alphabet == { tau }
      {
        // N.B. This is a tricky case. We can't return allow({tau}, m_expression),
        // as this is not allowed in mCRL2. We can take an arbitrary element of
        // A instead.
        multi_action_name_set A1;
        A1.insert(*A.A.begin());
        m_expression = make_allow(A1, m_expression);
      }
      else
      {
        m_expression = make_allow(alphabet, m_expression);
      }
    }
    m_needs_allow = false;
  }

  process_expression m_expression;

  // if true, the alphabet of this node is included in the allow set A
  boost::logic::tribool m_needs_allow;
};

inline
std::ostream& operator<<(std::ostream& out, const push_allow_node& x)
{
  return out << "alphabet = " << lps::pp(x.alphabet) << " expression = " << process::pp(x.m_expression) << " needs_allow = " << std::boolalpha << x.m_needs_allow << std::endl;
}

push_allow_node push_allow(const process_expression& x, const allow_set& A, const atermpp::vector<process_equation>& equations);

template <typename Derived, typename Node = push_allow_node>
struct push_allow_traverser: public process_expression_traverser<Derived>
{
  typedef process_expression_traverser<Derived> super;
  using super::enter;
  using super::leave;
  using super::operator();

#if BOOST_MSVC
#include "mcrl2/core/detail/traverser_msvc.inc.h"
#endif

  // used for computing the alphabet
  const atermpp::vector<process_equation>& equations;

  // the parameter A
  const allow_set& A;

  atermpp::vector<Node> node_stack;

  push_allow_traverser(const atermpp::vector<process_equation>& equations_, const allow_set& A_)
    : equations(equations_), A(A_)
  {}

  Derived& derived()
  {
    return static_cast<Derived&>(*this);
  }

  // Push a node to node_stack
  void push(const Node& node)
  {
    node_stack.push_back(node);
  }

  // Pop the top element of node_stack and return it
  Node pop()
  {
    Node result = node_stack.back();
    node_stack.pop_back();
    return result;
  }

  // Return the top element of node_stack
  Node& top()
  {
    return node_stack.back();
  }

  // Return the top element of node_stack
  const Node& top() const
  {
    return node_stack.back();
  }

  void log_push_result(const process_expression& x, const allow_set& A, const push_allow_node& result, const std::string& msg = "", const std::string& text = "")
  {
    std::string text1 = text;
    if (!text1.empty())
    {
      text1 = text1 + " = ";
    }
    push_allow_node result1 = result;
    result1.finish(equations, A);
    mCRL2log(log::debug) << msg << "push(" << A << ", " << process::pp(x) << ") = "
      << text1
      << result1.print(A) << " with alphabet(" << process::pp(result.m_expression) << ") = " << lps::pp(result.alphabet) << std::endl;
  }

  void log(const process_expression& x, const std::string& text = "")
  {
    log_push_result(x, A, top(), "", text);
  }

  bool is_pcrl(const process_expression& x) const
  {
    return !is_merge(x) && !is_left_merge(x) && !is_sync(x) && !is_hide(x) && !is_rename(x) && !is_block(x) && !is_allow(x) && !is_comm(x);
  }

  void leave_pcrl(const process_expression& x)
  {
    push(push_allow_node(process::alphabet(x, equations), x, boost::logic::indeterminate));
    log(x);
  }

  void operator()(const lps::action& x)
  {
    leave_pcrl(x);
  }


  void operator()(const process::process_instance& x)
  {
    const process_equation& eqn = find_equation(equations, x.identifier());
    process_expression p = eqn.expression();
    data::mutable_map_substitution<> sigma;
    data::variable_list d = eqn.formal_parameters();
    data::data_expression_list e = x.actual_parameters();
    data::variable_list::iterator di = d.begin();
    data::data_expression_list::iterator ei = e.begin();
    for (; di != d.end(); ++di, ++ei)
    {
      sigma[*di] = *ei;
    }
    p = process::replace_free_variables(p, sigma);
    derived()(p);
    if (is_pcrl(eqn.expression()))
    {
      top().m_expression = x;
    }
  }

  void operator()(const process::process_instance_assignment& x)
  {
    const process_equation& eqn = find_equation(equations, x.identifier());
    process_expression p = eqn.expression();
    data::mutable_map_substitution<> sigma;
    data::assignment_list a = x.assignments();
    for (data::assignment_list::iterator i = a.begin(); i != a.end(); ++i)
    {
      sigma[i->lhs()] = i->rhs();
    }
    p = process::replace_free_variables(p, sigma);
    derived()(p);
    if (is_pcrl(eqn.expression()))
    {
      top().m_expression = x;
    }
  }

  void operator()(const process::delta& x)
  {
    leave_pcrl(x);
  }

  void operator()(const process::tau& x)
  {
    leave_pcrl(x);
  }

  void operator()(const process::sum& x)
  {
    leave_pcrl(x);
  }

  void operator()(const process::at& x)
  {
    leave_pcrl(x);
  }

  void operator()(const process::seq& x)
  {
    leave_pcrl(x);
  }

  void operator()(const process::if_then& x)
  {
    leave_pcrl(x);
  }

  void operator()(const process::if_then_else& x)
  {
    leave_pcrl(x);
  }

  void operator()(const process::bounded_init& x)
  {
    leave_pcrl(x);
  }

  void operator()(const process::choice& x)
  {
    leave_pcrl(x);
  }

  std::string log_hide(const process::hide& x, const allow_set& A1)
  {
    std::ostringstream out;
    out << "hide({" << core::pp(x.hide_set()) << "}, push(" << A1 << ", " << process::pp(x.operand()) << "))";
    return out.str();
  }

  void operator()(const process::hide& x)
  {
    core::identifier_string_list I = x.hide_set();
    allow_set A1 = allow_set_operations::hide_inverse(I, A);
    push_allow_node node = push_allow(x.operand(), A1, equations);
    node.m_expression = process::hide(I, node.m_expression);
    push(node);
    log(x, log_hide(x, A1));
  }

  std::string log_block(const process::block& x, const allow_set& A1)
  {
    std::ostringstream out;
    out << "block({" << core::pp(x.block_set()) << "}, push(" << A1 << ", " << process::pp(x.operand()) << "))";
    return out.str();
  }

  void operator()(const process::block& x)
  {
    core::identifier_string_list B = x.block_set();
    allow_set A1 = allow_set_operations::block(B, A);
    push_allow_node node = push_allow(x.operand(), A1, equations);
    push(node);
    log(x, log_block(x, A1));
  }

  std::string log_rename(const process::rename& x, const allow_set& A1)
  {
    std::ostringstream out;
    out << "rename({" << process::pp(x.rename_set()) << "}, push(" << A1 << ", " << process::pp(x.operand()) << "))";
    return out.str();
  }

  void operator()(const process::rename& x)
  {
    rename_expression_list R = x.rename_set();
    allow_set A1 = allow_set_operations::rename_inverse(R, A);
    push_allow_node node = push_allow(x.operand(), A1, equations);
    node.m_expression = process::rename(R, node.m_expression);
    push(node);
    log(x, log_rename(x, A1));
  }

  std::string log_comm(const process::comm& x, const allow_set& A1)
  {
    std::ostringstream out;
    out << "comm({" << process::pp(x.comm_set()) << "}, push(" << A1 << ", " << process::pp(x.operand()) << "))";
    return out.str();
  }

  void operator()(const process::comm& x)
  {
    communication_expression_list C = x.comm_set();
    allow_set A1 = allow_set_operations::comm_inverse(C, A);
    push_allow_node node = push_allow(x.operand(), A1, equations);
    communication_expression_list C1 = filter_comm_set(x.comm_set(), node.alphabet);
    push(push_allow_node(alphabet_operations::comm(C1, node.alphabet), make_comm(C1, node.m_expression), boost::logic::indeterminate));
    log(x, log_comm(x, A1));
  }

  std::string log_allow(const process::allow& x, const allow_set& A1)
  {
    std::ostringstream out;
    out << "allow({" << process::pp(x.allow_set()) << "}, push(" << A1 << ", " << process::pp(x.operand()) << "))";
    return out.str();
  }

  void operator()(const process::allow& x)
  {
    action_name_multiset_list V = x.allow_set();
    allow_set A1 = allow_set_operations::allow(V, A);
    push_allow_node node = push_allow(x.operand(), A1, equations);
    push(node);
    log(x, log_allow(x, A1));
  }

  std::string log_merge(const process::merge& x, const allow_set& A_sub, const allow_set& A_arrow)
  {
    std::ostringstream out;
    out << "merge(push(" << A_sub << ", " << process::pp(x.left()) << "), push(" << A_arrow << ", " << process::pp(x.right()) << "))";
    return out.str();
  }

  void operator()(const process::merge& x)
  {
    allow_set A_sub = allow_set_operations::subsets(A);
    push_allow_node p1 = push_allow(x.left(), A_sub, equations);
    allow_set A_arrow = allow_set_operations::left_arrow(A, p1.alphabet);
    push_allow_node q1 = push_allow(x.right(), A_arrow, equations);
    push(push_allow_node(alphabet_operations::merge(p1.alphabet, q1.alphabet), make_merge(p1.m_expression, q1.m_expression), boost::logic::indeterminate));
    log(x, log_merge(x, A_sub, A_arrow));
  }

  std::string log_left_merge(const process::left_merge& x, const allow_set& A_sub, const allow_set& A_arrow)
  {
    std::ostringstream out;
    out << "left_merge(push(" << A_sub << ", " << process::pp(x.left()) << "), push(" << A_arrow << ", " << process::pp(x.right()) << "))";
    return out.str();
  }

  void operator()(const process::left_merge& x)
  {
    allow_set A_sub = allow_set_operations::subsets(A);
    push_allow_node p1 = push_allow(x.left(), A_sub, equations);
    allow_set A_arrow = allow_set_operations::left_arrow(A, p1.alphabet);
    push_allow_node q1 = push_allow(x.right(), A_arrow, equations);
    push(push_allow_node(alphabet_operations::left_merge(p1.alphabet, q1.alphabet), make_left_merge(p1.m_expression, q1.m_expression), boost::logic::indeterminate));
    log(x, log_left_merge(x, A_sub, A_arrow));
  }

  std::string log_sync(const process::left_merge& x, const allow_set& A_sub, const allow_set& A_arrow)
  {
    std::ostringstream out;
    out << "sync(push(" << A_sub << ", " << process::pp(x.left()) << "), push(" << A_arrow << ", " << process::pp(x.right()) << "))";
    return out.str();
  }

  void operator()(const process::sync& x)
  {
    allow_set A_sub = allow_set_operations::subsets(A);
    push_allow_node p1 = push_allow(x.left(), A_sub, equations);
    allow_set A_arrow = allow_set_operations::left_arrow(A, p1.alphabet);
    push_allow_node q1 = push_allow(x.right(), A_arrow, equations);
    push(push_allow_node(alphabet_operations::sync(p1.alphabet, q1.alphabet), make_sync(p1.m_expression, q1.m_expression), boost::logic::indeterminate));
    log(x, log_sync(x, A_sub, A_arrow));
  }
};

template <template <class, class> class Traverser, typename Node = push_allow_node>
struct apply_push_allow_traverser: public Traverser<apply_push_allow_traverser<Traverser, Node>, Node>
{
  typedef Traverser<apply_push_allow_traverser<Traverser, Node>, Node> super;
  using super::enter;
  using super::leave;
  using super::operator();

  apply_push_allow_traverser(const atermpp::vector<process_equation>& equations, const allow_set& A)
    : super(equations, A)
  {}

#ifdef BOOST_MSVC
#include "mcrl2/core/detail/traverser_msvc.inc.h"
#endif
};

inline
push_allow_node push_allow(const process_expression& x, const allow_set& A, const atermpp::vector<process_equation>& equations)
{
  apply_push_allow_traverser<push_allow_traverser> f(equations, A);
  f(x);
  f.node_stack.back().finish(equations, A);
  return f.node_stack.back();
}

struct alphabet_reduce_builder: public process_expression_builder<alphabet_reduce_builder>
{
  typedef process_expression_builder<alphabet_reduce_builder> super;
  using super::enter;
  using super::leave;
  using super::operator();

#if BOOST_MSVC
#include "mcrl2/core/detail/traverser_msvc.inc.h"
#endif

  const atermpp::vector<process_equation>& equations;

  alphabet_reduce_builder(const atermpp::vector<process_equation>& equations_)
    : equations(equations_)
  {}

  process_expression operator()(const process::allow& x)
  {
    allow_set A(make_name_set(x.allow_set()));
    push_allow_node node = push_allow(x.operand(), A, equations);
    return node.m_expression;
  }
};

inline
process_expression alphabet_reduce(const process_expression& x, const atermpp::vector<process_equation>& equations)
{
  alphabet_reduce_builder f(equations);
  return f(x);
}

} // detail

inline
multi_action_name_set alphabet(const process_expression& x, const atermpp::vector<process_equation>& equations)
{
  std::set<process_identifier> W;
  return detail::alphabet(x, equations, W).alphabet;
}

inline
void alphabet_reduce(process_specification& procspec)
{
  procspec.init() = detail::alphabet_reduce(procspec.init(), procspec.equations());
}

} // namespace process

} // namespace mcrl2

#endif // MCRL2_PROCESS_ALPHABET_H
