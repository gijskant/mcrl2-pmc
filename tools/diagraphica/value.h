// Author(s): A.J. (Hannes) Pretorius
// Copyright: see the accompanying file COPYING.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// LICENSE_1_0.txt)
//
/// \file ./value.h

// --- value.h ------------------------------------------------------
// (c) 2007  -  A.J. Pretorius  -  Eindhoven University of Technology
// ---------------------------  *  ----------------------------------

#ifndef VALUE_H
#define VALUE_H

#include <cstddef>
#include <string>
using namespace std;

class Value
{
public:
	// -- constructors and destructor -------------------------------
    Value( 
        const int &idx,
        const string &val );
    Value( const Value &val );
	virtual ~Value();
	
    // -- set functions ---------------------------------------------
    void setIndex( const int &idx );
    void setValue( const string &val );
    
    // -- get functions ---------------------------------------------
    int getIndex();
    string getValue();

protected:
    // -- data members ----------------------------------------------
	int    index; // index in attribute
    string value; // actual value
};

#endif

// -- end -----------------------------------------------------------
