/* Test gmp_scanf and related functions.

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


/* Usage: t-scanf [-s]

   -s  Check the data against the system scanf, where possible.  This is
       only an option since we don't want to fail if the system scanf is
       faulty or strange.

   There's some fairly unattractive repetition between check_z, check_q and
   check_f, but enough differences to make a common loop or a set of macros
   seem like too much trouble.  */


#include "config.h"

#if HAVE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if HAVE_UNISTD_H
#include <unistd.h>  /* for unlink */
#endif

#include "gmp.h"
#include "gmp-impl.h"
#include "tests.h"

/* SunOS 4 stdio.h doesn't provide a prototype for this */
#if ! HAVE_DECL_SSCANF
int sscanf _PROTO ((const char *input, const char *fmt, ...));
#endif


#define TEMPFILE  "t-scanf.tmp"

int   option_libc_scanf = 0;

typedef int (*fun1_t) _PROTO ((const char *, const char *, void *));
typedef int (*fun2_t) _PROTO ((const char *, const char *, void *, void *));


int
libc_scanf_convert (char *fmt)
{
  char  *p = fmt;

  if (! option_libc_scanf)
    return 0;

  for ( ; *fmt != '\0'; fmt++)
    {
      switch (*fmt) {
      case 'F':
      case 'Q':
      case 'Z':
        /* transmute */
        *p++ = 'l';
        break;
      default:
        *p++ = *fmt;
        break;
      }
    }
  *p = '\0';
  return 1;
}


long  got_ftell;
int   fromstring_next_c;

int
#if HAVE_STDARG
fromstring_gmp_fscanf (const char *input, const char *fmt, ...)
#else
fromstring_gmp_fscanf (va_alist)
     va_dcl
#endif
{
  va_list  ap;
  FILE     *fp;
  int      ret;
#if HAVE_STDARG
  va_start (ap, fmt);
#else
  const char    *input;
  const char    *fmt;
  va_start (ap);
  input = va_arg (ap, const char *);
  fmt = va_arg (ap, const char *);
#endif

  ASSERT_ALWAYS ((fp = fopen (TEMPFILE, "w+")) != NULL);
  ASSERT_ALWAYS (fputs (input, fp) != EOF);
  ASSERT_ALWAYS (fflush (fp) == 0);
  rewind (fp);

  ret = gmp_vfscanf (fp, fmt, ap);
  got_ftell = ftell (fp);
  ASSERT_ALWAYS (got_ftell != -1L);

  fromstring_next_c = getc (fp);

  ASSERT_ALWAYS (fclose (fp) == 0);
  return ret;
}

int
fromstring_fscanf1 (const char *input, const char *fmt, void *a1)
{
  FILE  *fp;
  int   ret;

  ASSERT_ALWAYS ((fp = fopen (TEMPFILE, "w+")) != NULL);
  ASSERT_ALWAYS (fputs (input, fp) != EOF);
  ASSERT_ALWAYS (fflush (fp) == 0);
  rewind (fp);

  ret = fscanf (fp, fmt, a1);
  got_ftell = ftell (fp);
  ASSERT_ALWAYS (got_ftell != -1L);

  fromstring_next_c = getc (fp);

  ASSERT_ALWAYS (fclose (fp) == 0);
  return ret;
}

int
fromstring_fscanf2 (const char *input, const char *fmt, void *a1, void *a2)
{
  FILE  *fp;
  int   ret;

  ASSERT_ALWAYS ((fp = fopen (TEMPFILE, "w+")) != NULL);
  ASSERT_ALWAYS (fputs (input, fp) != EOF);
  ASSERT_ALWAYS (fflush (fp) == 0);
  rewind (fp);

  ret = fscanf (fp, fmt, a1, a2);
  got_ftell = ftell (fp);
  ASSERT_ALWAYS (got_ftell != -1L);

  fromstring_next_c = getc (fp);

  ASSERT_ALWAYS (fclose (fp) == 0);
  return ret;
}


/* whether the format string consists entirely of ignored fields */
int
fmt_allignore (const char *fmt)
{
  int  saw_star = 1;
  for ( ; *fmt != '\0'; fmt++)
    {
      switch (*fmt) {
      case '%':
        if (! saw_star)
          return 0;
        saw_star = 0;
        break;
      case '*':
        saw_star = 1;
        break;
      }
    }
  return 1;
}

