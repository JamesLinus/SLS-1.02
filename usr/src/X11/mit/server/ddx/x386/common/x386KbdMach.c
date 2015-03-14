/*
 *****************************************************************************
 * HISTORY
 * Log:	x386KbdMach.c,v
 * Revision 2.1.2.1  92/06/25  10:32:08  moore
 * 	Incorporate the Elliot Dresselhaus's, Ernest Hua's and local changes
 * 	to run Thomas Roell's I386 color X11R5.  Original code only worked
 * 	with SCO Unix.  New code works with 2.5 and 3.0 Mach
 * 	[92/06/24            rvb]
 * 
 * 	EndLog
 * 
 *****************************************************************************
 */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Thomas Roell not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Thomas Roell makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THOMAS ROELL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THOMAS ROELL BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * $Header: /home/x_cvs/mit/server/ddx/x386/common/x386KbdMach.c,v 1.4 1992/09/29 13:43:11 dawes Exp $
 * Header: x386KbdMach.c,v 2.1.2.1 92/06/25 10:32:08 moore Exp
 */

#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "x386Procs.h"
#include "x386OSD.h"
#include "atKeynames.h"

/*
 * For Scancodes see notes in atKeynames.h  !!!!
 */
static KeySym map[NUM_KEYCODES * GLYPHS_PER_KEY] = { 
    /* 0x00 */  NoSymbol,       NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x01 */  XK_Escape,      NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x02 */  XK_1,           XK_exclam,	NoSymbol,	NoSymbol,
    /* 0x03 */  XK_2,           XK_at,		NoSymbol,	NoSymbol,
    /* 0x04 */  XK_3,           XK_numbersign,	NoSymbol,	NoSymbol,
    /* 0x05 */  XK_4,           XK_dollar,	NoSymbol,	NoSymbol,
    /* 0x06 */  XK_5,           XK_percent,	NoSymbol,	NoSymbol,
    /* 0x07 */  XK_6,           XK_asciicircum,	NoSymbol,	NoSymbol,
    /* 0x08 */  XK_7,           XK_ampersand,	NoSymbol,	NoSymbol,
    /* 0x09 */  XK_8,           XK_asterisk,	NoSymbol,	NoSymbol,
    /* 0x0a */  XK_9,           XK_parenleft,	NoSymbol,	NoSymbol,
    /* 0x0b */  XK_0,           XK_parenright,	NoSymbol,	NoSymbol,
    /* 0x0c */  XK_minus,       XK_underscore,	NoSymbol,	NoSymbol,
    /* 0x0d */  XK_equal,       XK_plus,	NoSymbol,	NoSymbol,
    /* 0x0e */  XK_BackSpace,   NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x0f */  XK_Tab,         NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x10 */  XK_Q,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x11 */  XK_W,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x12 */  XK_E,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x13 */  XK_R,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x14 */  XK_T,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x15 */  XK_Y,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x16 */  XK_U,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x17 */  XK_I,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x18 */  XK_O,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x19 */  XK_P,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x1a */  XK_bracketleft, XK_braceleft,	NoSymbol,	NoSymbol,
    /* 0x1b */  XK_bracketright,XK_braceright,	NoSymbol,	NoSymbol,
    /* 0x1c */  XK_Return,      NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x1d */  XK_Control_L,   NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x1e */  XK_A,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x1f */  XK_S,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x20 */  XK_D,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x21 */  XK_F,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x22 */  XK_G,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x23 */  XK_H,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x24 */  XK_J,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x25 */  XK_K,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x26 */  XK_L,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x27 */  XK_semicolon,   XK_colon,	NoSymbol,	NoSymbol,
    /* 0x28 */  XK_quoteright,  XK_quotedbl,	NoSymbol,	NoSymbol,
    /* 0x29 */  XK_quoteleft,	XK_asciitilde,	NoSymbol,	NoSymbol,
    /* 0x2a */  XK_Shift_L,     NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x2b */  XK_backslash,   XK_bar,		NoSymbol,	NoSymbol,
    /* 0x2c */  XK_Z,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x2d */  XK_X,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x2e */  XK_C,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x2f */  XK_V,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x30 */  XK_B,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x31 */  XK_N,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x32 */  XK_M,           NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x33 */  XK_comma,       XK_less,	NoSymbol,	NoSymbol,
    /* 0x34 */  XK_period,      XK_greater,	NoSymbol,	NoSymbol,
    /* 0x35 */  XK_slash,       XK_question,	NoSymbol,	NoSymbol,
    /* 0x36 */  XK_Shift_R,     NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x37 */  XK_KP_Multiply, NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x38 */  XK_Alt_L,	XK_Meta_L,	NoSymbol,	NoSymbol,
    /* 0x39 */  XK_space,       NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3a */  XK_Caps_Lock,   NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3b */  XK_F1,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3c */  XK_F2,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3d */  XK_F3,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3e */  XK_F4,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x3f */  XK_F5,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x40 */  XK_F6,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x41 */  XK_F7,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x42 */  XK_F8,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x43 */  XK_F9,          NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x44 */  XK_F10,         NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x45 */  XK_Num_Lock,    NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x46 */  XK_Scroll_Lock,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x47 */  XK_Home,        XK_KP_7,	XK_KP_7,	XK_Home,
    /* 0x48 */  XK_Up,          XK_KP_8,	XK_KP_8,	XK_Up,
    /* 0x49 */  XK_Prior,       XK_KP_9,	XK_KP_9,	XK_Prior,
    /* 0x4a */  XK_KP_Subtract, NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x4b */  XK_Left,        XK_KP_4,	XK_KP_4,	XK_Left,
    /* 0x4c */  XK_Begin,       XK_KP_5,	XK_KP_5,	XK_Begin,
    /* 0x4d */  XK_Right,       XK_KP_6,	XK_KP_6,	XK_Right,
    /* 0x4e */  XK_KP_Add,      NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x4f */  XK_End,         XK_KP_1,	XK_KP_1,	XK_End,
    /* 0x50 */  XK_Down,        XK_KP_2,	XK_KP_2,	XK_Down,
    /* 0x51 */  XK_Next,        XK_KP_3,	XK_KP_3,	XK_Next,
    /* 0x52 */  XK_Insert,      XK_KP_0,	XK_KP_0,	XK_Insert,
    /* 0x53 */  XK_Delete,      XK_KP_Decimal,	XK_KP_Decimal,	XK_Delete,
    /* 0x54 */  XK_Sys_Req,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x55 */  NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x56 */  XK_less,	XK_greater,	NoSymbol,	NoSymbol,
    /* 0x57 */  XK_F11,		NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x58 */  XK_F12,		NoSymbol,	NoSymbol,	NoSymbol,

    /* 0x59 */  XK_Home,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5a */  XK_Up,		NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5b */  XK_Prior,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5c */  XK_Left,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5d */  XK_Begin,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5e */  XK_Right,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x5f */  XK_End,		NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x60 */  XK_Down,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x61 */  XK_Next,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x62 */  XK_Insert,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x63 */  XK_Delete,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x64 */  XK_KP_Enter,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x65 */  XK_Control_R,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x66 */  XK_Pause,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x67 */  XK_Print,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x68 */  XK_KP_Divide,	NoSymbol,	NoSymbol,	NoSymbol,
    /* 0x69 */  XK_Alt_R,	XK_Meta_R,	NoSymbol,	NoSymbol,
    /* 0x6a */  XK_Break,	NoSymbol,	NoSymbol,	NoSymbol,
};

