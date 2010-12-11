/*{{{  credits */

/* Added by JFG */
/* This file is made by Jan Friso Groote, January 2000. It contains
   implementations of the ATtable and ATindexedSet, which are different
   implementations of hashtables. The implementations are 
   (intended to be) particularily efficient, both in space
   (appr 12 bytes are necessary for a set entry, and 16 for
   a table entry) and time. It is an improvement of the previously
   implemented ATtables which were a straightforward but not
   so efficient (both in time and space).
   Note that hashtables can only grow, until
   they are destroyed. Also resetting the hashtables does 
   not help reducing memory consumption */

/*}}}  */
/*{{{  includes */

#include "aterm2.h"
#include "_aterm.h"
#include "util.h"
#include "memory.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/*}}}  */
/*{{{  defines */

/* ======================================================= */

#define STEP 1   /* The position on which the next hash entry
		    searched */
#define INITIAL_NR_OF_TABLES 8
#define TABLE_SHIFT 14
#define ELEMENTS_PER_TABLE (1L<<TABLE_SHIFT)
#define modELEMENTS_PER_TABLE(n) ((n) & (ELEMENTS_PER_TABLE-1))
#define divELEMENTS_PER_TABLE(n) ((n) >> TABLE_SHIFT)

/*-----------------------------------------------------------*/

/* in the hashtable we use the following constants to
   indicate designated positions */
#define EMPTY (size_t)(-1)
#define DELETED (size_t)(-2)

#define a_prime_number 134217689

/* A very simple hashing function. */

#define hashcode(a,sizeMinus1) (((((size_t) a) >> 2) * a_prime_number ) & sizeMinus1)

/*}}}  */
/*{{{  types */

struct _ATermTable
{
  size_t sizeMinus1;
  size_t nr_entries; /* the number of occupied positions in the hashtable,
		      including the elements that are explicitly marked
		      as deleted */
  size_t nr_deletions;
  unsigned int max_load;
  size_t max_entries;
  size_t *hashtable; 
  size_t nr_tables;
  union _ATerm ***keys;
  size_t nr_free_tables;
  size_t first_free_position;
  size_t **free_table;
  union _ATerm ***values;
};

/*}}}  */

/*{{{  static size_t approximatepowerof2(size_t n) */

/**
 * return smallest 2^m-1 larger or equal than n, where
 * returned size must at least be 127 
 */

static size_t approximatepowerof2(size_t n)
{
  size_t mask = n;

  while(mask >>= 1) {
    n |= mask;
  }

  if (n<127) n=127;
  return n;
}

/*}}}  */
/*{{{  static size_t calc_size_t_max() */
static size_t calc_size_t_max()
{
  size_t try_size_t_max;
  size_t size_t_max;

  try_size_t_max = 1;
  do {
    size_t_max = try_size_t_max;
    try_size_t_max = size_t_max * 2+1;
  } while (try_size_t_max > size_t_max);

  return size_t_max;
}
/*}}}  */
/*{{{  static size_t calculateNewSize(sizeMinus1, nrdel, nrentries) */

static size_t calculateNewSize
            (size_t sizeMinus1, size_t nr_deletions, size_t nr_entries)
{ 

  /* Hack: LONG_MAX (limits.h) is often unreliable, we need to find
   * out the maximum possible value of a size_t dynamically.
   */
  static size_t st_size_t_max = 0;

  /* the resulting length has the form 2^k-1 */

  if (nr_deletions >= nr_entries/2) { 
    return sizeMinus1;
  }

  if (st_size_t_max == 0) 
  {
    st_size_t_max = calc_size_t_max();
  }

  if (sizeMinus1 > st_size_t_max / 2) {
    return st_size_t_max-1;
  }

  return (2*sizeMinus1)+1;
}

/*}}}  */
/*{{{  static ATerm tableGet(ATerm **tableindex, size_t n) */

static ATerm tableGet(ATerm **tableindex, size_t n)
{ 
  return tableindex[divELEMENTS_PER_TABLE(n)][modELEMENTS_PER_TABLE(n)];
}

/*}}}  */
/*{{{  static void insertKeyvalue(set, size_t n, ATerm t, ATerm v) */

