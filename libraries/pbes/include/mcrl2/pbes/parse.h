// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/parse.h
/// \brief Parser for pbes expressions.

#ifndef MCRL2_PBES_PARSE_H
#define MCRL2_PBES_PARSE_H

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/lexical_cast.hpp>
#include "mcrl2/core/parse.h"
#include "mcrl2/utilities/text_utility.h"
#include "mcrl2/atermpp/vector.h"
#include "mcrl2/data/parse.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/pbes/typecheck.h"

//#define MCRL2_LOG_PARSER_OUTPUT
#ifdef MCRL2_LOG_PARSER_OUTPUT
#include <fstream>
#include "mcrl2/utilities/file_utility.h"
#endif

namespace mcrl2
{

namespace pbes_system
{

struct pbes_actions: public data::data_specification_actions
{
  pbes_actions(const core::parser_table& table_)
    : data::data_specification_actions(table_)
  {}

  pbes_system::pbes_expression parse_PbesExpr(const core::parse_node& node)
  {
    if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "DataValExpr")) { return parse_DataValExpr(node.child(0)); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "true")) { return pbes_system::true_(); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "false")) { return pbes_system::false_(); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "forall") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "PbesExpr")) { return pbes_system::forall(parse_VarsDeclList(node.child(1)), parse_PbesExpr(node.child(3))); }
    else if ((node.child_count() == 4) && (symbol_name(node.child(0)) == "exists") && (symbol_name(node.child(1)) == "VarsDeclList") && (symbol_name(node.child(2)) == ".") && (symbol_name(node.child(3)) == "PbesExpr")) { return pbes_system::exists(parse_VarsDeclList(node.child(1)), parse_PbesExpr(node.child(3))); }
    else if ((node.child_count() == 2) && (symbol_name(node.child(0)) == "!") && (symbol_name(node.child(1)) == "PbesExpr")) { return pbes_system::not_(parse_PbesExpr(node.child(1))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "PbesExpr") && (symbol_name(node.child(1)) == "=>") && (symbol_name(node.child(2)) == "PbesExpr")) { return pbes_system::imp(parse_PbesExpr(node.child(0)), parse_PbesExpr(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "PbesExpr") && (symbol_name(node.child(1)) == "&&") && (symbol_name(node.child(2)) == "PbesExpr")) { return pbes_system::and_(parse_PbesExpr(node.child(0)), parse_PbesExpr(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "PbesExpr") && (symbol_name(node.child(1)) == "||") && (symbol_name(node.child(2)) == "PbesExpr")) { return pbes_system::or_(parse_PbesExpr(node.child(0)), parse_PbesExpr(node.child(2))); }
    else if ((node.child_count() == 3) && (symbol_name(node.child(0)) == "(") && (symbol_name(node.child(1)) == "PbesExpr") && (symbol_name(node.child(2)) == ")")) { return parse_PbesExpr(node.child(1)); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "PropVarInst")) { return parse_PropVarInst(node.child(0)); }
    report_unexpected_node(node);
    return pbes_system::pbes_expression();
  }

  pbes_system::propositional_variable parse_PropVarDecl(const core::parse_node& node)
  {
    return pbes_system::propositional_variable(parse_Id(node.child(0)), parse_VarsDeclList(node.child(1)));
  }

  pbes_system::propositional_variable_instantiation parse_PropVarInst(const core::parse_node& node)
  {
    return pbes_system::propositional_variable_instantiation(parse_Id(node.child(0)), parse_DataExprList(node.child(1)));
  }

  pbes_system::propositional_variable_instantiation parse_PbesInit(const core::parse_node& node)
  {
    return parse_PropVarInst(node.child(1));
  }

  pbes_system::fixpoint_symbol parse_FixedPointOperator(const core::parse_node& node)
  {
    if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "mu")) { return fixpoint_symbol::mu(); }
    else if ((node.child_count() == 1) && (symbol_name(node.child(0)) == "nu")) { return fixpoint_symbol::nu(); }
    report_unexpected_node(node);
    return pbes_system::fixpoint_symbol();
  }

  pbes_equation parse_PbesEqnDecl(const core::parse_node& node)
  {
    return pbes_equation(parse_FixedPointOperator(node.child(0)), parse_PropVarDecl(node.child(1)), parse_PbesExpr(node.child(3)));
  }

  atermpp::vector<pbes_equation> parse_PbesEqnDeclList(const core::parse_node& node)
  {
    return parse_vector<pbes_equation>(node, "PbesEqnDecl", boost::bind(&pbes_actions::parse_PbesEqnDecl, this, _1));
  }

  atermpp::vector<pbes_equation> parse_PbesEqnSpec(const core::parse_node& node)
  {
    return parse_PbesEqnDeclList(node.child(1));
  }

  pbes_system::pbes<> parse_PbesSpec(const core::parse_node& node)
  {
    return pbes<>(parse_DataSpec(node.child(0)), parse_PbesEqnSpec(node.child(2)), atermpp::convert<atermpp::set<data::variable> >(parse_GlobVarSpec(node.child(1))), parse_PbesInit(node.child(3)));
  }
};

