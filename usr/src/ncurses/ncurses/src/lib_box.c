
/* This work is copyrighted. See COPYRIGHT.OLD & COPYRIGHT.NEW for   *
*  details. If they are missing then this copy is in violation of    *
*  the copyright conditions.                                        */

/*
**	lib_box.c
**
**	line drawing routines:
**	wborder()
**	whline()
**	wvline()
**
*/

#include "curses.h"
#include "curses.priv.h"

int wborder(WINDOW *win, chtype ls, chtype rs, chtype ts,
	chtype bs, chtype tl, chtype tr, chtype bl, chtype br)
{
int i;
int endx, endy;

#ifdef TRACE
	if (_tracing)
	    _tracef("wborder() called");
#endif

	if (ls == 0) ls = ACS_VLINE;
	if (rs == 0) rs = ACS_VLINE;
	if (ts == 0) ts = ACS_HLINE;
	if (bs == 0) bs = ACS_HLINE;
	if (tl == 0) tl = ACS_ULCORNER;
	if (tr == 0) tr = ACS_URCORNER;
	if (bl == 0) bl = ACS_LLCORNER;
	if (br == 0) br = ACS_LRCORNER;

	ls |= win->_attrs;
	rs |= win->_attrs;
	ts |= win->_attrs;
	bs |= win->_attrs;
	tl |= win->_attrs;
	tr |= win->_attrs;
	bl |= win->_attrs;
	br |= win->_attrs;

	endx = win->_maxx;
	endy = win->_maxy;

	for (i = 0; i <= endx; i++) {
		win->_line[0][i] = ts; 
		win->_line[endy][i] = bs; 
	}
	win->_firstchar[endy] = win->_firstchar[0] = 0;
	win->_lastchar[endy] = win->_lastchar[0] = endx;

	for (i = 0; i <= endy; i++)
	{
		win->_line[i][0] =  ls;
		win->_line[i][endx] =  rs;
		win->_firstchar[i] = 0;
		win->_lastchar[i] = endx;
	}
	win->_line[0][0] = tl;
	win->_line[0][endx] = tr;
	win->_line[endy][0] = bl;
	win->_line[endy][endx] = br;

#if 0
	if (! win->_scroll  &&  (win->_flags & _SCROLLWIN))
	    fp[0] = fp[endx] = lp[0] = lp[endx] = ' ';
#endif

	return OK;
}

int whline(WINDOW *win, chtype ch, int n)
{
int line;
int start;
int end;

#ifdef TRACE
	if (_tracing)
	    _tracef("whline(%x,%x,%d) called", win, ch, n);
#endif

	line = win->_cury;
	start = win->_curx + 1;
	end = start + n;
	if (end > win->_maxx) end = win->_maxx;

	if (win->_firstchar[line] > start) win->_firstchar[line] = start;
	if (win->_lastchar[line] < start) win->_lastchar[line] = end;

	while ( end >= start) {
		win->_line[line][end] = ch;
		end--;
	}

	return OK;
}

int wvline(WINDOW *win, chtype ch, int n)
{
int row, col;
int end;

#ifdef TRACE
	if (_tracing)
	    _tracef("wvline(%x,%x,%d) called", win, ch, n);
#endif

	row = win->_cury + 1;
	col = win->_curx;
	end = row + n;
	if (end > win->_maxy) end = win->_maxy;
	while(end >= row) {
		win->_line[end][col] = ch;
		if (win->_firstchar[end] > col) win->_firstchar[end] = col;
		if (win->_lastchar[end] < col) win->_lastchar[end] = col;
		end--;
	}

	return OK;
}
	
