// Author(s): Jeroen Keiren
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file sumelm.h
/// \brief Provides an implemenation of the sum elimination lemma,
///        as well as the removal of unused summation variables.
///        The sum elimination lemma is the following:
///          sum d:D . d == e -> X(d) = X(e).
///        Removal of unused summation variables is according to the
///        following lemma:
///          d not in x implies sum d:D . x = x

#ifndef MCRL2_LPS_SUMELM_H
#define MCRL2_LPS_SUMELM_H

#include "mcrl2/data/replace.h"
#include "mcrl2/data/join.h"
#include "mcrl2/lps/replace.h"
#include "mcrl2/lps/detail/lps_algorithm.h"

namespace mcrl2
{
namespace lps
{

/// \brief Class implementing the sum elimination lemma.
class sumelm_algorithm: public lps::detail::lps_algorithm
{
  protected:
    /// Stores the number of summation variables that has been removed.
    size_t m_removed;

    /// Adds replacement lhs := rhs to the specified map of replacements.
    /// All replacements that have lhs as a right hand side will be changed to
    /// have rhs as a right hand side.
    void sumelm_add_replacement(std::map<data::variable, data::data_expression>& replacements,
                                const data::variable& lhs,
                                const data::data_expression& rhs)
    {
      using namespace mcrl2::data;
      // First apply already present substitutions to rhs
      data_expression new_rhs = data::replace_free_variables(rhs, data::make_map_substitution(replacements));
      for (std::map<variable, data_expression>::iterator i = replacements.begin(); i != replacements.end(); ++i)
      {
        i->second = data::replace_free_variables(i->second, assignment(lhs, new_rhs));
      }
      replacements[lhs] = new_rhs;
    }

    /// Returns true if x is a summand variable of summand s.
    bool is_summand_variable(const summand_base& s, const data::data_expression& x)
    {
      return data::is_variable(x) && data::search_variable(s.summation_variables(), x);
    }

    template <typename T>
    void swap(T& x, T& y)
    {
      T temp(x);
      x = y;
      y = temp;
    }

    data::data_expression compute_substitutions(const summand_base& s,
      std::map<data::variable, data::data_expression>& substitutions)
    {
      using namespace data;

      atermpp::set<data_expression> conjuncts = data::split_and(s.condition());
      atermpp::set<data_expression> new_conjuncts;

      for(atermpp::set<data_expression>::const_iterator i = conjuncts.begin(); i != conjuncts.end(); ++i)
      {
        bool replacement_added = false;
        if (is_equal_to_application(*i))
        {
          data_expression left = application(*i).left();
          data_expression right = application(*i).right();

          if(!is_summand_variable(s, left) && is_summand_variable(s,right))
          {
            swap(left, right);
          }

          // Expression x == e; we only add a substitution if x is a summation variable, and x does not occur in e;
          // We evaluate the following three cases:
          // 1. there is no substitution assinging to x yet -> add x := e
          // 2. there is a substitution x := d, and e is a summation variable,
          //     for which there is no substitution yet -> add e := x;
          // 3. there is a substitution x := d, and d is a summation variable,
          //    for which there is no substitution yet -> add d := e, and x := e
          if (is_summand_variable(s, left) && !search_data_expression(right, left))
          {
            // Check if we already have a substition with left as left hand side
            if (substitutions.find(left) == substitutions.end())
            {
              sumelm_add_replacement(substitutions, left, right);
              replacement_added = true;
            }
            else if (is_summand_variable(s, right) && substitutions.find(right) == substitutions.end())
            {
              sumelm_add_replacement(substitutions, right, substitutions[left]);
              replacement_added = true;
            }
            else if (is_summand_variable(s, substitutions[left]) && substitutions.find(substitutions[left]) != substitutions.end())
            {
              sumelm_add_replacement(substitutions, substitutions[left], right);
              sumelm_add_replacement(substitutions, left, right);
              replacement_added = true;
            }
          }
        }

        if(!replacement_added)
        {
          new_conjuncts.insert(*i);
        }
      }

      return data::join_and(new_conjuncts.begin(), new_conjuncts.end());
    }

  public:
    /// \brief Constructor.
    /// \param spec The specification to which sum elimination should be
    ///             applied.
    /// \param verbose Control whether verbose output should be given.
    sumelm_algorithm(specification& spec, bool verbose = false)
      : lps::detail::lps_algorithm(spec, verbose),
        m_removed(0)
    {}

    /// \brief Apply the sum elimination lemma to all summands in the
    ///        specification.
    void run()
    {
      m_removed = 0; // Re-initialise number of removed variables for a fresh run.

      for (action_summand_vector::iterator i = m_spec.process().action_summands().begin();
           i != m_spec.process().action_summands().end(); ++i)
      {
        (*this)(*i);
      }

      for (deadlock_summand_vector::iterator i = m_spec.process().deadlock_summands().begin();
           i != m_spec.process().deadlock_summands().end(); ++i)
      {
        (*this)(*i);
      }

      if (verbose())
      {
        std::cerr << "Removed " << m_removed << " summation variables" << std::endl;
      }
    }

    /// \brief Apply the sum elimination lemma to summand s.
    /// \param s an action_summand.
    void operator()(action_summand& s)
    {
      using namespace data;

      atermpp::map<variable, data_expression> substitutions;
      data::data_expression new_condition = compute_substitutions(s, substitutions);

      s.condition() = data::replace_free_variables(new_condition, data::make_map_substitution(substitutions));
      lps::replace_free_variables(s.multi_action(), data::make_map_substitution(substitutions));
      s.assignments() = data::replace_free_variables(s.assignments(), data::make_map_substitution(substitutions));

      const size_t var_count = s.summation_variables().size();
      remove_unused_summand_variables(s);
      m_removed += var_count - s.summation_variables().size();
    }

    /// \brief Apply the sum elimination lemma to summand s.
    /// \param s a deadlock_summand.
    void operator()(deadlock_summand& s)
    {
      using namespace data;

      std::map<variable, data_expression> substitutions;
      data::data_expression new_condition = compute_substitutions(s, substitutions);

      s.condition() = data::replace_free_variables(new_condition, data::make_map_substitution(substitutions));
      if (s.deadlock().has_time())
      {
        s.deadlock().time() = data::replace_free_variables(s.deadlock().time(), data::make_map_substitution(substitutions));
      }

      const size_t var_count = s.summation_variables().size();
      remove_unused_summand_variables(s);
      m_removed += var_count - s.summation_variables().size();
    }
};

/// \brief Apply the sum elimination lemma to summand s.
/// \param s an action summand
/// \return s to which the sum elimination lemma has been applied.
inline
void sumelm(action_summand& s)
{
  specification spec;
  sumelm_algorithm algorithm(spec);
  algorithm(s);
}

/// \brief Apply the sum elimination lemma to summand s.
/// \param s a deadlock summand
/// \return s to which the sum elimination lemma has been applied.
inline
void sumelm(deadlock_summand& s)
{
  specification spec;
  sumelm_algorithm algorithm(spec);
  algorithm(s);
}

} // namespace lps
} // namespace mcrl2

#endif // MCRL2_LPS_SUMELM_H

