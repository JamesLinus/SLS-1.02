/*-
 * Copyright (c) 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char sccsid[] = "@(#)krb_des.c	5.1 (Berkeley) 2/28/91";
#endif /* not lint */

/*
 * Copyright (C) 1990 by the Massachusetts Institute of Technology
 *
 * Export of this software from the United States of America is assumed
 * to require a specific license from the United States Government.
 * It is the responsibility of any person or organization contemplating
 * export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

#if	defined(AUTHENTICATE) && defined(ENCRYPT) && defined(DES_ENCRYPT)
#include <arpa/telnet.h>
#include <stdio.h>
#ifdef	__STDC__
#include <stdlib.h>
#endif

#include "encrypt.h"
#include "key-proto.h"
#include "misc-proto.h"

extern encrypt_debug_mode;

#define	CFB	0
#define	OFB	1

#define	NO_SEND_IV	1
#define	NO_RECV_IV	2
#define	NO_KEYID	4
#define	IN_PROGRESS	(NO_SEND_IV|NO_RECV_IV|NO_KEYID)
#define	SUCCESS		0
#define	FAILED		-1


struct fb {
	Block krbdes_key;
	Schedule krbdes_sched;
	Block temp_feed;
	unsigned char fb_feed[64];
	int need_start;
	int state[2];
	int keyid[2];
	int once;
	struct stinfo {
		Block		str_output;
		Block		str_feed;
		Block		str_iv;
		Block		str_ikey;
		Schedule	str_sched;
		int		str_index;
		int		str_flagshift;
	} streams[2];
};

static struct fb fb[2];

struct keyidlist {
	char	*keyid;
	int	keyidlen;
	char	*key;
	int	keylen;
	int	flags;
} keyidlist [] = {
	{ "\0", 1, 0, 0, 0 },		/* default key of zero */
	{ 0, 0, 0, 0, 0 }
};

#define	KEYFLAG_MASK	03

#define	KEYFLAG_NOINIT	00
#define	KEYFLAG_INIT	01
#define	KEYFLAG_OK	02
#define	KEYFLAG_BAD	03

#define	KEYFLAG_SHIFT	2

#define	SHIFT_VAL(a,b)	(KEYFLAG_SHIFT*((a)+((b)*2)))

#define	FB64_IV		1
#define	FB64_IV_OK	2
#define	FB64_IV_BAD	3


void fb64_stream_iv P((Block, struct stinfo *));
void fb64_init P((struct fb *));
int fb64_start P((struct fb *, int, int));
int fb64_is P((unsigned char *, int, struct fb *));
int fb64_reply P((unsigned char *, int, struct fb *));
void fb64_session P((Session_Key *, int, struct fb *));
void fb64_stream_key P((Block, struct stinfo *));
int fb64_keyid P((int, unsigned char *, int *, struct fb *));

	void
cfb64_init(server)
	int server;
{
	fb64_init(&fb[CFB]);
	fb[CFB].fb_feed[4] = ENCTYPE_DES_CFB64;
	fb[CFB].streams[0].str_flagshift = SHIFT_VAL(0, CFB);
	fb[CFB].streams[1].str_flagshift = SHIFT_VAL(1, CFB);
}

	void
ofb64_init(server)
	int server;
{
	fb64_init(&fb[OFB]);
	fb[OFB].fb_feed[4] = ENCTYPE_DES_OFB64;
	fb[CFB].streams[0].str_flagshift = SHIFT_VAL(0, OFB);
	fb[CFB].streams[1].str_flagshift = SHIFT_VAL(1, OFB);
}

	void
fb64_init(fbp)
	register struct fb *fbp;
{
	bzero((void *)fbp, sizeof(*fbp));
	fbp->state[0] = fbp->state[1] = FAILED;
	fbp->fb_feed[0] = IAC;
	fbp->fb_feed[1] = SB;
	fbp->fb_feed[2] = TELOPT_ENCRYPT;
	fbp->fb_feed[3] = ENCRYPT_IS;
}

