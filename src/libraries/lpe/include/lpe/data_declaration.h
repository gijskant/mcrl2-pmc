///////////////////////////////////////////////////////////////////////////////
/// \file data_declaration.h

#ifndef LPE_DATA_DECLARATION_H
#define LPE_DATA_DECLARATION_H

#include "atermpp/aterm.h"
#include "lpe/sort.h"
#include "lpe/function.h"
#include "lpe/data.h"

namespace lpe {

using atermpp::aterm_appl;
using atermpp::aterm_list;

///////////////////////////////////////////////////////////////////////////////
// data_declaration
/// \brief a data declaration of a mCRL specification.
///
class data_declaration
{
  // N.B. A data_declaration is not explicitly represented in the specification.

  protected:
    sort_list          m_sorts;       
    function_list      m_constructors;
    function_list      m_mappings;    
    data_equation_list m_equations;   

  public:
    typedef sort_list::iterator          sort_iterator;
    typedef function_list::iterator      function_iterator;
    typedef data_equation_list::iterator equation_iterator;

    data_declaration()
    {}

    data_declaration(aterm_list sorts, aterm_list constructors, aterm_list mappings, aterm_list equations)
      : m_sorts(sorts),
        m_constructors(constructors),
        m_mappings(mappings),
        m_equations(equations)
    {}

    /// Returns the list of sorts.
    ///
    sort_list sorts() const
    {
      return m_sorts;
    }

    /// Returns the list of constructors.
    ///
    function_list constructors() const
    {
      return m_constructors;
    }

    /// Returns the list of mappings.
    ///
    function_list mappings() const
    {
      return m_mappings;
    }

    /// Returns the list of equations.
    ///
    data_equation_list equations() const
    {
      return m_equations;
    }

    /// Sets the sequence of sorts.
    ///
    void set_sorts(sort_list sorts)
    { m_sorts = sorts; }

    /// Sets the sequence of constructors.
    ///
    void set_constructors(function_list constructors)
    { m_constructors = constructors; }

    /// Sets the sequence of mappings.
    ///
    void set_mappings(function_list mappings)
    { m_mappings = mappings; }

    /// Sets the sequence of data equations.
    ///
    void set_equations(data_equation_list equations)
    { m_equations = equations; }
};

} // namespace mcrl

#endif // LPE_DATA_DECLARATION_H
