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
//#include "fnv.h"

/* following constant (need not) be prime. for a list of prime numbers, 
   see http://www.utm.edu/research/primes/lists/small/1000.txt */
#define TABLE_SIZE 1021
//#define HASH(s) (fnv_32_str(s,FNV1_32_INIT) % TABLE_SIZE)
#define HASH(s) (djb2(s) % TABLE_SIZE)

struct heapnode{
	struct heapnode *left,*right;
	struct sym_rec *sym;
};
struct heapnode *makeheap(void);
struct sym_rec *hash_table[TABLE_SIZE];
unsigned maxlen;

extern struct sym_rec predefs[];
extern int symflag;

// hash function recommended by Ozan Yigit 
// http://www.cs.yorku.ca/~oz/hash.html
// "this algorithm (k=33) was first reported by dan bernstein"
unsigned long djb2(unsigned char *str){
	unsigned long hash = 5381;
	unsigned c;

	while ( (c = *str++) )
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

void init_symtab(){
	struct sym_rec *p;
	int i;

	for(i = TABLE_SIZE; i--;)
		hash_table[i] = 0;
	for(p = predefs; p->name; p++){
		if(debug && lookup(p->name))
			printf("!!!! duplicate predefined symbol: %s\n",p->name);
		p->flags |= F_ASSIGNED;
		insert(p);
	}
	cpu_initsyms();
}

void dump_symbols(){
	struct sym_rec *p;
	int i,occ,maxchain,chain;

	puts("\nsymbol table:");
	for(i = occ = maxchain = 0; i < TABLE_SIZE; i++)
		if(hash_table[i]){
			++occ;
			for(p = hash_table[i], chain = 0; p; p = p->next){
				++chain;
				if(  /*debug ||*/  (p->flags & F_USER) ){
					printf("  %-10s",p->name);
					if(p->flags & F_ASSIGNED){
						printf(" = %c%#7o",p->value<0?'-':' ',abs(p->value));
						DPRINTF(" token=%d",p->token);
						printf(" relmode=%d",p->relmode);
						if(p->lineno)
							printf("  (line %2d)\n",p->lineno);
						else putchar('\n');
					}
					else
						puts(" (unassigned)");
				}
			}
			if(chain>maxchain)
				maxchain = chain;
		}
	DPRINTF("# hash stats: occupancy=%d/%d (%.1f%%) longest chain=%d\n",
		occ,TABLE_SIZE,(100.*occ)/TABLE_SIZE,maxchain);
}

struct sym_rec *lookup(char *s){
	struct sym_rec *p;
	unsigned idx = HASH((unsigned char*)s);
	/* DPRINTF("# lookup \"%s\" hash=%5d\n",s,idx); */
	for(p = hash_table[idx]; p; p = p->next)
		if(!strcmp(s,p->name))
			return p;
	return 0; /* not found */
}

void insert(struct sym_rec *p){
	unsigned idx = HASH((unsigned char*)(p->name));
	p->next = hash_table[idx];
	hash_table[idx] = p;
	/* DPRINTF("# insert symbol [%5d] \"%s\"\n",idx,p->name);*/
}

void expunge(){
	int i;
	struct sym_rec *p,*q;

	for(i = 0; i < TABLE_SIZE; i++){
		for(p = hash_table[i]; p; p = q){
			DPRINTF(" deleting symbol \"%s\"\n",p->name);
			q = p->next;
			if(p->flags & F_USER)
				free(p);
		}
		hash_table[i] = 0;
	}
}
struct sym_rec *freechain(struct sym_rec *p,int l){
	struct sym_rec *newnext;

	if(p){
		newnext = freechain(p->next,l+1);
		/*while(l--) DPRINTF("  ");
		DPRINTF("freechain(%#x) \"%s\"",p,p->name);*/
		if( p->flags & F_USER ){
			/*DPRINTF(" FREED; head is now %#x\n",newnext);*/
			DPRINTF("deleting symbol \"%s\"\n",p->name);
			free(p);
			return newnext;/* this node is gone, return next*/
		}else{
			/*DPRINTF(" KEPT; next is %#x\n",newnext);*/
			p->next = newnext;
			return p;/* keep this node*/
		}
	}
	return 0;
}

void clean_syms(){
	int i;

	for(i = 0; i < TABLE_SIZE; i++)
		hash_table[i] = freechain(hash_table[i],0);
}

struct sym_rec *dosymbol(char *yytext,int tok){ 
	struct sym_rec *p; 

	if(!casesense) uppercase(yytext);
	p = lookup(yytext);
	if(p){
		DPRINTF("known symbol \"%s\" = %#o (token=%d)\n", 
			p->name,p->value,p->token);
	}else{ /* unknown symbol */
		NEW(p);
		p->name = sdup(yytext);
		p->value = 0;
		p->token = tok;
		p->flags = symflag; /* normally F_USER unless in 'header' file (-s) */
		p->type = USER_SYMBOL;
		p->pageno = pageno;
		p->lineno = yylineno;
		insert(p);
		DPRINTF("new symbol \"%s\"\n",p->name);
	}
	return p;
}

/* heap structure:
		root of a tree is greater than or equal to both children.
		left child is less than or equal to right child.
*/

struct heapnode *heapinsert(struct heapnode *root,struct heapnode *newnode){
	if(root==NULL || strcmp(root->sym->name,newnode->sym->name) <= 0){
		newnode->left = root;
		return newnode;
	}else {
		if(root->left==NULL || strcmp(newnode->sym->name,root->left->sym->name) <= 0)
			root->left = heapinsert(root->left,newnode);
		else 
			root->right = heapinsert(root->right,newnode);
	}
	return root;
}

void heapdump(struct heapnode *root){
	if(root){
		heapdump(root->left);
		heapdump(root->right);
		printsym(root->sym);
		free(root);
	}
}

struct heapnode *makeheap(void){
	struct heapnode *root = NULL,*q;
	struct sym_rec *p;
	int i;

	for(i = maxlen = 0; i < TABLE_SIZE; i++){
		if(hash_table[i]){
			for(p = hash_table[i]; p; p = p->next){
				if(  /*debug ||*/  (p->flags & F_USER) ){
					/*if(p->flags & F_ASSIGNED)*/ {
						NEW(q);
						q->left = q->right = NULL;
						q->sym = p;
						if(strlen(p->name) > maxlen)
							maxlen = strlen(p->name);
						root = heapinsert(root,q);
					}
				}
			}
		}
	}
	return root;
}

void list_symbols(){
	struct heapnode *heap = makeheap();
	fputc('\n',listfile);
	heapdump(heap);
}