static void insertKeyValue(ATermIndexedSet s, 
			   size_t n, ATerm t, ATerm v)
{
  size_t x,y;
  ATerm *keytable, *valuetable;
  size_t nr_tables = s->nr_tables;

  x = divELEMENTS_PER_TABLE(n);
  y = modELEMENTS_PER_TABLE(n);

  if (x>=nr_tables) { 
    s->keys = (ATerm **)AT_realloc(s->keys,
				sizeof(ATerm *)*nr_tables*2);
    if (s->keys==NULL) { 
      ATerror("insertKeyValue: Cannot extend key table\n");
    }
    memset((void*)&s->keys[nr_tables], 0, sizeof(ATerm*)*nr_tables); 

    if (s->values!=NULL) { 
      s->values = (ATerm **)AT_realloc(s->values,
				    sizeof(ATerm *)*nr_tables*2);
      if(s->values == NULL) { 
	ATerror("insertKeyValue: Cannot extend value table\n");
      }
      memset((void*)&s->values[nr_tables], 0, sizeof(ATerm*)*nr_tables); 
    }

    s->nr_tables = nr_tables*2;
  }

  keytable = s->keys[x];
  if (keytable == NULL) { 
    /* create a new key table */
    /* keytable = AT_alloc_protected_minmax(y+1, ELEMENTS_PER_TABLE);  JFG */
    keytable = AT_alloc_protected_minmax(ELEMENTS_PER_TABLE, ELEMENTS_PER_TABLE);
    s->keys[x] = keytable;
    if (keytable == NULL) { 
      ATerror("insertKeyValue: Cannot create new key table\n");
    }

    if (s->values != NULL) { 
      /* valuetable = AT_alloc_protected_minmax(y+1, ELEMENTS_PER_TABLE); */
      valuetable = AT_alloc_protected_minmax(ELEMENTS_PER_TABLE, ELEMENTS_PER_TABLE);
      s->values[x] = valuetable;
      if (valuetable == NULL) { 
	ATerror("insertKeyValue: Cannot create new value table\n");
      }
    }
  }
  else if (n == s->nr_entries-1) {
    keytable = AT_grow_protected((void*)s->keys[x], y+1);
    s->keys[x] = keytable;
    if (keytable == NULL) { 
      ATerror("insertKeyValue: Cannot grow key table\n");
    }

    if (s->values != NULL) { 
      valuetable = AT_grow_protected((void*)s->values[x], y+1);
      s->values[x] = valuetable;
      if (valuetable == NULL) { 
	ATerror("insertKeyValue: Cannot grow value table\n");
      }
    }
  }

  assert(keytable != NULL);

  keytable[y] = t;
  if(s->values != NULL) { 
    s->values[x][y] = v; 
  }
}

/*}}}  */
/*{{{  static size_t hashPut(ATermTable s, ATerm key, size_t n) */

static size_t hashPut(ATermTable s, ATerm key, size_t n)
{  
  size_t c,v;

  /* Find a place to insert key, 
     and find whether key already exists */

  c = hashcode(key, s->sizeMinus1);

  while (1) {
    v = s->hashtable[c];
    if (v == EMPTY || v == DELETED) {
      /* Found an empty spot, insert a new index belonging to key */
      s->hashtable[c] = n;
      return n;
    }

    if (ATisEqual(tableGet(s->keys, v), key)) {
      /* key is already in the set, return position of key */
      return v;
    }
    c = (c + STEP) & s->sizeMinus1;
  } 
}

/*}}}  */
/*{{{  static void hashResizeSet(ATermIndexedSet s) */

static void hashResizeSet(ATermIndexedSet s)
{
  size_t i,newsizeMinus1;
  ATerm t;
  size_t *newhashtable;

  newsizeMinus1 = calculateNewSize(s->sizeMinus1,
				   s->nr_deletions, s->nr_entries);

  newhashtable = (size_t *)AT_malloc(sizeof(size_t) * (newsizeMinus1+1));
  
  if (newhashtable!=NULL) 
  { /* the hashtable has properly been resized */
    AT_free(s->hashtable);
  	
    s->hashtable = newhashtable;
    s->sizeMinus1=newsizeMinus1;
    s->max_entries = ((s->sizeMinus1/100)*s->max_load);
  }
  else
  { /* resizing the hashtable failed and s->hashtable still
       points to the old hashtable. We keep this old hashtable */
#ifndef NDEBUG
    fprintf(stderr,"No memory to increase the size of the indexed set hash table\n");
#endif
    if(s->nr_entries-s->nr_deletions+2>=s->sizeMinus1) 
    {
      ATerror("hashResizeSet: Hashtable of indexed set is full\n");
    }

    /* I do not know whether the bound below is very optimal,
       but we are anyhow in a precarious situation now */
    s->max_entries = (s->nr_entries-s->nr_deletions+s->sizeMinus1)/2;
  }

  /* reset the hashtable. */
  for(i=0; i<=s->sizeMinus1 ; i++ ) { 
    s->hashtable[i] = EMPTY; 
  }

  /* rebuild the hashtable again */
  for (i=0; i<s->nr_entries; i++) { 
    t = tableGet(s->keys, i);
    if (t != NULL) { 
      hashPut(s, t, i);
    }
  }  
  s->nr_deletions = 0;
}

