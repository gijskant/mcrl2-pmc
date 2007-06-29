//  Copyright 2007 A.j. (Hannes) pretorius. Distributed under the Boost
//  Software License, Version 1.0. (See accompanying file
//  LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ./colleague.h

// --- colleague.h --------------------------------------------------
// (c) 2007  -  A.J. Pretorius  -  Eindhoven University of Technology
// ---------------------------  *  ----------------------------------

// ------------------------------------------------------------------
// This is the BASE CLASS from which all colleages inheret and 
// provides acces to a single Mediator. This allows for the 
// implementation of a MEDIATOR design pattern where an instance of
// Mediator serves as the mediator or controller.
// ------------------------------------------------------------------

#ifndef COLLEAGUE_H
#define COLLEAGUE_H

#include <cstddef>
#include <cstdlib>
#include "mediator.h"

class Colleague
{
public:
    // -- contstructors and destructors -----------------------------
    Colleague( Mediator* m );
    Colleague( const Colleague &colleague );
    ~Colleague();
    
protected:
    // -- data members ----------------------------------------------
    Mediator* mediator;     // association
};

#endif

// -- end -----------------------------------------------------------
