// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/process/detail/process_sort_traverser.h
/// \brief add your file description here.

#ifndef MCRL2_PROCESS_DETAIL_PROCESS_SORT_TRAVERSER_H
#define MCRL2_PROCESS_DETAIL_PROCESS_SORT_TRAVERSER_H

#include "mcrl2/data/traverse.h"
#include "mcrl2/process/process_specification.h"

namespace mcrl2 {

namespace process {

namespace detail {

  /// \brief Function object for applying a substitution to LPS data types.
  template <typename OutIter>
  struct process_sort_traverser: process_expression_visitor<>
  {
    OutIter dest;

    typedef process_expression_visitor<> super;
    
    process_sort_traverser(OutIter dest_)
      : dest(dest_)
    {}

    /// \brief Traverses the elements of a container
    template <typename Container>
    void traverse_container(const Container& c)
    {
      for (typename Container::const_iterator i = c.begin(); i != c.end(); ++i)
      {
        traverse(*i);
      }
    }   
  
    /// \brief Traverses a sort expression
    /// \param d A sort expression
    void traverse(const data::sort_expression& d)    
    {                                         
      dest = data::traverse_sort_expressions(d, dest);
    } 
  
    /// \brief Traverses a variable
    /// \param d A variable
    void traverse(const data::variable& d)    
    {                                         
      dest = data::traverse_sort_expressions(d, dest);
    } 
  
    /// \brief Traverses a data expression
    /// \param d A data expression
    void traverse(const data::data_expression& d)    
    {                                         
      dest = data::traverse_sort_expressions(d, dest);
    } 
  
    /// \brief Traverses an assignment
    /// \param a An assignment
    void traverse(const data::assignment& a)
    {
      dest = data::traverse_sort_expressions(a, dest);
    } 

    /// \brief Traverses a process expression
    /// \param d A process expression
    void traverse(const process_expression& p)    
    {
      super::visit(p);
    } 
  
    /// \brief Traverses an action label
    void traverse(const lps::action_label& l)
    {
      traverse_container(l.sorts());
    }

    /// \brief Traverses an action
    /// \param a An action
    void traverse(const lps::action& a)
    {
      traverse(a.label());
      traverse_container(a.arguments());
    }

    void traverse(const process_initialization& init)
    {
      traverse_container(init.free_variables());
      traverse(init.expression());
    }
    
    void traverse(const process_identifier& pi)
    {
      traverse_container(pi.sorts());
    }
    
    void traverse(const process_equation& eq)
    {
      traverse_container(eq.free_variables());
      traverse_container(eq.formal_parameters());
      traverse(eq.identifier());
      traverse(eq.expression());
    }

    void traverse(const process_specification& spec)
    {
      traverse_container(spec.action_labels());
      traverse_container(spec.equations());
      traverse(spec.init());
    }

    template <typename Term>
    void operator()(const Term& t)
    {
      traverse(t);
    }

    //---------------------------------------------------------------//
    //                visit functions
    //---------------------------------------------------------------//

    /// \brief Visit action node
    /// \return The result of visiting the node
    bool visit_action(const process_expression& x, const lps::action_label& l, const data::data_expression_list& v)
    {
      traverse(l);
      traverse_container(v);
      return continue_recursion;
    }
  
    /// \brief Visit process_assignment node
    /// \return The result of visiting the node
    bool visit_process_instance_assignment(const process_expression& x, const process_identifier& pi, const data::assignment_list& v)
    {
      traverse(pi);
      traverse_container(v);
      return continue_recursion;
    }
  
    /// \brief Visit sum node
    /// \return The result of visiting the node
    bool visit_sum(const process_expression& x, const data::variable_list& v, const process_expression& right)
    {
      traverse_container(v);
      return continue_recursion;
    }
  
    /// \brief Visit at_time node
    /// \return The result of visiting the node
    bool visit_at(const process_expression& x, const process_expression& left, const data::data_expression& d)
    {
      traverse(d);
      return continue_recursion;
    }
  
    /// \brief Visit if_then node
    /// \return The result of visiting the node
    bool visit_if_then(const process_expression& x, const data::data_expression& d, const process_expression& right)
    {
      traverse(d);
      return continue_recursion;
    }
  
    /// \brief Visit if_then_else node
    /// \return The result of visiting the node
    bool visit_if_then_else(const process_expression& x, const data::data_expression& d, const process_expression& left, const process_expression& right)
    {
      traverse(d);
      return continue_recursion;
    }
  };

  /// \brief Utility function to create an process_sort_traverser.
  template <typename OutIter>    
  process_sort_traverser<OutIter> make_process_sort_traverser(OutIter dest)        
  {
    return process_sort_traverser<OutIter>(dest);
  } 

} // namespace detail

} // namespace process

} // namespace mcrl2

#endif // MCRL2_PROCESS_DETAIL_PROCESS_SORT_TRAVERSER_H
