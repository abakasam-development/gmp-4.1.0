/* mpz_congruent_2exp_p -- test congruence of mpz mod 2^n */

/*
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
MA 02111-1307, USA.
*/

#include "gmp.h"
#include "gmp-impl.h"


int
mpz_congruent_2exp_p (mpz_srcptr a, mpz_srcptr c, unsigned long d)
{
  unsigned long  i, dlimbs, dbits;
  mp_ptr         ap, cp;
  mp_limb_t      dmask, alimb, climb, sum;
  int            asize_signed, csize_signed, asize, csize;

  if (ABSIZ(a) < ABSIZ(c))
    MPZ_SRCPTR_SWAP (a, c);

  dlimbs = d / BITS_PER_MP_LIMB;
  dbits = d % BITS_PER_MP_LIMB;
  dmask = (CNST_LIMB(1) << dbits) - 1;

  ap = PTR(a);
  cp = PTR(c);

  asize_signed = SIZ(a);
  asize = ABS(asize_signed);

  csize_signed = SIZ(c);
  csize = ABS(csize_signed);

  if (csize_signed == 0)
    goto a_zeros;

  if ((asize_signed ^ csize_signed) >= 0)
    {
      /* same signs, direct comparison */

      /* a==c for limbs in common */
      if (mpn_cmp (ap, cp, MIN (csize, dlimbs)) != 0)
        return 0;

      /* if that's all of dlimbs, then a==c for remaining bits */
      if (csize > dlimbs)
        return ((ap[dlimbs]-cp[dlimbs]) & dmask) == 0;

    a_zeros:
      /* a remains, need all zero bits */

      /* if d covers all of a and c, then must be exactly equal */
      if (asize <= dlimbs)
        return asize == csize;
      
      /* whole limbs zero */
      for (i = csize; i < dlimbs; i++)
        if (ap[i] != 0)
          return 0;
      
      /* partial limb zero */
      return (ap[dlimbs] & dmask) == 0;
    }
  else
    {
      /* different signs, negated comparison */

      /* common low zero limbs, stopping at first non-zeros, which must
         match twos complement */
      i = 0;
      for (;;)
        {
          ASSERT (i < csize);  /* always have a non-zero limb on c */
          alimb = ap[i];
          climb = cp[i];
          sum = alimb + climb;

          if (i >= dlimbs)
            return (sum & dmask) == 0;
          i++;

          /* require both zero, or first non-zeros as twos-complements */
          if (sum != 0)
            return 0;

          if (alimb != 0)
            break;
        }

      /* further limbs matching as ones-complement */
      for (;;)
        {
          if (i >= csize)
            break;

          alimb = ap[i];
          climb = cp[i];
          sum = alimb + climb + 1;

          if (i >= dlimbs)
            return (sum & dmask) == 0;

          if (sum != 0)
            return 0;

          i++;
        }

      /* no more c, so require all 1 bits in a */

      if (asize < dlimbs)
        return 0;   /* not enough a */

      /* whole limbs */
      for ( ; i < dlimbs; i++)
        if (ap[i] + 1 != 0)
          return 0;

      /* if only whole limbs, no further fetches from a */
      if (dbits == 0)
        return 1;

      /* need enough a */
      if (asize == dlimbs)
        return 0;

      return ((ap[dlimbs]+1) & dmask) == 0;
    }
}