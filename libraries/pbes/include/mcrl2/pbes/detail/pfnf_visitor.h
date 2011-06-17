// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/detail/pfnf_visitor.h
/// \brief add your file description here.

#ifndef MCRL2_PBES_DETAIL_PFNF_VISITOR_H
#define MCRL2_PBES_DETAIL_PFNF_VISITOR_H

#include <algorithm>
#include <iterator>
#include <numeric>
#include <utility>
#include <vector>
#include <stdexcept>
#include <set>
#include <sstream>
#include <functional>
#include "mcrl2/atermpp/aterm_traits.h"
#include "mcrl2/atermpp/convert.h"
#include "mcrl2/utilities/optimized_boolean_operators.h"
#include "mcrl2/core/detail/print_utility.h"
#include "mcrl2/data/set_identifier_generator.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/pbes/replace.h"
#include "mcrl2/pbes/pbes_expression_visitor.h"
#include "mcrl2/pbes/pbes_expression.h"

namespace mcrl2
{

namespace pbes_system
{

namespace detail
{

/// \brief Represents a quantifier Qv:V. If the bool is true it is a forall, otherwise an exists.
typedef std::pair<bool, data::variable_list> pfnf_visitor_quantifier;

struct variable_variable_substitution: public std::unary_function<data::variable, data::variable>
{
  atermpp::map<data::variable, data::variable> sigma;

  data::variable operator()(const data::variable& v) const
  {
    atermpp::map<data::variable, data::variable>::const_iterator i = sigma.find(v);
    if (i == sigma.end())
    {
      return v;
    }
    return i->second;
  }

  data::variable_list operator()(const data::variable_list& v) const
  {
    atermpp::vector<data::variable> result;
    for (data::variable_list::const_iterator i = v.begin(); i != v.end(); ++i)
    {
      result.push_back((*this)(*i));
    }
    return atermpp::convert<data::variable_list>(result);
  }
  
  std::string to_string() const
  {
    std::ostringstream out;
    out << "[";
    for (atermpp::map<data::variable, data::variable>::const_iterator i = sigma.begin(); i != sigma.end(); ++i)
    {
      if (i != sigma.begin())
      {
        out << ", ";
      }
      out << core::pp(i->first) << " := " << core::pp(i->second);
    }
    out << "]";
    return out.str();
  }
};

struct variable_data_expression_substitution: public std::unary_function<data::variable, data::data_expression>
{
  typedef data::variable variable_type;
  typedef data::data_expression expression_type;

  const variable_variable_substitution& sigma;

  variable_data_expression_substitution(const variable_variable_substitution& sigma_)
    : sigma(sigma_)
  {}

  data::data_expression operator()(const data::variable& v) const
  {
    return sigma(v);
  }
};

/// \brief Represents the implication g => ( X0(e0) \/ ... \/ Xk(ek) )
struct pfnf_visitor_implication
{
  pbes_expression g;
  atermpp::vector<propositional_variable_instantiation> rhs;

  pfnf_visitor_implication(const atermpp::aterm_appl& g_, const atermpp::vector<propositional_variable_instantiation>& rhs_)
    : g(g_),
      rhs(rhs_)
  {}

  pfnf_visitor_implication(const atermpp::aterm_appl& x)
    : g(x)
  {}

  // applies a substitution to variables
  void substitute(const variable_variable_substitution& sigma)
  {
    for (atermpp::vector<propositional_variable_instantiation>::iterator i = rhs.begin(); i != rhs.end(); ++i)
    {
      *i = pbes_system::replace_free_variables(*i, variable_data_expression_substitution(sigma));
    }
    g = pbes_system::replace_free_variables(g, variable_data_expression_substitution(sigma));
  }
  
  void mark() const
  {
  	g.mark();
  }
  
  void protect() const
  {
  	g.protect();
  }
  
  void unprotect() const
  {
  	g.unprotect();
  }
};

struct pfnf_visitor_expression
{ 
	pbes_expression expr;
  atermpp::vector<pfnf_visitor_quantifier> quantifiers;
  atermpp::vector<pfnf_visitor_implication> implications;

  pfnf_visitor_expression(const atermpp::aterm_appl& x, const atermpp::vector<pfnf_visitor_quantifier>& quantifiers_, const atermpp::vector<pfnf_visitor_implication>& implications_)
    : expr(x),
      quantifiers(quantifiers_),
      implications(implications_)
  {}

  pfnf_visitor_expression(const atermpp::aterm_appl& x)
    : expr(x)
  {}

  // applies a substitution to variables
  void substitute(const variable_variable_substitution& sigma)
  {
    for (atermpp::vector<pfnf_visitor_quantifier>::iterator i = quantifiers.begin(); i != quantifiers.end(); ++i)
    {
      i->second = sigma(i->second);
    }
    for (atermpp::vector<pfnf_visitor_implication>::iterator i = implications.begin(); i != implications.end(); ++i)
    {
      i->substitute(sigma);
    }
    expr = pbes_system::replace_free_variables(expr, variable_data_expression_substitution(sigma));
  }

