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

/* Portions from pal.c "a 2 pass PDP-8 pal-like assembler",
   by:  Douglas Jones, Rich Coon, Bernhard Baehr 
   see http://www.cs.uiowa.edu/~jones/pdp8/
   (used with permission)
*/

extern int wordmask,listing; // full word bit mask for target architecture
int curloc,words,cksum,rimflag = 0,leader = 0;
extern FILE *listfile;

char *objsuffix = ".bin";

void initcurloc(){
	curloc = 0200;
}

void putleader()
/* generate 2 feet of leader on the object file, as per DEC documentation */
{
	if (obj && leader) {
		int i;
		for (i = 240 ; i-- ; )
			fputc( 0200, obj );
	}
}

void puto(int c)
/* put one character to obj file and include it in checksum */
{
	c &= 0377;
	if(obj)
		fputc(c, obj);
	cksum += c;
}

void putorg( int loc )
{
	puto( ((loc >> 6) & 0077) | 0100 );
	puto( loc & 0077 );
}

void putout( int loc, int val )
/* put out a word of object code */
{
	if (obj != NULL) {

		if (rimflag == 1) { /* put out origin in rim mode */
			putorg( loc );
		}
		puto( (val >> 6) & 0077 );
		puto( val & 0077 );
	}
}

void objheader(){
	putleader();
	cksum = 0;
}

void objfooter(){
	if ((rimflag == 0) && (obj != NULL)) { /* checksum */
		/* for now, put out the wrong value */
		fputc( (cksum >> 6) & 0077, obj );
		fputc( cksum & 0077, obj );
	}
	putleader();
}

void setorg(int loc){
	curloc = loc;
	if(pass==2){
		VPRINTF("*%o\n",loc);
		if(!rimflag)
			putorg(loc);
	}
}

void assemble(int word,int m){
	char s[200];
	
	word &= wordmask;
	if(pass==2){
		if(verbose){
			disasm(s,word);
			printf("[%06o] = %06o  %s\n", curloc,word,s);
		}
		putout(curloc,word);
		listo(curloc,word,0);
	}
	++curloc;
	++words;
}
