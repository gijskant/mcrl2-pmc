// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file rewriter_test.cpp
/// \brief Test for the pbes rewriters.

//#define MCRL2_PBES_EXPRESSION_BUILDER_DEBUG

#include <iostream>
#include <set>
#include <sstream>
#include <boost/test/minimal.hpp>
#include "mcrl2/core/text_utility.h"
#include "mcrl2/pbes/pbes_parse.h"
#include "mcrl2/data/parser.h"
#include "mcrl2/data/rewriter.h"
#include "mcrl2/data/enumerator.h"
#include "mcrl2/data/data_expression_with_variables.h"
#include "mcrl2/pbes/pbes_parse.h"
#include "mcrl2/pbes/pbes_expression_with_variables.h"
#include "mcrl2/pbes/rewriter.h"

using namespace mcrl2;

/// Parse a string of the form "b: Bool := true, n: Nat := 0", and add them
/// to the substition function sigma.
template <typename SubstitutionFunction>
void parse_substitutions(std::string text, SubstitutionFunction& sigma)
{
  std::vector<std::string> substitutions = core::split(text, ";");
  for (std::vector<std::string>::iterator i = substitutions.begin(); i != substitutions.end(); ++i)
  {
    std::vector<std::string> words = core::regex_split(*i, ":=");
    if (words.size() != 2)
    {
      continue;
    }
    data::data_variable v = data::parse_data_variable(words[0]);
    data::data_expression e = data::parse_data_expression(words[1]);
    sigma[v] = e;
  }
}

/// Create a string representation of a substitution map.
template <typename SubstitutionFunction>
std::string print_substitution(const SubstitutionFunction& sigma)
{
  std::stringstream out;
  for (typename SubstitutionFunction::const_iterator i = sigma.begin(); i != sigma.end(); ++i)
  {
    out << "  " << core::pp(i->first) << " -> " << core::pp(i->second) << std::endl;
  }
  return out.str();
}

const std::string VARIABLE_SPECIFICATION =
  "datavar         \n"
  "  b:  Bool;     \n"
  "  b1: Bool;     \n"
  "  b2: Bool;     \n"
  "  b3: Bool;     \n"
  "                \n"
  "  n:  Nat;      \n"
  "  n1: Nat;      \n"
  "  n2: Nat;      \n"
  "  n3: Nat;      \n"
  "                \n"
  "  p:  Pos;      \n"
  "  p1: Pos;      \n"
  "  p2: Pos;      \n"
  "  p3: Pos;      \n"
  "                \n"
  "predvar         \n"
  "  X;            \n"
  "  Y: Nat;       \n"
  "  W: Bool;      \n"
  "  Z: Bool, Pos; \n"
  ;

inline
pbes_expression expr(const std::string& text)
{
  return pbes_system::parse_pbes_expression(text, VARIABLE_SPECIFICATION);
}

template <typename Rewriter>
void test_expressions(Rewriter R, std::string expr1, std::string expr2)
{
  if (R(expr(expr1)) != R(expr(expr2)))
  {
    BOOST_CHECK(R(expr(expr1)) == R(expr(expr2)));
    std::cout << "--- failed test --- " << expr1 << " -> " << expr2 << std::endl;
    std::cout << "expr1    " << core::pp(expr(expr1)) << std::endl;
    std::cout << "expr2    " << core::pp(expr(expr2)) << std::endl;
    std::cout << "R(expr1) " << core::pp(R(expr(expr1))) << std::endl;
    std::cout << "R(expr2) " << core::pp(R(expr(expr2))) << std::endl;
    std::cout << "R(expr1) " << R(expr(expr1)) << std::endl;
    std::cout << "R(expr2) " << R(expr(expr2)) << std::endl;
  }
}

