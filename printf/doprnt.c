/* __gmp_doprnt -- printf style formatted output.

   THE FUNCTIONS IN THIS FILE ARE FOR INTERNAL USE ONLY.  THEY'RE ALMOST
   CERTAIN TO BE SUBJECT TO INCOMPATIBLE CHANGES OR DISAPPEAR COMPLETELY IN
   FUTURE GNU MP RELEASES.

Copyright 2001 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. */

#include "config.h"

#if HAVE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <ctype.h>     /* for isdigit */
#include <stddef.h>    /* for ptrdiff_t */
#include <string.h>
#include <stdio.h>     /* for NULL */
#include <stdlib.h>

#if HAVE_INTTYPES_H
#include <inttypes.h>  /* for intmax_t */
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h> /* for quad_t */
#endif

#include "gmp.h"
#include "gmp-impl.h"


/* change this to "#define TRACE(x) x" for diagnostics */
#define TRACE(x) 


/* printf is convenient because it allows various types to be printed in one
   fairly compact call, so having gmp_printf support the standard types as
   well as the gmp ones is important.  This ends up meaning all the standard
   parsing must be duplicated, to get a new routine recognising the gmp
   extras.

   With the currently favoured handling of mpz etc as Z, Q and F type
   markers, it's not possible to use glibc register_printf_function since
   that only accepts new conversion characters, not new types.  If Z was a
   conversion there'd be no way to specify hex, decimal or octal, or
   similarly with F no way to specify fixed point or scientific format.

   It seems wisest to pass float conversions %f, %e and %g of float, double
   and long double over to the standard printf.  It'd be hard to be sure of
   getting the right handling for NaNs, rounding, etc.  Integer conversions
   %d etc and string conversions %s on the other hand could be easily enough
   handled within gmp_doprnt, but if floats are going to libc then it's just
   as easy to send all non-gmp types there.

   "Z" was a type marker for size_t in old glibc, but there seems no need to
   provide access to that now "z" is standard.

   Possibilities:

   "b" might be nice for binary output, and could even be supported for the
   standard C types too if desired.

   POSIX style "%n$" parameter numbering would be possible, but would need
   to be handled completely within gmp_doprnt, since the numbering will be
   all different once the format string it cut into pieces.

   Some options for mpq formatting would be good.  Perhaps a non-zero
   precision field could give a width for the denominator and mean always
   put a "/".  A form "n+p/q" might interesting too, though perhaps that's
   better left to applications.

   Right now there's no way for an application to know whether types like
   intmax_t are supported here.  If configure is doing its job and the same
   compiler is used for gmp as for the application then there shouldn't be
   any problem, but perhaps gmp.h should have some preprocessor symbols to
   say what libgmp can do.  */



/* If a gmp format is the very first thing or there are two gmp formats with
   nothing in between then we'll reach here with this_fmt == last_fmt and we
   can do nothing in that case.  */

#define FLUSH()                                         \
  do {                                                  \
    if (this_fmt == last_fmt)                           \
      {                                                 \
        TRACE (printf ("nothing to flush\n"));          \
        ASSERT (this_ap == last_ap);                    \
      }                                                 \
    else                                                \
      {                                                 \
        ASSERT (*this_fmt == '%');                      \
        *this_fmt = '\0';                               \
        TRACE (printf ("flush \"%s\"\n", last_fmt));    \
        DOPRNT_FORMAT (last_fmt, last_ap);              \
        last_ap = ap;                                   \
      }                                                 \
    last_fmt = fmt+1;                                   \
  } while (0)


/* Parse up the given format string and do the appropriate output using the
   given "funs" routines.  The data parameter is passed through to those
   routines.  */

