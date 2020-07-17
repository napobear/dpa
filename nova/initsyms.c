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
#include "parser.tab.h"

char target_arch[] = "DG NOVA";
int rbformat = 1;

#define NOAC(m,op)   {0,m,op<<11,TOK_NOAC,F_NONE,0,0,0,0}
#define ONEAC(m,op)  {0,m,op<<13,TOK_ONEAC,F_NONE,0,0,0,0}
#define IO(m,op,c,d) {0,m,060000|(op<<8)|(c<<6)|d,TOK_IO,F_NONE,0,0,0,0}
#define SYM(m,v)     {0,m,v,TOK_SYM,F_NONE,0,0,0,0}
#define SKIP(m,v)    {0,m,v,TOK_SKIP,F_NONE,0,0,0,0}
#define PSEUDO(m,t)  {0,m,0,t,F_NONE,0,0,0,0}

enum{
	DEV_STACK=01,
	DEV_CPU=077,
	DEV_MDV=01,
	DEV_RTC=014,
	DEV_PAR=04,
	DEV_MMPU=02,
	DEV_MMU=02,
	DEV_MMU1=03,
	DEV_MAP=02,
	DEV_MAP1=03,
	DEV_FPU1=074,
	DEV_FPU2=075,
	DEV_FPU=076
};