/*}}}  */
/*{{{  static ATermList tableContent(ATerm **tableidx, entries) */

static ATermList tableContent(ATerm **tableindex,size_t nr_entries)
{ 
  size_t i;
  ATerm t;
  ATermList result = ATempty;

  for (i=nr_entries; i>0; i--) {
    t = tableGet(tableindex, i-1);
    if (t != NULL) { 
      result = ATinsert(result, t);
    }
  }
  return result;
}

/*}}}  */

/*{{{  ATermIndexedSet ATindexedSetCreate(init_size, max_load_pct) */

ATermIndexedSet ATindexedSetCreate(size_t initial_size, unsigned int max_load_pct)
{
  size_t i;
  ATermIndexedSet hashset;

  hashset = (ATermIndexedSet)AT_malloc(sizeof(struct _ATermTable));
  if (hashset==NULL) { 
    ATerror("ATindexedSetCreate: cannot allocate new ATermIndexedSet n");
  }
  hashset->sizeMinus1 = approximatepowerof2(initial_size);
  hashset->nr_entries = 0;
  hashset->nr_deletions = 0;
  hashset->max_load = max_load_pct;
  hashset->max_entries = ((hashset->sizeMinus1/100)*hashset->max_load);
  hashset->hashtable=
    (size_t *)AT_malloc(sizeof(size_t)*(1+hashset->sizeMinus1));
  if (hashset->hashtable==NULL) { 
    ATerror("ATindexedSetCreate: cannot allocate ATermIndexedSet "
	    "of %d entries\n", initial_size);
  }
  for(i=0 ; i<=hashset->sizeMinus1 ; i++ ) 
  { 
    hashset->hashtable[i] = EMPTY; 
  }

  hashset->nr_tables = INITIAL_NR_OF_TABLES;
  hashset->keys = (ATerm **)AT_calloc(hashset->nr_tables,
				   sizeof(ATerm *));
  if (hashset->keys == NULL) {
    ATerror("ATindexedSetCreate: cannot creat key index table\n");
  }

  hashset->nr_free_tables = INITIAL_NR_OF_TABLES;
  hashset->first_free_position = 0;
  hashset->free_table=AT_calloc(sizeof(size_t *),
			     hashset->nr_free_tables);
  if (hashset->free_table == NULL) { 
    ATerror("ATindexedSetCreate: cannot allocate table to store deleted elements\n");
  }

  hashset->values = NULL;

  return hashset;
}

/*}}}  */
/*{{{  void ATindexedSetDestroy(ATermIndexedSet hashset) */

void ATindexedSetDestroy(ATermIndexedSet hashset)
{ 
  ATtableDestroy(hashset);
}

/*}}}  */
/*{{{  void ATindexedSetReset(ATermIndexedSet hashset) */

void ATindexedSetReset(ATermIndexedSet hashset)
{ 
  ATtableReset(hashset);
}

/*}}}  */
/*{{{  static size_t keyPut(ATermIndexedSet hashset, key, value, ATbool *isnew)*/