/*
 * Returns:
 *	-1: some error.  Negotiation is done, encryption not ready.
 *	 0: Successful, initial negotiation all done.
 *	 1: successful, negotiation not done yet.
 *	 2: Not yet.  Other things (like getting the key from
 *	    Kerberos) have to happen before we can continue.
 */
	int
cfb64_start(dir, server)
	int dir;
	int server;
{
	return(fb64_start(&fb[CFB], dir, server));
}
	int
ofb64_start(dir, server)
	int dir;
	int server;
{
	return(fb64_start(&fb[OFB], dir, server));
}

	static int
fb64_start(fbp, dir, server)
	struct fb *fbp;
	int dir;
	int server;
{
	Block b;
	int x;
	unsigned char *p;
	register int state;

	switch (dir) {
	case DIR_DECRYPT:
		/*
		 * This is simply a request to have the other side
		 * start output (our input).  He will negotiate an
		 * IV so we need not look for it.
		 */
		state = fbp->state[dir-1];
		if (state == FAILED)
			state = IN_PROGRESS;
		break;

	case DIR_ENCRYPT:
		state = fbp->state[dir-1];
		if (state == FAILED)
			state = IN_PROGRESS;
		else if ((state & NO_SEND_IV) == 0)
			break;

		if (!VALIDKEY(fbp->krbdes_key)) {
			fbp->need_start = 1;
			break;
		}
		state &= ~NO_SEND_IV;
		state |= NO_RECV_IV;
		if (encrypt_debug_mode)
			printf("Creating new feed\r\n");
		/*
		 * Create a random feed and send it over.
		 */
		des_new_random_key(fbp->temp_feed);
		des_ecb_encrypt(fbp->temp_feed, fbp->temp_feed,
				fbp->krbdes_sched, 1);
		p = fbp->fb_feed + 3;
		*p++ = ENCRYPT_IS;
		p++;
		*p++ = FB64_IV;
		for (x = 0; x < sizeof(Block); ++x) {
			if ((*p++ = fbp->temp_feed[x]) == IAC)
				*p++ = IAC;
		}
		*p++ = IAC;
		*p++ = SE;
		printsub('>', &fbp->fb_feed[2], p - &fbp->fb_feed[2]);
		net_write(fbp->fb_feed, p - fbp->fb_feed);
		break;
	default:
		return(FAILED);
	}
	return(fbp->state[dir-1] = state);
}

/*
 * Returns:
 *	-1: some error.  Negotiation is done, encryption not ready.
 *	 0: Successful, initial negotiation all done.
 *	 1: successful, negotiation not done yet.
 */
	int
cfb64_is(data, cnt)
	unsigned char *data;
	int cnt;
{
	return(fb64_is(data, cnt, &fb[CFB]));
}
	int
ofb64_is(data, cnt)
	unsigned char *data;
	int cnt;
{
	return(fb64_is(data, cnt, &fb[OFB]));
}

	int
fb64_is(data, cnt, fbp)
	unsigned char *data;
	int cnt;
	struct fb *fbp;
{
	int x;
	unsigned char *p;
	Block b;
	register int state = fbp->state[DIR_DECRYPT-1];

	if (cnt-- < 1)
		goto failure;

