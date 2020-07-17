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

#include "rb.h"

char UNDEF[] = "???";
char *rb_relflag_types[]={
	"Illegal",
	"Absolute",
	"Normal Relocatable",
	"Normal Byte Relocatable",
	"Page Zero Relocatable",
	"Page Zero Byte Relocatable",
	"Data Reference External Displacement",
	"Illegal"
};
char *rb_relflag_short[]={"ILL","ABS"," N "," NB","Z  ","Z B","EXT","ILL"};
char rb_relflag_chars[] = "? '\"-=??";
char *rb_block_types[]={
	UNDEF,UNDEF,
	"Relocatable Data",
	"Entry",
	"External Displacement",
	"Normal External",
	"Start",
	"Title",
	"Local Symbol",
	"Library Start",
	"Library End",
	"Labeled Common",
	"Global Addition",
	"Unlabeled Common Size",
	"Global Location Start",
	"Global Location End",
	"Global Reference"
};
char *rb_block_short[]={
	UNDEF, UNDEF,
	"REL_DATA",
	"ENT",
	"EXTD",
	"EXTN",
	"START",
	"TITL",
	"LOCAL_SYM",
	"LIB_START",
	"LIB_END",
	"COMM",
	"GADD",
	"CSIZ",
	"GLOC_START",
	"GLOC_END",
	"GREF"
};
char *rb_sym_types[]={
	/*000*/	"Entry Symbol",
		"Normal External Symbol",
		"Labeled Common",
		"External Displacement Symbol",
		"Overlay Symbol",
		UNDEF,UNDEF,UNDEF,
	/*010*/ "Local Symbol",
		UNDEF,UNDEF,UNDEF,UNDEF,UNDEF,UNDEF,UNDEF,
	/*020*/	UNDEF,UNDEF,UNDEF,UNDEF,
	/*024*/	"Title Symbol"
};
char *rb_sym_typecodes[]={
	/* these codes appear in cross reference listing */
	/*000*/	"EN",
		"XN",
		"NC",
		"XD",
		"EO",
};

RB_WORD rb_checksum;

RB_WORD rb_getword(FILE *f){
	RB_WORD w = fgetc(f);
	w |= (fgetc(f) << 8);
	rb_checksum += w;
	return w;
}
int rb_getwords(FILE *f,RB_WORD w[],int n){
	while(n-- && !feof(f))
		*w++ = rb_getword(f);
	return !feof(f);
}

RB_WORD rb_getheader(FILE *f,struct rb_block_header *h){
	rb_checksum = 0;
	h->type = rb_getword(f);
	h->word_count = rb_getword(f);
	h->rel_flags[0] = rb_getword(f);
	h->rel_flags[1] = rb_getword(f);
	h->rel_flags[2] = rb_getword(f);
	h->checksum = rb_getword(f);
	return h->type;
}

int rb_putword(FILE *f,RB_WORD w){
	return fputc(w,f) != EOF && fputc(w >> 8,f) != EOF;
}
int rb_putwords(FILE *f,RB_WORD w[],int n){
	while( n-- && rb_putword(f,*w++) )
		;
	return !n;
}
int getrelflag(RB_WORD h[],int i){
	//printf("relflag(%d): word %d, shift %d\n",i,i/5,13-3*(i%5) );
	return ( h[RB_RELFLAGS0 + i/5] >> (13-3*(i%5)) ) & 7 ;
}
void setrelflag(RB_WORD h[],int i,int m){
	h[RB_RELFLAGS0 + i/5] |= (m & 7) << (13-3*(i%5));
}