void
check_z (void)
{
  static const struct {
    const char  *fmt;
    const char  *input;
    const char  *want;
    int         want_ret;
    long        want_ftell;
    int         want_upto;
    int         not_glibc;

  } data[] = {

    { "%Zd",    "0",    "0", 1, -1, -1 },
    { "%Zd",    "1",    "1", 1, -1, -1 },
    { "%Zd",  "123",  "123", 1, -1, -1 },
    { "%Zd",   "+0",    "0", 1, -1, -1 },
    { "%Zd",   "+1",    "1", 1, -1, -1 },
    { "%Zd", "+123",  "123", 1, -1, -1 },
    { "%Zd",   "-0",    "0", 1, -1, -1 },
    { "%Zd",   "-1",   "-1", 1, -1, -1 },
    { "%Zd", "-123", "-123", 1, -1, -1 },

    { "%Zo",    "0",    "0", 1, -1, -1 },
    { "%Zo",  "173",  "123", 1, -1, -1 },
    { "%Zo",   "+0",    "0", 1, -1, -1 },
    { "%Zo", "+173",  "123", 1, -1, -1 },
    { "%Zo",   "-0",    "0", 1, -1, -1 },
    { "%Zo", "-173", "-123", 1, -1, -1 },

    { "%Zx",    "0",    "0", 1, -1, -1 },
    { "%Zx",   "7b",  "123", 1, -1, -1 },
    { "%Zx",   "7b",  "123", 1, -1, -1 },
    { "%Zx",   "+0",    "0", 1, -1, -1 },
    { "%Zx",  "+7b",  "123", 1, -1, -1 },
    { "%Zx",  "+7b",  "123", 1, -1, -1 },
    { "%Zx",   "-0",   "-0", 1, -1, -1 },
    { "%Zx",  "-7b", "-123", 1, -1, -1 },
    { "%Zx",  "-7b", "-123", 1, -1, -1 },
    { "%ZX",    "0",    "0", 1, -1, -1 },
    { "%ZX",   "7b",  "123", 1, -1, -1 },
    { "%ZX",   "7b",  "123", 1, -1, -1 },
    { "%ZX",   "+0",    "0", 1, -1, -1 },
    { "%ZX",  "+7b",  "123", 1, -1, -1 },
    { "%ZX",  "+7b",  "123", 1, -1, -1 },
    { "%ZX",   "-0",   "-0", 1, -1, -1 },
    { "%ZX",  "-7b", "-123", 1, -1, -1 },
    { "%ZX",  "-7b", "-123", 1, -1, -1 },
    { "%Zx",    "0",    "0", 1, -1, -1 },
    { "%Zx",   "7B",  "123", 1, -1, -1 },
    { "%Zx",   "7B",  "123", 1, -1, -1 },
    { "%Zx",   "+0",    "0", 1, -1, -1 },
    { "%Zx",  "+7B",  "123", 1, -1, -1 },
    { "%Zx",  "+7B",  "123", 1, -1, -1 },
    { "%Zx",   "-0",   "-0", 1, -1, -1 },
    { "%Zx",  "-7B", "-123", 1, -1, -1 },
    { "%Zx",  "-7B", "-123", 1, -1, -1 },
    { "%ZX",    "0",    "0", 1, -1, -1 },
    { "%ZX",   "7B",  "123", 1, -1, -1 },
    { "%ZX",   "7B",  "123", 1, -1, -1 },
    { "%ZX",   "+0",    "0", 1, -1, -1 },
    { "%ZX",  "+7B",  "123", 1, -1, -1 },
    { "%ZX",  "+7B",  "123", 1, -1, -1 },
    { "%ZX",   "-0",   "-0", 1, -1, -1 },
    { "%ZX",  "-7B", "-123", 1, -1, -1 },
    { "%ZX",  "-7B", "-123", 1, -1, -1 },

    { "%Zi",    "0",    "0", 1, -1, -1 },
    { "%Zi",    "1",    "1", 1, -1, -1 },
    { "%Zi",  "123",  "123", 1, -1, -1 },
    { "%Zi",   "+0",    "0", 1, -1, -1 },
    { "%Zi",   "+1",    "1", 1, -1, -1 },
    { "%Zi", "+123",  "123", 1, -1, -1 },
    { "%Zi",   "-0",    "0", 1, -1, -1 },
    { "%Zi",   "-1",   "-1", 1, -1, -1 },
    { "%Zi", "-123", "-123", 1, -1, -1 },

    { "%Zi",    "00",    "0", 1, -1, -1 },
    { "%Zi",  "0173",  "123", 1, -1, -1 },
    { "%Zi",   "+00",    "0", 1, -1, -1 },
    { "%Zi", "+0173",  "123", 1, -1, -1 },
    { "%Zi",   "-00",    "0", 1, -1, -1 },
    { "%Zi", "-0173", "-123", 1, -1, -1 },

    { "%Zi",    "0x0",    "0", 1, -1, -1 },
    { "%Zi",   "0x7b",  "123", 1, -1, -1 },
    { "%Zi",   "0x7b",  "123", 1, -1, -1 },
    { "%Zi",   "+0x0",    "0", 1, -1, -1 },
    { "%Zi",  "+0x7b",  "123", 1, -1, -1 },
    { "%Zi",  "+0x7b",  "123", 1, -1, -1 },
    { "%Zi",   "-0x0",   "-0", 1, -1, -1 },
    { "%Zi",  "-0x7b", "-123", 1, -1, -1 },
    { "%Zi",  "-0x7b", "-123", 1, -1, -1 },
    { "%Zi",    "0X0",    "0", 1, -1, -1 },
    { "%Zi",   "0X7b",  "123", 1, -1, -1 },
    { "%Zi",   "0X7b",  "123", 1, -1, -1 },
    { "%Zi",   "+0X0",    "0", 1, -1, -1 },
    { "%Zi",  "+0X7b",  "123", 1, -1, -1 },
    { "%Zi",  "+0X7b",  "123", 1, -1, -1 },
    { "%Zi",   "-0X0",   "-0", 1, -1, -1 },
    { "%Zi",  "-0X7b", "-123", 1, -1, -1 },
    { "%Zi",  "-0X7b", "-123", 1, -1, -1 },
    { "%Zi",    "0x0",    "0", 1, -1, -1 },
    { "%Zi",   "0x7B",  "123", 1, -1, -1 },
    { "%Zi",   "0x7B",  "123", 1, -1, -1 },
    { "%Zi",   "+0x0",    "0", 1, -1, -1 },
    { "%Zi",  "+0x7B",  "123", 1, -1, -1 },
    { "%Zi",  "+0x7B",  "123", 1, -1, -1 },
    { "%Zi",   "-0x0",   "-0", 1, -1, -1 },
    { "%Zi",  "-0x7B", "-123", 1, -1, -1 },
    { "%Zi",  "-0x7B", "-123", 1, -1, -1 },
    { "%Zi",    "0X0",    "0", 1, -1, -1 },
    { "%Zi",   "0X7B",  "123", 1, -1, -1 },
    { "%Zi",   "0X7B",  "123", 1, -1, -1 },
    { "%Zi",   "+0X0",    "0", 1, -1, -1 },
    { "%Zi",  "+0X7B",  "123", 1, -1, -1 },
    { "%Zi",  "+0X7B",  "123", 1, -1, -1 },
    { "%Zi",   "-0X0",   "-0", 1, -1, -1 },
    { "%Zi",  "-0X7B", "-123", 1, -1, -1 },
    { "%Zi",  "-0X7B", "-123", 1, -1, -1 },

    { "%Zd",    " 0",    "0", 1, -1, -1 },
    { "%Zd",   "  0",    "0", 1, -1, -1 },
    { "%Zd",  "   0",    "0", 1, -1, -1 },
    { "%Zd",   "\t0",    "0", 1, -1, -1 },
    { "%Zd", "\t\t0",    "0", 1, -1, -1 },

    { "hello%Zd",      "hello0",       "0", 1, -1, -1 },
    { "hello%Zd",      "hello 0",      "0", 1, -1, -1 },
    { "hello%Zd",      "hello \t0",    "0", 1, -1, -1 },
    { "hello%Zdworld", "hello 0world", "0", 1, -1, -1 },

    { "hello%*Zd",      "hello0",       "-999", 0, -1, -1 },
    { "hello%*Zd",      "hello 0",      "-999", 0, -1, -1 },
    { "hello%*Zd",      "hello \t0",    "-999", 0, -1, -1 },
    { "hello%*Zdworld", "hello 0world", "-999", 0, -1, -1 },

    { "%Zd",    "",     "-999", -1, -1, -555 },
    { "%Zd",    " ",    "-999", -1, -1, -555 },
    { " %Zd",   "",     "-999", -1, -1, -555 },
    { "xyz%Zd", "",     "-999", -1, -1, -555 },

    { "%*Zd",    "",     "-999", -1, -1, -555 },
    { " %*Zd",   "",     "-999", -1, -1, -555 },
    { "xyz%*Zd", "",     "-999", -1, -1, -555 },

    { "%Zd",    "xyz",  "0",     0, 0, -555 },

    /* match something, but invalid */
    { "%Zd",    "-",    "-999",  0, 1, -555 },
    { "%Zd",    "+",    "-999",  0, 1, -555 },
    { "xyz%Zd", "xyz-", "-999",  0, 4, -555 },
    { "xyz%Zd", "xyz+", "-999",  0, 4, -555 },

    { "%1Zi",  "1234", "1",    1, 1, 1 },
    { "%2Zi",  "1234", "12",   1, 2, 2 },
    { "%3Zi",  "1234", "123",  1, 3, 3 },
    { "%4Zi",  "1234", "1234", 1, 4, 4 },
    { "%5Zi",  "1234", "1234", 1, 4, 4 },
    { "%6Zi",  "1234", "1234", 1, 4, 4 },

    { "%1Zi",  "01234", "0",     1, 1, 1 },
    { "%2Zi",  "01234", "01",    1, 2, 2 },
    { "%3Zi",  "01234", "012",   1, 3, 3 },
    { "%4Zi",  "01234", "0123",  1, 4, 4 },
    { "%5Zi",  "01234", "01234", 1, 5, 5 },
    { "%6Zi",  "01234", "01234", 1, 5, 5 },
    { "%7Zi",  "01234", "01234", 1, 5, 5 },

    { "%1Zi",  "0x1234", "0",      1, 1, 1 },
    { "%2Zi",  "0x1234", "0",      1, 2, 2 },
    { "%3Zi",  "0x1234", "0x1",    1, 3, 3 },
    { "%4Zi",  "0x1234", "0x12",   1, 4, 4 },
    { "%5Zi",  "0x1234", "0x123",  1, 5, 5 },
    { "%6Zi",  "0x1234", "0x1234", 1, 6, 6 },
    { "%7Zi",  "0x1234", "0x1234", 1, 6, 6 },
    { "%8Zi",  "0x1234", "0x1234", 1, 6, 6 },

    { "%%xyz%Zd",  "%xyz123",  "123", 1, -1, -1 },
    { "12%%34%Zd", "12%34567", "567", 1, -1, -1 },
    { "%%%%%Zd",   "%%123",    "123", 1, -1, -1 },

    /* various subtle EOF cases */
    { "x",       "",    "-999", EOF, 0, -555 },
    { " x",      "",    "-999", EOF, 0, -555 },
    { "xyz",     "",    "-999", EOF, 0, -555 },
    { " ",       "",    "-999",   0, 0,    0 },
    { " ",       " ",   "-999",   0, 1,    1 },
    { "%*Zd%Zd", "",    "-999", EOF, 0, -555 },
    { "%*Zd%Zd", "123", "-999", EOF, 3, -555 },
    { "x",       "x",   "-999",   0, 1,    1 },
    { "xyz",     "x",   "-999", EOF, 1, -555 },
    { "xyz",     "xy",  "-999", EOF, 2, -555 },
    { "xyz",     "xyz", "-999",   0, 3,    3 },
    { "%Zn",     "",    "0",      0, 0,    0 },
    { " %Zn",    "",    "0",      0, 0,    0 },
    { " x%Zn",   "",    "-999", EOF, 0, -555 },
    { "xyz%Zn",  "",    "-999", EOF, 0, -555 },
    { " x%Zn",   "",    "-999", EOF, 0, -555 },
    { " %Zn x",  " ",   "-999", EOF, 1, -555 },

    /* these seem to tickle a bug in glibc 2.2.4 */
    { " x",      " ",   "-999", EOF, 1, -555, 1 },
    { " xyz",    " ",   "-999", EOF, 1, -555, 1 },
    { " x%Zn",   " ",   "-999", EOF, 1, -555, 1 },
  };

  int         i, j, ignore;
  int         got_ret, want_ret, got_upto, want_upto;
  mpz_t       got, want;
  long        got_l, want_ftell;
  int         error = 0;
  fun1_t      fun1;
  fun2_t      fun2;
  const char  *name;
  char        fmt[128];

  mpz_init (got);
  mpz_init (want);

  for (i = 0; i < numberof (data); i++)
    {
      mpz_set_str_or_abort (want, data[i].want, 0);

      ASSERT_ALWAYS (strlen (data[i].fmt) + 2 < sizeof (fmt));
      strcpy (fmt, data[i].fmt);
      strcat (fmt, "%n");

      ignore = fmt_allignore (fmt);

      for (j = 0; j <= 3; j++)
        {
          want_ret = data[i].want_ret;

          want_ftell = data[i].want_ftell;
          if (want_ftell == -1)
            want_ftell = strlen (data[i].input);

          want_upto = data[i].want_upto;
          if (want_upto == -1)
            want_upto = strlen (data[i].input);

          switch (j) {
          case 0:
            name = "gmp_sscanf";
            fun1 = (fun1_t) gmp_sscanf;
            fun2 = (fun2_t) gmp_sscanf;
            break;
          case 1:
            name = "gmp_fscanf";
            fun1 = (fun1_t) fromstring_gmp_fscanf;
            fun2 = (fun2_t) fromstring_gmp_fscanf;
            break;
          case 2:
#ifdef __GLIBC__
            if (data[i].not_glibc)
              continue;
#endif
            if (! libc_scanf_convert (fmt))
              continue;
            name = "standard sscanf";
            fun1 = (fun1_t) sscanf;
            fun2 = (fun2_t) sscanf;
            break;
          case 3:
#ifdef __GLIBC__
            if (data[i].not_glibc)
              continue;
#endif
            if (! libc_scanf_convert (fmt))
              continue;
            name = "standard fscanf";
            fun1 = fromstring_fscanf1;
            fun2 = fromstring_fscanf2;
            break;
          default:
            ASSERT_ALWAYS (0);
            break;
          }

          got_upto = -555;
          got_ftell = -1L;

          switch (j) {
          case 0:
          case 1:
            mpz_set_si (got, -999L);
            if (ignore)
              got_ret = (*fun1) (data[i].input, fmt, &got_upto);
            else
              got_ret = (*fun2) (data[i].input, fmt, got, &got_upto);
            break;
          case 2:
          case 3:
            got_l = -999L;
            if (ignore)
              got_ret = (*fun1) (data[i].input, fmt, &got_upto);
            else
              got_ret = (*fun2) (data[i].input, fmt, &got_l, &got_upto);
            mpz_set_si (got, got_l);
            break;
          default:
            ASSERT_ALWAYS (0);
            break;
          }

          MPZ_CHECK_FORMAT (got);

          if (got_ret != want_ret)
            {
              printf ("%s wrong return value\n", name);
              error = 1;
            }
          if (want_ret == 1 && mpz_cmp (want, got) != 0)
            {
              printf ("%s wrong result\n", name);
              error = 1;
            }
          if (got_upto != want_upto)
            {
              printf ("%s wrong upto\n", name);
              error = 1;
            }
          if (got_ftell != -1 && want_ftell != -1 && got_ftell != want_ftell)
            {
              printf ("%s wrong ftell\n", name);
              error = 1;
            }
          if (error)
            {
              printf    ("  fmt   \"%s\"\n", data[i].fmt);
              printf    ("  input \"%s\"\n", data[i].input);
              printf    ("  ignore %d\n", ignore);
              printf    ("  ret   want=%d\n", want_ret);
              printf    ("        got =%d\n", got_ret);
              mpz_trace ("  value want", want);
              mpz_trace ("        got ", got);
              printf    ("  upto  want =%d\n", want_upto);
              printf    ("        got  =%d\n", got_upto);
              if (got_ftell != -1)
                {
                  printf    ("  ftell want =%ld\n", want_ftell);
                  printf    ("        got  =%ld\n", got_ftell);
                }
              abort ();
            }
        }
    }

  mpz_clear (got);
  mpz_clear (want);
}

