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

char *opcodes[]={"AND","TAD","ISZ","DCA","JMS","JMP","IOT","OPR"},
     *group1[]={"CLA","CLL","CMA","CML","RAR","RAL","x2 ","IAC"},
     *group2_0[]={"CLA","SMA","SZA","SNL","---","OSR","HLT","???"},
     *group2_1[]={"CLA","SPA","SNA","SZL","---","OSR","HLT","???"},
     *iot[0100][3];

char *verbose_heading = " address |  word   | opcode         instruction layout         | effective addr";

struct iotdev_rec{
	int devsel;
	char *name;
} iotdevices[]={
	{000,"Program Interrupt"},
	{001,"High Speed Punch"},
	{002,"High Speed Reader"},
	{003,"Keyboard/Reader"},
	{004,"Teleprinter/Punch"},
	{005,"Oscilloscope/CRT"},
	{006,"Oscilloscope/CRT"},
	{007,"Oscilloscope/CRT"},
	{010,"Memory Parity"},
	{020,"Memory Extension"},
	{040,"Data Comms"},
	{050,"Plotter"},
	{051,"Plotter"},
	{052,"Plotter"},
	{053,"Converter/Mux/DVM"},
	{054,"Converter/Mux/DVM"},
	{057,"DVM"},
	{060,"Drum/Disk"},
	{061,"Drum/Disk"},
	{062,"Drum/Disk"},
	{063,"Card Reader"},
	{065,"Line Printer"},
	{066,"Line Printer"},
	{067,"Card Reader"},
	{070,"Mag. Tape"},
	{071,"Mag. Tape"},
	{072,"Mag. Tape"},
	{076,"DECtape"},
	{077,"DECtape"},
	{0,NULL}
	};


void microinst(char *s,int word,char *codes[]){
	int mask,i;
	char *q = endof(s);
	for(mask=0200,i=0;mask;mask>>=1,++i){
		*q++ = '|';
		q = scpy(q,word & mask ? codes[i] : "   ");
	}
	*q++ = '|';
	*q = 0;
}

void disasm(char *s,int word){
	int op = word >> 9,ea,devsel;
	struct iotdev_rec *p;
	char *q,*devstr,t[20],**grp;

	sprintf(t,"|  %s  |",opcodes[op]);
	
	if(op<6){ /* memory reference instruction */

		/* compute effective address */
		ea = word & PAGE_ADDR;
		if(word & CURR_PAGE)
			ea |= curloc & ~PAGE_ADDR;

		/*         |---|---|---|---|---|---|---|---|---|---|---|---| */
		sprintf(s,"%s %c | %c |            %03o            | (addr=%04o)",
			t,
			word & INDIRECT_MODE ? 'I' : ' ',
			word & CURR_PAGE ? '1' : 'Z',
			word & PAGE_ADDR,
			ea );
	}
	else if(op==6){
		devsel = (word & DEVICE_SELECT)>>3;
		for(p=iotdevices,devstr=0;p->name;p++)
			if(p->devsel == devsel)
				devstr = p->name;
		/*         |---|---|---|---|---|---|---|---|---|---|---|---| */
		sprintf(s,"%s%-19s(%02o)|%s|%s|%s|",
			t,devstr?devstr:"",
			devsel,
			word & 04 ? (iot[devsel][0]?iot[devsel][0]:" 1 ") : "   ",
			word & 02 ? (iot[devsel][1]?iot[devsel][1]:" 1 ") : "   ",
			word & 01 ? (iot[devsel][2]?iot[devsel][2]:" 1 ") : "   ");
	}
	else if(op==7){
		q = scpy(s,t);
		if(word & GROUP_BIT){ /* group 2 microinstruction */
			scpy(q," 1 ");
			/* allow for "reverse sense" bit */
			if(word & REVERSE_SENSE){
				/* special case for unconditional skip */
				group2_1[4] = word & SKIPS ? " 1 " : "SKP";
				grp = group2_1;
			}else 
				grp = group2_0;
		}else{ /* group 1 microinstruction */
			scpy(q," 0 ");
			grp = group1;
		}
		microinst(s,word,grp);
	}
}
