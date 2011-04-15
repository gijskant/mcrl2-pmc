// Author(s): Muck van Weerdenburg
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/data/detail/rewrite/jitty.h

#ifndef __REWR_JITTY_H
#define __REWR_JITTY_H

#include "mcrl2/data/detail/rewrite.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/atermpp/map.h"

namespace mcrl2
{
namespace data
{
namespace detail
{

class RewriterJitty: public Rewriter
{
  public:
    RewriterJitty(const data_specification& DataSpec);
    ~RewriterJitty();

    RewriteStrategy getStrategy();

    ATermAppl rewrite(ATermAppl Term);

    ATerm toRewriteFormat(ATermAppl Term);
    ATermAppl fromRewriteFormat(ATerm Term);
    ATerm rewriteInternal(ATerm Term);

    bool addRewriteRule(ATermAppl Rule);
    bool removeRewriteRule(ATermAppl Rule);

  private:
    unsigned int num_opids;
    size_t max_vars;
    bool need_rebuild;

    ATermAppl jitty_true;

    atermpp::map< ATerm, ATermInt > term2int;
    atermpp::map< ATermInt, ATermList > jitty_eqns;
    ATermList* jitty_strat;

    ATermAppl rewrite_aux(ATermAppl Term);

    ATerm OpId2Int(ATermAppl Term, bool add_opids);
    ATermAppl toInner(ATermAppl Term, bool add_opids);
    ATermAppl fromInner(ATermAppl Term);

    const mcrl2::data::data_specification *m_data_specification;

    ATerm internal_existential_quantifier_enumeration( ATerm ATermInInnerFormat );
    ATerm internal_universal_quantifier_enumeration( ATerm ATermInInnerFormat );
    ATerm internal_quantifier_enumeration( ATerm ATermInInnerFormat );


};
}
}
}

#endif