struct sym_rec predefs[] = {
	ONEAC("LDA",1),
	ONEAC("STA",2),
	IO("PSHA",3,0,DEV_STACK),
	IO("POPA",3,2,DEV_STACK),
	IO("SAV", 5,0,DEV_STACK),
	IO("MTSP",2,0,DEV_STACK),
	IO("MTFP",0,0,DEV_STACK),
	IO("MFSP",2,2,DEV_STACK),
	IO("MFFP",0,2,DEV_STACK),
	IO("RET", 5,2,DEV_STACK),
	NOAC("JMP",0),
	NOAC("JSR",1),
	NOAC("ISZ",2),
	NOAC("DSZ",3),
	{0,"TRAP",0100010,TOK_TRAP,F_NONE,0,0,0,0}, /* a special 2-ac instruction */
	IO("INTEN",0,1,DEV_CPU),
	IO("INTDS",0,2,DEV_CPU),
	IO("READS",1,0,DEV_CPU),/*FIXME:not sure what implicit F should be for these? */
	IO("INTA", 3,0,DEV_CPU),
	IO("MSKO", 4,0,DEV_CPU),
	IO("IORST",5,0,DEV_CPU),
	IO("HALT", 6,0,DEV_CPU),
	IO("MUL",026,3,DEV_MDV),
	IO("DIV",026,1,DEV_MDV),
	IO(".FLDS",4,3,DEV_FPU1), IO(".FLDD",4,3,DEV_FPU2),
	IO(".FSRS",4,1,DEV_FPU1), IO(".FSRD",4,1,DEV_FPU2),
	IO(".FAS",2,0,DEV_FPU1),  IO(".FAD",2,0,DEV_FPU2),
	IO(".FSS",2,1,DEV_FPU1),  IO(".FSD",2,1,DEV_FPU2),
	IO(".FMS",2,3,DEV_FPU1),  IO(".FMD",2,3,DEV_FPU2),
	IO(".FDS",2,2,DEV_FPU1),  IO(".FDD",2,2,DEV_FPU2),
	IO(".FMFT",0,3,DEV_FPU2),
	IO(".FMTF",0,2,DEV_FPU2),
	IO(".FATS",6,0,DEV_FPU1), IO(".FATD",6,0,DEV_FPU2),
	IO(".FSTS",6,1,DEV_FPU1), IO(".FSTD",6,1,DEV_FPU2),
	IO(".FMTS",6,3,DEV_FPU1), IO(".FMTD",6,3,DEV_FPU2),
	IO(".FDTS",6,2,DEV_FPU1), IO(".FDTD",6,2,DEV_FPU2),
	IO(".FABS",0,3,DEV_FPU1),
	IO(".FCLR",0,1,DEV_FPU1),
	IO(".FLDX",4,2,DEV_FPU2),
	IO(".FNEG",0,2,DEV_FPU1),
	IO(".FNRM",0,1,DEV_FPU2),
	IO(".FHWD",1,0,DEV_FPU1),
	IO(".FSCL",4,0,DEV_FPU2),
	IO(".FRST",1,2,DEV_FPU),
	IO(".FWST",2,0,DEV_FPU),

/* the following are simply constants */

/* special skip qualifier symbols (2 accumulator instructions) */
	SKIP("SKP",1),
	SKIP("SZC",2),
	SKIP("SNC",3),
	SKIP("SZR",4),
	SKIP("SNR",5),
	SKIP("SEZ",6),
	SKIP("SBN",7),
/* device code constants (I/O instructions) */
	SYM("MDV", 001),
	SYM("MMPU",002), SYM("MMU", 002), SYM("MAP", 002),
	SYM("MAP1",003), SYM("MMU1",003),
	SYM("PAR", 004),
	SYM("MCAT",006), SYM("MCAT1",046),
	SYM("MCAR",007), SYM("MCAR1",047),
	SYM("TTI", 010), SYM("TTI1", 050),
	SYM("TTO", 011), SYM("TTO1", 051),
	SYM("PTR", 012), SYM("PTR1", 052),
	SYM("PTP", 013), SYM("PTP1", 053),
	SYM("RTC", 014), SYM("RTC1", 054),
	SYM("PLT", 015), SYM("PLT1", 055),
	SYM("CDR", 016), SYM("CDR1", 056),
	SYM("LPT", 017), SYM("LPT1", 057),
	SYM("DSK", 020), SYM("DSK1", 060),
	SYM("ADCV",021), SYM("ADCV1",061),
	SYM("MTA", 022), SYM("MTA1", 062),
	SYM("DACV",023), SYM("DACV1",063),
	SYM("DCM", 024),
	SYM("QTY", 030), SYM("QTY1", 070), 
	SYM("SLA", 030), SYM("SLA1", 070),
	SYM("IBM1",031),
	SYM("IBM2",032),
	SYM("DKP", 033), SYM("DKP1", 073),
	SYM("CAS", 034), SYM("CAS1", 074), 
	SYM("MUX8",034),
	SYM("CRC", 035),
	SYM("IPB", 036),
	SYM("IVT", 037),
	SYM("DPI", 040), SYM("SCR", 040),
	SYM("DPO", 041), SYM("SCT", 041),
	SYM("DIO", 042),
	SYM("DIOT",043),
	SYM("MXM", 044),
	SYM("FPU1",074),
	SYM("FPU2",075),
	SYM("FPU", 076),
	SYM("CPU", 077),

/* symbol table pseudo-ops */
	PSEUDO(".DALC",TOK_DALC),
//	PSEUDO(".DCMR",TOK_DCMR),
//	PSEUDO(".DEMR",TOK_DEMR),
//	PSEUDO(".DERA",TOK_DERA),
//	PSEUDO(".DEUR",TOK_DEUR),
//	PSEUDO(".DFLM",TOK_DFLM),
//	PSEUDO(".DFLS",TOK_DFLS),
	PSEUDO(".DIAC",TOK_DIAC),
//	PSEUDO(".DICD",TOK_DICD),
//	PSEUDO(".DIMM",TOK_DIMM),
	PSEUDO(".DIO",TOK_DIO),
	PSEUDO(".DIOA",TOK_DIOA),
	PSEUDO(".DISD",TOK_DISD),
	PSEUDO(".DISS",TOK_DISS),
	PSEUDO(".DMR",TOK_DMR),
	PSEUDO(".DMRA",TOK_DMRA),
	PSEUDO(".DUSR",TOK_DUSR),
//	PSEUDO(".DXOP",TOK_DXOP),
	PSEUDO(".XPNG",TOK_XPNG),
/* location counter pseudo-ops */
	PSEUDO(".BLK",TOK_BLK),
	PSEUDO(".NREL",TOK_NREL),
	PSEUDO(".ZREL",TOK_ZREL),
/* intermodule communication pseudo-ops */
	PSEUDO(".COMM",TOK_COMM),
	PSEUDO(".CSIZ",TOK_CSIZ),
	PSEUDO(".ENT",TOK_ENT),
	PSEUDO(".ENTO",TOK_ENTO),
	PSEUDO(".EXTD",TOK_EXTD),
	PSEUDO(".EXTN",TOK_EXTN),
	PSEUDO(".EXTU",TOK_EXTU),
	PSEUDO(".GADD",TOK_GADD),
	PSEUDO(".GLOC",TOK_GLOC),
	PSEUDO(".GREF",TOK_GREF),
/* repetition and conditional pseudo-ops */
//	PSEUDO(".DO",TOK_DO),
	PSEUDO(".ENDC",TOK_ENDC),
//	PSEUDO(".GOTO",TOK_GOTO),
	PSEUDO(".IFE",TOK_IFE),
	PSEUDO(".IFG",TOK_IFG),
	PSEUDO(".IFL",TOK_IFL),
	PSEUDO(".IFN",TOK_IFN),
/* macro definition pseudo-op and values */
//	PSEUDO(".MACRO",TOK_MACRO),
//	PSEUDO(".ARGCT",TOK_ARGCT),
//	PSEUDO(".MCALL",TOK_MCALL),
/* stack pseudo-ops and values */
//	PSEUDO(".PUSH",TOK_PUSH),
//	PSEUDO(".POP",TOK_POP),
//	PSEUDO(".TOP",TOK_TOP),
/* text string pseudo-ops and values */
	PSEUDO(".TXT",TOK_TXT),
	PSEUDO(".TXTE",TOK_TXTE),
	PSEUDO(".TXTF",TOK_TXTF),
	PSEUDO(".TXTM",TOK_TXTM),
	PSEUDO(".TXTN",TOK_TXTN),
	PSEUDO(".TXTO",TOK_TXTO),
/* listing pseudo-ops and values */
	PSEUDO(".EJEC",TOK_EJEC),
	PSEUDO(".NOCON",TOK_NOCON),
	PSEUDO(".NOLOC",TOK_NOLOC),
	PSEUDO(".NOMAC",TOK_NOMAC),
/* miscellaneous pseudo-ops */
	PSEUDO(".REV",TOK_REV),
	PSEUDO(".RDX",TOK_RDX),
	PSEUDO(".RDXO",TOK_RDXO),
	PSEUDO(".TITL",TOK_TITL),
	PSEUDO(".RB",TOK_RB),
	PSEUDO(".LMIT",TOK_LMIT),
	PSEUDO(".PASS",TOK_PASS),
	PSEUDO(".EOF",TOK_EOF),
	PSEUDO(".EOT",TOK_EOT),
	PSEUDO(".END",TOK_END),

