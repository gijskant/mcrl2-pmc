#ifndef _ATERM_H
#define _ATERM_H

#include "aterm2.h"

#ifdef __cplusplus
extern "C"
{
#endif/* __cplusplus */

typedef struct ProtEntry
{
	struct ProtEntry *next;
	ATerm            *start;
	size_t     size;
} ProtEntry;

#define STRING_MARK 0xFF /* marker for binary strings */
#define LENSPEC 12

extern ATbool low_memory;

extern ProtEntry **at_prot_table;
extern size_t at_prot_table_size;
extern ProtEntry *at_prot_memory;
extern ATermProtFunc *at_prot_functions;
extern size_t at_prot_functions_count;

void AT_markTerm(ATerm t);
void AT_markTerm_young(ATerm t);

void AT_unmarkIfAllMarked(ATerm t);
size_t  AT_calcUniqueAFuns(ATerm t);

#ifdef __cplusplus
}
#endif/* __cplusplus */ 

#endif
