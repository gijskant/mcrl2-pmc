// Author(s): Wieger Wesselink
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/lps/linear_process.h
/// \brief Add your file description here.

#ifndef MCRL2_LPS_LINEAR_PROCESS_H
#define MCRL2_LPS_LINEAR_PROCESS_H

#include <string>
#include <cassert>
#include <algorithm>
#include <functional>
#include "atermpp/aterm.h"
#include "atermpp/aterm_list.h"
#include "atermpp/filtered_list.h"
#include "atermpp/algorithm.h"
#include "atermpp/utility.h"
#include "mcrl2/data/utility.h"        // find_variables
#include "mcrl2/lps/summand.h"
#include "mcrl2/lps/process_initializer.h"
#include "mcrl2/lps/detail/utility.h"
#include "mcrl2/lps/detail/free_variables.h"

namespace lps {

using namespace std::rel_ops; // for definition of operator!= in terms of operator==
using atermpp::aterm_appl;
using atermpp::read_from_named_file;

struct is_non_delta_summand
{
  bool operator()(const summand& s) const
  {
    return !s.is_delta();
  }
};

typedef atermpp::filtered_list<summand_list, is_non_delta_summand> non_delta_summand_list;

class linear_process; // prototype declaration

inline
std::set<data_variable> compute_free_variables(const linear_process& process); // prototype declaration

///////////////////////////////////////////////////////////////////////////////
// linear_process
/// \brief linear process.
///
class linear_process: public aterm_appl
{
  protected:
    data_variable_list m_free_variables;
    data_variable_list m_process_parameters;
    summand_list       m_summands;

  public:
    linear_process()
      : aterm_appl(detail::constructLinearProcess())
    {}

    linear_process(data_variable_list free_variables,
        data_variable_list process_parameters,
        summand_list       summands
       )
     : aterm_appl(gsMakeLinearProcess(free_variables, process_parameters, summands)),
       m_free_variables    (free_variables    ),
       m_process_parameters(process_parameters),
       m_summands          (summands          )
    { }

    linear_process(aterm_appl lps)
      : aterm_appl(lps)
    {
      assert(detail::check_term_LinearProcess(m_term));

      // unpack LPS(.,.,.) term
      aterm_appl::iterator i = lps.begin();
      m_free_variables     = data_variable_list(*i++);
      m_process_parameters = data_variable_list(*i++);
      m_summands           = summand_list(*i);
    }

    /// Returns the sequence of LPS summands.
    ///
    summand_list summands() const
    {
      return m_summands;
    }

    /// Returns the sequence of non delta summands.
    ///
    non_delta_summand_list non_delta_summands() const
    {
      return non_delta_summand_list(m_summands, is_non_delta_summand());
    }

    /// Returns the sequence of free variables.
    ///
    data_variable_list free_variables() const
    {
      return m_free_variables;
    }

    /// Returns the sequence of process parameters.
    ///
    data_variable_list process_parameters() const
    {
      return m_process_parameters;
    }

    /// Returns true if time is available in at least one of the summands.
    ///
    bool has_time() const
    {
      for (summand_list::iterator i = summands().begin(); i != summands().end(); ++i)
      {
        if(i->has_time()) return true;
      }
      return false;
    }

    /// Applies a substitution to this LPS and returns the result.
    /// The Substitution object must supply the method aterm operator()(aterm).
    ///
    template <typename Substitution>
    linear_process substitute(Substitution f)
    {
      data_variable_list d = m_free_variables    .substitute(f);
      data_variable_list p = m_process_parameters.substitute(f);
      summand_list       s = m_summands          .substitute(f);
      return linear_process(d, p, s);
    }

    /// Returns the set of free variables that appear in the process.
    /// This set is a subset of <tt>free_variables()</tt>.
    ///
    std::set<data_variable> find_free_variables()
    {
      // TODO: the efficiency of this implementation is not optimal
      std::set<data_variable> result;
      std::set<data_variable> parameters = detail::make_set(process_parameters());
      for (summand_list::iterator i = m_summands.begin(); i != m_summands.end(); ++i)
      {
        std::set<data_variable> summation_variables = detail::make_set(i->summation_variables());
        std::set<data_variable> used_variables = find_variables(make_list(i->condition(), i->actions(), i->time(), i->assignments()));
        std::set<data_variable> bound_variables = detail::set_union(parameters, summation_variables);
        std::set<data_variable> free_variables = detail::set_difference(used_variables, bound_variables);
        result.insert(free_variables.begin(), free_variables.end());
      }
      return result;
    }

