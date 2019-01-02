#include "convert.h"
#include <stdio.h>

//==============================================================================
// Type Functions
//==============================================================================

int isKeyword(char c[]){
	int max_dim;
	for(int i=0; i<=15; i++){
		switch(i){
		case 0: max_dim=15; break;
		case 4: max_dim=3; break;
		case 12: max_dim=3; break;
		case 13: max_dim=0; break;
		case 15: max_dim=13; break;
		default: max_dim=1; break;
		}
		for(int j=0; j<=max_dim; j++){
			if(strcmp(c, keyword[i][j])==0)
				return i;
		}
	}
	return -1;
}

int isPseuodoOp(char c[]){
	for(int i=0; i<=4; i++){
		for(int j=0; j<=1; j++){
			if(strcmp(c, pseudoop[i][j])==0)
				return i;
		}
	}
	return -1;
}

int isRegister(char c[]){
	for(int i=0; i<=7; i++){
		for(int j=0; j<=1; j++){
			if(strcmp(c, regs[i][j])==0)
				return i;
		}
	}
	return -1;
}

// extends isxdigit functionality by returning decimal value (0-16)
int isHexChar(char c){
	for(int i=0; i<=15; i++){
		if((i<=9)&&(c==hex_chars[i]))
			return i;
		else if((i>9)&&((c==hex_chars[i])||(c==hex_chars[i]+0x20)))
			return i;
	}
	return -1;
}

// printf("Invalid hex number at line %d: %s\n", ln, c);
// printf("Invalid binary number at line %d: %s\n", ln, c);

int isValidOffset(char c[]){
	int i=1;
	if(c[0]==0x78){
		if(c[MAX_WORD_SIZE+1]<2) return 0;
		while(c[i]!=0x00){
			if(isHexChar(c[i])<0) return 0;
			i++;
		}
		return 1;
	}
	else if(c[0]==0x62){
		if(c[MAX_WORD_SIZE+1]<2) return 0;
		while(c[i]!=0x00){
			if((c[i]!=0x30)&&(c[i]!=0x31)) return 0;
			i++;
		}
		return 2;
	}
	else if((c[0]==0x2D)||isdigit(c[0])){
		if((c[0]==0x2D)&&(c[MAX_WORD_SIZE+1]<2)) return 0;
		while(c[i]!=0x00){
			if(!isdigit(c[i])) return 0;
			i++;
		}
		return 3;
	}
	else return 0;
}

int isLabel(char c[]){
	if((isKeyword(c)<0)&&(isPseuodoOp(c)<0)&&(isValidOffset(c)==0)) return 1;
	else return 0;
}

//==============================================================================
// Convert Functions
//==============================================================================

// binary conversions
int binToDec(int bin[], int size){
	int i=0, r, n=0;
	while(i<=size-1){
		n*=2;
		n+=bin[i];
		i++;
	}
	return n;
}

void zext(int n, int bin[], int size){
	int i=size-1, r;
	while(i>=0){
		r=n%10;
		n/=10;
		bin[i]=r;
		i--;
	}
}

// binary operations
void notArr(int bin[], int size){
	for(int i=0; i<size; i++){
		switch(bin[i]){
			case 0: bin[i]=1; break;
			case 1: bin[i]=0; break;
		}
	}
}

// bin3 is result array, bin1 and bin2 are operands
void addArr(int bin1[], int s1, int bin2[], int s2, int bin3[], int s3){
	int carry=0, i=s1-1, j=s2-1, k=s3-1, b1, b2;
	while(k>=0){
		if(i<0) b1=bin1[0];
		else b1=bin1[i];
		if(j<0) b2=bin2[0];
		else b2=bin2[j];
		switch(b1+b2+carry){
		case 0: bin3[k]=0; carry=0; break;
		case 1: bin3[k]=1; carry=0; break;
		case 2: bin3[k]=0; carry=1; break;
		case 3: bin3[k]=1; carry=1; break;
		}
		i--;
		j--;
		k--;
	}
}

// two's complement conversions
void decToTwoComp(int n, int bin[], int size){
	int i=size-1, carry=0, r, m=n;
	if(n<0)
		m *= -1;
	while(i>=0){
		r=m%2;
		m/=2;
		bin[i]=r;
		i--;
	}
	if(n<0){
		notArr(bin, 16);
		addArr(bin, 16, one_16b, 16, bin, 16);
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

// hex conversions
// note: hex_size should always be at 1/4 of bin_size
void binToHex(int bin[], int bin_size, char hex[], int hex_size){
	if(bin_size==4*hex_size){
		for(int i=0; i<hex_size; i++){
			hex[i]=hex_chars[binToDec(&bin[4*i], 4)];
		}
	}
}

int hexToDec(char hex[]){
	int i=1, dec_num=0;
	while(hex[i]!=0x00){
		dec_num=dec_num*16+isHexChar(hex[i]);
		i++;
	}
	i--;
	if(isHexChar(hex[1])>=8) dec_num=dec_num-pow(2, (4*i));
	return dec_num;
}

// note: only handles positive numbers, intended for use with addresses
// DO NOT use to calculate offsets
int addrToDec(char hex[]){
	int i, j, dec_num=0;
	for(j=1; j<=4; j++){
		for(i=0; i<=15; i++){
			if(hex[j]==hex_chars[i]){
				dec_num=dec_num*16+i;
			}
		}
	}
	return dec_num;
}

// note: only handles positive numbers, intended for use with addresses
// DO NOT use to calculate offsets
char* decToAddr(char hex[], int dec_num){
	int i=3, r;
	hex[4]=0x00;			// null terminate
	while(dec_num>0&&i>=0){
		r=dec_num%16;
		dec_num/=16;
		hex[i]=hex_chars[r];
		i--;
	}
	return hex;
}