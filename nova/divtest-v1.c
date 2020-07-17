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
			for( bit=1,sub=dvr ; dvd>=sub<<1 ; bit<<=1,sub<<=1 )
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

main(int argc,char *argv[]){
	if(argc>2)
			printf("=%d check quo=%d rem=%d\n", div(atoi(argv[1]),atoi(argv[2])), 
							atoi(argv[1])/atoi(argv[2]),
							atoi(argv[1])%atoi(argv[2]));
}

