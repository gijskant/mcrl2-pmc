#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include "aterm2.h"

//Global precondition: the ATerm library has been initialised

ATermAppl gsParse(FILE *formStream);
/*Pre: FormStream is a valid formula stream from which can be read       
  Post:the GenSpect specification in formStream is parsed
  Ret: the parsed formula, if everything went ok
       NULL, otherwise
*/ 

#ifdef __cplusplus
}
#endif

