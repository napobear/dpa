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

#include "asm.h"
#include "pdp8.h"

int wordmask = WORDMASK;

int mri(int opcode,int mod,int addr){
	int page = addr & ~PAGE_ADDR,
	    curpage = curloc & ~PAGE_ADDR;
	DPRINTF("MRI(%#o,%s%saddr=%#o) page=%#o curpage=%#o\n",
		opcode,
		mod & F_INDIRECT ? "indirect," : "",
		mod & F_ZEROPAGE ? "zero page," : "",
		addr,page,curpage);
	if(page != 0){
		if(mod & F_ZEROPAGE) 
			warn("conflicting use of zero page modifier?");
		opcode |= CURR_PAGE;
		if(page != curpage) 
			warn("illegal out-of-page reference");
	}
	if(mod & F_INDIRECT)
		opcode |= INDIRECT_MODE;
	return opcode | (addr & PAGE_ADDR);
}

#define ISCLA(op) ((op & ~GROUP_BIT) == 07200)
#define ISSKP(op) ((op & (SKIPS|REVERSE_SENSE)) == REVERSE_SENSE)
#define DIFFGROUPS(op1,op2) ((op1 ^ op2) & GROUP_BIT)

int combine(int op1,int op2){ /* combine non-memory reference instructions */
	int combop = op1|op2;
	if((op1&OPCODE)==OPCODE_OPR && (op2&OPCODE)==OPCODE_OPR){
		/* are opcodes from different groups? */
		if(DIFFGROUPS(op1,op2)){
			/* special case CLA, it is in both groups
			   so can always be combined */
			if(ISCLA(op1)) 
				op1 ^= GROUP_BIT;
			else if(ISCLA(op2)) 
				op2 ^= GROUP_BIT;
			else
				fatal("can't combine microinstructions from different groups");
		}
		if(!DIFFGROUPS(op1,op2)){
			if(op1 & GROUP_BIT){
				/* group 2: check for incompatible skips: */
				if( ( (op1 & SKIPS) && (op2 & SKIPS) /* both conditional */
				  && ((op1^op2) & REVERSE_SENSE) ) /* and different senses */
				|| ( (combop & SKIPS) /* or, either is conditional */
				     && (ISSKP(op1) || ISSKP(op2)) ) ) /* and one is unconditional */
				fatal("can't combine these skips");
			}else{
				/* group 1: are both rotates? */
				if( (combop & ROTATES) == ROTATES )
					fatal("can't combine rotates");
				else if( (combop & ROTATES) && (combop & 01) )
					fatal("can't combine rotate with IAC");
			}
		}
	}else if((op1&OPCODE)==OPCODE_IOT && (op2&OPCODE)==OPCODE_IOT){
		if( (op1^op2) & DEVICE_SELECT )
			fatal("can't combine I/O transfers on different devices");
	}else
		fatal("can't combine memory-reference instructions");
	return combop;
}
