#include "parse.h"
#include "convert.h"
#include "calc.h"

void parseFile (FILE *fp, char *fname) {

	//==========================================================================
	//	Declarations
	//==========================================================================

	int org_addr = 0, addr = 0, ln = 0, ln_st = 0;
	bool orig = false, op = false, src = true;
	int bin[16];

	char line_buf[MAX_LEN + 1];
	char word_buf[MAX_WORD_NUM][MAX_WORD_SIZE + 2];	// room for null + size

	int s_cnt = 0;
	struct Symbol *symbols = malloc (sizeof (struct Symbol));

	int a_cnt = 0;
	struct Alias *aliases = malloc (sizeof (struct Alias));

	struct File file;
	fpos_t pos;

	struct Alert alert;
	alert.warn = 0;
	alert.err = 0;
	alert.exception = 0;

	//==========================================================================
	//	Create files and write headers (where applicable)
	//==========================================================================

	file.sym = fopen (replaceExt(fname, ".sym"), "w");
	if (file.sym == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	file.bin = fopen (replaceExt(fname, ".bin"), "w");
	if (file.bin == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	file.hex = fopen (replaceExt(fname, ".hex"), "w");
	if (file.hex == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	file.lst = fopen (replaceExt(fname, ".lst"), "w");
	if (file.lst == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	file.obj = fopen (replaceExt(fname, ".obj"), "w");
	if (file.obj == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	replaceExt(fname, ".asm");

	fprintf (file.sym, "Symbol Name\t\t\t Page Address\n---------------------------------\n");
	fprintf (file.lst, " Addr |  Hex  | Line |\tSource\n");
	
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

		if (!orig) {
			op = false;
			src = true;
			int o = isOrig (word_buf);
			int a = isAlias (word_buf);
			if (o >= 0) {
				op = true;
				org_addr = addr = addrToDec (word_buf[o + 1]);
				ln_st = ln;
				decToTwoComp (addr, bin, 16);
				fgetpos (fp, &pos);
				orig = true;
			} else if (a >= 0) {
				aliases = realloc (aliases, (a_cnt + 1) * sizeof (struct Alias));
				memcpy (aliases[a_cnt].word, word_buf[a + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
				memcpy (aliases[a_cnt].replace, word_buf[a + 2], sizeof(char) * (MAX_WORD_SIZE + 2));
				aliases[a_cnt].count = 0;
				a_cnt++;
			}
			fprintAsm (file, bin, addr, ln, line_buf, op, src);
		} else if (isEnd (word_buf) >= 0) {
			break;
		} else {
			int i;
			int keyword, pseudoop, label;
			for (i = 0; i < MAX_WORD_NUM; i++) {
				char *c = word_buf[i];
				keyword = isKeyword (word_buf[i]);
				pseudoop = isPseuodoOp (word_buf[i]);
				label = isLabel (word_buf[i]);
				if (label && i == 0) {
					symbols = realloc (symbols, (s_cnt + 1) * sizeof (struct Symbol));
					memcpy (symbols[s_cnt].label, c, sizeof(char) * (MAX_WORD_SIZE + 2));
					symbols[s_cnt].addr = addr;
					symbols[s_cnt].count = 0;
					s_cnt++;
					
					char addr_str[4];
					decToAddr (addr_str, addr);
					putSymbol (file.sym, c, addr_str);
				} else if (label && i >= 0) {
					symbols = realloc (symbols, (s_cnt + 1) * sizeof (struct Symbol));
					memcpy (symbols[s_cnt].label, c, sizeof(char) * (MAX_WORD_SIZE + 2));
					symbols[s_cnt].addr = addr;
					symbols[s_cnt].count = 0;
					s_cnt++;
					
					char addr_str[4];
					decToAddr (addr_str, addr);
					putSymbol (file.sym, c, addr_str);

					alert.warn++;
					printf ("Warning: (%s line %d) ", fname, ln);
					printf ("multiple label declaration!\n%s", line_buf);
					printf ("\tConsider consolidating labels.\n");
				} else if (word_buf[i][0] == '\0') {
					break;
				} else if (!isValidOffset (word_buf[i])) {
					addr++;
					break;
				} else {
					alert.err++;
					printf ("Error: (%s line %d) ", fname, ln);
					printf ("Unrecognized syntax!\n%s", line_buf);
					break;
				}
			}

			int op_num = 0;
			int off = 0;

			switch (pseudoop) {
			case NO_OP:
			{
				break;
			}
			case ORIG:
			{
				op_num = 1;
				alert.err++;
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("Multiple .ORIG declaration!\n%s", line_buf);
				break;
			}
			case END:
			{
				alert.exception++;
				printf ("%d: Unhandled exception!\n", pseudoop);
				break;
			}
			case STRINGZ:
			{
				op_num = 1;
				char *string = word_buf[i + 1];
				int j = 1;
				while (string[j] != '\0') {
					if (string[j] == '\"'){
						break;
					} else if (string[j] == '\\') {
						addr++;
						j += 2;
					} else {
						addr++;
						j++;
					}
				}
				break;
			}
			case BLKW:
			{
				op_num = 1;
				off = offset (isValidOffset (word_buf[i + 1]), word_buf[i + 1], 15);
				for (int j = off; j > 1; j--) {
					addr++;
				}
				break;
			}
			case FILL:
			{
				op_num = 1;
				break;
			}
			case ALIAS:
			{
				op_num = 2;
				alert.err++;
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("aliases must be declared before .ORIG!\n\t%s", line_buf);
				break;
			}
			default:
			{
				alert.exception++;
				printf ("%d: Unhandled exception!\n", pseudoop);
				break;
			}
			}

			switch (keyword) {
			case NO_OP:
			{
				break;
			}
			case BR:
			{
				op_num = 1;
				break;
			}
			case ADD:
			{
				op_num = 3;
				break;
			}
			case LD:
			{
				op_num = 2;
				break;
			}
			case ST:
			{
				op_num = 2;
				break;
			}
			case JSR:
			{
				op_num = 1;
				break;
			}
			case AND:
			{
				op_num = 3;
				break;
			}
			case LDR:
			{
				op_num = 3;
				break;
			}
			case STR:
			{
				op_num = 3;
				break;
			}
			case RTI:
			{
				op_num = 0;
				break;
			}
			case NOT:
			{
				op_num = 2;
				break;
			}
			case LDI:
			{
				op_num = 2;
				break;
			}
			case STI:
			{
				op_num = 2;
				break;
			}
			case JMP:
			{
				if(strcmp(word_buf[i], "RET")==0||strcmp(word_buf[i], "ret")==0)
					op_num = 0;
				else
					op_num = 1;
				break;
			}
			case LEA:
			{
				op_num = 2;
				break;
			}
			case TRAP:
			{
				if (isTrap (word_buf[i]) > 1)
					op_num = 0;
				else
					op_num = 1;
				break;
			}
			default:
			{
				alert.exception++;
				printf ("%d: Unhandled exception!\n", keyword);
				break;
			}
			}

			if (pseudoop >= 0 || keyword >= 0)
				i++;

			char c;
			if (op_num == 1)
				c = '\0';
			else
				c = 's';

			if ((countWords (i, word_buf) - op_num) > 0) {
				alert.warn++;
				printf ("Warning: (%s line %d) ", fname, ln);
				printf ("'%s' takes %d operand%c!\n", word_buf[i-1], op_num, c);
				printf ("%s", line_buf);
			} else if ((countWords (i, word_buf) - op_num) < 0) {
				alert.err++;
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("'%s' takes %d operand%c!\n", word_buf[i-1], op_num, c);
				printf ("%s", line_buf);
			}
		}
	}

	printAlertSummary (alert);

	if (file.sym != NULL)
		fclose(file.sym);

	//==========================================================================
	//	Pass 2 - Generate List, Binary, Hex, and Object files
	//==========================================================================

	printf ("Pass 2:\n");

	struct Alert alert_st;
	alert_st.err = alert.err;
	alert_st.exception = alert.exception;

	alert.warn = 0;
	alert.err = 0;
	alert.exception = 0;

	fsetpos (fp, &pos);		// sets starting pos to origin pos, save some reads
	ln = ln_st;
	addr = org_addr;

	while(fgets(line_buf, MAX_LEN+1, fp)!=NULL){
		ln++;
		bool comment = (line_buf[0] == ';');
		bool empty = (line_buf[0] == '\0');

		if (comment || empty){
			op = false;
			fprintAsm (file, bin, addr, ln, line_buf, op, src);
			continue;
		}


		memset (word_buf, 0, sizeof(word_buf));
		lineToWords (line_buf, word_buf);

		//======================================================================
		//	Generate Binary and Hex Files
		//======================================================================

		memset (bin, 0, sizeof(int) * 16);		// clear bin array
		
		op = false;
		src = true;
		int offset_bits = 0;
		int off, off_type;
		char *op1;
		char *op2;
		char *op3;

		for (int i = 0; i < MAX_WORD_NUM; i++) {
			aliasWord (aliases, a_cnt, word_buf[i]);
		}

		int i = 0;

		while (labelAddress (symbols, s_cnt, word_buf[i]) >= 0) {
			i++;
		}
		
		switch (isPseuodoOp (word_buf[i])) {
		case NO_OP:
		{
			break;
		}
		case ORIG:
		{
			alert.err++;
			printf ("Error: (%s line %d) ", fname, ln);
			printf ("Multiple .ORIG declaration\n%s", line_buf);
			break;			
		}
		case END:		// clean up and return
		{
			printAlertSummary (alert);
			alert.err += alert_st.err;
			alert.exception += alert_st.exception;

			free(symbols);
			if (file.bin != NULL)
				fclose(file.bin);
			if (file.hex != NULL)
				fclose(file.hex);
			if (file.lst != NULL)
				fclose(file.lst);
			if (file.obj != NULL)
				fclose(file.obj);

			if (alert.err > 0 || alert.exception > 0) {	// clear written files
				replaceExt(fname, ".sym");
				file.sym = fopen (fname, "w");
				replaceExt(fname, ".bin");
				file.bin = fopen (fname, "w");
				replaceExt(fname, ".hex");
				file.hex = fopen (fname, "w");
				replaceExt(fname, ".lst");
				file.lst = fopen (fname, "w");
				replaceExt(fname, ".obj");
				file.obj = fopen (fname, "w");

				if (file.bin != NULL)
					fclose(file.bin);
				if (file.hex != NULL)
					fclose(file.hex);
				if (file.lst != NULL)
					fclose(file.lst);
				if (file.obj != NULL)
					fclose(file.obj);
			}

			return;
		}
		case STRINGZ:
		{
			op = true;
			op1 = word_buf[i + 1];
			if (op1[0] == '\"') {
				int i = 1;
				while (op1[i] != '\0') {
					if (op1[i] == '\"'){
						memset (bin, 0, sizeof(int) * 16);
						fprintAsm (file, bin, addr, ln, line_buf, op, src);
						addr++;
						break;
					} else if (op1[i] == '\\' && escapeValue (op1[i+1])) {
						decToTwoComp (escapeValue (op1[i+1]), bin, 16);
						fprintAsm (file, bin, addr, ln, line_buf, op, src);
						if (src) src = false;
						addr++;
						i += 2;
					} else if (op1[i] == '\\' && !escapeValue (op1[i+1])) {
						alert.err++;
						printf ("Error: (%s line %d) ", fname, ln);
						printf ("invalid escape sequence in string %s\n", op1);
					} else {
						decToTwoComp (op1[i], bin, 16);
						fprintAsm (file, bin, addr, ln, line_buf, op, src);
						if (src) src = false;
						addr++;
						i++;
					}
				}
			} else {
				alert.err++;
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("invalid operand for '%s': %s\n", word_buf[i], op1);
			}
			op = false;
			break;
		}
		case BLKW:
		{
			op = true;
			src = true;
			op1 = word_buf[i + 1];
			off_type = isValidOffset (op1);
			if (off_type == 0){
				alert.err++;
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("invalid operand for '%s': %s\n", word_buf[i], op1);
			} else {
				for (off = offset (off_type, op1, 15); off > 0; off--){
					fprintAsm (file, bin, addr, ln, line_buf, op, src);
					if (src) src = false;
					addr++;
				}
			}
			op = false;
			src = false;
			break;
		}
		case FILL:
		{
			op = true;
			src = true;
			op1 = word_buf[i + 1];
			off_type = isValidOffset (op1);
			int label_addr = labelAddress (symbols, s_cnt, op1);
			if (off_type > 0) {
				off = offset (off_type, op1, 15);
				decToTwoComp (off, bin, 16);
			} else if (label_addr >= 0) {
				decToTwoComp (label_addr, bin, 16);
			} else {
				alert.err++;
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("invalid operand for '%s': %s\n", word_buf[i], op1);
			}
			break;
		}
		case ALIAS:
		{
			break;
		}
		default:
		{
			printf ("%d, Unhandled pseudoop exception!\n", isPseuodoOp (word_buf[i]));
			break;
		}
		}

		switch (isKeyword (word_buf[i])) {
		case NO_OP:
		{
			break;
		}
		case BR:
		{
			op = true;
			offset_bits = 9;
			// set condition codes
			branchCondition (word_buf[i], bin);

			op1 = word_buf[i + 1];
			op_offset (word_buf[i], op1, offset_bits, ln, bin, s_cnt, symbols, addr, &alert, fname);
			break;
		}
		case ADD:
		{
			op = true;
			bin[3] = 1;
			offset_bits = 5;

			op1 = word_buf[i + 1];
			op2 = word_buf[i + 2];
			op3 = word_buf[i + 3];
			op_register (word_buf[i], op1, 0, bin, ln, &alert, fname);
			op_register (word_buf[i], op2, 1, bin, ln, &alert, fname);
			op_reg_imm (word_buf[i], op3, bin, 2, offset_bits, ln, &alert, fname);
			break;
		}
		case LD:
		{
			op = true;
			bin[2] = 1;
			offset_bits = 9;

			op1 = word_buf[i + 1];
			op2 = word_buf[i + 2];
			op_register (word_buf[i], op1, 0, bin, ln, &alert, fname);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, &alert, fname);
			break;
		}
		case ST:
		{
			op = true;
			bin[3] = bin[2] = 1;
			offset_bits = 9;

			op1 = word_buf[i + 1];
			op2 = word_buf[i + 2];
			op_register (word_buf[i], op1, 0, bin, ln, &alert, fname);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, &alert, fname);
			break;
		}
		case JSR:
		{
			op = true;
			bin[1] = 1;
			if (strcmp (word_buf[i], "JSR") == 0 || strcmp (word_buf[i], "jsr") == 0) {
				bin[4] = 1;
				offset_bits = 11;

				op1 = word_buf[i + 1];
				op_offset( word_buf[i], op1, offset_bits, ln, bin, s_cnt, symbols, addr, &alert, fname);
			} else {
				op1 = word_buf[i + 1];
				op_register (word_buf[i], op1, 1, bin, ln, &alert, fname);
			}
			break;
		}
		case AND:
		{
			op = true;
			bin[1] = bin[3] = 1;
			offset_bits = 5;

			op1 = word_buf[i + 1];
			op2 = word_buf[i + 2];
			op3 = word_buf[i + 3];
			op_register (word_buf[i], op1, 0, bin, ln, &alert, fname);
			op_register (word_buf[i], op2, 1, bin, ln, &alert, fname);
			op_reg_imm (word_buf[i], op3, bin, 2, offset_bits, ln, &alert, fname);
			break;
		}
		case LDR:
		{
			op = true;
			bin[1] = bin[2] = 1;
			offset_bits = 6;

			op1 = word_buf[i + 1];
			op2 = word_buf[i + 2];
			op3 = word_buf[i + 3];
			op_register (word_buf[i], op1, 0, bin, ln, &alert, fname);
			op_register (word_buf[i], op2, 1, bin, ln, &alert, fname);
			op_offset (word_buf[i], op3, offset_bits, ln, bin, s_cnt, symbols, addr, &alert, fname);
			break;
		}
		case STR:
		{
			op = true;
			bin[1] = bin[2] = bin[3] = 1;
			offset_bits = 6;
			
			op1 = word_buf[i + 1];
			op2 = word_buf[i + 2];
			op3 = word_buf[i + 3];
			op_register (word_buf[i], op1, 0, bin, ln, &alert, fname);
			op_register (word_buf[i], op2, 1, bin, ln, &alert, fname);
			op_offset (word_buf[i], op3, offset_bits, ln, bin, s_cnt, symbols, addr, &alert, fname);
			break;
		}
		case RTI:
		{
			op = true;
			bin[0] = 1;
			break;
		}
		case NOT:
		{
			op = true;
			bin[0] = bin[3] = bin[10] = bin[11] = bin[12] = bin[13] = bin[14] = bin[15] = 1;

			op1 = word_buf[i + 1];
			op2 = word_buf[i + 2];
			op_register (word_buf[i], op1, 0, bin, ln, &alert, fname);
			op_register (word_buf[i], op2, 1, bin, ln, &alert, fname);
			break;
		}
		case LDI:
		{
			op = true;
			bin[0] = bin[2] = 1;
			offset_bits = 9;

			op1 = word_buf[i + 1];
			op2 = word_buf[i + 2];
			op_register (word_buf[i], op1, 0, bin, ln, &alert, fname);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, &alert, fname);
			break;
		}
		case STI:
		{
			op=true;
			bin[0]=bin[2]=bin[3]=1;
			offset_bits=9;

			op1 = word_buf[i + 1];
			op2 = word_buf[i + 2];
			op_register (word_buf[i], op1, 0, bin, ln, &alert, fname);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, &alert, fname);
			break;
		}
		case JMP:
		{
			op=true;
			bin[0]=bin[1]=1;

			if(strcmp(word_buf[i], "RET")==0||strcmp(word_buf[i], "ret")==0){
				bin[7]=bin[8]=bin[9]=1;
			}
			else{
				op1 = word_buf[i + 1];
				op_register (word_buf[i], op1, 1, bin, ln, &alert, fname);
			}
			break;
		}
		case LEA:
		{
			op = true;
			bin[0] = bin[1] = bin[2] = 1;
			offset_bits = 9;
			

			op1 = word_buf[i + 1];
			op2 = word_buf[i + 2];

			op_register (word_buf[i], op1, 0, bin, ln, &alert, fname);
			op_offset (word_buf[i], op2, offset_bits, ln, bin, s_cnt, symbols, addr, &alert, fname);
			break;
		}
		case TRAP:
		{
			op = true;
			bin[0] = bin[1] = bin[2] = bin[3] = 1;
			offset_bits = 8;

			op1 = word_buf[i + 1];
			trapShortcut (word_buf[i], op1);
			op_offset (word_buf[i], op1, offset_bits, ln, bin, s_cnt, symbols, addr, &alert, fname);
			break;
		}
		default:
			printf ("%d Unhandled keyword exception!\n", isKeyword (word_buf[i]));
			break;
		}

		fprintAsm (file, bin, addr, ln, line_buf, op, src);
		if(op)
			addr++;
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

void aliasWord (struct Alias *aliases, int a_cnt, char c[MAX_WORD_SIZE + 2])
{
	for (int i = 0; i < a_cnt; i++) {
		if (strcmp (aliases[i].word, c) == 0) {
			memcpy (c, aliases[i].replace, sizeof (char) * (MAX_WORD_SIZE + 2));
			aliases[i].count++;
			return;
		}
	}
	return;
}

void op_reg_imm (char *keyword, char *op, int *bin, int loc,
				int offset_bits, int ln, struct Alert *alert, char *fname)
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
			alert->err++;
			printf ("Error: (%s line %d) ", fname, ln);
			printf ("%d cannot be expressed in %d bits!\n", off, offset_bits);
		}
	} else {
		alert->err++;
		printf ("Error: (%s line %d) ", fname, ln);
		printf ("invalid operand for '%s': %s\n", keyword, op);
	}
}

void op_register (char *keyword, char *op, int loc, int *bin,
					int ln, struct Alert *alert, char *fname)
{
	int reg = isRegister(op);
	if (reg >= 0) {
		fillRegister (reg, bin, loc);
	} else {
		alert->err++;
		printf ("Error: (%s line %d) ", fname, ln);
		printf ("invalid operand for '%s': %s\n", keyword, op);
	}
}

void op_offset (char *keyword, char *op, int offset_bits, int ln, int *bin,
				int s_cnt, struct Symbol *symbols, int addr, struct Alert *alert, char *fname)
{
	int off_type = isValidOffset (op);
	if (off_type > 0) {
		int off = offset (off_type, op, offset_bits);
		fillDecOffset (off, offset_bits, ln, bin);
	} else {
		int label_addr = labelAddress (symbols, s_cnt, op);
		if (label_addr >= 0) {
			int off = label_addr-(addr+1);
			if(!fillDecOffset(off, offset_bits, ln, bin)){
				alert->err++;
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("%d cannot be expressed in %d bits!\n", off, offset_bits);
			}
		} else {
			alert->err++;
			printf ("Error: (%s line %d) ", fname, ln);
			printf ("Undeclared label '%s'!\n", op);
		}
	}
}

void lineToWords (char *line_buf, char word_buf[][MAX_WORD_SIZE + 2])
{
	int i = 0, j = 0, k = 0;		// counter inits
	bool prev = false;
	bool quote = false;

	while (i <= MAX_LEN && !(line_buf[i] == '\0') && !(line_buf[i] == ';')) {
		if (line_buf[i] == '\"')
			quote = !quote;
		bool space = isspace (line_buf[i]);
		bool comma = line_buf[i] == ',';
		if (quote || (!space && !comma)) {
			word_buf[j][k] = line_buf[i];
			k++;
			prev = true;
		} else if (!quote && (space||comma) && prev) {
			word_buf[j][k] = 0x00;
			word_buf[j][MAX_WORD_SIZE + 1] = k;
			j++;
			k = 0;
			prev = false;
		}
		i++;
	}
}

int countWords (int offset, char word_buf[][MAX_WORD_SIZE + 2])
{
	int i;
	for (i = 0; i < MAX_WORD_NUM; i++){
		if (word_buf[i][0] == '\0')
			break;
	}
	return i - offset;
}

void fprintAsm (struct File file, int *bin, int addr, int ln,
				char *line_buf, bool op, bool src)
{
	char hex[4];
	if (op) {
		char byte[2];
		unsigned char value;
		fprintIntArr (file.bin, bin, 16);
		fprintf (file.bin, "\n");
		binToHex (bin, 16, hex, 4);
		fprintCharArr (file.hex, hex, 4);
		fprintf (file.hex, "\n");

		byte[0] = hex[0];
		byte[1] = hex[1];
		value = byteValue (byte);
		fwrite (&value, sizeof(value), 1, file.obj);

		byte[0] = hex[2];
		byte[1] = hex[3];
		value = byteValue (byte);
		fwrite (&value, sizeof(value), 1, file.obj);
	}

	if (op) {
		fprintf(file.lst, " ");
		decToAddr (hex, addr);
		fprintCharArr (file.lst, hex, 4);
		fprintf (file.lst, " | x");
		binToHex (bin, 16, hex, 4);
		fprintCharArr (file.lst, hex, 4);
		fprintf (file.lst, " | ");
	} else if (src) {
		fprintf (file.lst, "      |       | ");
	}

	if (src) {
		fprintf (file.lst, "%d", ln);
		int n = ln;
		while (n < 10000){
			fprintf (file.lst, " ");
			n *= 10;
		}
		fprintf (file.lst, "|\t%s", line_buf);
	} else if (op) {
		fprintf (file.lst, "     |\n");
	}
}

void printAlertSummary (struct Alert alert)
{
	char warn, err, exception;
	if (alert.err == 1)
		err = '\0';
	else
		err = 's';
	if (alert.exception == 1)
		exception = '\0';
	else
		exception = 's';
	if (alert.warn == 1)
		warn = '\0';
	else
		warn = 's';
	printf ("%d error%c,", alert.err, err);
	printf (" %d warning%c,", alert.warn, warn);
	printf (" and %d exception%c!\n", alert.exception, exception);
}

void branchCondition (char *c, int *bin)
{
	switch (isBranch (c)) {
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
	return;
}

void trapShortcut (char *op, char *trapvect)
{
	switch (isTrap (op)){
	case -1:
		break;
	case 0 ... 1:
		break;
	case 2 ... 3:
		trapvect[MAX_WORD_SIZE+1] = 3;
		trapvect[0] = 'x'; trapvect[1] = '2';
		trapvect[2] = '0'; trapvect[3] = '\0';
		break;
	case 4 ... 5:
		trapvect[MAX_WORD_SIZE+1] = 3;
		trapvect[0] = 'x'; trapvect[1] = '2';
		trapvect[2] = '1'; trapvect[3] = '\0';
		break;
	case 6 ... 7:
		trapvect[MAX_WORD_SIZE+1] = 3;
		trapvect[0] = 'x'; trapvect[1] = '2';
		trapvect[2] = '2'; trapvect[3] = '\0';
		break;
	case 8 ... 9:
		trapvect[MAX_WORD_SIZE+1] = 3;
		trapvect[0] = 'x'; trapvect[1] = '2';
		trapvect[2] = '3'; trapvect[3] = '\0';
		break;
	case 10 ... 11:
		trapvect[MAX_WORD_SIZE+1] = 3;
		trapvect[0] = 'x'; trapvect[1] = '2';
		trapvect[2] = '4'; trapvect[3] = '\0';
		break;
	case 12 ... 13:
		trapvect[MAX_WORD_SIZE+1] = 3;
		trapvect[0] = 'x'; trapvect[1] = '2';
		trapvect[2] = '5'; trapvect[3] = '\0';
		break;
	default:
		break;
	}
}