static size_t keyPut(ATermIndexedSet hashset, ATerm key, 
		   ATerm value, ATbool *isnew)
{ 
  size_t n,m;

  if(hashset->first_free_position == 0) { 
    m = hashset->nr_entries; 
    n = hashPut(hashset,key,m);
    if (n != m)  { 
      if (isnew != NULL) {
	*isnew = ATfalse;
      }
      if(value != NULL) { 
	assert(hashset->values!=NULL);
	hashset->values[ divELEMENTS_PER_TABLE(n)]
	  [ modELEMENTS_PER_TABLE(n)] = value;
      }
      return n;
    }
    hashset->nr_entries++;
  } else { 
    m = hashset->free_table
      [divELEMENTS_PER_TABLE(hashset->first_free_position-1)]
      [modELEMENTS_PER_TABLE(hashset->first_free_position-1)]; 
    n = hashPut(hashset, key, m);
    if (n != m) { 
      if (isnew != NULL) {
	*isnew = ATfalse;
      }
      if(value != NULL) { 
	assert(hashset->values != NULL);
	hashset->values[ divELEMENTS_PER_TABLE(n)]
	  [ modELEMENTS_PER_TABLE(n)] = value;
      }
      return n;
    }
    hashset->first_free_position--;
  }

  if (isnew != NULL) {
    *isnew = ATtrue;
  }
  insertKeyValue(hashset, n, key, value);
  if(hashset->nr_entries >= hashset->max_entries) {
    hashResizeSet(hashset); /* repaired by Jan Friso Groote, 25/7/00 */
  }

  return n;
}

/*}}}  */
/*{{{  size_t ATindexedSetPut(ATermIndexedSet hashset, elem, *isnew) */

/**
 * insert entry elem into the hashtable, and deliver
 * an index. If elem is already in the set, deliver 0 
 */

size_t ATindexedSetPut(ATermIndexedSet hashset, ATerm elem, ATbool *isnew)
{
  return keyPut(hashset, elem, NULL, isnew);
}

/*}}}  */
/*{{{  size_t ATindexedSetGetIndex(ATermIndexedSet hashset, ATerm key) */

ssize_t ATindexedSetGetIndex(ATermIndexedSet hashset, ATerm elem)
{ 
  size_t c,start,v;

  start = hashcode(elem, hashset->sizeMinus1);
  c = start;
  do 
  {
    v=hashset->hashtable[c];
    if (v == EMPTY) 
    {
      return -1; /* Not found. */
    }

    if (v != DELETED && ATisEqual(elem,tableGet(hashset->keys, v))) 
    {
      return (ssize_t)v;
    }

    c = (c+STEP) & hashset->sizeMinus1;
  } while (c != start);

  return -1; /* Not found. */
}

/*}}}  */
/*{{{  void ATindexedSetRemove(ATermIndexedSet hashset, ATerm elem) */

void ATindexedSetRemove(ATermIndexedSet hashset, ATerm elem)
{ 
  ATtableRemove(hashset, elem);
}

/*}}}  */
/*{{{  ATermList ATindexedSetElements(ATermIndexedSet hashset) */

ATermList ATindexedSetElements(ATermIndexedSet hashset)
{ 
  return tableContent(hashset->keys, hashset->nr_entries);
}

/*}}}  */
/*{{{  ATerm ATindexedSetGetElem(ATermIndexedSet hashset, size_t index) */

ATerm ATindexedSetGetElem(ATermIndexedSet hashset, size_t index)
{ 
  assert(hashset->nr_entries>index);
  return tableGet(hashset->keys, index);
}

/*}}}  */

/*{{{  ATermTable ATtableCreate(size_t initial_size, int max_load_pct) */

ATermTable ATtableCreate(size_t initial_size, unsigned int max_load_pct)
{ 
  ATermTable hashtable;

  hashtable = (ATermTable)ATindexedSetCreate(initial_size,
					     max_load_pct);

  hashtable->values = (ATerm **)AT_calloc(hashtable->nr_tables,
				       sizeof(ATerm *));

  if (hashtable->values == NULL) { 
    ATerror("ATtableCreate: cannot creat value index table\n");
  }

  return hashtable;
}

/*}}}  */
/*{{{  void ATtableDestroy(ATermTable table) */

void ATtableDestroy(ATermTable table)
{ 
  size_t i;

  AT_free(table->hashtable);
  for(i=0; ((i<table->nr_tables) && (table->keys[i]!=NULL)) ; i++) { 
    AT_free_protected(table->keys[i]);
  }

  AT_free(table->keys);

  if(table->values != NULL) { 
    for(i=0; ((i<table->nr_tables) && 
	      (table->values[i]!=NULL)); i++) { 
      AT_free_protected(table->values[i]);
    }

    AT_free(table->values);
  }

  for(i=0; ((i<table->nr_free_tables) &&
	    (table->free_table[i]!=NULL)) ; i++) { 
    AT_free(table->free_table[i]);
  }

  AT_free(table->free_table);

  AT_free(table);
}

