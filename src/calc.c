#include "calc.h"
#include "convert.h"

int intPow (int a, int b)
{
	int ret = 1;
	for (int i = 0; i < b; i++)
		ret *= a;
	return ret;
}

int offsetMask (int offset_bits)
{
	int mask = 0;
	for (int i = 0; i < offset_bits; i++)
		mask += intPow (2, i);
	return mask;
}

int checkExt (const char *filename, const char *ext)
{
	char *dot=strrchr(filename, '.');
	if(strcmp(dot, ext)==0) return 1;
	else return 0;
}

char *replaceExt (char *filename, const char *ext)
{
	char *dot=strrchr(filename, '.');
	int i=0;
	while (dot[i] != '\0') {
		dot[i] = ext [i];
		i++;
	}
	return filename;
}

// .bin generator functions
int fillRegister (int r, int bin[], int n)
{
	if(n==2) n++;
	int m=4+(3*n);
	switch(r){
	case 0: return 1;
	case 1: bin[m+2]=1; return 1;
	case 2: bin[m+1]=1; return 1;
	case 3: bin[m+2]=bin[m+1]=1; return 1;
	case 4: bin[m]=1; return 1;
	case 5: bin[m+2]=bin[m]=1; return 1;
	case 6: bin[m+1]=bin[m]=1; return 1;
	case 7: bin[m+2]=bin[m+1]=bin[m]=1; return 1;
	default: return 0;
	}
}

int fillDecOffset (int off, int bits, int ln, int put_bin[])
{
	if((off>(pow(2, bits-1)-1))||(off<-pow(2, bits))){
		return 0;
	}
	else{
		int bin[16];
		memset(bin, 0, sizeof(int)*16);
		decToTwoComp(off, bin, 16);

		for(int i=1; i<=bits; i++){
			put_bin[16-i]=bin[16-i];
		}
		return 1;
	}
}

// read a fixed offset or immediate value and express it in n bits
int offset (int type, char c[], int bits)
{
	int off=0, bin[16];
	memset(bin, 0, sizeof(int)*16);

	if(type==0){
		return 0;
	}
	else if(type==1){
		if (c[0] == '-' || c[1] == '-'){
			decToTwoComp(-1 * hexToDec(&c[1]), bin, 16);
		} else {
			decToTwoComp(hexToDec(c), bin, 16);
		}

	}
	else if(type==2){
		int i, j=15-c[MAX_WORD_SIZE+1]+2;
		if (c[0] == '-' || c[1] == '-') {
			i = 2;
		} else {
			i = 1;
		}
		for (int k = 0; k < j; k++) 
			bin[k] = c[1] - 0x30;		// sext input binary
		while(c[i] != 0x00){
			bin[j] = c[i] - 0x30;
			j++;
			i++;
		}
		if (c[0] == '-' || c[1] == '-') {
			int addone[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};
			notArr (bin, 16);
			addArr (bin, 16, addone, 16, bin, 16);
		}
	} else if(type == 3) {
		int i, dec_num=0;
		if(c[0] == '-' || c[1] == '-')
			i = 2;
		else
			i = 1;
		while(c[i] != '\0'){
			dec_num = dec_num * 10 + c[i]-0x30;
			i++;
		}
		if(c[0] == '-' || c[1] == '-')
			dec_num = -1 * dec_num;
		decToTwoComp(dec_num, bin, 16);
	} else if (type == 4) {
		int i, dec_num=0;
		if(c[0] == '-' || c[1] == '-')
			i = 1;
		else
			i = 0;
		while(c[i] != '\0'){
			dec_num = dec_num * 10 + c[i]-0x30;
			i++;
		}
		if(c[0] == '-' || c[1] == '-')
			dec_num = -1 * dec_num;
		decToTwoComp(dec_num, bin, 16);
	}
	
	for(int i=0; i<bits; i++){
		off+=bin[15-i]*pow(2, i);
	}
	off-=bin[15-bits]*pow(2, bits);

	return off;
}

// print functions
void printIntArr (int num[], int size)
{
	for (int i = 0; i <= size - 1; i++)
		printf("%d", num[i]);
	printf("\n");
}

void fprintIntArr (FILE *fp, int num[], int size)
{
	for (int i = 0; i <= size - 1; i++)
		fprintf(fp, "%d", num[i]);
}

void printCharArr (char hex[], int size)
{
	for (int i = 0; i <= size-1; i++)
		printf("%c", hex[i]);
	printf("\n");
}

void fprintCharArr (FILE *fp, char hex[], int size)
{
	for (int i = 0; i <= size - 1; i++)
		fprintf (fp, "%c", hex[i]);
}

void putSymbol (FILE *fp, char symbol[], char addr[])
{
	int i = 0;
	fprintf (fp, "%s", symbol);
	if (USE_SPACES_IN_SYM) {
		for (i = 35 - symbol[MAX_WORD_SIZE + 1]; i >= 0; i--)
			fprintf (fp, " ");
	} else {
		for (i = 8 - (symbol[MAX_WORD_SIZE + 1] / TABSIZE); i >= 0; i--)
			fprintf (fp, "\t");
	}
	fprintf (fp, "x");
	fprintCharArr(fp, addr, 4);
	fprintf (fp, "\n");
}