#    This file is part of The Didactic PDP-8 Assembler
#    Copyright (C) 2002 Toby Thain, toby@telegraphics.com.au
#
#    This program is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by  
#    the Free Software Foundation; either version 2 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License  
#    along with this program; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# public domain FNV hash library used with permission
# see http://www.isthe.com/chongo/tech/comp/fnv/

# lexer requires features of GNU flex 
# including exclusive start conditions, and YY_USER_ACTION
LEX = flex

# prefer GNU bison as a parser generator, although yacc should also work
YACC = bison -y

CFLAGS += -O2 -W -Wall -I.

OBJ_COMMON = main.o str.o error.o assign.o symtab.o object.o gpl.o list.o

OBJ_PDP8 = pdp8/lexer.o pdp8/parser.tab.o pdp8/obj.o pdp8/list.o \
	pdp8/initsyms.o pdp8/predefs.o pdp8/instr.o pdp8/disasm.o \

OBJ_NOVA = nova/lexer.o nova/parser.tab.o nova/obj.o nova/list.o \
	nova/initsyms.o nova/instr.o nova/disasm.o \
	nova/partables.o nova/rb.o nova/radix50.o

PROGRAMS = p8a dga

all : $(PROGRAMS) 

nova/parity : nova/parity.o

nova/partables.c : nova/parity
	nova/parity > nova/partables.c

nova/dumprb :
	cd nova && make dumprb

%.tab.c %.tab.h : %.y
	$(YACC) -d $<
	mv y.tab.c $*.tab.c
	mv y.tab.h $*.tab.h

p8a : $(OBJ_COMMON) $(OBJ_PDP8)
	$(CC) -o $@ $^

dga : $(OBJ_COMMON) $(OBJ_NOVA)
	$(CC) -o $@ $^

# include dependencies

assign.o : asm.h
error.o : asm.h
gpl.o : asm.h
list.o : asm.h
main.o : asm.h version.h
obj.o : asm.h nova/rb.h
str.o : asm.h
symtab.o : asm.h

nova/disasm.o : asm.h nova.h
nova/initsyms.o : asm.h nova/parser.tab.c nova.h
nova/instr.o : asm.h  nova.h nova/rb.h
nova/lexer.o : nova/lexer.l nova/parser.tab.c asm.h nova/rb.h nova.h
nova/parser.tab.o : nova/parser.y asm.h nova/rb.h nova.h

pdp8/disasm.o : asm.h pdp8.h
pdp8/initsyms.o : asm.h pdp8/parser.tab.c pdp8.h
pdp8/instr.o : asm.h  pdp8.h
pdp8/lexer.o : pdp8/lexer.l pdp8/parser.tab.c asm.h pdp8.h
pdp8/parser.tab.o : pdp8/parser.y asm.h pdp8.h
pdp8/predefs.c : asm.h pdp8/parser.tab.c pdp8.h

clean: 
	rm -f $(PROGRAMS) nova/parity nova/partables.c nova/dumprb \
		$(OBJ_COMMON) $(OBJ_NOVA) $(OBJ_PDP8) \
		pdp8/parser.tab.[ch] pdp8/y.tab.[ch] pdp8/lexer.c \
		nova/parser.tab.[ch] nova/y.tab.[ch] nova/lexer.c \
		core \
		pdp8/examples/*.pal.* nova/examples/*.sr.*

test: p8a dga nova/dumprb
	./p8a -v pdp8/examples/*.pal
	./dga -v -l -s nova/rdos/osid.sr nova/examples/hello2.sr
	nova/dumprb nova/examples/hello2.sr.rb

