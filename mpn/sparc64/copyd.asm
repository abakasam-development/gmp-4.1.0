dnl  SPARC v9 mpn_copyd -- Copy a limb vector, decrementing.

dnl  Copyright 1999, 2000, 2001 Free Software Foundation, Inc.

dnl  This file is part of the GNU MP Library.

dnl  The GNU MP Library is free software; you can redistribute it and/or modify
dnl  it under the terms of the GNU Lesser General Public License as published
dnl  by the Free Software Foundation; either version 2.1 of the License, or (at
dnl  your option) any later version.

dnl  The GNU MP Library is distributed in the hope that it will be useful, but
dnl  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
dnl  or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
dnl  License for more details.

dnl  You should have received a copy of the GNU Lesser General Public License
dnl  along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
dnl  the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
dnl  MA 02111-1307, USA.


dnl  INPUT PARAMETERS
dnl  rptr	%o0
dnl  sptr	%o1
dnl  n		%o2

include(`../config.m4')

ASM_START()
	.register	%g2,#scratch
	.register	%g3,#scratch
PROLOGUE(mpn_copyd)
	sllx	%o2,3,%g1
	add	%g1,%o0,%o0
	add	%g1,%o1,%o1
	addcc	%o2,-8,%g0
	bl,pn	%icc,L(loop2)
	nop
	add	%o2,-8,%o2

L(loop1):
	ldx	[%o1-8],%g1
	ldx	[%o1-16],%g2
	ldx	[%o1-24],%g3
	ldx	[%o1-32],%g4
	ldx	[%o1-40],%g5
	ldx	[%o1-48],%o3
	ldx	[%o1-56],%o4
	ldx	[%o1-64],%o5
	add	%o1,-64,%o1
	stx	%g1,[%o0-8]
	stx	%g2,[%o0-16]
	stx	%g3,[%o0-24]
	stx	%g4,[%o0-32]
	stx	%g5,[%o0-40]
	stx	%o3,[%o0-48]
	stx	%o4,[%o0-56]
	stx	%o5,[%o0-64]
	addcc	%o2,-8,%o2
	bge,pt	%icc,L(loop1)
	add	%o0,-64,%o0

	addcc	%o2,8,%o2
	bz,pn	%icc,L(end)
	nop

L(loop2):
	ldx	[%o1-8],%g1
	add	%o1,-8,%o1
	addcc	%o2,-1,%o2
	stx	%g1,[%o0-8]
	bg,pt	%icc,L(loop2)
	add	%o0,-8,%o0

L(end):	retl
	nop
EPILOGUE(mpn_copyd)