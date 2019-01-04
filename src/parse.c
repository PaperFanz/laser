#include "parse.h"

void parseFile (FILE *fp, char *fname) {

	//==========================================================================
	//	Declarations
	//==========================================================================

	int org_addr = 0, addr = 0, ln = 0, ln_st = 0;
	bool orig = false, op = false;
	int bin[16];
	char addr_str[5];

	char line_buf[MAX_LEN + 1];			// see definitions in laser.h
	char word_buf[MAX_WORD_NUM][MAX_WORD_SIZE + 2];	// room for null + size

	int s_cnt = 0;
	struct Symbol *symbols = malloc (sizeof (struct Symbol));

	struct File file;
	fpos_t pos;

	struct Alert alert;
	alert.warn = 0;
	alert.err = 0;
	alert.exception = 0;

	//==========================================================================
	//	Create files and write headers (where applicable)
	//==========================================================================

	replaceExt(fname, ".sym");
	file.sym = fopen (fname, "w");
	if (file.sym == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	replaceExt(fname, ".bin");
	file.bin = fopen (fname, "w");
	if (file.bin == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	replaceExt(fname, ".hex");
	file.hex = fopen (fname, "w");
	if (file.hex == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	replaceExt(fname, ".lst");
	file.lst = fopen (fname, "w");
	if (file.lst == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	replaceExt(fname, ".obj");
	file.obj = fopen (fname, "w");
	if (file.obj == NULL)
		printf ("Error: Unable to open %s!\n", fname);

	// symbol file header
	fprintf (file.sym, "Symbol Name\t\t\t Page Address\n---------------------------------\n");
	
	//==========================================================================
	//	Pass 1 - Generate Symbol file
	//==========================================================================

	printf ("Pass 1: \n");

	while(fgets (line_buf, MAX_LEN + 1, fp) != NULL) {
		ln++;
		bool comment = (line_buf[0] == ';');
		bool empty = (line_buf[0] == '\0');

		if (comment || empty)
			continue;

		memset (word_buf, 0, sizeof(word_buf));
		lineToWords (line_buf, word_buf);

		//look for .ORIG and subsequent starting address
		if (!orig && (isPseuodoOp (word_buf[0]) == 0)) {
			org_addr = addr = addrToDec (word_buf[1]) - 1;
			ln_st = ln;
			decToTwoComp (addr + 1, bin, 16);
			fprintAsm (file, bin);
			fgetpos (fp, &pos);
			orig = true;
		}
		if (isPseuodoOp (word_buf[0]) == 1) {
			break;
		}
		
		if (orig && isLabel(word_buf[0])) {
			symbols = realloc (symbols, (s_cnt + 1) * sizeof (struct Symbol));
			memcpy (symbols[s_cnt].label, word_buf[0], sizeof(char) * (MAX_WORD_SIZE + 2));
			symbols[s_cnt].addr = addr;
			s_cnt++;

			decToAddr (addr_str, addr);
			putSymbol (file.sym, word_buf[0], addr_str);
			if (word_buf[1][0] != 0x00)
				addr++;
		} else if (orig){
			addr++;
		}
	}
	if (file.sym != NULL)
		fclose(file.sym);

	//==========================================================================
	//	Pass 2 - Generate List, Binary, Hex, and Object files
	//==========================================================================

	printf ("Pass 2:\n");

	fsetpos (fp, &pos);		// sets starting pos to origin pos, save some reads
	ln = ln_st;
	addr = org_addr + 1;

	while(fgets(line_buf, MAX_LEN+1, fp)!=NULL){
		ln++;
		bool comment = (line_buf[0] == ';');
		bool empty = (line_buf[0] == '\0');

		if (comment || empty)
			continue;

		memset (word_buf, 0, sizeof(word_buf));
		lineToWords (line_buf, word_buf);

		//======================================================================
		//	Generate Binary and Hex Files
		//======================================================================

		memset (bin, 0, sizeof(int) * 16);		// clear bin array
		int i = 0;
		op = false;							// reset opcode indicator
		int offset_bits = 0;
		int off, off_type;
		char op1[MAX_WORD_SIZE + 2];
		char op2[MAX_WORD_SIZE + 2];
		char op3[MAX_WORD_SIZE + 2];

		while (labelAddress (symbols, s_cnt, word_buf[i]) >= 0)
			i++;
		
		switch (isPseuodoOp (word_buf[i])){
		case -1:
			break;
		case 0:			// ORIG
			printf("Error: (line %d) Multiple .ORIG declaration\n\t%s", ln, line_buf);
			break;
		case 1:			// END, clean up and return
			free(symbols);
			if (file.bin != NULL)
				fclose(file.bin);
			if (file.hex != NULL)
				fclose(file.hex);
			if (file.lst != NULL)
				fclose(file.lst);
			if (file.obj != NULL)
				fclose(file.obj);
			return;
		case 2:			// STRINGZ
			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			if (isQuote (op1[0])) {
				int i = 1;
				while (op1[i] != '\0') {
					if (isQuote (op1[i])){
						memset (bin, 0, sizeof(int) * 16);
						fprintAsm (file, bin);
						break;
					}	
					decToTwoComp (op1[i], bin, 16);
					fprintAsm (file, bin);
					addr++;
					i++;
				}
			} else {
				printf ("Error: (line %d) invalid operand for '%s': %s\n", ln, word_buf[i], op1);
			}
			break;
		case 3:			// BLKW
			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			off_type = isValidOffset (op1);
			if (off_type == 0){
				printf ("Error: (line %d) invalid operand for '%s': %s\n", ln, word_buf[i], op1);
			} else {
				for (off = offset (off_type, op1, 15); off > 0; off--){
					fprintAsm (file, bin);
					addr++;
				}
			}
			break;
		case 4:			// FILL
			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			off_type = isValidOffset (op1);
			int label_addr = labelAddress (symbols, s_cnt, op1);
			if (off_type > 0) {
				off = offset (off_type, op1, 15);
				decToTwoComp (off, bin, 16);
			} else if (label_addr >= 0) {
				decToTwoComp (label_addr, bin, 16);
			} else {
				printf ("Error: (line %d) invalid operand for '%s': %s\n", ln, word_buf[i], op1);
			}
			fprintAsm (file, bin);
			break;
		default:
			printf ("%d, Unhandled pseudoop exception!\n", isPseuodoOp (word_buf[i]));
			break;
		}

		switch (isKeyword (word_buf[i])) {
		case -1:
			break;
		case 0:			// BR
			op = true;
			offset_bits = 9;
			
			// set condition codes
			switch (isBranch (word_buf[i])) {
			case -1:
				break;
			case 0 ... 3:
				bin[4] = bin[5] = bin[6] = 1;
				break;
			case 4 ... 5:
				bin[4] = bin[5] = 1;
				break;
			case 6 ... 7:
				bin[4] = 1;
				break;
			case 8 ... 9:
				bin[4] = bin[6] = 1;
				break;
			case 10 ... 11:
				bin[5] = bin[6] = 1;
			case 12 ... 13:
				bin[5] = 1;
				break;
			case 14 ... 15:
				bin[6] = 1;
				break;
			default:
				break;
			}

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_offset (word_buf[i], op1, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 1:			// ADD
			op = true;
			bin[3] = 1;
			offset_bits = 5;
			

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op3, word_buf[i + 3], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_register (word_buf[i], op2, 1, bin, ln, alert);
			op_reg_imm (word_buf[i], op3, bin, 2, offset_bits, ln, alert);
			break;
		case 2:			// LD
			op = true;
			bin[2] = 1;
			offset_bits = 9;
			

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 3:			// ST
			op = true;
			bin[3] = bin[2] = 1;
			offset_bits = 9;
			

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 4:			// JSR and JSRR, check addr mode
			op = true;
			bin[1] = 1;
			if (strcmp (word_buf[i], "JSR") == 0 || strcmp (word_buf[i], "jsr") == 0) {
				bin[4] = 1;
				offset_bits = 11;
				

				memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));

				//OPERAND_OFFSET (op1);
			} else {
				offset_bits = 6;
				

				memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
				memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

				op_register (word_buf[i], op1, 1, bin, ln, alert);
				op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			}
			break;
		case 5:			// AND
			op = true;
			bin[1] = bin[3] = 1;
			offset_bits = 5;
			

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op3, word_buf[i + 3], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_register (word_buf[i], op2, 1, bin, ln, alert);
			op_reg_imm (word_buf[i], op3, bin, 2, offset_bits, ln, alert);
			break;
		case 6:			// LDR
			op = true;
			bin[1] = bin[2] = 1;
			offset_bits = 6;
			
			
			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op3, word_buf[i + 3], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_register (word_buf[i], op2, 1, bin, ln, alert);
			op_offset (word_buf[i], op3, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 7:			// STR
			op = true;
			bin[1] = bin[2] = bin[3] = 1;
			offset_bits = 6;
			
			
			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op3, word_buf[i + 3], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_register (word_buf[i], op2, 1, bin, ln, alert);
			op_offset (word_buf[i], op3, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 8:			// RTI
			op = true;
			bin[0] = 1;
			break;
		case 9:			// NOT
			op = true;
			bin[0] = bin[3] = bin[10] = bin[11] = bin[12] = bin[13] = bin[14] = bin[15] = 1;
			

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_register (word_buf[i], op2, 1, bin, ln, alert);
			break;
		case 10:		// LDI
			op = true;
			bin[0] = bin[2] = 1;
			offset_bits = 9;
			

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 11:		// STI
			op=true;
			bin[0]=bin[2]=bin[3]=1;
			offset_bits=9;
			

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 12:		// JMP and RET, check shortcut
			op=true;
			bin[0]=bin[1]=1;

			if(strcmp(word_buf[i], "RET")==0||strcmp(word_buf[i], "ret")==0){
				
				bin[7]=bin[8]=bin[9]=1;
			}
			else{
				
				memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
				op_register (word_buf[i], op1, 1, bin, ln, alert);
			}
			break;
		case 14:		// LEA
			op = true;
			bin[0] = bin[1] = bin[2] = 1;
			offset_bits = 9;
			

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 15:		// TRAP, check shortcuts
			op = true;
			bin[0] = bin[1] = bin[2] = bin[3] = 1;
			offset_bits = 8;
			
			op1[MAX_WORD_SIZE+1] = 3;

			switch (isTrap (word_buf[i])){
			case -1:
				break;
			case 0 ... 1:
				
				memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
				break;
			case 2 ... 3:
				op1[0] = 'x'; op1[1] = '2'; op1[2] = '0';
				break;
			case 4 ... 5:
				op1[0] = 'x'; op1[1] = '2'; op1[2] = '1';
				break;
			case 6 ... 7:
				op1[0] = 'x'; op1[1] = '2'; op1[2] = '2';
				break;
			case 8 ... 9:
				op1[0] = 'x'; op1[1] = '2'; op1[2] = '3';
				break;
			case 10 ... 11:
				op1[0] = 'x'; op1[1] = '2'; op1[2] = '4';
				break;
			case 12 ... 13:
				op1[0] = 'x'; op1[1] = '2'; op1[2] = '5';
				break;
			default:
				break;
			}

			op_offset (word_buf[i], op1, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		default:
			printf ("%d Unhandled keyword exception!\n", isKeyword (word_buf[i]));
			break;
		}

		if(op){
			fprintAsm (file, bin);
			addr++;
		}
	}
}

int labelAddress (struct Symbol *symbols, int s_cnt, char *label)
{
	for (int i = 0; i < s_cnt; i++) {
		if (strcmp (symbols[i].label, label) == 0)
			return symbols[i].addr;
	}
	return -1;
}

void op_reg_imm (char *keyword, char *op, int *bin, int loc,
				int offset_bits, int ln, struct Alert alert)
{
	int off;
	int reg = isRegister (op);
	int off_type = isValidOffset(op);
	if (reg >= 0) {
		fillRegister (reg, bin, loc);
	} else if ( off_type > 0 ) {
		off = offset (off_type, op, offset_bits);
		if(fillDecOffset (off, offset_bits, ln, bin)) {
			bin[10]=1;
		} else {
			printf("Error: (line %d) %d cannot be expressed in %d bits!\n", ln, off, offset_bits);
		}
	} else {
		printf ("Error: (line %d) invalid operand for '%s': %s\n", ln, keyword, op);
	}
}

void op_register (char *keyword, char *op, int loc, int *bin, int ln, struct Alert alert)
{
	int reg = isRegister(op);
	if (reg >= 0) {
		fillRegister (reg, bin, loc);
	} else {
		printf ("Error: (line %d) invalid operand for '%s': %s\n", ln, keyword, op);
	}
}

void op_offset (char *keyword, char *op, int offset_bits, int ln,
				int *bin, int s_cnt, struct Symbol *symbols, int addr, struct Alert alert)
{
	int off_type = isValidOffset (op);
	if (off_type > 0) {
		int off = offset (off_type, op, offset_bits);
		fillDecOffset (off, offset_bits, ln, bin);
	} else {
		int label_addr = labelAddress (symbols, s_cnt, op);
		if (label_addr >= 0) {
			int off = label_addr-(addr+1);
			if(!fillDecOffset(off, offset_bits, ln, bin))
				printf("Error: (line %d) %d cannot be expressed in %d bits!\n", ln, off, offset_bits);
		} else {
			printf("Error: (line %d) Undeclared label '%s'!\n", ln, op);
		}
	}
}

void lineToWords (char *line_buf, char word_buf[][MAX_WORD_SIZE + 2])
{
	int i = 0, j = 0, k = 0;		// counter inits
	bool prev = false;

	while (i <= MAX_LEN && !(line_buf[i] == '\0') && !(line_buf[i] == ';')) {
		bool space = isspace(line_buf[i]);
		bool comma = line_buf[i] == 0x2C;
		if (!space && !comma) {
			word_buf[j][k] = line_buf[i];
			k++;
			prev = true;
		} else if ((space||comma)&&prev) {		// commas also denote EOW
			word_buf[j][k] = 0x00;
			word_buf[j][MAX_WORD_SIZE + 1] = k;
			j++;
			k = 0;
			prev = false;
		}
		i++;
	}
}

void fprintAsm (struct File file, int *bin)
{
	char hex[4];
	fprintIntArr(file.bin, bin, 16);
	binToHex(bin, 16, hex, 4);
	fprintCharArr(file.hex, hex, 4);
}