  void mark() const
  {
  	expr.mark();
  }
  
  void protect() const
  {
  	expr.protect();
  }
  
  void unprotect() const
  {
  	expr.unprotect();
  }
};

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

namespace atermpp
{

template<>
struct aterm_traits<mcrl2::pbes_system::detail::pfnf_visitor_quantifier>
{
  static void protect(const mcrl2::pbes_system::detail::pfnf_visitor_quantifier& t)
  {
    t.second.protect();
  }

  static void unprotect(const mcrl2::pbes_system::detail::pfnf_visitor_quantifier& t)
  {
    t.second.unprotect();
  }

  static void mark(const mcrl2::pbes_system::detail::pfnf_visitor_quantifier& t)
  {
    t.second.mark();
  }
};

template<>
struct aterm_traits<mcrl2::pbes_system::detail::pfnf_visitor_implication>
{
  static void protect(const mcrl2::pbes_system::detail::pfnf_visitor_implication& t)
  {
    t.protect();
  }

  static void unprotect(const mcrl2::pbes_system::detail::pfnf_visitor_implication& t)
  {
    t.unprotect();
  }

  static void mark(const mcrl2::pbes_system::detail::pfnf_visitor_implication& t)
  {
    t.mark();
  }
};

template<>
struct aterm_traits<mcrl2::pbes_system::detail::pfnf_visitor_expression>
{
  static void protect(const mcrl2::pbes_system::detail::pfnf_visitor_expression& t)
  {
    t.protect();
  }

  static void unprotect(const mcrl2::pbes_system::detail::pfnf_visitor_expression& t)
  {
    t.unprotect();
  }

  static void mark(const mcrl2::pbes_system::detail::pfnf_visitor_expression& t)
  {
    t.mark();
  }
};

} // namespace atermpp

namespace mcrl2
{

namespace pbes_system
{

namespace detail
{

/// \brief Concatenates two containers
/// \param x A container
/// \param y A container
/// \return The concatenation of x and y
template <typename Container>
Container concat(const Container& x, const Container& y)
{
  Container result = x;
  result.insert(result.end(), y.begin(), y.end());
  return result;
}

struct pfnf_visitor: public pbes_expression_visitor<pbes_expression>
{
    typedef pfnf_visitor self;
    typedef pbes_expression_visitor<pbes_expression> super;
    typedef core::term_traits<pbes_expression> tr;

    // makes sure there are no name clashes between quantifier variables in left and right
    // TODO: the efficiency can be increased by maintaining some additional data structures
    void resolve_name_clashes(pfnf_visitor_expression& left, pfnf_visitor_expression& right)
    {
      std::set<data::variable> left_variables;
      std::set<data::variable> name_clashes;
      for (atermpp::vector<pfnf_visitor_quantifier>::const_iterator i = left.quantifiers.begin(); i != left.quantifiers.end(); ++i)
      {
        left_variables.insert(i->second.begin(), i->second.end());
      }
      for (atermpp::vector<pfnf_visitor_quantifier>::const_iterator j = right.quantifiers.begin(); j != right.quantifiers.end(); ++j)
      {
        std::set_intersection(left_variables.begin(), left_variables.end(), j->second.begin(), j->second.end(), std::inserter(name_clashes, name_clashes.end()));
      }
#ifdef MCRL2_PFNF_VISITOR_DEBUG
std::cout << "NAME CLASHES: " << core::detail::print_pp_set(name_clashes) << std::endl;
#endif
      if (!name_clashes.empty())
      {
        data::set_identifier_generator generator;
        for (std::set<data::variable>::const_iterator i = left_variables.begin(); i != left_variables.end(); ++i)
        {
          generator.add_identifier(i->name());
        }
        variable_variable_substitution sigma;
        for (std::set<data::variable>::iterator i = name_clashes.begin(); i != name_clashes.end(); ++i)
        {
          sigma.sigma[*i] = data::variable(generator(std::string(i->name())), i->sort());
        }
#ifdef MCRL2_PFNF_VISITOR_DEBUG
std::cout << "LEFT\n"; print_expression(left);
std::cout << "RIGHT BEFORE\n"; print_expression(right);
std::cout << "SIGMA = " << sigma.to_string() << std::endl;
#endif
        right.substitute(sigma);
#ifdef MCRL2_PFNF_VISITOR_DEBUG
std::cout << "RIGHT AFTER\n"; print_expression(right);
#endif
      }
    }

    pbes_expression and_(const pfnf_visitor_expression& left, const pfnf_visitor_expression& right) const
    {
      return utilities::optimized_and(left.expr, right.expr);
    }