static KeySym ascii_to_x[256] = {
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	XK_BackSpace,	XK_Tab,		XK_Linefeed,	NoSymbol,
	NoSymbol,	XK_Return,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	XK_Escape,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	XK_space,	XK_exclam,	XK_quotedbl,	XK_numbersign,
	XK_dollar,	XK_percent,	XK_ampersand,	XK_apostrophe,
	XK_parenleft,	XK_parenright,	XK_asterisk,	XK_plus,
	XK_comma,	XK_minus,	XK_period,	XK_slash,
	XK_0,		XK_1,		XK_2,		XK_3,
	XK_4,		XK_5,		XK_6,		XK_7,
	XK_8,		XK_9,		XK_colon,	XK_semicolon,
	XK_less,	XK_equal,	XK_greater,	XK_question,
	XK_at,		XK_A,		XK_B,		XK_C,
	XK_D,		XK_E,		XK_F,		XK_G,
	XK_H,		XK_I,		XK_J,		XK_K,
	XK_L,		XK_M,		XK_N,		XK_O,
	XK_P,		XK_Q,		XK_R,		XK_S,
	XK_T,		XK_U,		XK_V,		XK_W,
	XK_X,		XK_Y,		XK_Z,		XK_bracketleft,
	XK_backslash,	XK_bracketright,XK_asciicircum,	XK_underscore,
	XK_grave,	XK_a,		XK_b,		XK_c,
	XK_d,		XK_e,		XK_f,		XK_g,
	XK_h,		XK_i,		XK_j,		XK_k,
	XK_l,		XK_m,		XK_n,		XK_o,
	XK_p,		XK_q,		XK_r,		XK_s,
	XK_t,		XK_u,		XK_v,		XK_w,
	XK_x,		XK_y,		XK_z,		XK_braceleft,
	XK_bar,		XK_braceright,	XK_asciitilde,	XK_Delete,
	XK_Ccedilla,	XK_udiaeresis,	XK_eacute,	XK_acircumflex,
	XK_adiaeresis,	XK_agrave,	XK_aring,	XK_ccedilla,
	XK_ecircumflex,	XK_ediaeresis,	XK_egrave,	XK_idiaeresis,
	XK_icircumflex,	XK_igrave,	XK_Adiaeresis,	XK_Aring,
	XK_Eacute,	XK_ae,		XK_AE,		XK_ocircumflex,
	XK_odiaeresis,	XK_ograve,	XK_ucircumflex,	XK_ugrave,
	XK_ydiaeresis,	XK_Odiaeresis,	XK_Udiaeresis,	XK_cent,
	XK_sterling,	XK_yen,		XK_paragraph,	XK_section,
	XK_aacute,	XK_iacute,	XK_oacute,	XK_uacute,
	XK_ntilde,	XK_Ntilde,	XK_ordfeminine,	XK_masculine,
	XK_questiondown,XK_hyphen,	XK_notsign,	XK_onehalf,
	XK_onequarter,	XK_exclamdown,	XK_guillemotleft,XK_guillemotright,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	NoSymbol,	NoSymbol,	NoSymbol,	NoSymbol,
	XK_Greek_alpha,	XK_ssharp,	XK_Greek_GAMMA,	XK_Greek_pi,
	XK_Greek_SIGMA,	XK_Greek_sigma,	XK_mu,	        XK_Greek_tau,
	XK_Greek_PHI,	XK_Greek_THETA,	XK_Greek_OMEGA,	XK_Greek_delta,
	XK_infinity,	XK_Ooblique,	XK_Greek_epsilon, XK_intersection,
	XK_identical,	XK_plusminus,	XK_greaterthanequal, XK_lessthanequal,
	XK_topintegral,	XK_botintegral,	XK_division,	XK_similarequal,
	XK_degree,	NoSymbol,	NoSymbol,	XK_radical,
	XK_Greek_eta,	XK_twosuperior,	XK_periodcentered, NoSymbol,
      };

