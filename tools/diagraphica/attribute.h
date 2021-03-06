// Author(s): A.J. (Hannes) Pretorius
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ./attribute.h

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <QtCore>
#include <QtGui>

#include <algorithm>
#include <cstddef>
#include <string>
#include <map>
#include <vector>
#include "value.h"

#ifndef NON_EXISTING
#define NON_EXISTING (size_t)(-1)
#endif

class Attribute : public QObject
{
  Q_OBJECT

  public:
    // -- constructors and destructor -------------------------------
    Attribute(
      QString name,
      QString type,
      const size_t& idx);
    Attribute(const Attribute& attr);
    virtual ~Attribute();

    // -- set functions ---------------------------------------------
    void setIndex(const size_t& idx);
    void setName(QString name);
    void setType(QString type);

    virtual void clusterValues(
      const std::vector< int > &indices,
      const std::string& newValue);
    virtual void moveValue(
      const size_t& idxFr,
      const size_t& idxTo);

    // functions overridden by AttrDiscr
    virtual void configValues(
      const std::vector< std::string > &curDomain,
      std::map< size_t, size_t  > &origToCurDomain);

    // -- get functions ---------------------------------------------
    size_t getIndex();
    QString name();
    QString type();

    // functions overridden by AttrDiscr
    virtual size_t getSizeOrigValues();
    virtual Value* getOrigValue(size_t idx);
    virtual Value* getCurValue(size_t idx);

    virtual size_t getSizeCurValues() = 0;
    virtual Value* mapToValue(double key) = 0;

    // -- clear functions -------------------------------------------
    virtual void clearClusters() = 0;

  public:
    void emitDuplicated() { emit duplicated(); }
    void emitDeleted() { emit deleted(); }
    void emitMoved(int newPosition) { emit moved(newPosition); }

  signals:
    void changed();
    void duplicated();
    void renamed();
    void deleted();
    void moved(int newPosition);

  protected:
    // -- private utility functions ---------------------------------
    virtual void deleteCurValues() = 0;
    virtual void deleteCurMap() = 0;

    // -- data members ----------------------------------------------
    size_t    index;
    QString m_name;
    QString m_type;
};

#endif

// -- end -----------------------------------------------------------