inline
pbes_expression parse_pbes_expression_new(const std::string& text)
{
  core::parser p(parser_tables_mcrl2, core::detail::ambiguity_fn);
  unsigned int start_symbol_index = p.start_symbol_index("PbesExpr");
  bool partial_parses = false;
  core::parse_node node = p.parse(text, start_symbol_index, partial_parses);
  pbes_expression result = pbes_actions(parser_tables_mcrl2).parse_PbesExpr(node);
  p.destroy_parse_node(node);
  return result;
}

inline
pbes<> parse_pbes_new(const std::string& text)
{
  core::parser p(parser_tables_mcrl2, core::detail::ambiguity_fn);
  unsigned int start_symbol_index = p.start_symbol_index("PbesSpec");
  bool partial_parses = false;
  core::parse_node node = p.parse(text, start_symbol_index, partial_parses);
  pbes<> result = pbes_actions(parser_tables_mcrl2).parse_PbesSpec(node);
  p.destroy_parse_node(node);
  return result;
}

inline
void complete_pbes(pbes<>& x)
{
  type_check(x);
  pbes_system::translate_user_notation(x);
  pbes_system::normalize_sorts(x, x.data());
  complete_data_specification(x);
}

/// \brief Reads a PBES from an input stream.
/// \param from An input stream
/// \param result A PBES
/// \return The input stream
inline
std::istream& operator>>(std::istream& from, pbes<>& result)
{
  std::string text = utilities::read_text(from);
  result = pbes_system::parse_pbes_new(text);

#ifdef MCRL2_LOG_PARSER_OUTPUT
  std::string filename = utilities::create_filename("pbes", ".txt");
  std::ofstream out(filename.c_str());
  out << "CHECK PBES " << core::detail::check_rule_PBES(result) << std::endl;
  out << pbes_system::pp(result) << std::endl << std::endl;
  out << atermpp::aterm_appl(result).to_string() << std::endl;
#endif

  type_check(result);
  pbes_system::translate_user_notation(result);
  pbes_system::normalize_sorts(result, result.data());
  complete_data_specification(result);
  return from;
}

inline
pbes<> parse_pbes(std::istream& in)
{
  std::string text = utilities::read_text(in);
  pbes<> result = parse_pbes_new(text);
  complete_pbes(result);
  return result;
}

inline
pbes<> parse_pbes(const std::string& text)
{
  std::istringstream in(text);
  return parse_pbes(in);
}

