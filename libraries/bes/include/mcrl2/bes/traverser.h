// Author(s): Jeroen van der Wulp, Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/bes/traverser.h
/// \brief add your file description here.

// To avoid circular inclusion problems
#ifndef MCRL2_BES_BOOLEAN_EQUATION_SYSTEM_H
#include "mcrl2/bes/boolean_equation_system.h"
#endif

#ifndef MCRL2_BES_TRAVERSER_H
#define MCRL2_BES_TRAVERSER_H

#include "mcrl2/core/traverser.h"

namespace mcrl2 {

namespace bes {

  /// \brief Traversal class for BES data types
  template <typename Derived>
  class traverser: public core::traverser<Derived>
  {
    public:
      typedef core::traverser<Derived> super;

      using super::enter;
      using super::leave;
      using super::operator();

      void operator()(const core::identifier_string& )
      { }

      void operator()(const fixpoint_symbol& )
      { }

// Include file with traverse member functions. This is to avoid duplication.
#include "mcrl2/bes/detail/traverser.inc.h"
  };

  /// \brief Selective traversal class for BES data types
  template <typename Derived, typename AdaptablePredicate>
  class selective_traverser : public core::selective_traverser<Derived, AdaptablePredicate, bes::traverser>
  {
    public:
      typedef core::selective_traverser<Derived, AdaptablePredicate, bes::traverser> super;

      using super::enter;
      using super::leave;
      using super::operator();

      selective_traverser()
      { }

      selective_traverser(AdaptablePredicate predicate) : super(predicate)
      { }
  };

//--- start generated add_traverser_boolean_expressions code ---//
  template <template <class> class Traverser, class Derived>
  struct add_traverser_boolean_expressions: public Traverser<Derived>
  {
    typedef Traverser<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    void operator()(const bes::boolean_equation& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.formula());
      static_cast<Derived&>(*this).leave(x);
    }
    
    template <typename Container>
    void operator()(const bes::boolean_equation_system<Container>& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.equations());
      static_cast<Derived&>(*this)(x.initial_state());
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::true_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::false_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::not_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.operand());
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::and_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.left());
      static_cast<Derived&>(*this)(x.right());
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::or_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.left());
      static_cast<Derived&>(*this)(x.right());
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::imp& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.left());
      static_cast<Derived&>(*this)(x.right());
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::boolean_variable& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::boolean_expression& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      bes::boolean_expression result;
      if (bes::is_true(x)) { static_cast<Derived&>(*this)(bes::true_(atermpp::aterm_appl(x))); }
      else if (bes::is_false(x)) { static_cast<Derived&>(*this)(bes::false_(atermpp::aterm_appl(x))); }
      else if (bes::is_not(x)) { static_cast<Derived&>(*this)(bes::not_(atermpp::aterm_appl(x))); }
      else if (bes::is_and(x)) { static_cast<Derived&>(*this)(bes::and_(atermpp::aterm_appl(x))); }
      else if (bes::is_or(x)) { static_cast<Derived&>(*this)(bes::or_(atermpp::aterm_appl(x))); }
      else if (bes::is_imp(x)) { static_cast<Derived&>(*this)(bes::imp(atermpp::aterm_appl(x))); }
      else if (bes::is_boolean_variable(x)) { static_cast<Derived&>(*this)(bes::boolean_variable(atermpp::aterm_appl(x))); }
      static_cast<Derived&>(*this).leave(x);
    }
    
  };

  /// \brief Traverser class
  template <typename Derived>
  struct boolean_expression_traverser: public add_traverser_boolean_expressions<core::traverser, Derived>
  {
    typedef add_traverser_boolean_expressions<core::traverser, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated add_traverser_boolean_expressions code ---//

//--- start generated add_traverser_boolean_variables code ---//
  template <template <class> class Traverser, class Derived>
  struct add_traverser_boolean_variables: public Traverser<Derived>
  {
    typedef Traverser<Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();

    void operator()(const bes::boolean_equation& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.variable());
      static_cast<Derived&>(*this)(x.formula());
      static_cast<Derived&>(*this).leave(x);
    }
    
    template <typename Container>
    void operator()(const bes::boolean_equation_system<Container>& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.equations());
      static_cast<Derived&>(*this)(x.initial_state());
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::true_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::false_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::not_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.operand());
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::and_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.left());
      static_cast<Derived&>(*this)(x.right());
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::or_& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.left());
      static_cast<Derived&>(*this)(x.right());
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::imp& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      static_cast<Derived&>(*this)(x.left());
      static_cast<Derived&>(*this)(x.right());
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::boolean_variable& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      // skip
      static_cast<Derived&>(*this).leave(x);
    }
    
    void operator()(const bes::boolean_expression& x)
    {
      static_cast<Derived&>(*this).enter(x);  
      bes::boolean_expression result;
      if (bes::is_true(x)) { static_cast<Derived&>(*this)(bes::true_(atermpp::aterm_appl(x))); }
      else if (bes::is_false(x)) { static_cast<Derived&>(*this)(bes::false_(atermpp::aterm_appl(x))); }
      else if (bes::is_not(x)) { static_cast<Derived&>(*this)(bes::not_(atermpp::aterm_appl(x))); }
      else if (bes::is_and(x)) { static_cast<Derived&>(*this)(bes::and_(atermpp::aterm_appl(x))); }
      else if (bes::is_or(x)) { static_cast<Derived&>(*this)(bes::or_(atermpp::aterm_appl(x))); }
      else if (bes::is_imp(x)) { static_cast<Derived&>(*this)(bes::imp(atermpp::aterm_appl(x))); }
      else if (bes::is_boolean_variable(x)) { static_cast<Derived&>(*this)(bes::boolean_variable(atermpp::aterm_appl(x))); }
      static_cast<Derived&>(*this).leave(x);
    }
    
  };

  /// \brief Traverser class
  template <typename Derived>
  struct boolean_variable_traverser: public add_traverser_boolean_variables<core::traverser, Derived>
  {
    typedef add_traverser_boolean_variables<core::traverser, Derived> super;
    using super::enter;
    using super::leave;
    using super::operator();
  };
//--- end generated add_traverser_boolean_variables code ---//

//--- start generated add_traverser_identifier_strings code ---//
//--- end generated add_traverser_identifier_strings code ---//

} // namespace bes

} // namespace mcrl2

#endif // MCRL2_BES_TRAVERSER_H
