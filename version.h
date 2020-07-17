/*
    This file is part of The Didactic PDP-8 Assembler
    Copyright (C) 2002-7 Toby Thain, toby@telegraphics.com.au

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by  
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License  
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* version history
v1.0, 4-Sep-2002
 01-Jul-2003: 1.4b1 - Nova, PDP-8 targets
 02-          1.4b2 - Makefile fixes
		    - Nova: now parses a lone label as a statement
 26-Nov-2003: 1.5 - produce conventional assembly listing file
 02-Dec-2003: 1.6 - fix Nova TXT  pseudo-ops (lexer,parser)
 05-Dec-2003: 1.7 - finish TXT; add -s for 'header' files; other changes
 07-Dec-2003: 1.8 - add proper relocation mode handling; basic RB file output; fix implicit EA modes
 08-Dec-2003: 1.9 - Makefile cleanups
 10-Dec-2003: 1.91 - finally fix B operator, now lexed correctly in all cases 
 14-Dec-2003: 1.92 - add relocation flags to listing
 19-Dec-2003: 1.93 - implement conditional assembly
 19-Dec-2003: 1.94 - fix .RDX; check for invalid Nova 3 2AC instructions
 07-Jan-2004: 1.95 - fix failure to increment curloc on pass 1 (Nova)
 16-May-2004: 1.95r2 - fix missing files (pdp8/list.c, nova/list.c) in dist :( !
 25-Apr-2005: 1.96 - add Nova bootstrap format output (per Emil Sarlija's suggestion)
 01-May-2005: 1.97 - ignore directives in bootstrap(!); also fix many compiler warnings
 03-May-2005: 1.98 - fix possible overflow crash in listing code
 07-May-2005: 1.99 - minor cleanups, most importantly, always clean symbols after each file
 09-May-2007: 1.991 - fix some warnings
 06-Jun-2007: 1.992 - use suffix .boot for bootstrap binary output;
                      fix simple <N> and <"C> in strings (octal/character literal)
*/
#define VERS_STR "1.992"