/// \brief Parses a sequence of pbes expressions. The format of the text is as
/// follows:
/// <ul>
/// <li><tt>"datavar"</tt>, followed by a sequence of data variable declarations</li>
/// <li><tt>"predvar"</tt>, followed by a sequence of predicate variable declarations</li>
/// <li><tt>"expressions"</tt>, followed by a sequence of pbes expressions separated by newlines</li>
/// </ul>
/// An example of this is:
/// \code
/// datavar
///   n: Nat;
/// predvar
///   X: Pos;
///   Y: Nat, Bool;
/// \endcode
/// \param text A string
/// \param data_spec A string
/// N.B. A side effect of the data specification is that it determines whether rewrite rules
/// for types like Pos and Nat are generated or not.
/// \return The parsed expression and the data specification that was used.
inline
std::pair<atermpp::vector<pbes_expression>, data::data_specification> parse_pbes_expressions(std::string text, std::string data_spec = "")
{
  std::string unique_prefix("UNIQUE_PREFIX");
  int unique_prefix_index = 0;

  text = utilities::remove_comments(text);
  const std::string separator1 = "datavar";
  const std::string separator2 = "predvar";
  const std::string separator3 = "expressions";

  std::string::size_type i = text.find(separator1);
  std::string::size_type j = text.find(separator2);
  std::string::size_type k = text.find(separator3);
  if (i == std::string::npos)
  {
    throw std::runtime_error("Error in parse_pbes_expressions: could not find keyword " + separator1);
  }
  if (j == std::string::npos)
  {
    throw std::runtime_error("Error in parse_pbes_expressions: could not find keyword " + separator2);
  }
  if (j == std::string::npos)
  {
    throw std::runtime_error("Error in parse_pbes_expressions: could not find keyword " + separator3);
  }

  std::string datavar_text     = text.substr(i + separator1.size(), j - i - separator1.size());
  std::string predvar_text     = text.substr(j + separator2.size(), k - j - separator2.size());
  std::string expressions_text = text.substr(k + separator3.size());

  // the generated pbes specification
  std::string pbesspec = "pbes";

  std::vector<std::string> pwords = utilities::split(predvar_text, ";");
  for (std::vector<std::string>::iterator i = pwords.begin(); i != pwords.end(); ++i)
  {
    if (boost::trim_copy(*i).empty())
    {
      continue;
    }
    std::vector<std::string> args;
    std::vector<std::string> words = utilities::split(*i, ":");
    std::string var = boost::trim_copy(words[0]);
    if (words.size() >= 2 && !boost::trim_copy(words[1]).empty())
    {
      args = utilities::split(boost::trim_copy(words[1]), "#");
    }
    for (std::vector<std::string>::iterator j = args.begin(); j != args.end(); ++j)
    {
      std::vector<std::string> w = utilities::split(*j, ",");
      for (std::vector<std::string>::iterator k = w.begin(); k != w.end(); ++k)
      {
        *k = unique_prefix + boost::lexical_cast<std::string>(unique_prefix_index++) + ": " + *k;
      }
      *j = boost::algorithm::join(w, ", ");
    }
    std::string arg;
    if (!args.empty())
    {
      arg = "(" + boost::algorithm::join(args, ", ") + ")";
    }
    pbesspec = pbesspec + "\nmu " + var + arg + " = true;";
  }

  datavar_text = utilities::remove_whitespace(datavar_text);
  if (!datavar_text.empty() && datavar_text[datavar_text.size() - 1] == ';')
  {
    datavar_text = datavar_text.erase(datavar_text.size() - 1);
  }
  datavar_text = utilities::regex_replace(";", ", ", datavar_text);

  // add a dummy propositional variable to the pbes, that is used for the initialization
  pbesspec = pbesspec + "\nmu dummy1 = true;";

  // for each expression add an equation to the pbes
  std::vector<std::string> expressions = utilities::split(expressions_text, ";");
  if (!expressions.empty() && boost::trim_copy(expressions.back()).empty())
  {
    expressions.pop_back();
  }
  for (std::vector<std::string>::iterator i = expressions.begin(); i != expressions.end(); ++i)
  {
    pbesspec = pbesspec
               + "\nmu "
               + unique_prefix
               + boost::lexical_cast<std::string>(unique_prefix_index++)
               + (datavar_text.empty() ? "" : "(")
               + datavar_text
               + (datavar_text.empty() ? "" : ")")
               + " = "
               + boost::trim_copy(*i) + ";";
  }

  // add an initialization section to the pbes
  pbesspec = data_spec + (data_spec.empty() ? "" : "\n") + pbesspec + "\ninit dummy1;";

  pbes<> p;
  std::stringstream in(pbesspec);
  try
  {
    in >> p;
  }
  catch (std::runtime_error e)
  {
    std::cerr << "parse_pbes_expression: parse error detected in the generated specification\n"
              << pbesspec
              << std::endl;
    throw e;
  }

  atermpp::vector<pbes_expression> result;
  for (pbes<>::container_type::iterator i = p.equations().end() - expressions.size(); i != p.equations().end(); ++i)
  {
    result.push_back(i->formula());
  }

  return std::make_pair(result, p.data());
}

