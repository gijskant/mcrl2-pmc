// Author(s): Jeroen van der Wulp, Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/process/traverser.h
/// \brief add your file description here.

// To avoid circular inclusion problems
#ifndef MCRL2_PROCESS_SPECIFICATION_H
#include "mcrl2/process/process_specification.h"
#endif

#ifndef MCRL2_PROCESS_TRAVERSER_H
#define MCRL2_PROCESS_TRAVERSER_H

#include "mcrl2/data/traverser.h"
#include "mcrl2/lps/specification.h"
#include "mcrl2/process/process_specification.h"

namespace mcrl2 {

namespace process {

  /// \brief Traversal class for process data types
  template <typename Derived>
  class traverser: public data::traverser<Derived>
  {
    public:
      typedef data::traverser<Derived> super;
      using super::operator();
      using super::enter;
      using super::leave;

#include "mcrl2/lps/detail/traverser.inc.h" // needed for traversal of lps::action
#include "mcrl2/process/detail/traverser.inc.h"
  };

  template <typename Derived>
  class binding_aware_traverser: public data::binding_aware_traverser<Derived>
  {
    public:
      typedef data::binding_aware_traverser<Derived> super;
      using super::operator();
      using super::enter;
      using super::leave;

#include "mcrl2/lps/detail/traverser.inc.h" // needed for traversal of lps::action
#include "mcrl2/process/detail/traverser.inc.h"
  };

  /// \brief Selective traversal class for process data types
  template <typename Derived, typename AdaptablePredicate>
  class selective_traverser : public core::selective_traverser<Derived, AdaptablePredicate, process::traverser>
  {
    public:
      typedef core::selective_traverser<Derived, AdaptablePredicate, process::traverser> super;
      using super::operator();
      using super::enter;
      using super::leave;

      selective_traverser()
      { }

      selective_traverser(AdaptablePredicate predicate) : super(predicate)
      { }
  };

//  template <typename Derived, typename AdaptablePredicate>
//  class selective_binding_aware_traverser: public process::selective_traverser<Derived, AdaptablePredicate, process::binding_aware_traverser>
//  {
//    public:
//      typedef process::selective_traverser<Derived, AdaptablePredicate, process::binding_aware_traverser> super;
//      using super::enter;
//      using super::leave;
//      using super::operator();
//
//      selective_binding_aware_traverser()
//      { }
//
//      selective_binding_aware_traverser(AdaptablePredicate predicate): super(predicate)
//      { }
//  };

} // namespace process

} // namespace mcrl2

#endif // MCRL2_PROCESS_TRAVERSER_H
