#include "laser.h"

void parse_file(FILE *fp, char *fname){

//==============================================================================
//	Declarations
//==============================================================================

	int MAX_LEN=MAX_WORD_NUM*MAX_WORD_SIZE;
	char line_buf[MAX_LEN+1];
	char word_buf[MAX_WORD_NUM][MAX_WORD_SIZE+1];
	char fname_buf[MAX_WORD_SIZE+5];
	FILE *fp_st=fp;
	FILE *fp_sym;
	
//==============================================================================
//	Pass 1 - Generate Symbol file
//==============================================================================

	// create a file filename.sym from filename.asm
	for(int i=0; i<=MAX_WORD_SIZE; i++){
		if(fname[i]==0x2E){
			fname_buf[i]=0x2E;			// .
			fname_buf[i+1]=0x73;		// s
			fname_buf[i+2]=0x79;		// y
			fname_buf[i+3]=0x6D;		// m
			fname_buf[i+4]=0x00;		// completes symbol table filename
			break;
		}
		else{
			fname_buf[i]=fname[i];
		}
	}

	fp_sym=fopen(fname_buf, "w");

	while(fgets(line_buf, MAX_LEN+1, fp)!=NULL){
		if(line_buf[0]!=0x3B){
			int i=0, j=0, k=0;		// counter inits
			bool prev=false, space=false, comma=false;

			memset(word_buf, 0, sizeof(word_buf));

			// separate line into words->word_buf
			while(i<=MAX_LEN&&line_buf[i]!=0x00&&line_buf[i]!=0x3B){
				space=isspace(line_buf[i]);
				comma=line_buf[i]==0x2C;
				if(!space&&!comma){
					word_buf[j][k]=line_buf[i];
					k++;
					prev=true;
				}
				else if((space||comma)&&prev){		// commas also denote EOW
					word_buf[j][k]=0x00;
					j++;
					k=0;
					prev=false;
				}
				i++;
			}

			// check for label declarations and print to filename.sym
			if((isKeyword(word_buf[0])==-1)&&(isPseuodoOp(word_buf[0])==-1)){
				fprintf(fp_sym, "%s\n", word_buf[0]);
			}

		}
	}

//==============================================================================
//	Pass 2 - Generate List, Binary, Hex, and Object files
//==============================================================================
}

void printIntArr(int num[], int size){
	int i=0;
	while(i<=size-1){
		printf("%i", num[i]);
		i++;
	}
	printf("\n");
}

void printCharArr(char hex[], int size){
	int i=0;
	while(i<=size-1){
		printf("%c", hex[i]);
		i++;
	}
	printf("\n");
}