template <typename Rewriter>
void test_expressions(Rewriter R, std::string expr1, std::string expr2, std::string var_decl, std::string substitutions)
{
  data::rewriter_map<std::map<data::data_variable, data::data_expression_with_variables> > sigma;
  parse_substitutions(substitutions, sigma);
  pbes_system::pbes_expression d1 = pbes_system::parse_pbes_expression(expr1, var_decl);
  pbes_system::pbes_expression d2 = pbes_system::parse_pbes_expression(expr2, var_decl);
  if (R(d1, sigma) != R(d2))
  {
    BOOST_CHECK(R(d1, sigma) == R(d2));
    std::cout << "--- failed test --- " << expr1 << " -> " << expr2 << std::endl;
    std::cout << "d1           " << core::pp(d1) << std::endl;
    std::cout << "d2           " << core::pp(d2) << std::endl;
    std::cout << "sigma        " << substitutions << std::endl;
    std::cout << "R(d1, sigma) " << core::pp(R(d1, sigma)) << std::endl;
    std::cout << "R(d2)        " << core::pp(R(d2)) << std::endl;
  }
  else {
    BOOST_CHECK(R(d1, sigma) == R(d2));
    std::cout << "--- succeeded test --- " << expr1 << " -> " << expr2 << std::endl;
    std::cout << "d1           " << core::pp(d1) << std::endl;
    std::cout << "d2           " << core::pp(d2) << std::endl;
    std::cout << "sigma        " << substitutions << std::endl;
    std::cout << "R(d1, sigma) " << core::pp(R(d1, sigma)) << std::endl;
    std::cout << "R(d2)        " << core::pp(R(d2)) << std::endl;
  }
}

void test_simplifying_rewriter()
{
  std::cout << "<test_simplifying_rewriter>" << std::endl;
  data::rewriter datar;
  pbes_system::simplifying_rewriter<pbes_system::pbes_expression, data::rewriter> R(datar);

  test_expressions(R, "val(n >= 0) || Y(n)"                                             , "val(true)");
  test_expressions(R, "false"                                                           , "val(false)");
  test_expressions(R, "true"                                                            , "val(true)");
  test_expressions(R, "true && true"                                                    , "val(true)");
  test_expressions(R, "(true && true) && true"                                          , "val(true)");
  test_expressions(R, "true && false"                                                   , "val(false)");
  test_expressions(R, "true => val(b)"                                                  , "val(b)");
  test_expressions(R, "X && true"                                                       , "X");
  test_expressions(R, "true && X"                                                       , "X");
  test_expressions(R, "X && false"                                                      , "val(false)");
  test_expressions(R, "X && val(false)"                                                 , "val(false)");
  test_expressions(R, "false && X"                                                      , "val(false)");
  test_expressions(R, "X && (false && X)"                                               , "val(false)");
  test_expressions(R, "Y(1+2)"                                                          , "Y(3)");
  test_expressions(R, "true || true"                                                    , "true");
  test_expressions(R, "(true || true) || true"                                          , "true");
  test_expressions(R, "true || false"                                                   , "true");
  test_expressions(R, "false => X"                                                      , "true");
  test_expressions(R, "Y(n+n)"                                                          , "Y(n+n)");
  test_expressions(R, "Y(n+p)"                                                          , "Y(n+p)");
  test_expressions(R, "forall m:Nat. false"                                             , "false");
  test_expressions(R, "X && X"                                                          , "X");
  test_expressions(R, "val(true)"                                                       , "true");  
  test_expressions(R, "false => (exists m:Nat. exists k:Nat. val(m*m == k && k > 20))"  , "true");
  test_expressions(R, "exists m:Nat.true"                                               , "true");

  // test_expressions(R, "Y(n+p) && Y(p+n)"                                                , "Y(n+p)");
  // test_expressions(R, "exists m:Nat. val( m== p) && Y(m)"                               , "Y(p)");
  // test_expressions(R, "X && (Y(p) || X)"                                                , "X");
  // test_expressions(R, "X || (Y(p) && X)"                                                , "X");
  // test_expressions(R, "val(b || !b)"                                                    , "val(true)");
  // test_expressions(R, "Y(n1 + n2)"                                                      , "Y(n2 + n1)");
  
  // pbes_expression p = R(expr("Y(n)"));
  // BOOST_CHECK(!core::term_traits<pbes_expression>::is_constant(p));
}

template <typename variable_type, typename data_term_type>
void test_enumerate_quantifiers_sequence_assign(variable_type v, data_term_type t)
{
  typedef data::rewriter_map<std::map<variable_type, data_term_type> > substitution_map;
  substitution_map sigma;
  pbes_system::detail::enumerate_quantifiers_sequence_assign<substitution_map> assign(sigma);
  assign(v, t);
}

