/* FriBidi
 * fribidi-char-sets-cap-rtl.c - CapRTL character set conversion routines
 *
 * $Id: fribidi-char-sets-cap-rtl.c,v 1.1 2004-04-25 18:47:57 behdad Exp $
 * $Author: behdad $
 * $Date: 2004-04-25 18:47:57 $
 * $Revision: 1.1 $
 * $Source: /home/behdad/src/fdo/fribidi/togit/git/../fribidi/fribidi2/charset/fribidi-char-sets-cap-rtl.c,v $
 *
 * Authors:
 *   Behdad Esfahbod, 2001, 2002, 2004
 *   Dov Grobgeld, 1999, 2000
 *
 * Copyright (C) 2004 Sharif FarsiWeb, Inc
 * Copyright (C) 2001,2002 Behdad Esfahbod
 * Copyright (C) 1999,2000 Dov Grobgeld
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library, in a file named COPYING; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA
 * 
 * For licensing issues, contact <license@farsiweb.info>.
 */

#include <fribidi-char-sets-cap-rtl.h>

#include <fribidi-unicode.h>
#include <fribidi-mirroring.h>
#include <fribidi-bidi-type.h>

#include <common.h>
#include <bidi-types.h>

#include <stdio.h>

#define WS FRIBIDI_PROP_TYPE_WS
#define BS FRIBIDI_PROP_TYPE_BS
#define EO FRIBIDI_PROP_TYPE_EO
#define CTL FRIBIDI_PROP_TYPE_CTL
#define LRE FRIBIDI_PROP_TYPE_LRE
#define RLE FRIBIDI_PROP_TYPE_RLE
#define ES FRIBIDI_PROP_TYPE_ES
#define LRO FRIBIDI_PROP_TYPE_LRO
#define RLO FRIBIDI_PROP_TYPE_RLO
#define AL FRIBIDI_PROP_TYPE_AL
#define SS FRIBIDI_PROP_TYPE_SS
#define ET FRIBIDI_PROP_TYPE_ET
#define NSM FRIBIDI_PROP_TYPE_NSM
#define LTR FRIBIDI_PROP_TYPE_LTR
#define ON FRIBIDI_PROP_TYPE_ON
#define AN FRIBIDI_PROP_TYPE_AN
#define BN FRIBIDI_PROP_TYPE_BN
#define RTL FRIBIDI_PROP_TYPE_RTL
#define CS FRIBIDI_PROP_TYPE_CS
#define PDF FRIBIDI_PROP_TYPE_PDF
#define EN FRIBIDI_PROP_TYPE_EN

static FriBidiPropCharType CapRTLCharTypes[] = {
/* *INDENT-OFF* */
  ON, ON, ON, ON, LTR,RTL,ON, ON, ON, ON, ON, ON, ON, BS, RLO,RLE, /* 00-0f */
  LRO,LRE,PDF,WS, ON, ON, ON, ON, ON, ON, ON, ON, ON, ON, ON, ON,  /* 10-1f */
  WS, ON, ON, ON, ET, ON, ON, ON, ON, ON, ON, ET, CS, ON, ES, ES,  /* 20-2f */
  EN, EN, EN, EN, EN, EN, AN, AN, AN, AN, CS, ON, ON, ON, ON, ON,  /* 30-3f */
  RTL,AL, AL, AL, AL, AL, AL, RTL,RTL,RTL,RTL,RTL,RTL,RTL,RTL,RTL, /* 40-4f */
  RTL,RTL,RTL,RTL,RTL,RTL,RTL,RTL,RTL,RTL,RTL,ON, BS, ON, ON, ON,  /* 50-5f */
  NSM,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR, /* 60-6f */
  LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,LTR,ON, SS, ON, WS, ON,  /* 70-7f */
/* *INDENT-ON* */
};

#undef WS
#undef BS
#undef EO
#undef CTL
#undef LRE
#undef RLE
#undef ES
#undef LRO
#undef RLO
#undef AL
#undef SS
#undef ET
#undef NSM
#undef LTR
#undef ON
#undef AN
#undef BN
#undef RTL
#undef CS
#undef PDF
#undef EN

#define CAPRTL_CHARS (sizeof CapRTLCharTypes / sizeof CapRTLCharTypes[0])

static FriBidiChar *caprtl_to_unicode = NULL;

static void
init_cap_rtl (
  void
)
{
  int request[_FRIBIDI_PROP_TYPES_COUNT + 1];
  int i, count;

  caprtl_to_unicode =
    (FriBidiChar *) fribidi_malloc (CAPRTL_CHARS *
				    sizeof caprtl_to_unicode[0]);
  for (i = 0; i < _FRIBIDI_PROP_TYPES_COUNT; i++)
    request[i] = 0;
  for (i = 0; i < CAPRTL_CHARS; i++)
    if (fribidi_get_mirror_char (i, NULL))
      caprtl_to_unicode[i] = i;
  for (count = 0, i = 0; i < CAPRTL_CHARS; i++)
    if (caprtl_to_unicode[i] == 0)
      {
	request[(unsigned char) CapRTLCharTypes[i]]++;
	count++;
      }
  for (i = 1; i < 0x10000 && count; i++)	/* Assign BMP chars to CapRTL entries */
    if (!fribidi_get_mirror_char (i, NULL))
      {
	int j, k;
	for (j = 0; j < _FRIBIDI_PROP_TYPES_COUNT; j++)
	  if (fribidi_prop_to_type_[j] == fribidi_get_bidi_type (i))
	    break;
	if (!request[j])	/* Do not need this type */
	  continue;
	for (k = 0; k < CAPRTL_CHARS; k++)
	  if (!caprtl_to_unicode[k] && j == CapRTLCharTypes[k])
	    break;
	if (k < CAPRTL_CHARS)
	  {
	    request[j]--;
	    count--;
	    caprtl_to_unicode[k] = i;
	  }
      }
}

