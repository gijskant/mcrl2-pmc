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

*/
/*
 * This module implements a hash table for storing mCRL states,
 * represented as term-number pairs. All the states stored in a
 * table are protected by being inserted in a protected aterm
 * list constructed with "ins" and "emt". The hashing function
 * assumes that each aterm address is unique. The operations
 * provided are insertion and membership.
 */

/* Changes per 1/1/00 by Izak van Langevelde:
   - numbers used are unsigned long instead of long, 0 means absent
   - indices returned are in the range 1..N, but internally in the
     term table the entries are in the range 0..N-1
   - hash table is parameter instead of global variable
   - errors are flagged by return value instead of error message
   Changes per 1/5/00 by Izak van Langevelde:
   - use the new aterm lib with indexed sets*/

#include <svc/hashtable.h>

using namespace atermpp;


/* ======= Initialize the hash table ======= */
int HTinit(HTable* table)
{
  new (&table->terms) atermpp::indexed_set(PT_INITIALSIZE, 75); // Placement new.
  PTinit(&table->pointers);

  return 0;
}

void HTfree(HTable* table)
{
  using namespace atermpp;
  table->terms.clear();  // Remove all elements, but do not destroy. 
                         // This is sometimes (sic) done at later instance.
                         // But the content of this indexed set must be destroyed.
                         // HTables are constructed by new, and in ordinary
                         // ways, this needs to be investigated.
  PTfree(&table->pointers);
}

unsigned int HTinsert(HTable* table, aterm a, void* ptr)
{

  // ret= (unsigned int)ATindexedSetPut(table->terms, a, &_new);

  unsigned int ret= (unsigned int)table->terms.put(a).first; 

  PTput(&table->pointers,ret,ptr);

  return ret;
}


int HTmember(HTable* table, aterm a, long* pn)
{
  int index;
  index=(int)table->terms.index(a);

  if (index!=-1)
  {
    *pn=index;
    return 1;
  }
  else
  {
    return 0;
  }

}

/*----------------------------------------------------------*/
/* Get the term belonging to an index */

aterm HTgetTerm(HTable* table, long n)
{

  return table->terms.get(n);

}


/*----------------------------------------------------------*/
/* Get the stored pointer belonging to an index */

void* HTgetPtr(HTable* table, long n)
{

  return PTget(&table->pointers,n);
}


void HTsetPtr(HTable* table, long n, void* ptr)
{

  PTput(&table->pointers, n,ptr);
}
