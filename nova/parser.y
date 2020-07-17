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
#include "nova.h"

#ifndef TOK_TWOAC
  #include "parser.tab.h"
#endif

extern int endflag,curloc,nrel_loc,zrel_loc,bootprog,indirect;
int radix,saveradix,saveinexpr,indexseen,seenterm,relmode = NORMAL_REL,
    condtop = 0,cond = 1,condstack[MAXCONDDEPTH],txtm=0,txtn=0;
struct sym_rec *symlist[MAXSYMLIST];
int nsyms = 0;

void dalc(struct sym_rec *symbol,int v,int lineno){
	char *sym = symbol->name,*p;
	/* look for trailing flag characters */
	if( strlen(sym) > 3 && (p = strpbrk(sym+3,"ZOCLRS")) == sym ){
		DPUTS(".DALC: stripping flag characters from mnemonic");
		*p = 0; // strip flags. FIXME: there might be non-flag characters in the trailing part!!
		twoac(sym,v);
	}else
		doassign(symbol,v,TOK_TWOAC,ABSOLUTE,lineno); 
}

void dio(struct sym_rec *s,int v,int lineno){
	extern char *class_f[];
	char *sym = s->name,*p;
	
	/* look for a three character mnemonic immediately followed by flags */
	if( strlen(sym) > 3 && (p = strpbrk(sym+3,"SCP")) == sym ){
		DPUTS(".DIO: stripping flag characters from mnemonic");
		*p = 0; // strip flags. FIXME: there might be non-flag characters in the trailing part!!
		iodata(sym,v,class_f);
	}else
		doassign(s,v,TOK_IO,ABSOLUTE,lineno); 
}

void badreloc(char *op){
	if(cond && pass==2)
		fatal("bad relocation mode: %s",op);
}

void popcond(){
	if(condtop) 
		cond = condstack[--condtop];
	else 
		fatal("ENDC without IF");
}

void pushcond(int c){
	if(condtop == MAXCONDDEPTH)
		fatal("too many nested IFs!");
	else{
		condstack[condtop++] = cond;
		cond = cond && c;
	}
}

void ignoring(char *s){
	if(pass == 1) warn("ignoring %s directive in bootstrap",s);
}

%}

%locations

/* Bison declarations */
%union{
	struct operand value;
	unsigned long valuel;
	struct sym_rec *symbol;
	char *string;
}

%token TOK_LE TOK_GE TOK_EQ TOK_NE
%token TOK_SEP 
%token <symbol> TOK_SYM TOK_SKIP TOK_NOAC TOK_ONEAC TOK_TWOAC TOK_IO TOK_TRAP

/* pseudo-ops */
%token <symbol> TOK_DALC TOK_DIAC TOK_DIO TOK_DIOA TOK_DISD TOK_DISS TOK_DMR TOK_DMRA TOK_DUSR TOK_XPNG
%token <symbol> TOK_BLK TOK_NREL TOK_ZREL
%token <symbol> TOK_COMM TOK_CSIZ TOK_ENT TOK_ENTO TOK_EXTD TOK_EXTN TOK_EXTU TOK_GADD TOK_GLOC TOK_GREF
%token <symbol> TOK_ENDC TOK_IFE TOK_IFG TOK_IFL TOK_IFN
%token <symbol> TOK_TXT TOK_TXTE TOK_TXTF TOK_TXTM TOK_TXTN TOK_TXTO
%token <symbol> TOK_EJEC TOK_NOCON TOK_NOLOC TOK_NOMAC
%token <symbol> TOK_REV TOK_RDX TOK_RDXO TOK_TITL TOK_RB TOK_LMIT TOK_PASS TOK_EOF TOK_EOT TOK_END
%token <symbol> TOK_LOC

%token <value> TOK_NUM TOK_DOT
%token <valuel> TOK_NUMD TOK_FPS
%token <string> TOK_STRING
%type <value> term termnotsym
%type <value> expr subexpr instr assign iooperand assignval asminstr
%type <value> noac oneac twoac io trap opthash optskip optindex ac ea
%type <symbol> symlist textop 
%type <value> pseudoop /*FIXME?*/

%left TOK_EQ TOK_NE '<' TOK_LE '>' TOK_GE
%left '-' '+' '*' '/' '!' '&'
%left 'B'

%% /* grammar rules */

program: /* empty */ 
	| program stmt { /*dolisting(yy_act,yyleng,yytext);*/ } TOK_SEP { indirect = 0; flushlist(); }
	| error TOK_SEP { indirect = 0; flushlist(); yyerrok; } 
	  /* on error, skip to end of statement and recover */
	;