    pbes_expression or_(const pfnf_visitor_expression& left, const pfnf_visitor_expression& right) const
    {
      return utilities::optimized_or(left.expr, right.expr);
    }

    pbes_expression not_(const pfnf_visitor_expression& x) const
    {
      return utilities::optimized_not(x.expr);
    }

    /// \brief A stack containing expressions in PFNF format.
    atermpp::vector<pfnf_visitor_expression> expression_stack;

    /// \brief A stack containing quantifier variables.
    std::vector<data::variable_list> quantifier_stack;

    /// \brief Returns the top element of the expression stack converted to a pbes expression.
    /// \return The top element of the expression stack converted to a pbes expression.
    pbes_expression evaluate() const
    {
      assert(!expression_stack.empty());
      const pfnf_visitor_expression& expr = expression_stack.back();
      const atermpp::vector<pfnf_visitor_quantifier>& q = expr.quantifiers;
      pbes_expression h = expr.expr;
      const atermpp::vector<pfnf_visitor_implication>& g = expr.implications;
      pbes_expression result = h;
      for (atermpp::vector<pfnf_visitor_implication>::const_iterator i = g.begin(); i != g.end(); ++i)
      {
        pbes_expression x = std::accumulate(i->rhs.begin(), i->rhs.end(), tr::false_(), &utilities::optimized_or<pbes_expression>);
        result = utilities::optimized_and(result, utilities::optimized_imp(i->g, x));
      }
      for (atermpp::vector<pfnf_visitor_quantifier>::const_iterator i = q.begin(); i != q.end(); ++i)
      {
        result = i->first ? tr::forall(i->second, result) : tr::exists(i->second, result);
      }
      return result;
    }

    /// \brief Prints an expression
    /// \param expr An expression
    void print_expression(const pfnf_visitor_expression& expr) const
    {
      const atermpp::vector<pfnf_visitor_quantifier>& q = expr.quantifiers;
      pbes_expression h = expr.expr;
      const atermpp::vector<pfnf_visitor_implication>& g = expr.implications;
      for (atermpp::vector<pfnf_visitor_quantifier>::const_iterator i = q.begin(); i != q.end(); ++i)
      {
        std::cout << (i->first ? "forall " : "exists ") << core::pp(i->second) << " ";
      }
      std::cout << (q.empty() ? "" : " . ") << core::pp(h) << "\n";
      for (atermpp::vector<pfnf_visitor_implication>::const_iterator i = g.begin(); i != g.end(); ++i)
      {
        std::cout << " /\\ " << core::pp(i->g) << " => ";
        if (i->rhs.empty())
        {
          std::cout << "true";
        }
        else
        {
          std::cout << "( ";
          for (atermpp::vector<propositional_variable_instantiation>::const_iterator j = i->rhs.begin(); j != i->rhs.end(); ++j)
          {
            if (j != i->rhs.begin())
            {
              std::cout << " \\/ ";
            }
            std::cout << core::pp(*j);
          }
          std::cout << " )";
          std::cout << std::endl;
        }
      }
      std::cout << std::endl;
    }

    /// \brief Prints the expression stack
    /// \param msg A string
    void print(std::string msg = "") const
    {
      std::cout << "--- " << msg << std::endl;
      for (atermpp::vector<pfnf_visitor_expression>::const_iterator i = expression_stack.begin(); i != expression_stack.end(); ++i)
      {
        print_expression(*i);
      }
      std::cout << "value = " << core::pp(evaluate()) << std::endl;
    }

    /// \brief Visit data_expression node
    /// \param e A term
    /// \return The result of visiting the node
    bool visit_data_expression(const pbes_expression& e, const data::data_expression& /* d */)
    {
      expression_stack.push_back(pfnf_visitor_expression(e));
      return super::continue_recursion;
    }

    /// \brief Visit true node
    /// \param e A term
    /// \return The result of visiting the node
    bool visit_true(const pbes_expression& e)
    {
      expression_stack.push_back(pfnf_visitor_expression(e));
      return super::continue_recursion;
    }

    /// \brief Visit false node
    /// \param e A term
    /// \return The result of visiting the node
    bool visit_false(const pbes_expression& e)
    {
      expression_stack.push_back(pfnf_visitor_expression(e));
      return super::continue_recursion;
    }

    /// \brief Visit not node
    /// \param e A term
    /// \return The result of visiting the node
    bool visit_not(const pbes_expression& /* e */, const pbes_expression& /* arg */)
    {
      throw std::runtime_error("operation not should not occur");
      return super::continue_recursion;
    }

