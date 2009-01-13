// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file realelm.cpp
/// \brief 

#include <algorithm>
#include <stdlib.h>

#include "mcrl2/atermpp/set_operations.h"
#include "mcrl2/core/messaging.h"
#include "mcrl2/data/find.h"
#include "mcrl2/data/postfix_identifier_generator.h"
#include "mcrl2/data/replace.h"
#include "mcrl2/data/sort_utility.h"

#include "realelm.h"

using namespace atermpp;
using namespace mcrl2;
using namespace mcrl2::core;
using namespace mcrl2::data;
using namespace mcrl2::data::data_expr;
using namespace mcrl2::lps;

static
bool is_inconsistent(const data_expression_list& cond, const rewriter& r);

static inline
data_specification add_ad_hoc_real_equations(const data_specification& specification)
{
  ATermAppl nil = gsMakeNil();
  ATermAppl zero = gsMakeDataExprC0();
  ATermAppl one = gsMakeDataExprC1();
  ATermAppl two = gsMakeDataExprCDub(gsMakeDataExprFalse(), one);
  ATermAppl real_zero = gsMakeDataExprCReal(gsMakeDataExprCInt(zero), one);
  ATermAppl real_one = gsMakeDataExprCReal(gsMakeDataExprCInt(gsMakeDataExprCNat(one)), one);
  ATermAppl real_two = gsMakeDataExprCReal(gsMakeDataExprCInt(gsMakeDataExprCNat(two)), one);
  ATermAppl r = gsMakeDataVarId(gsString2ATermAppl("r"), gsMakeSortExprReal());
  ATermAppl s = gsMakeDataVarId(gsString2ATermAppl("s"), gsMakeSortExprReal());
  ATermAppl t = gsMakeDataVarId(gsString2ATermAppl("t"), gsMakeSortExprReal());
  ATermList rl = ATmakeList1((ATerm) r);
  ATermList rsl = ATmakeList2((ATerm) r, (ATerm) s);
  ATermList rstl = ATmakeList3((ATerm) r, (ATerm) s, (ATerm) t);

  ATermList result = ATmakeList(21,
    // General ad-hoc rewrite rules, should be added in the data implementation
    // r+0=r
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprAdd(r, real_zero), r),
    // 0+r=r
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprAdd(real_zero, r), r),
    // r+-r=0
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprAdd(r, gsMakeDataExprNeg(r)), real_zero),
    // -r+r=0
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprAdd(gsMakeDataExprNeg(r), r), real_zero),
    // r- -s=r+s
    (ATerm) gsMakeDataEqn(rsl,nil,gsMakeDataExprSubt(r,gsMakeDataExprNeg(s)),gsMakeDataExprAdd(r,s)),
    // r-0=r
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprSubt(r, real_zero), r),
    // 0-r=-r
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprSubt(real_zero, r), gsMakeDataExprNeg(r)),
    // r*1=r
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprMult(r, real_one), r),
    // 1*r=r
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprMult(real_one, r), r),
    // r*0=0
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprMult(r, real_zero), real_zero),
    // 0*r=r
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprMult(real_zero, r), real_zero),
    // --r=r
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprNeg(gsMakeDataExprNeg(r)), r),

    // realelm-specific rewrite rules to fully group variables in expressions of
    // the form a1*x1+...+an*xn
    // r+r=2*r
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprAdd(r, r), gsMakeDataExprMult(real_two, r)),
    // -r+-r=-2*r
    (ATerm) gsMakeDataEqn(rl, nil, gsMakeDataExprAdd(gsMakeDataExprNeg(r), gsMakeDataExprNeg(r)), gsMakeDataExprMult(gsMakeDataExprNeg(real_two), r)),
    // s*r+t*r=(s+t)*r
    (ATerm) gsMakeDataEqn(rstl, nil, gsMakeDataExprAdd(gsMakeDataExprMult(s, r), gsMakeDataExprMult(t, r)), gsMakeDataExprMult(gsMakeDataExprAdd(s, t), r)),
    // (r+s)+-s=r
    (ATerm) gsMakeDataEqn(rsl, nil, gsMakeDataExprAdd(gsMakeDataExprAdd(r,s), gsMakeDataExprNeg(s)), r),
    // (r+-s)+s=r
    (ATerm) gsMakeDataEqn(rsl, nil, gsMakeDataExprAdd(gsMakeDataExprAdd(r,gsMakeDataExprNeg(s)), s), r),
    // -(r+s)=-r+-s
    (ATerm) gsMakeDataEqn(rsl, nil, gsMakeDataExprNeg(gsMakeDataExprAdd(r,s)), gsMakeDataExprAdd(gsMakeDataExprNeg(r), gsMakeDataExprNeg(s))),
    // -(r-s)=-r+s
    (ATerm) gsMakeDataEqn(rsl, nil, gsMakeDataExprNeg(gsMakeDataExprSubt(r,s)), gsMakeDataExprAdd(gsMakeDataExprNeg(r), s)), 
    // (r+s)/t=r/t+s/t
    (ATerm) gsMakeDataEqn(rstl,nil,gsMakeDataExprDivide(gsMakeDataExprAdd(r,s),t),gsMakeDataExprAdd(gsMakeDataExprDivide(r,t),gsMakeDataExprDivide(s,t))),
    // (r-s)/t=r/t-s/t
    (ATerm) gsMakeDataEqn(rstl,nil,gsMakeDataExprDivide(gsMakeDataExprSubt(r,s),t),gsMakeDataExprSubt(gsMakeDataExprDivide(r,t),gsMakeDataExprDivide(s,t)))
  );

  return set_equations(specification, specification.equations() + data_equation_list(result));
}

// Custom replace functions
// Needed as the replace functions of the data library do not
// recurse into data expressions
template <typename ReplaceFunction>
struct realelm_replace_data_expressions_helper
{
  const ReplaceFunction& r_;

  realelm_replace_data_expressions_helper(const ReplaceFunction& r)
    : r_(r)
  {}

  std::pair<atermpp::aterm_appl, bool> operator()(atermpp::aterm_appl t) const
  {
    if (is_sort_expression(t))
    {
      return std::pair<atermpp::aterm_appl, bool>(t, false); // do not continue the recursion
    }
    else if (is_data_expression(t))
    {
      data_expression new_t = r_(t);
      if(t == new_t)
      {
        return std::pair<atermpp::aterm_appl, bool>(t, true); // continue the recursion
      }
      else
      {
        return std::pair<atermpp::aterm_appl, bool>(new_t, false); // do not continue the recursion
      }
    }
    else
    {
      return std::pair<atermpp::aterm_appl, bool>(t, true); // continue the recursion
    }
  }
};

/// \cond INTERNAL_DOCS
template <typename MapContainer>
struct realelm_map_replace_helper
{
  const MapContainer& replacements_;

  /// \brief Constructor.
  ///
  realelm_map_replace_helper(const MapContainer& replacements)
    : replacements_(replacements)
  {}

  /// \brief Returns s if a substitution of the form t := s is present in the replacement map,
  /// otherwise t.
  ///
  data_expression operator()(const data_expression& t) const
  {
    typename MapContainer::const_iterator i = replacements_.find(t);
    if (i == replacements_.end())
    {
      return atermpp::aterm_appl(t);
    }
    else
    {
      return i->second;
    }
  }
};

template <typename Term, typename ReplaceFunction>
Term realelm_replace_data_expressions(Term t, ReplaceFunction r)
{
  return atermpp::partial_replace(t, realelm_replace_data_expressions_helper<ReplaceFunction>(r));
}

template <typename Term, typename MapContainer>
Term realelm_data_expression_map_replace(Term t, const MapContainer& replacements)
{
  return realelm_replace_data_expressions(t, realelm_map_replace_helper<MapContainer>(replacements));
}
// End of replace substitute

/// \brief Integer exponentiation
/// \param base Base of the exponentiation
/// \param exponent Exponent of the exponentiation
/// \ret base^exponent
static
unsigned long pow(unsigned long base, unsigned long exponent)
{
  if(exponent == 0)
  {
    return 1;
  }
  else
  {
    div_t q = div(exponent, 2);
    if(q.rem == 0)
    {
      return pow(base * base, q.quot);
    }
    else
    {
      assert(q.rem == 1);
      return base * pow(base, exponent - 1);
    }
  }
}

/// \brief Retrieve the left hand side of a data expression
/// \param e A data expression
/// \pre e is a data application d(x,y) with two arguments
/// \ret x
static inline
data_expression lhs(const data_expression e)
{
  assert(is_data_application(e));
  data_expression_list arguments = static_cast<const data_application&>(e).arguments();
  assert(arguments.size() == 2);
  return *(arguments.begin());
}

/// \brief Retrieve the right hand side of a data expression
/// \param e A data expression
/// \pre e is a data application d(x,y) with two arguments
/// \ret y
static inline
data_expression rhs(const data_expression e)
{
  assert(is_data_application(e));
  data_expression_list arguments = static_cast<const data_application&>(e).arguments();
  assert(arguments.size() == 2);
  return *(++arguments.begin());
}

/// \brief returns the number zero of sort Real
/// \ret the data_expression representing zero
static inline data_expression real_zero()
{
  return gsMakeDataExprReal_int(0);
}

/// \brief Determine whether a data expression is a constant expression
///        encoding a number
/// \param e A data expression
/// \ret true iff e is a numeric constant
static inline
bool is_number(const data_expression e)
{
  return core::detail::gsIsDataExprC0(e) ||
         core::detail::gsIsDataExprCDub(e) ||
         core::detail::gsIsDataExprC1(e) ||
         core::detail::gsIsDataExprCNat(e) ||
         core::detail::gsIsDataExprCNeg(e) ||
         core::detail::gsIsDataExprCInt(e) ||
         core::detail::gsIsDataExprCReal(e);
}

