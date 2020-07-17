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
#include "parser.tab.h"

#define MRI(S,V)    {NULL,S,V,TOK_MRI,F_MRI,0,0,0,0}
#define NM(S,V)     {NULL,S,V,TOK_SYM,0,0,0,0,0}
#define PSEUDO(S,T) {NULL,S,0,T,0,0,0,0,0}
#define IOT NM
#define IOT1 IOT /* FIXME: IOT1 instructions take one operand */

struct sym_rec predefs[] = {
	/* pseudo-instructions (directives) */
	PSEUDO("FIELD",TOK_FIELD),
	PSEUDO("EXPUNGE",TOK_EXPUNGE),
	PSEUDO("FIXMRI",TOK_FIXMRI),
	PSEUDO("PAUSE",TOK_PAUSE),
	PSEUDO("FIXTAB",TOK_FIXTAB),
	PSEUDO("DECIMAL",TOK_DECIMAL),
	PSEUDO("OCTAL",TOK_OCTAL),

	/* memory reference instructions */
	MRI("AND",00000),
	MRI("TAD",01000),
	MRI("ISZ",02000),
	MRI("DCA",03000),
	MRI("JMS",04000),
	MRI("JMP",05000),

	/* floating point instructions */
	MRI("FEXT",00000),
	MRI("FADD",01000),
	MRI("FSUB",02000),
	MRI("FMPY",03000),
	MRI("FDIV",04000),
	MRI("FGET",05000),
	MRI("FPUT",06000),
	MRI("FNOR",07000),

	/* group 1 operates */
	NM("OPR",07000), NM("NOP",07000),
	NM("IAC",07001),
	NM("RAL",07004),
	NM("RTL",07006),
	NM("RAR",07010),
	NM("RTR",07012),
	NM("CML",07020),
	NM("CMA",07040),
	NM("CLL",07100),
	NM("CLA",07200),
	NM("BSW",07002),

	/* group 2 operates */
	NM("HLT",07402),
	NM("OSR",07404),

	/* combined operates */
	NM("CIA",07041),
	NM("LAS",07604),
	NM("SKP",07410),
	NM("SNL",07420),
	NM("SZL",07430),
	NM("SZA",07440),
	NM("SNA",07450),
	NM("SMA",07500),
	NM("SPA",07510),
	NM("STL",07120),
	NM("GLK",07204),
	NM("STA",07240),

	NM("MQL",07421),
	NM("MQA",07501),
	NM("CAM",07621),
	NM("SWP",07521),
	NM("ACL",07701),

	/* basic IOT microinstructions (from Intro, D1-8) */
	/* Program Interrupt */
	IOT("IOT",06000), IOT("SKON",06000),
	IOT("ION",06001),
	IOT("IOF",06002),
	IOT("SRQ",06003),
	IOT("GTF",06004),
	IOT("RTF",06005),
	IOT("SGT",06006),
	IOT("CAF",06007),

	/* High Speed Perforated Tape Reader and Control */
	IOT("RSF",06011),
	IOT("RRB",06012),
	IOT("RFC",06014),
	IOT("RPE",06010),
	IOT("RCC",06016),
	IOT("PCE",06020),
	
	/* High Speed Perforated Tape Punch and Control */
	IOT("PSF",06021),
	IOT("PCF",06022),
	IOT("PPC",06024),
	IOT("PLS",06026),

	/* Teletype Keyboard/Reader */
	IOT("KSF",06031),
	IOT("KCC",06032),
	IOT("KRS",06034),
	IOT("KRB",06036),
	IOT("KCF",06030),
	IOT("KIE",06035),

	/* Teletype Teleprinter/Punch */
	IOT("TSF",06041),
	IOT("TCF",06042),
	IOT("TPC",06044),
	IOT("TLS",06046),
	IOT("TFL",06040),
	IOT("TSK",06045),

	/* Oscilloscope Display Type VC8/I
	   and Precision CRT Display Type 30N */
	IOT("DCX",06051),
	IOT("DXL",06053),
	IOT("DIX",06054),
	IOT("DXS",06057),
	IOT("DCY",06061),
	IOT("DYL",06063),
	IOT("DIY",06064),
	IOT("DYS",06067),

