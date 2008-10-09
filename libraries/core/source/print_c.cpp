// Author(s): Aad Mathijssen
// Copyright: see the accompanying file COPYING or copy at
// https://svn.win.tue.nl/trac/MCRL2/browser/trunk/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file print_c.cpp

#include <boost/cstdint.hpp>
#include <cstddef>
#include "mcrl2/core/print.h"

#define PRINT_C
#include "mcrl2/core/detail/print_implementation.h"

using namespace mcrl2::core::detail;
using boost::intmax_t;

namespace mcrl2 {
  namespace core {

void PrintPart_C(FILE *OutStream, const ATerm Part, t_pp_format pp_format)
{
  PrintPart__C(OutStream, Part, pp_format);
}

int gsprintf(const char *format, ...)
{
  int             result = 0;
  va_list         args;

  va_start(args, format);
  result = gsvfprintf(stdout, format, args);
  va_end(args);

  return result;
}

int gsfprintf(FILE *stream, const char *format, ...)
{
  int             result = 0;
  va_list         args;

  va_start(args, format);
  result = gsvfprintf(stream, format, args);
  va_end(args);

  return result;
}

int gsvfprintf(FILE *stream, const char *format, va_list args)
{
  const char     *p;
  char           *s;
  char            fmt[32];
  int             result = 0;

  for (p = format; *p; p++)
  {
    if (*p != '%')
    {
      fputc(*p, stream);
      continue;
    }

    s = fmt;
    *s++ = *p++;
    if ( *p == '%' )
    {
      fputc('%',stream);
      continue;
    }
    while (!isalpha((int) *p))  /* parse formats %-20s, etc. */
      *s++ = *p++;
    bool islonglong = false;
    bool islong = false;
    bool islongdouble = false;
    bool isintmax = false;
    bool issize = false;
    bool isptrdiff = false;
    switch ( *p )
    {
      case 'l':
        *s++ = *p++;
        if ( (*p) == 'l' )
        {
          *s++ = *p++;
          islonglong = true;
        } else {
          islong = true;
        }
        break;
      case 'h':
        *s++ = *p++;
        if ( (*p) == 'h' )
          *s++ = *p++;
        break;
      case 'L':
        *s++ = *p++;
        islongdouble = true;
        break;
      case 'j':
        *s++ = *p++;
        isintmax = true;
        break;
      case 'z':
        *s++ = *p++;
        issize = true;
        break;
      case 't':
        *s++ = *p++;
        isptrdiff = true;
        break;
      default:
        break;
    }
    *s++ = *p;
    *s = '\0';

    switch (*p)
    {
      case 'c':
        fprintf(stream, fmt, va_arg(args, int));
        break;

      case 'd':
      case 'i':
      case 'o':
      case 'u':
      case 'x':
      case 'X':
        if ( islong )
          fprintf(stream, fmt, va_arg(args, long int));
        else if ( islonglong )
          fprintf(stream, fmt, va_arg(args, long long int));
        else if ( isintmax )
          fprintf(stream, fmt, va_arg(args, intmax_t));
        else if ( issize )
          fprintf(stream, fmt, va_arg(args, size_t));
        else if ( isptrdiff )
          fprintf(stream, fmt, va_arg(args, ptrdiff_t));
        else
          fprintf(stream, fmt, va_arg(args, int));
        break;

      case 'a':
      case 'A':
      case 'e':
      case 'E':
      case 'f':
      case 'g':
      case 'G':
        if ( islongdouble )
          fprintf(stream, fmt, va_arg(args, long double));
        else
          fprintf(stream, fmt, va_arg(args, double));
        break;

      case 'n':
      case 'p':
        fprintf(stream, fmt, va_arg(args, void *));
        break;

      case 's':
        fprintf(stream, fmt, va_arg(args, char *));
        break;

      /*
       * MCRL2 specifics start here: "%P" to pretty print an ATerm using
       * the advanced method
       */
      case 'P':
        PrintPart_C(stream, va_arg(args, ATerm), ppDefault);
        break;

      /*
       * ATerm specifics start here: "%T" to print an ATerm; "%F" to
       * print an AFun
       *
       * Commented out are:
       * "%I" to print a list; "%N" to print a single ATerm node;
       * "%H" to print the MD5 sum of a ATerm
       */
      case 'T':
        fmt[strlen(fmt)-1] = 't';
        ATfprintf(stream, fmt, va_arg(args, ATerm));
        break;
/*      case 'I':
        fmt[strlen(fmt)-1] = 'l';
        ATfprintf(stream, fmt, va_arg(args, ATermList));
        break;*/
      case 'F':
        fmt[strlen(fmt)-1] = 'y';
        ATfprintf(stream, fmt, va_arg(args, AFun));
        break;
/*      case 'N':
        fmt[strlen(fmt)-1] = 'n';
        ATfprintf(stream, fmt, va_arg(args, ATerm));
        break;
      case 'H':
        fmt[strlen(fmt)-1] = 'h';
        ATfprintf(stream, fmt, va_arg(args, ATerm));
        break;*/

      default:
        fputc(*p, stream);
        break;
    }
  }
  return result;
}

  }
}
