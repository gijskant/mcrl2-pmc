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

#include "mcrl2/pbes/replace.h"
#include "mcrl2/pbes/pbes.h"

namespace mcrl2 {

namespace pbes_system {

/// \brief Namespace for Gauss elimination
namespace gauss {

  /// \brief Returns true if e.symbol() == nu(), else false.
  /// \param e A pbes equation
  /// \return True if e.symbol() == nu(), else false.
  inline
  pbes_expression sigma(const pbes_equation& e)
  {
    using namespace pbes_expr;
    return e.symbol().is_nu() ? true_() : false_();
  }

  /// \brief Applies the substitution X := phi to the pbes equation eq.
  /// \param eq A pbes equation
  /// \param X A propositional variable
  /// \param phi A pbes expression
  /// \return The substition result
  inline
  pbes_equation substitute(pbes_equation eq, propositional_variable X, pbes_expression phi)
  {
    pbes_expression formula = substitute_propositional_variable(eq.formula(), X, phi);
    return pbes_equation(eq.symbol(), eq.variable(), formula);
  }

  /// \brief Applies the substitution X := phi to the sequence of pbes equations [first, last).
  /// \param first Start of a range of pbes equations
  /// \param last End of a range of pbes equations
  /// \param X A propositional variable
  /// \param phi A pbes expression
  template <typename Iter>
  void substitute(Iter first, Iter last, propositional_variable X, pbes_expression phi)
  {
    for (Iter i = first; i != last; ++i)
    {
      *i = substitute(*i, X, phi);
    }
  }

} // namespace gauss

/// \brief Algorithm class for the Gauss elimination algorithm for solving
/// systems of pbes equations.
template <typename PbesRewriter, typename EquationSolver>
class gauss_elimination_algorithm
{
  protected:
    /// \brief A pbes rewriter
    PbesRewriter& m_rewriter;
    
    /// \brief An equation solver
    EquationSolver& m_equation_solver;

    /// \brief Pretty print an equation without generating a newline after the equal sign
    /// \param eq A pbes equation
    /// \return A pretty printed string
    std::string pp(pbes_equation eq)
    {
      return core::pp(eq.symbol()) + " " + core::pp(eq.variable()) + " = " + core::pp(eq.formula());
    }

    /// \brief Prints the sequence of pbes equations [first, last) to standard out.
    /// \param first Start of a range of pbes equations
    /// \param last End of a range of pbes equations
    template <typename Iter>
    void print(Iter first, Iter last)
    {
      std::cerr << "pbes\n";
      for (Iter i = first; i != last; ++i)
      {
        std::cerr << "  " << pp(*i) << std::endl;
      }
    }

  public:
    
    /// \brief Constructor
    /// \param rewriter A pbes rewriter
    /// \param equation_solver An equation solver   
    gauss_elimination_algorithm(PbesRewriter& rewriter, EquationSolver& equation_solver)
      : m_rewriter(rewriter), m_equation_solver(equation_solver)
    {}

    /// \brief Runs the algorithm. Applies Gauss elimination to the sequence of pbes equations [first, last).
    /// \param first Start of a range of pbes equations
    /// \param last End of a range of pbes equations
    template <typename Iter>
    void run(Iter first, Iter last)
    {
#ifdef MCRL2_GAUSS_ELIMINATION_DEBUG
  print(first, last);
#endif
      if (first == last)
      {
        return;
      }

      Iter i = last;
      while (i != first)
      {
        --i;
#ifdef MCRL2_GAUSS_ELIMINATION_DEBUG
  std::cerr << "solving equation\n";
  std::cerr << "  before: " << pp(*i) << std::endl;
#endif
        *i = m_equation_solver(*i);
#ifdef MCRL2_GAUSS_ELIMINATION_DEBUG
  std::cerr << "   after: " << pp(*i) << std::endl;
#endif
        gauss::substitute(first, i, i->variable(), i->formula());
      }
      *i = m_equation_solver(*i); // TODO: clean the logic of this algorithm up
#ifdef MCRL2_GAUSS_ELIMINATION_DEBUG
  print(first, last);
#endif
    }
};

} // namespace pbes_system

} // namespace mcrl2

#endif // MCRL2_PBES_GAUSS_ELIMINATION_H
