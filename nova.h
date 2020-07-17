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

#include "nova/rb.h"

#define WORDMASK 0177777 /* words are 16 bits */

/* instruction bit masks */
enum{
	INDIR_DATA = 0100000,
	OPCODE = 0160000,
/* no accumulator - effective address */
	NOAC_OPCODE = 014000,
	NOAC_INDIRECT = 02000,
	NOAC_INDEX = 01400,
	NOAC_DISP = 0377,
/* one accumulator - effective address */
	ONEAC_OPCODE = 060000,
	ONEAC_AC = 014000,
	TWOAC_NOLOAD = 010,

	DEVICE_SELECT = 077,

	MAXCONDDEPTH = 20
};

enum{ /* symbol flags */
	F_NOAC  = 0100,
	F_ONEAC = 0200,
	F_TWOAC = 0400,
	F_IO   = 01000,
	F_TRAP = 02000,
	F_SKIP = 04000
};

struct operand{
	int value,relmode;
};

void assemblerb(int w,int relmode);
int ea(int index,int indexseen,struct operand *op);
int currentloc(void);
void setcurrentloc(int);

void rbtitle(struct sym_rec *s);
void rbsymlist(RB_WORD type,int symtype,struct sym_rec *symlist[],int nsyms);
void rbexpr(RB_WORD type,int w,int m);
void rbcomm(struct sym_rec *s,int w,int m);
void rbgadd(RB_WORD type,struct sym_rec *s,int w,int m);
