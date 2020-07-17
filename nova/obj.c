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

#define MAX_BOOTSTRAP 192  /* bootstrap program size limit (words);
                              see Nova Prog. Ref., page VI-7 */

extern int wordmask,listing,bootprog; // full word bit mask for target architecture
int words,cksum,rimflag = 0,leader = 0,bootwords;

/* there is one location counter for each of the three
   relocation modes: Absolute (curloc), Normal Relocatable (nrel_loc)
   and Zero Page Relocatable (zrel_loc) */
int relmode, /* relocation mode currently in effect */
	curloc = 0,nrel_loc = 0,zrel_loc = 0;

extern FILE *listfile;

char *objsuffix = ".rb";
static RB_WORD rb_block[ RB_HEADER_WORDS + 3*15 ];
static int rb_count = 0,rb_blocks = 0;

void initcurloc(){
	if(bootprog){
		curloc = nrel_loc = 0100;
		zrel_loc = 0;
		relmode = ABSOLUTE;
	}else
		curloc = nrel_loc = zrel_loc = 0;
}

int currentloc(){
	switch(relmode){
	case ABSOLUTE: return curloc; 
	case NORMAL_REL: return nrel_loc; 
	case PAGE_ZERO_REL: return zrel_loc; 
	}
	DPRINTF("bad relmode [currentloc()]");
	return 0;
}

void objheader(){
	int i;

	if(bootprog){
		for(i=15;i--;) 
			fputc(0,obj); /* 7 words & 1 byte of nominal leader */
		fputc(0377,obj); /* non-zero "synchronisation byte" */
		bootwords = words+1;
		if(bootwords > MAX_BOOTSTRAP){
			warn("bootstrap is longer than 192 words; excess ignored");
			bootwords = MAX_BOOTSTRAP;
		}
		/* first word is word count including this one: */
		fputc((-bootwords)>>8,obj); fputc(-bootwords,obj);
		--bootwords;
	}else
		rb_blocks = rb_count = 0;
}

void objfooter(){
	if(!bootprog)
		flushrb();
}

void rb_putblock(RB_WORD type,RB_WORD b[],int n){
	int i,c;

	b[ RB_TYPE ] = type;
	b[ RB_WORDCOUNT ] = -n; /* must be negative! */
	b[ RB_CHECKSUM ] = 0;
	for( i = c = 0 ; i < n+RB_HEADER_WORDS ; ++i )
		c += b[i];
	b[ RB_CHECKSUM ] = -c;
	rb_putwords(obj,b,n+RB_HEADER_WORDS);
	rb_count = 0;
	++rb_blocks;
}

void startrb(){
	rb_block[ RB_RELFLAGS0 ] = 
	rb_block[ RB_RELFLAGS1 ] = 
	rb_block[ RB_RELFLAGS2 ] = 0;
	rb_block[ RB_HEADER_WORDS ] = currentloc();
	setrelflag(rb_block,0,relmode);
	rb_count = 1;
}

void flushrb(){
	if(bootprog) DPUTS("!!! flushrb() called in bootstrap");
	if(pass==2 && rb_count){
		DPUTS("# flushing Relocatable Data block...");
		rb_putblock(REL_DATA_BLK,rb_block,rb_count);
	}
}

void rbtitle(struct sym_rec *s){
	if(pass==2){
		flushrb();
		if(rb_blocks)
			warn(".TITL directive must precede assembly program");
		else{
			rb_block[ RB_RELFLAGS0 ] = 
			rb_block[ RB_RELFLAGS1 ] = 
			rb_block[ RB_RELFLAGS2 ] = 0;
			to_radix50(s->name,rb_block+RB_HEADER_WORDS,TITLE_SYM);
			rb_block[ RB_HEADER_WORDS+2 ] = 0; /* equivalence value */
			rb_putblock(TITL_BLK,rb_block,3);
		}
	}
}

