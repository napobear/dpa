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

char target_arch[] = "DEC PDP-8";
int rbformat = 0;

extern char *iot[0100][3];

void cpu_initsyms(void){
	struct sym_rec *p;

	for(p = predefs; p->name; p++){
		/* setup disassembly table for IOT instruction */
		if((p->value & OPCODE) == OPCODE_IOT){
			int devsel = (p->value & DEVICE_SELECT) >> 3;
			if((p->value & 07) == 04)
				iot[devsel][0] = p->name;
			if((p->value & 07) == 02)
				iot[devsel][1] = p->name;
			if((p->value & 07) == 01)
				iot[devsel][2] = p->name;
		}
	}
}