/*}}}  */
/*{{{  void ATtableReset(ATermTable table) */

void ATtableReset(ATermTable table)
{ 
  size_t i;

  table->nr_entries = 0;
  table->nr_deletions = 0;

  for(i=0; i<=table->sizeMinus1 ; i++) { 
    table->hashtable[i] = EMPTY;
  }

  for(i=0; ((i<table->nr_tables) && 
	    (table->keys[i]!=NULL)); i++) {
    /* table->keys[i] = AT_realloc_protected_minmax(table->keys[i], 0, ELEMENTS_PER_TABLE);	  JFG */
    table->keys[i] = AT_realloc_protected_minmax(table->keys[i], ELEMENTS_PER_TABLE, ELEMENTS_PER_TABLE);	 
    if (table->values!=NULL) {
      /* table->values[i] = AT_realloc_protected_minmax(table->values[i], 0, ELEMENTS_PER_TABLE);	  */
      table->values[i] = AT_realloc_protected_minmax(table->values[i], ELEMENTS_PER_TABLE, ELEMENTS_PER_TABLE);	 
    }
  }

/*
  for(i=0; ((i<table->nr_free_tables) &&
	    (table->free_table[i]!=NULL)); i++) { 
    memset(table->free_table[i], 0, ELEMENTS_PER_TABLE);
  }
*/
  table->first_free_position = 0;
}

/*}}}  */
/*{{{  void ATtablePut(ATermTable table, ATerm key, ATerm value) */

void ATtablePut(ATermTable table, ATerm key, ATerm value)
{ 
  /* insert entry key into the hashtable, and deliver
     an index. If key is already in the set, deliver 0 */

  ATbool isnew;

  keyPut(table, key, value, &isnew);
}

/*}}}  */
/*{{{  ATerm ATtableGet(ATermTable table, ATerm key) */

ATerm ATtableGet(ATermTable table, ATerm key)
{ 
  size_t v;

  v = ATindexedSetGetIndex(table, key);
  if(v==ATERM_NON_EXISTING_POSITION) 
  {
    return NULL;
  }
  return tableGet(table->values, v);
}

/*}}}  */
/*{{{  void ATtableRemove(ATermTable table, ATerm key) */

void ATtableRemove(ATermTable table, ATerm key)
{ 
  size_t start,c,v,x,y;
  size_t *ltable;

  start = hashcode(key,table->sizeMinus1);
  c = start;
  while(1) {
    v = table->hashtable[c];
    if(v == EMPTY) {
      return;
    }
    if(v != DELETED && ATisEqual(key, tableGet(table->keys, v))) {
      break;
    }

    c = (c + STEP) & table->sizeMinus1;
    if (c == start) {
      return;
    }
  }

  table->hashtable[c] = DELETED;

  insertKeyValue(table, v, NULL, NULL);

  x=divELEMENTS_PER_TABLE(table->first_free_position);
  if (x>=table->nr_free_tables) { 
    table->free_table = (size_t **)AT_realloc(table->free_table,
					 sizeof(size_t)*table->nr_free_tables*2);
    if (table->free_table==NULL) {
      ATerror("ATtableRemove: Cannot allocate memory for free table index\n");
    }

    memset((void*)&table->free_table[table->nr_free_tables], 0, table->nr_free_tables);

    table->nr_free_tables = table->nr_free_tables*2;
  }

  ltable = table->free_table[x];
  if (ltable == NULL) {
    /* create a new key table */
    ltable = AT_malloc(sizeof(size_t)*ELEMENTS_PER_TABLE);
    table->free_table[x] = ltable;
    if (ltable == NULL) { 
      ATerror("ATtableRemove: Cannot create new free table\n");
    }
  }

  y = modELEMENTS_PER_TABLE(table->first_free_position);
  ltable[y] = v;
  table->first_free_position++;
  table->nr_deletions++; 
}

/*}}}  */
/*{{{  ATermList ATtableKeys(ATermTable table) */

ATermList ATtableKeys(ATermTable table)
{ 
  return tableContent(table->keys, table->nr_entries);
}

/*}}}  */
/*{{{  ATermList  ATtableValues(ATermTable table) */

ATermList  ATtableValues(ATermTable table)
{ 
  return tableContent(table->values, table->nr_entries);
}

/*}}}  */
