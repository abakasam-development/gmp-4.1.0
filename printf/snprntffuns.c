/* __gmp_snprintf_funs -- support for gmp_snprintf and gmp_vsnprintf.

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

#if HAVE_VSNPRINTF

#if HAVE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <stdio.h>
#include <string.h>

#include "gmp.h"
#include "gmp-impl.h"


/* glibc 2.0.x vsnprintf returns size-1 for an overflow, with no indication
   how big the output would have been.  It's necessary to re-run to
   determine that size.

   "size-1" would mean sucess from a C99 vsnprintf, and the re-run is
   unnecessary in this case, but we don't bother to try to detect what sort
   of vsnprintf we've got.  size-1 should occur rarely in normal
   circumstances.  */

static int
gmp_snprintf_format (struct gmp_snprintf_t *d, const char *fmt, va_list ap)
{
  int   ret, step, alloc;
  char  *p;

  ASSERT (d->size >= 0);

  if (d->size > 1)
    {
      ret = vsnprintf (d->buf, d->size, fmt, ap);
      if (ret == -1)
        return ret;

      step = MIN (ret, d->size-1);
      d->size -= step;
      d->buf += step;

      if (ret != d->size-1)
        return ret;

      /* probably glibc 2.0.x truncated output, probe for actual size */
      alloc = MAX (128, ret);
    }
  else
    {
      /* no space to write anything, just probe for size */
      alloc = 128;
    }

  do
    {
      alloc *= 2;
      p = (*__gmp_allocate_func) (alloc);
      ret = vsnprintf (p, alloc, fmt, ap);
      (*__gmp_free_func) (p, alloc);
    }
  while (ret == alloc-1);

  return ret;
}

static int
gmp_snprintf_memory (struct gmp_snprintf_t *d, const char *str, size_t len)
{
  size_t n;

  ASSERT (d->size >= 0);

  if (d->size > 1)
    {
      n = MIN (d->size-1, len);
      memcpy (d->buf, str, n);
      d->buf += n;
      d->size -= n;
    }
  return len;
}

static int
gmp_snprintf_reps (struct gmp_snprintf_t *d, int c, int reps)
{
  size_t n;

  ASSERT (reps >= 0);
  ASSERT (d->size >= 0);

  if (d->size > 1)
    {
      n = MIN (d->size-1, reps);
      memset (d->buf, c, n);
      d->buf += n;
      d->size -= n;
    }
  return reps;
}

static int
gmp_snprintf_final (struct gmp_snprintf_t *d, int c, int reps)
{
  if (d->size >= 1)
    d->buf[0] = '\0';
  return 0;
}

const struct doprnt_funs_t  __gmp_snprintf_funs = {
  (doprnt_format_t) gmp_snprintf_format,
  (doprnt_memory_t) gmp_snprintf_memory,
  (doprnt_reps_t)   gmp_snprintf_reps,
  (doprnt_final_t)  gmp_snprintf_final
};

#endif /* HAVE_VSNPRINTF */