    /// \brief Leave and node
    void leave_and()
    {
      // join the two expressions on top of the stack
      pfnf_visitor_expression right = expression_stack.back();
      expression_stack.pop_back();
      pfnf_visitor_expression left  = expression_stack.back();
      expression_stack.pop_back();
      resolve_name_clashes(left, right);
      atermpp::vector<pfnf_visitor_quantifier> q = concat(left.quantifiers, right.quantifiers);
      pbes_expression h = and_(left, right);
      atermpp::vector<pfnf_visitor_implication> g = concat(left.implications, right.implications);
//std::cout << "AND RESULT\n"; print_expression(pfnf_visitor_expression(h, q, g));
      expression_stack.push_back(pfnf_visitor_expression(h, q, g));
    }

    /// \brief Leave or node
    void leave_or()
    {
      // join the two expressions on top of the stack
      pfnf_visitor_expression right = expression_stack.back();
      expression_stack.pop_back();
      pfnf_visitor_expression left  = expression_stack.back();
      expression_stack.pop_back();
      resolve_name_clashes(left, right);

      atermpp::vector<pfnf_visitor_quantifier> q = concat(left.quantifiers, right.quantifiers);

      pbes_expression h_phi = left.expr;
      pbes_expression h_psi = right.expr;
      pbes_expression h = or_(h_phi, h_psi);

      pbes_expression not_h_phi = not_(left.expr);
      pbes_expression not_h_psi = not_(right.expr);

      const atermpp::vector<pfnf_visitor_implication>& q_phi = left.implications;
      const atermpp::vector<pfnf_visitor_implication>& q_psi = right.implications;

      atermpp::vector<pfnf_visitor_implication> g;

      // first conjunction
      for (atermpp::vector<pfnf_visitor_implication>::const_iterator i = q_phi.begin(); i != q_phi.end(); ++i)
      {
        g.push_back(pfnf_visitor_implication(and_(not_h_psi, i->g), i->rhs));
      }

      // second conjunction
      for (atermpp::vector<pfnf_visitor_implication>::const_iterator i = q_psi.begin(); i != q_psi.end(); ++i)
      {
        g.push_back(pfnf_visitor_implication(and_(not_h_phi, i->g), i->rhs));
      }

      // third conjunction
      for (atermpp::vector<pfnf_visitor_implication>::const_iterator i = q_phi.begin(); i != q_phi.end(); ++i)
      {
        for (atermpp::vector<pfnf_visitor_implication>::const_iterator k = q_psi.begin(); k != q_psi.end(); ++k)
        {
          g.push_back(pfnf_visitor_implication(and_(i->g, k->g), concat(i->rhs, k->rhs)));
        }
      }
//std::cout << "OR RESULT\n"; print_expression(pfnf_visitor_expression(h, q, g));
      expression_stack.push_back(pfnf_visitor_expression(h, q, g));
    }

    /// \brief Visit imp node
    /// \param e A term
    /// \return The result of visiting the node
    bool visit_imp(const pbes_expression& /* e */, const pbes_expression& /* left */, const pbes_expression& /* right */)
    {
      throw std::runtime_error("operation imp should not occur");
      return super::continue_recursion;
    }

    /// \brief Visit forall node
    /// \param e A term
    /// \param variables A sequence of variables
    /// \return The result of visiting the node
    bool visit_forall(const pbes_expression& /* e */, const data::variable_list& variables, const pbes_expression& /* expression */)
    {
      quantifier_stack.push_back(variables);
      return super::continue_recursion;
    }

    /// \brief Leave forall node
    void leave_forall()
    {
      // push the quantifier on the expression stack
      expression_stack.back().quantifiers.push_back(std::make_pair(true, quantifier_stack.back()));
      quantifier_stack.pop_back();
    }

    /// \brief Visit exists node
    /// \param e A term
    /// \param variables A sequence of variables
    /// \return The result of visiting the node
    bool visit_exists(const pbes_expression& /* e */, const data::variable_list& variables, const pbes_expression& /* expression */)
    {
      quantifier_stack.push_back(variables);
      return super::continue_recursion;
    }

    /// \brief Leave exists node
    void leave_exists()
    {
      // push the quantifier on the expression stack
      expression_stack.back().quantifiers.push_back(std::make_pair(false, quantifier_stack.back()));
      quantifier_stack.pop_back();
    }

    /// \brief Visit propositional_variable node
    /// \param e A term
    /// \param X A propositional variable
    /// \return The result of visiting the node
    bool visit_propositional_variable(const pbes_expression& /* e */, const propositional_variable_instantiation& X)
    {
      // push the propositional variable on the expression stack
      atermpp::vector<pfnf_visitor_quantifier> q;
      pbes_expression h = tr::true_();
      atermpp::vector<pfnf_visitor_implication> g(1, pfnf_visitor_implication(tr::true_(), atermpp::vector<propositional_variable_instantiation>(1, X)));
      expression_stack.push_back(pfnf_visitor_expression(h, q, g));
      return super::continue_recursion;
    }
};

} // namespace detail

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_DETAIL_PFNF_VISITOR_H
