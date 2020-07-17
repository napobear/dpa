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
#include "version.h"

extern int yydebug;
extern FILE *listfile;

extern void yyrestart(FILE*);

int debug = 0,pass,interactive = 0,verbose = 0,listing = 0,bootprog = 0;
char *default_out = "dpa.out",*inputfile;
int symfile[20],symflag, /* default flags for new symbols */
	endflag; /* parser sets this if symbol table needs to be reset */

void makepass(int p,FILE *fp){
	pass = p;
	initcurloc();
	words = 0;
	radix = inputradix = 8;
	pageno = 1;
	newline();
	yyrestart(fp);
	endflag = 0; 

	yyparse();

	flushlist();
	DPRINTF("=== pass %d complete\n\n",p);
	if(p==2){
		if(listfile) list_symbols();
		if(verbose) dump_symbols();
		/*if(endflag)*/ clean_syms();
		VPRINTF("# %s : %d errors, %d words\n",inputfile,errors,words);
	}
}

void banner(){
	printf("%s Assembler " VERS_STR ", Copyright (C) 2002-7 Toby Thain\n\n",target_arch);
}

void usage(char *s){
	printf("usage: %s [OPTION]... [FILE]...\n\
       -s FILE : assemble file; keep symbols but suppress output\n\
       -d : debug mode (trace parsing etc.)\n\
       -r : [PDP-8] produce RIM-format output file\n\
            (default output format is BIN)\n\
       -p : [PDP-8] punch 2' lead-in and lead-out\n\
       -b : [Nova] output a bootstrap program (see Prog. Ref. page VI-7)\n\
       -v : be verbose (show object as it is generated, and dump symbol table)\n\
       -l : write listing file (FILE.lst)\n\
       -w : show warranty\n\
       -c : show copyright\n\
       -h : show this help\n\
Invoke with no source files to use interactively.\n",s);
}

int main(int argc,char *argv[]){
	extern char *verbose_heading;
	int i,nfiles=0;
	FILE *fp;

	init_symtab();

	for(i=1;i<argc;i++)
		if(*argv[i] == '-'){
			switch((argv[i])[1]){
			case 's': 
				if(++i == argc) printf("-s must be followed by an input file name");
				else symfile[i] = 1; 
				break;
			case 'd': debug = 1; break;
			case 'r': rimflag = 1; objsuffix = ".rim"; break;
			case 'p': leader = 1; break;
			case 'b': bootprog = 1; objsuffix = ".boot"; break;
			case 'v': verbose = 1; break;
			case 'l': listing = 1; break;
			case 'w': banner(); warranty(); return EXIT_SUCCESS;
			case 'c': banner(); copyright(); return EXIT_SUCCESS;
			case 'h': banner(); usage(argv[0]); return EXIT_SUCCESS;
			default:
				banner();
				printf("unrecognised option; try \"%s -h\" for more information.\n",argv[0]);
				return EXIT_FAILURE;
			}
		}else
			++nfiles;

	init_symtab();

	if(nfiles){
		if(verbose) banner();
		for(i=1;i<argc;i++)
			if(*argv[i] != '-'){
				if( (fp = fopen(inputfile = argv[i],"r")) ){
					VPRINTF("--- input: %s\n",inputfile);
					errors = 0;

					obj = NULL;
					symflag = symfile[i] ? 0 : F_USER;
					makepass(1,fp);
					if(errors){
						fprintf(stderr,"# assembly aborted; skipping pass 2\n");
					}else if(!symfile[i]){
						open_out(inputfile);
						/* do pass 2 */
						rewind(fp);
						yylineno = 0;
						if(verbose) puts(verbose_heading);
						makepass(2,fp);
						close_out();
					}
					fclose(fp);
				}else
					fprintf(stderr,"# can't open \"%s\"\n",inputfile);
			}
	}else{
		interactive = 1;
		banner();
		puts("This software comes with ABSOLUTELY NO WARRANTY; for details run with option -w.\n\
This is free software, and you are welcome to redistribute it\n\
under certain conditions; run with option -c for details.\n");
		puts("# no files specified; running interactively on standard input (single pass)\n\
# WARNING: forward symbol references will be incorrect!");
		open_out("dpa.out");
		inputfile = "<stdin>";
		symflag = F_USER;
		makepass(2,stdin);
		close_out();
		/*clean_syms();*/
	}
	return EXIT_SUCCESS;
}

