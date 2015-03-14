/*
  This file is part of the NetFax system.

  (c) Copyright 1989 by David M. Siegel and Sundar Narasimhan.
      All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation.

    This program is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef INChashh
#define INChashh 1

typedef struct _htentry {
    char *ht_key;
    char *ht_data;
    struct _htentry *ht_next;
} HTENTRY;

typedef struct _httable {
    HTENTRY **ht_entries;
    int ht_size;
    int ht_keysize;
    int ht_noe;
    int ht_noc;
} HTTABLE;

#define htnoe(x) x->ht_noe
#define htnoc(x) x->ht_noc

/*
  Prototypes:
*/

HTTABLE *htinit(
#ifdef _PROTO
    int size,
    int keysize
#endif
);

void htfree(
#ifdef _PROTO
    HTTABLE *table
#endif
);

/*VARARGS*/
int htmap();

void htstat(
#ifdef _PROTO
    FILE *fp,
    HTTABLE *table
#endif
);

int hthash(
#ifdef _PROTO
    char *s,
    HTTABLE *table
#endif
);

HTENTRY *htlookup(
#ifdef _PROTO
    char *str,
    HTTABLE *table
#endif
);

char *htgetdata(
#ifdef _PROTO
    char *str,
    HTTABLE *table
#endif
);

HTENTRY *htadd_hte(
#ifdef _PROTO
    char *str,
    HTTABLE *table,
    HTENTRY *hte,
    char *data
#endif
);

HTENTRY *htadd(
#ifdef _PROTO
    char *str,
    HTTABLE *table,
    char *data
#endif
);

HTENTRY *htdelete_hte(
#ifdef _PROTO
    char *str,
    HTTABLE *table
#endif
);

int htdelete(
#ifdef _PROTO
    char *str,
    HTTABLE *table
#endif
);

#endif
