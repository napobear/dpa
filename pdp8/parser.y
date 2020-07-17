%{ 
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
#include "pdp8.h"

#ifndef TOK_SYM
  #include "parser.tab.h"
#endif

int radix,fixmri = TOK_SYM;

%}

/* Bison declarations */
%union{
	int value;
	struct sym_rec *symbol;
}

%token TOK_SEP 
%token TOK_FIELD TOK_EXPUNGE TOK_FIXMRI TOK_PAUSE TOK_FIXTAB TOK_DECIMAL TOK_OCTAL
%token <symbol> TOK_SYM TOK_MRI 
%token <value> TOK_NUM 
%type <value> mod mods comb mri term expr instr labelinstr asminstr assign assignval

%left '-' '+'

%% /* grammar rules */

stmts: /* empty */
	| stmts stmt TOK_SEP { flushlist(); }
	| error TOK_SEP { yyerrok; } 
		/* on error, skip to end of statement and recover */
	;

assign : TOK_SYM '=' assignval { doassign($1,$$ = $3,fixmri,0,@1.first_line); }
	;
assignval : assign | instr ;

labelinstr: TOK_SYM ',' {
		if( pass==1 && ($1->flags & F_ASSIGNED) )
			warn("label already defined; ignoring this definition");
		else 
			doassign($1,curloc,TOK_SYM,0,@1.first_line);
	}
	instr { $$ = $4; }
	;
asminstr : instr | labelinstr ;

stmt: /*empty*/ 
	| '*' expr { setorg($2); DPRINTF("-- set origin = %#o\n",$2); } 
	| asminstr { assemble($1,0); DPUTS("-- assemble word"); }
	| assign { /*throw away final value of assignments*/ }

	| TOK_FIELD TOK_NUM { DPRINTF("-- FIELD %o pseudo-instruction\n",$2); }
	| TOK_EXPUNGE { expunge(); DPUTS("-- EXPUNGE pseudo-instruction"); }
	| TOK_FIXMRI { fixmri = TOK_MRI; } 
	  assign { fixmri = TOK_SYM; DPUTS("-- FIXMRI pseudo-instruction"); }
	| TOK_PAUSE { DPUTS("-- PAUSE pseudo-instruction"); }
	| TOK_FIXTAB { DPUTS("-- FIXTAB pseudo-instruction"); }
	| TOK_DECIMAL { radix = 10; DPUTS("-- radix now decimal"); }
	| TOK_OCTAL { radix = 8; DPUTS("-- radix now octal"); }
	;

/* optional addressing modifier */
mod:  'I' { $$ = F_INDIRECT; DPUTS(" indirect modifier"); }
	| 'Z' { $$ = F_ZEROPAGE; DPUTS(" zero page modifier"); }
	;
mods: /*empty*/ { $$ = 0; }
	| mods mod { $$ = $1 | $2; }
	;

mri: TOK_MRI mods expr { $$ = mri($1->value,$2,$3); 
						 DPUTS("  memory-ref instruction"); }
	;

term: TOK_NUM 
	| TOK_SYM {
			if(pass==2 && !($1->flags & F_ASSIGNED))
				fatal("undefined symbol");
			$$ = $1->value;
		}
	| '.' { $$ = curloc; }
	;
comb: term | comb term { $$ = combine($1,$2); DPUTS(" (combine)"); }
	;

expr: comb
	| expr '+' expr { $$ = ($1 + $3) ; DPUTS(" (add) "); }
	| expr '-' expr { $$ = ($1 - $3) ; DPUTS(" (subtract) "); }
	| '-' expr { $$ = -$2 ; DPUTS(" (negate)"); }
	;
instr: expr | mri ;

%%