	PSEUDO(".LOC",TOK_LOC),
	PSEUDO(".",TOK_DOT), /* symbol means: value of the location counter */

	{0,0,0,0,0,0,0,0,0}
};

void composite(char *mnemonic,char *s1,char *s2,int op,int tok){
	struct sym_rec *s;
	char mn[40],*q;

	NEW(s);
	/* form composite mnemonic by adding suffixes */
	q = scpy(mn,mnemonic);
	q = scpy(q,s1);
	scpy(q,s2);
	s->name = sdup(mn);
	s->value = op;
	s->token = tok;
	s->flags = F_ASSIGNED;
	if(debug && lookup(mn))
		printf("!! duplicate predefined symbol: %s (composite mnemonic)\n",mn);
	insert(s);
	/* DPRINTF("# composite(%s,%s,%s,%#o,%d)\n",mnemonic,s1,s2,op,tok); */
}

char *class_c[]  = {
		"",
		"Z", // instruction will initialise the carry bit to 0
		"O", // instruction will initialise the carry bit to 1
		"C", // instruction will complement the carry bit
		0},
	 *class_sh[] = {
	 	"",
	 	"L", // instruction will combine carry and result, and rotate left
	 	"R", // instruction will combine carry and result, and rotate right
	 	"S", // instruction will exchange 8-bit halves of 16-bit result
	 	0},
	 *class_f[]  = {
	 	"",
	 	"S", // instruction will start the device (Busy=1, Done=0)
	 	"C", // instruction will idle the device (Busy=0, Done=0)
	 	"P", // instruction will pulse the special in-out bus control line
	 	0},
	 *class_t[]  = {
	 	"BN", // instruction tests for Interrupt On = 1
	 	"BZ", // instruction tests for Interrupt On = 0
	 	"DN", // instruction tests for Power Fail = 1
	 	"DZ", // instruction tests for Power Fail = 0
	 	0},
	 *twoac_op[8],*io_op[8],*noac_op[4],*oneac_op[4],*skips[8];

void iodata(char *mnemonic,int opcode,char **class){
	char **fsuf;
	int f;

	for( fsuf=class,f=0 ; *fsuf ; f++,fsuf++ )
		composite(mnemonic,*fsuf,"",060000|opcode|(f<<6),TOK_IO);
}

void twoac(char *mnemonic,int opcode){
	char **csuf,**shsuf;
	int c,sh;

	for( csuf=class_c,c=0 ; *csuf ; c++,csuf++ )
		for( shsuf=class_sh,sh=0 ; *shsuf ; sh++,shsuf++ )
			composite(mnemonic,*csuf,*shsuf,
				      0100000|opcode|(sh<<6)|(c<<4),TOK_TWOAC);
}

void cpu_initsyms(){
	struct sym_rec *p;

	twoac(twoac_op[0] = "COM",0<<8);
	twoac(twoac_op[1] = "NEG",1<<8);
	twoac(twoac_op[2] = "MOV",2<<8);
	twoac(twoac_op[3] = "INC",3<<8);
	twoac(twoac_op[4] = "ADC",4<<8);
	twoac(twoac_op[5] = "SUB",5<<8);
	twoac(twoac_op[6] = "ADD",6<<8);
	twoac(twoac_op[7] = "AND",7<<8);

	iodata(io_op[0] = "NIO",0<<8,class_f);
	iodata(io_op[1] = "DIA",1<<8,class_f);
	iodata(io_op[2] = "DOA",2<<8,class_f);
	iodata(io_op[3] = "DIB",3<<8,class_f);
	iodata(io_op[4] = "DOB",4<<8,class_f);
	iodata(io_op[5] = "DIC",5<<8,class_f);
	iodata(io_op[6] = "DOC",6<<8,class_f);
	iodata(io_op[7] = "SKP",7<<8,class_t);

	skips[0] = "   ";
	for(p=predefs;p->name;++p)
		if(p->token == TOK_NOAC)
			noac_op[(p->value>>11)&3] = p->name;
		else if(p->token == TOK_ONEAC)
			oneac_op[(p->value>>13)&3] = p->name;
		else if(p->token == TOK_SKIP)
			skips[p->value] = p->name;
}