template <typename PbesTerm>
void test_enumerate_quantifiers_sequence_action(PbesTerm phi)
{
  typedef typename core::term_traits<PbesTerm>::variable_type variable_type;
  typedef typename core::term_traits<PbesTerm>::data_term_type data_term_type;
  typedef data::rewriter_map<std::map<variable_type, data_term_type> > substitution_map;
  data::rewriter datar = data::default_data_specification();
  std::set<PbesTerm> A;
  pbes_system::simplifying_rewriter<PbesTerm, data::rewriter> r(datar);
  substitution_map sigma;
  bool is_constant;
  data::data_variable_list v;
  pbes_system::detail::make_enumerate_quantifiers_sequence_action(A, r, phi, sigma, v, is_constant, core::term_traits<PbesTerm>::is_false)();
}

template <typename PbesTerm, typename DataEnumerator>
void test_enumerator_quantifiers(PbesTerm phi, DataEnumerator datae)
{
  typedef typename core::term_traits<PbesTerm>::variable_type variable_type;
  typedef typename core::term_traits<PbesTerm>::variable_sequence_type variable_sequence_type;
  typedef typename core::term_traits<PbesTerm>::data_term_type data_term_type;
  typedef data::rewriter_map<std::map<variable_type, data_term_type> > substitution_map;
  data::rewriter datar = data::default_data_specification();
  pbes_system::simplifying_rewriter<PbesTerm, data::rewriter> r(datar);
  substitution_map sigma;

  variable_sequence_type variables;

  PbesTerm result =
  pbes_system::detail::enumerate_quantifiers(variables,
                                             phi,
                                             sigma,
                                             datae,
                                             r,
                                             core::term_traits<PbesTerm>::is_true,
                                             core::term_traits<PbesTerm>::true_(),
                                             pbes_system::detail::enumerate_quantifiers_join_or<PbesTerm>()
                                          );
}

void test_enumerate_quantifiers_rewriter()
{
  std::cout << "<test_enumerate_quantifiers_rewriter>" << std::endl;

  data::data_specification data_spec = default_data_specification();
  data::rewriter datar(data_spec);
  data::number_postfix_generator generator("UNIQUE_PREFIX");
  data::data_enumerator<data::number_postfix_generator> datae(data_spec, datar, generator);
  data::rewriter_with_variables datarv(data_spec);

  data::data_variable   v = data::parse_data_expression("n", "n: Pos;\n");
  data::data_expression d = data::parse_data_expression("n < 10", "n: Pos;\n");
  data::data_expression_with_variables dv(d);
  test_enumerate_quantifiers_sequence_assign(v, d);
  test_enumerate_quantifiers_sequence_assign(v, dv);

  pbes_system::pbes_expression y = expr("Y(n)");
  pbes_system::pbes_expression_with_variables yv(y, data::data_variable_list()); 
  // test_enumerate_quantifiers_sequence_action(y);
  test_enumerate_quantifiers_sequence_action(yv);

  // test_enumerator_quantifiers(y, datae); This doesn't work because of a mismatch between y and datae
  test_enumerator_quantifiers(yv, datae);

  pbes_system::enumerate_quantifiers_rewriter<pbes_system::pbes_expression, data::rewriter_with_variables, data::data_enumerator<> > R(datarv, datae);

  test_expressions(R, "exists b: Bool. val(if(b, false, b))"                            , "val(false)");
  // test_expressions(R, "val(!(b && n < 10)) || Z(!b, n + 1)"                             , "true");
  test_expressions(R, "exists b: Bool. W(b)"                                            , "W(true) || W(false)");
  test_expressions(R, "forall n: Nat.val(!(n < 1)) || Y(n)"                             , "Y(0)");
  test_expressions(R, "false"                                                           , "val(false)");
  test_expressions(R, "true"                                                            , "val(true)");
  test_expressions(R, "true && true"                                                    , "val(true)");
  test_expressions(R, "(true && true) && true"                                          , "val(true)");
  test_expressions(R, "true && false"                                                   , "val(false)");
  test_expressions(R, "true => val(b)"                                                  , "val(b)");
  test_expressions(R, "X && true"                                                       , "X");
  test_expressions(R, "true && X"                                                       , "X");
  test_expressions(R, "X && false"                                                      , "val(false)");
  test_expressions(R, "X && val(false)"                                                 , "val(false)");
  test_expressions(R, "false && X"                                                      , "val(false)");
  test_expressions(R, "X && (false && X)"                                               , "val(false)");
  test_expressions(R, "Y(1+2)"                                                          , "Y(3)");
  test_expressions(R, "true || true"                                                    , "true");
  test_expressions(R, "(true || true) || true"                                          , "true");
  test_expressions(R, "true || false"                                                   , "true");
  test_expressions(R, "false => X"                                                      , "true");
  test_expressions(R, "Y(n+n)"                                                          , "Y(n+n)");
  test_expressions(R, "Y(n+p)"                                                          , "Y(n+p)");
  test_expressions(R, "forall m:Nat. false"                                             , "false");
  test_expressions(R, "X && X"                                                          , "X");
  test_expressions(R, "val(true)"                                                       , "true");  
  test_expressions(R, "false => (exists m:Nat. exists k:Nat. val(m*m == k && k > 20))"  , "true");
  test_expressions(R, "exists m:Nat.true"                                               , "true");
  test_expressions(R, "forall m:Nat.val(m < 3)"                                         , "false");
  test_expressions(R, "exists m:Nat.val(m > 3)"                                         , "true");
}

