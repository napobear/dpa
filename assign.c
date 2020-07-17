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

void doassign(struct sym_rec *p,int v,int tok,int relmode,int lineno){
	if( pass==1 && (p->flags & F_ASSIGNED) )
		warn("symbol redefined");
	p->value = v;
	p->token = tok;
	p->flags |= F_ASSIGNED;
	p->pageno = pageno;
	p->lineno = lineno;
	p->relmode = relmode;
	DPRINTF("  \"%s\" <- %#o (relmode=%o)\n",p->name,p->value,relmode);
}
