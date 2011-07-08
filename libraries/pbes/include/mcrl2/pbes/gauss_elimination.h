// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/pbes/gauss_elimination.h
/// \brief Gauss elimination algorithm for pbes equation systems.

#ifndef MCRL2_PBES_GAUSS_ELIMINATION_H
#define MCRL2_PBES_GAUSS_ELIMINATION_H

#include <sstream>

#include "mcrl2/pbes/replace.h"
#include "mcrl2/pbes/pbes.h"
#include "mcrl2/utilities/algorithm.h"
#include "mcrl2/utilities/logger.h"

namespace mcrl2
{

namespace pbes_system
{

/// \brief Algorithm class for the Gauss elimination algorithm for solving
/// systems of (P)BES equations.

template <typename ExpressionTraits>
class gauss_elimination_algorithm : public utilities::algorithm
{
  public:
    typedef typename ExpressionTraits::expression_type expression_type;
    typedef typename ExpressionTraits::variable_type variable_type;
    typedef typename ExpressionTraits::equation_type equation_type;

  protected:

    std::string print_equation(const equation_type& eq) const
    {
      return ExpressionTraits::print(eq);
    }

    template <typename Iter>
    std::string print_equations(Iter first, Iter last) const
    {
      std::ostringstream out;
      for (Iter i = first; i != last; ++i)
      {
        out << "  " << print_equation(*i) << std::endl;
      }
      return out.str();
    }

    void LOG_EQUATION_VERBOSE(const std::string& msg, const equation_type& eq) const
    {
      mCRL2log(verbose) << msg << print_equation(eq) << "\n";
    }

    /// \brief Prints the sequence of pbes equations [first, last) to clog.
    /// \param first Start of a range of equations
    /// \param last End of a range of equations
    template <typename Iter>
    void LOG_EQUATIONS_DEBUG(const std::string& msg, Iter first, Iter last) const
    {
      mCRL2log(debug) << msg << print_equations(first, last);
    }

    template <typename Iter>
    void LOG_EQUATIONS_DEBUG1(const std::string& msg, Iter first, Iter last) const
    {
      mCRL2log(debug1) << msg << print_equations(first, last);
    }

  public:
    /// \brief Runs the algorithm. Applies Gauss elimination to the sequence of pbes equations [first, last).
    /// \param first Start of a range of pbes equations
    /// \param last End of a range of pbes equations
    /// \param solver An equation solver

    template <typename Iter, typename FixpointEquationSolver>
    void run(Iter first, Iter last, FixpointEquationSolver solve)
    {
      LOG_EQUATIONS_DEBUG("equations before solving\n", first, last);
      if (first == last)
      {
        return;
      }

      Iter i = last;
      while (i != first)
      {
        --i;
        LOG_EQUATION_VERBOSE("solving equation\n  before: ", *i);
        solve(*i);
        LOG_EQUATION_VERBOSE("   after: ", *i);
        for (Iter j = first; j != i; ++j)
        {
          j->formula() = ExpressionTraits::substitute(j->formula(), i->variable(), i->formula());
        }
        LOG_EQUATIONS_DEBUG1("equations after substitution\n", first, last);
      }
      LOG_EQUATIONS_DEBUG("equations after solving\n", first, last);
    }
};

/// \brief Approximation algorithm

template <typename BooleanExpressionTraits, typename Compare>
struct approximate
{
  Compare m_compare;

  approximate(Compare compare)
    : m_compare(compare)
  {
  }

  void operator()(typename BooleanExpressionTraits::equation_type& eq) const
  {
    typedef BooleanExpressionTraits tr;
    typedef typename BooleanExpressionTraits::expression_type expression_type;
    typedef typename BooleanExpressionTraits::variable_type variable_type;
    typedef typename BooleanExpressionTraits::equation_type equation_type;

    const expression_type& phi = eq.formula();
    const variable_type& X = eq.variable();
    expression_type next = eq.symbol() == tr::nu() ? tr::true_() : tr::false_();
    expression_type prev;
    do
    {
      prev = next;
      next = tr::substitute(phi, X, prev);
    }
    while (!m_compare(prev, next));
    eq.formula() = prev;
  }
};

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_GAUSS_ELIMINATION_H