void test_substitutions1()
{
  std::cout << "<test_substitutions1>" << std::endl;

  data::rewriter  datar;
  pbes_system::simplifying_rewriter<pbes_system::pbes_expression, data::rewriter> r(datar);

  data::rewriter_map<atermpp::map<data::data_variable, pbes_system::pbes_expression> > sigma; 
  sigma[data::parse_data_variable("m: Pos")] = r(data::parse_data_expression("3"));
  sigma[data::parse_data_variable("n: Pos")] = r(data::parse_data_expression("4"));

  std::string var_decl =
    "datavar         \n"
    "  m, n:  Pos;   \n"
    "                \n"
    "predvar         \n"
    "  X: Pos;       \n"
    ;
  pbes_system::pbes_expression d1 = pbes_system::parse_pbes_expression("X(m+n)", var_decl);
  pbes_system::pbes_expression d2 = pbes_system::parse_pbes_expression("X(7)", var_decl);
  BOOST_CHECK(r(d1, sigma) == r(d2));
}

void test_substitutions2()
{
  std::cout << "<test_substitutions2>" << std::endl;
  data::data_specification data_spec = default_data_specification();
  data::number_postfix_generator generator("UNIQUE_PREFIX");
  data::rewriter datar(data_spec);
  data::data_enumerator<data::number_postfix_generator> datae(data_spec, datar, generator);
  data::rewriter_with_variables datarv(data_spec);
  pbes_system::enumerate_quantifiers_rewriter<pbes_system::pbes_expression, data::rewriter_with_variables, data::data_enumerator<> > R(datarv, datae);

  std::string var_decl;
  std::string sigma;
  std::string expr1;
  std::string expr2;
  
  //------------------------//
  var_decl =
    "datavar         \n"
    "  m, n:  Pos;   \n"
    "                \n"
    "predvar         \n"
    "  X: Pos;       \n"
    ;
	expr1 = "X(m+n)";
	expr2 = "X(7)";
  sigma = "m: Pos := 3; n: Pos := 4"; 
  test_expressions(R, expr1, expr2, var_decl, sigma); 

  //------------------------//
  var_decl =
    "datavar         \n"
    "  n: Nat;       \n"
    "                \n"
    "predvar         \n"
    "  X: Bool, Nat; \n"
    ;
	expr1 = "forall c: Bool. X(c, n)";
	expr2 = "X(true, 0) && X(false, 0)";
  sigma = "b: Bool := true; n: Nat := 0";
  test_expressions(R, expr1, expr2, var_decl, sigma); 

//  //------------------------//
//  var_decl =
//    "datavar         \n"
//    "predvar         \n"
//    "  X: Nat;       \n"
//    ;
//	expr1 = "exists b: Bool, c: Bool. val(if(b, c, b))";
//	expr2 = "val(false)";
//  sigma = "c: Bool := false";
//  test_expressions(R, expr1, expr2, var_decl, sigma); 
//
//  //------------------------//
//  var_decl =
//    "datavar         \n"
//    "predvar         \n"
//    "  X: Nat;       \n"
//    ;
//	expr1 = "exists b: Bool, c: Bool. val(if(b, false, b))";
//	expr2 = "val(false)";
//  sigma = "d: Bool := false";
//  test_expressions(R, expr1, expr2, var_decl, sigma); 

  //------------------------//
  var_decl =
    "datavar         \n"
    "predvar         \n"
    "  X: Nat;       \n"
    ;
	expr1 = "exists b: Bool, c: Bool. val(b && c)";
	expr2 = "val(true)";
  sigma = "";
  test_expressions(R, expr1, expr2, var_decl, sigma); 

  //------------------------//
  var_decl =
    "datavar         \n"
    "predvar         \n"
    "  X: Nat;       \n"
    ;
	expr1 = "exists b: Bool.exists c:Bool. val(b && c)";
	expr2 = "val(true)";
  sigma = "";
  test_expressions(R, expr1, expr2, var_decl, sigma);
}