    /// Returns true if
    /// <ul>
    /// <li>the free variables occurring in the process are contained in free_variables()        </li>
    /// <li>the process parameters have unique names</li>
    /// <li>the free variables have unique names</li>
    /// <li>the names of the process parameters do not appear as the name of a summation variable</li>
    /// <li>the left hand sides of the assignments of summands are contained in the process parameters</li>
    ///
    /// <li>the summands are well typed</li>
    /// </ul>
    ///
    bool is_well_typed() const
    {
      // check 6)
      std::set<data_variable> declared_free_variables  = detail::make_set(free_variables());
      std::set<data_variable> occurring_free_variables = compute_free_variables(*this);
      if (!(std::includes(declared_free_variables.begin(),
                          declared_free_variables.end(),
                          occurring_free_variables.begin(),
                          occurring_free_variables.end()
                         )
          ))
      {
        std::cerr << "linear_process::is_well_typed() failed: some of the free variables were not declared\n";
        std::cerr << "declared free variables: ";
        for (std::set<data_variable>::iterator i = declared_free_variables.begin(); i != declared_free_variables.end(); ++i)
        {
          std::cerr << pp(*i) << " ";
        }
        std::cerr << "occurring free variables: ";
        for (std::set<data_variable>::iterator i = occurring_free_variables.begin(); i != occurring_free_variables.end(); ++i)
        {
          std::cerr << pp(*i) << " ";
        }
        std::cerr << std::endl;
        return false;
      }

      // check 1)
      for (summand_list::iterator i = m_summands.begin(); i != m_summands.end(); ++i)
      {
        if (!i->is_well_typed())
          return false;
      }

      // check 2)
      if (!detail::unique_names(m_process_parameters))
      {
        std::cerr << "linear_process::is_well_typed() failed: process parameters " << pp(m_process_parameters) << " don't have unique names." << std::endl;
        return false;
      }

      // check 3)
      if (!detail::unique_names(m_free_variables))
      {
        std::cerr << "linear_process::is_well_typed() failed: free variables " << pp(m_process_parameters) << " don't have unique names." << std::endl;
        return false;
      }

      // check 4)
      std::set<identifier_string> names;
      for (data_variable_list::iterator i = m_process_parameters.begin(); i != m_process_parameters.end(); ++i)
      {
        names.insert(i->name());
      }
      for (summand_list::iterator i = m_summands.begin(); i != m_summands.end(); ++i)
      {
        if (!detail::check_variable_names(i->summation_variables(), names))
        {
          std::cerr << "linear_process::is_well_typed() failed: some of the names of the summation variables " << pp(i->summation_variables()) << " also appear as process parameters." << std::endl;
          return false;
        }
      }

      // check 5)
      for (summand_list::iterator i = m_summands.begin(); i != m_summands.end(); ++i)
      {
        if (!detail::check_assignment_variables(i->assignments(), m_process_parameters))
        {
          std::cerr << "linear_process::is_well_typed() failed: some left hand sides of the assignments " << pp(i->assignments()) << " do not appear as process parameters." << std::endl;
          return false;
        }
      }

      return true;
    }
  };

/// \brief Computes the free variables that occur in the specification
inline
std::set<data_variable> compute_free_variables(const linear_process& process)
{
  std::set<data_variable> result;
  std::set<data_variable> process_parameters = detail::make_set(process.process_parameters());
  for (summand_list::iterator i = process.summands().begin(); i != process.summands().end(); ++i)
  {
    detail::collect_free_variables(*i, process_parameters, std::inserter(result, result.end()));
  }
  return result;
}

/// \brief Computes the action labels that occur in the process
inline
std::set<action_label> compute_action_labels(const linear_process& process)
{
  std::set<action_label> result;
  atermpp::find_all_if(process, is_action_label, std::inserter(result, result.end()));
  return result;
}

/// \brief Sets the free variables of l and returns the result
inline
linear_process set_free_variables(linear_process l, data_variable_list free_variables)
{
  return linear_process(free_variables,
             l.process_parameters(),
             l.summands          ()
            );
}

/// \brief Sets the process parameters of l and returns the result
inline
linear_process set_process_parameters(linear_process l, data_variable_list process_parameters)
{
  return linear_process(l.free_variables    (),
             process_parameters,
             l.summands          ()
            );
}

/// \brief Sets the summands of l and returns the result
inline
linear_process set_summands(linear_process l, summand_list summands)
{
  return linear_process(l.free_variables    (),
             l.process_parameters(),
             summands
            );
}

} // namespace lps

/// INTERNAL ONLY
namespace atermpp
{
using lps::linear_process;

template<>
struct aterm_traits<linear_process>
{
  typedef ATermAppl aterm_type;
  static void protect(linear_process t)   { t.protect(); }
  static void unprotect(linear_process t) { t.unprotect(); }
  static void mark(linear_process t)      { t.mark(); }
  static ATerm term(linear_process t)     { return t.term(); }
  static ATerm* ptr(linear_process& t)    { return &t.term(); }
};

} // namespace atermpp

#endif // MCRL2_LPS_LINEAR_PROCESS_H
