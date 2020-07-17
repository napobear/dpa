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

#include <ctype.h>

#include "rb.h"

/*
MAC and ASM use radix 50 representation to condense
symbols of five characters into two words of storage using
only 27 bits. Each symbol consists of from 1 to 5 characters;
a symbol having five characters may be represented as:

a_4 a_3 a_2 a_1 a_0

where each a may be one of the following characters:

A through Z (one of 26 characters)
0 through 9 (one of 10 characters)
. or ? (one of 2 characters)

All symbols are padded, if necessary, with nulls. Each character
is translated into octal representation as follows:

Character a	Translation b
null		0
0 to 9		1 to 012
A to Z		013 to 044
.			045
?			046

If any a is translated to b, the bits required to represent the
original a can be computed as follows:

N_1 = ((b_4 * 050 + b_3) * 050) + b_2
N_1maximum = 050^3 - 1 = 0174777, which can be represented 
in 16 bits (one word)
N_2 = (b_1 * 050) + b_0
N_2maximum = 050^2 - 1 = 03077, which can be represented in 11 bits.

Thus, any symbol a can be represented in 27 bits of storage, 
as shown below in the binary output block formats.
*/
enum{ RADIX = 050 };
void to_radix50(char *a,RB_WORD w[],int flags){
	enum{ MAXLEN = 5 };
	int b[MAXLEN],i,j,c,n1,n2;

	for( i = MAXLEN,j = 0 ; i ; ){
		c = toupper(a[j++]);
		if(!c) break; /* end of input string */
		else if(isdigit(c)) b[--i] = 01+(c-'0');
		else if(isalpha(c)) b[--i] = 013+(c-'A');
		else if(c=='.') b[--i] = 045;
		else if(c=='?') b[--i] = 046;
	}

	/* pad with zeroes */
	while(i)
		b[--i] = 0;

	n1 = (b[4]*RADIX + b[3])*RADIX + b[2];
	n2 = (b[1]*RADIX + b[0]);

	w[0] = n1;
	w[1] = (n2<<5) | flags;
}

void from_radix50(char *s,RB_WORD n1,RB_WORD n2){
	static char map[]=" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ.?";
	n2 >>= 5;
	s[4] = map[n2 % RADIX];
	s[3] = map[n2 / RADIX];
	s[2] = map[n1 % RADIX];
	n1 /= RADIX;
	s[1] = map[n1 % RADIX];
	n1 /= RADIX;
	s[0] = map[n1 % RADIX];
	s[5] = 0;
}

/*
int swap(int w){
	return ((w & 0377) << 8) | ((w >> 8) & 0377);
}

int main(int ac,char *av[]){
	int w1,w2;

	if(ac==2){
		to_radix50(av[1],&w1,&w2,TITLE_SYM);
		printf("radix50(\"%s\")= %06o %06o\n", av[1],swap(w1),swap(w2));
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}
*/