	switch (*data++) {
	case FB64_IV:
		if (cnt != sizeof(Block)) {
			if (encrypt_debug_mode)
				printf("CFB64: initial vector failed on size\r\n");
			state = FAILED;
			goto failure;
		}

		if (encrypt_debug_mode)
			printf("CFB64: initial vector received\r\n");

		if (encrypt_debug_mode)
			printf("Initializing Decrypt stream\r\n");

		fb64_stream_iv((void *)data, &fbp->streams[DIR_DECRYPT-1]);

		p = fbp->fb_feed + 3;
		*p++ = ENCRYPT_REPLY;
		p++;
		*p++ = FB64_IV_OK;
		*p++ = IAC;
		*p++ = SE;
		printsub('>', &fbp->fb_feed[2], p - &fbp->fb_feed[2]);
		net_write(fbp->fb_feed, p - fbp->fb_feed);

		state = fbp->state[DIR_DECRYPT-1] = IN_PROGRESS;
		break;

	default:
		if (encrypt_debug_mode) {
			printf("Unknown option type: %d\r\n", *(data-1));
			printd(data, cnt);
			printf("\r\n");
		}
		/* FALL THROUGH */
	failure:
		/*
		 * We failed.  Send an FB64_IV_BAD option
		 * to the other side so it will know that
		 * things failed.
		 */
		p = fbp->fb_feed + 3;
		*p++ = ENCRYPT_REPLY;
		p++;
		*p++ = FB64_IV_BAD;
		*p++ = IAC;
		*p++ = SE;
		printsub('>', &fbp->fb_feed[2], p - &fbp->fb_feed[2]);
		net_write(fbp->fb_feed, p - fbp->fb_feed);

		break;
	}
	return(fbp->state[DIR_DECRYPT-1] = state);
}

/*
 * Returns:
 *	-1: some error.  Negotiation is done, encryption not ready.
 *	 0: Successful, initial negotiation all done.
 *	 1: successful, negotiation not done yet.
 */
	int
cfb64_reply(data, cnt)
	unsigned char *data;
	int cnt;
{
	return(fb64_reply(data, cnt, &fb[CFB]));
}
	int
ofb64_reply(data, cnt)
	unsigned char *data;
	int cnt;
{
	return(fb64_reply(data, cnt, &fb[OFB]));
}


	int
fb64_reply(data, cnt, fbp)
	unsigned char *data;
	int cnt;
	struct fb *fbp;
{
	int x;
	unsigned char *p;
	Block b;
	register int state = fbp->state[DIR_ENCRYPT-1];

	if (cnt-- < 1)
		goto failure;

	switch (*data++) {
	case FB64_IV_OK:
		fb64_stream_iv(fbp->temp_feed, &fbp->streams[DIR_ENCRYPT-1]);
		if (state == FAILED)
			state = IN_PROGRESS;
		state &= ~NO_RECV_IV;
		encrypt_send_keyid(DIR_ENCRYPT, "\0", 1, 1);
		break;

	case FB64_IV_BAD:
		bzero(fbp->temp_feed, sizeof(Block));
		fb64_stream_iv(fbp->temp_feed, &fbp->streams[DIR_ENCRYPT-1]);
		state = FAILED;
		break;

	default:
		if (encrypt_debug_mode) {
			printf("Unknown option type: %d\r\n", data[-1]);
			printd(data, cnt);
			printf("\r\n");
		}
		/* FALL THROUGH */
	failure:
		state = FAILED;
		break;
	}
	return(fbp->state[DIR_ENCRYPT-1] = state);
}

	void
cfb64_session(key, server)
	Session_Key *key;
	int server;
{
	fb64_session(key, server, &fb[CFB]);
}

	void
ofb64_session(key, server)
	Session_Key *key;
	int server;
{
	fb64_session(key, server, &fb[OFB]);
}

	static void
fb64_session(key, server, fbp)
	Session_Key *key;
	int server;
	struct fb *fbp;
{

	if (!key || key->type != SK_DES) {
		if (encrypt_debug_mode)
			printf("Can't set krbdes's session key (%d != %d)\r\n",
				key ? key->type : -1, SK_DES);
		return;
	}
	bcopy((void *)key->data, (void *)fbp->krbdes_key, sizeof(Block));

	fb64_stream_key(fbp->krbdes_key, &fbp->streams[DIR_ENCRYPT-1]);
	fb64_stream_key(fbp->krbdes_key, &fbp->streams[DIR_DECRYPT-1]);

	if (fbp->once == 0) {
		des_set_random_generator_seed(fbp->krbdes_key);
		fbp->once = 1;
	}
	des_key_sched(fbp->krbdes_key, fbp->krbdes_sched);
	/*
	 * Now look to see if krbdes_start() was was waiting for
	 * the key to show up.  If so, go ahead an call it now
	 * that we have the key.
	 */
	if (fbp->need_start) {
		fbp->need_start = 0;
		fb64_start(fbp, DIR_ENCRYPT, server);
	}
}

