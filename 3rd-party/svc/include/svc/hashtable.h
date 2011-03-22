/*
   SVC -- the SVC (Systems Validation Centre) file format library

   Copyright (C) 2000  Stichting Mathematisch Centrum, Amsterdam,
                       The  Netherlands

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   $Id: hashtable.h,v 1.2 2008/09/30 08:22:51 bertl Exp $ */

#ifndef __HASH_TABLE_H
#define __HASH_TABLE_H

#include "aterm2.h"
#include "ptable.h"

  typedef struct
  {
    ATermIndexedSet terms;
    PTable pointers;
  } HTable;

  int HTinit(HTable*);
  void HTfree(HTable*);
  unsigned int HTinsert(HTable*, ATerm, void*);
  int HTmember(HTable*, ATerm, long*);
  ATerm HTgetTerm(HTable*, long);
  void* HTgetPtr(HTable*, long);
  void HTsetPtr(HTable*, long, void*);

#endif


