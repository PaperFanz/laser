#include "laser.h"

void parse_file(FILE *fp, char *fname){

	//==========================================================================
	//	Declarations
	//==========================================================================

	int MAX_LEN=MAX_WORD_NUM*MAX_WORD_SIZE;
	int addr=0, ln=0, ln_st=0;
	int bin[16];
	char addr_str[6];					// "x", 4 hex chars and null
	char line_buf[MAX_LEN+1];			// see definitions in laser.h
	char word_buf[MAX_WORD_NUM][MAX_WORD_SIZE+2];	// room for null + size
	char fname_buf[MAX_WORD_SIZE+5];
	bool b_org=false, op=false;
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
			if(fp_sym==NULL) printf("Unable to open %s!\n", fname_buf);
			fname_buf[i+1]=0x6C;		// l
			fname_buf[i+2]=0x73;		// s
			fname_buf[i+3]=0x74;		// t
			fp_lst=fopen(fname_buf, "w");		// create or clear filename.lst
			if(fp_lst==NULL) printf("Unable to open %s!\n", fname_buf);
			fname_buf[i+1]=0x62;		// b
			fname_buf[i+2]=0x69;		// i
			fname_buf[i+3]=0x6E;		// n
			fp_bin=fopen(fname_buf, "w");		// create or clear filename.bin
			if(fp_bin==NULL) printf("Unable to open %s!\n", fname_buf);
			fname_buf[i+1]=0x68;		// h
			fname_buf[i+2]=0x65;		// e
			fname_buf[i+3]=0x78;		// x
			fp_hex=fopen(fname_buf, "w");		// create or clear filename.hex
			if(fp_hex==NULL) printf("Unable to open %s!\n", fname_buf);
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
		ln++;
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
				addr=addrToDec(word_buf[1])-1;
				ln_st=ln;
				fgetpos(fp, &pos);
				b_org=true;
			}
			
			if(isPseuodoOp(word_buf[0])==1){
				break;
			}

			// check for label declarations and print to filename.sym
			if(b_org){
				if(isLabel(word_buf[0])){
					decToAddr(addr_str, addr);
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
	ln=ln_st;

	while(fgets(line_buf, MAX_LEN+1, fp)!=NULL){
		ln++;
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

			//==================================================================
			//	Generate Binary File
			//==================================================================

			memset(bin, 0, sizeof(int)*16);		// clear bin array
			i=0;								// reset count
			op=false;							// reset opcode indicator
			char op1[MAX_WORD_SIZE+2];
			char op2[MAX_WORD_SIZE+2];
			char op3[MAX_WORD_SIZE+2];

			while(word_buf[i][0]!=0x00){
				int opcode=isKeyword(word_buf[i]);
				switch(opcode){
					case -1:	// not keyword
						break;
					case 0:		// BR, check condition codes
						op=true;
						break;
					case 1:		// ADD
						op=true;
						bin[3]=1;
						memcpy(op1, word_buf[i+1], sizeof(char)*(MAX_WORD_SIZE+2));
						memcpy(op2, word_buf[i+2], sizeof(char)*(MAX_WORD_SIZE+2));
						memcpy(op3, word_buf[i+3], sizeof(char)*(MAX_WORD_SIZE+2));
						if(!fillRegister(isRegister(op1), bin, 0))
							printf("Error: (line %d) invalid operand for %s: %s\n", ln, word_buf[i], op1);
						if(!fillRegister(isRegister(op2), bin, 1))
							printf("Error: (line %d) invalid operand for %s: %s\n", ln, word_buf[i], op2);
						if(!fillRegister(isRegister(op3), bin, 2)){
							if(fillOffset(isValidOffset(op3), op3, 5, ln, bin)==1)
								bin[10]=1;
							else
								printf("Error: (line %d) invalid operand for %s: %s\n", ln, word_buf[i], op3);
						}
						i+=3;
						break;
					case 2:		// LD
						op=true;
						bin[2]=1;
						break;
					case 3:		// ST
						op=true;
						bin[3]=bin[2]=1;
						break;
					case 4:		// JSR and JSRR, check addr mode
						op=true;
						bin[1]=1;
						break;
					case 5:		// AND
						op=true;
						bin[1]=bin[3]=1;
						memcpy(op1, word_buf[i+1], sizeof(char)*(MAX_WORD_SIZE+2));
						memcpy(op2, word_buf[i+2], sizeof(char)*(MAX_WORD_SIZE+2));
						memcpy(op3, word_buf[i+3], sizeof(char)*(MAX_WORD_SIZE+2));
						if(!fillRegister(isRegister(op1), bin, 0))
							printf("Error: (line %d) invalid operand for %s: %s\n", ln, word_buf[i], op1);
						if(!fillRegister(isRegister(op2), bin, 1))
							printf("Error: (line %d) invalid operand for %s: %s\n", ln, word_buf[i], op2);
						if(!fillRegister(isRegister(op3), bin, 2)){
							if(fillOffset(isValidOffset(op3), op3, 5, ln, bin)==1)
								bin[10]=1;
							else
								printf("Error: (line %d) invalid operand for %s: %s\n", ln, word_buf[i], op3);
						}
						i+=3;
						break;
					case 6:		// LDR
						op=true;
						bin[1]=bin[2]=1;
						break;
					case 7:		// STR
						op=true;
						bin[1]=bin[2]=bin[3]=1;
						break;
					case 8:		// RTI
						op=true;
						bin[0]=1;
						break;
					case 9:		// NOT
						op=true;
						bin[0]=bin[3]=bin[10]=bin[11]=bin[12]=bin[13]=bin[14]=bin[15]=1;
						memcpy(op1, word_buf[i+1], sizeof(char)*(MAX_WORD_SIZE+2));
						memcpy(op2, word_buf[i+2], sizeof(char)*(MAX_WORD_SIZE+2));
						if(!fillRegister(isRegister(op1), bin, 0))
							printf("Error: (line %d) invalid operand for %s: %s\n", ln, word_buf[i], op1);
						if(!fillRegister(isRegister(op2), bin, 1))
							printf("Error: (line %d) invalid operand for %s: %s\n", ln, word_buf[i], op2);
						i+=3;
						break;
					case 10:	// LDI
						op=true;
						bin[0]=bin[2]=1;
						break;
					case 11:	// STI
						op=true;
						bin[0]=bin[2]=bin[3]=1;
						break;
					case 12:	// JMP and RET, check shortcut
						op=true;
						bin[0]=bin[1]=1;
						break;
					case 14:		// LEA
						op=true;
						bin[0]=bin[1]=bin[2]=1;
						break;
					case 15:	// TRAP, check shortcuts
						op=true;
						bin[0]=bin[1]=bin[2]=bin[3]=1;
						break;
					default:
						break;
				}
				i++;
			}
			if(op){
				fprintf(fp_bin, "%d\t", ln);
				fprintIntArr(fp_bin, bin, 16);	// print only if opcode detected
			}
		}
	}
}