#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdbool.h>
#include <aterm2.h>

//Global preconditions:
//- the ATerm library has been initialised

ATermAppl gsParseSpecification(FILE *SpecStream);
/*Pre: SpecStream points to a stream from which can be read
  Post:if SpecStream contains GenSpect specification, this specification is
       parsed and returned in the ATerm format; otherwise NULL is returned and
       an appropriate error message is printed to stderr.
*/ 

void gsPrintSpecification(FILE *OutStream, const ATermAppl Spec);
/*Pre: OutStream points to a stream to which can be written
       Spec is a specification or a part of it, that adheres to the internal
       format
  Post:A textual representation of the GenSpect specification is written to
       OutStream.
*/

void gsPrintPart(FILE *OutStream, const ATermAppl Part, bool ShowSorts,
  int PrecLevel);
void gsPrintParts(FILE *OutStream, const ATermList Parts, bool ShowSorts,
  int PrecLevel, const char *Terminator, const char *Separator);  
  
void gsTest(void);
//will be removed in the final version

#ifdef __cplusplus
}
#endif