/// \brief Determine wheter a number is negative
/// \param e A data expression
/// \ret true iff e is -e' or @rational(-e',e'')
static inline
bool is_negative(const data_expression e)
{
  return is_negate(e) || gsIsDataExprCNeg(e) || (gsIsDataExprCReal(e) && is_negative(lhs(e)));
}

/// \brief Determine whether a data expression is an inequality
/// \param e A data expression
/// \ret true iff e is a data application of ==, <, <=, > or >= to
///      two arguments.
static inline
bool is_inequality(const data_expression& e)
{
  return is_equal_to(e) || is_less(e) || is_less_equal(e) || is_greater(e) ||
         is_greater_equal(e);
}

static 
data_expression negate_less(const data_expression &e)
{ if (is_less(e))
  { return data_expression(gsMakeDataExprLTE(rhs(e),lhs(e)));
  }
  if (is_less_equal(e))
  { return data_expression(gsMakeDataExprLT(rhs(e),lhs(e)));
  }
  if (is_greater(e))
  { return data_expression(gsMakeDataExprLTE(lhs(e),rhs(e)));
  }
  if (is_greater_equal(e))
  { return data_expression(gsMakeDataExprLT(lhs(e),rhs(e)));
  }

  std::cerr << "WHY " << pp(e) << "\n";
  assert(false); // All cases where covered above.
  return data_expression();
}

/// \brief Determine whether the variables in two data expressions are lexicographically ordered.
/// \param e1 A data expression containing a single variable
/// \param e2 A data expression also containing at most a single
/// \pre e1 and e2 are either a negation, multiplication with a number as left hand
///      side, a data variable or a constant
/// \ret true iff e1 <= e2, where <= is lexicographic order on the variable name. An expression
/// without a variable is considered the smallest.
static
bool less_or_equal(const data_expression& e1, const data_expression& e2)
{
  // gsDebugMsg("less or equal %P, %P\n", (ATermAppl)e1, (ATermAppl)e2);
  if(is_negate(e1))
  {
    return less_or_equal(*(static_cast<const data_application&>(e1).arguments().begin()), e2);
  }
  else if(is_negate(e2))
  {
    return less_or_equal(e1, *(static_cast<const data_application&>(e2).arguments().begin()));
  }
  else if (is_multiplies(e1))
  {
    return less_or_equal(rhs(e1), e2);
  }
  else if (is_multiplies(e2))
  {
    return less_or_equal(e1, rhs(e2));
  }
  else if (gsIsDataExprCReal(e1))
  {
    return less_or_equal(lhs(e1), e2);
  }
  else if (gsIsDataExprCReal(e2))
  {
    return less_or_equal(e1, lhs(e2));
  }
  else if (gsIsDataExprCInt(e1))
  {
    return true;
  }
  else if (gsIsDataExprCInt(e2) )
  {
    return false;
  }

  // gsDebugMsg("e1 = %s, e2 = %s\n", (ATermAppl)e1, (ATermAppl)e2);
  assert(is_data_variable(e1) && is_data_variable(e2));
  return (static_cast<const data_variable&>(e1).name() <= static_cast<const data_variable&>(e2).name());
}

/// \brief Merge two sorted lists of data expressions
/// \param l1 A list of data expressions
/// \param l2 A list of data expressions
/// \pre l1 and l2 are sorted
/// \ret The sorted list of data expressions consisting of all elements in l1
///      and l2
static
data_expression_list merge(const data_expression_list& l1, const data_expression_list& l2)
{
  data_expression_list r;
  data_expression_list::iterator i = l1.begin();
  data_expression_list::iterator j = l2.begin();
  while(i != l1.end() && j != l2.end())
  {
    if(less_or_equal(*i, *j))
    {
      r = push_front(r, *i++);
    }
    else
    {
      r = push_front(r, *j++);
    }
  }
  while(i != l1.end())
  {
    r = push_front(r, *i++);
  }
  while(j != l2.end())
  {
    r = push_front(r, *j++);
  }
  r = reverse(r);
  return r;
}

/// \brief Sort a data expression
/// \param e A data expression of the form c1 * x1 + ... + cn * xn
/// \ret The list of data expressions ci * xi, such that it is sorted
///      in lexicographic order of the names of xi
static inline
data_expression_list sort(const data_expression& e)
{
  data_expression_list r;
  if (is_plus(e))
  {
    r = merge(sort(lhs(e)), sort(rhs(e)));
  }
  else
  {
    r = push_front(r, e);
  }
  return r;
}

/// \brief Order the variables in the lhs of an inequality, using a lexicographic order.
/// \param inequality An inequality
/// \param r A rewriter
/// \param negated If initially true, it allows to negate the data expression. If it results
//         in true, the data expression has been negated.
/// \ret A version of the inequality in which the variables occur in
///      lexicographically sorted order.
static inline
data_expression order_lhs_inequality(const data_expression left, const rewriter& r,bool &negated)
{ 
  data_expression_list sorted = sort(left);
  if (sorted.empty())
  { return real_zero();
  }
  if (negated)
  { negated=is_negate(sorted.front());
  }

  data_expression result = real_zero();
  
  for(data_expression_list::iterator j = sorted.begin(); j != sorted.end(); ++j)
  {
    result = plus(result, *j);
  }
  if (negated)
  { result=negate(result);
  }
  return r(result);
}

/// \brief Split constant and variable parts of a data expression
/// \param e A data expression of the form c1 * x1 + ... + cn * xn + d1 + ... +
///          dm, where ci and di are constants and xi are variables. Constants
///          and variables may occur mixed.
/// \ret The pair (c1 * x1 + ... + cn * xn, d1 + ... + dm)
static
std::pair<data_expression, data_expression> split_variables_and_constants(const data_expression& e)
{
  // gsDebugMsg("Splitting constants and variables in %P\n", (ATermAppl)e);
  std::pair<data_expression, data_expression> result;
  if(is_plus(e))
  {
    std::pair<data_expression, data_expression> left = split_variables_and_constants(lhs(e));
    std::pair<data_expression, data_expression> right = split_variables_and_constants(rhs(e));
    result = std::make_pair(plus(left.first, right.first), plus(left.second, right.second));
  }
  else if (is_minus(e))
  {
    std::pair<data_expression, data_expression> left = split_variables_and_constants(lhs(e));
    std::pair<data_expression, data_expression> right = split_variables_and_constants(rhs(e));
    result = std::make_pair(plus(left.first, negate(right.first)), plus(left.second, negate(right.second)));
  }
  else if (is_negate(e))
  {
    data_expression argument = *static_cast<const data_application&>(e).arguments().begin();
    if(is_plus(argument))
    {
      result = split_variables_and_constants(plus(negate(lhs(argument)), negate(rhs(argument))));
    }
    else if(is_minus(argument))
    {
      result = split_variables_and_constants(plus(negate(lhs(argument)), rhs(argument)));
    }
    else if(is_number(argument))
    {
      result = std::make_pair(real_zero(), e);
    }
    else
    {
      result = std::make_pair(e, real_zero());
    }
  }
  else if (gsIsDataExprCReal(e) && !is_number(*static_cast<const data_application&>(e).arguments().begin()))
  {
    result = std::make_pair(e, real_zero());
  }
  else if (is_multiplies(e) || is_data_variable(e))
  {
    result = std::make_pair(e, real_zero());
  }
  else
  {
    assert(is_number(e));
    result = std::make_pair(real_zero(), e);
  }
  // gsDebugMsg("split version: left = %P, right = %P\n", (ATermAppl)result.first, (ATermAppl)result.second);
  return result;
}

/// \brief Returns a list of all real variables in l
/// \param l a list of data variables
/// \ret The list of all v in l such that v.sort() == real()
static inline
data_variable_list get_real_variables(const data_variable_list& l)
{
  data_variable_list r;
  for(data_variable_list::const_iterator i = l.begin(); i != l.end(); ++i)
  {
    if(i->sort() == sort_expr::real())
    {
      r = push_front(r, *i);
    }
  }
  return r;
}

/// \brief Returns a list of all nonreal variables in l
/// \param l a list of data variables
/// \ret The list of all v in l such that v.sort() != real()
static inline
data_variable_list get_nonreal_variables(const data_variable_list& l)
{
  data_variable_list r;
  for(data_variable_list::const_iterator i = l.begin(); i != l.end(); ++i)
  {
    if(i->sort() != sort_expr::real())
    {
      r = push_front(r, *i);
    }
  }
  return r;
}

/// \brief Returns a list of all real expressions in l
/// \param l a list of data expressions
/// \ret The list of all e in l such that e.sort() == real()
static inline
data_expression_list get_real_expressions(const data_expression_list& l)
{
  data_expression_list r;
  for(data_expression_list::const_iterator i = l.begin(); i != l.end(); ++i)
  {
    if(i->sort() == sort_expr::real())
    {
      r = push_front(r, *i);
    }
  }
  return r;
}

/// \brief Returns a list of all nonreal expressions in l
/// \param l a list of data expressions
/// \ret The list of all e in l such that e.sort() != real()
static inline
data_expression_list get_nonreal_expressions(const data_expression_list& l)
{
  data_expression_list r;
  for(data_expression_list::const_iterator i = l.begin(); i != l.end(); ++i)
  {
    if(i->sort() != sort_expr::real())
    {
      r = push_front(r, *i);
    }
  }
  return r;
}

/// \brief Returns a list of all real assignments in l
/// \param l a list of data assignments
/// \ret The list of all x := e in l such that x.sort() == e.sort() == real()
static inline
data_assignment_list get_real_assignments(const data_assignment_list& l)
{
  data_assignment_list r;
  for(data_assignment_list::const_iterator i = l.begin(); i != l.end(); ++i)
  {
    if(i->lhs().sort() == sort_expr::real())
    {
      r = push_front(r, *i);
    }
  }
  return r;
}