static char
fribidi_unicode_to_cap_rtl_c (
  /* input */
  FriBidiChar uch
)
{
  int i;

  if (!caprtl_to_unicode)
    init_cap_rtl ();

  for (i = 0; i < CAPRTL_CHARS; i++)
    if (uch == caprtl_to_unicode[i])
      return (char) i;
  return '?';
}

FriBidiStrIndex
fribidi_cap_rtl_to_unicode (
  /* input */
  const char *s,
  FriBidiStrIndex len,
  /* output */
  FriBidiChar *us
)
{
  FriBidiStrIndex i, j;

  if (!caprtl_to_unicode)
    init_cap_rtl ();

  j = 0;
  for (i = 0; i < len; i++)
    {
      char ch;

      ch = s[i];
      if (ch == '_')
	{
	  switch (ch = s[++i])
	    {
	    case '>':
	      us[j++] = FRIBIDI_CHAR_LRM;
	      break;
	    case '<':
	      us[j++] = FRIBIDI_CHAR_RLM;
	      break;
	    case 'l':
	      us[j++] = FRIBIDI_CHAR_LRE;
	      break;
	    case 'r':
	      us[j++] = FRIBIDI_CHAR_RLE;
	      break;
	    case 'o':
	      us[j++] = FRIBIDI_CHAR_PDF;
	      break;
	    case 'L':
	      us[j++] = FRIBIDI_CHAR_LRO;
	      break;
	    case 'R':
	      us[j++] = FRIBIDI_CHAR_RLO;
	      break;
	    case '_':
	      us[j++] = '_';
	      break;
	    default:
	      us[j++] = '_';
	      i--;
	      break;
	    }
	}
      else
	us[j++] = caprtl_to_unicode[(int) s[i]];
    }

  return j;
}

FriBidiStrIndex
fribidi_unicode_to_cap_rtl (
  /* input */
  const FriBidiChar *us,
  int len,
  /* output */
  char *s
)
{
  int i, j;

  j = 0;
  for (i = 0; i < len; i++)
    {
      FriBidiChar ch = us[i];
      if (!FRIBIDI_IS_EXPLICIT (fribidi_get_type (ch)) && ch != '_'
	  && ch != FRIBIDI_CHAR_LRM && ch != FRIBIDI_CHAR_RLM)
	s[j++] = fribidi_unicode_to_cap_rtl_c (ch);
      else
	{
	  s[j++] = '_';
	  switch (ch)
	    {
	    case FRIBIDI_CHAR_LRM:
	      s[j++] = '>';
	      break;
	    case FRIBIDI_CHAR_RLM:
	      s[j++] = '<';
	      break;
	    case FRIBIDI_CHAR_LRE:
	      s[j++] = 'l';
	      break;
	    case FRIBIDI_CHAR_RLE:
	      s[j++] = 'r';
	      break;
	    case FRIBIDI_CHAR_PDF:
	      s[j++] = 'o';
	      break;
	    case FRIBIDI_CHAR_LRO:
	      s[j++] = 'L';
	      break;
	    case FRIBIDI_CHAR_RLO:
	      s[j++] = 'R';
	      break;
	    case '_':
	      s[j++] = '_';
	      break;
	    default:
	      j--;
	      if (ch < 256)
		s[j++] = fribidi_unicode_to_cap_rtl_c (ch);
	      else
		s[j++] = '?';
	      break;
	    }
	}
    }
  s[j] = 0;

  return j;
}

const char *
fribidi_char_set_desc_cap_rtl (
  void
)
{
  static char *s = 0;
  int l, i, j;

  if (s)
    return s;

  l = 10000;
  s = (char *) fribidi_malloc (l);
  i = 0;
  i += sprintf (s + i,		/*l - i, */
		"CapRTL is a character set for testing with the reference\n"
		"implementation, with explicit marks escape strings, and\n"
		"the property that contains all unicode character types in\n"
		"ASCII range 1-127.\n"
		"\n"
		"Warning: CapRTL character types are subject to change.\n"
		"\n" "CapRTL's character types:\n");
  for (j = 0; j < CAPRTL_CHARS; j++)
    {
      if (j % 4 == 0)
	s[i++] = '\n';
      i += sprintf (s + i, /*l - i, */ "  * 0x%02x %c%c %-3s ", j,
		    j < 0x20 ? '^' : ' ',
		    j < 0x20 ? j + '@' : j < 0x7f ? j : ' ',
		    fribidi_type_name (fribidi_prop_to_type_
				       [(unsigned char) CapRTLCharTypes[j]]));
    }
  i += sprintf (s + i,		/*l - i, */
		"\n\n"
		"Escape sequences:\n"
		"  Character `_' is used to escape explicit marks. The list is:\n"
		"    * _>  LRM\n" "    * _<  RLM\n"
		"    * _l  LRE\n" "    * _r  RLE\n"
		"    * _L  LRO\n" "    * _R  RLO\n"
		"    * _o  PDF\n" "    * __  `_' itself\n" "\n");
  return s;
}

/* Editor directions:
 * vim:textwidth=78:tabstop=8:shiftwidth=2:autoindent:cindent
 */