/*
    This file is part of The Didactic PDP-8 Assembler
    Copyright (C) 2002 Toby Thain, toby@telegraphics.com.au

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

#define WORDMASK 07777 /* words are 12 bits */

/* instruction bit masks */
enum{
	OPCODE = 07000,
	OPCODE_IOT = 06000,
	OPCODE_OPR = 07000,
	/* microcoded instructions */
	GROUP_BIT = 0400,
	REVERSE_SENSE = 010,
	ROTATES = 014, /* group 1 rotates */
	SKIPS = 0160, /* group 2 skips */
	/* memory reference instructions */
	PAGE_ADDR = 0177,
	CURR_PAGE = 0200,
	INDIRECT_MODE = 0400,
	/* IOT microinstructions */
	DEVICE_SELECT = 0770
};

enum{ /* symbol flags */
	F_MRI=0400 /* symbol represents a memory-reference instruction */
};
enum{ /* addressing modifier flags */
	F_INDIRECT = 01,
	F_ZEROPAGE = 02
};