assign : TOK_SYM '=' assignval 
		{ if(cond) doassign($1, $3.value, TOK_SYM, $3.relmode, @1.first_line);
		$$ = $3; }
	;
assignval : assign | instr ;

asminstr : instr { assemble($1.value,$1.relmode); }
	| pseudoop
	;

label : TOK_SYM ':' {     
			if(cond){
				if( $1->flags & F_ASSIGNED ){
					if(pass==1)
						warn("label already defined; ignoring this definition");
				}else 
					doassign($1, currentloc(), TOK_SYM, relmode, @1.first_line);
			}
		}
	;
labels : label | labels label ;

stmt: /*empty*/
	| labels
	| labels asminstr
	| asminstr { }
	| TOK_NUMD { assemble($1 >> 16,ABSOLUTE); assemble($1,ABSOLUTE); }
	| assign { }
	;

ac : expr { $$.value = $1.value & 3; 
			if(cond && $1.value>3) fatal("no such accumulator"); }
	;
opthash : /* empty */ { $$.value = 0; }
	| '#' { $$.value = TWOAC_NOLOAD; DPUTS("-- saw #"); }
	;
optskip : /* empty */ { $$.value = 0; }
	| ',' TOK_SKIP { $$.value = $2->value; DPRINTF("-- saw skip (%s)",$2->name); }
	;
optindex : /* empty */ { $$.value = indexseen = 0; }
	| ',' expr { $$.value = $2.value & 3; 
				 indexseen = 1;
				 if(cond && $2.value>3) fatal("bad index");
				 DPRINTF("-- saw index = %o\n",$2.value); }
	;

termnotsym : TOK_NUM { seenterm=1; }
	| TOK_DOT { /* special symbol: location counter */
				seenterm=1; $$.value = currentloc(); $$.relmode = relmode; }  
	| '(' { inputradix = radix; } subexpr ')' { seenterm=1; DPUTS(" () "); $$ = $3; }
	| '-' term { seenterm=1; DPUTS(" (negate)"); 
				$$.value = - $2.value; 
				$$.relmode = $2.relmode; 
				if($2.relmode != ABSOLUTE) badreloc("-"); }
	;

term : termnotsym 
	| TOK_SYM { if(cond && pass==2 && !($1->flags & F_ASSIGNED)) 
					fatal("undefined symbol");
				$$.value = $1->value;
				$$.relmode = $1->relmode; }
	;

/*
	rules for combining relocation modes
	+ (addition)
		a + a  ->  a
		a + r  ->  r
		r + r  ->  2r (byte reloc)
		nr + mr -> (n+m)r    (i.e. valid if n+m = 1 or 2??)
		a - a  ->  a
		r - a  ->  r
		a - r  ->  -r  (illegal?)
		r - r  ->  a 
		nr - mr -> (n-m)r    (??)
		a * a  ->  a
		a * r  ->  ar  (illegal?)
		r * r  ->  illegal
		a / a  ->  a 
		kr / a ->  (k/a)r  (only if k/a yields no remainder)
		a / r  ->  illegal
		a & a  ->  a
		a ! a  ->  a
		r & r  ->  illegal
		a & r  ->  illegal
		r ! r  ->  illegal
		a ! r  ->  illegal

	"All expressions involving the operators <= < >= > == or <>
	result in an absolute value of either zero (false) or one (true).
	When operands in these expressions have different relocation properties,
	all comparisons result in a value of absolute zero (false)
	except when the operator is <> (not equal to)."
*/