void
check_q (void)
{
  static const struct {
    const char  *fmt;
    const char  *input;
    const char  *want;
    int         ret;
    long        ftell;

  } data[] = {

    { "%Qd",    "0",    "0", 1, -1 },
    { "%Qd",    "1",    "1", 1, -1 },
    { "%Qd",  "123",  "123", 1, -1 },
    { "%Qd",   "+0",    "0", 1, -1 },
    { "%Qd",   "+1",    "1", 1, -1 },
    { "%Qd", "+123",  "123", 1, -1 },
    { "%Qd",   "-0",    "0", 1, -1 },
    { "%Qd",   "-1",   "-1", 1, -1 },
    { "%Qd", "-123", "-123", 1, -1 },

    { "%Qo",    "0",    "0", 1, -1 },
    { "%Qo",  "173",  "123", 1, -1 },
    { "%Qo",   "+0",    "0", 1, -1 },
    { "%Qo", "+173",  "123", 1, -1 },
    { "%Qo",   "-0",    "0", 1, -1 },
    { "%Qo", "-173", "-123", 1, -1 },

    { "%Qx",    "0",    "0", 1, -1 },
    { "%Qx",   "7b",  "123", 1, -1 },
    { "%Qx",   "7b",  "123", 1, -1 },
    { "%Qx",   "+0",    "0", 1, -1 },
    { "%Qx",  "+7b",  "123", 1, -1 },
    { "%Qx",  "+7b",  "123", 1, -1 },
    { "%Qx",   "-0",   "-0", 1, -1 },
    { "%Qx",  "-7b", "-123", 1, -1 },
    { "%Qx",  "-7b", "-123", 1, -1 },
    { "%QX",    "0",    "0", 1, -1 },
    { "%QX",   "7b",  "123", 1, -1 },
    { "%QX",   "7b",  "123", 1, -1 },
    { "%QX",   "+0",    "0", 1, -1 },
    { "%QX",  "+7b",  "123", 1, -1 },
    { "%QX",  "+7b",  "123", 1, -1 },
    { "%QX",   "-0",   "-0", 1, -1 },
    { "%QX",  "-7b", "-123", 1, -1 },
    { "%QX",  "-7b", "-123", 1, -1 },
    { "%Qx",    "0",    "0", 1, -1 },
    { "%Qx",   "7B",  "123", 1, -1 },
    { "%Qx",   "7B",  "123", 1, -1 },
    { "%Qx",   "+0",    "0", 1, -1 },
    { "%Qx",  "+7B",  "123", 1, -1 },
    { "%Qx",  "+7B",  "123", 1, -1 },
    { "%Qx",   "-0",   "-0", 1, -1 },
    { "%Qx",  "-7B", "-123", 1, -1 },
    { "%Qx",  "-7B", "-123", 1, -1 },
    { "%QX",    "0",    "0", 1, -1 },
    { "%QX",   "7B",  "123", 1, -1 },
    { "%QX",   "7B",  "123", 1, -1 },
    { "%QX",   "+0",    "0", 1, -1 },
    { "%QX",  "+7B",  "123", 1, -1 },
    { "%QX",  "+7B",  "123", 1, -1 },
    { "%QX",   "-0",   "-0", 1, -1 },
    { "%QX",  "-7B", "-123", 1, -1 },
    { "%QX",  "-7B", "-123", 1, -1 },

    { "%Qi",    "0",    "0", 1, -1 },
    { "%Qi",    "1",    "1", 1, -1 },
    { "%Qi",  "123",  "123", 1, -1 },
    { "%Qi",   "+0",    "0", 1, -1 },
    { "%Qi",   "+1",    "1", 1, -1 },
    { "%Qi", "+123",  "123", 1, -1 },
    { "%Qi",   "-0",    "0", 1, -1 },
    { "%Qi",   "-1",   "-1", 1, -1 },
    { "%Qi", "-123", "-123", 1, -1 },

    { "%Qi",    "00",    "0", 1, -1 },
    { "%Qi",  "0173",  "123", 1, -1 },
    { "%Qi",   "+00",    "0", 1, -1 },
    { "%Qi", "+0173",  "123", 1, -1 },
    { "%Qi",   "-00",    "0", 1, -1 },
    { "%Qi", "-0173", "-123", 1, -1 },

    { "%Qi",    "0x0",    "0", 1, -1 },
    { "%Qi",   "0x7b",  "123", 1, -1 },
    { "%Qi",   "0x7b",  "123", 1, -1 },
    { "%Qi",   "+0x0",    "0", 1, -1 },
    { "%Qi",  "+0x7b",  "123", 1, -1 },
    { "%Qi",  "+0x7b",  "123", 1, -1 },
    { "%Qi",   "-0x0",   "-0", 1, -1 },
    { "%Qi",  "-0x7b", "-123", 1, -1 },
    { "%Qi",  "-0x7b", "-123", 1, -1 },
    { "%Qi",    "0X0",    "0", 1, -1 },
    { "%Qi",   "0X7b",  "123", 1, -1 },
    { "%Qi",   "0X7b",  "123", 1, -1 },
    { "%Qi",   "+0X0",    "0", 1, -1 },
    { "%Qi",  "+0X7b",  "123", 1, -1 },
    { "%Qi",  "+0X7b",  "123", 1, -1 },
    { "%Qi",   "-0X0",   "-0", 1, -1 },
    { "%Qi",  "-0X7b", "-123", 1, -1 },
    { "%Qi",  "-0X7b", "-123", 1, -1 },
    { "%Qi",    "0x0",    "0", 1, -1 },
    { "%Qi",   "0x7B",  "123", 1, -1 },
    { "%Qi",   "0x7B",  "123", 1, -1 },
    { "%Qi",   "+0x0",    "0", 1, -1 },
    { "%Qi",  "+0x7B",  "123", 1, -1 },
    { "%Qi",  "+0x7B",  "123", 1, -1 },
    { "%Qi",   "-0x0",   "-0", 1, -1 },
    { "%Qi",  "-0x7B", "-123", 1, -1 },
    { "%Qi",  "-0x7B", "-123", 1, -1 },
    { "%Qi",    "0X0",    "0", 1, -1 },
    { "%Qi",   "0X7B",  "123", 1, -1 },
    { "%Qi",   "0X7B",  "123", 1, -1 },
    { "%Qi",   "+0X0",    "0", 1, -1 },
    { "%Qi",  "+0X7B",  "123", 1, -1 },
    { "%Qi",  "+0X7B",  "123", 1, -1 },
    { "%Qi",   "-0X0",   "-0", 1, -1 },
    { "%Qi",  "-0X7B", "-123", 1, -1 },
    { "%Qi",  "-0X7B", "-123", 1, -1 },

    { "%Qd",    " 0",    "0", 1, -1 },
    { "%Qd",   "  0",    "0", 1, -1 },
    { "%Qd",  "   0",    "0", 1, -1 },
    { "%Qd",   "\t0",    "0", 1, -1 },
    { "%Qd", "\t\t0",    "0", 1, -1 },

    { "%Qd",  "3/2",   "3/2", 1, -1 },
    { "%Qd", "+3/2",   "3/2", 1, -1 },
    { "%Qd", "-3/2",  "-3/2", 1, -1 },

    { "%Qx",  "f/10", "15/16", 1, -1 },
    { "%Qx",  "F/10", "15/16", 1, -1 },
    { "%QX",  "f/10", "15/16", 1, -1 },
    { "%QX",  "F/10", "15/16", 1, -1 },

    { "%Qo",  "20/21",  "16/17", 1, -1 },
    { "%Qo", "-20/21", "-16/17", 1, -1 },

    { "%Qi",    "10/11",  "10/11", 1, -1 },
    { "%Qi",   "+10/11",  "10/11", 1, -1 },
    { "%Qi",   "-10/11", "-10/11", 1, -1 },
    { "%Qi",   "010/11",   "8/11", 1, -1 },
    { "%Qi",  "+010/11",   "8/11", 1, -1 },
    { "%Qi",  "-010/11",  "-8/11", 1, -1 },
    { "%Qi",  "0x10/11",  "16/11", 1, -1 },
    { "%Qi", "+0x10/11",  "16/11", 1, -1 },
    { "%Qi", "-0x10/11", "-16/11", 1, -1 },

    { "%Qi",    "10/011",  "10/9", 1, -1 },
    { "%Qi",   "+10/011",  "10/9", 1, -1 },
    { "%Qi",   "-10/011", "-10/9", 1, -1 },
    { "%Qi",   "010/011",   "8/9", 1, -1 },
    { "%Qi",  "+010/011",   "8/9", 1, -1 },
    { "%Qi",  "-010/011",  "-8/9", 1, -1 },
    { "%Qi",  "0x10/011",  "16/9", 1, -1 },
    { "%Qi", "+0x10/011",  "16/9", 1, -1 },
    { "%Qi", "-0x10/011", "-16/9", 1, -1 },

    { "%Qi",    "10/0x11",  "10/17", 1, -1 },
    { "%Qi",   "+10/0x11",  "10/17", 1, -1 },
    { "%Qi",   "-10/0x11", "-10/17", 1, -1 },
    { "%Qi",   "010/0x11",   "8/17", 1, -1 },
    { "%Qi",  "+010/0x11",   "8/17", 1, -1 },
    { "%Qi",  "-010/0x11",  "-8/17", 1, -1 },
    { "%Qi",  "0x10/0x11",  "16/17", 1, -1 },
    { "%Qi", "+0x10/0x11",  "16/17", 1, -1 },
    { "%Qi", "-0x10/0x11", "-16/17", 1, -1 },

    { "hello%Qd",      "hello0",         "0", 1, -1 },
    { "hello%Qd",      "hello 0",        "0", 1, -1 },
    { "hello%Qd",      "hello \t0",      "0", 1, -1 },
    { "hello%Qdworld", "hello 0world",   "0", 1, -1 },
    { "hello%Qd",      "hello3/2",     "3/2", 1, -1 },

    { "hello%*Qd",      "hello0",        "-999/121", 0, -1 },
    { "hello%*Qd",      "hello 0",       "-999/121", 0, -1 },
    { "hello%*Qd",      "hello \t0",     "-999/121", 0, -1 },
    { "hello%*Qdworld", "hello 0world",  "-999/121", 0, -1 },
    { "hello%*Qdworld", "hello3/2world", "-999/121", 0, -1 },

    { "%Qd",    "",     "-999/121", -1, -1 },
    { "%Qd",   " ",     "-999/121", -1, -1 },
    { " %Qd",   "",     "-999/121", -1, -1 },
    { "xyz%Qd", "",     "-999/121", -1, -1 },

    { "%*Qd",    "",     "-999/121", -1, -1 },
    { " %*Qd",   "",     "-999/121", -1, -1 },
    { "xyz%*Qd", "",     "-999/121", -1, -1 },

    /* match something, but invalid */
    { "%Qd",    "-",     "-999/121",  0, 1 },
    { "%Qd",    "+",     "-999/121",  0, 1 },
    { "%Qd",    "/-",    "-999/121",  0, 1 },
    { "%Qd",    "/+",    "-999/121",  0, 1 },
    { "%Qd",    "-/",    "-999/121",  0, 1 },
    { "%Qd",    "+/",    "-999/121",  0, 1 },
    { "%Qd",    "-/-",   "-999/121",  0, 1 },
    { "%Qd",    "-/+",   "-999/121",  0, 1 },
    { "%Qd",    "+/+",   "-999/121",  0, 1 },
    { "%Qd",    "/123",  "-999/121",  0, 1 },
    { "%Qd",    "-/123", "-999/121",  0, 1 },
    { "%Qd",    "+/123", "-999/121",  0, 1 },
    { "%Qd",    "123/",  "-999/121",  0, 1 },
    { "%Qd",    "123/-", "-999/121",  0, 1 },
    { "%Qd",    "123/+", "-999/121",  0, 1 },
    { "xyz%Qd", "xyz-",  "-999/121",  0, 4 },
    { "xyz%Qd", "xyz+",  "-999/121",  0, 4 },

    { "%1Qi",  "12/57", "1",        1, 1 },
    { "%2Qi",  "12/57", "12",       1, 2 },
    { "%3Qi",  "12/57", "-999/121", 0, -1 },
    { "%4Qi",  "12/57", "12/5",     1, 4 },
    { "%5Qi",  "12/57", "12/57",    1, 5 },
    { "%6Qi",  "12/57", "12/57",    1, 5 },
    { "%7Qi",  "12/57", "12/57",    1, 5 },

    { "%1Qi",  "012/057", "0",        1, 1 },
    { "%2Qi",  "012/057", "01",       1, 2 },
    { "%3Qi",  "012/057", "012",      1, 3 },
    { "%4Qi",  "012/057", "-999/121", 0, -1 },
    { "%5Qi",  "012/057", "012/0",    1, 5 },
    { "%6Qi",  "012/057", "012/5",    1, 6 },
    { "%7Qi",  "012/057", "012/057",  1, 7 },
    { "%8Qi",  "012/057", "012/057",  1, 7 },
    { "%9Qi",  "012/057", "012/057",  1, 7 },

    { "%1Qi",  "0x12/0x57", "0",         1, 1 },
    { "%2Qi",  "0x12/0x57", "0",         1, 2 },
    { "%3Qi",  "0x12/0x57", "0x1",       1, 3 },
    { "%4Qi",  "0x12/0x57", "0x12",      1, 4 },
    { "%5Qi",  "0x12/0x57", "-999/121",  0, -1 },
    { "%6Qi",  "0x12/0x57", "0x12/0",    1, 6 },
    { "%7Qi",  "0x12/0x57", "0x12/0",    1, 7 },
    { "%8Qi",  "0x12/0x57", "0x12/0x5",  1, 8 },
    { "%9Qi",  "0x12/0x57", "0x12/0x57", 1, 9 },
    { "%10Qi", "0x12/0x57", "0x12/0x57", 1, 9 },
    { "%11Qi", "0x12/0x57", "0x12/0x57", 1, 9 },

    { "%Qd",  "xyz", "0", 0, 0 },
  };

  int         i, j, ignore, got_ret, want_ret, got_upto, want_upto;
  mpq_t       got, want;
  long        got_l, want_ftell;
  int         error = 0;
  fun1_t      fun1;
  fun2_t      fun2;
  const char  *name;
  char        fmt[128];

  mpq_init (got);
  mpq_init (want);

  for (i = 0; i < numberof (data); i++)
    {
      mpq_set_str_or_abort (want, data[i].want, 0);

      ASSERT_ALWAYS (strlen (data[i].fmt) + 2 < sizeof (fmt));
      strcpy (fmt, data[i].fmt);
      strcat (fmt, "%n");

      ignore = (strchr (fmt, '*') != NULL);

      for (j = 0; j <= 3; j++)
        {
          want_ret = data[i].ret;

          want_ftell = data[i].ftell;
          if (want_ftell == -1)
            want_ftell = strlen (data[i].input);
          want_upto = want_ftell;

          if (want_ret == -1 || (want_ret == 0 && ! ignore))
            {
              want_ftell = -1;
              want_upto = -555;
            }

          switch (j) {
          case 0:
            name = "gmp_sscanf";
            fun1 = (fun1_t) gmp_sscanf;
            fun2 = (fun2_t) gmp_sscanf;
            break;
          case 1:
            name = "gmp_fscanf";
            fun1 = (fun1_t) fromstring_gmp_fscanf;
            fun2 = (fun2_t) fromstring_gmp_fscanf;
            break;
          case 2:
            if (strchr (data[i].input, '/') != NULL)
              continue;
            if (! libc_scanf_convert (fmt))
              continue;
            name = "standard sscanf";
            fun1 = (fun1_t) sscanf;
            fun2 = (fun2_t) sscanf;
            break;
          case 3:
            if (strchr (data[i].input, '/') != NULL)
              continue;
            if (! libc_scanf_convert (fmt))
              continue;
            name = "standard fscanf";
            fun1 = fromstring_fscanf1;
            fun2 = fromstring_fscanf2;
            break;
          default:
            ASSERT_ALWAYS (0);
            break;
          }

          got_upto = -555;
          got_ftell = -1;

          switch (j) {
          case 0:
          case 1:
            mpq_set_si (got, -999L, 121L);
            if (ignore)
              got_ret = (*fun1) (data[i].input, fmt, &got_upto);
            else
              got_ret = (*fun2) (data[i].input, fmt, got, &got_upto);
            break;
          case 2:
          case 3:
            got_l = -999L;
            if (ignore)
              got_ret = (*fun1) (data[i].input, fmt, &got_upto);
            else
              got_ret = (*fun2) (data[i].input, fmt, &got_l, &got_upto);
            mpq_set_si (got, got_l, (got_l == -999L ? 121L : 1L));
            break;
          default:
            ASSERT_ALWAYS (0);
            break;
          }

          MPZ_CHECK_FORMAT (mpq_numref (got));
          MPZ_CHECK_FORMAT (mpq_denref (got));

          if (got_ret != want_ret)
            {
              printf ("%s wrong return value\n", name);
              error = 1;
            }
          /* use direct mpz compares, since some of the test data is
             non-canonical and can trip ASSERTs in mpq_equal */
          if (want_ret == 1
              && ! (mpz_cmp (mpq_numref(want), mpq_numref(got)) == 0
                    && mpz_cmp (mpq_denref(want), mpq_denref(got)) == 0))
            {
              printf ("%s wrong result\n", name);
              error = 1;
            }
          if (got_upto != want_upto)
            {
              printf ("%s wrong upto\n", name);
              error = 1;
            }
          if (got_ftell != -1 && want_ftell != -1 && got_ftell != want_ftell)
            {
              printf ("%s wrong ftell\n", name);
              error = 1;
            }
          if (error)
            {
              printf    ("  fmt   \"%s\"\n", data[i].fmt);
              printf    ("  input \"%s\"\n", data[i].input);
              printf    ("  ret   want=%d\n", want_ret);
              printf    ("        got =%d\n", got_ret);
              mpq_trace ("  value want", want);
              mpq_trace ("        got ", got);
              printf    ("  upto  want=%d\n", want_upto);
              printf    ("        got =%d\n", got_upto);
              if (got_ftell != -1)
                {
                  printf    ("  ftell want =%ld\n", want_ftell);
                  printf    ("        got  =%ld\n", got_ftell);
                }
              abort ();
            }
        }
    }

  mpq_clear (got);
  mpq_clear (want);
}