void rbsymlist(RB_WORD type,int symtype,struct sym_rec *symlist[],int nsyms){
	int i;
	if(pass==2){
		flushrb();
		rb_block[ RB_RELFLAGS0 ] = 
		rb_block[ RB_RELFLAGS1 ] = 
		rb_block[ RB_RELFLAGS2 ] = 0;
		DPRINTF("rbsymlist: %d symbols\n",nsyms);
		for( i = 0 ; i < nsyms ; ++i ){
			DPRINTF("symbol \"%s\" value=%#o relmode=%o\n",
				symlist[i]->name,symlist[i]->value,symlist[i]->relmode);
			if(symtype == ENTRY_SYM || symtype == OVERLAY_SYM
			   || symtype == EXT_DISP_SYM || symtype == NORMAL_EXT_SYM)
				symlist[i]->type = symtype;

			setrelflag(rb_block,i,symlist[i]->relmode);
			to_radix50(symlist[i]->name,rb_block+RB_HEADER_WORDS+i*3,symtype);
			rb_block[RB_HEADER_WORDS+i*3+2] = symlist[i]->value;
		}
		rb_putblock(type,rb_block,3*nsyms);
	}
}

void rbexpr(RB_WORD type,int w,int m){
	if(pass==2){
		flushrb();
		rb_block[ RB_RELFLAGS0 ] = 
		rb_block[ RB_RELFLAGS1 ] = 
		rb_block[ RB_RELFLAGS2 ] = 0;
		setrelflag(rb_block,0,m);
		rb_block[ RB_HEADER_WORDS ] = w; /* equivalence value */
		rb_putblock(type,rb_block,1);
	}
}

void rbcomm(struct sym_rec *s,int w,int m){
	if(pass==2){
		flushrb();
		s->type = LABELED_COMMON;
		rb_block[ RB_RELFLAGS0 ] = 
		rb_block[ RB_RELFLAGS1 ] = 
		rb_block[ RB_RELFLAGS2 ] = 0;
		setrelflag(rb_block,0,m);
		to_radix50(s->name,rb_block+RB_HEADER_WORDS,LABELED_COMMON);
		rb_block[ RB_HEADER_WORDS+2 ] = 0; /* equivalence value */
		rb_block[ RB_HEADER_WORDS+3 ] = w; /* expression value */
		rb_putblock(COMM_BLK,rb_block,4);
	}
}

void rbgadd(RB_WORD type,struct sym_rec *s,int w,int m){
	if(pass==2){
		flushrb();
		rb_block[ RB_RELFLAGS0 ] = 
		rb_block[ RB_RELFLAGS1 ] = 
		rb_block[ RB_RELFLAGS2 ] = 0;
		setrelflag(rb_block,0,s->relmode);
		rb_block[ RB_HEADER_WORDS ] = s->value; /* address */
		to_radix50(s->name,rb_block+RB_HEADER_WORDS+1,0);
		rb_block[ RB_HEADER_WORDS+3 ] = 0; /* equivalence value */
		setrelflag(rb_block,1,m);
		rb_block[ RB_HEADER_WORDS+4 ] = w; /* expression value */
		rb_putblock(type,rb_block,5);
	}
}
	
void assemble(int word,int m){
	char s[200];
	extern int cond;

	if(cond){
		word &= wordmask;

		if(pass==2){
			if(verbose){
				disasm(s,word);
				printf("[%06o] = %06o (%s)  %s\n", 
					   currentloc(),word,rb_relflag_short[m],s);
			}

			if(bootprog){
				if(bootwords){
					fputc(word>>8,obj); fputc(word,obj);
					--bootwords;
				}
			}else{
				if(!rb_count)
					startrb();
				setrelflag(rb_block,rb_count,m);
				rb_block[RB_HEADER_WORDS + rb_count] = word;
				++rb_count;
				if(rb_count == 15)
					flushrb();
			}

			listo(currentloc(),word,m);
		}
		switch(relmode){
		case ABSOLUTE: ++curloc; break;
		case NORMAL_REL: ++nrel_loc; break;
		case PAGE_ZERO_REL: ++zrel_loc; break;
		}
		++words;
	}else flushlist();
}