/// \brief Returns a list of all nonreal assignments in l
/// \param l a list of data assignments
/// \ret The list of all x := e in l such that x.sort() == e.sort() != real()
static inline
data_assignment_list get_nonreal_assignments(const data_assignment_list& l)
{
  data_assignment_list r;
  for(data_assignment_list::const_iterator i = l.begin(); i != l.end(); ++i)
  {
    if(i->lhs().sort() != sort_expr::real())
    {
      r = push_front(r, *i);
    }
  }
  return r;
}

/// \brief Normalize a pair of terms.
/// \details Transform the inequality in e such that the left hand side only
///        contains multiples of variables and the right hand side contains a
///        single constant. 
/// \param e A data expression
/// \param r A rewriter
/// \param preserve_ordering If preserver_ordering is true the left and right hand
//         side are not multiplied by a negative number. For the resulting t' and u' 
//         it holds that t'<u' iff t<u. If preserve_ordering is false, then it holds
//         that t'<u' iff t<u or u<t.
/// \pre e is an inequality
/// \ret An inequality e' equivalent to e, such that the left hand side is of
///      the form c1 * e1 + ... + cn * en === d, with === one of ==, <, <=, >,
///      >=. In case there are no variables left, the left hand side is the 
///      expression 0.
void normalize_pair(data_expression& t, data_expression &u, const rewriter& r, const bool preserve_ordering)
{

  std::pair<data_expression, data_expression> left = split_variables_and_constants(r(t));
  std::pair<data_expression, data_expression> right = split_variables_and_constants(r(u));
  // Variables are moved to the left, constants to the right of each pair.

  // gsDebugMsg("left.first = %P, left.second = %P, right.first = %P, right.second = %P\n", (ATermAppl)left.first, (ATermAppl)left.second, (ATermAppl)right.first, (ATermAppl)right.second);

  data_expression new_left = left.first;
  while(is_plus(right.first))
  {
    new_left = plus(new_left, negate(rhs(right.first)));
    right.first = lhs(right.first);
  }
  new_left = r(plus(new_left, negate(right.first)));

  data_expression new_right = right.second;
  while(is_plus(left.second))
  {
    new_right = plus(new_right, negate(rhs(left.second)));
    left.second = lhs(left.second);
  }

  bool negateable=!preserve_ordering;
  t=order_lhs_inequality(new_left,r,negateable);
  if (negateable)
  { u = r(plus(negate(new_right), left.second));
  }
  else
  { u = r(plus(new_right, negate(left.second)));
  }
}


/// \brief Normalize an equality.
/// \details Transform the inequality in e such that the left hand side only
///        contains multiples of variables and the right hand side contains a
///        single constant. 
/// \param e A data expression
/// \param r A rewriter
/// \pre e is an inequality
/// \ret An inequality e' equivalent to e, such that the left hand side is of
///      the form c1 * e1 + ... + cn * en === d, with === one of ==, <, <=, >,
///      >=. In case there are no variables left, the left hand side is the 
///      expression 0.
static inline
data_expression normalize_inequality(const data_expression& e, const rewriter& r)
{

  assert(is_inequality(e));
  // gsDebugMsg("Normalizing inequality %P\n", (ATermAppl)e);

  data_application d = static_cast<const data_application&>(e);
  data_expression left=lhs(d);
  data_expression right=rhs(d);
  // procedure below changes left and right into normal form.
  normalize_pair(left,right,r,true);
  data_expression result = data_application(d.head(), make_list(left, right));
  return result;
}



/// \brief Normalize the inequalities in l. 
/// \details See normalize_inequality for more details. The result
///          can be an empty list, if the l was equivalent to true.
///          In case the input was inconsistent, the result can be 
///          [false]
/// \param l A list of data expressions
/// \param r A rewriter
/// \ret The list of data expressions l' equivalent to l, but with all
///      inequalities normalized.
static
data_expression_list normalize_inequalities(const data_expression_list& l, const rewriter& r)
{ // std::cerr << "Normalize_inequalities " << pp(l) << "\n";
  data_expression_list result;
  for(data_expression_list::iterator i = l.begin(); i != l.end() ; ++i)
  {
    data_expression inequality = r(*i);
    if (is_inequality(inequality))
    {
      inequality = r(normalize_inequality(inequality, r));
      if (inequality==true_())
      { /* do nothing */
      }
      else if (inequality==false_())
      {
        return push_front(data_expression_list(),false_());
      }
      else if (!find_data_expression(result, inequality))
      {
        result = push_front(result, inequality);
      }
    }
    else
    {
      assert(inequality==false_() || inequality==true_());
      if (inequality==true_())
      { /* do nothing */
      }
      else if (inequality==false_())
      {
        return push_front(data_expression_list(),false_());
      }
    }
  }
  return result;
}

/// \brief Splits a conjunct in a conjunct ranging over inequalities of reals
///        and a conjunct ranging over other expressions.
/// \param e A data expression
/// \pre e is a conjunct
/// \ret Pair (r,x) such that r is a list of expressions over real numbers and
///      x is a list of expressions not over real numbers.
static
std::pair<data_expression_list, data_expression_list> split_conjunct(data_expression e)
{
  data_expression_list real_conjuncts;
  data_expression_list nonreal_conjuncts;

  while(is_and(e))
  {
    std::pair<data_expression_list, data_expression_list> c = split_conjunct(lhs(e));
    real_conjuncts = real_conjuncts + c.first;
    nonreal_conjuncts = nonreal_conjuncts + c.second;
    e = rhs(e);
  }

  if(is_inequality(e) && (lhs(e).sort() == sort_expr::real() || rhs(e).sort() == sort_expr::real()))
  {
    real_conjuncts = push_front(real_conjuncts, e);
  }
  else
  {
    nonreal_conjuncts = push_front(nonreal_conjuncts, e);
  }
  return std::make_pair(real_conjuncts, nonreal_conjuncts);
}

/// \brief Removes constant real numbers from the nextstate vectors of all
///        summands by introducing auxiliary sum variables
/// \param s A data specification
/// \param variable_generator A variable generator
/// \ret A specification equivalent to s in which no real constants occur in
///      nextstate vectors.
static
specification remove_real_constants_from_nextstate(specification s, identifier_generator& variable_generator)
{
  linear_process lps = s.process();
  summand_list sl;
  for(summand_list::const_iterator i = lps.summands().begin(); i != lps.summands().end(); ++i)
  {
    data_assignment_list nextstate;
    data_expression condition = i->condition();
    data_variable_list sum_variables = i->summation_variables();
    for(data_assignment_list::const_iterator j = i->assignments().begin(); j != i->assignments().end(); ++j)
    {
      if(j->rhs().sort() == sort_expr::real() && is_number(j->rhs()))
      {
        data_expression right = j->rhs();
        data_variable var(variable_generator("y"), sort_expr::real());
        condition = and_(condition, equal_to(var, right));
        nextstate = push_front(nextstate, data_assignment(j->lhs(), var));
        sum_variables = push_front(sum_variables, var);
      }
      else
      {
        nextstate = push_front(nextstate, *j);
      }
    }
    nextstate = reverse(nextstate);
    summand s(sum_variables, condition, i->is_delta(), i->actions(), nextstate);
    sl = push_front(sl, s);
  }
  sl = reverse(sl);
  lps = set_summands(lps, sl);
  s = set_lps(s, lps);
  return s;

}

/// \brief Normalize all inequalities in the summands of the specification
/// \details All real constants occurring in the nextstate vectors of all
///          summands are replaced by a summation variable (see
///          remove_real_constants_from_nextstate). Furthermore, the parts of the
///          conditions ranging over real numbers and the other parts of the conditions
///          are separated and the part ranging over real numbers is normalized
///          (see normalize_inequalities for details).
/// \param s A data specification
/// \param r A rewriter
/// \param variable_generator A variable generator
/// \ret A specification equivalent to s, but of which all inequalities have
///      been normalized.
static
specification normalize_specification(specification s, rewriter& r, identifier_generator& variable_generator)
{
  //s = remove_real_constants_from_nextstate(s, variable_generator);
  linear_process lps = s.process();
  summand_list sl;
  for(summand_list::const_iterator i = lps.summands().begin(); i != lps.summands().end(); ++i)
  {
    std::pair<data_expression_list, data_expression_list> conjuncts = split_conjunct(i->condition());

    data_expression non_real = join_and(conjuncts.second.begin(), conjuncts.second.end());
    data_expression_list real_inequalities = normalize_inequalities(conjuncts.first, r);
    data_expression real = join_and(real_inequalities.begin(), real_inequalities.end());
    data_expression condition = and_(non_real, real);

    summand t = set_condition(*i, condition);
    sl = push_front(sl, set_condition(*i, condition));
  }
  sl = reverse(sl);
  lps = set_summands(lps, sl);

  s = set_lps(s, lps);
  return s;
}

/// \brief Determine the inequalities ranging over real numbers in a data expression.
/// \param e A data expression
/// \param inequalities A list of inequalities
/// \post inequalities contains all inequalities ranging over real numbers in e.
static
void determine_real_inequalities(const data_expression& e, data_expression_list& inequalities)
{
  if (is_and(e))
  {
    determine_real_inequalities(lhs(e), inequalities);
    determine_real_inequalities(rhs(e), inequalities);
  }
  else if ((is_equal_to(e) || is_less(e) || is_less_equal(e)) && rhs(e).sort() == sort_expr::real())
  {
    inequalities = push_front(inequalities, e);
  }
}

