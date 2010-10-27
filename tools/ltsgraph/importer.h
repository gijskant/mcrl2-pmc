// Author(s): Carst Tankink
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file importer.h
/// \brief Importer superclass.

#ifndef IMPORTER_H
#define IMPORTER_H

#include <string>
#include "graph.h"

class Importer
{
  public:
    Importer();
    virtual ~Importer();
    virtual Graph* importFile(const std::string &filename) = 0;

};

#endif //IMPORTER_H
