/*{{{  includes */

#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <stdexcept>
#include "_aterm.h"
#include "aterm2.h"
#include "memory.h"
#include "util.h"
#include "gc.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

/*}}}  */

namespace aterm
{

/*{{{  defines */

static const size_t INITIAL_TERM_TABLE_CLASS = 17;

inline
void CHECK_HEADER(const header_type h)
{
  assert(GET_AGE(h)==0 && !IS_MARKED(h));
}
//#define CHECK_HEADER(h) assert((GET_AGE(h)==0) && !IS_MARKED(h) )

inline
void CHECK_ARGUMENT(const ATermAppl t, const size_t n)
{
  assert((GET_AGE(t->header) <= GET_AGE(ATgetArgument(t,n)->header)));
  assert((GET_AGE(t->header) <= GET_AGE(at_lookup_table[ATgetAFun((ATermAppl)t)]->header)));
}
/*
#define CHECK_ARGUMENT(t,n) { assert( (GET_AGE(t->header) <= GET_AGE(ATgetArgument(t,n)->header)) );\
    assert( (GET_AGE(t->header) <= GET_AGE(at_lookup_table[ATgetAFun((ATermAppl)t)]->header))); }
*/

inline
void CHECK_ARITY(const size_t ari1, const size_t ari2)
{
  assert((ari1) == (ari2));
}
//#define CHECK_ARITY(ari1,ari2) (assert((ari1) == (ari2)))

/* Uncomment the second line, to do a garbage collection before a term
 * is constructed. This can be used to find subtle garbage collection problems.
 * Note that this is very time consuming... The first line below expresses that
 * no agressive checking is done, which is the default. */
/* #define DO_AGGRESSIVE_GARBAGE_COLLECT  */

#ifdef DO_AGGRESSIVE_GARBAGE_COLLECT
#define AGGRESSIVE_GARBAGE_COLLECT_CHECK AT_collect()
#else
#define AGGRESSIVE_GARBAGE_COLLECT_CHECK
#endif


/* #define IDX(w)         HN(((w>>2) ^ (w>>10)) & 0xFF) */
/* #define SHIFT(w)       HN((w>>3) & 0xF) */

inline
size_t START(const MachineWord w)
{
  return FOLD(HIDE_AGE_MARK(w));
}
//#define START(w)       FOLD(HIDE_AGE_MARK(w))

inline
size_t COMBINE(const HashNumber hnr, const MachineWord w)
{
  return ((hnr)<<1 ^(hnr)>>1 ^ HN(FOLD(w)));
}
//#define COMBINE(hnr,w) ((hnr)<<1 ^ (hnr)>>1 ^ HN(FOLD(w)))

inline
HashNumber FINISH(const HashNumber hnr)
{
  return hnr;
}
//#define FINISH(hnr)    (hnr)

inline
void CHECK_TERM(const ATerm t)
{
  assert((t) != NULL && (AT_isValidTerm(t)));
}
//#define CHECK_TERM(t) assert((t) != NULL && (AT_isValidTerm(t) ))

/*}}}  */
/*{{{  globals */

char memory_id[] = "$Id$";

size_t maxTermSize = INITIAL_MAX_TERM_SIZE;

TermInfo* terminfo = NULL;

Block* at_freeblocklist = NULL;
size_t at_freeblocklist_size = 0;
BlockBucket block_table[BLOCK_TABLE_SIZE] = { { NULL, NULL } };

size_t total_nodes = 0;

static size_t table_class = INITIAL_TERM_TABLE_CLASS;
static HashNumber table_size    = AT_TABLE_SIZE(INITIAL_TERM_TABLE_CLASS);
static HashNumber table_mask    = AT_TABLE_MASK(INITIAL_TERM_TABLE_CLASS);

/*
 * For GC tuning
 */
size_t nb_minor_since_last_major = 0;
size_t old_bytes_in_young_blocks_after_last_major = 0; /* only live old cells in young blocks */
size_t old_bytes_in_old_blocks_after_last_major = 0; /* only live old cells in old blocks */
size_t old_bytes_in_young_blocks_since_last_major = 0; /* only live cells */

static int maxload = 80;
static ATerm* hashtable;

extern void AT_initMemmgnt();

ATermList ATempty;

size_t AT_getMaxTermSize()
{
  return maxTermSize;
}

/*{{{  static size_t term_size(ATerm t) */

/**
 * Calculate the size (in words) of a term.
 */

static size_t term_size(ATerm t)
{
  size_t size = 0;

  switch (ATgetType(t))
  {
    case AT_INT:
      size = TERM_SIZE_INT;
      break;
    case AT_LIST:
      size = TERM_SIZE_LIST;
      break;
    case AT_APPL:
      size = TERM_SIZE_APPL(ATgetArity(ATgetAFun((ATermAppl)t)));
      break;
  }

  return size;
}

/*}}}  */

/*{{{  static HashNumber hash_number(ATerm t, size_t size) */

static HashNumber hash_number(ATerm t, size_t size)
{
  HashNumber hnr;

  hnr = START(HIDE_AGE_MARK(t->word[0]));

  for (size_t i=ARG_OFFSET; i<size; i++)
  {
    hnr = COMBINE(hnr, t->word[i]);
  }

  return FINISH(hnr);
}


/*}}}  */
/*{{{  HashNumber AT_hashnumber(ATerm t) */

HashNumber AT_hashnumber(ATerm t)
{
  return hash_number(t, term_size(t));
}

/*}}}  */

/*{{{  static void resize_hashtable() */

/**
 * Resize the hashtable
 */

void resize_hashtable()
{
  ATerm* newhalf, *p;
  HashNumber oldsize;
  ATerm* newtable;

  oldsize = table_size;
  table_class++;
  table_size = ((HashNumber)1)<<table_class;
  table_mask = table_size-1;

  /*{{{  Create new term table */
  newtable = (ATerm*) AT_realloc(hashtable, table_size * sizeof(ATerm));

  if (newtable)
  {
    hashtable = newtable;
  }
  else
  {
    fprintf(stderr, "warning: could not resize hashtable to class %lu.\n",
    table_class);
    table_class--;
    table_size = ((HashNumber)1)<<table_class;
    table_mask = table_size-1;
    return;
  }
  /*}}}  */

  /*{{{  Clear 2nd half of new table, uses increment == 2*oldsize */
  memset(hashtable+oldsize, 0, oldsize*sizeof(ATerm));
  /*}}}  */

  /*{{{  Rehash all old elements */

  newhalf = hashtable + oldsize;
  for (p=hashtable; p < newhalf; p++)
  {
    ATerm marked = *p;
    ATerm prev = NULL;
    /*{{{  Loop over marked part */
    while (marked && IS_MARKED(marked->header))
    {
      CLR_MARK(marked->header);
      prev = marked;
      marked = marked->aterm.next;
    }
    /*}}}  */

    /*{{{  Loop over unmarked part */

    if (marked)
    {
      ATerm unmarked;
      ATerm* hashspot;

      if (marked == *p)
      {
        /* No marked terms */
        unmarked = marked;
        *p = NULL;
      }
      else
      {
        /* disconnect unmarked terms from rest */
        unmarked = marked;
        prev->aterm.next = NULL;
      }

      while (unmarked)
      {
        ATerm next = unmarked->aterm.next;
        HashNumber hnr;

        hnr = hash_number(unmarked, term_size(unmarked));
        hnr &= table_mask;
        hashspot = hashtable+hnr;
        unmarked->aterm.next = *hashspot;
        *hashspot = unmarked;

        if (hashspot > p && hashspot < newhalf)
        {
          SET_MARK(unmarked->header);
        }

        unmarked = next;
      }
    }

    /*}}}  */
  }

  /*}}}  */

}

/*}}}  */

/*{{{  void AT_initMemory(unsigned int argc, char *argv[]) */
/**
 * Initialize memory allocation datastructures
 */

void AT_initMemory(int, char**)
{
  HashNumber hnr;

  /*{{{  Initialize blocks */

  terminfo = (TermInfo*) AT_calloc(maxTermSize, sizeof(TermInfo));

  /*}}}  */
  /*{{{  Create term term table */

  hashtable = (ATerm*)AT_calloc(table_size, sizeof(ATerm));
  if (!hashtable)
  {
    std::runtime_error("AT_initMemory: cannot allocate term table of size " + to_string(table_size));
  }

  for (size_t i=0; i<BLOCK_TABLE_SIZE; i++)
  {
    block_table[i].first_before = NULL;
    block_table[i].first_after  = NULL;
  }

  /*}}}  */
  /*{{{  Create the empty list */

  ATempty = (ATermList)AT_allocate(TERM_SIZE_LIST);
  ATempty->header = EMPTY_HEADER;
  CHECK_HEADER(ATempty->header);
  ATempty->aterm.next = NULL;
  ATempty->aterm.head = NULL;
  ATempty->aterm.tail = NULL;

  hnr = hash_number((ATerm)ATempty, TERM_SIZE_LIST);
  hashtable[hnr& table_mask] = (ATerm)ATempty;

  ATprotectList(&ATempty);

  /*}}}  */

  AT_initMemmgnt();
}

/*}}}  */
/*{{{  void AT_cleanupMemory() */

/**
 * Print hashtable info
 */

void AT_cleanupMemory()
{
  AT_free(hashtable);
  AT_free(terminfo);
  AT_free_protected_blocks();
}

/*}}}  */

/**
 * Allocate a new block of a particular size class
 */

header_type* min_heap_address = (header_type*)(~0);
header_type* max_heap_address = 0;

/*{{{  static void allocate_block(size_t size)  */

static void allocate_block(size_t size)
{
  size_t idx;
  Block* newblock;
  int init = 0;
  TermInfo* ti;

  if (at_freeblocklist != NULL)
  {
    newblock = at_freeblocklist;
    at_freeblocklist = at_freeblocklist->next_by_size;
    at_freeblocklist_size--;
  }
  else
  {
    newblock = (Block*)AT_calloc(1, sizeof(Block));
    if (newblock == NULL)
    {
      std::runtime_error("allocate_block: out of memory!");
    }
    init = 1;

    min_heap_address = MIN(min_heap_address,(newblock->data));
    max_heap_address = MAX(max_heap_address,(newblock->data+BLOCK_SIZE));
    assert(min_heap_address < max_heap_address);

  }

  assert(size >= MIN_TERM_SIZE && size < maxTermSize);

  ti = &terminfo[size];
  ti->at_nrblocks++;

  newblock->end = (newblock->data) + (BLOCK_SIZE - (BLOCK_SIZE % size));

  CLEAR_FROZEN(newblock);
  newblock->size = size;
  newblock->next_by_size = ti->at_blocks[AT_BLOCK];
  ti->at_blocks[AT_BLOCK] = newblock;
  ti->top_at_blocks = newblock->data;
  assert(ti->at_blocks[AT_BLOCK] != NULL);
  assert(((size_t)ti->top_at_blocks % MAX(sizeof(double), sizeof(void*))) == 0);

  /* [pem: Feb 14 02] TODO: fast allocation */
  assert(ti->at_freelist == NULL);

  if (init)
  {
    /* TODO: optimize */
    /* Place the new block in the block_table */
    /*idx = (((MachineWord)newblock) >> (BLOCK_SHIFT+2)) % BLOCK_TABLE_SIZE;*/
    idx = ADDR_TO_BLOCK_IDX(newblock);
    newblock->next_after = block_table[idx].first_after;
    block_table[idx].first_after = newblock;
    idx = (idx+1) % BLOCK_TABLE_SIZE;
    newblock->next_before = block_table[idx].first_before;
    block_table[idx].first_before = newblock;
  }
}

/*}}}  */

/**
 * Allocate a node of a particular size
 */

/*{{{  statistics macros */

#define MCRL2_ALLOCATE_BLOCK_TEXT\
  allocate_block(size);\
  if((total_nodes/maxload)*100 > table_size) {\
    resize_hashtable();\
  }\
  assert(ti->at_blocks[AT_BLOCK] != NULL);\
  at = (ATerm)ti->top_at_blocks;\
  ti->top_at_blocks += size;

#define MCRL2_GC_MINOR_TEXT\
  nb_minor_since_last_major++;\
  AT_collect_minor();

#define MCRL2_GC_MAJOR_TEXT\
  nb_minor_since_last_major = 0;\
  AT_collect();

/*}}}  */

static size_t nb_at_allocate=0;

/*{{{  size_t AT_getAllocatedCount() */

size_t AT_getAllocatedCount()
{
  return nb_at_allocate;
}

/*}}}  */

void AT_growMaxTermSize(size_t neededsize)
{
  TermInfo* newterminfo;
  size_t newsize;

  /* Grow to the minimum of what is needed and twice as the current size,
     to avoid the overhead of repeatedly growing chuncks of memory */
  newsize = (neededsize> 2*maxTermSize?neededsize:2*maxTermSize);

#ifndef NDEBUG
  fprintf(stderr, "Growing administrative structures to accomodate terms of size %lu\n", newsize);
#endif

  newterminfo = (TermInfo*)AT_realloc((void*)terminfo, newsize*sizeof(TermInfo));
  if ((!newterminfo)&&(newsize>neededsize))
  {
    /* Realloc failed again; try with needed size */
    newsize = neededsize;
    newterminfo = (TermInfo*)AT_realloc((void*)terminfo, newsize*sizeof(TermInfo));
  }
  if (!newterminfo)
  {
    std::runtime_error("AT_growMaxTermSize: cannot allocate " + to_string(newsize-maxTermSize) + " extra TermInfo elements.");
  }

  /* Clear new area */
  memset(&newterminfo[maxTermSize], 0, (newsize-maxTermSize)*sizeof(TermInfo));

  terminfo = newterminfo;
  maxTermSize = newsize;
}

/*{{{  ATerm AT_allocate(size_t size)  */

ATerm AT_allocate(size_t size)
{
  ATerm at;
  TermInfo* ti;

  nb_at_allocate++;

  if (size+1 > maxTermSize)
  {
    AT_growMaxTermSize(size+1);
  }

  ti = &terminfo[size];

  while (1)
  {
    if (ti->at_blocks[AT_BLOCK] && ti->top_at_blocks < ti->at_blocks[AT_BLOCK]->end)
    {
      /* the first block is not full: allocate a cell */
      at = (ATerm)ti->top_at_blocks;
      ti->top_at_blocks += size;
      break;

    }
    else if (ti->at_freelist)
    {
      /* the freelist is not empty: allocate a cell */
      at = ti->at_freelist;
      ti->at_freelist = ti->at_freelist->aterm.next;
      assert(ti->at_blocks[AT_BLOCK] != NULL);
      assert(ti->top_at_blocks == ti->at_blocks[AT_BLOCK]->end);
      break;

    }
    else
    {
      /* there is no more memory: run the GC or allocate a block */
      if (ti->at_nrblocks <= gc_min_number_of_blocks)
      {
        MCRL2_ALLOCATE_BLOCK_TEXT;
      }
      else
      {
        size_t reclaimed_memory_during_last_gc =
        /*(ti->nb_reclaimed_blocks_during_last_gc*sizeof(Block)) +*/
        (ti->nb_reclaimed_cells_during_last_gc*SIZE_TO_BYTES(size));
        /* +1 to avoid division by zero */
        size_t reclaimed_memory_ratio_during_last_gc =
        (100*reclaimed_memory_during_last_gc) / (1+ti->nb_live_blocks_before_last_gc*sizeof(Block));
        if (reclaimed_memory_ratio_during_last_gc > good_gc_ratio)
        {
          if (nb_minor_since_last_major < min_nb_minor_since_last_major)
          {
            MCRL2_GC_MINOR_TEXT;
          }
          else
          {
            MCRL2_GC_MAJOR_TEXT;
          }

        }
        else
        {
          size_t nb_allocated_blocks_since_last_gc = ti->at_nrblocks-ti->nb_live_blocks_before_last_gc;
          /* +1 to avoid division by zero */
          size_t allocation_rate =
          (100*nb_allocated_blocks_since_last_gc)/(1+ti->nb_live_blocks_before_last_gc);

          if (allocation_rate < small_allocation_rate_ratio)
          {
            MCRL2_ALLOCATE_BLOCK_TEXT;
          }
          else
          {
            /* +1 to avoid division by zero */
            size_t old_increase_rate =
            (100*(old_bytes_in_young_blocks_since_last_major-old_bytes_in_young_blocks_after_last_major)) /
            (1+old_bytes_in_young_blocks_after_last_major+old_bytes_in_old_blocks_after_last_major);

            if (old_increase_rate < old_increase_rate_ratio)
            {
              MCRL2_GC_MINOR_TEXT;
            }
            else
            {
              MCRL2_GC_MAJOR_TEXT;
            }
          }
        }
      }
    }
  }

  total_nodes++;

  return at;
}

/*}}}  */

/*{{{  void AT_freeTerm(size_t size, ATerm t) */

/**
 * Free a term of a particular size.
 */

void AT_freeTerm(size_t size, ATerm t)
{
  HashNumber hnr = hash_number(t, size);
  ATerm prev = NULL, cur;

  terminfo[size].nb_reclaimed_cells_during_last_gc++;

  /* Remove the node from the hashtable */
  hnr &= table_mask;
  cur = hashtable[hnr];

  do
  {
    if (!cur)
    {
      std::runtime_error("AT_freeTerm: cannot find term " + ATwriteToString(t) + " at " + to_string(t) + " in hashtable at pos " + to_string(hnr) + " header=" + to_string(t->header));
    }
    if (cur == t)
    {
      if (prev)
      {
        prev->aterm.next = cur->aterm.next;
      }
      else
      {
        hashtable[hnr] = cur->aterm.next;
      }
      /* Put the node in the appropriate free list */
      total_nodes--;
      return;
    }
  }
  while (((prev=cur), (cur=cur->aterm.next)));
}

/*}}}  */

/*{{{  ATermAppl ATmakeAppl(AFun sym, ...) */

/**
 * Create a new ATermAppl. The argument count can be found in the symbol.
 */

ATermAppl ATmakeAppl(AFun sym, ...)
{
  ATermAppl cur;
  size_t arity = ATgetArity(sym);
  bool found;
  header_type header;
  HashNumber hnr;
  va_list args;
  ATerm arg;
  ATerm* buffer;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  header = APPL_HEADER(arity > MAX_INLINE_ARITY ?
  MAX_INLINE_ARITY+1 : arity, sym);

  buffer = AT_alloc_protected(arity);

  hnr = START(header);
  va_start(args, sym);
  for (size_t i=0; i<arity; i++)
  {
    arg = va_arg(args, ATerm);
    CHECK_TERM(arg);
    hnr = COMBINE(hnr, HN(arg));
    buffer[i] = arg;
  }
  va_end(args);
  hnr = FINISH(hnr);

  cur = (ATermAppl)hashtable[hnr & table_mask];
  while (cur)
  {
    if (EQUAL_HEADER(cur->header,header))
    {
      found = true;
      for (size_t i=0; i<arity; i++)
      {
        if (!ATisEqual(ATgetArgument(cur, i), buffer[i]))
        {
          found = false;
          break;
        }
      }
      if (found)
      {
        break;
      }
    }
    cur = (ATermAppl)cur->aterm.next;
  }

  if (!cur)
  {
    cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(arity));
    /* Delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = header;
    CHECK_HEADER(cur->header);
    for (size_t i=0; i<arity; i++)
    {
      ATgetArgument(cur, i) = buffer[i];
    }
    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  AT_free_protected(buffer);

  return cur;
}

/*}}}  */
/*{{{  ATermAppl ATmakeAppl0(AFun sym) */

ATermAppl ATmakeAppl0(AFun sym)
{
  ATermAppl cur, prev, *hashspot;
  header_type header = APPL_HEADER(0, sym);
  HashNumber hnr;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  PARK_SYMBOL(sym);

  CHECK_ARITY(ATgetArity(sym), 0);

  hnr = FINISH(START(header));

  prev = NULL;
  hashspot = (ATermAppl*) &(hashtable[hnr & table_mask]);

  cur = *hashspot;
  while (cur)
  {
    if (EQUAL_HEADER(cur->header,header))
    {
      /* Promote current entry to front of hashtable */
      if (prev != NULL)
      {
        prev->aterm.next = cur->aterm.next;
        cur->aterm.next = (ATerm) *hashspot;
        *hashspot = cur;
      }

      return cur;
    }
    prev = cur;
    cur = (ATermAppl)cur->aterm.next;
  }

  cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(0));
  /* Delay masking until after AT_allocate */
  hnr &= table_mask;
  cur->header = header;
  CHECK_HEADER(cur->header);
  cur->aterm.next = hashtable[hnr];
  hashtable[hnr] = (ATerm) cur;

  return cur;
}



/**
 * Create an ATermAppl with zero arguments.
 */

/*}}}  */
/*{{{  ATermAppl ATmakeAppl1(AFun sym, ATerm arg0) */

/**
 * Create an ATermAppl with one argument.
 */

ATermAppl ATmakeAppl1(AFun sym, ATerm arg0)
{
  ATermAppl cur, prev, *hashspot;
  header_type header = APPL_HEADER(1, sym);
  HashNumber hnr;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  PARK_SYMBOL(sym);

  CHECK_TERM(arg0);
  CHECK_ARITY(ATgetArity(sym), 1);

  hnr = START(header);
  hnr = COMBINE(hnr, HN((char*)arg0));
  hnr = FINISH(hnr);

  prev = NULL;
  hashspot = (ATermAppl*) &(hashtable[hnr & table_mask]);

  cur = *hashspot;
  while (cur)
  {
    if (EQUAL_HEADER(header,cur->header)
    && ATgetArgument(cur, 0) == arg0)
    {
      /* Promote current entry to front of hashtable */
      if (prev != NULL)
      {
        prev->aterm.next = cur->aterm.next;
        cur->aterm.next = (ATerm) *hashspot;
        *hashspot = cur;
      }
      CHECK_ARGUMENT(cur, 0);
      return cur;
    }
    prev = cur;
    cur = (ATermAppl)cur->aterm.next;
  }

  cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(1));
  /* Delay masking until after AT_allocate */
  hnr &= table_mask;
  cur->header = header;
  CHECK_HEADER(cur->header);
  ATgetArgument(cur, 0) = arg0;
  CHECK_ARGUMENT(cur, 0);
  cur->aterm.next = hashtable[hnr];
  hashtable[hnr] = (ATerm) cur;