/// \brief Add postponed inequalities to variable context
/// \param inequalities_to_add The pairs of inequalities to be added.
/// \param context A variable context
/// \param variable_generator A fresh variable generator
/// \post All inequalities in l are in the context
/// \ret true iff a variable has been added to the context
static void add_postponed_inequalities_to_context(
                const atermpp::vector < data_expression > &inequalities_to_add,
                std::vector < summand_information > &summand_info,
                context_type& context, 
                const rewriter& r, 
                identifier_generator& variable_generator)
{ assert(inequalities_to_add.size() % 2==0);
  for(atermpp::vector < data_expression > ::const_iterator i=inequalities_to_add.begin();
                        i!=inequalities_to_add.end(); i=i+2)
  {
    data_variable xi(variable_generator("xi"), sort_identifier("Comp"));
    context.push_back(real_representing_variable(xi,*i, *(i+1)));
    if (core::gsVerbose)
    { gsVerboseMsg("Introduced variable %s for < %s, %s >\n", pp(xi).c_str(), pp(*i).c_str(), pp(*(i+1)).c_str());
    }
    // print_debug(summand_info);
    for(std::vector < summand_information >::iterator j = summand_info.begin();
                       j != summand_info.end(); ++j)
    { j->add_a_new_next_state_argument(context,r);
    }
    // print_debug(summand_info);
  }
}

/// \brief Add inequalities to variable context
/// \param l A list of inequalities
/// \param context A variable context
/// \param variable_generator A fresh variable generator
/// \post All inequalities in l are in the context
/// \ret true iff a variable has been added to the context
static void add_inequalities_to_context_postponed(
                atermpp::vector < data_expression > &inequalities_to_add,
                data_expression_list l, 
                context_type& context, 
                const rewriter& r)
{ assert(inequalities_to_add.size() % 2==0);
  for(data_expression_list::const_iterator i = l.begin(); i != l.end(); ++i)
  {
    if (is_inequality(*i))
    { data_expression left = lhs(*i);
      data_expression right = rhs(*i);
      normalize_pair(left,right,r,false);

      // if(is_negative(right) || (right == real_zero() && is_negate(left)))
      // {
      //  left = r(negate(left));
      //  right = r(negate(right));
      //}
      if (left!=real_zero())
      { bool pair_is_new(true);
        for(context_type::const_iterator c=context.begin() ; c!=context.end() && pair_is_new; ++c)
        { if ((c->get_lowerbound()==left) && (c->get_upperbound()==right))
          { pair_is_new=false;
          }
        }
        if (pair_is_new)
        { for(atermpp::vector < data_expression >::const_iterator j=inequalities_to_add.begin() ; 
                     j!=inequalities_to_add.end() && pair_is_new ; j=j+2)
          { if ((*j==left) && (*(j+1)==right))
            { pair_is_new=false;
            }
          }
          if(pair_is_new)
          {
            inequalities_to_add.push_back(left);
            inequalities_to_add.push_back(right);
          }
        }
      }
    }
  } 
}


/// \brief Try to eliminate variables from a system of inequalities using Gauss
///        elimination.
/// \param inequalities A list of inequalities over real numbers
/// \param variables A list of variables to be eliminated
/// \param r A rewriter.
/// \post variables contains the list of variables that have not been eliminated
/// \ret The system of normalized inequalities after Gauss eliminatation.
static
data_expression_list gauss_elimination(
                         data_expression_list inequalities, 
                         data_variable_list &variables, 
                         const rewriter& r)
{
  // gsDebugMsg("Trying to eliminate variables %P from system %P using gauss elimination\n", (ATermList)variables, (ATermList)inequalities);

  // First find out whether there are variables that occur in an equality, so
  // that we can perform gauss elimination.
  data_variable_list eliminated_variables;
  for(data_variable_list::const_iterator i = variables.begin(); i != variables.end(); ++i)
  {
    for(data_expression_list::const_iterator j = inequalities.begin(); j != inequalities.end(); ++j)
    {
      if(is_equal_to(*j) && find_data_variable(*j, *i))
      {
        // There is an equality in which the variable occurs in the left hand
        // side, perform gauss elimination, and break the loop.
        data_expression left = lhs(*j);
        data_expression right = rhs(*j);
        while(is_plus(left))
        {
          if(find_data_variable(lhs(left), *i))
          {
            right = minus(right, rhs(left));
            left = lhs(left);
          }
          else
          {
            right = minus(right, lhs(left));
            left = rhs(left);
          }
          if(is_multiplies(left))
          {
            // Divide out the factor in the right hand side
            data_expression factor = lhs(left);
            left = rhs(left);
            right = gsMakeDataExprDivide(right, factor);
          }
          if(is_negate(left))
          {
            left = r(negate(left));
            right = negate(right);
          }
        }

        if(is_multiplies(left))
        {
          // Divide out the factor in the right hand side
          data_expression factor = lhs(left);
          left = rhs(left);
          right = gsMakeDataExprDivide(right, factor);
        }
        if(is_negate(left))
        {
          left = r(negate(left));
          right = r(negate(right));
        }

        // left := right is the desired substitution, carry out in the other
        // (in)equalities
        atermpp::map<data_expression, data_expression> replacements;
        replacements[left] = right;

        data_expression_list new_inequalities;
        for(data_expression_list::const_iterator k = inequalities.begin(); k != inequalities.end(); ++k)
        {
          if(*k != *j)
          {
            new_inequalities = push_front(new_inequalities, r(realelm_data_expression_map_replace(*k, replacements)));
          }
        }
        inequalities = new_inequalities;
        eliminated_variables = push_front(eliminated_variables, *i);
        break;
      }
    }
  }

  // Remove the variables that we have eliminated from the list of variables
  variables = term_list_difference(variables, eliminated_variables);

  // gsDebugMsg("Gauss elimination eliminated variables %P, resulting in the system %P\n", 
  //                       (ATermList)eliminated_variables, (ATermList)inequalities);

  return normalize_inequalities(inequalities, r);
}

/// \brief Remove a variable from an inequality
/// \param variable A variable
/// \param inequality An inequality over real numbers
/// \pre inequality is an inequality
/// \ret The inequality from which variable has been removed
static
data_expression remove_variable(const data_variable& variable, const data_expression& inequality)
{
  assert(is_inequality(inequality));

  // gsDebugMsg("Removing variable %P from inequality %P\n", (ATermAppl)variable, (ATermAppl)inequality);

  data_expression left = lhs(inequality);
  data_expression new_left = real_zero();
  while(is_plus(left))
  {
    // gsDebugMsg("left = %P is a plus expression\n", (ATermAppl)left);
    if(is_multiplies(rhs(left)))
    {
      data_expression factor = lhs(rhs(left));
      new_left = gsMakeDataExprDivide(plus(new_left, lhs(left)), factor);
      return data_application(static_cast<const data_application&>(inequality).head(), make_list(new_left, gsMakeDataExprDivide(rhs(inequality), factor)));
    }
    else if (rhs(left) == variable || rhs(left) == negate(static_cast<const data_expression&>(variable)))
    {
      return data_application(static_cast<const data_application&>(inequality).head(), make_list(plus(new_left, lhs(left)), rhs(inequality)));
    }
    else
    {
      new_left = plus(new_left, rhs(left));
      left = lhs(left);
    }
  }

  // gsDebugMsg("left = %P\n", (ATermAppl)left);

  if(is_negate(left))
  {
    data_expression argument = *static_cast<const data_application&>(left).arguments().begin();
    if(is_plus(argument))
    {
      data_expression p = plus(negate(lhs(argument)), negate(rhs(argument)));
      return remove_variable(variable, data_application(static_cast<const data_application&>(inequality).head(), make_list(p, rhs(inequality))));
    }
  }
  if (left == variable || left == negate(static_cast<const data_expression&>(variable)))
  {
    return data_application(static_cast<const data_application&>(inequality).head(), make_list(new_left, rhs(inequality)));
  }

  gsErrorMsg("cannot remove variable %P from %P\n", (ATermAppl)variable, (ATermAppl)inequality);

  assert(false);
  return data_expression(); // Never reached, silence gcc 4.1.2
}

/// \brief Group inequalities by positive, 0 and negative occurrences of a
///        variable.
/// \param v A variable
/// \param inequalities A list of inequalities
/// \param positive_occurrences an empty list
/// \param zero_occurrences an empty list
/// \param negative_occurrences and empty list
/// \post positive_occurrences contains all inequalities in which c * v occurs,
///       zero_occurrences contains all inequalities in which v does not occur,
///       negative_occurrences contains all inequalities in which -c * v occurs.
static
void group_inequalities(const data_variable& v, const data_expression_list& inequalities,
                        data_expression_list& positive_occurrences,
                        data_expression_list& zero_occurrences,
                        data_expression_list& negative_occurrences)
{
  assert(positive_occurrences.empty());
  assert(zero_occurrences.empty());
  assert(negative_occurrences.empty());
  for(data_expression_list::iterator j = inequalities.begin(); j != inequalities.end(); ++j)
  {
    if(find_data_variable(*j, v))
    {
      if(find_data_expression(*j, negate(static_cast<const data_expression&>(v))))
      {
        negative_occurrences = push_front(negative_occurrences, *j);
      }
      else
      {
        positive_occurrences = push_front(positive_occurrences, *j);
      }
    }
    else
    {
      zero_occurrences = push_front(zero_occurrences, *j);
    }
  }
}

/* void print_debug(const std::vector <summand_information> &s)
{
  for(std::vector < summand_information >::const_iterator i = s.begin();
                       i != s.end(); ++i)
  {
     atermpp::vector<mcrl2::data::data_expression> new_values_for_xi_variables = i->get_new_values_for_xi_variables();
     for(atermpp::vector < mcrl2::data::data_expression > ::const_iterator
                                value_for_xi=new_values_for_xi_variables.begin();
                                value_for_xi!=new_values_for_xi_variables.end();
                                ++value_for_xi)
      {
        assert(is_data_expression(*value_for_xi) || *value_for_xi == data_expression());
        if ((*value_for_xi)==data_expression())
        { 
          std::cerr << "Pvalue is undefined ";
        }
        else
        { assert(is_data_expression(*value_for_xi));
          std::cerr << "Pvalue is " << pp(*value_for_xi);
        }
        std::cerr << " A " << &(*value_for_xi) << " ";
      }
      std::cerr << "\n";
  }
} */


