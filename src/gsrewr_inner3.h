/* $Id: gsrewr_inner3.h,v 1.1 2005/04/08 09:49:58 muck Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include "aterm2.h"
#include "gslowlevel.h"
#include "gsfunc.h"

void rewrite_init_inner3();

void rewrite_add_inner3(ATermAppl eqn);
void rewrite_remove_inner3(ATermAppl eqn);

ATerm rewrite_inner3(ATerm Term, int *b);