  return cur;
}

/*}}}  */
/*{{{  ATermAppl ATmakeAppl2(AFun sym, arg0, arg1) */

/**
 * Create an ATermAppl with one argument.
 */

ATermAppl ATmakeAppl2(AFun sym, ATerm arg0, ATerm arg1)
{
  ATermAppl cur, prev, *hashspot;
  header_type header = APPL_HEADER(2, sym);
  HashNumber hnr;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  PARK_SYMBOL(sym);

  CHECK_TERM(arg0);
  CHECK_TERM(arg1);
  CHECK_ARITY(ATgetArity(sym), 2);

  hnr = START(header);
  hnr = COMBINE(hnr, HN((char*)arg0));
  hnr = COMBINE(hnr, HN((char*)arg1));
  hnr = FINISH(hnr);

  prev = NULL;
  hashspot = (ATermAppl*) &(hashtable[hnr & table_mask]);

  cur = *hashspot;
  while (cur)
  {
    if (EQUAL_HEADER(cur->header,header)
    && ATgetArgument(cur, 0) == arg0
    && ATgetArgument(cur, 1) == arg1)
    {
      /* Promote current entry to front of hashtable */
      if (prev != NULL)
      {
        prev->aterm.next = cur->aterm.next;
        cur->aterm.next = (ATerm) *hashspot;
        *hashspot = cur;
      }
      CHECK_ARGUMENT(cur, 0);
      CHECK_ARGUMENT(cur, 1);
      return cur;
    }
    prev = cur;
    cur = (ATermAppl)cur->aterm.next;
  }

  cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(2));
  /* Delay masking until after AT_allocate */
  hnr &= table_mask;
  cur->header = header;
  CHECK_HEADER(cur->header);
  ATgetArgument(cur, 0) = arg0;
  ATgetArgument(cur, 1) = arg1;
  CHECK_ARGUMENT(cur, 0);
  CHECK_ARGUMENT(cur, 1);

  cur->aterm.next = hashtable[hnr];
  hashtable[hnr] = (ATerm) cur;

  return cur;
}

