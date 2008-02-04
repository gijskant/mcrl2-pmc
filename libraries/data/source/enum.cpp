// Author(s): Muck van Weerdenburg
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file mcrl2/enum.cpp

#include "mcrl2/data/rewrite.h"
#include "mcrl2/data/enum.h"
#include "mcrl2/data/enum/standard.h"

EnumeratorSolutions::~EnumeratorSolutions()
{
}

Enumerator::~Enumerator()
{
}

Enumerator *createEnumerator(ATermAppl data_spec, Rewriter *r, bool clean_up_rewriter, EnumerateStrategy strategy)
{
	switch ( strategy )
	{
		case ENUM_STANDARD:
			return new EnumeratorStandard(data_spec, r,clean_up_rewriter);
		default:
			return NULL;
	}
}
