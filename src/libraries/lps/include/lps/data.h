///////////////////////////////////////////////////////////////////////////////
/// \file data.h
/// Contains data data structures for the LPS Library.

#ifndef LPS_DATA_H
#define LPS_DATA_H

#include <iostream> // for debugging

#include <string>
#include <cassert>
#include "atermpp/atermpp.h"
#include "atermpp/algorithm.h"
#include "atermpp/aterm_access.h"
#include "atermpp/utility.h"
#include "lps/identifier_string.h"
#include "lps/data_expression.h"
#include "lps/detail/soundness_checks.h"
#include "libstruct.h"

namespace lps {

using atermpp::aterm_appl;
using atermpp::aterm_list;
using atermpp::term_list;
using atermpp::aterm;
using atermpp::arg1;
using atermpp::arg2;
using atermpp::arg3;

///////////////////////////////////////////////////////////////////////////////
// data_variable
/// \brief data variable.
///
// DataVarId(<String>, <SortExpr>)
class data_variable: public data_expression
{
  public:
    data_variable()
      : data_expression(detail::constructDataVarId())
    {}

    data_variable(aterm_appl t)
     : data_expression(t)
    {
      assert(detail::check_rule_DataVarId(m_term));
    }

    /// Very incomplete implementation for initialization using strings like "d:D".
    data_variable(const std::string& s)
    {
      std::string::size_type idx = s.find(':');
      assert (idx != std::string::npos);
      std::string name = s.substr(0, idx);
      std::string type = s.substr(idx+1);
      m_term = reinterpret_cast<ATerm>(gsMakeDataVarId(gsString2ATermAppl(name.c_str()), lps::sort(type)));
    }

    data_variable(const std::string& name, const lps::sort& s)
     : data_expression(gsMakeDataVarId(gsString2ATermAppl(name.c_str()), s))
    {}

    /// Returns the name of the data_variable.
    ///
    identifier_string name() const
    {
      return arg1(*this);
    }

    /// Returns the sort of the data_variable.
    ///
    lps::sort sort() const
    {
      return gsGetSort(*this);
    }
  };
                                                            
///////////////////////////////////////////////////////////////////////////////
// data_variable_list
/// \brief singly linked list of data variables
///
typedef term_list<data_variable> data_variable_list;

inline
bool is_data_variable(aterm_appl t)
{
  return gsIsDataVarId(t);
}

///////////////////////////////////////////////////////////////////////////////
// data_application
/// \brief data application.
///
// DataAppl(<DataExpr>, <DataExpr>)
class data_application: public data_expression
{
  public:
    data_application()
      : data_expression(detail::constructDataAppl())
    {}

    data_application(aterm_appl t)
     : data_expression(t)
    {
      assert(detail::check_term_DataAppl(m_term));
    }

    data_application(data_expression expr, data_expression arg)
     : data_expression(gsMakeDataAppl(expr, arg))
    {}
  };
                                                            
///////////////////////////////////////////////////////////////////////////////
// data_application_list
/// \brief singly linked list of data applications
///
typedef term_list<data_application> data_application_list;

inline
bool is_data_application(aterm_appl t)
{
  return gsIsDataAppl(t);
}

///////////////////////////////////////////////////////////////////////////////
// data_operation
/// \brief operation on data.
///
class data_operation: public data_expression
{
  public:
    data_operation()
      : data_expression(detail::constructOpId())
    {}

    data_operation(aterm_appl t)
     : data_expression(t)
    {
      assert(detail::check_rule_OpId(m_term));
    }

    data_operation(identifier_string name, lps::sort s)
     : data_expression(gsMakeOpId(name, s))
    {}

    /// Returns the name of the data_operation.
    ///
    identifier_string name() const
    {
      return arg1(*this);
    }

    /// Returns the sort of the data_operation.
    ///
    lps::sort sort() const
    {
      return gsGetSort(*this);
    }
  };
                                                            
///////////////////////////////////////////////////////////////////////////////
// data_operation_list
/// \brief singly linked list of data operations
///
typedef term_list<data_operation> data_operation_list;

inline
bool is_data_operation(aterm_appl t)
{
  return gsIsOpId(t);
}

///////////////////////////////////////////////////////////////////////////////
// data_equation
/// \brief data equation.
///
//<DataEqn>      ::= DataEqn(<DataVarId>*, <DataExprOrNil>,
//                     <DataExpr>, <DataExpr>)
class data_equation: public aterm_appl
{
  protected:
    data_variable_list m_variables;
    data_expression m_condition;
    data_expression m_lhs;
    data_expression m_rhs;

  public:
    typedef data_variable_list::iterator variable_iterator;

    data_equation()
      : aterm_appl(detail::constructDataEqn())
    {}

