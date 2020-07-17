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

#include <stdio.h>

typedef unsigned RB_WORD;

enum{ 
	RB_WORDMASK = 0177777, /* word is 16 bits */
	MAXSYMLIST = 15 /* most symbols represented in a single block */
}; 

/*
[ from "RDOS/DOS Assembly Language and Program Utilities",
  Data General document 069-400019-01 ]

MAC, ASM and any other language code generator divides
binary output into a series of blocks. The order in which
blocks appear, if each type of block is present, is as follows:

Title Block
Labeled COMMON Blocks
Entry Blocks
Unlabeled COMMON Blocks (CSIZ)
External Displacement Blocks
Relocatable Data Blocks,
  Global Addition and Reference Blocks,
  Global Start and End Blocks
Normal External Blocks
Local Symbol Blocks
Start Block

The relocatable binary output must contain at least a title 
block and a start block. The presence of one or more of the 
other types of blocks will depend upon source input. 

Bytes are always swapped in the word; thus, 003400 means
00 000 111/00 000 000; and, after swapping, 7. The first
word of each block contains a number indicating the type
of block. The number is in the range of 2 through 20(8).

The second word of each block is the word count. It is
always in two's complement format. [ i.e. -N ]

Words 3 through 5 are reserved for relocation flags. Some
block types contain these flags, others do not. The relocation
property of each address, datum or symbol is defined in
three bits. For example, for a relocatable data block, bits 0
through 2 of word 3 apply to the address, bits 3 through 5
apply to the first data word, and so on.
*/
enum{ 
	/* relocation property - values 0 and 7 are illegal */
	ABSOLUTE = 01,
	NORMAL_REL = 02,
	NORMAL_BYTE_REL = 03,
	PAGE_ZERO_REL = 04,
	PAGE_ZERO_BYTE_REL = 05,
	EXT_DISP = 06 /* Data Reference External Displacement */
};
#define ISRELOC(m) ( (m)>=NORMAL_REL && (m)<=PAGE_ZERO_BYTE_REL )
extern char *rb_relflag_types[],*rb_relflag_short[];
extern char rb_relflag_chars[];

/*
All other blocks use bits of word 3 only and set words 4
and 5 to zero.

Word 6 contains a checksum, so the sum of all words in
the block is 0.

For those words containing user symbols, each symbol entry
is three words long.

The first 27 bits of the three-word entry contain the user
symbol name in radix 50 form. The last five bits of the
second word are used as a symbol type flag, where the
currently defined types are:
*/
enum{
	/* Symbol types */
	ENTRY_SYM = 0,
	NORMAL_EXT_SYM = 01,
	LABELED_COMMON = 02,
	EXT_DISP_SYM = 03,
	TITLE_SYM = 024,
	OVERLAY_SYM = 04,
	LOCAL_SYM = 010
};
extern char *rb_sym_types[];
extern char *rb_sym_typecodes[];

/* BLOCK FORMATS */

enum{
	/* block type */
	TITL_BLK = 07,
	COMM_BLK = 013,
	ENT_BLK = 03,
	CSIZ_BLK = 015,
	EXTD_BLK = 04,
	REL_DATA_BLK = 02,
	GADD_BLK = 014,
	GREF_BLK = 020,
	GLOC_START_BLK = 016,
	GLOC_END_BLK = 017,
	EXTN_BLK = 05,
	LOCAL_SYM_BLK = 010,
	LIB_START_BLK = 011,
	LIB_END_BLK = 012,
	START_BLK = 06
};
extern char *rb_block_types[];
	
struct rb_block_header {
	RB_WORD type;
	RB_WORD word_count;
	RB_WORD rel_flags[3];
	RB_WORD checksum;
};

enum{ 
	RB_HEADER_WORDS = 6,
	/* offsets into block header: */
	RB_TYPE = 0,
	RB_WORDCOUNT,
	RB_RELFLAGS0,RB_RELFLAGS1,RB_RELFLAGS2,
	RB_CHECKSUM
};

/*
The setting of the third word allocated for each user symbol
entry varies with the type of block and is described in the
format writeups of each block.
*/
struct rb_sym_entry {
	RB_WORD radix50[2];
	RB_WORD value;
};

extern RB_WORD rb_checksum;

RB_WORD rb_getword(FILE *f);
int rb_getwords(FILE *f,RB_WORD w[],int n);
int rb_putword(FILE *f,RB_WORD w);
int rb_putwords(FILE *f,RB_WORD w[],int n);
void to_radix50(char *a,RB_WORD w[],int flags);
void from_radix50(char *s,RB_WORD n1,RB_WORD n2);
int getrelflag(RB_WORD b[],int i);
void  setrelflag(RB_WORD b[],int i,int m);