subexpr: term
	| termnotsym 'B' { seenterm=0; inputradix = 10; } term { 
				inputradix = radix; 
				$$.value = $1.value << (15-$4.value); 
				$$.relmode = ABSOLUTE;
				if($1.relmode != ABSOLUTE || $4.relmode != ABSOLUTE) 
					badreloc("B [both operands must be absolute]");
				if($4.value < 0 || $4.value > 15)
					warn("bit alignment value not in range 0-15");
				DPUTS(" (bit align) "); 
			}
	| subexpr '+' {seenterm=0;} subexpr { 
				$$.value = $1.value + $4.value; 
				if($1.relmode == ABSOLUTE && $4.relmode == ABSOLUTE)
					$$.relmode = ABSOLUTE;
				else if($1.relmode == ABSOLUTE && ISRELOC($4.relmode))
					$$.relmode = $4.relmode;
				else if(ISRELOC($1.relmode) && $4.relmode == ABSOLUTE)
					$$.relmode = $1.relmode;
				else if($1.relmode == NORMAL_REL && $4.relmode == NORMAL_REL)
					$$.relmode = NORMAL_BYTE_REL;
				else if($1.relmode == PAGE_ZERO_REL && $4.relmode == PAGE_ZERO_REL)
					$$.relmode = PAGE_ZERO_BYTE_REL;
				else badreloc("+");
				DPUTS(" (add) "); }
	| subexpr '-' {seenterm=0;} subexpr { 
				$$.value = $1.value - $4.value; 
				if( ($1.relmode == ABSOLUTE || ISRELOC($1.relmode))
						 && $4.relmode == $1.relmode )
					$$.relmode = ABSOLUTE;
				else if(ISRELOC($1.relmode) && $4.relmode == ABSOLUTE)
					$$.relmode = $1.relmode;
				else badreloc("-");
				DPUTS(" (subtract) "); }
	| subexpr '*' {seenterm=0;} subexpr { 
				$$.value = $1.value * $4.value; 
				if($1.relmode == ABSOLUTE){
					if($4.relmode == ABSOLUTE) $$.relmode = ABSOLUTE;
					else if($1.value==1) $$.relmode = $4.relmode;
					else if($4.relmode == NORMAL_REL && $1.value==2)
						$$.relmode = NORMAL_BYTE_REL;
					else if($4.relmode == PAGE_ZERO_REL && $1.value==2)
						$$.relmode = PAGE_ZERO_BYTE_REL;
					else badreloc("* [unsuitable reloc for second operand]");
				}else if($4.relmode == ABSOLUTE){
					if($1.relmode == ABSOLUTE) $$.relmode = ABSOLUTE;
					else if($4.value==1) $$.relmode = $1.relmode;
					else if($1.relmode == NORMAL_REL && $4.value==2)
						$$.relmode = NORMAL_BYTE_REL;
					else if($1.relmode == PAGE_ZERO_REL && $4.value==2)
						$$.relmode = PAGE_ZERO_BYTE_REL;
					else badreloc("* [unsuitable reloc for first operand]");
				}else badreloc("* [neither operand is absolute]");
				DPUTS(" (multiply) "); 
				}
	| subexpr '/' {seenterm=0;} subexpr { 
				$$.value = $1.value / $4.value; 
				if($1.relmode == ABSOLUTE && $4.relmode == ABSOLUTE)
					$$.relmode = ABSOLUTE;
				else badreloc("/ [both operands must be absolute]");
				DPUTS(" (divide) "); }
	| subexpr '&' {seenterm=0;} subexpr { 
				$$.value = $1.value & $4.value;
				if($1.relmode == ABSOLUTE && $4.relmode == ABSOLUTE)
					$$.relmode = ABSOLUTE;
				else badreloc("& [both operands must be absolute]");
				DPUTS(" (logical AND) "); }
	| subexpr '!' {seenterm=0;} subexpr { 
				$$.value = $1.value | $4.value;
				if($1.relmode == ABSOLUTE && $4.relmode == ABSOLUTE)
					$$.relmode = ABSOLUTE;
				else badreloc("! [both operands must be absolute]");
				DPUTS(" (logical OR) "); }
	| subexpr TOK_EQ {seenterm=0;} subexpr { 
				$$.value = ($1.value == $4.value) && ($1.relmode == $4.relmode);
				$$.relmode = ABSOLUTE;
				DPUTS(" (==) "); }
	| subexpr TOK_GE {seenterm=0;} subexpr { 
				$$.value = ($1.value >= $4.value) && ($1.relmode == $4.relmode);
				$$.relmode = ABSOLUTE;
				DPUTS(" (>=) "); }
	| subexpr '>' {seenterm=0;} subexpr { 
				$$.value = ($1.value > $4.value) && ($1.relmode == $4.relmode);
				$$.relmode = ABSOLUTE;
				DPUTS(" (>) "); }
	| subexpr '<' {seenterm=0;} subexpr { 
				$$.value = ($1.value < $4.value) && ($1.relmode == $4.relmode);
				$$.relmode = ABSOLUTE;
				DPUTS(" (<) "); }
	| subexpr TOK_LE {seenterm=0;} subexpr { 
				$$.value = ($1.value < $4.value) && ($1.relmode == $4.relmode);
				$$.relmode = ABSOLUTE;
				DPUTS(" (<=) "); }
	| subexpr TOK_NE {seenterm=0;} subexpr { 
				$$.value = ($1.value != $4.value) || ($1.relmode != $4.relmode);
				$$.relmode = ABSOLUTE;
				DPUTS(" (<>) "); }
	;
expr : subexpr { seenterm = 0; }
	;

