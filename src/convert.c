#include "laser.h"
#include "arr_const.h"

int binToDec(int bin[], int size){
	int dec_num=0, i=0, r, n=0;
	while(i<=size-1){
		n*=10;
		n+=bin[i];
		i++;
	}
	return dec_num;
}

void zext(unsigned int n, int bin[], int size){
	int i=size-1, r;
	while(i>=0){
		r=n%10;
		n/=10;
		bin[i]=r;
		i--;
	}
}

int twoCompToDec(int bin[], int size){
	int dec_num=0, i=0, r, n=0;
	while(i<=size-1){
		n*=10;
		n+=bin[i];
		i++;
	}
	i=0;
	while(n>0){
		r=n%10;
		n/=10;
		dec_num+=r*pow(2, i);
		i++;
	}
	if(bin[0]==1) dec_num-=r*pow(2, i);
	return dec_num;
}

void decToTwoComp(int n, int bin[], int size){
	int bin_num=0, i=0, mask=0, carry=0, r;
	bool neg=false, stillOnes=true;
	if(n<0){n=-n;neg=true;}
	while(n>0){
		r=n%2;
		n/=2;
		bin_num+=r*pow(10, i);
		i++;
	}
	if(neg){
		while(i>=0){
			mask*=10;
			mask++;
			i--;
		}
		bin_num=n=abs(bin_num-mask);
		while(n>0&&stillOnes){
			r=n%10;
			n/=10;
			if(r==1){
				carry*=10;
				carry++;
			}
			else{
				stillOnes=false;
			}
		}
		if(bin_num%2==1){
			bin_num+=8*carry;
			bin_num=bin_num+1;
		}
		else{
			bin_num+=1;
		}
	}
	i=size-1;
	while(i>=0){
		bin[i]=bin_num%10;
		bin_num/=10;
		if(neg&&bin_num==0) bin_num++;
		i--;
	}
}

char binToHex(int bin[], int bin_size, char hex[]){

}