/// \brief remove every redundatn inequality from inequalities.

static
data_expression_list remove_redundant_inequalities(const data_expression_list &inequalities, const rewriter &r)
{
  if (inequalities.empty())
  { return inequalities;
  }

  // If false is among the inequalities, [false] is the minimal result.
  if(std::find(inequalities.begin(), inequalities.end(), false_()) != inequalities.end())
  { return push_front(data_expression_list(),false_());
  }

  data_expression_list resulting_inequalities(inequalities);
  for(data_expression_list::iterator e1=inequalities.begin();
                            e1!=inequalities.end(); ++e1)
  { // Create a list of inequalities where e1 is negated.
    if (is_equal_to(*e1))
    { // Do nothing.
    }
    else 
    { data_expression_list inconsistent_inequalities;
      for(data_expression_list::iterator e2=resulting_inequalities.begin();
                              e2!=resulting_inequalities.end(); ++e2)
      { // copy resulting inequalities with e1 negated.
        inconsistent_inequalities=push_front(inconsistent_inequalities,(*e1==*e2?negate_less(*e2):*e2));
      }
      if (is_inconsistent(inconsistent_inequalities,r))
      {  
        // copy resulting inequalities with e1 negated.
        data_expression_list tempresult;
        for(data_expression_list::iterator e2=resulting_inequalities.begin();
                                e2!=resulting_inequalities.end(); ++e2)
        { if (*e1!=*e2)
          { tempresult=push_front(tempresult,*e2);
          }
        }
        resulting_inequalities=tempresult;
      }
    }
  }
  return resulting_inequalities;
}


/// \brief Eliminate variables from inequalities using Gauss elimination and
///        Fourrier-Motzkin elimination.
/// \details Deliver a set of inequalities equivalent to exists variables.inequalities.
//           If the resulting list of inequalities is inconsistent, then [false] is 
//           returned. Furthermore, the list of resulting inequalities is minimal in
//           the sense that no individual inequality can be removed, without altering the
//           set of solutions of the inequalities.
/// \param inequalities A list of linear inequalities; the input can also contain linear equations.
/// \param variables The list of variables to be eliminated
/// \param r A rewriter
/// \pre inequalities has been normalized
/// \post All variables in variables have been eliminated, inequalities contains
///       the resulting system of normalized inequalities.
static void fourier_motzkin(data_expression_list& inequalities, 
                     data_variable_list variables, 
                     const rewriter& r)
{
  gsDebugMsg("Starting Fourier-Motzkin elimination on system produced by Gauss elimination on %P\n", (ATermList)inequalities);
  inequalities = gauss_elimination(inequalities, variables, r);
  inequalities = normalize_inequalities(inequalities, r);

  // At this stage, the variables that should be eliminated only occur in
  // inequalities. Group the inequalities into positive, 0, and negative
  // occurrences of each variable, and create a new system.
  for(data_variable_list::iterator i = variables.begin(); i != variables.end(); ++i)
  {
    data_expression_list positive_variables;
    data_expression_list zero_variables;
    data_expression_list negative_variables;

    group_inequalities(*i, inequalities, positive_variables, zero_variables, negative_variables);
    data_expression_list new_inequalities = zero_variables;

    // Variables are grouped, now construct new inequalities as follows:
    // Keep the zero occurrences
    // Combine each positive and negative equation as follows:
    // Given inequalities x1 + bi * x <= ci
    //                   -x1 + bj * x <= cj
    // This is equivalent to bj * x + bi * x <= ci + cj
    for(data_expression_list::iterator j = positive_variables.begin(); j != positive_variables.end(); ++j)
    {
      data_expression positive_inequality = remove_variable(*i, *j);
      // gsDebugMsg("positive inequality: %P\n", (ATermAppl)positive_inequality);
      positive_inequality = normalize_inequality(positive_inequality,r);
      // gsDebugMsg("positive inequality after normalization: %P\n", (ATermAppl)positive_inequality);
      assert(is_inequality(positive_inequality));
      assert(!is_greater(positive_inequality) && !is_greater_equal(positive_inequality));
      for(data_expression_list::iterator k = negative_variables.begin(); k != negative_variables.end(); ++k)
      {
        // gsDebugMsg("combining %P and %P into new inequality\n", (ATermAppl)*j, (ATermAppl)*k);
        data_expression negative_inequality = remove_variable(*i, *k);
        // gsDebugMsg("negative inequality: %P\n", (ATermAppl)negative_inequality);
        negative_inequality = normalize_inequality(negative_inequality, r);
        // gsDebugMsg("negative inequality after normalization: %P\n", (ATermAppl)negative_inequality);
        // Results may not be inequalities any more
        assert(is_inequality(negative_inequality));
        assert(!is_greater(negative_inequality) && !is_greater_equal(negative_inequality));
        data_expression new_inequality;
        if(is_less(positive_inequality) || is_less(negative_inequality))
        {
          new_inequality = less(plus(lhs(positive_inequality), lhs(negative_inequality)), plus(rhs(positive_inequality), rhs(negative_inequality)));
        }
        else
        {
          new_inequality = less_equal(plus(lhs(positive_inequality), lhs(negative_inequality)), plus(rhs(positive_inequality), rhs(negative_inequality)));
        }
        new_inequalities = push_front(new_inequalities, new_inequality);
      }
    }
    inequalities = normalize_inequalities(new_inequalities, r);
  }
  
  inequalities = remove_redundant_inequalities(inequalities,r);
}

/// \brief Compute a concrete inequality given a pair of data expressions and a
///        number
/// \param i a number
/// \param e A pair <x,y> of data expressions
/// \pre 0 <= i <= 2
/// \ret Data expression for the following inequality:
///      x < y if i = 0
///      x = y if i = 1
///      -x < -y if i = 2
static inline
data_expression compute_inequality(unsigned long i, 
                                   const data_expression& lb, 
                                   const data_expression& ub)
{
  if(i == 0)
  {
    return less(lb, ub);
  }
  else if(i == 1)
  {
    return equal_to(lb, ub);
  }
  else
  {
    assert(i == 2);
    return less(negate(lb), negate(ub));
  }
}

/// \brief Compute a list of concrete inequalities given a variable context
/// \param i a number
/// \param context A variable context
/// \ret The list of concrete inequalities corresponding to combination i
static inline
data_expression_list compute_inequalities(unsigned long i, const context_type &context)
{
  data_expression_list result;
  for(context_type::const_iterator j = context.begin(); j != context.end(); ++j)
  {
    div_t q = div(i, 3);
    i = q.quot;
    result = push_front(result, compute_inequality(q.rem, j->get_lowerbound(),j->get_upperbound()));
  }
  return result;
}

/// \brief Apply replacements to a list of inequalities
/// \param inequalities A list of data expressions
/// \param replacements A map of replacements
/// \ret inequalities to which the substitutions in replacements have been
///      applied
data_expression_list data_expression_map_replace_list(const data_expression_list& inequalities, const atermpp::map<data_expression, data_expression>& replacements)
{
  data_expression_list result;
  for(data_expression_list::const_iterator i = inequalities.begin(); i != inequalities.end(); ++i)
  {
    result = push_front(result, realelm_data_expression_map_replace(*i, replacements));
  }
  return result;
}

/// \brief Simplify condition given a context
/// \param cond A list of inequalities
/// \param context A list of inequalities
/// \ret A simplified version of inequalities, assuming context holds.
static
data_expression_list simplify_condition(const data_expression_list& cond, const data_expression_list& context)
{
  data_expression_list result;
  for(data_expression_list::const_iterator i = cond.begin(); i != cond.end(); ++i)
  {
    // Only unique elements in cond
    if(std::find(result.begin(), result.end(), *i) == result.end())
    {
      result = push_front(result, *i);
    }
  }
  return result;
}

/// \brief Determine whether a list of data expressions is inconsistent
/// \param cond A list of inequalities
/// \param r A rewriter
/// \ret true if the system of inequalities can be determined to be
///      inconsistent, false otherwise.
static
bool is_inconsistent(const data_expression_list& cond, const rewriter& r)
{
  data_expression_list result(cond);
  std::set<data_variable> dvs=find_all_data_variables(result);
  data_variable_list dl;
  for(std::set<data_variable>::const_iterator i=dvs.begin(); i!=dvs.end(); i++)
  { dl=push_front(dl, *i);
  }
  fourier_motzkin(result,dl,r);
  // Check if result contains false
  if(std::find(result.begin(), result.end(), false_()) != result.end())
  {
    return true;
  }
  return false;
}
/*   for(data_expression_list::const_iterator i = cond.begin(); i != cond.end(); ++i)
  {
    for(data_expression_list::const_iterator j = cond.begin(); j != cond.end(); ++j)
    {
      if(is_inequality(*i) && is_inequality(*j) && *i != *j && !is_less_equal(*i) && !is_less_equal(*j))
      {
        if(lhs(*i) == lhs(*j))
        {
          if(rhs(*i) == rhs(*j))
          {
            return true;
          }
          else if((is_less(*i) && is_equal_to(*j) && r(less(rhs(*j), rhs(*i))) == false_()) ||
             (is_equal_to(*i) && is_less(*j) && r(less(rhs(*i), rhs(*j))) == false_()))
          {
            return true;
          }
          else if (is_equal_to(*i) && is_equal_to(*j) && rhs(*i) != rhs(*j))
          {
            return true;
          }
        }
        else
        {
          if(rhs(*i) == real_zero() && !is_negate(lhs(*i)) && is_negate(rhs(*j)) &&
             lhs(*i) == r(negate(lhs(*j))))
          {
            if(r(greater(rhs(*j), rhs(*i))) == false_())
            {
              return true;
            }
          }
          else if (rhs(*j) == real_zero() && !is_negate(lhs(*j)) && is_negate(lhs(*i)) &&
            lhs(*j) == r(negate(lhs(*i))))
          {
            if(r(greater(rhs(*i), rhs(*j))) == false_())
            {
              return true;
            }
          }
        }
        // gsDebugMsg("Cannot conclude anything\n");
      }
    }
  }
  return false;
} */