    data_equation(aterm_appl t)
     : aterm_appl(t)
    {
      assert(detail::check_rule_DataEqn(m_term));
      aterm_appl::iterator i = t.begin();
      m_variables = data_variable_list(*i++);
      m_condition = data_expression(*i++);
      m_lhs       = data_expression(*i++);
      m_rhs       = data_expression(*i);
      assert(m_condition.is_nil() || data_expr::is_bool(m_condition));
    } 

    data_equation(data_variable_list variables,
                  data_expression    condition,
                  data_expression    lhs,
                  data_expression    rhs
                 )
     : aterm_appl(gsMakeDataEqn(variables, condition, lhs, rhs)),
       m_variables(variables),
       m_condition(condition),
       m_lhs(lhs),
       m_rhs(rhs)     
    {
      assert(m_condition.is_nil() || data_expr::is_bool(m_condition));
    }

    /// Returns the sequence of variables.
    ///
    data_variable_list variables() const
    {
      return m_variables;
    }

    /// Returns the condition of the summand (must be of type bool).
    ///
    data_expression condition() const
    {
      return m_condition;
    }

    /// Returns the left hand side of the Assignment.
    ///
    data_expression lhs() const
    {
      return m_lhs;
    }

    /// Returns the right hand side of the Assignment.
    ///
    data_expression rhs() const
    {
      return m_rhs;
    }

    /// Applies a substitution to this data_equation and returns the result.
    /// The Substitution object must supply the method aterm operator()(aterm).
    ///
    template <typename Substitution>
    data_equation substitute(Substitution f) const
    {
      return data_equation(f(aterm(*this)));
    }
    
    /// Returns true if
    /// <ul>
    /// <li>the types of the left and right hand side are equal</li>
    /// </ul>
    bool is_well_typed() const
    {
      // check 1)
      if (m_lhs.sort() != m_rhs.sort())
      {
        std::cerr << "data_equation::is_well_typed() failed: the left and right hand sides " << pp(m_lhs) << " and " << pp(m_rhs) << " have different types." << std::endl;
        return false;
      }
      
      return true;
    }   
};

///////////////////////////////////////////////////////////////////////////////
// data_equation_list
/// \brief singly linked list of data equations
///
typedef term_list<data_equation> data_equation_list;

inline
bool is_data_equation(aterm_appl t)
{
  return gsIsDataEqn(t);
}

///////////////////////////////////////////////////////////////////////////////
// data_assignment
/// \brief data_assignment is an assignment to a data variable.
///
// syntax: data_assignment(data_variable lhs, data_expression rhs)
class data_assignment: public aterm_appl
{
  protected:
    data_variable   m_lhs;         // left hand side of the assignment
    data_expression m_rhs;         // right hand side of the assignment

  public:
    data_assignment()
      : aterm_appl(detail::constructPBExpr())
    {}

    data_assignment(aterm_appl t)
     : aterm_appl(t)
    {
      assert(detail::check_rule_DataVarIdInit(m_term));
      aterm_appl::iterator i = t.begin();
      m_lhs = data_variable(*i++);
      m_rhs = data_expression(*i);
      assert(is_well_typed());
    }

    data_assignment(data_variable lhs, data_expression rhs)
     : 
       aterm_appl(gsMakeDataVarIdInit(lhs, rhs)),
       m_lhs(lhs),
       m_rhs(rhs)
    {
    }

    /// Returns true if the sorts of the left and right hand side are equal.
    bool is_well_typed() const
    {
      bool result = gsGetSort(m_lhs) == gsGetSort(m_rhs);
      if (!result)
      {
        std::cerr << "data_assignment::is_well_typed() failed: the left and right hand sides " << pp(m_lhs) << " and " << pp(m_rhs) << " have different sorts." << std::endl;
        return false;
      }
      return true;
    }

    /// Applies the assignment to t and returns the result.
    ///
    aterm operator()(aterm t) const
    {
      return atermpp::replace(t, aterm(m_lhs), aterm(m_rhs));
    }

    /// Returns the left hand side of the data_assignment.
    ///
    data_variable lhs() const
    {
      return m_lhs;
    }