ea : expr optindex { 
				$$ = $1;
				$$.value = ea($2.value,indexseen,&($$)); }
	;
noac : TOK_NOAC ea { 
				$$.value = $1->value | $2.value; 
				$$.relmode = $2.relmode; }
	;
oneac : TOK_ONEAC ac ',' ea { 
				$$.value = $1->value | ($2.value<<11) | $4.value; 
				$$.relmode = $4.relmode; }
	;
twoac : TOK_TWOAC opthash ac ',' ac optskip
		{ $$.value = $1->value | $2.value | ($3.value<<13) | ($5.value<<11) | $6.value;
		  if($2.value && !$6.value)
			warn("invalid instruction for Nova 3 ('no load' & 'never skip')");
		  $$.relmode = ABSOLUTE; /* ALC instruction; no address to relocate! */
		  DPRINTF("2AC %#o acs=%o acd=%o\n",$1->value,$3.value,$5.value); }
	;
iooperand : /*empty*/ { $$.value = 0; }
	| expr 
	| ac ',' expr { $$.value = ($1.value<<11) | $3.value; }
	;
io : TOK_IO iooperand { 
				$$.value = $1->value | $2.value; 
				$$.relmode = ABSOLUTE; /* no address encoded in I/O instr */ }
	;
trap : TOK_TRAP ac ',' ac ',' expr
		{ $$.value = $1->value | ($2.value<<13) | ($4.value<<11) | ($6.value<<4);
		  $$.relmode = ABSOLUTE;
		  DPRINTF("TRAP %#o acs=%o acd=%o trap=%#o\n",$1->value,$2.value,$4.value,$6.value); }
	;

instr: expr { $$.value = indirect ? $1.value | INDIR_DATA : $1.value; }
	| noac  { $$.value = indirect ? $1.value | NOAC_INDIRECT : $1.value; }
	| oneac { $$.value = indirect ? $1.value | NOAC_INDIRECT : $1.value; }
	| twoac 
	| io 
	| trap ;

optexpr : /*empty*/ 
	| expr { 
		if(bootprog)
			ignoring("END <start-loc>");
		else if(cond) 
			rbexpr(START_BLK,$1.value,$1.relmode); 
		}
	;

symlist : TOK_SYM { symlist[nsyms++] = $1; } 
	| symlist ',' TOK_SYM { symlist[nsyms++] = $3; } 
	;

textop : TOK_TXT | TOK_TXTE | TOK_TXTF | TOK_TXTO ;

