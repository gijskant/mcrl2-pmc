// Author(s): Jeroen van der Wulp
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef __COMMAND_LINE_REWRITING_H
#define __COMMAND_LINE_REWRITING_H

#include "mcrl2/utilities/command_line_interface.h"
#include "mcrl2/data/rewriter.h"

namespace mcrl2 {
  namespace utilities {
    /// \cond INTERNAL_DOCS
    inline std::istream& operator>>(std::istream& is, data::rewriter::strategy& s) {
      char strategy[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

      using namespace mcrl2::data::detail;

      is.readsome(strategy, 9);

      size_t new_s = static_cast< size_t >(RewriteStrategyFromString(strategy));

      s = static_cast< data::rewriter::strategy >(new_s);

      if (static_cast< size_t >(new_s) == static_cast< size_t >(GS_REWR_INVALID)) {
        is.setstate(std::ios_base::failbit);
      }

      return is;
    }

    namespace detail {
      template <>
      struct initialiser< data::rewriter::strategy > {
        static void add_options(interface_description& standard) {
          standard.add_option(
            "rewriter", interface_description::mandatory_argument< data::rewriter::strategy >("NAME", "jitty"),
            "use rewrite strategy NAME:\n"
            "  'jitty' for jitty rewriting (default),\n"
            "  'jittyp' for jitty rewriting with prover,\n"
#ifdef MCRL2_JITTYC_AVAILABLE
            "  'jittyc' for compiled jitty rewriting,\n"
#endif
            "  'inner' for innermost rewriting,\n"
            "  'innerp' for innermost rewriting with prover, or\n"
#ifdef MCRL2_JITTYC_AVAILABLE
            "  'innerc' for compiled innermost rewriting"
#endif
            ,'r'
          );
        }
      };

      void register_rewriting_options(interface_description& d) {
        initialiser< data::rewriter::strategy >::add_options(d);
      }
    }
  /// \endcond
  }
}
#endif

