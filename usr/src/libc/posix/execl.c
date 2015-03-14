/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include <stddef.h>

#ifndef	HAVE_GNU_LD
#define	__environ	environ
#endif

/* Execute PATH with all arguments after PATH until
   a NULL pointer and environment from `environ'.  */
int
DEFUN(execl, (path), CONST char *path AND CONST char *arg DOTS)
{
  CONST char *argv[8192];
  register unsigned int i;
  va_list args;

  va_start(args, path);
  argv[0] = arg;
  for (i = 0; argv[i++] != NULL;)
    {
      if (i >= sizeof argv / sizeof argv[0])
	{
	  va_end(args);
	  return E2BIG;
	}
      argv[i] = va_arg(args, CONST char *);
    }

  va_end(args);

  return __execve(path, (char *CONST *) argv, __environ);
}