/// \brief Transform a list of inequalities over Reals to expressions over Cond,
///        given a context.
/// \param cond A list of inequalities
/// \param context A variable context
/// \param r A rewriter
/// \ret The list of inequalities corresponding to cond, but expressed in
///      expressions over sort Cond.
static data_expression_list transform_real_to_cond(
              const data_expression_list& cond, 
              context_type& context, 
              rewriter& r)
{
  data_expression_list result;
  for(data_expression_list::const_iterator i = cond.begin(); i != cond.end(); ++i)
  {
    if(is_inequality(*i))
    {
      data_expression left = lhs(*i);
      data_expression right = rhs(*i);
      if(is_negative(right) || (right == real_zero() && is_negate(left)))
      {
        left = r(negate(left));
        right = r(negate(right));
      }
      context_type::const_iterator j;
      // find a j with correct upper and lowerbound.
      for(j=context.begin() ; j!=context.end() ; ++i)
      { if ((j->get_lowerbound()==left) && (j->get_upperbound()==right))
        { break;
        }
      }
      if(j == context.end())
      {
        result = push_front(result, *i);
      }
      else
      {
        if(is_negative(rhs(*i)))
        {
          if(is_less(*i))
          {
            result = push_front(result, static_cast<const data_expression&>(is_larger(j->get_variable())));
          }
          else if(is_less_equal(*i))
          {
            result = push_front(result, or_(static_cast<const data_expression&>(is_larger(j->get_variable())), static_cast<const data_expression&>(is_equal(j->get_variable()))));
          }
          else if(is_equal_to(*i))
          {
            result = push_front(result, static_cast<const data_expression&>(is_equal(j->get_variable())));
          }
          else
          {
            assert(false);
          }
        }
        else
        {
          if(is_less(*i))
          {
            result = push_front(result,static_cast<const data_expression&>(is_smaller(j->get_variable())));
          }
          else if(is_less_equal(*i))
          {
            result = push_front(result, or_(static_cast<const data_expression&>(is_smaller(j->get_variable())), static_cast<const data_expression&>(is_equal(j->get_variable()))));
          }
          else if(is_equal_to(*i))
          {
            result = push_front(result, static_cast<const data_expression&>(is_equal(j->get_variable())));
          }
          else
          {
            assert(false);
          }
        }
      }
    }
  }
  return result;
}

/// \brief Generate a summand
/// \param s A summand
/// \param i A number, denoting the next state
/// \param cond A list of inequalities denoting the real part of the condition
/// \param context A variable context
/// \param r A rewriter
/// \ret The summand corresponding to s with real part of condition cond, and
///      nextstate determined by i.
static
summand generate_summand(const summand_information &summand_info,
                         const data_expression &xi_condition,
                         // unsigned long i, 
                         const data_expression_list &extra_zeta_values,
                         // data_expression_list cond, 
                         context_type& zeta_context, 
                         context_type& complete_context, 
                         rewriter& r,
                         action_label_list &a,
                         identifier_generator& variable_generator,
                         const bool is_may_summand=false)
{ static atermpp::vector < sort_expression_list > protect_against_garbage_collect;
  static std::map < std::pair < std::string, sort_expression_list >, std::string> action_label_map; 
                                         // Used to recall which may actions labels have been
                                         // introduced, in order to re-use them.
  const summand s=summand_info.get_summand();
  std::pair<data_expression_list, data_expression_list> real_nonreal_condition = split_conjunct(s.condition());
  data_expression condition = and_(true_(), join_and(real_nonreal_condition.second.begin(), real_nonreal_condition.second.end()));
  condition=and_(condition,xi_condition);

  data_assignment_list nextstate = get_nonreal_assignments(s.assignments());
  nextstate = reverse(nextstate);
  context_type::const_iterator c_complete = complete_context.begin();
  data_expression_list::iterator extra_zeta_value=extra_zeta_values.begin();

  for(atermpp::vector < data_expression >::const_iterator j = summand_info.get_new_values_for_xi_variables_begin(); 
                j != summand_info.get_new_values_for_xi_variables_end(); ++j,++c_complete)
  { 
    if ((*j)!=data_expression())
    { // We have a preset value for the j'th variable in the next state
      nextstate=push_front(nextstate,data_assignment(c_complete->get_variable(),*j));
    }
    else
    { // We have no preset value for the j'th variable in the next state. So, use the one from extra_zeta_value.
      nextstate=push_front(nextstate,data_assignment(c_complete->get_variable(),*extra_zeta_value));
      ++extra_zeta_value;
    }
  }
  assert(extra_zeta_value==extra_zeta_values.end());
 
  nextstate = reverse(nextstate);

  action_list new_actions=s.actions();
  if ((!s.is_delta()) && is_may_summand)
  { action_list resulting_actions;
    for(action_list::const_iterator i=new_actions.begin();
                 i!=new_actions.end(); i++)
    { // put "_MAY" behind each action, and add its declaration to the action declarations.
      data_expression_list args=i->arguments();
      sort_expression_list sorts=get_sorts(args);
      std::map < std::pair< std::string, sort_expression_list >,
                 std::string> ::iterator action_label_it=
                     action_label_map.find(std::pair< std::string, sort_expression_list >
                                         (std::string(i->label().name()),sorts));
      if (action_label_it==action_label_map.end())
      { std::string may_action_label=variable_generator(std::string(i->label().name())+"_MAY");
        std::pair< std::string, sort_expression_list > p(std::string(i->label().name()),sorts);
        action_label_it=(action_label_map.insert(
                    std::pair< std::pair< std::string, sort_expression_list >,std::string>
                      ( p,may_action_label))).first;
        a=push_front(a,action_label(may_action_label,sorts));
        protect_against_garbage_collect.push_back(sorts);
      }
 
      action_label may_action_label(action_label_it->second,sorts); 
      resulting_actions=push_front(resulting_actions,action(may_action_label,args));
    }
    new_actions=reverse(resulting_actions);
  }

  summand result = summand(get_nonreal_variables(s.summation_variables()), 
                           r(condition), s.is_delta(), new_actions, nextstate);

  // gsDebugMsg("Generated summand %P\n", (ATermAppl)result);

  return result;
}

/// \brief Compute process initialisation given a variable context and a process
///        initialisation containing real values.
/// \param initialization A process initialisation containing real values
/// \param context A variable context
/// \param r A rewriter
/// \ret A process initialisation in which all assignments to real variables
///      have been replaced with an initialization for each variable in context.
data_assignment_list determine_process_initialization(
                          const data_assignment_list& initialization, 
                          context_type& context, 
                          rewriter& r)
{
  data_assignment_list init = get_nonreal_assignments(initialization);
  data_assignment_list real_assignments = get_real_assignments(initialization);
  atermpp::map<data_expression, data_expression> replacements;
  for(data_assignment_list::const_iterator i = real_assignments.begin(); i != real_assignments.end(); ++i)
  {
    replacements[i->lhs()] = i->rhs();
  }

  for(context_type::const_iterator i = context.begin(); i != context.end(); ++i)
  {
    data_expression left = realelm_data_expression_map_replace(i->get_lowerbound(), replacements);
    data_expression right = realelm_data_expression_map_replace(i->get_upperbound(), replacements);
    data_assignment assignment;
    if(r(less(left, right)) == true_())
    {
      assignment = data_assignment(i->get_variable(), smaller());
    }
    else if(r(equal_to(left, right)) == true_())
    {
      assignment = data_assignment(i->get_variable(), equal());
    }
    else
    {
      assert(r(greater(left, right)) == true_());
      assignment = data_assignment(i->get_variable(), larger());
    }
    init = push_front(init, assignment);
  }
  return reverse(init);
}


static void generate_zeta_combinations_rec(
              const context_type::const_reverse_iterator context_begin,
              const context_type::const_reverse_iterator context_end,
              const data_expression_list condition_list,
              const data_expression_list comp_value_list,
              atermpp::vector < data_expression_list > &resulting_conditions,
              atermpp::vector < data_expression_list > &resulting_comp_values,
              const atermpp::map<mcrl2::data::data_expression, mcrl2::data::data_expression>  
                    &summand_real_nextstate_map,
              const rewriter &r)
{ if (context_begin==context_end)
  { resulting_conditions.push_back(condition_list);
    resulting_comp_values.push_back(comp_value_list);
    return;
  }
  data_expression t=realelm_data_expression_map_replace(
                           context_begin->get_lowerbound(),
                           summand_real_nextstate_map);
  data_expression u=realelm_data_expression_map_replace(
                           context_begin->get_upperbound(),
                           summand_real_nextstate_map);

  data_expression_list new_condition_list=
                         push_front(condition_list,r(equal_to(t,u)));
  data_expression_list new_comp_value_list=
                         push_front(comp_value_list, data_expression(equal()));
  if (!is_inconsistent(new_condition_list,r))
  { generate_zeta_combinations_rec(context_begin+1,
                                      context_end,
                                      new_condition_list,
                                      new_comp_value_list,
                                      resulting_conditions,
                                      resulting_comp_values,
                                      summand_real_nextstate_map,
                                      r);
  }
  new_condition_list=push_front(condition_list,r(less(t,u)));
  new_comp_value_list=push_front(comp_value_list, data_expression(smaller()));
  if (!is_inconsistent(new_condition_list,r))
  { generate_zeta_combinations_rec(context_begin+1,
                                      context_end,
                                      new_condition_list,
                                      new_comp_value_list,
                                      resulting_conditions,
                                      resulting_comp_values,
                                      summand_real_nextstate_map,
                                      r);
  }
  new_condition_list=push_front(condition_list,r(less(u,t)));
  new_comp_value_list=push_front(comp_value_list,data_expression(larger()));
  if (!is_inconsistent(new_condition_list,r))
  { generate_zeta_combinations_rec(context_begin+1,
                                      context_end,
                                      new_condition_list,
                                      new_comp_value_list,
                                      resulting_conditions,
                                      resulting_comp_values,
                                      summand_real_nextstate_map,
                                      r);
  }
}

