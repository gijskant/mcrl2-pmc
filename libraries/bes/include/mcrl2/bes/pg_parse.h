// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/bes/pg_parse.h
/// \brief Parsing of parity games in the format used by PGSolver.

#ifndef MCRL2_BES_PG_PARSE_H
#define MCRL2_BES_PG_PARSE_H

#include <cstdio>
#include <cctype>
#include <map>

#include "mcrl2/utilities/text_utility.h"
#include "mcrl2/bes/boolean_equation_system.h"

namespace mcrl2
{

namespace bes
{

typedef unsigned long long identifier_t;
typedef unsigned short priority_t;
typedef bool owner_t;

struct node_t
{
  identifier_t id;
  priority_t prio;
  owner_t owner;
  std::set<identifier_t> successors;

  bool operator<(node_t const& other)
  {
    return id < other.id;
  }
};

// Build a formula from the strings in v. if p = 0, than a disjunction is built,
// otherwise the result is a conjunction.
// Prefix is added to each of the identifiers in v.
boolean_expression formula(std::set<identifier_t> const& v, priority_t p, std::string prefix = "X")
{
  atermpp::set<boolean_expression> v_prefixed;
  for (std::set<identifier_t>::const_iterator i = v.begin(); i != v.end(); ++i)
  {
    std::stringstream id;
    id << prefix << *i;
    v_prefixed.insert(boolean_variable(id.str()));
  }

  if (p == 0)
  {
    return join_or(v_prefixed.begin(), v_prefixed.end());
  }
  else
  {
    return join_and(v_prefixed.begin(), v_prefixed.end());
  }
}

/// \brief Reads a parity games from an input stream, and stores it as a BES.
/// \param from An input stream
/// \param b A boolean equation system
/// \return The input stream
// EBNF:
// <parity_game> ::= [parity <identifier> ;] <node_spec>+
// <node_spec> ::= <identifier> <priority> <owner> <successors> [name] ;
// <identifier> ::= N
// <priority> ::= N
// <owner> ::= 0 | 1
// <successors> ::= <identifier> (, <identifier>)*
// <name> ::= " ( any ASCII string not containing `"') "
// In this N means a natural number.
// There must be whitespace characters between the following pairs of tokens:
// <identifier> and <priority>, <priority> and <owner>, <owner> and <identifier>
template <typename Container>
inline
void parse_pgsolver(std::istream& from, boolean_equation_system<Container>& b)
{
  while (!isalnum(from.peek()))
  {
    from.ignore();
  }

  // We ignore parity line
  if (from.peek() == 'p')
  {
    from.ignore(1024, '\n');
  }

  // Parse node specifications (store in map)
  std::map<identifier_t, node_t> game;
  bool init = false;
  identifier_t initial_node;

  while (isalnum(from.peek()))
  {
    while (!isalnum(from.peek()))
    {
      from.ignore();
    }
    node_t node;
    from >> node.id;
    from >> node.prio;
    from >> node.owner;

    if (!init)
    {
      initial_node = node.id;
      init = true;
    }

    std::string successors;
    std::getline(from, successors);
    // Rest of line. First remove comments
    size_t index = successors.find('"');
    if (index != std::string::npos)
    {
      successors = successors.substr(0, index);
    }

    successors = utilities::remove_whitespace(successors);
    std::vector<std::string> v(utilities::split(successors,","));
    for (std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i)
    {
      identifier_t id;
      std::stringstream tmp;
      tmp << *i;
      tmp >> id;
      node.successors.insert(id);
    }

    game[node.id] = node;
  }

  // Build Boolean equation system. First we group equations by block
  std::map<priority_t, atermpp::set<boolean_equation> > blocks;
  // Translation scheme:
  // prefix every id with X. Owner 0 means ||, owner 1 means &&

  for (std::map<identifier_t, node_t>::const_iterator i = game.begin(); i != game.end(); ++i)
  {
    std::stringstream id;
    id << "X" << i->second.id;

    fixpoint_symbol fp(fixpoint_symbol::mu());
    if (i->second.prio % 2 == 0)
    {
      fp = fixpoint_symbol::nu();
    }

    boolean_equation eqn(fp, boolean_variable(id.str()), formula(i->second.successors, i->second.owner));

    blocks[i->second.prio].insert(eqn);
  }

  atermpp::vector<boolean_equation> eqns;
  for (std::map<priority_t, atermpp::set<boolean_equation> >::const_iterator i = blocks.begin(); i != blocks.end(); ++i)
  {
    eqns.insert(eqns.end(), i->second.begin(), i->second.end());
  }

  b.equations() = eqns;
  std::stringstream init_id;
  init_id << initial_node;
  b.initial_state() = boolean_variable("X" + init_id.str());

}

} // namespace bes

} // namespace mcrl2

#endif // MCRL2_PBES_BES_PARSE_H
