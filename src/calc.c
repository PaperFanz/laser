#include "laser.h"

// .bin generator functions
int fillRegister(int r, int bin[], int n){
	if(n==2) n++;
	int m=4+(3*n);
	switch(r){
		case 0: break;
		case 1: bin[m+2]=1; break;
		case 2: bin[m+1]=1; break;
		case 3: bin[m+2]=bin[m+1]=1; break;
		case 4: bin[m]=1; break;
		case 5: bin[m+2]=bin[m]=1; break;
		case 6: bin[m+1]=bin[m]=1; break;
		case 7: bin[m+2]=bin[m+1]=bin[m]=1; break;
		default: return 0;
	}
	return 1;
}

// read a fixed offset or immediate value and express it in n bits
int fillOffset(int type, char c[], int bits, int ln, int put_bin[]){
	int off=0, bin[16];
	memset(bin, 0, sizeof(int)*16);

	if(type==0){
		printf("Error: (line %d) %s is an invalid offset!\n", ln, c);
		return 0;
	}
	else if(type==1){
		if((4*(c[MAX_WORD_SIZE+1]-1))>bits)
			printf("Warning: (line %d) %s will be truncated to %d bits; unexpeced errors may occur!\n", ln, c, bits);
		decToTwoComp(hexToDec(c), bin, 16);
	}
	else if(type==2){
		int i=1, j=15-c[MAX_WORD_SIZE+1]+2, k;
		if((c[MAX_WORD_SIZE+1]-2)>bits)
			printf("Warning: (line %d) %s will be truncated to %d bits; unexpeced errors may occur!\n", ln, c, bits);
		for(k=0; k<j; k++) bin[k]=c[1]-0x30;		// sext input binary
		while(c[i]!=0x00){
			bin[j]=c[i]-0x30;
			j++;
			i++;
		}
	}
	else if(type==3){
		int i=0, dec_num=0;
		bool neg=false;

		if(c[0]==0x2D){
			neg=true;
			i++;
		}
		while(c[i]!=0x00){
			dec_num=dec_num*10+(c[i]-0x30);
			i++;
		}
		if(neg){
			dec_num=-1*dec_num;
		}
		if((dec_num>(pow(2, bits-1)-1))||(dec_num<-pow(2, bits))){
			printf("Error: (line %d) %d cannot be expressed in %d bits!\n", ln, dec_num, bits);
			return 0;
		}
		decToTwoComp(dec_num, bin, 16);
	}
	
	for(int i=0; i<bits; i++){
		off+=bin[15-i]*pow(2, i);
	}
	off-=bin[15-bits]*pow(2, bits);

	if((off>(pow(2, bits-1)-1))||(off<-pow(2, bits))){
		printf("Error: (line %d) %d cannot be expressed in %d bits!\n", ln, off, bits);
		return 0;
	}

	for(int i=1; i<=bits; i++){
		put_bin[16-i]=bin[16-i];
	}
	return 1;
}

// print functions
void printIntArr(int num[], int size){
	int i=0;
	while(i<=size-1){
		printf("%d", num[i]);
		i++;
	}
	printf("\n");
}

void fprintIntArr(FILE *fp, int num[], int size){
	int i=0;
	while(i<=size-1){
		fprintf(fp, "%d", num[i]);
		i++;
	}
	fprintf(fp, "\n");
}

void printCharArr(char hex[], int size){
	int i=0;
	while(i<=size-1){
		printf("%c", hex[i]);
		i++;
	}
	printf("\n");
}

void fprintCharArr(FILE *fp, char hex[], int size){
	int i=0;
	while(i<=size-1){
		fprintf(fp, "%c", hex[i]);
		i++;
	}
	fprintf(fp, "\n");
}

void putSymbol(FILE *fp, char symbol[], char addr[]){
	int i=0;
	fprintf(fp, "%s", symbol);
	printf("%d\n", symbol[MAX_WORD_SIZE+1]);
	for(i=6-(symbol[MAX_WORD_SIZE+1]/TABSIZE); i>=0; i--) fprintf(fp, "\t");
	fprintf(fp, "%s\n", addr);
}