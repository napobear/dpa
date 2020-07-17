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
#include <stdlib.h>

/* method due to http://remus.rutgers.edu/~rhoads/Code/bitcount2.c */
int bit_count(long x)
{
	int n = 0;

	if (x)
		do {
			n++;
		} while ((x &= x-1));

	return(n);
}
void dotable(char *name,int omask,int emask){
	int i;
	printf("unsigned char %s[0200] = {",name);
	for(i=0;i<0200;++i){
		if(!(i % 8)){
			putchar('\n');
			putchar('\t');
		}
		printf("%04o",bit_count(i) & 1 ? i|omask : i|emask);
		putchar(',');
	}
	printf("\n};\n");
}

int main(){
	dotable("eparity",0200,0);
	dotable("oparity",0,0200);
	return EXIT_SUCCESS;
}
