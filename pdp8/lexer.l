/*
    This file is part of The Didactic PDP-8 Assembler
    Copyright (C) 2002-7 Toby Thain, toby@telegraphics.com.au

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

%option noyywrap

%{

#include "asm.h"
#include "pdp8.h"
#include "parser.tab.h"

#define GOT(S) printf("%s: \"%s\"\n",S,yytext);

int inputradix,pageno,input_lineno,lineno,casesense = 0;

#ifndef FLEX_SCANNER
void yyrestart(FILE *fp){
	extern FILE *yyin;
	yyin = fp;
}
#endif

#define YY_USER_ACTION dolisting(yy_act,yyleng,yytext); 

%}

%%

\n|\r|\r\n|\f	{ ++lineno; return TOK_SEP; } 

[ \t]+			; /* whitespace */

\/[^\f\n\r]*	; /* comment */

\;				return TOK_SEP;

[0-9]+			{
				yylval.value = read_num(yytext,inputradix); 
				DPRINTF("number: \"%s\" = octal %#o = decimal %d\n",
				        yytext,yylval.value,yylval.value); 
				return TOK_NUM;
				}

i|I				return 'I';
z|Z				return 'Z';

[A-Za-z][A-Za-z0-9]* { 
				struct sym_rec *p = dosymbol(yytext,TOK_SYM);
				yylval.symbol = p;
				return p->token;
				}

[\-+.*,=]		return yytext[0]; 

\$				return 0; /* end of input */

.				printf("illegal character:'%c'\n",yytext[0]);