	/* Oscilloscope Display Type VC8/I */
/*	IOT("DSB",06074-06077),*/
	
	/* Precision CRT Display Type 30N */
	IOT("DLB",06074),

	/* Light Pen Type 370 */
	IOT("DSF",06071),
	IOT("DCF",06072),

	/* Memory Parity Type MP8/I */
	IOT("SMP",06101),
	IOT("CMP",06104),

	/* Automatic Restart Type KP8/I */
	IOT("SPL",06102),

	/* Memory Extension Control Type MC8/I */
	IOT1("CDF",06201),
	IOT1("CIF",06202),
	IOT("RDF",06214),
	IOT("RIF",06224),
	IOT("RIB",06234),
	IOT("RMF",06244),
	IOT1("CDI",06203),

	/* Data Communications Systems Type 680 */
	IOT("TTINCR",06401),
	IOT("TTI",06402),
	IOT("TTO",06404),
	IOT("TTCL",06411),
	IOT("TTSL",06412),
	IOT("TTRL",06414),
	IOT("TTSKP",06421),
	IOT("TTXON",06424),
	IOT("TTXOF",06422),

	/* Incremental Plotter and Control Type VP8/I */
	IOT("PLSF",06501),
	IOT("PLCF",06502),
	IOT("PLPU",06504),
	IOT("PLPR",06511),
	IOT("PLDU",06512),
	IOT("PLDD",06514),
	IOT("PLPL",06521),
	IOT("PLUD",06522),
	IOT("PLPD",06524),

	/* Serial Magnetic Drum System Type 251 */
	IOT("DRCR",06603),
	IOT("DRCW",06605),
	IOT("DRCF",06611),
	IOT("DREF",06612),
		IOT("DRES",06612),/*Type RM08*/
	IOT("DRTS",06615),
	IOT("DRSE",06621),
	IOT("DRSC",06622),
	IOT("DRCN",06624),
		IOT("DRFS",06624),/*Type RM08*/

	/* Random Access Disc File (Type DF32) */
	IOT("DCMA",06601),
	IOT("DMAR",06603),
	IOT("DMAW",06605),
	IOT("DCEA",06611),
	IOT("DSAC",06612),
	IOT("DEAL",06615),
	IOT("DEAC",06616),
	IOT("DFSE",06621),
	IOT("DFSC",06622),
	IOT("DMAC",06626),

	/* Automatic Line Printer and Control Type 645 */
	IOT("LSE",06651),
	IOT("LCB",06652),
	IOT("LLB",06654),
	IOT("LSD",06661),
	IOT("LCF",06662),
	IOT("LPR",06664),

	/* DECtape Transport Type TU55 and DECtape Control Type TC01 */
	IOT("DTRA",06761),
	IOT("DTCA",06762),
	IOT("DTXA",06764),
	IOT("DTSF",06771),
	IOT("DTRB",06772),
	IOT("DTLB",06774),

	/* Card Reader and Control Type CR8/I */
	IOT("RCSF",06631),
	IOT("RCSA",06632),
	IOT("RCRB",06634),
	IOT("RCSP",06671),
	IOT("RCSE",06672),
	IOT("RCRD",06674),

	/* Automatic Magnetic Tape Control Type TC58 */
	IOT("MTSF",06701),
	IOT("MTCR",06711),
	IOT("MTTR",06721),
	IOT("MTAF",06712),
/*	IOT("--",06724),*/
	IOT("MTCM",06714),
	IOT("MTLC",06716),
/*	IOT("--",06704),*/
	IOT("MTRS",06706),
	IOT("MTGO",06722),
/*	IOT("--",06702),*/

	/* General Purpose Converter and Multiplexer Control Type AF01A */
	IOT("ADSF",06531),
	IOT("ADVC",06532),
	IOT("ADRB",06534),
	IOT("ADCC",06541),
	IOT("ADSC",06542),
	IOT("ADIC",06544),

	/* Guarded Scanning Digital Voltmeter Type AF04A */
	IOT("VSEL",06542),
	IOT("VCNV",06541),
	IOT("VINX",06544),
	IOT("VSDR",06531),
	IOT("VRD", 06532),
	IOT("VBA", 06534),
	IOT("VSCC",06571),

	{0,0,0,0,0,0,0,0,0} /* mark end of table */
};
