/* mpfr_nan_p, mpfr_inf_p -- check for NaN or infinities

Copyright (C) 2000-2001 Free Software Foundation.

This file is part of the MPFR Library.

The MPFR Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The MPFR Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with the MPFR Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA. */

#include "gmp.h"
#include "gmp-impl.h"
#include "mpfr.h"
#include "mpfr-impl.h"

int
mpfr_nan_p (mpfr_srcptr x)
{
  return MPFR_IS_NAN (x);
}

int
mpfr_inf_p (mpfr_srcptr x)
{
  return (!MPFR_IS_NAN(x) && MPFR_IS_INF(x));
}

int
mpfr_number_p (mpfr_srcptr x)
{
  return !MPFR_IS_NAN(x) && !MPFR_IS_INF(x);
}