/// \brief Parses a single pbes expression.
/// \param text A string
/// \param var_decl A string
/// with their types.<br>
/// An example of this is:
/// \code
/// datavar
///   n: Nat;
/// predvar
///   X: Pos;
///   Y: Nat, Bool;
/// \endcode
/// \param data_spec A string
/// \return The parsed expression
inline
pbes_expression parse_pbes_expression(std::string text, std::string var_decl = "datavar\npredvar\n", std::string data_spec = "")
{
  return parse_pbes_expressions(var_decl + "\nexpressions\n" + text, data_spec).first.front();
}

/// \brief Parses a string with substitutions and adds them to a substition function
/// \param text A string
/// \param data_spec A data specification
/// \param sigma A substitution function
template <typename SubstitutionFunction>
void parse_substitutions(std::string text, data::data_specification const& data_spec, SubstitutionFunction& sigma)
{
  std::cerr << "SSSSWWW" << text << std::endl;
  std::vector<std::string> substitutions = utilities::split(text, ";");
  for (std::vector<std::string>::iterator i = substitutions.begin(); i != substitutions.end(); ++i)
  {
    std::vector<std::string> words = utilities::regex_split(*i, ":=");
    if (words.size() != 2)
    {
      continue;
    }
    data::variable v = data::parse_variable(words[0], data_spec);
    data::data_expression e = data::parse_data_expression(words[1], "", data_spec);
    sigma[v] = e;
  }
}

/// \brief Parses a pbes expression.
/// \param expr A string
/// \param subst A string
/// \param p A PBES
/// \param sigma A substitution function
/// \return The parsed expression
template <typename SubstitutionFunction>
pbes_expression parse_pbes_expression(std::string expr, std::string subst, const pbes<>& p, SubstitutionFunction& sigma)
{
  typedef core::term_traits<pbes_expression> tr;

  parse_substitutions(subst, p.data(), sigma);

  std::string datavar_text;
  for (typename SubstitutionFunction::iterator i = sigma.begin(); i != sigma.end(); ++i)
  {
    data::variable v = i->first;
    datavar_text = datavar_text + (i == sigma.begin() ? "" : ", ") + data::pp(v) + ": " + data::pp(v.sort());
  }

  pbes<> q = p;
  q.initial_state() == tr::true_();
  std::string pbesspec = pbes_system::pp(q);
  std::string init("init");
  // remove the init declaration
  pbesspec = pbesspec.substr(0, std::find_end(pbesspec.begin(), pbesspec.end(), init.begin(), init.end()) - pbesspec.begin());

  // add an equation mu dummy1(vars) = (expr = expr)
  pbesspec = pbesspec
             + "\nmu "
             + "dummy1"
             + (datavar_text.empty() ? "" : "(")
             + datavar_text
             + (datavar_text.empty() ? "" : ")")
             + " = "
             + boost::trim_copy(expr) + ";";

  // add a dummy propositional variable to the pbes, that is used for the initialization
  pbesspec = pbesspec + "\nmu dummy2 = true;";

  // add an initialization section to the pbes
  pbesspec = pbesspec + "\ninit dummy2;";

  std::stringstream in(pbesspec);
  try
  {
    in >> q;
  }
  catch (std::runtime_error e)
  {
    std::cerr << "parse_pbes_expression: parse error detected in the generated specification\n"
              << pbesspec
              << std::endl;
    throw e;
  }

  pbes_expression result = q.equations()[q.equations().size() - 2].formula();
  return result;
}

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_PARSE_H
