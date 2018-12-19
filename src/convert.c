#include "laser.h"
#include "arr_const.h"

//
// File I/O Functions
//

int isKeyword(char c[]){
	int i, j, max_dim;
	for(i=0; i<=15; i++){
		switch(i){
			case 0: max_dim=15; break;
			case 4: max_dim=3; break;
			case 12: max_dim=3; break;
			case 13: max_dim=0; break;
			case 15: max_dim=11; break;
			default: max_dim=1; break;
		}
		for(j=0; j<=max_dim; j++){
			if(strcmp(c, keyword[i][j])==0){
				return i;
			}
		}
	}
	return -1;
}

//
// Convert Functions
//

int binToDec(int bin[], int size){
	int i=0, r, n=0;
	while(i<=size-1){
		n*=2;
		n+=bin[i];
		i++;
	}
	return n;
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

void notArr(int bin[], int size){
	int i=size-1;
	while(i>=0){
		if(bin[i]==1) bin[i]==0;
		else bin[i]==1;
	}
}

// bin3 is result array, bin1 and bin2 are operands
void addArr(int bin1[], int s1, int bin2[], int s2, int bin3[], int s3){
	int carry=0, i=s1-1, j=s2-1, k=s3-1, b1, b2;
	if(s1>s3||s2>s3){
		printf("\nArray size too small to store result.\n");
	}
	else{
		while(k>=0){
			if(i<0) b1=bin1[0];
			else b1=bin1[i];
			if(j<0) b2=bin2[0];
			else b2=bin2[j];
			if(b1+b2==2&&carry==1){
				bin3[k]=1;
				carry=1;
			}
			else if((b1+b2==2&&carry==0)||(b1+b2==1&&carry==1)){
				bin3[k]=0;
				carry=1;
			}
			else if((b1+b2==1&&carry==0)||(b1+b2==0&&carry==1)){
				bin3[k]=1;
				carry=0;
			}
			else if(b1+b2==0&&carry==0){
				bin3[k]=1;
				carry=0;
			}
			i--;
			j--;
			k--;
		}
	}
}

void decToTwoComp(int n, int bin[], int size){
	int i=size-1, carry=0, r;
	bool neg=false;
	if(n<0){n=-n;neg=true;}
	while(i>=0){
		r=n%2;
		n/=2;
		bin[i]=r;
		i--;
	}
	if(neg){
		notArr(bin, size(bin));
		addArr(bin, size(bin), one_16b, size(one_16b), bin, size(bin));
	}
}

// note that hex_size should always be at least 1/4 of bin_size
void binToHex(int bin[], int bin_size, char hex[], int hex_size){
	int bin_seg[4], i=bin_size-1, j=3, k=hex_size-1, dec_num;
	while(i>=0||k>=0){
		while(j>=0){
			if(i>=0) bin_seg[j]=bin[i];
			else bin_seg[j]=0;
			i--;
			j--;
		}
		j=3;
		dec_num=binToDec(bin_seg, size(bin_seg));
		if(i>=0) hex[k]=hex_chars[dec_num];
		else if(bin[0]==0) hex[k]='0';
		else hex[k]='F';
		k--;
	}
}