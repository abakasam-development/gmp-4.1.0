/* Test .type directives on assembler functions.

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

#include <stdio.h>
#include <stdlib.h>

#include "gmp.h"
#include "gmp-impl.h"

#include "tests.h"


/* This apparently trivial test is designed to detect missing .type and
   .size directives in asm code, per the problem described under
   GMP_ASM_TYPE in acinclude.m4.

   A failure can be provoked in a shared or shared+static build by making
   TYPE and SIZE in config.m4 empty, either by editing it or by configuring
   with

       ./configure gmp_cv_asm_type= gmp_cv_asm_size=

   mpn_add_n is used for the test because normally it's implemented in
   assembler on a CPU that has any asm code.

   Enhancement: As noted with GMP_ASM_TYPE, if .type is wrong but .size is
   right then everything works, but doesn't go through the PLT properly,
   more than likely producing relocations in the library image.  Maybe we
   could detect that if we built a test library with an object that had
   .size deliberately disabled.  */

int
main (void)
{
  static const mp_limb_t x[3]    = { 1, 2, 3 };
  static const mp_limb_t y[3]    = { 4, 5, 6 };
  static const mp_limb_t want[3] = { 5, 7, 9 };
  mp_limb_t  got[3];

  mpn_add_n (got, x, y, (mp_size_t) 3);

  if (refmpn_cmp (got, want, (mp_size_t) 3) != 0)
    {
      printf ("Wrong result from mpn_add_n\n");
      abort ();
    }

  exit (0);
}