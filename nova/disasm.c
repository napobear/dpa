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
#include "nova.h"

extern char *class_sh[],*class_c[],*class_f[],
	 *twoac_op[8],*io_op[8],*noac_op[4],*oneac_op[4],*skips[8];

char *verbose_heading = " address |  word  |rel|  type |      instruction layout       |  effective addr";

void disasm(char *s,int word){
						   /* no / 1ac/ 2ac / io */
	int f1 = (word>>13)&3, /* 00 / op / acs / 11 */
		f2 = (word>>11)&3, /* op / ac / acd / ac */
		f3 = (word>>8)&7;  /* @i / @i / op  / op */
	if(word>>15){
		/* two accumulator - multiple operation */
		/*			          | | | | | | | | | | | | | | | | | */
		sprintf(s,"2AC: |1| %o | %o | %s | %1s | %1s |%c| %s |",
			f1,f2,twoac_op[f3],
			class_sh[(word>>6)&3],class_c[(word>>4)&3],word&TWOAC_NOLOAD?'#':' ',skips[word&7]);
	}else{
		if(f1 == 3) /* input/output */
			sprintf(s,"I/O: |0 1 1| %o | %s | %1s |    %02o     |",
				f2,io_op[f3],class_f[(word>>6)&3],word&077);
		else{ /* one accumulator - effective address */	
			char ind[10],eastr[40],sign = ' ',
				 *type[]={"pg0","rel","+AC2","+AC3"};
			int ea,disp = word & 0377,
				signeddisp = (disp>127 ? disp-256 : disp),
				index = f3&3;
			*ind = 0;
			if(index==0)
				ea = disp; /* zero page */
			else if(index==1)
				ea = currentloc() + signeddisp; /* relative */
			else{
				ea = signeddisp;
				sign = signeddisp<0 ? '-' : '+';
				//sprintf(ind,"AC%c",index+'0');
			}
			sprintf(eastr,"|%c| %o |      %03o      | E=%c%c%#6o %s",
				word&NOAC_INDIRECT?'@':' ',index,disp,
				word&NOAC_INDIRECT?'@':' ',sign,abs(ea),type[index]);
			if(f1)
				sprintf(s,"1AC: |0|%s| %o %s",oneac_op[f1],f2,eastr);
			else
				sprintf(s,"0AC: |0 0 0|%s%s",noac_op[f2],eastr);
		}
	}
}
