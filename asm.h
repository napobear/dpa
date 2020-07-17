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
#include <string.h>

extern char version[],target_arch[],
            listfmt[],listfmt_noloc[],listfmt_empty[],
            *objsuffix;

enum{ PARITYBIT = 0200 };

/* #define NULL ((void*)0) */
#define NEW(P) ( (P) = malloc(sizeof(*(P))) )

#define VPUTS if(verbose) puts
#define VPRINTF if(verbose) printf
#define DPUTS if(debug) puts
#define DPRINTF if(debug) printf

enum{ 
	USER_SYMBOL = -1, /* default type */

	/* symbol flags */
	F_NONE = 0,
	F_USER=01, /* symbol was user-created rather than built-in */
	F_ASSIGNED=02, /* symbol has been given a value */
};

struct sym_rec{
	/* ADD ANY EXTRA FIELDS AT THE END OF THIS STRUCT
	   otherwise initialisers in initsyms.c will screw up! */
	struct sym_rec *next;
	char *name;
	int value,token,flags,pageno,lineno,relmode,type;
};
extern int listpos;
#define LIST_LINE 72
extern char listline[];
extern FILE *listfile;

extern struct sym_rec predefs[],*lastsym;
extern int casesense,rimflag,leader,rbformat,
	errors,pass,pageno,yylineno,debug,interactive,verbose,
	inputradix,radix,curloc,words;
extern FILE *obj;

void yyerror(char*s);
extern int yyparse();

struct sym_rec *lookup(char *s);
void init_symtab();
void insert(struct sym_rec *p);
void expunge();
void clean_syms();
struct sym_rec *dosymbol(char *text,int tok);
void list_symbols();
void dump_symbols();

void iodata(char *mnemonic,int opcode,char **class);
void twoac(char *mnemonic,int opcode);
void cpu_initsyms();

void fatal(char *fmt,...);
void warn(char *fmt,...);

void assemble(int word,int relmode);
void setorg(int loc);
void dump_object(void);

void doassign(struct sym_rec *p,int value,int flags,int relmode,int lineno);

int mri(int opcode,int mod,int operand);
int combine(int op1,int op2);

char *scpy(char *dst,char *s);
char *sdup(char*);
char *uppercase(char*);
char *endof(char *str);
int read_num(char *p,int radix);
int read_signed(char *p,int radix);

void microinst(char *s,int word,char *codes[]);
void disasm(char *s,int word);

void header();
void footer();
void putout(int loc,int val);
void open_out(char *fn);
void close_out();
void flushrb();
void rbtitle(struct sym_rec *s);
void rbstart(int w,int relmode);
void initcurloc();

void warranty();
void copyright();

void dolisting(int act,int leng,char *text);
void listo(int a,int w,int m);
void newline(void);
void flushlist(void);
void printsym(struct sym_rec *p);
void listempty(char *);
