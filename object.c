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

extern void objheader();
extern void objfooter();

extern int wordmask,listing; // full word bit mask for target architecture
//int curloc,radix,words,cksum,rimflag = 0,leader = 0;
FILE *obj;
extern FILE *listfile;
extern char *objsuffix;

void open_out(char *fn){
	char out[FILENAME_MAX+1], *ext;
	
	strcpy(out,fn);
	ext = index(out, '.');
	if(ext) {
	    *ext = 0;
	}
	strcat(out,objsuffix);
	if( (obj = fopen(out,"w")) ){
		if(verbose||interactive)
			printf("--- object output: %s\n",out);
		objheader();
	}
	strcpy(out,fn);
    ext = index(out, '.');
    if(ext) {
        *ext = 0;
    }
	strcat(out,".lst");
	if( listing && (listfile = fopen(out,"w")) ){
		if(verbose||interactive)
			printf("--- listing output: %s\n",out);
	}
}
void close_out(){
	if(obj){
		objfooter();
		fclose(obj);
	}
	if(listfile) fclose(listfile);
}