void test_substitutions3()
{
  std::cout << "<test_substitutions3>" << std::endl;
  std::string DATA_SPEC =
    "sort D = struct d1 | d2;                                                                                                   \n"
    "     DBuf = List(D);                                                                                                       \n"
    "     BBuf = List(Bool);                                                                                                    \n"
    "                                                                                                                           \n"
    "map  n: Pos;                                                                                                               \n"
    "     empty: BBuf;                                                                                                          \n"
    "     insert: D # Nat # DBuf -> DBuf;                                                                                       \n"
    "     insert: Bool # Nat # BBuf -> BBuf;                                                                                    \n"
    "     nextempty_mod: Nat # BBuf # Nat # Pos -> Nat;                                                                         \n"
    "     q1,q2: DBuf;                                                                                                          \n"
    "                                                                                                                           \n"
    "var  d,d': D;                                                                                                              \n"
    "     i,j,m: Nat;                                                                                                           \n"
    "     q: DBuf;                                                                                                              \n"
    "     c,c': Bool;                                                                                                           \n"
    "     n': Pos;                                                                                                              \n"
    "     b: BBuf;                                                                                                              \n"
    "eqn  q1  =  [d1, d1];                                                                                                      \n"
    "     q2  =  [d1, d1];                                                                                                      \n"
    "     n  =  2;                                                                                                              \n"
    "     q1  =  [d1, d1];                                                                                                      \n"
    "     q2  =  [d1, d1];                                                                                                      \n"
    "     empty  =  [false, false];                                                                                             \n"
    "     i == 0  ->  insert(d, i, q)  =  d |> tail(q);                                                                         \n"
    "     i > 0  ->  insert(d, i, d' |> q)  =  d' |> insert(d, Int2Nat(i - 1), q);                                              \n"
    "     i == 0  ->  insert(c, i, b)  =  c |> tail(b);                                                                         \n"
    "     i > 0  ->  insert(c, i, c' |> b)  =  c' |> insert(c, Int2Nat(i - 1), b);                                              \n"
    "     b . (i mod n') && m > 0  ->  nextempty_mod(i, b, m, n')  =  nextempty_mod((i + 1) mod 2 * n', b, Int2Nat(m - 1), n'); \n"
    "     !(b . (i mod n') && m > 0)  ->  nextempty_mod(i, b, m, n')  =  i mod 2 * n';                                          \n"
  ;
  data::data_specification data_spec = data::parse_data_specification(DATA_SPEC);
  data::number_postfix_generator generator("UNIQUE_PREFIX");
  data::rewriter datar(data_spec);
  data::data_enumerator<data::number_postfix_generator> datae(data_spec, datar, generator);
  data::rewriter_with_variables datarv(data_spec);
  pbes_system::enumerate_quantifiers_rewriter<pbes_system::pbes_expression, data::rewriter_with_variables, data::data_enumerator<> > r(datarv, datae);
    
  data::rewriter_map<std::map<data::data_variable, data::data_expression_with_variables> > sigma;
  sigma[data::parse_data_variable("l_S:Nat")]             = data::parse_data_expression("0");
  sigma[data::parse_data_variable("m_S:Nat")]             = data::parse_data_expression("0");
  sigma[data::parse_data_variable("bst_K:Bool")]          = data::parse_data_expression("false");
  sigma[data::parse_data_variable("bst1_K:Bool")]         = data::parse_data_expression("false");
  sigma[data::parse_data_variable("k_K:Nat")]             = data::parse_data_expression("0");
  sigma[data::parse_data_variable("bst2_L:Bool")]         = data::parse_data_expression("false");
  sigma[data::parse_data_variable("bst3_L:Bool")]         = data::parse_data_expression("false");
  sigma[data::parse_data_variable("k_L:Nat")]             = data::parse_data_expression("0");
  sigma[data::parse_data_variable("l'_R:Nat")]            = data::parse_data_expression("0");
  sigma[data::parse_data_variable("b_R:BBuf", DATA_SPEC)] = data::parse_data_expression("[false, false]");

  std::string var_decl =
    "datavar                                                     \n"
    "  l_S:Nat    ;                                              \n"
    "  m_S:Nat    ;                                              \n"
    "  bst_K:Bool ;                                              \n"
    "  bst1_K:Bool;                                              \n"
    "  k_K:Nat    ;                                              \n"
    "  bst2_L:Bool;                                              \n"
    "  bst3_L:Bool;                                              \n"
    "  k_L:Nat    ;                                              \n"
    "  l'_R:Nat   ;                                              \n"
    "  b_R:BBuf   ;                                              \n"
    "                                                            \n"
    "predvar                                                     \n"
    "  X: Nat, Nat, Bool, Bool, Nat, Bool, Bool, Nat, Nat, BBuf; \n"
    ;

  // pbes_system::pbes_expression phi = pbes_system::parse_pbes_expression("(((((((((((val(bst2_L && !bst3_L) || (exists k_S2_00: Nat. val(k_S2_00 < m_S && !bst_K && !bst1_K))) || val(!bst_K && bst1_K)) || val(!bst_K && bst1_K)) || val(!bst2_L && !bst3_L)) || val(b_R . (l'_R mod 2))) || val(!bst2_L && bst3_L)) || val(!bst2_L && bst3_L)) || val((bst_K && !bst1_K) && (k_K - l'_R) mod 4 < 2)) || val((bst_K && !bst1_K) && !((k_K - l'_R) mod 4 < 2))) || val(m_S < 2)) || val(m_S < 2)) && (((((((((((val(!(bst2_L && !bst3_L)) || X(k_L, (m_S - k_L + l_S) mod 4, bst_K, bst1_K, k_K, false, false, 0, l'_R, b_R)) && (forall k_S2_00: Nat. val(!(k_S2_00 < m_S && !bst_K && !bst1_K)) || X(l_S, m_S, false, true, (l_S + k_S2_00) mod 4, bst2_L, bst3_L, k_L, l'_R, b_R))) && val(!(!bst_K && bst1_K)) || X(l_S, m_S, true, false, k_K, bst2_L, bst3_L, k_L, l'_R, b_R)) && val(!(!bst_K && bst1_K)) || X(l_S, m_S, false, false, 0, bst2_L, bst3_L, k_L, l'_R, b_R)) && val(!(!bst2_L && !bst3_L)) || X(l_S, m_S, bst_K, bst1_K, k_K, false, true, nextempty_mod(l'_R, b_R, 2, 2), l'_R, b_R)) && val(!b_R . (l'_R mod 2)) || X(l_S, m_S, bst_K, bst1_K, k_K, bst2_L, bst3_L, k_L, (l'_R + 1) mod 4, insert(false, l'_R mod 2, b_R))) && val(!(!bst2_L && bst3_L)) || X(l_S, m_S, bst_K, bst1_K, k_K, true, false, k_L, l'_R, b_R)) && val(!(!bst2_L && bst3_L)) || X(l_S, m_S, bst_K, bst1_K, k_K, false, false, 0, l'_R, b_R)) && val(!((bst_K && !bst1_K) && (k_K - l'_R) mod 4 < 2)) || X(l_S, m_S, false, false, 0, bst2_L, bst3_L, k_L, l'_R, insert(true, k_K mod 2, b_R))) && val(!((bst_K && !bst1_K) && !((k_K - l'_R) mod 4 < 2))) || X(l_S, m_S, false, false, 0, bst2_L, bst3_L, k_L, l'_R, b_R)) && val(!(m_S < 2)) || X(l_S, m_S + 1, bst_K, bst1_K, k_K, bst2_L, bst3_L, k_L, l'_R, b_R)) && val(!(m_S < 2)) || X(l_S, m_S + 1, bst_K, bst1_K, k_K, bst2_L, bst3_L, k_L, l'_R, b_R)", var_decl, DATA_SPEC);
  pbes_system::pbes_expression phi = pbes_system::parse_pbes_expression("forall k_S2_00: Nat. val(!(k_S2_00 < m_S && !bst_K && !bst1_K)) || X(l_S, m_S, false, true, (l_S + k_S2_00) mod 4, bst2_L, bst3_L, k_L, l'_R, b_R)", var_decl, DATA_SPEC);
  pbes_system::pbes_expression x = r(phi, sigma);
}

int test_main(int argc, char* argv[])
{
  MCRL2_ATERMPP_INIT(argc, argv)

  test_simplifying_rewriter();
  test_enumerate_quantifiers_rewriter();
  test_substitutions1();
  test_substitutions2();
  test_substitutions3();

  return 0;
}
