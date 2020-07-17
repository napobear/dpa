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

#include  "asm.h"
#include  "nova.h"

int wordmask = WORDMASK;
extern int cond,relmode; /* relocation mode in effect: NREL/ZREL/ABS */

int ea(int index,int indexseen,struct operand *op){
	int disp = op->value;
	if(!indexseen){
		/* the addressing mode is implicit; we decide:
			- If the address is absolute and fits in 8 bits, 
			  the displacement field is set to address [mode 0];
			- If the address is page zero relocatable (that is,
			  assembled with the .ZREL pseudo-op),
			  the displacement field is set to <address> with 
			  page zero relocation;
			- If the address is an external displacement (that is,
			  assembled with the .EXTD pseudo-op), the displacement
			  is set to the assembler .EXTD number  */
		if( (op->relmode == ABSOLUTE && op->value == (op->value & 0377))
		 || (op->relmode == PAGE_ZERO_REL) ){
			index = 0; 
		}else if(op->relmode == EXT_DISP)
			fatal("EXT_DISP relocation mode not yet supported");
		else{
			disp -= currentloc();
			index = 1; /* try relative */
			op->relmode = ABSOLUTE;
		}
	}
	if( cond && pass==2 && (index ? (disp<-128 || disp>127) : (disp<0 || disp>255)) )
		fatal("displacement out of range");
	return (index<<8) | (disp&0377); 
}