int
__gmp_doprnt (const struct doprnt_funs_t *funs, void *data,
              const char *orig_fmt, va_list ap)
{
  va_list  this_ap, last_ap;
  size_t   alloc_fmt_size;
  char     *fmt, *alloc_fmt, *last_fmt, *this_fmt;
  int      retval = 0;
  int      type, fchar, *value;
  struct doprnt_params_t param;

  TRACE (printf ("gmp_doprnt \"%s\"\n", orig_fmt));

  /* The format string is chopped up into pieces to be passed to
     funs->format.  Unfortunately that means it has to be copied so each
     piece can be null-terminated.  */
  alloc_fmt_size = strlen (orig_fmt) + 1;
  alloc_fmt = (*__gmp_allocate_func) (alloc_fmt_size);
  fmt = alloc_fmt;
  strcpy (fmt, orig_fmt);

  last_ap = ap;
  last_fmt = fmt;

  for (;;)
    {
      TRACE (printf ("next: \"%s\"\n", fmt));

      fmt = strchr (fmt, '%');
      if (fmt == NULL)
        break;

      this_fmt = fmt;
      this_ap = ap;
      TRACE (printf ("considering\n");
             printf ("  last: \"%s\"\n", last_fmt);
             printf ("  this: \"%s\"\n", this_fmt));

      type = '\0';
      value = &param.width;
  
      param.base = 10;
      param.conv = 0;
      param.expfmt = "e%c%02d";
      param.exptimes4 = 0;
      param.fill = ' ';
      param.justify = DOPRNT_JUSTIFY_RIGHT;
      param.prec = 6;
      param.showbase = DOPRNT_SHOWBASE_NO;
      param.showpoint = 0;
      param.showtrailing = 1;
      param.sign = '\0';
      param.width = 0;

      /* This loop parses a single % sequence.  "break" from the switch
         means continue with this %, "goto next" means the conversion
         character has been seen and a new % should be sought.  */
      for (;;)
        {
          fmt++;
          fchar = *fmt;
          if (fchar == '\0')
            break;

          switch (fchar) {
          case 'a':
            param.base = 16;
            param.expfmt = "p%c%d";
            goto conv_a;
          case 'A':
            param.base = -16;
            param.expfmt = "P%c%d";
          conv_a:
            param.conv = DOPRNT_CONV_SCIENTIFIC;
            param.exptimes4 = 1;
            param.showbase = DOPRNT_SHOWBASE_YES;
            param.showtrailing = 0;
            goto floating;

          case 'c':
            /* Let's assume wchar_t will be promoted to "int" in the call,
               the same as char will be. */
            (void) va_arg (ap, int);
            goto next;

          case 'd':
          case 'i':
          case 'u':
          integer:
            TRACE (printf ("integer, base=%d\n", param.base));
            switch (type) {
            case 'j':
              /* Let's assume uintmax_t is the same size as intmax_t. */
#if HAVE_INTMAX_T
              (void) va_arg (ap, intmax_t);
#else
              ASSERT_FAIL (intmax_t not available);
#endif
              break;
            case 'l':
              (void) va_arg (ap, long);
              break;
            case 'L':
#if HAVE_LONG_LONG
              (void) va_arg (ap, long long);
#else
              ASSERT_FAIL (long long not available);
#endif
              break;
            case 'q':
              /* quad_t and is probably the same as long long, but let's
                 treat it separately just to be sure.  Also let's assume
                 u_quad_t will be the same size as quad_t.  */
#if HAVE_QUAD_T
              (void) va_arg (ap, quad_t);
#else
              ASSERT_FAIL (quad_t not available);
#endif
              break;
            case 'Q':
              {
                mpq_srcptr q;
                int        ret;
                char       *s;
                FLUSH ();
                q = va_arg (ap, mpq_srcptr);
                s = mpq_get_str (NULL, param.base, q);
                ret = __gmp_doprnt_integer (funs, data, &param, s);
                (*__gmp_free_func) (s, strlen(s)+1);
                DOPRNT_ACCUMULATE (ret);
              }
              break;
            case 't':
#if HAVE_PTRDIFF_T
              (void) va_arg (ap, ptrdiff_t);
#else
              ASSERT_FAIL (ptrdiff_t not available);
#endif
              break;
            case 'z':
              (void) va_arg (ap, size_t);
              break;
            case 'Z':
              {
                mpz_srcptr z;
                int        ret;
                char       *s;
                FLUSH ();
                z = va_arg (ap, mpz_srcptr);
                s = mpz_get_str (NULL, param.base, z);
                ret = __gmp_doprnt_integer (funs, data, &param, s);
                (*__gmp_free_func) (s, strlen(s)+1);
                DOPRNT_ACCUMULATE (ret);
              }
              break;
            default:
              /* default is an "int", and this includes h=short and hh=char
                 since they're promoted to int in a function call */
              (void) va_arg (ap, int);
              break;
            }
            goto next;

          case 'E':
            param.base = -10;
            param.expfmt = "E%c%02d";
            /*FALLTHRU*/
          case 'e':
            param.conv = DOPRNT_CONV_SCIENTIFIC;
          floating:
            switch (type) {
            case 'F':
              {
                mpf_srcptr f;
                char       *s;
                mp_exp_t   exp;
                int        ndigits, ret;
                FLUSH ();
                f = va_arg (ap, mpf_srcptr);
                if (fchar == 'a' || fchar == 'A')
                  ndigits = 0;  /* all significant digits */
                else
                  ndigits = __gmp_doprnt_float_digits (&param, f);
                s = mpf_get_str (NULL, &exp, param.base, ndigits, f);
                ASSERT_DOPRNT_NDIGITS (param, ndigits, exp);
                ret = __gmp_doprnt_float (funs, data, &param, s, exp);
                (*__gmp_free_func) (s, strlen(s)+1);
                DOPRNT_ACCUMULATE (ret);
              }
              break;
            case 'L':
#if HAVE_LONG_DOUBLE
              (void) va_arg (ap, long double);
#else
              ASSERT_FAIL (long double not available);
#endif
              break;
            default:
              (void) va_arg (ap, double);
              break;
            }
            goto next;

          case 'f':
            param.conv = DOPRNT_CONV_FIXED;
            goto floating;

          case 'F':
          case 'h':
          case 'j':
          case 'L':
          case 'q':
          case 'Q':
          case 't':
          case 'Z':
            type = fchar;
            break;

          case 'G':
            param.base = -10;
            param.expfmt = "E%c%02d";
            /*FALLTHRU*/
          case 'g':
            param.conv = DOPRNT_CONV_GENERAL;
            param.showtrailing = 0;
            goto floating;

          case 'l':
            if (type == 'l')
              type = 'L';   /* "ll" means "L" */
            else
              type = 'l';
            break;

          case 'm':
            /* glibc strerror(errno), no argument */
            goto next;
            
          case 'n':
            {
              int  *p;
              FLUSH ();
              p = va_arg (ap, int *);
              *p = retval;
            }
            goto next;

          case 'o':
            param.base = 8;
            goto integer;

          case 'p':
          case 's':
            /* "void *" will be good enough for "char *" or "wchar_t *", no
               need for separate code.  */
            (void) va_arg (ap, const void *);
            break;
            
          case 'x':
            param.base = 16;
            goto integer;
          case 'X':
            param.base = -16;
            goto integer;

          case '%':
            goto next;

          case '#':
            param.showbase = DOPRNT_SHOWBASE_NONZERO;
            break;

          case '\'':
            /* glibc digit grouping, just pass it through, no support for it
               on gmp types */
            break;

          case '+':
          case ' ':
            param.sign = fchar;
            break;

          case '-':
            param.justify = DOPRNT_JUSTIFY_LEFT;
            break;
          case '.':
            value = &param.prec;
            break;

          case '*':
            *value = va_arg (ap, int);
            break;

          case '0':
            if (value == &param.width)
              {
                /* in width field, set fill */
                param.fill = '0';

                /* for right justify, put the fill after any minus sign */
                if (param.justify == DOPRNT_JUSTIFY_RIGHT)
                  param.justify = DOPRNT_JUSTIFY_INTERNAL;
              }
            else
              {
                /* in precision field, set value */
                *value = 0;
              }
            break;

          case '1': case '2': case '3': case '4': case '5':
          case '6': case '7': case '8': case '9':
            /* process all digits to form a value */
            {
              int  n = 0;
              do {
                n = n * 10 + (*fmt-'0');
                fmt++;
              } while (isascii (*fmt) && isdigit (*fmt));
              fmt--;
              *value = n;
            }
            break;

          default:
            /* something invalid */
            ASSERT (0);
            goto next;
          }
        }

    next:
      /* Stop parsing the current "%" format, look for a new one. */
      ;
    }

  TRACE (printf ("remainder: \"%s\"\n", last_fmt));
  if (*last_fmt != '\0')
    DOPRNT_FORMAT (last_fmt, last_ap);

  if (funs->final != NULL)
    if ((*funs->final) (data) == -1)
      goto error;

 done:
  (*__gmp_free_func) (alloc_fmt, alloc_fmt_size);
  return retval;

 error:
  retval = -1;
  goto done;
}