/// \brief generate all combinations of contexts that are not inconsistent with each other
///
/// \param context The context variables from which the combinations must be generated.
/// \param resulting_comp_values A vector with lists of values for the zeta/xi variables.
///                              Depending on the parameter value_no_condition each list
///                              contains values smaller, equal or greater corresponding to
///                              the corresponding list in the result, or it contains
///                              conditions is_smaller(xi), is_equal(xi) and is_greater(xi).
/// \ret A vector containing consistent lists of conditions corresponding to context.
static atermpp::vector < data_expression_list > 
       generate_zeta_combinations(
                   const context_type &context,
                   atermpp::vector < data_expression_list > &resulting_comp_values,
                   const atermpp::map<mcrl2::data::data_expression, mcrl2::data::data_expression>  
                         &summand_real_nextstate_map,
                   const rewriter &r)
{ atermpp::vector < data_expression_list > resulting_conditions;
  generate_zeta_combinations_rec(
                   context.rbegin(),
                   context.rend(),
                   data_expression_list(),
                   data_expression_list(),
                   resulting_conditions,
                   resulting_comp_values,
                   summand_real_nextstate_map,
                   r);
  return resulting_conditions;
}

static void generate_xi_combinations_rec(
              const context_type::const_reverse_iterator context_begin,
              const context_type::const_reverse_iterator context_end,
              const data_expression_list condition_list,
              const data_expression_list comp_value_list,
              atermpp::vector < data_expression_list > &resulting_conditions,
              atermpp::vector < data_expression_list > &resulting_comp_values,
              const rewriter &r)
{ if (context_begin==context_end)
  { resulting_conditions.push_back(condition_list);
    resulting_comp_values.push_back(comp_value_list);
    return;
  }
  data_expression_list new_condition_list=
                         push_front(condition_list,
                             equal_to(
                                    context_begin->get_lowerbound(),
                                    context_begin->get_upperbound()));
  data_expression_list new_comp_value_list=
                         push_front(comp_value_list,
                                    data_expression(is_equal(context_begin->get_variable())));
  if (!is_inconsistent(new_condition_list,r))
  { generate_xi_combinations_rec(context_begin+1,
                                      context_end,
                                      new_condition_list,
                                      new_comp_value_list,
                                      resulting_conditions,
                                      resulting_comp_values,
                                      r);
  }
  new_condition_list=push_front(condition_list,
                               less(context_begin->get_lowerbound(),
                                    context_begin->get_upperbound()));
  new_comp_value_list=push_front(comp_value_list,
                                 data_expression(is_smaller(context_begin->get_variable())));
  if (!is_inconsistent(new_condition_list,r))
  { generate_xi_combinations_rec(context_begin+1,
                                      context_end,
                                      new_condition_list,
                                      new_comp_value_list,
                                      resulting_conditions,
                                      resulting_comp_values,
                                      r);
  }
  new_condition_list=push_front(condition_list,
                             less(context_begin->get_upperbound(),
                                  context_begin->get_lowerbound()));
  new_comp_value_list=push_front(comp_value_list,
                                 data_expression(is_larger(context_begin->get_variable())));
  if (!is_inconsistent(new_condition_list,r))
  { generate_xi_combinations_rec(context_begin+1,
                                      context_end,
                                      new_condition_list,
                                      new_comp_value_list,
                                      resulting_conditions,
                                      resulting_comp_values,
                                      r);
  }
}

/// \brief generate all combinations of contexts that are not inconsistent with each other
///
/// \param context The context variables from which the combinations must be generated.
/// \param resulting_comp_values A vector with lists of values for the zeta/xi variables.
///                              Depending on the parameter value_no_condition each list
///                              contains values smaller, equal or greater corresponding to
///                              the corresponding list in the result, or it contains
///                              conditions is_smaller(xi), is_equal(xi) and is_greater(xi).
/// \ret A vector containing consistent lists of conditions corresponding to context.
static atermpp::vector < data_expression_list > 
       generate_xi_combinations(
                   const context_type &context,
                   atermpp::vector < data_expression_list > &resulting_comp_values,
                   const data_expression_list context_conditions,
                   const rewriter &r)
{ atermpp::vector < data_expression_list > resulting_conditions;
  generate_xi_combinations_rec(
                   context.rbegin(),
                   context.rend(),
                   context_conditions,
                   data_expression_list(),
                   resulting_conditions,
                   resulting_comp_values,
                   r);
  return resulting_conditions;
}

// Check whether variables in the first two arguments coincide with those in the last two
static bool are_data_variables_shared(
                 const data_expression d1,
                 const data_expression d2,
                 const data_expression_list l)
{ 
  std::set < data_variable> s1=find_all_data_variables(d1);
  std::set < data_variable> s2=find_all_data_variables(d2);

  // Check whether the variables in d1 and d2 occur in e.
  std::set < data_variable> s3=find_all_data_variables(l);
  for(std::set < data_variable>::iterator i=s3.begin();
              i!=s3.end(); i++)
  { if ((s1.count(*i)>0) || (s2.count(*i)>0))
    { // found
      return true;
    }
  }

  // So, the variables in d1 and d2 do not occur in e and l.
  return false;
}

// Determine whether the pair t,u belonging to the variable xi
// already occurs in the list of conditions in the form t<u, t==u
// or t>u. If so, return in new_condition_for_xi smaller(xi),
// equal(xi) or larger(xi), respectively.
static bool check_whether_condition_exist(
                 const real_representing_variable &xi_variable,
                 const data_expression_list conditions,
                 data_expression &new_condition_for_xi,
                 data_expression &original_xi_condition)
{ const data_expression t(xi_variable.get_lowerbound());
  const data_expression u(xi_variable.get_upperbound());
  original_xi_condition=gsMakeDataExprLT(t,u);
  if (std::find(conditions.begin(),conditions.end(),original_xi_condition)!=conditions.end())
  { new_condition_for_xi=is_smaller(xi_variable.get_variable());
    return true;
  }

  original_xi_condition=gsMakeDataExprEq(t,u);
  if (std::find(conditions.begin(),conditions.end(),original_xi_condition)!=conditions.end())
  { new_condition_for_xi=is_equal(xi_variable.get_variable());
    return true;
  }

  original_xi_condition=gsMakeDataExprGT(t,u);
  if (std::find(conditions.begin(),conditions.end(),original_xi_condition)!=conditions.end())
  { new_condition_for_xi=is_larger(xi_variable.get_variable());
    return true;
  }

  return false;
}

