#include <stdio.h>

int div(int dvd,int dvr){
	int quo,sub,bit,sign,dvdsign;

	quo = 0;
	if(dvd<0){ 
		dvd = -dvd; dvdsign = sign = -1; 
	}else 
		dvdsign = sign = 1;
	if(dvr<0){
		dvr = -dvr; sign = -sign;
	}
	if(dvr){
			/* shift divisor left until next shift would exceed dividend */
			for( bit=1,sub=dvr ; dvd >= sub<<1 ; bit <<= 1, sub <<= 1 )
				;
			do{
				printf("dvd=%d sub=%d quo=%d\n",dvd,sub,quo);
				if(dvd>=sub){
					dvd -= sub;
					quo += bit;
				}
				sub >>= 1;
				bit >>= 1;
			}while(bit);
			printf("quo=%d*%d rem=%d\n",sign,quo,dvdsign*dvd);
	}else printf("division by zero!\n");
	return sign*quo;
}

/* method taken from Nova Programmer's Reference, example #20 
   This is long division, in binary. */
int div2(long dd,int dr){
	enum{ WORDBITS = 16, WORDMASK = (1U << WORDBITS)-1 };
	long divhi = (dd >> WORDBITS) & WORDMASK,divlo = dd & WORDMASK;
	int c,step;
	
	dr &= WORDMASK;
	if(divhi > dr)
		printf("cannot divide %d by %d: result would overflow a single word",dd,dr);
	else{
		c = (divlo <<= 1) >> WORDBITS; divlo &= WORDMASK;
		if(c) puts("left shift divlo SET carry");
		for( step=0 ; step<WORDBITS ; ++step ){
			printf("step %2d: divlo=%5d divhi=%5d c=%d\n",step,divlo,divhi,c);
			divhi = (divhi << 1) | c;
			c = 0;
			if(dr <= divhi){
				printf("dr(%5d) <= divhi(%5d): subtract dr from divhi\n",dr,divhi);
				divhi -= dr; 
				c = 1;
			}
			divlo = (divlo << 1) | c; c = divlo >> WORDBITS; divlo &= WORDMASK;
			if(c) puts("left shift divlo SET carry");
		}
		printf("after loop: divlo(quo)=%5d divhi(rem)=%5d c=%d\n",divlo,divhi,c);
	}
	return divlo; /* quotient */
}

main(int argc,char *argv[]){
	if(argc>2)
			printf("=%d check quo=%d rem=%d\n", div2(atoi(argv[1]),atoi(argv[2])), 
							atoi(argv[1])/atoi(argv[2]),
							atoi(argv[1])%atoi(argv[2]));
}