/*
 * We only accept a keyid of 0.  If we get a keyid of
 * 0, then mark the state as SUCCESS.
 */
	int
cfb64_keyid(dir, kp, lenp)
	int dir, *lenp;
	unsigned char *kp;
{
	return(fb64_keyid(dir, kp, lenp, &fb[CFB]));
}

	int
ofb64_keyid(dir, kp, lenp)
	int dir, *lenp;
	unsigned char *kp;
{
	return(fb64_keyid(dir, kp, lenp, &fb[OFB]));
}

	int
fb64_keyid(dir, kp, lenp, fbp)
	int dir, *lenp;
	unsigned char *kp;
	struct fb *fbp;
{
	register int state = fbp->state[dir-1];

	if (*lenp != 1 || (*kp != '\0')) {
		*lenp = 0;
		return(state);
	}

	if (state == FAILED)
		state = IN_PROGRESS;

	state &= ~NO_KEYID;

	return(fbp->state[dir-1] = state);
}

	void
fb64_printsub(data, cnt, buf, buflen, type)
	unsigned char *data, *buf, *type;
	int cnt, buflen;
{
	char lbuf[32];
	register int i;
	char *cp;

	buf[buflen-1] = '\0';		/* make sure it's NULL terminated */
	buflen -= 1;

	switch(data[2]) {
	case FB64_IV:
		sprintf(lbuf, "%s_IV", type);
		cp = lbuf;
		goto common;

	case FB64_IV_OK:
		sprintf(lbuf, "%s_IV_OK", type);
		cp = lbuf;
		goto common;

	case FB64_IV_BAD:
		sprintf(lbuf, "%s_IV_BAD", type);
		cp = lbuf;
		goto common;

	default:
		sprintf(lbuf, " %d (unknown)", data[2]);
		cp = lbuf;
	common:
		for (; (buflen > 0) && (*buf = *cp++); buf++)
			buflen--;
		for (i = 3; i < cnt; i++) {
			sprintf(lbuf, " %d", data[i]);
			for (cp = lbuf; (buflen > 0) && (*buf = *cp++); buf++)
				buflen--;
		}
		break;
	}
}

	void
cfb64_printsub(data, cnt, buf, buflen)
	unsigned char *data, *buf;
	int cnt, buflen;
{
	fb64_printsub(data, cnt, buf, buflen, "CFB64");
}

	void
ofb64_printsub(data, cnt, buf, buflen)
	unsigned char *data, *buf;
	int cnt, buflen;
{
	fb64_printsub(data, cnt, buf, buflen, "OFB64");
}

	void
fb64_stream_iv(seed, stp)
	Block seed;
	register struct stinfo *stp;
{

	bcopy((void *)seed, (void *)stp->str_iv, sizeof(Block));
	bcopy((void *)seed, (void *)stp->str_output, sizeof(Block));

	des_key_sched(stp->str_ikey, stp->str_sched);

	stp->str_index = sizeof(Block);
}

	void
fb64_stream_key(key, stp)
	Block key;
	register struct stinfo *stp;
{
	bcopy((void *)key, (void *)stp->str_ikey, sizeof(Block));
	des_key_sched(key, stp->str_sched);

	bcopy((void *)stp->str_iv, (void *)stp->str_output, sizeof(Block));

	stp->str_index = sizeof(Block);
}

/*
 * DES 64 bit Cipher Feedback
 *
 *     key --->+-----+
 *          +->| DES |--+
 *          |  +-----+  |
 *	    |           v
 *  INPUT --(--------->(+)+---> DATA
 *          |             |
 *	    +-------------+
 *         
 *
 * Given:
 *	iV: Initial vector, 64 bits (8 bytes) long.
 *	Dn: the nth chunk of 64 bits (8 bytes) of data to encrypt (decrypt).
 *	On: the nth chunk of 64 bits (8 bytes) of encrypted (decrypted) output.
 *
 *	V0 = DES(iV, key)
 *	On = Dn ^ Vn
 *	V(n+1) = DES(On, key)
 */

	void
