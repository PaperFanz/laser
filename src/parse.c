#include "parse.h"

void parse_file (FILE *fp, char *fname) {

	//==========================================================================
	//	Declarations
	//==========================================================================

	int org_addr = 0, addr = 0, ln = 0, ln_st = 0;
	bool orig = false, op = false;
	int bin[16];
	char hex[4];
	char addr_str[5];					// 4 hex chars and null

	int MAX_LEN = MAX_WORD_NUM * MAX_WORD_SIZE;
	char line_buf[MAX_LEN + 1];			// see definitions in laser.h
	char word_buf[MAX_WORD_NUM][MAX_WORD_SIZE + 2];	// room for null + size

	int s_cnt = 0;
	struct Symbol *symbols = malloc (sizeof (struct Symbol));
	// char **symbol = malloc(sizeof(char*));
	// int *dec_addr = malloc(sizeof(int));

	char fname_buf[MAX_WORD_SIZE + 5];
	FILE *fp_sym, *fp_lst, *fp_bin, *fp_hex, *fp_obj;
	fpos_t pos;

	struct Alert alert;
	alert.warn = 0;
	alert.err = 0;
	alert.exception = 0;

	//==========================================================================
	//	Create files and write headers (where applicable)
	//==========================================================================
	for (int i = 0; i <= MAX_WORD_SIZE; i++) {
		if (fname[i] == '.') {
			fname_buf[i] = '.';			// .
			fname_buf[i + 1] = 's';		// s
			fname_buf[i + 2] = 'y';		// y
			fname_buf[i + 3] = 'm';		// m
			fname_buf[i + 4] = '\0';
			fp_sym = fopen (fname_buf, "w");	// create or clear filename.sym
			if (fp_sym == NULL)
				printf ("Error: Unable to open %s!\n", fname_buf);
			fname_buf[i + 1] = 'l';		// l
			fname_buf[i + 2] = 's';		// s
			fname_buf[i + 3] = 't';		// t
			fp_lst = fopen (fname_buf, "w");	// create or clear filename.lst
			if (fp_lst == NULL)
				printf ("Error: Unable to open %s!\n", fname_buf);
			fname_buf[i + 1] = 'b';		// b
			fname_buf[i + 2] = 'i';		// i
			fname_buf[i + 3] = 'n';		// n
			fp_bin = fopen (fname_buf, "w");	// create or clear filename.bin
			if (fp_bin == NULL)
				printf ("Error: Unable to open %s!\n", fname_buf);
			fname_buf[i + 1] = 'h';		// h
			fname_buf[i + 2] = 'e';		// e
			fname_buf[i + 3] = 'x';		// x
			fp_hex = fopen (fname_buf, "w");	// create or clear filename.hex
			if (fp_hex == NULL)
				printf ("Error: Unable to open %s!\n", fname_buf);
			fname_buf[i + 1] = 'o';		// o
			fname_buf[i + 2] = 'b';		// b
			fname_buf[i + 3] = 'j';		// j
			fp_obj = fopen (fname_buf, "w");	// create or clear filename.obj
			if (fp_obj == NULL)
				printf ("Error: Unable to open %s!\n", fname_buf);
			break;
		} else {
			fname_buf[i] = fname[i];
		}
	}

	// symbol file header
	fprintf (fp_sym, "Symbol Name\t\t\t Page Address\n---------------------------------\n");
	
	//==========================================================================
	//	Pass 1 - Generate Symbol file
	//==========================================================================

	printf ("Pass 1: \n");

skip_symbol_parse:
	while(fgets (line_buf, MAX_LEN + 1, fp) != NULL) {
		ln++;
		bool comment = (line_buf[0] == ';');
		bool empty = (line_buf[0] == '\0');

		if (comment || empty)
			goto skip_symbol_parse;

		memset (word_buf, 0, sizeof(word_buf));

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

		//look for .ORIG and subsequent starting address
		if (!orig && (isPseuodoOp (word_buf[0]) == 0)) {
			org_addr = addr = addrToDec (word_buf[1]) - 1;
			ln_st = ln;
			decToTwoComp (addr + 1, bin, 16);
			binToHex (bin, 16, hex, 4);
			fprintIntArr (fp_bin, bin, 16);
			fprintCharArr (fp_hex, hex, 4);
			fgetpos (fp, &pos);
			orig = true;
		}
		if (isPseuodoOp (word_buf[0]) == 1) {
			break;
		}
		
		if (orig && isLabel(word_buf[0])) {
			symbols = realloc (symbols, (s_cnt + 1) * sizeof (struct Symbol));
			// symbol = realloc (symbol, (s_cnt + 1) * sizeof(char*));
			// dec_addr = realloc (dec_addr, (s_cnt + 1) * sizeof(int));
			// symbol[s_cnt] = malloc (sizeof(char) * (MAX_WORD_SIZE + 2));
			// memcpy (symbol[s_cnt], word_buf[0], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (symbols[s_cnt].label, word_buf[0], sizeof(char) * (MAX_WORD_SIZE + 2));
			symbols[s_cnt].addr = addr;
			s_cnt++;

			decToAddr (addr_str, addr);
			putSymbol (fp_sym, word_buf[0], addr_str);
			if (word_buf[1][0] != 0x00)
				addr++;
		} else if (orig){
			addr++;
		}
	}

	if (fp_sym != NULL)
		fclose(fp_sym);

	//==========================================================================
	//	Pass 2 - Generate List, Binary, Hex, and Object files
	//==========================================================================

	printf ("Pass 2:\n");

	fsetpos (fp, &pos);		// sets starting pos to origin pos, save some reads
	ln = ln_st;
	addr = org_addr;

	while(fgets(line_buf, MAX_LEN+1, fp)!=NULL){
		ln++;
		bool comment = (line_buf[0] == ';');
		bool empty = (line_buf[0] == '\0');

		if (comment || empty)
			continue;

		memset (word_buf, 0, sizeof(word_buf));

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

		if(!(isLabel(word_buf[0]) && (word_buf[1][0] == 0x00)))
			addr++;

		//======================================================================
		//	Generate Binary and Hex Files
		//======================================================================

		memset (bin, 0, sizeof(int) * 16);		// clear bin array
		i = j = 0;								// reset counters
		op = false;							// reset opcode indicator
		int offset_bits = 0;
		int max_operands = 0;
		int reg;
		int off, off_type;
		bool match = false;
		char op1[MAX_WORD_SIZE + 2];
		char op2[MAX_WORD_SIZE + 2];
		char op3[MAX_WORD_SIZE + 2];

		while (isLabel (word_buf[i]))
			i++;
		
		switch (isPseuodoOp (word_buf[i])){
		case -1:
			break;
		case 0:			// ORIG
			printf("Error: (line %d) Multiple .ORIG declaration\n\t%s", ln, line_buf);
			break;
		case 1:			// END, clean up and return

			free(symbols);

			if (fp_bin != NULL)
				fclose(fp_bin);
			if (fp_hex != NULL)
				fclose(fp_hex);
			if (fp_lst != NULL)
				fclose(fp_lst);
			if (fp_obj != NULL)
				fclose(fp_obj);

			return;
		case 2:			// STRINGZ
			break;
		case 3:			// BLKW
			break;
		case 4:			// FILL
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
			max_operands = 1;
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
			max_operands = 3;

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
			max_operands = 2;

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 3:			// ST
			op = true;
			bin[3] = bin[2] = 1;
			offset_bits = 9;
			max_operands = 2;

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
				max_operands = 1;

				memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));

				//OPERAND_OFFSET (op1);
			} else {
				offset_bits = 6;
				max_operands = 2;

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
			max_operands = 3;

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
			max_operands = 3;
			
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
			max_operands = 3;
			
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
			max_operands = 2;

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_register (word_buf[i], op2, 1, bin, ln, alert);
			break;
		case 10:		// LDI
			op = true;
			bin[0] = bin[2] = 1;
			offset_bits = 9;
			max_operands = 2;

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 11:		// STI
			op=true;
			bin[0]=bin[2]=bin[3]=1;
			offset_bits=9;
			max_operands = 2;

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 12:		// JMP and RET, check shortcut
			op=true;
			bin[0]=bin[1]=1;

			if(strcmp(word_buf[i], "RET")==0||strcmp(word_buf[i], "ret")==0){
				max_operands = 0;
				bin[7]=bin[8]=bin[9]=1;
			}
			else{
				max_operands = 1;
				memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
				op_register (word_buf[i], op1, 1, bin, ln, alert);
			}
			break;
		case 14:		// LEA
			op = true;
			bin[0] = bin[1] = bin[2] = 1;
			offset_bits = 9;
			max_operands = 2;

			memcpy (op1, word_buf[i + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
			memcpy (op2, word_buf[i + 2], sizeof(char) * (MAX_WORD_SIZE + 2));

			op_register (word_buf[i], op1, 0, bin, ln, alert);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, alert);
			break;
		case 15:		// TRAP, check shortcuts
			op = true;
			bin[0] = bin[1] = bin[2] = bin[3] = 1;
			offset_bits = 8;
			max_operands = 0;
			op1[MAX_WORD_SIZE+1] = 3;

			switch (isTrap (word_buf[i])){
			case -1:
				break;
			case 0 ... 1:
				max_operands = 1;
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
			fprintf(fp_bin, "%d\t%s\t", ln, decToAddr(hex, addr));
			fprintIntArr(fp_bin, bin, 16);	// print only if opcode detected
			fprintf(fp_hex, "%d\t", ln);
			binToHex(bin, 16, hex, 4);
			fprintCharArr(fp_hex, hex, 4);
		}
	}
}

