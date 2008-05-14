//  Author(s): A.j. (Hannes) pretorius
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  LICENSE_1_0.txt)
//
/// \file ./colleague.cpp


#include "colleague.h"


// -- contstructors and destructors ---------------------------------


// --------------------------------
Colleague::Colleague( Mediator* m )
// --------------------------------
{
    mediator = m;
}


// -----------------------------------------------
Colleague::Colleague( const Colleague &colleague )
// -----------------------------------------------
{
    mediator = colleague.mediator;
}


// --------------------
Colleague::~Colleague()
// --------------------
{
    mediator = NULL;
}


// -- end -----------------------------------------------------------
