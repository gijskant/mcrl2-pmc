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

#include <limits.h>
#include "mcrl2/atermpp/detail/utility.h"
#include "mcrl2/atermpp/aterm_io.h"
#include "mcrl2/atermpp/aterm_int.h"
#include <svc/compress.h>


using namespace atermpp;

static char buffer[102400];
static void calcDelta(CompressedStream*, long*);
static void uncalcDelta(CompressedStream*, long*);



CompressedStream* CSinit(HTable* table, BitStream* bs, int indexed)
{
  CompressedStream* ret;

  ret=(CompressedStream*)malloc(sizeof(CompressedStream));
  if (indexed)
  {
    ret->indices=table;
    HTinit(&ret->deltas);
    HFinit(&ret->tree, &ret->deltas);
  }
  else
  {
    HTinit(&ret->deltas);
    HFinit(&ret->tree, table);
  }

  ret->bs=bs;
  ret->last=0;
  ret->preLast=0;

  return ret;
}


void CSfree(CompressedStream* cs)
{

  HFfree(&cs->tree);
  HTfree(&cs->deltas);
  free(cs);

}

int CSreadIndex(CompressedStream* cs, aterm* term)
{
  long index;
  if (HFdecodeIndex(cs->bs, &cs->tree, &index))
  {
    uncalcDelta(cs,&index);
    *term=aterm_int(index);
    HTinsert(cs->indices,*term,NULL); /* IZAK */
    return 1;
  }
  else
  {
    return 0;
  }
}


int CSreadATerm(CompressedStream* cs, aterm* term)
{

  if (HFdecodeATerm(cs->bs, &cs->tree, term))
  {

    if (detail::address(*term)==NULL)
    {
      return 0;
    }
    else
    {
      return 1;
    }
  }
  else
  {
    return 0;
  }

}

int CSureadATerm(CompressedStream* cs, aterm* term)
{

  if (BSreadString(cs->bs,buffer))
  {
    *term=read_term_from_string(buffer);
    return 1;
  }
  else
  {
    return 0;
  }
}

/* static int CSreadString(CompressedStream* cs)
{
  aterm term;

  HFdecodeATerm(cs->bs, &cs->tree, &term); 
  return 0;

} */

int CSureadString(CompressedStream* cs, char** str)
{

  if (BSreadString(cs->bs, buffer))
  {
    *str=buffer;
    return 1;
  }
  else
  {
    return 0;
  }
}


int CSreadInt(CompressedStream* cs, long* n)
{
  aterm term;

  /* if(HFdecodeATerm(cs->bs, &cs->tree, &term) && ATmatch(term, "<int>", &n)) */
  if (HFdecodeATerm(cs->bs, &cs->tree, &term) && term.type_is_int())
  {
    *n =((aterm_int)term).value();
    return 1;
  }
  else
  {
    return 0;
  }

}

int CSureadInt(CompressedStream* cs, long* n)
{

  if (BSreadInt(cs->bs,n))
  {
    /*
    fprintf(stderr, "Read int %ld\n", *n);
    */
    return 1;
  }
  else
  {
    return 0;
  }

}



int CSwriteIndex(CompressedStream* cs, aterm term)
{
  long index;


  if (detail::address(term)==NULL)
  {
    return HFencodeIndex(cs->bs, &cs->tree, NO_INT);
  }
  else
  {
    if (!HTmember(cs->indices,term,&index))
    {
      index=HTinsert(cs->indices,term,NULL);
    }
    calcDelta(cs, &index);

    return HFencodeIndex(cs->bs, &cs->tree, index);
  }
}


int CSwriteATerm(CompressedStream* cs, aterm term)
{

  return HFencodeATerm(cs->bs, &cs->tree, term);
}

int CSuwriteATerm(CompressedStream* cs, aterm term)
{

  BSwriteString(cs->bs,to_string(term).c_str());
  return 1;
}
int CSwriteString(CompressedStream* cs, const char* str)
{

  return HFencodeATerm(cs->bs, &cs->tree, aterm_appl(function_symbol(str,0)));
}

int CSuwriteString(CompressedStream* cs, const char* str)
{
  BSwriteString(cs->bs,str);
  return 1;
}
int CSwriteInt(CompressedStream* cs, long n)
{
  return HFencodeATerm(cs->bs, &cs->tree, aterm_int(n));
}
int CSuwriteInt(CompressedStream* cs, long n)
{
  BSwriteInt(cs->bs,n);
  return 1;
}

void  CSflush(CompressedStream* cs)
{

  BSflush(cs->bs);

}

int  CSseek(CompressedStream* cs, long offset, int whence)
{

  return BSseek(cs->bs, offset, whence);
}

long CStell(CompressedStream* cs)
{

  return BStell(cs->bs);
}



static void calcDelta(CompressedStream* bs, long* n)
{
  long tmp;

  tmp=*n;
  *n=*n-bs->preLast;
  bs->preLast=bs->last;
  bs->last=tmp;
}

static void uncalcDelta(CompressedStream* bs, long* n)
{
  *n=*n+bs->preLast;
  bs->preLast=bs->last;
  bs->last=*n;
}
