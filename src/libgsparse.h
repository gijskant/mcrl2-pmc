#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include "aterm2.h"

//Global preconditions:
//- the ATerm library has been initialised

ATermAppl gsParseSpecification(FILE *SpecStream, int VbLevel);
/*Pre: SpecStream points to a stream from which can be read
       0 <= VbLevel <= 3
  Post:if SpecStream contains GenSpect specification, this specification is
       parsed and returned in the ATerm format; otherwise NULL is returned and
       an appropriate error message is printed to stderr.
       The printing of intermediate messages is controlled by VbLevel:
       - 0 (silent) : only errors are printed
       - 1 (normal) : errors and warnings are printed
       - 2 (verbose): errors, warnings and short status information is printed
       - 3 (debug)  : many messages are printed to make debugging possible
*/ 

void gsTest(void);
//will be removed in the final version

#ifdef __cplusplus
}
#endif
