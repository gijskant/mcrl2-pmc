// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/detail/enum/standard.h

#ifndef _ENUM_STANDARD_H
#define _ENUM_STANDARD_H

#include "mcrl2/atermpp/aterm.h"
#include "mcrl2/atermpp/aterm_int.h"
#include "mcrl2/data/detail/rewrite.h"
#include "mcrl2/data/detail/enumerator_variable_limit.h"


#define MAX_VARS_INIT   1000
#define MAX_VARS_FACTOR 5

/// \cond INTERNAL_DOCS
namespace mcrl2
{
namespace data
{
namespace detail
{

class ss_solution
{
  protected:
    atermpp::term_list< atermpp::aterm_appl > m_solution;  // A list containing the solution of a condition in internal format.
    atermpp::aterm_appl m_evaluated_condition;             // The condition after substituting the solution, in internal format.

  public:

    // Constructor.
    ss_solution(const atermpp::term_list< atermpp::aterm_appl > &solution, const atermpp::aterm_appl evaluated_condition) :
      m_solution(solution),
      m_evaluated_condition(evaluated_condition)
    {} 
   
    atermpp::aterm_appl evaluated_condition() const
    { 
      return m_evaluated_condition;
    }

    atermpp::term_list< atermpp::aterm_appl > solution() const
    {
      return m_solution; 
    }
};

class fs_expr
{
  protected:
    variable_list m_vars;                              // The vars over which enumeration must take place.
    variable_list m_substituted_vars;                  // Variables for which a substitution exist. The substituted
                                                       // values are given in m_vals;
    atermpp::term_list< atermpp::aterm_appl > m_vals;  // Data expressions in internal format that are to be substituted
                                                       // in the variables in m_substituted_vars.
    atermpp::aterm_appl m_expr;                        // data_expression in internal format to which internal variables
                                                       // must adhere.

  public:
    // Default constructor
    fs_expr()
    {}

    // Constructor
    fs_expr(
        const variable_list &vars, 
        const variable_list &substituted_vars, 
        const atermpp::term_list< atermpp::aterm_appl > &vals, 
        const atermpp::aterm_appl &expr):
       m_vars(vars), m_substituted_vars(substituted_vars),m_vals(vals), m_expr(expr)
    {
    }

    const variable_list &vars() const
    {
      return m_vars;
    }

    const variable_list &substituted_vars() const
    {
      return m_substituted_vars;
    }

    const atermpp::term_list< atermpp::aterm_appl > &vals() const
    {
      return m_vals;
    }

    const atermpp::aterm_appl &expr() const
    {
      return m_expr;
    }
};

class EnumeratorSolutionsStandard;


/**
 * \brief Class for finding solutions to boolean expressions.
 * \deprecated
 * Finding concrete solutions can be done by using the 
 * EnumeratorSolutionsStandard class. For each instance of an EnumeratorStandard class
 * at most one active instance of an EnumeratorSolutionsStandard class can be used.
 *
 * Use of these classes is discouraged. Use the classic_enumerator class instead.
 **/

class EnumeratorStandard 
{
  public:
    const mcrl2::data::data_specification &m_data_spec;
    Rewriter* rewr_obj;
    std::set< atermpp::aterm_int > eqs;
  
    EnumeratorStandard(mcrl2::data::data_specification const& data_spec, Rewriter* r); 
    ~EnumeratorStandard();

    Rewriter* getRewriter() const
    {
      return rewr_obj;
    }
};

/**
 * \brief Class for enumerating solutions to boolean expressions.
 **/

class EnumeratorSolutionsStandard 
{
  public:
    typedef Rewriter::substitution_type substitution_type;
    typedef Rewriter::internal_substitution_type internal_substitution_type;

  protected:

    detail::EnumeratorStandard *m_enclosing_enumerator;
/*    atermpp::aterm_appl desired_truth_value;    // We search for solutions for the condition enum_expr that are not
    atermpp::aterm_appl forbidden_truth_value;  // equal to the forbidden truth value, and if the output matches the
                                                // desired truth value, then the variable solution_is_exact is set. */

    variable_list enum_vars;                    // The variables over which a solution is searched.
    atermpp::aterm_appl enum_expr;              // Condition to be satisfied in internal format.
    internal_substitution_type &enum_sigma;

    std::deque < fs_expr> fs_stack;
    std::vector< ss_solution > ss_stack;

    size_t used_vars;
    size_t max_vars;
    size_t m_max_internal_variables;

    internal_substitution_type &default_sigma()
    {
      static internal_substitution_type default_sigma;
      return default_sigma;
    }

  public:

    /// \brief Default constructor
    EnumeratorSolutionsStandard():
       enum_sigma(default_sigma()),
       m_max_internal_variables(0)
    {
    } 

    /// \brief Constructor. Generate solutions for the variables in Vars that satisfy Expr.
    /// If not equal_to_false is set all solutions are generated that make Expr not equal to false.
    /// Otherwise all solutions are generated that make Expr not equal to true. The enumerator
    /// uses internal variables. By setting max_internal_variables to any value larger than
    /// 0 the number of these variables can be bound, guaranteeing termination (provided the
    /// rewriter terminates). If set to 0, there is no bound, but the enumerator will issue 
    /// warnings.
    /// \param[in] vars The list of variables for which solutions are being sought.
    /// \param[in] expr The expression which determine the solutions. This expr is assumed to be
    ///                 in rewrite normal form. 
    /// \param[in] not_equal_to_false Determine whether the condition is supposed to be true, or
    ///                               whether it should become false. If the condition must be
    ///                               true, all solutions that make the condition not equal to
    ///                               false are enumerated. 
    /// \param[in] enclosing_enumerator Pointer to the enclosing enumerator.
    /// \param[in] max_internal_variables Maximal number of internal variables that will be used
    ///                                   when generating solutions. If 0 there is no limit, and
    ///                                   the enumerator may not terminate.
    //  \param[in] expr_is_normal_form A boolean that indicates whether expr is in normal form. If
    //                                 not, the first action of the enumerator is to rewrite it in
    //                                 normal form, but sometimes this is known, and in such 
    //                                 a case rewriting can be avoided.