/// \brief Perform elimination of real variables on a specification in a maximum
///        number of iterations.
/// \param s A specification
/// \param max_iterations The maximal number of iterations the algorithm should
///        perform
/// \param strategy The rewrite strategy that should be used.
specification realelm(specification s, int max_iterations, RewriteStrategy strategy)
{
  // First prepare the rewriter and normalize the specification.
  s = set_data_specification(s, add_comp_sort(s.data()));
  rewriter r = rewriter(add_ad_hoc_real_equations(s.data()), (rewriter::strategy)strategy);
  postfix_identifier_generator variable_generator("");
  variable_generator.add_to_context(s);
  s = normalize_specification(s, r, variable_generator);

  // Create for each summand a vector with meta information about this summand, including
  // the summand itself.
  linear_process lps = s.process();
  std::vector < summand_information > summand_info;

  for(summand_list::const_iterator i = lps.summands().begin(); 
                      i != lps.summands().end(); 
                      ++i)
  { data_expression_list inequalities;
    determine_real_inequalities(i->condition(), inequalities);
    // Replacements is the nextstate vector in a map
    atermpp::map<data_expression, data_expression> replacements;
    for(data_assignment_list::const_iterator j = i->assignments().begin(); j != i->assignments().end(); ++j)
    {
      if(j->lhs().sort() == sort_expr::real())
      {
        replacements[j->lhs()] = j->rhs();
      }
    }
    const summand_information s(*i,
                                atermpp::vector < data_expression >(),
                                inequalities,
                                replacements);
    summand_info.push_back(s);
  }

  context_type context; // Contains introduced variables
  data_variable_list real_parameters = get_real_variables(lps.process_parameters());
  data_variable_list nonreal_parameters = get_nonreal_variables(lps.process_parameters());


  // Compute some context information for each summand.

  atermpp::vector < data_expression > new_inequalities; // New inequalities are stored in two consecutive positions;
                                                        // I.e., for t<u, t is at position i, and u at position i+1.
  int iteration = 0;
  do
  {
    new_inequalities.clear();
    iteration++;
    gsVerboseMsg("Iteration %d, starting with %d context variables\n", iteration, context.size());

    for(std::vector < summand_information >::const_iterator i = summand_info.begin(); 
                       i != summand_info.end(); ++i)
    {
      // First calculate the newly introduced variables xi for which the next_state value is not yet known.
      // get , by only looking at variables that
      // occur in the condition or in the effect.
      context_type nextstate_context_for_this_summand;
      atermpp::vector < data_expression > ::const_iterator 
                                value_for_xi=i->get_new_values_for_xi_variables_begin();
      for(context_type::const_iterator c = context.begin() ; 
                                       c!=context.end(); ++c, ++value_for_xi)
      { 
        if ((*value_for_xi)==data_expression())
        { nextstate_context_for_this_summand.push_back(*c);
        }
      }

      //Combinations to be considered
      atermpp::vector < data_expression_list > nextstate_values;
      atermpp::vector < data_expression_list > nextstate_context_combinations =
                    generate_zeta_combinations(
                                    nextstate_context_for_this_summand,
                                    nextstate_values,
                                    i->get_summand_real_nextstate_map(),
                                    r);
      assert(nextstate_values.size()==nextstate_context_combinations.size());
      // std::cerr << "#zeta combinations: " << nextstate_context_combinations.size() << "\n";
      atermpp::vector < data_expression_list >::iterator nextstate_value=nextstate_values.begin();
      
      for(atermpp::vector < data_expression_list >::iterator 
                nextstate_combination = nextstate_context_combinations.begin(); 
                nextstate_combination != nextstate_context_combinations.end(); 
                        ++ nextstate_combination,++ nextstate_value)
      { 
        // zeta[x := g(x)]
        data_expression_list zeta_condition=*nextstate_combination;

        //zeta_condition = data_expression_map_replace_list(zeta_condition, i->get_summand_real_nextstate_map());

        // original condition of the summand && zeta[x := g(x)]
        data_expression_list condition = zeta_condition + i->get_summand_real_conditions();
        // condition = normalize_inequalities(condition, r);

        // Eliminate sum bound variables, resulting in inequalities over
        // process parameters of sort Real. 

        if (core::gsDebug)
        { std::cerr << "Inequalities before Fourier-Motzkin: " << pp(condition) << "\n";
        }
        fourier_motzkin(condition, i->get_summand().summation_variables(), r);
        if (core::gsDebug)
        { std::cerr << "Inequalities after Fourier-Motzkin: " << pp(condition) << "\n";
        }

        // First check which of these inequalities are equivalent to concrete values of xi variables.
        // Add these values for xi variables as a new condition. Remove these variables from the
        // context combinations to be considered for the xi variables.
        
        if(!is_inconsistent(condition, r))
        {
          // condition contains the inequalities over the process parameters
          add_inequalities_to_context_postponed(new_inequalities,condition, context, r);
        }

      }
    }
    add_postponed_inequalities_to_context(
                new_inequalities,
                summand_info,
                context,
                r,
                variable_generator);

  } while ((iteration < max_iterations) && !new_inequalities.empty());

  gsVerboseMsg("generated the following variables in %d iterations:\n", iteration);
  for(context_type::iterator i = context.begin(); i != context.end(); ++i)
  {
    gsVerboseMsg("< %P, %P > %P\n", (ATermAppl)i->get_lowerbound(), 
                   (ATermAppl)i->get_upperbound(), (ATermAppl)i->get_variable());
  }

  if (!new_inequalities.empty())
  {
    gsVerboseMsg("The generated lps is may bisimilar, but most likely not strongly bisimilar.\n");
  }
  else
  {
    gsVerboseMsg("A strongly bisimilar lps is generated.\n");
  }


  /* Generate the new summand list */
  summand_list summands;
  action_label_list new_act_declarations;
  for(std::vector < summand_information >::const_iterator i = summand_info.begin(); 
                       i != summand_info.end(); ++i)
  {
    // First calculate the newly introduced variables xi for which the next_state value is not yet known.
    // get , by only looking at variables that
    // occur in the condition or in the effect.
    context_type nextstate_context_for_this_summand;
    atermpp::vector < data_expression > ::const_iterator 
                              value_for_xi=i->get_new_values_for_xi_variables_begin();
    // context_type::const_iterator c = context.begin();
    // print_debug(summand_info);
    for(context_type::const_iterator c = context.begin() ; 
                                     c!=context.end(); ++c, ++value_for_xi)
    { 
      if ((*value_for_xi)==data_expression())
      { nextstate_context_for_this_summand.push_back(*c);
      }
    }

    //Combinations to be considered
    //const unsigned long nextstate_context_combinations = 
    atermpp::vector < data_expression_list > nextstate_values;
    atermpp::vector < data_expression_list > nextstate_context_combinations =
                  generate_zeta_combinations(
                                  nextstate_context_for_this_summand,
                                  nextstate_values,
                                  i->get_summand_real_nextstate_map(),
                                  r);
    assert(nextstate_values.size()==nextstate_context_combinations.size());
    atermpp::vector < data_expression_list >::iterator nextstate_value=nextstate_values.begin();
    
    for(atermpp::vector < data_expression_list >::iterator 
              nextstate_combination = nextstate_context_combinations.begin(); 
              nextstate_combination != nextstate_context_combinations.end(); 
                      ++ nextstate_combination,++ nextstate_value)
    { 
      // zeta[x := g(x)]
      data_expression_list zeta_condition=*nextstate_combination;

      // original condition of the summand && zeta[x := g(x)]
      data_expression_list condition = zeta_condition + i->get_summand_real_conditions();

      // Eliminate sum bound variables, resulting in inequalities over
      // process parameters of sort Real. 

      fourier_motzkin(condition, i->get_summand().summation_variables(), r);

      // First check which of these inequalities are equivalent to concrete values of xi variables.
      // Add these values for xi variables as a new condition. Remove these variables from the
      // context combinations to be considered for the xi variables.
      
      if(!is_inconsistent(condition, r))
      {
        context_type xi_context_for_this_summand;
        data_expression_list xi_condition;
        // data_expression_list original_xi_conditions;
        value_for_xi=i->get_new_values_for_xi_variables_begin();
        for(context_type::iterator c=context.begin();
                                     c!=context.end(); ++c, ++value_for_xi)
        { data_expression new_condition_for_xi;
          data_expression original_xi_condition;
          if (check_whether_condition_exist(*c,condition,new_condition_for_xi,original_xi_condition))
          { // A value for xi is known. So, we might want to set it.
            xi_condition=push_front(xi_condition,new_condition_for_xi);
            // original_xi_conditions=push_front(original_xi_conditions,original_xi_condition);
            // std::cerr << "new xi condition " << pp(new_condition_for_xi) << "  " << pp(original_xi_condition) << "\n";
          }
          else
          { xi_context_for_this_summand.push_back(*c);
          }
        }

        // Filter the xi_context_for_this_summand by removing variables for which lower and upperbound
        // do not share variables with the expressions in condition.
        context_type filtered_xi_context_for_this_summand;
        for(context_type::iterator c=xi_context_for_this_summand.begin();
                                   c!=xi_context_for_this_summand.end(); ++c)
        { if (are_data_variables_shared(c->get_lowerbound(),c->get_upperbound(),condition))
          { filtered_xi_context_for_this_summand.push_back(*c);
          }
        }

        atermpp::vector < data_expression_list > xi_context_conditions;
        atermpp::vector < data_expression_list > xi_context_combinations =
                    generate_xi_combinations(filtered_xi_context_for_this_summand,
                    xi_context_conditions,
                    condition, //original_xi_conditions,
                    r);

        // std::cerr << "Xi combinations: " << xi_context_combinations.size() << "\n";
        atermpp::vector < data_expression_list >::const_iterator 
                      xi_context_condition=xi_context_conditions.begin();
        for(atermpp::vector < data_expression_list >::iterator 
                  xi_context_combination = xi_context_combinations.begin(); 
                  xi_context_combination != xi_context_combinations.end(); 
                  ++xi_context_combination, ++xi_context_condition)
        { 
          atermpp::vector < data_expression > new_inequalities;
          add_inequalities_to_context_postponed(new_inequalities,condition, context, r);
          if (!new_inequalities.empty())
          { // add a may transition. 
            summand s = generate_summand(*i,
                                         and_(join_and(xi_condition.begin(), xi_condition.end()),
                                              join_and(xi_context_condition->begin(), 
                                                     xi_context_condition->end())), 
                                         *nextstate_value, 
                                         // condition, 
                                         nextstate_context_for_this_summand,
                                         context,
                                         r,
                                         new_act_declarations,
                                         variable_generator,
                                         true);
            // std::cerr << "MAY SUMMAND_OUT: " << pp(s) << "\n";
            summands = push_front(summands, s);
          }
          else
          { // add a must transition.
            summand s = generate_summand(*i,
                                      and_(join_and(xi_condition.begin(), xi_condition.end()),
                                            join_and(xi_context_condition->begin(), 
                                                     xi_context_condition->end())), 
                                       *nextstate_value, 
                                       // condition, 
                                       nextstate_context_for_this_summand,
                                       context,
                                       r,
                                       new_act_declarations,
                                       variable_generator,
                                       false);
            summands = push_front(summands, s);
          }
        }
      }
    }
  }
  summands = reverse(summands);

  // Process parameters
  data_variable_list process_parameters = reverse(nonreal_parameters);
  for(context_type::const_iterator i = context.begin(); i != context.end(); ++i)
  {
    process_parameters = push_front(process_parameters, i->get_variable());
  }
  process_parameters = reverse(process_parameters);

  // New lps
  lps = linear_process(lps.free_variables(), process_parameters, summands);
  // s = set_lps(s, lps);

  // New process initializer
  data_assignment_list initialization(determine_process_initialization(s.initial_process().assignments(), context, r));
  process_initializer init(s.initial_process().free_variables(), initialization);
  // s = set_initial_process(s, init);

  return specification(s.data(),
                       s.action_labels()+new_act_declarations,
                       lps,
                       init);

}