/*}}}  */
/*{{{  ATermAppl ATmakeAppl3(AFun sym, ATerm arg0, arg1, arg2) */

/**
 * Create an ATermAppl with one argument.
 */

ATermAppl ATmakeAppl3(AFun sym, ATerm arg0, ATerm arg1, ATerm arg2)
{
  ATermAppl cur;
  header_type header = APPL_HEADER(3, sym);
  HashNumber hnr;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  PARK_SYMBOL(sym);

  CHECK_TERM(arg0);
  CHECK_TERM(arg1);
  CHECK_TERM(arg2);
  CHECK_ARITY(ATgetArity(sym), 3);

  hnr = START(header);
  hnr = COMBINE(hnr, HN((char*)arg0));
  hnr = COMBINE(hnr, HN((char*)arg1));
  hnr = COMBINE(hnr, HN((char*)arg2));
  hnr = FINISH(hnr);

  cur = (ATermAppl) hashtable[hnr & table_mask];
  while (cur && (!EQUAL_HEADER(cur->header,header) ||
  ATgetArgument(cur, 0) != arg0 ||
  ATgetArgument(cur, 1) != arg1 ||
  ATgetArgument(cur, 2) != arg2))
  {
    cur = (ATermAppl) cur->aterm.next;
  }

  if (!cur)
  {
    cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(3));
    /* Delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = header;
    CHECK_HEADER(cur->header);
    ATgetArgument(cur, 0) = arg0;
    ATgetArgument(cur, 1) = arg1;
    ATgetArgument(cur, 2) = arg2;
    CHECK_ARGUMENT(cur, 0);
    CHECK_ARGUMENT(cur, 1);
    CHECK_ARGUMENT(cur, 2);

    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  return cur;
}

/*}}}  */
/*{{{  ATermAppl ATmakeAppl4(AFun sym, ATerm arg0, arg1, arg2, a3) */

/**
 * Create an ATermAppl with four arguments.
 */

ATermAppl ATmakeAppl4(AFun sym, ATerm arg0, ATerm arg1, ATerm arg2, ATerm arg3)
{
  ATermAppl cur;
  header_type header;
  HashNumber hnr;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  PARK_SYMBOL(sym);

  header = APPL_HEADER(4, sym);

  CHECK_TERM(arg0);
  CHECK_TERM(arg1);
  CHECK_TERM(arg2);
  CHECK_TERM(arg3);
  CHECK_ARITY(ATgetArity(sym), 4);

  hnr = START(header);
  hnr = COMBINE(hnr, HN((char*)arg0));
  hnr = COMBINE(hnr, HN((char*)arg1));
  hnr = COMBINE(hnr, HN((char*)arg2));
  hnr = COMBINE(hnr, HN((char*)arg3));
  hnr = FINISH(hnr);

  cur = (ATermAppl)hashtable[hnr & table_mask];
  while (cur && (!EQUAL_HEADER(cur->header,header) ||
  ATgetArgument(cur, 0) != arg0 ||
  ATgetArgument(cur, 1) != arg1 ||
  ATgetArgument(cur, 2) != arg2 ||
  ATgetArgument(cur, 3) != arg3))
  {
    cur = (ATermAppl)cur->aterm.next;
  }

  if (!cur)
  {
    cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(4));
    /* Delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = header;
    CHECK_HEADER(cur->header);
    ATgetArgument(cur, 0) = arg0;
    ATgetArgument(cur, 1) = arg1;
    ATgetArgument(cur, 2) = arg2;
    ATgetArgument(cur, 3) = arg3;
    CHECK_ARGUMENT(cur, 0);
    CHECK_ARGUMENT(cur, 1);
    CHECK_ARGUMENT(cur, 2);
    CHECK_ARGUMENT(cur, 3);

    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  return cur;
}

/*}}}  */
/*{{{  ATermAppl ATmakeAppl5(AFun sym, ATerm arg0, arg1, arg2, a3, a4) */

/**
 * Create an ATermAppl with five arguments.
 */

ATermAppl ATmakeAppl5(AFun sym, ATerm arg0, ATerm arg1, ATerm arg2,
ATerm arg3, ATerm arg4)
{
  ATermAppl cur;
  header_type header = APPL_HEADER(5, sym);
  HashNumber hnr;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  PARK_SYMBOL(sym);

  CHECK_TERM(arg0);
  CHECK_TERM(arg1);
  CHECK_TERM(arg2);
  CHECK_TERM(arg3);
  CHECK_TERM(arg4);

  CHECK_ARITY(ATgetArity(sym), 5);

  hnr = START(header);
  hnr = COMBINE(hnr, HN((char*)arg0));
  hnr = COMBINE(hnr, HN((char*)arg1));
  hnr = COMBINE(hnr, HN((char*)arg2));
  hnr = COMBINE(hnr, HN((char*)arg3));
  hnr = COMBINE(hnr, HN((char*)arg4));
  hnr = FINISH(hnr);

  cur = (ATermAppl) hashtable[hnr & table_mask];
  while (cur && (!EQUAL_HEADER(cur->header,header) ||
  ATgetArgument(cur, 0) != arg0 ||
  ATgetArgument(cur, 1) != arg1 ||
  ATgetArgument(cur, 2) != arg2 ||
  ATgetArgument(cur, 3) != arg3 ||
  ATgetArgument(cur, 4) != arg4))
  {
    cur = (ATermAppl) cur->aterm.next;
  }

  if (!cur)
  {
    cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(5));
    /* Delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = header;
    CHECK_HEADER(cur->header);
    ATgetArgument(cur, 0) = arg0;
    ATgetArgument(cur, 1) = arg1;
    ATgetArgument(cur, 2) = arg2;
    ATgetArgument(cur, 3) = arg3;
    ATgetArgument(cur, 4) = arg4;
    CHECK_ARGUMENT(cur, 0);
    CHECK_ARGUMENT(cur, 1);
    CHECK_ARGUMENT(cur, 2);
    CHECK_ARGUMENT(cur, 3);
    CHECK_ARGUMENT(cur, 4);

    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  return cur;
}

/*}}}  */
/*{{{  ATermAppl ATmakeAppl6(AFun sym, ATerm arg0, arg1, arg2, a3, a4, a5) */

/**
 * Create an ATermAppl with six arguments.
 */

ATermAppl ATmakeAppl6(AFun sym, ATerm arg0, ATerm arg1, ATerm arg2,
ATerm arg3, ATerm arg4, ATerm arg5)
{
  ATermAppl cur;
  header_type header = APPL_HEADER(6, sym);
  HashNumber hnr;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  PARK_SYMBOL(sym);

  CHECK_TERM(arg0);
  CHECK_TERM(arg1);
  CHECK_TERM(arg2);
  CHECK_TERM(arg3);
  CHECK_TERM(arg4);
  CHECK_TERM(arg5);
  CHECK_ARITY(ATgetArity(sym), 6);

  hnr = START(header);
  hnr = COMBINE(hnr, HN((char*)arg0));
  hnr = COMBINE(hnr, HN((char*)arg1));
  hnr = COMBINE(hnr, HN((char*)arg2));
  hnr = COMBINE(hnr, HN((char*)arg3));
  hnr = COMBINE(hnr, HN((char*)arg4));
  hnr = COMBINE(hnr, HN((char*)arg5));
  hnr = FINISH(hnr);

  cur = (ATermAppl) hashtable[hnr & table_mask];
  while (cur && (!EQUAL_HEADER(cur->header,header) ||
  ATgetArgument(cur, 0) != arg0 ||
  ATgetArgument(cur, 1) != arg1 ||
  ATgetArgument(cur, 2) != arg2 ||
  ATgetArgument(cur, 3) != arg3 ||
  ATgetArgument(cur, 4) != arg4 ||
  ATgetArgument(cur, 5) != arg5))
  {
    cur = (ATermAppl) cur->aterm.next;
  }

  if (!cur)
  {
    cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(6));
    /* Delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = header;
    CHECK_HEADER(cur->header);
    ATgetArgument(cur, 0) = arg0;
    ATgetArgument(cur, 1) = arg1;
    ATgetArgument(cur, 2) = arg2;
    ATgetArgument(cur, 3) = arg3;
    ATgetArgument(cur, 4) = arg4;
    ATgetArgument(cur, 5) = arg5;
    CHECK_ARGUMENT(cur, 0);
    CHECK_ARGUMENT(cur, 1);
    CHECK_ARGUMENT(cur, 2);
    CHECK_ARGUMENT(cur, 3);
    CHECK_ARGUMENT(cur, 4);
    CHECK_ARGUMENT(cur, 5);

    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  return cur;
}

/*}}}  */
/*{{{  ATermAppl ATmakeApplList(AFun sym, ATermList args) */

/**
 * Build a function application from a symbol and a list of arguments.
 */

ATermAppl ATmakeApplList(AFun sym, ATermList args)
{
  ATermAppl cur;
  ATermList argptr;
  size_t arity = ATgetArity(sym);
  bool found;
  header_type header = APPL_HEADER(arity > MAX_INLINE_ARITY ?
  MAX_INLINE_ARITY+1 : arity, sym);
  HashNumber hnr;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  PARK_SYMBOL(sym);

  CHECK_TERM((ATerm)args);
  assert(arity == ATgetLength(args));

  argptr = args;
  hnr = START(header);
  for (size_t i=0; i<arity; i++)
  {
    hnr = COMBINE(hnr, HN((char*)ATgetFirst(argptr)));
    argptr = ATgetNext(argptr);
  }
  hnr = FINISH(hnr);

  cur = (ATermAppl) hashtable[hnr & table_mask];
  while (cur)
  {
    if (EQUAL_HEADER(cur->header,header))
    {
      found = true;
      argptr = args;
      for (size_t i=0; i<arity; i++)
      {
        if (!ATisEqual(ATgetArgument(cur, i), ATgetFirst(argptr)))
        {
          found = false;
          break;
        }
        argptr = ATgetNext(argptr);
      }
      if (found)
      {
        break;
      }
    }
    cur = (ATermAppl) cur->aterm.next;
  }

  if (!cur)
  {
    ATprotectList(&args);
    cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(arity));
    ATunprotect((ATerm*)(void*)&args);

    /* Delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = header;
    CHECK_HEADER(cur->header);
    argptr = args;
    for (size_t i=0; i<arity; i++)
    {
      ATgetArgument(cur, i) = ATgetFirst(argptr);
      CHECK_ARGUMENT(cur, i);
      argptr = ATgetNext(argptr);
    }
    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  return cur;
}

/*}}}  */
/*{{{  ATermAppl ATmakeApplArray(AFun sym, ATerm args[]) */

/**
 * Build a function application from a symbol and an array of arguments.
 */

ATermAppl ATmakeApplArray(AFun sym, ATerm args[])
{
  ATermAppl cur;
  size_t arity = ATgetArity(sym);
  bool found;
  HashNumber hnr;
  header_type header = APPL_HEADER(arity > MAX_INLINE_ARITY ?
  MAX_INLINE_ARITY+1 : arity, sym);

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  PARK_SYMBOL(sym);

  hnr = START(header);
  for (size_t i=0; i<arity; i++)
  {
    hnr = COMBINE(hnr, (HashNumber)(char*)args[i]);
  }
  hnr = FINISH(hnr);

  cur = (ATermAppl) hashtable[hnr & table_mask];
  while (cur)
  {
    if (EQUAL_HEADER(cur->header,header))
    {
      found = true;
      for (size_t i=0; i<arity; i++)
      {
        if (!ATisEqual(ATgetArgument(cur, i), args[i]))
        {
          found = false;
          break;
        }
      }
      if (found)
      {
        break;
      }
    }
    cur = (ATermAppl) cur->aterm.next;
  }

  if (!cur)
  {
    ATprotectArray(args, arity);
    cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(arity));
    ATunprotectArray(args);

    /* Delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = header;
    CHECK_HEADER(cur->header);
    for (size_t i=0; i<arity; i++)
    {
      ATgetArgument(cur, i) = args[i];
      CHECK_ARGUMENT(cur, i);
    }
    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  return cur;
}

/*}}}  */

/*{{{  ATermInt ATmakeInt(int val) */

/**
 * Create an ATermInt
 */

ATermInt ATmakeInt(int val)
{
  ATermInt cur;
  /* The following emulates the encoding trick that is also used in the definition
   * of ATermInt. Not using a union here leads to incorrect hashing results.
   */
  union
  {
    int value;
    MachineWord reserved;
  } _val;

  _val.reserved = 0;
  _val.value = val;

  header_type header = INT_HEADER;
  HashNumber hnr;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  hnr = START(header);
  hnr = COMBINE(hnr, HN(_val.reserved));
  hnr = FINISH(hnr);

  cur = (ATermInt) hashtable[hnr & table_mask];
  while (cur && (!EQUAL_HEADER(cur->header,header) || (cur->aterm.value != _val.value)))
  {
    cur = (ATermInt) cur->aterm.next;
  }

  if (!cur)
  {
    cur = (ATermInt)AT_allocate(TERM_SIZE_INT);
    /* Delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = header;
    CHECK_HEADER(cur->header);
    cur->aterm.reserved = _val.reserved;
    cur->aterm.value = _val.value;

    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  assert((hnr & table_mask) == (hash_number((ATerm)cur, TERM_SIZE_INT) & table_mask));

  return cur;
}

/*}}}  */

/*{{{  ATermList ATmakeList1(ATerm el) */

/**
 * Build a list with one element.
 */

ATermList ATmakeList1(ATerm el)
{
  ATermList cur;
  header_type header = LIST_HEADER(1);
  HashNumber hnr;

  AGGRESSIVE_GARBAGE_COLLECT_CHECK;
  CHECK_TERM(el);

  hnr = START(header);
  hnr = COMBINE(hnr, HN((char*)el));
  hnr = COMBINE(hnr, HN((char*)ATempty));
  hnr = FINISH(hnr);

  cur = (ATermList)hashtable[hnr & table_mask];
  while (cur && (!EQUAL_HEADER(cur->header,header)
  || ATgetFirst(cur) != el
  || ATgetNext(cur) != ATempty))
  {
    cur = (ATermList) cur->aterm.next;
  }

  if (!cur)
  {
    cur = (ATermList) AT_allocate(TERM_SIZE_LIST);
    /* Delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = header;
    CHECK_HEADER(cur->header);
    ATgetFirst(cur) = el;
    ATgetNext(cur) = ATempty;
    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  return cur;
}

/*}}}  */
/*{{{  ATermList ATinsert(ATermList tail, ATerm el) */

/**
 * Insert an element at the front of a list.
 */

ATermList ATinsert(ATermList tail, ATerm el)
{
  size_t curLength = GET_LENGTH(tail->header);
  size_t newLength;
  header_type header;
  HashNumber hnr;
  ATermList cur;

  /* If length exceeds the maximum length that can be stored in the header,
     store MAX_LENGTH-1 in the header. ATgetLength will then count the length of the
     list instead of using on the header
  */
  if (curLength >= MAX_LENGTH-1)
  {
    newLength = MAX_LENGTH-1;
  }
  else
  {
    newLength = curLength+1;
  }

  header = LIST_HEADER(newLength);

  CHECK_TERM((ATerm)tail);
  CHECK_TERM(el);

  assert(ATgetType((ATerm)tail) == AT_LIST);

  hnr = START(header);
  hnr = COMBINE(hnr, HN((char*)el));
  hnr = COMBINE(hnr, HN((char*)tail));
  hnr = FINISH(hnr);

  cur = (ATermList) hashtable[hnr & table_mask];
  while (cur && (!EQUAL_HEADER(cur->header,header)
  || ATgetFirst(cur) != el
  || ATgetNext(cur) != tail))
  {
    cur = (ATermList) cur->aterm.next;
  }

  if (!cur)
  {
    cur = (ATermList) AT_allocate(TERM_SIZE_LIST);
    /* Hashtable might be resized, so delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = header;
    CHECK_HEADER(cur->header);
    ATgetFirst(cur) = el;
    ATgetNext(cur) = tail;
    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  return cur;
}

/*}}}  */

/*{{{  ATermAppl ATsetArgument(ATermAppl appl, ATerm arg, size_t n) */

/**
 * Change one argument of an application.
 */

ATermAppl ATsetArgument(ATermAppl appl, ATerm arg, size_t n)
{
  size_t arity;
  AFun sym = ATgetAFun(appl);
  ATermAppl cur;
  bool found;
  HashNumber hnr;

  CHECK_TERM(arg);

  arity = ATgetArity(sym);
  assert(n < arity);

  hnr = START(appl->header);
  for (size_t i=0; i<arity; i++)
  {
    if (i!=n)
    {
      hnr = COMBINE(hnr, (HashNumber)(char*)ATgetArgument(appl, i));
    }
    else
    {
      hnr = COMBINE(hnr, (HashNumber)(char*)arg);
    }
  }

  hnr = FINISH(hnr);

  cur = (ATermAppl) hashtable[hnr & table_mask];
  while (cur)
  {
    if (EQUAL_HEADER(cur->header,appl->header))
    {
      found = true;
      for (size_t i=0; i<arity; i++)
      {
        if (i!=n)
        {
          if (!ATisEqual(ATgetArgument(cur, i), ATgetArgument(appl, i)))
          {
            found = false;
            break;
          }
        }
        else
        {
          if (!ATisEqual(ATgetArgument(cur, i), arg))
          {
            found = false;
            break;
          }
        }
      }
      if (found)
      {
        break;
      }
    }
    cur = (ATermAppl) cur->aterm.next;
  }

  if (!cur)
  {
    cur = (ATermAppl) AT_allocate(TERM_SIZE_APPL(arity));
    /* Delay masking until after AT_allocate */
    hnr &= table_mask;
    cur->header = HIDE_AGE_MARK(appl->header);
    CHECK_HEADER(cur->header);
    for (size_t i=0; i<arity; i++)
    {
      if (i!=n)
      {
        ATgetArgument(cur, i) = ATgetArgument(appl, i);
      }
      else
      {
        ATgetArgument(cur, i) = arg;
      }
    }
    cur->aterm.next = hashtable[hnr];
    hashtable[hnr] = (ATerm) cur;
  }

  return cur;
}

/*}}}  */

/*{{{  ATbool ATisValidTerm(ATerm term) */

/**
 * Determine if a given term is valid.
 */

bool AT_isValidTerm(ATerm term)
{
  Block* cur;
  header_type header;
  bool inblock = false;
  size_t idx = ADDR_TO_BLOCK_IDX(term);
  size_t type;
  ptrdiff_t offset = 0;

  assert(block_table[idx].first_after == block_table[(idx+1)%BLOCK_TABLE_SIZE].first_before);

  for (cur=block_table[idx].first_after; cur; cur=cur->next_after)
  {
    if (cur->size)
    {
      assert(cur->next_before == cur->next_after);
      offset  = ((char*)term) - ((char*)&cur->data);
      if (offset >= 0 && offset < (ptrdiff_t)(BLOCK_SIZE * sizeof(header_type)))
      {
        inblock = true;
        break;
      }
    }
  }

  if (!inblock)
  {
    for (cur=block_table[idx].first_before; cur; cur=cur->next_before)
    {
      if (cur->size)
      {
        assert(cur->next_before == cur->next_after);
        offset  = ((char*)term) - ((char*)&cur->data);
        if (offset >= 0 && offset < (ptrdiff_t)(BLOCK_SIZE * sizeof(header_type)))
        {
          inblock = true;
          break;
        }
      }
    }
  }

  if (!inblock)
  {
    return false;
  }

  /* Check if we point to the start of a term. Pointers inside terms
     are not allowed.
     */
  if (offset % (cur->size*sizeof(header)))
  {
    return false;
  }

  header = term->header;
  type = GET_TYPE(header);

  /* The only possibility left for an invalid term is AT_FREE */
  return (((type == AT_FREE) || (type == AT_SYMBOL)) ? false : true);
}

/*}}}  */

/**
 * Determine if a given term is valid.
 */


/*{{{  ATerm AT_isInsideValidTerm(ATerm term) */

ATerm AT_isInsideValidTerm(ATerm term)
{
  Block* cur;
  TermInfo* ti;
  bool inblock = false;
  size_t idx = ADDR_TO_BLOCK_IDX(term);
  size_t type;

  assert(block_table[idx].first_after == block_table[(idx+1)%BLOCK_TABLE_SIZE].first_before);

  /* Warning: symboles*/

  for (cur=block_table[idx].first_after; cur; cur=cur->next_after)
  {
    header_type* end;
    if (cur->size)
    {
      assert(cur->next_before == cur->next_after);
      ti = &terminfo[cur->size];

      if (cur != ti->at_blocks[AT_BLOCK])
      {
        end = cur->end;
      }
      else
      {
        assert(ti->at_blocks[AT_BLOCK] != NULL);
        end = ti->top_at_blocks;
      }

      if (cur->data <= (header_type*)term && (header_type*)term < end)
      {
        inblock = true;
        break;
      }
    }
  }

  if (!inblock)
  {
    for (cur=block_table[idx].first_before; cur; cur=cur->next_before)
    {
      header_type* end;
      if (cur->size)
      {
        ti = &terminfo[cur->size];
        assert(cur->next_before == cur->next_after);

        if (cur != ti->at_blocks[AT_BLOCK])
        {
          end = cur->end;
        }
        else
        {
          assert(ti->at_blocks[AT_BLOCK] != NULL);
          end = ti->top_at_blocks;
        }

        if (cur->data <= (header_type*)term && (header_type*)term < end)
        {
          inblock = true;
          break;
        }
      }
    }
  }

  if (!inblock)
  {
    return NULL;
  }


  term = (ATerm)(((char*)term)-(((char*)term-((char*)cur->data))%(cur->size*sizeof(header_type))));

  type = GET_TYPE(term->header);

  /* The only possibility left for an invalid term is AT_FREE */
  return (((type == AT_FREE) || (type == AT_SYMBOL)) ? NULL : term);
}

/*}}}  */

/*{{{  size_t AT_inAnyFreeList(ATerm t) */

/**
 * Check if a term is in any free list.
 */

size_t AT_inAnyFreeList(ATerm t)
{
  for (size_t i=MIN_TERM_SIZE; i<maxTermSize; i++)
  {
    ATerm cur = terminfo[i].at_freelist;

    while (cur)
    {
      if (cur == t)
      {
        return i;
      }
      cur = cur->aterm.next;
    }
  }
  return 0;
}

/*}}}  */

} // namespace aterm

