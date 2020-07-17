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

extern int relmode,noloc;

//efine LISTFORMAT "LL CCCCCRDDDDDDR....."
char listfmt[] = "%02d %05o%c%06o%c%s\n",
     listfmt_noloc[] = "%02d       %06o%c%s\n",
     listfmt_empty[] = "%02d              %s\n";

void listo(int a,int w,int m){
	if(listpos){
		if(listfile) fprintf(listfile,listfmt,yylineno,a,rb_relflag_chars[relmode],
							 w,rb_relflag_chars[m],listline);
		newline();
	}else if(!noloc && listfile) 
		fprintf(listfile,listfmt_noloc,yylineno,w,rb_relflag_chars[m],listline);
}
void printsym(struct sym_rec *p){
	extern int maxlen;
	if(listfile) fprintf(listfile,"%-*s %06o%c %s    %d/%02d\n",
		   maxlen,p->name,p->value & WORDMASK,rb_relflag_chars[p->relmode],
		   p->type == USER_SYMBOL ? "  " : rb_sym_typecodes[p->type],
		   p->pageno,p->lineno);
}

void listempty(char *line){
	if(listfile) fprintf(listfile,listfmt_empty,yylineno,line);
}