void op_reg_imm(char *keyword, char *op, int *bin, int loc,
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

void op_register(char *keyword, char *op, int loc, int *bin, int ln, struct Alert alert)
{
	int reg = isRegister(op);
	if (reg >= 0) {
		fillRegister (reg, bin, loc);
	} else {
		printf ("Error: (line %d) invalid operand for '%s': %s\n", ln, keyword, op);
	}
}

void op_offset(char *keyword, char *op, int offset_bits, int ln,
				int *bin, int s_cnt, struct Symbol *symbols, int addr, struct Alert alert)
{
	int off_type = isValidOffset (op);
	if (off_type > 0) {
		int off = offset (off_type, op, offset_bits);
		fillDecOffset (off, offset_bits, ln, bin);
	} else {
		int j;
		bool match = false;
		for (j=0; j<s_cnt; j++) {
			if (strcmp (op, symbols[j].label) == 0) {
				match = true;
				break;
			}
		}

		if (match) {
			int off = (symbols[j].addr-(addr+1));
			if(!fillDecOffset(off, offset_bits, ln, bin))
				printf("Error: (line %d) %d cannot be expressed in %d bits!\n", ln, off, offset_bits);
		} else {
			printf("Error: (line %d) Undeclared label '%s'!\n", ln, op);
		}
	}
}