pseudoop :
	  TOK_DALC TOK_SYM '=' instr { 
		/* define an ALC instruction or expression */
		if(cond) dalc($2,$4.value,@2.first_line); }
	| TOK_DIAC TOK_SYM '=' instr {
		/* define an instruction requiring an accumulator */
		if(cond) doassign($2,$4.value,TOK_ONEAC,0,@2.first_line); }
	| TOK_DIO TOK_SYM '=' instr { 
		/* define an I/O instruction that does not use an accumulator */
		if(cond) dio($2,$4.value,@2.first_line); }
	| TOK_DIOA TOK_SYM '=' instr { 
		/* define an I/O instruction having two required fields */
		if(cond) dio($2,$4.value,@2.first_line); }
	| TOK_DISD TOK_SYM '=' instr { 
		/* define an instruction with source and destination accumulators, no skip */
		if(cond) doassign($2,$4.value,TOK_TWOAC,0,@2.first_line); }
	| TOK_DISS TOK_SYM '=' instr { 
		/* define an instruction with source and destination accumulators allowing skip */
		if(cond) doassign($2,$4.value,TOK_TWOAC,0,@2.first_line); }
	| TOK_DMR TOK_SYM '=' instr { 
		/* define a memory reference instruction with displacement and index */
		if(cond) doassign($2,$4.value,TOK_NOAC,0,@2.first_line); }
	| TOK_DMRA TOK_SYM '=' instr { 
		/* define a memory reference instruction with 2 or 3 fields */
		if(cond) doassign($2,$4.value,TOK_NOAC,0,@2.first_line); }
	| TOK_DUSR TOK_SYM '=' instr { 
		/* define a user symbol without implied formatting */
		if(cond) doassign($2,$4.value,TOK_SYM,ABSOLUTE /*??*/,@2.first_line); }
	| TOK_XPNG { if(cond) clean_syms(); } /* remove all nonpermanent macro and symbol definitions */

	| TOK_BLK expr { /* reserve a block of storage */ 
				if(bootprog) ignoring("BLK");
				else if(cond){
					flushrb(); 
					switch(relmode){
					case ABSOLUTE: curloc += $2.value; break;
					case NORMAL_REL: nrel_loc += $2.value; break; 
					case PAGE_ZERO_REL: zrel_loc += $2.value; break;
					}
				}
		}
	| TOK_NREL { /* specify NREL code relocation */
			if(bootprog) ignoring("NREL");
			else if(cond && relmode != NORMAL_REL){ flushrb(); relmode = NORMAL_REL; } 
		}
	| TOK_ZREL { /* specify page zero relocation */
			if(bootprog) ignoring("ZREL");
			else if(cond && relmode != PAGE_ZERO_REL){ flushrb(); relmode = PAGE_ZERO_REL; }
		}
	| TOK_LOC expr { /* set the current location counter, to an absolute address */
			if(bootprog) ignoring("LOC");
			else if(cond){ flushrb(); relmode = ABSOLUTE; curloc = $2.value; }
		}
	
	| TOK_COMM TOK_SYM ',' expr { /* reserve a named common area */ 
		if(bootprog) ignoring("COMM");
		else if(cond) rbcomm($2,$4.value,$4.relmode);
		}
	| TOK_CSIZ expr { /* specify an unlabelled common area */ 
		if(bootprog) ignoring("CSIZ");
		else if(cond) rbexpr(CSIZ_BLK,$2.value,$2.relmode); 
		}
	| TOK_ENT { nsyms=0; } symlist { /* define a program entry */ 
		if(bootprog) ignoring("ENT");
		else if(cond) rbsymlist(ENT_BLK,ENTRY_SYM,symlist,nsyms); 
		}
	| TOK_ENTO TOK_SYM { /* define an overlay entry */
		symlist[0] = $2;
		if(bootprog) ignoring("ENTO");
		else if(cond) rbsymlist(ENT_BLK,OVERLAY_SYM,symlist,1); 
		}
	| TOK_EXTD { nsyms=0; } symlist { /* define an external displacement reference */
		if(bootprog) ignoring("EXTD");
		else if(cond) rbsymlist(EXTD_BLK,EXT_DISP_SYM,symlist,nsyms); 
		}
	| TOK_EXTN { nsyms=0; } symlist { /* define an external normal reference */
		if(bootprog) ignoring("EXTN");
		else if(cond) rbsymlist(EXTN_BLK,NORMAL_EXT_SYM,symlist,nsyms); 
		}
	/* | TOK_EXTU { } */ /* treat undefined symbols as external displacements */
	| TOK_GADD TOK_SYM ',' expr { /* add an expression value to an external symbol */ 
		if(bootprog) ignoring("GADD");
		else if(cond) rbgadd(GADD_BLK,$2,$4.value,$4.relmode);
		}
	| TOK_GREF TOK_SYM ',' expr { /* add an expression value to an external symbol (0b0) */
		if(bootprog) ignoring("GREF");
		else if(cond) rbgadd(GREF_BLK,$2,$4.value,$4.relmode);
		}
	/* | TOK_GLOC TOK_SYM { } */ /* reserve an absolute data block */ 
	
	| TOK_ENDC { popcond(); } /* specify the end of conditional assembly */ 
	| TOK_IFE expr { pushcond($2.value == 0); /* assemble if expr = 0 */ }
	| TOK_IFG expr { pushcond($2.value > 0);  /* assemble if expr > 0 */ }
	| TOK_IFL expr { pushcond($2.value < 0);  /* assemble if expr < 0 */ }
	| TOK_IFN expr { pushcond($2.value != 0); /* assemble if expr != 0 */ }
	
	| textop TOK_STRING { DPUTS("TXT directive"); }
	| TOK_TXTM expr { if(cond) txtm = $2.value; /* change text byte packing */ }
	| TOK_TXTN expr { if(cond) txtn = $2.value; /* determine text string termination */ }
	
	/*| TOK_REV */
	| TOK_RDX { /* radix for numeric input conversion */ 
			saveradix = radix; inputradix = 10; } 
		expr { radix = saveradix; 
			if($3.value < 2 || $3.value > 20)
				warn("invalid input radix; must be >=2 and <=20");
			else 
				if(cond) radix = inputradix = $3.value; 
		}
		
	| TOK_TITL TOK_SYM { /* entitle an RB file */ 
		if(bootprog) ignoring("TITL");
		else if(cond) rbtitle($2); }
	/*| TOK_LMIT TOK_PASS */
	
	| TOK_END optexpr { if(cond){ endflag = 1; YYACCEPT; } /* end of program */ }
	| TOK_EOF { if(cond) YYACCEPT; }
	| TOK_EOT { if(cond) YYACCEPT; }
	;

%%