    EnumeratorSolutionsStandard(
                   const variable_list &vars, 
                   const atermpp::aterm_appl &expr, 
                   internal_substitution_type &sigma,
                   const bool not_equal_to_false, 
                   detail::EnumeratorStandard *enclosing_enumerator,
                   const size_t max_internal_variables=0,
                   const bool expr_is_normal_form=false) :
      m_enclosing_enumerator(enclosing_enumerator),
      enum_vars(vars),
      enum_expr(expr),
      enum_sigma(sigma),
      used_vars(0),
      max_vars(MAX_VARS_INIT),
      m_max_internal_variables(max_internal_variables)
    { 
      reset(not_equal_to_false,expr_is_normal_form);
    }

    /// Standard destructor.
    ~EnumeratorSolutionsStandard()
    {
    }
 
   /**
    * \brief Get next solution as a term_list in internal format if available.
    * \param[out] evaluated_condition This optional parameter is used to return the
    *             condition in which solution is substituted. 
    * \param[out] solution Place to store the solutions.
    *             The aterm_list solution contains solutions for the variables in internal
    *             format in the same order as the variable list Vars.
    * \param[out] solution_possible. This boolean indicates whether it was possible to
    *             generate a solution. If there is a variable of a sort without a constructor
    *             sort, it is not possible to generate solutions. Similarly, it can be
    *             that the maximum number of solutions has been reached. In this case the variable
    *             solution_possible is false, and the function returns false. 
    *             This variable should be true when calling next. If it is initially false, 
    *             or if a variant of next is used
    *             without this parameter, an mcrl2::runtime_error exception is thrown if no solutions exist, or
    *             if the maximum number of internal variables is reached.
    * \param[in]  max_internal_variables The maximum number of variables to be 
    *             used internally when generating solutions. If set to 0 an unbounded number
    *             of variables are used, and warnings are printed to warn for potentially
    *             unbounded loops.
    * \return Whether or not a solution was found and stored in
    *         solution. If false, there are no more solutions to be found. 
    *
    **/

    bool next(atermpp::aterm_appl &evaluated_condition,
              atermpp::term_list<atermpp::aterm_appl> &solution, 
              bool &solution_possible);

  /** \brief Get next solution as a term_list in internal format.
   **/
    bool next(atermpp::term_list<atermpp::aterm_appl> &solution);

  /** \brief Get next solution as a term_list in internal format.
   **/
    bool next(atermpp::aterm_appl &evaluated_condition,
              atermpp::term_list<atermpp::aterm_appl> &solution);

  /** \brief Get next solution as a term_list in internal format.
   **/
    bool next(atermpp::term_list<atermpp::aterm_appl> &solution, 
              bool &solution_possible);


  private:
    void reset(const bool not_equal_to_false, const bool expr_is_normal_form); 

    bool find_equality(const atermpp::aterm_appl &T, 
                            const mcrl2::data::variable_list &vars, 
                            mcrl2::data::variable &v, 
                            atermpp::aterm_appl &e); 

    void EliminateVars(fs_expr &e);

    atermpp::aterm_appl build_solution_single(
                 const atermpp::aterm_appl &t,
                 variable_list substituted_vars,
                 atermpp::term_list < atermpp::aterm_appl> exprs) const;

    atermpp::term_list < atermpp::aterm_appl> build_solution(
                 const variable_list &vars,
                 const variable_list &substituted_vars,
                 const atermpp::term_list < atermpp::aterm_appl> &exprs) const;

    atermpp::term_list < atermpp::aterm_appl> build_solution2(
                 const variable_list &vars,
                 const variable_list &substituted_vars,
                 const atermpp::term_list < atermpp::aterm_appl> &exprs) const;
    atermpp::aterm_appl build_solution_aux(
                 const atermpp::aterm_appl &t,
                 const variable_list &substituted_vars,
                 const atermpp::term_list < atermpp::aterm_appl> &exprs) const;
    atermpp::aterm_appl add_negations(
                 const atermpp::aterm_appl &condition,
                 const atermpp::term_list< atermpp::aterm_appl > &negation_term_list,
                 const bool negated) const;
    void push_on_fs_stack_and_split_or(
                 std::deque < fs_expr> &fs_stack,
                 const variable_list &var_list,
                 const variable_list &substituted_vars,
                 const atermpp::term_list< atermpp::aterm_appl > &substitution_terms,
                 const atermpp::aterm_appl &condition,
                 const atermpp::term_list< atermpp::aterm_appl > &negated_term_list,
                 const bool negated) const;
    void push_on_fs_stack_and_split_or_without_rewriting(
                 std::deque < fs_expr> &fs_stack,
                 const variable_list &var_list,
                 const variable_list &substituted_vars,
                 const atermpp::term_list< atermpp::aterm_appl > &substitution_terms,
                 const atermpp::aterm_appl &condition,
                 const atermpp::term_list< atermpp::aterm_appl > &negated_term_list,
                 const bool negated) const;
    atermpp::term_list< atermpp::aterm_appl > negate(
                 const atermpp::term_list< atermpp::aterm_appl > &l) const;
};
}
}
} // namespace atermpp
/// \endcond

#endif