void
check_f (void)
{
  static const struct {
    const char  *fmt;
    const char  *input;
    const char  *want;
    int         ret;
    long        ftell;

  } data[] = {

    { "%Ff",    "0",    "0", 1, -1 },
    { "%Fe",    "0",    "0", 1, -1 },
    { "%FE",    "0",    "0", 1, -1 },
    { "%Fg",    "0",    "0", 1, -1 },
    { "%FG",    "0",    "0", 1, -1 },

    { "%Ff",  "123",    "123", 1, -1 },
    { "%Ff", "+123",    "123", 1, -1 },
    { "%Ff", "-123",   "-123", 1, -1 },
    { "%Ff",  "123.",   "123", 1, -1 },
    { "%Ff", "+123.",   "123", 1, -1 },
    { "%Ff", "-123.",  "-123", 1, -1 },
    { "%Ff",  "123.0",  "123", 1, -1 },
    { "%Ff", "+123.0",  "123", 1, -1 },
    { "%Ff", "-123.0", "-123", 1, -1 },
    { "%Ff",  "123e",   "123", 1, -1 },
    { "%Ff", "-123e",  "-123", 1, -1 },
    { "%Ff",  "123e-",  "123", 1, -1 },
    { "%Ff", "-123e-", "-123", 1, -1 },
    { "%Ff",  "123e+",  "123", 1, -1 },
    { "%Ff", "-123e+", "-123", 1, -1 },

    { "%Ff",  "123.456e3",   "123456", 1, -1 },
    { "%Ff", "-123.456e3",  "-123456", 1, -1 },
    { "%Ff",  "123.456e+3",  "123456", 1, -1 },
    { "%Ff", "-123.456e+3", "-123456", 1, -1 },
    { "%Ff",  "123000e-3",      "123", 1, -1 },
    { "%Ff", "-123000e-3",     "-123", 1, -1 },
    { "%Ff",  "123000.e-3",     "123", 1, -1 },
    { "%Ff", "-123000.e-3",    "-123", 1, -1 },

    { "%Ff",  "123.456E3",   "123456", 1, -1 },
    { "%Ff", "-123.456E3",  "-123456", 1, -1 },
    { "%Ff",  "123.456E+3",  "123456", 1, -1 },
    { "%Ff", "-123.456E+3", "-123456", 1, -1 },
    { "%Ff",  "123000E-3",      "123", 1, -1 },
    { "%Ff", "-123000E-3",     "-123", 1, -1 },
    { "%Ff",  "123000.E-3",     "123", 1, -1 },
    { "%Ff", "-123000.E-3",    "-123", 1, -1 },

    { "%Ff",  ".456e3",   "456", 1, -1 },
    { "%Ff", "-.456e3",  "-456", 1, -1 },
    { "%Ff",  ".456e+3",  "456", 1, -1 },
    { "%Ff", "-.456e+3", "-456", 1, -1 },

    { "%Ff",    " 0",    "0", 1, -1 },
    { "%Ff",   "  0",    "0", 1, -1 },
    { "%Ff",  "   0",    "0", 1, -1 },
    { "%Ff",   "\t0",    "0", 1, -1 },
    { "%Ff", "\t\t0",    "0", 1, -1 },

    { "hello%Fg",      "hello0",       "0",   1, -1 },
    { "hello%Fg",      "hello 0",      "0",   1, -1 },
    { "hello%Fg",      "hello \t0",    "0",   1, -1 },
    { "hello%Fgworld", "hello 0world", "0",   1, -1 },
    { "hello%Fg",      "hello3.0",     "3.0", 1, -1 },

    { "hello%*Fg",      "hello0",        "-999", 0, -1 },
    { "hello%*Fg",      "hello 0",       "-999", 0, -1 },
    { "hello%*Fg",      "hello \t0",     "-999", 0, -1 },
    { "hello%*Fgworld", "hello 0world",  "-999", 0, -1 },
    { "hello%*Fgworld", "hello3.0world", "-999", 0, -1 },

    { "%Ff",     "",   "-999", -1, -1 },
    { "%Ff",    " ",   "-999", -1, -1 },
    { "%Ff",   "\t",   "-999", -1, -1 },
    { "%Ff",  " \t",   "-999", -1, -1 },
    { " %Ff",    "",   "-999", -1, -1 },
    { "xyz%Ff",  "",   "-999", -1, -1 },

    { "%*Ff",    "",   "-999", -1, -1 },
    { " %*Ff",   "",   "-999", -1, -1 },
    { "xyz%*Ff", "",   "-999", -1, -1 },

    { "%Ff",    "xyz", "0", 0 },

    /* various non-empty but invalid */
    { "%Ff",    "-",    "-999",  0 },
    { "%Ff",    "+",    "-999",  0 },
    { "xyz%Ff", "xyz-", "-999",  0 },
    { "xyz%Ff", "xyz+", "-999",  0 },
    { "xyz%Ff", "-.",   "-999",  0 },
    { "xyz%Ff", "+.",   "-999",  0 },
    { "xyz%Ff", ".e",   "-999",  0 },
    { "xyz%Ff", "-.e",  "-999",  0 },
    { "xyz%Ff", "+.e",  "-999",  0 },
    { "xyz%Ff", ".E",   "-999",  0 },
    { "xyz%Ff", "-.E",  "-999",  0 },
    { "xyz%Ff", "+.E",  "-999",  0 },
    { "xyz%Ff", ".e123",   "-999",  0 },
    { "xyz%Ff", "-.e123",  "-999",  0 },
    { "xyz%Ff", "+.e123",  "-999",  0 },

  };

  int         i, j, ignore, got_ret, want_ret, got_upto, want_upto;
  mpf_t       got, want;
  double      got_d;
  long        want_ftell;
  int         error = 0;
  fun1_t      fun1;
  fun2_t      fun2;
  const char  *name;
  char        fmt[128];

  mpf_init (got);
  mpf_init (want);

  for (i = 0; i < numberof (data); i++)
    {
      mpf_set_str_or_abort (want, data[i].want, 0);

      ASSERT_ALWAYS (strlen (data[i].fmt) + 2 < sizeof (fmt));
      strcpy (fmt, data[i].fmt);
      strcat (fmt, "%n");

      ignore = (strchr (fmt, '*') != NULL);

      for (j = 0; j <= 3; j++)
        {
          want_ret = data[i].ret;

          want_ftell = data[i].ftell;
          if (want_ftell == -1)
            want_ftell = strlen (data[i].input);
          want_upto = want_ftell;

          if (want_ret == -1 || (want_ret == 0 && ! ignore))
            {
              want_ftell = -1;
              want_upto = -555;
            }

          switch (j) {
          case 0:
            name = "gmp_sscanf";
            fun1 = (fun1_t) gmp_sscanf;
            fun2 = (fun2_t) gmp_sscanf;
            break;
          case 1:
            name = "gmp_fscanf";
            fun1 = (fun1_t) fromstring_gmp_fscanf;
            fun2 = (fun2_t) fromstring_gmp_fscanf;
            break;
          case 2:
            if (! libc_scanf_convert (fmt))
              continue;
            name = "standard sscanf";
            fun1 = (fun1_t) sscanf;
            fun2 = (fun2_t) sscanf;
            break;
          case 3:
            if (! libc_scanf_convert (fmt))
              continue;
            name = "standard fscanf";
            fun1 = fromstring_fscanf1;
            fun2 = fromstring_fscanf2;
            break;
          default:
            ASSERT_ALWAYS (0);
            break;
          }

          got_upto = -555;
          got_ftell = -1;

          switch (j) {
          case 0:
          case 1:
            mpf_set_si (got, -999L);
            if (ignore)
              got_ret = (*fun1) (data[i].input, fmt, &got_upto);
            else
              got_ret = (*fun2) (data[i].input, fmt, got, &got_upto);
            break;
          case 2:
          case 3:
            got_d = -999L;
            if (ignore)
              got_ret = (*fun1) (data[i].input, fmt, &got_upto);
            else
              got_ret = (*fun2) (data[i].input, fmt, &got_d, &got_upto);
            mpf_set_d (got, got_d);
            break;
          default:
            ASSERT_ALWAYS (0);
            break;
          }

          MPF_CHECK_FORMAT (got);

          if (got_ret != want_ret)
            {
              printf ("%s wrong return value\n", name);
              error = 1;
            }
          if (want_ret == 1 && mpf_cmp (want, got) != 0)
            {
              printf ("%s wrong result\n", name);
              error = 1;
            }
          if (got_upto != want_upto)
            {
              printf ("%s wrong upto\n", name);
              error = 1;
            }
          if (got_ftell != -1 && want_ftell != -1 && got_ftell != want_ftell)
            {
              printf ("%s wrong ftell\n", name);
              error = 1;
            }
          if (error)
            {
              printf    ("  fmt   \"%s\"\n", data[i].fmt);
              printf    ("  input \"%s\"\n", data[i].input);
              printf    ("  ret   want=%d\n", want_ret);
              printf    ("        got =%d\n", got_ret);
              mpf_trace ("  value want", want);
              mpf_trace ("        got ", got);
              printf    ("  upto  want=%d\n", want_upto);
              printf    ("        got =%d\n", got_upto);
              if (got_ftell != -1)
                {
                  printf    ("  ftell want =%ld\n", want_ftell);
                  printf    ("        got  =%ld\n", got_ftell);
                }
              abort ();
            }
        }
    }

  mpf_clear (got);
  mpf_clear (want);
}


