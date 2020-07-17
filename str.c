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

#include "asm.h"

/* return pointer after last character of string */
char *endof(char *s){
	while(*s)
		++s;
	return s;
}

/* copy string; return pointer after last char copied */
char *scpy(char *dst,char *s){
	while( (*dst = *s++) )
		++dst;
	return dst;
}

char *sdup(char *s){
	char *p = malloc(strlen(s)+1);
	strcpy(p,s);
	return p;
}

char *uppercase(char *s){
	char *p;
	p = s;
	while( (*p = toupper(*p)) )
		p++;
	return s;
}

int read_num(char *p,int radix) /* read a positive constant */
{
	int acc;
	for(acc=0;isdigit(*p);)
		acc = (acc*radix) + (*p++) - '0';
	return acc;
}

int read_signed(char *p,int radix){
	int sign = 1;
	if(*p == '+'){
		++p;
	}else if(*p == '-'){
		++p;
		sign = -1;
	}
	return sign*read_num(p,radix);
}
