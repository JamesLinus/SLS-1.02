/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)pathnames.h	5.4 (Berkeley) 6/1/90
 */

#ifdef linux

#include <paths.h>
#include <netdb.h>

#define _PATH_BOOT	__PATH_ETC"/named.boot"
#define	_PATH_XFER	__PATH_ETC"/named-xfer"
#define	_PATH_DEBUG	"/usr/tmp/named.run"
#define	_PATH_DUMPFILE	"/usr/tmp/named_dump.db"
#define	_PATH_PIDFILE	__PATH_ETC"/named.pid"
#define	_PATH_STATS	"/usr/tmp/named.stats"
#define	_PATH_TMPXFER	"/usr/tmp/xfer.ddt.XXXXXX"
#define	_PATH_TMPDIR	"/usr/tmp"

#else

#define	_PATH_BOOT	"/etc/named.boot"

#if defined(BSD) && BSD >= 198810
#include <paths.h>
#define	_PATH_XFER	"/usr/libexec/named-xfer"
#define	_PATH_DEBUG	"/var/tmp/named.run"
#define	_PATH_DUMPFILE	"/var/tmp/named_dump.db"
#define	_PATH_PIDFILE	"/var/run/named.pid"
#define	_PATH_STATS	"/var/tmp/named.stats"
#define	_PATH_TMPXFER	"/var/tmp/xfer.ddt.XXXXXX"
#define	_PATH_TMPDIR	"/var/tmp"

#else /* BSD */
#define	_PATH_DEVNULL	"/dev/null"
#define	_PATH_TTY	"/dev/tty"
#define	_PATH_XFER	"/etc/named-xfer"
#define	_PATH_DEBUG	"/usr/tmp/named.run"
#define	_PATH_DUMPFILE	"/usr/tmp/named_dump.db"
#define	_PATH_PIDFILE	"/etc/named.pid"
#define	_PATH_STATS	"/usr/tmp/named.stats"
#define	_PATH_TMPXFER	"/usr/tmp/xfer.ddt.XXXXXX"
#define	_PATH_TMPDIR	"/usr/tmp"
#endif /* BSD */

#endif /* linux */