void
check_misc (void)
{
  {
    int  a=9, b=8, c=7, n=66;
    mpz_t  z;
    mpz_init (z);
    ASSERT_ALWAYS (gmp_sscanf ("1 2 3 4", "%d %d %d %Zd%n",
                               &a, &b, &c, z, &n) == 4);
    ASSERT_ALWAYS (a == 1);
    ASSERT_ALWAYS (b == 2);
    ASSERT_ALWAYS (c == 3);
    ASSERT_ALWAYS (n == 7);
    ASSERT_ALWAYS (mpz_cmp_ui (z, 4L) == 0);
    mpz_clear (z);
  }
  {
    int  a=9, b=8, c=7, n=66;
    mpz_t  z;
    mpz_init (z);
    ASSERT_ALWAYS (fromstring_gmp_fscanf ("1 2 3 4", "%d %d %d %Zd%n",
                                          &a, &b, &c, z, &n) == 4);
    ASSERT_ALWAYS (a == 1);
    ASSERT_ALWAYS (b == 2);
    ASSERT_ALWAYS (c == 3);
    ASSERT_ALWAYS (mpz_cmp_ui (z, 4L) == 0);
    ASSERT_ALWAYS (n == 7);
    ASSERT_ALWAYS (got_ftell == 7);
    mpz_clear (z);
  }

  {
    int  a=9, n=8;
    mpz_t  z;
    mpz_init (z);
    ASSERT_ALWAYS (gmp_sscanf ("1 2 3 4", "%d %*d %*d %Zd%n", &a, z, &n) == 2);
    ASSERT_ALWAYS (a == 1);
    ASSERT_ALWAYS (mpz_cmp_ui (z, 4L) == 0);
    ASSERT_ALWAYS (n == 7);
    mpz_clear (z);
  }
  {
    int  a=9, n=8;
    mpz_t  z;
    mpz_init (z);
    ASSERT_ALWAYS (fromstring_gmp_fscanf ("1 2 3 4", "%d %*d %*d %Zd%n",
                                          &a, z, &n) == 2);
    ASSERT_ALWAYS (a == 1);
    ASSERT_ALWAYS (mpz_cmp_ui (z, 4L) == 0);
    ASSERT_ALWAYS (n == 7);
    ASSERT_ALWAYS (got_ftell == 7);
    mpz_clear (z);
  }

  /* -1 for no matching */
  {
    char buf[128];
    ASSERT_ALWAYS (gmp_sscanf ("   ", "%s", buf) == -1);
    ASSERT_ALWAYS (fromstring_gmp_fscanf ("   ", "%s", buf) == -1);
    if (option_libc_scanf)
      {
        ASSERT_ALWAYS (sscanf ("   ", "%s", buf) == -1);
        ASSERT_ALWAYS (fromstring_fscanf1 ("   ", "%s", buf) == -1);
      }
  }

  /* suppressed field, then eof */
  {
    int  x;
    ASSERT_ALWAYS (gmp_sscanf ("123", "%*d%d", &x) == -1);
    ASSERT_ALWAYS (fromstring_gmp_fscanf ("123", "%*d%d", &x) == -1);
    if (option_libc_scanf)
      {
        ASSERT_ALWAYS (sscanf ("123", "%*d%d", &x) == -1);
        ASSERT_ALWAYS (fromstring_fscanf1 ("123", "%*d%d", &x) == -1);
      }
  }
  {
    mpz_t  x;
    mpz_init (x);
    ASSERT_ALWAYS (gmp_sscanf ("123", "%*Zd%Zd", x) == -1);
    ASSERT_ALWAYS (fromstring_gmp_fscanf ("123", "%*Zd%Zd", x) == -1);
    mpz_clear (x);
  }

  /* %n suppressed */
  {
    int n = 123;
    gmp_sscanf ("   ", " %*n", &n);
    ASSERT_ALWAYS (n == 123);
  }
  {
    int n = 123;
    fromstring_gmp_fscanf ("   ", " %*n", &n);
    ASSERT_ALWAYS (n == 123);
  }

  /* %Zn */
  {
    mpz_t  z;
    mpz_init (z);
    ASSERT_ALWAYS (gmp_sscanf ("xyz   ", "xyz%Zn", z) == 0);
    ASSERT_ALWAYS (mpz_cmp_ui (z, 3L) == 0);
    mpz_clear (z);
  }
  {
    mpz_t  z;
    mpz_init (z);
    ASSERT_ALWAYS (fromstring_gmp_fscanf ("xyz   ", "xyz%Zn", z) == 0);
    ASSERT_ALWAYS (mpz_cmp_ui (z, 3L) == 0);
    mpz_clear (z);
  }

  /* %[...], glibc only */
#ifdef __GLIBC__
  {
    char  buf[128];
    int   n = -1;
    buf[0] = '\0';
    ASSERT_ALWAYS (gmp_sscanf ("abcdefgh", "%[a-d]ef%n", buf, &n) == 1);
    ASSERT_ALWAYS (strcmp (buf, "abcd") == 0);
    ASSERT_ALWAYS (n == 6);
  }
  {
    char  buf[128];
    int   n = -1;
    buf[0] = '\0';
    ASSERT_ALWAYS (gmp_sscanf ("xyza", "%[^a]a%n", buf, &n) == 1);
    ASSERT_ALWAYS (strcmp (buf, "xyz") == 0);
    ASSERT_ALWAYS (n == 4);
  }
  {
    char  buf[128];
    int   n = -1;
    buf[0] = '\0';
    ASSERT_ALWAYS (gmp_sscanf ("ab]ab]", "%[]ab]%n", buf, &n) == 1);
    ASSERT_ALWAYS (strcmp (buf, "ab]ab]") == 0);
    ASSERT_ALWAYS (n == 6);
  }
  {
    char  buf[128];
    int   n = -1;
    buf[0] = '\0';
    ASSERT_ALWAYS (gmp_sscanf ("xyzb", "%[^]ab]b%n", buf, &n) == 1);
    ASSERT_ALWAYS (strcmp (buf, "xyz") == 0);
    ASSERT_ALWAYS (n == 4);
  }
#endif
}


int
main (int argc, char *argv[])
{
  if (argc > 1 && strcmp (argv[1], "-s") == 0)
    option_libc_scanf = 1;

  tests_start ();

  check_z ();
  check_q ();
  check_f ();
  check_misc ();

  unlink (TEMPFILE);
  tests_end ();
  exit (0);
}