    /// Returns the right hand side of the data_assignment.
    ///
    data_expression rhs() const
    {
      return m_rhs;
    }
};

///////////////////////////////////////////////////////////////////////////////
// data_assignment_list
/// \brief singly linked list of data assignments
///
typedef term_list<data_assignment> data_assignment_list;

inline
bool is_data_assignment(aterm_appl t)
{
  return gsIsDataVarIdInit(t);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Returns the right hand sides of the assignments.
inline
data_assignment_list make_assignment_list(data_variable_list lhs, data_expression_list rhs)
{
  assert(lhs.size() == rhs.size());
  data_assignment_list result;
  data_variable_list::iterator i = lhs.begin();
  data_expression_list::iterator j = rhs.begin();
  for ( ; i != lhs.end(); ++i, ++j)
  {
    result = push_front(result, data_assignment(*i, *j));
  }
  return atermpp::reverse(result);
}

///////////////////////////////////////////////////////////////////////////////
/// \brief Returns the right hand sides of the assignments.
inline
data_expression_list data_assignment_expressions(data_assignment_list l)
{
  data_expression_list result;
  for (data_assignment_list::iterator i = l.begin(); i != l.end(); ++i)
  {
    result = push_front(result, i->rhs());
  }
  return atermpp::reverse(result);
}

/// INTERNAL ONLY
/// Function object for comparing a data variable with the
/// the left hand side of a data assignment.
// TODO: move to detail directory
struct compare_assignment_lhs
{
  data_variable m_variable;

  compare_assignment_lhs(const data_variable& variable)
    : m_variable(variable)
  {}
  
  bool operator()(const data_assignment& a) const
  {
    return m_variable == a.lhs();
  }
};

/// INTERNAL ONLY
/// Utility class for applying a list of assignments to a term.
// TODO: move to detail directory
struct assignment_list_substitution_helper
{
  const data_assignment_list& l;
  
  assignment_list_substitution_helper(const data_assignment_list& l_)
    : l(l_)
  {}
  
  std::pair<aterm_appl, bool> operator()(aterm_appl t) const
  {
    if (!is_data_variable(t))
    {
      return std::make_pair(t, true); // continue the recursion
    }
    data_assignment_list::iterator i = std::find_if(l.begin(), l.end(), compare_assignment_lhs(t));
    if (i == l.end())
    {
      return std::make_pair(t, false); // don't continue the recursion
    }
    else
    {
      return std::make_pair(i->rhs(), false); // don't continue the recursion
    }
  }
};

///////////////////////////////////////////////////////////////////////////////
// assignment_list_substitution
/// Utility class for applying a sequence of data assignments. Can be used
/// in the atermpp replace algorithms.
struct assignment_list_substitution
{
  const data_assignment_list& m_assignments;
  
  assignment_list_substitution(const data_assignment_list& assignments)
    : m_assignments(assignments)
  {}
  
  aterm operator()(aterm t) const
  {
    return partial_replace(t, assignment_list_substitution_helper(m_assignments));
  }
  private:
    assignment_list_substitution& operator=(const assignment_list_substitution&)
    {
      return *this;
    }
};

} // namespace lps

/// INTERNAL ONLY
namespace atermpp
{
using lps::data_variable;
using lps::data_application;
using lps::data_operation;
using lps::data_assignment;
using lps::data_equation;

template<>
struct aterm_traits<data_variable>
{
  typedef ATermAppl aterm_type;
  static void protect(data_variable t)   { t.protect(); }
  static void unprotect(data_variable t) { t.unprotect(); }
  static void mark(data_variable t)      { t.mark(); }
  static ATerm term(data_variable t)     { return t.term(); }
  static ATerm* ptr(data_variable& t)    { return &t.term(); }
};

template<>
struct aterm_traits<data_application>
{
  typedef ATermAppl aterm_type;
  static void protect(data_application t)   { t.protect(); }
  static void unprotect(data_application t) { t.unprotect(); }
  static void mark(data_application t)      { t.mark(); }
  static ATerm term(data_application t)     { return t.term(); }
  static ATerm* ptr(data_application& t)    { return &t.term(); }
};

template<>
struct aterm_traits<data_operation>
{
  typedef ATermAppl aterm_type;
  static void protect(data_operation t)   { t.protect(); }
  static void unprotect(data_operation t) { t.unprotect(); }
  static void mark(data_operation t)      { t.mark(); }
  static ATerm term(data_operation t)     { return t.term(); }
  static ATerm* ptr(data_operation& t)    { return &t.term(); }
};

template<>
struct aterm_traits<data_assignment>
{
  typedef ATermAppl aterm_type;
  static void protect(data_assignment t)   { t.protect(); }
  static void unprotect(data_assignment t) { t.unprotect(); }
  static void mark(data_assignment t)      { t.mark(); }
  static ATerm term(data_assignment t)     { return t.term(); }
  static ATerm* ptr(data_assignment& t)    { return &t.term(); }
};

template<>
struct aterm_traits<data_equation>
{
  typedef ATermAppl aterm_type;
  static void protect(data_equation t)   { t.protect(); }
  static void unprotect(data_equation t) { t.unprotect(); }
  static void mark(data_equation t)      { t.mark(); }
  static ATerm term(data_equation t)     { return t.term(); }
  static ATerm* ptr(data_equation& t)    { return &t.term(); }
};

} // namespace atermpp

#endif // LPS_DATA_H
