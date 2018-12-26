#include "laser.h"

void parse_file(FILE *fp, char *fname){

	//==========================================================================
	//	Declarations
	//==========================================================================

	int MAX_LEN=MAX_WORD_NUM*MAX_WORD_SIZE;
	int addr=0;
	int bin_buf[16], opcode_buf[4];
	char addr_str[6];					// "x", 4 hex chars and null
	char line_buf[MAX_LEN+1];			// see definitions in laser.h
	char word_buf[MAX_WORD_NUM][MAX_WORD_SIZE+2];	// room for null + size
	char fname_buf[MAX_WORD_SIZE+5];
	bool b_org=false;
	FILE *fp_sym, *fp_lst, *fp_bin, *fp_hex, *fp_obj;
	fpos_t pos;

	//==========================================================================
	//	Create files and write headers (where applicable)
	//==========================================================================
	for(int i=0; i<=MAX_WORD_SIZE; i++){
		if(fname[i]==0x2E){
			fname_buf[i]=0x2E;			// .
			fname_buf[i+1]=0x73;		// s
			fname_buf[i+2]=0x79;		// y
			fname_buf[i+3]=0x6D;		// m
			fname_buf[i+4]=0x00;
			fp_sym=fopen(fname_buf, "w");		// create or clear filename.sym
			fname_buf[i+1]=0x6C;		// l
			fname_buf[i+2]=0x73;		// s
			fname_buf[i+3]=0x74;		// t
			fp_lst=fopen(fname_buf, "w");		// create or clear filename.lst
			fname_buf[i+1]=0x62;		// b
			fname_buf[i+2]=0x69;		// i
			fname_buf[i+3]=0x6E;		// n
			fp_bin=fopen(fname_buf, "w");		// create or clear filename.bin
			fname_buf[i+1]=0x68;		// h
			fname_buf[i+2]=0x65;		// e
			fname_buf[i+3]=0x78;		// x
			fp_hex=fopen(fname_buf, "w");		// create or clear filename.hex
			fname_buf[i+1]=0x6F;		// o
			fname_buf[i+2]=0x62;		// b
			fname_buf[i+3]=0x6A;		// j
			fp_obj=fopen(fname_buf, "w");		// create or clear filename.obj
			break;
		}
		else{
			fname_buf[i]=fname[i];
		}
	}

	if(fp_sym!=NULL) fprintf(fp_sym, "Symbol Name\t\t\t Page Address\n---------------------------------\n");
	
	//==========================================================================
	//	Pass 1 - Generate Symbol file
	//==========================================================================

	while(fgets(line_buf, MAX_LEN+1, fp)!=NULL){
		if(line_buf[0]!=0x3B&&line_buf[0]!=0x00){
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
					word_buf[j][MAX_WORD_SIZE+1]=k;
					j++;
					k=0;
					prev=false;
				}
				i++;
			}

			//look for .ORIG and subsequent starting address
			if(!b_org&&(isPseuodoOp(word_buf[0])==0)){
				addr=hexToDec(word_buf[1])-1;
				fgetpos(fp, &pos);
				b_org=true;
			}
			
			if(isPseuodoOp(word_buf[0])==1){
				break;
			}

			// check for label declarations and print to filename.sym
			if(b_org){
				if((isKeyword(word_buf[0])<0)&&(isPseuodoOp(word_buf[0])<0)){
					decToHex(addr_str, addr);
					putSymbol(fp_sym, word_buf[0], addr_str);
					if((word_buf[1][0]!=0x00)) addr++;
				}
				else{
					addr++;
				}
			}
		}
	}

	printf("Pass 1: Clear\n");

	//==========================================================================
	//	Pass 2 - Generate List, Binary, Hex, and Object files
	//==========================================================================

	fsetpos(fp, &pos);		// sets starting pos to origin pos, save some reads

	while(fgets(line_buf, MAX_LEN+1, fp)!=NULL){
		if(line_buf[0]!=0x3B&&line_buf[0]!=0x00){
			int i=0, j=0, k=0;		// counter inits
			bool prev=false, space=false, comma=false;

			printf("%s", line_buf);

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
					word_buf[j][MAX_WORD_SIZE+1]=k;
					j++;
					k=0;
					prev=false;
				}
				i++;
			}

			//==================================================================
			//	Generate Binary File
			//==================================================================

			while(word_buf[i][0]!=0x00){
				int opcode=isPseuodoOp(word_buf[i]);
				switch(opcode){
					case 0:
						bin_buf[15]=bin_buf[14]=bin_buf[13]=bin_buf[12]=0;
					default:
						break;
				}
				i++;
			}

		}
	}
}

// print functions
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

void putSymbol(FILE *fp, char symbol[], char addr[]){
	int i=0;
	fprintf(fp, "%s", symbol);
	for(i=6-(symbol[MAX_WORD_SIZE+1]/TABSIZE); i>=0; i--) fprintf(fp, "\t");
	fprintf(fp, "%s\n", addr);
}