cfb64_encrypt(s, c)
	register unsigned char *s;
	int c;
{
	register struct stinfo *stp = &fb[CFB].streams[DIR_ENCRYPT-1];
	register int index;

	index = stp->str_index;
	while (c-- > 0) {
		if (index == sizeof(Block)) {
			Block b;
			des_ecb_encrypt(stp->str_output, b, stp->str_sched, 1);
			bcopy((void *)b, (void *)stp->str_feed, sizeof(Block));
			index = 0;
		}

		/* On encryption, we store (feed ^ data) which is cypher */
		*s = stp->str_output[index] = (stp->str_feed[index] ^ *s);
		s++;
		index++;
	}
	stp->str_index = index;
}

	int
cfb64_decrypt(data)
	int data;
{
	register struct stinfo *stp = &fb[CFB].streams[DIR_DECRYPT-1];
	int index;

	if (data == -1) {
		/*
		 * Back up one byte.  It is assumed that we will
		 * never back up more than one byte.  If we do, this
		 * may or may not work.
		 */
		if (stp->str_index)
			--stp->str_index;
		return(0);
	}

	index = stp->str_index++;
	if (index == sizeof(Block)) {
		Block b;
		des_ecb_encrypt(stp->str_output, b, stp->str_sched, 1);
		bcopy((void *)b, (void *)stp->str_feed, sizeof(Block));
		stp->str_index = 1;	/* Next time will be 1 */
		index = 0;		/* But now use 0 */ 
	}

	/* On decryption we store (data) which is cypher. */
	stp->str_output[index] = data;
	return(data ^ stp->str_feed[index]);
}

/*
 * DES 64 bit Output Feedback
 *
 * key --->+-----+
 *	+->| DES |--+
 *	|  +-----+  |
 *	+-----------+
 *	            v
 *  INPUT -------->(+) ----> DATA
 *
 * Given:
 *	iV: Initial vector, 64 bits (8 bytes) long.
 *	Dn: the nth chunk of 64 bits (8 bytes) of data to encrypt (decrypt).
 *	On: the nth chunk of 64 bits (8 bytes) of encrypted (decrypted) output.
 *
 *	V0 = DES(iV, key)
 *	V(n+1) = DES(Vn, key)
 *	On = Dn ^ Vn
 */
	void
ofb64_encrypt(s, c)
	register unsigned char *s;
	int c;
{
	register struct stinfo *stp = &fb[OFB].streams[DIR_ENCRYPT-1];
	register int index;

	index = stp->str_index;
	while (c-- > 0) {
		if (index == sizeof(Block)) {
			Block b;
			des_ecb_encrypt(stp->str_feed, b, stp->str_sched, 1);
			bcopy((void *)b, (void *)stp->str_feed, sizeof(Block));
			index = 0;
		}
		*s++ ^= stp->str_feed[index];
		index++;
	}
	stp->str_index = index;
}

	int
ofb64_decrypt(data)
	int data;
{
	register struct stinfo *stp = &fb[OFB].streams[DIR_DECRYPT-1];
	int index;

	if (data == -1) {
		/*
		 * Back up one byte.  It is assumed that we will
		 * never back up more than one byte.  If we do, this
		 * may or may not work.
		 */
		if (stp->str_index)
			--stp->str_index;
		return(0);
	}

	index = stp->str_index++;
	if (index == sizeof(Block)) {
		Block b;
		des_ecb_encrypt(stp->str_feed, b, stp->str_sched, 1);
		bcopy((void *)b, (void *)stp->str_feed, sizeof(Block));
		stp->str_index = 1;	/* Next time will be 1 */
		index = 0;		/* But now use 0 */ 
	}

	return(data ^ stp->str_feed[index]);
}
#endif
