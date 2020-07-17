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
%option yylineno

%{
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>

#include "asm.h"
#include "nova.h"
#include "parser.tab.h"

extern int seenterm,symflag;
extern unsigned char eparity[],oparity[]; /* lookup tables for parity */
extern int txtm,txtn;

int inputradix,pageno,casesense = 0,txttok,indirect = 0;
char delim; /* string delimiter for current TXT directive */
int nchars,strbuf[2];

#ifndef FLEX_SCANNER
void yyrestart(FILE *fp){
	extern FILE *yyin;
	yyin = fp;
}
#endif

void stringchar(int c){
	strbuf[nchars++] = c;
	if(nchars == 2){
		assemble(txtm ? strbuf[0]<<8 | strbuf[1] : strbuf[1]<<8 | strbuf[0],ABSOLUTE);
		nchars = 0;
	}
}
unsigned long read_float(char *s,int base){
	int sign = 0,exp = 0,infrac = 0;
	unsigned long v = 0;

	for( ; *s ; ++s ){
		if(*s == '-') sign = 1;
		else if(*s == '.') infrac = 1;
		else if(toupper(*s) == 'E'){
			DPRINTF("adding specified exponent (%d)\n",atoi(s+1));
			exp += atoi(s+1);
			break;
		}else{
			/* must be a digit */
			DPRINTF("digit:%c\n",*s);
			if(v > ((ULONG_MAX-(*s-'0'))/base)){
				if(infrac){
					warn("float constant: throwing away excess precision");
					while(isdigit(s[1]))
						++s; /* skip rest of fraction */
					DPRINTF("skipping digits; next char=%c\n",*s);
					continue;
				}
				++exp; /* can't store any more precision in whole part,
						  so just account for magnitude */
				DPRINTF("throwing away digit but increasing exponent to %d\n",exp);
			}else{
				v = base*v + (*s - '0');
				DPRINTF("value is now %lu\n",v);
				if(infrac){ --exp; DPRINTF("decreased exponent to %d\n",exp); }
			}
		}
	}
	DPRINTF("float constant: sign=%d v=%lu exp=%d\n",sign,v,exp);
	
	return 0;
}

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno; dolisting(yy_act,yyleng,yytext);

%}
%x STRING INSTRING

SPACE			[ \t\0\177]
NEWLINE			\n|\r|\r\n|\f

%%

{NEWLINE}		{ return TOK_SEP; } 

{SPACE}+		; /* whitespace */

@				{ indirect = 1; }

\;[^\n\r\f]*	; /* comment */

<STRING>{SPACE}+ ; /* ignore space between TXT and opening delimiter */
<STRING>.		{ 
				delim = yytext[0];
				nchars = 0; 
				BEGIN(INSTRING);
				}

<INSTRING>(NEWLINE) { newline(); } /* ignore these in a string */
<INSTRING>[^\n\r\f]+ { /* any other character */
					char *p;
					int i, c, bad = 0;
					for(i = yyleng, p = yytext; i--;){
						c = *p++;
						if(c == delim){
							stringchar(0); /* fill last unfilled word with zero byte */
							if(!txtn) assemble(0,ABSOLUTE); /* trail with zero word, if enabled */
							BEGIN(INITIAL);
							return TOK_STRING;
						}else{
							if(c == '<'){ // handle inline 'expression' for character code
								if(i >= 3){ // shortest valid possibility is <N>
									if(*p == '\"'){
										if(i >= 3){ /* shortest is "X> */
											c = p[1] & 0177;
											i -= 2; p += 2;
										}else bad = 1;
									}else{ // parse a single octal number.
										// FIXME: DG assembler handles a more complex expression here. Should ignore whitespace too?
										for(c = 0; i && isdigit(*p); --i, ++p)
											c = (c << 3) + (*p - '0'); // accumulate octal digit
									}
									if(i && *p == '>')
										--i, ++p; // skip the '>'
									else bad = 1;
								}else bad = 1;
								if(bad){
									printf("bad <...> in string\n");
									BEGIN(INITIAL);
									return TOK_STRING; // what's the best way to signal a lexical error?
								}
							}
							c &= 0177;
							switch(txttok){
							case TOK_TXTF: c |= PARITYBIT; break;
							case TOK_TXTE: c = eparity[c]; break;
							case TOK_TXTO: c = oparity[c]; break;
							}
							stringchar(c);
						}
					}
				}

\".				{ yylval.value.value = yytext[1]; 
				yylval.value.relmode = ABSOLUTE;
				return TOK_NUM; }

\'[^\'\n\r\f]*\'? { yylval.value.value = 0;
				if(yyleng>0 && yytext[1] != '\'') 
					yylval.value.value |= yytext[1]<<8;
				if(yyleng>1 && yytext[2] != '\'') 
					yylval.value.value |= yytext[2]; 
				yylval.value.relmode = ABSOLUTE;
				return TOK_NUM; }

[0-9]+			{ yylval.value.value = read_num(yytext,inputradix); 
				DPRINTF("number: \"%s\" = octal %#o = decimal %d\n",
					yytext,yylval.value.value ,yylval.value.value ); 
				yylval.value.relmode = ABSOLUTE;
				return TOK_NUM ; }

[+-]?[0-9]+\.[0-9]*([eE][+-]?[0-9]+)? {
				/* single precision floating point constant */
				warn("floating point constants not yet implemented");
				//yylval.valuel = read_float(yytext,10);
				return TOK_FPS; }

[+-]?[0-9]+[dD]	{ yylval.valuel = read_signed(yytext,inputradix); 
				DPRINTF("double precision integer: \"%s\" = octal %#lo = decimal %ld\n",
					yytext,yylval.valuel,yylval.valuel); 
				return TOK_NUMD ; }

[.?A-Za-z][.?A-Za-z0-9_]*	 {
					struct sym_rec *p;
					if(toupper(yytext[0]) == 'B'){
						DPRINTF("recognised B... seenterm=%d\n",seenterm);
						if(seenterm){
							yyless(1); /* take B, and give rest of symbol 
										  back for re-parsing */
							return 'B';
						}
					}
					p = dosymbol(yytext,TOK_SYM);
					if( p->token == TOK_TXT  || p->token == TOK_TXTE
					 || p->token == TOK_TXTF || p->token == TOK_TXTO ){
						txttok = p->token;
						BEGIN(STRING);
					}
					yylval.symbol = p;
					return p->token;
				}

"=="			return TOK_EQ;
">="			return TOK_GE;
"<="			return TOK_LE;
"<>"			return TOK_NE;

[\-+.*,=#:/<>&!()] return yytext[0]; 

.				printf("illegal character:'%c'\n",yytext[0]);
