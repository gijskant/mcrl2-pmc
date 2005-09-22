#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <aterm2.h>

//Global preconditions:
//- the ATerm library has been initialised
//- gsEnableConstructorFunctions() has been called

void PrintPart_C(FILE *OutStream, const ATerm Part);
/*Pre: OutStream points to a stream to which can be written
       Part is an ATerm containing a part of a mCRL2 specification
  Post:A textual representation of Part is written to OutStream
*/

#ifdef __cplusplus
}
#endif