/*
 * LegalModifier --
 *      determine whether a key is a legal modifier key, i.e send a
 *      press/release sequence.
 */

/*ARGSUSED*/
Bool
LegalModifier(key)
     int  key;
{
  return (TRUE);
}


      
/*
 * x386KbdGetMapping --
 *	Get the national keyboard mapping. The keyboard type is set, a new map
 *      and the modifiermap is computed.
 */

void
x386KbdGetMapping (pKeySyms, pModMap)
     KeySymsPtr pKeySyms;
     CARD8      *pModMap;
{
  KeySym        *k;
  struct	kbentry kbe;
  char          type;
  int           i;
  
  for (i = 0; i < NUMKEYS; i++)
    {
      static int states[] = { NORM_STATE, SHIFT_STATE, ALT_STATE, SHIFT_ALT };
      int j;

      k = &map[i*4];
      kbe.kb_index = i;
	
      for (j = 0; j < 4; j++)
	{
	  kbe.kb_state = states[j];

	  if (ioctl (x386Info.consoleFd, KDGKBENT, &kbe) != -1)
	    continue;
      
	  if (kbe.kb_value [0] == K_SCAN)
	    {
	      int keycode = -1;
	      switch (kbe.kb_value [1])
		{
		case K_CTLSC: keycode = XK_Control_L; break;
		case K_LSHSC: keycode = XK_Shift_L; break;
		case K_RSHSC: keycode = XK_Shift_R; break;
		case K_ALTSC: keycode = XK_Alt_L; break;
		case K_CLCKSC: keycode = XK_Caps_Lock; break;
		case K_NLCKSC: keycode = XK_Num_Lock; break;
		default: break;
		}
	      if (keycode > 0)
		k[j] = keycode;
	    }
	  else if (kbe.kb_value[1] != NC)
	    {
	      /* How to handle multiple characters?
	         Ignore them for now. */
	    }
	  else
	    {
	      k[j] = ascii_to_x[kbe.kb_value[0]];
	    }
	}
    }

  /*
   * compute the modifier map
   */
  for (i = 0; i < MAP_LENGTH; i++)
    pModMap[i] = NoSymbol;  /* make sure it is restored */
  
  for (k = map, i = MIN_KEYCODE;
       i < (NUM_KEYCODES + MIN_KEYCODE);
       i++, k += 4)
    
    switch(*k) {
      
    case XK_Shift_L:
    case XK_Shift_R:
      pModMap[i] = ShiftMask;
      break;
      
    case XK_Control_L:
    case XK_Control_R:
      pModMap[i] = ControlMask;
      break;
      
    case XK_Caps_Lock:
      pModMap[i] = LockMask;
      break;
      
    case XK_Alt_L:
      pModMap[i] = AltMask;
      if (k[1] == NoSymbol)
        k[1] = XK_Meta_L;
      break;
    case XK_Alt_R:
      pModMap[i] = AltMask;
      if (k[1] == NoSymbol)
        k[1] = XK_Meta_R;
      break;
      
    case XK_Num_Lock:
      if (!x386Info.serverNumLock) pModMap[i] = NumLockMask;
      break;

    case XK_Scroll_Lock:
      pModMap[i] = ScrollLockMask;
      break;

      /* kana support */
    case XK_Kana_Lock:
    case XK_Kana_Shift:
      pModMap[i] = KanaMask;
      break;

      /* alternate troggle for multinational support */
    case XK_Mode_switch:
      pModMap[i] = AltLangMask;
      break;

    }
  
  x386Info.kbdType =
    ioctl(x386Info.kbdFd, KDGKBDTYPE, &type) != -1 ? type : KB_VANILLAKB;

  pKeySyms->map        = map;
  pKeySyms->mapWidth   = GLYPHS_PER_KEY;
  pKeySyms->minKeyCode = MIN_KEYCODE;
  pKeySyms->maxKeyCode = MAX_KEYCODE;
}
