/* $Id: libgsrewrite.h,v 1.3 2005/04/08 09:49:58 muck Exp $ */
#ifdef __cplusplus
extern "C" {
#endif

#include <aterm2.h>

#define GS_REWR_INNER	0
#define GS_REWR_INNER2	1
#define GS_REWR_INNER3	2
#define GS_REWR_INNERC	3
#define GS_REWR_INNERC2	4
#define GS_REWR_JITTY	5

void gsRewriteInit(ATermAppl Eqns, int strat);
void gsRewriteFinalize();

void gsRewriteAddEqn(ATermAppl Eqn);
void gsRewriteRemoveEqn(ATermAppl Eqn);

ATermAppl gsRewriteTerm(ATermAppl Term);
ATermList gsRewriteTerms(ATermList Terms);

ATerm gsToRewriteFormat(ATermAppl Term);
ATermAppl gsFromRewriteFormat(ATerm Term);
ATerm gsRewriteInternal(ATerm Term);
ATermList gsRewriteInternals(ATermList Terms);

#ifdef __cplusplus
}
#endif
