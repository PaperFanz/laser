#include "parse.h"
#include "convert.h"
#include "calc.h"

void parseFile (FILE *fp, char *fname) {

	//==========================================================================
	//	Declarations
	//==========================================================================

	int addr_st = 0, ln = 0, ln_st = 0;
	bool orig = false;
	int bin[16];

	char line_buf[MAX_LEN + 1];
	char word_buf[MAX_WORD_NUM][MAX_WORD_SIZE + 2];	// room for null + size

	int s_cnt = 0;
	struct Symbol *symbols = malloc (sizeof (struct Symbol));
	memcpy (symbols[0].label, fname, sizeof (char) * (MAX_WORD_SIZE + 1));
	symbols[0].addr = 0;
	symbols[0].count = 1;

	int a_cnt = 0;
	struct Alias *aliases = malloc (sizeof (struct Alias));
	memcpy (aliases[0].word, fname, sizeof (char) * (MAX_WORD_SIZE + 1));
	aliases[0].count = 1;

	struct File file;
	fpos_t pos;

	struct Alert alert;
	alert.warn = 0;
	alert.err = 0;
	alert.exception = 0;

	struct Instruction instruction;
	instruction.fname = fname;

	const char *symHeader = "Symbol Name                       Address\n"
							"=========================================\n";
	const char *lstHeader = " Addr |  Hex  | Line |     Source\n";
	const char *binlstHeader = " Addr |  Hex  |      Binary      | Line |     Source\n";
	const char *underline = "=================================================="
	"==============================\n";

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

	fprintf (file.sym, "%s", symHeader);

	if (PRINT_BIN_IN_LST)
		fprintf(file.lst, "%s", binlstHeader);
	else
		fprintf (file.lst, "%s", lstHeader);
	fprintf(file.lst, "%s", underline);
	
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

		instruction.line_buf = line_buf;
		instruction.ln = ln;
		instruction.instr = 0;

		if (!orig) {
			instruction.type = N_OP;
			instruction.src = true;
			int o = isOrig (word_buf);
			int a = isAlias (word_buf);
			if (o >= 0) {
				instruction.type = P_OP;
				addr_st = instruction.addr = addrToDec (word_buf[o + 1]);
				instruction.instr = instruction.addr;
				ln_st = ln;
				fgetpos (fp, &pos);
				orig = true;
			} else if (a >= 0) {
				instruction.type = P_OP;
				if (word_buf[a + 1][0] == '\0' || word_buf[a + 2][0] == '\0'){
					alert.warn++;
					printf ("Warning: (%s line %d) ", fname, ln);
					printf ("'%s' requires two operands!\n%s", word_buf[a], line_buf);
				} else {
					a_cnt = aliases[0].count;
					aliases = realloc (aliases, (a_cnt + 1) * sizeof (struct Alias));
					memcpy (aliases[a_cnt].word, word_buf[a + 1], sizeof(char) * (MAX_WORD_SIZE + 2));
					memcpy (aliases[a_cnt].replace, word_buf[a + 2], sizeof(char) * (MAX_WORD_SIZE + 2));
					aliases[a_cnt].ln = ln;
					aliases[a_cnt].count = 0;
					aliases[0].count++;
				}
			}
			fprintAsm (file, instruction);
		} else if (isEnd (word_buf) >= 0) {
			break;
		} else {

			for (int i = 0; i < MAX_WORD_NUM; i++) {
				aliasWord (aliases, word_buf[i]);
			}

			int i;
			int keyword, pseudoop, label;
			for (i = 0; i < MAX_WORD_NUM; i++) {
				char *c = word_buf[i];
				keyword = isKeyword (word_buf[i]);
				pseudoop = isPseuodoOp (word_buf[i]);
				label = isLabel (word_buf[i]);
				s_cnt = symbols[0].count;
				if (label && i == 0) {
					symbols = realloc (symbols, (s_cnt + 1) * sizeof (struct Symbol));
					memcpy (symbols[s_cnt].label, c, sizeof(char) * (MAX_WORD_SIZE + 2));
					symbols[s_cnt].addr = instruction.addr;
					symbols[s_cnt].count = 0;
					symbols[s_cnt].ln = ln;
					symbols[0].count++;

					char addr_str[4];
					decToAddr (addr_str, instruction.addr);
					putSymbol (file.sym, c, addr_str);
				} else if (label && i >= 0) {
					symbols = realloc (symbols, (s_cnt + 1) * sizeof (struct Symbol));
					memcpy (symbols[s_cnt].label, c, sizeof(char) * (MAX_WORD_SIZE + 2));
					symbols[s_cnt].addr = instruction.addr;
					symbols[s_cnt].count = 0;
					symbols[s_cnt].ln = ln;
					symbols[0].count++;

					char addr_str[4];
					decToAddr (addr_str, instruction.addr);
					putSymbol (file.sym, c, addr_str);

					alert.warn++;
					printf ("Warning: (%s line %d) ", fname, ln);
					printf ("multiple label declaration!\n%s", line_buf);
					printf ("\tConsider consolidating labels.\n");
				} else if (word_buf[i][0] == '\0') {
					break;
				} else if (!isValidOffset (word_buf[i])) {
					instruction.addr++;
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
						instruction.addr++;
						j += 2;
					} else {
						instruction.addr++;
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
					instruction.addr++;
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
				printf ("\t%s", line_buf);
			} else if ((countWords (i, word_buf) - op_num) < 0) {
				alert.err++;
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("'%s' takes %d operand%c!\n", word_buf[i-1], op_num, c);
				printf ("\t%s", line_buf);
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
	instruction.addr = addr_st;

	while(fgets(line_buf, MAX_LEN+1, fp)!=NULL){
		ln++;
		bool comment = (line_buf[0] == ';');
		bool empty = (line_buf[0] == '\0');

		if (comment || empty){
			instruction.type = N_OP;
			fprintAsm (file, instruction);
			continue;
		}

		memset (word_buf, 0, sizeof(word_buf));
		lineToWords (line_buf, word_buf);

		//======================================================================
		//	Generate Binary and Hex Files
		//======================================================================

		memset (bin, 0, sizeof(int) * 16);		// clear bin array

		int offset_bits = 0;
		int off, off_type, label_addr;

		instruction.line_buf = line_buf;
		instruction.ln = ln;
		instruction.instr = 0;
		instruction.type = N_OP;
		instruction.src = true;

		int instr_bin[16];

		for (int i = 0; i < MAX_WORD_NUM; i++) {
			aliasWord (aliases, word_buf[i]);
		}

		int i = 0;
		while (labelAddress (symbols, word_buf[i]) >= 0) {
			i++;
		}

		char *op1 = word_buf[i + 1];
		char *op2 = word_buf[i + 2];
		char *op3 = word_buf[i + 3];
		
		switch (isPseuodoOp (word_buf[i])) {
		case NO_OP:
		{
			break;
		}
		case ORIG:
		{
			alert.err++;
			printf ("Error: (%s line %d) ", fname, ln);
			printf ("Multiple .ORIG declaration\n\t%s", line_buf);
			break;			
		}
		case END:		// clean up and return
		{
			unusedSymbol (symbols, &alert);
			unusedAlias (aliases, &alert);
			
			printAlertSummary (alert);
			alert.err += alert_st.err;
			alert.exception += alert_st.exception;

			free (symbols);
			free (aliases);
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
			instruction.type = P_OP;
			operandString (op1, &instruction, file, &alert);
			instruction.type = N_OP;
			instruction.src = false;
			break;
		}
		case BLKW:
		{
			instruction.type = P_OP;
			instruction.src = true;
			off_type = isValidOffset (op1);
			if (off_type == 0){
				alert.err++;
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("invalid operand for '%s': %s\n", word_buf[i], op1);
			} else {
				for (off = offset (off_type, op1, 15); off > 0; off--){
					fprintAsm (file, instruction);
					if (instruction.src) instruction.src = false;
					instruction.addr++;
				}
			}
			instruction.type = N_OP;
			instruction.src = false;
			break;
		}
		case FILL:
		{
			instruction.type = P_OP;
			instruction.src = true;
			off_type = isValidOffset (op1);
			int label_addr = labelAddress (symbols, op1);
			if (off_type > 0) {
				off = offset (off_type, op1, 15);
				instruction.instr += off;
			} else if (label_addr >= 0) {
				instruction.instr += label_addr;
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
	
		int keyword = isKeyword (word_buf[i]);
		if (keyword >= 0 && keyword <= 15 && keyword != 13){
			instruction.instr = keyword << 12;
			instruction.opcode = word_buf[i];
			instruction.type = OP;
			instruction.src = true;
		}

		switch (keyword) {
		case NO_OP:
		{
			break;
		}
		case BR:
		{
			offset_bits = 9;
			int branch = isBranch (word_buf[i]);
			if (branch >= 0) {
				instruction.instr += branch << 9;
			} else {
				alert.err++;
				printf ("Error: (%s line %d) ", instruction.fname, instruction.ln);
				printf ("invalid branch conditions: %s\n", instruction.opcode);
			}
			operandOffset (op1, &instruction, symbols, offset_bits, &alert);
			break;
		}
		case ADD:
		{
			offset_bits = 5;
			operandRegister (op1, 0, &instruction, &alert);
			operandRegister (op2, 1, &instruction, &alert);
			operandImmediate (op3, &instruction, 2, offset_bits, &alert);
			break;
		}
		case LD:
		{
			offset_bits = 9;
			operandRegister (op1, 0, &instruction, &alert);
			operandOffset (op2, &instruction, symbols, offset_bits, &alert);
			break;
		}
		case ST:
		{
			offset_bits = 9;
			operandRegister (op1, 0, &instruction, &alert);
			operandOffset (op2, &instruction, symbols, offset_bits, &alert);
			break;
		}
		case JSR:
		{
			if (strcmp (word_buf[i], "JSR") == 0 || strcmp (word_buf[i], "jsr") == 0) {
				instruction.instr += 1 << 11;
				offset_bits = 11;
				operandOffset (op1, &instruction, symbols, offset_bits, &alert);
			} else {
				operandRegister (op1, 1, &instruction, &alert);
			}
			break;
		}
		case AND:
		{
			offset_bits = 5;
			operandRegister (op1, 0, &instruction, &alert);
			operandRegister (op2, 1, &instruction, &alert);
			operandImmediate (op3, &instruction, 2, offset_bits, &alert);
			break;
		}
		case LDR:
		{
			offset_bits = 6;
			operandRegister (op1, 0, &instruction, &alert);
			operandRegister (op2, 1, &instruction, &alert);
			operandOffset (op3, &instruction, symbols, offset_bits, &alert);
			break;
		}
		case STR:
		{
			offset_bits = 6;
			operandRegister (op1, 0, &instruction, &alert);
			operandRegister (op2, 1, &instruction, &alert);
			operandOffset (op3, &instruction, symbols, offset_bits, &alert);
			break;
		}
		case RTI:
		{
			break;
		}
		case NOT:
		{
			instruction.instr += offsetMask (6);
			operandRegister (op1, 0, &instruction, &alert);
			operandRegister (op2, 1, &instruction, &alert);
			break;
		}
		case LDI:
		{
			offset_bits = 9;
			operandRegister (op1, 0, &instruction, &alert);
			operandOffset (op2, &instruction, symbols, offset_bits, &alert);
			break;
		}
		case STI:
		{
			offset_bits = 9;
			operandRegister (op1, 0, &instruction, &alert);
			operandOffset (op2, &instruction, symbols, offset_bits, &alert);
			break;
		}
		case JMP:
		{
			if(strcmp(word_buf[i], "RET")==0||strcmp(word_buf[i], "ret")==0)
				instruction.instr += offsetMask (3) << 6;
			else
				operandRegister (op1, 1, &instruction, &alert);
			break;
		}
		case LEA:
		{
			offset_bits = 9;
			operandRegister (op1, 0, &instruction, &alert);
			operandOffset (op2, &instruction, symbols, offset_bits, &alert);
			break;
		}
		case TRAP:
		{
			offset_bits = 8;
			trapShortcut (word_buf[i], op1);
			operandOffset (op1, &instruction, symbols, offset_bits, &alert);
			break;
		}
		default:
			printf ("%d Unhandled keyword exception!\n", isKeyword (word_buf[i]));
			break;
		}

		fprintAsm (file, instruction);
		if(instruction.type)
			instruction.addr++;
	}
}

int labelAddress (struct Symbol *sym, char *label)
{
	for (int i = 1; i < sym[0].count; i++) {
		if (strcmp (sym[i].label, label) == 0) {
			sym[i].count++;
			return sym[i].addr;
		}
	}
	return -1;
}

void aliasWord (struct Alias *al, char c[MAX_WORD_SIZE + 2])
{
	for (int i = 1; i < al[0].count; i++) {
		if (strcmp (al[i].word, c) == 0) {
			memcpy (c, al[i].replace, sizeof (char) * (MAX_WORD_SIZE + 2));
			al[i].count++;
			return;
		}
	}
	return;
}

int unusedSymbol (struct Symbol *sym, struct Alert *a)
{
	for (int i = 1; i < sym[0].count; i++) {
		if (sym[i].count == 0) {
			a->warn++;
			printf ("Warning: (%s line %d) ", sym[0].label, sym[i].ln);
			printf ("unused label: %s\n", sym[i].label);
		}
	}
	return 1;
}

int unusedAlias (struct Alias *al, struct Alert *a)
{
	for (int i = 1; i < al[0].count; i++) {
		if (al[i].count == 0) {
			a->warn++;
			printf ("Warning: (%s line %d) ", al[0].word, al[i].ln);
			printf ("unused alias: %s\n", al[i].word);
		}
	}
	return 1;
}

int operandImmediate (char *op, struct Instruction *ins, int loc, int off_b, struct Alert *a)
{
	int off;
	int off_type = isValidOffset (op);
	int r = isRegister (op);
	if (loc == 2) loc = 3;
	int shift = 9 - (3 * loc);
	if (r >= 0) {
		ins->instr += r << shift;
		return 1;
	} else if (off_type > 0){
		off = offset (off_type, op, off_b);
	} else {
		a->err++;
		printf ("Error: (%s line %d) ", ins->fname, ins->ln);
		printf ("invalid operand for '%s': %s!\n", ins->opcode, op);
		return 0;
	}

	if (off > (1 << off_b - 1) - 1 || off < -(1 << off_b - 1)) {
		a->err++;
		printf ("Error: (%s line %d) ", ins->fname, ins->ln);
		printf ("%d will not fit in %d offset bits!\n", off, off_b);
		return 0;
	} else {
		ins->instr += off & offsetMask (off_b);
		ins->instr += 1 << off_b;
		return 1;
	}
}

int operandRegister (char *op, int loc, struct Instruction *ins, struct Alert *a)
{
	int r = isRegister (op);
	if (loc == 2) loc = 3;
	int shift = 9 - (3 * loc);
	if (r >= 0) {
		ins->instr += r << shift;
		return 1;
	} else {
		a->err++;
		printf ("Error: (%s line %d) ", ins->fname, ins->ln);
		printf ("'%s' expects operand type register; ", ins->opcode);
		printf ("'%s' is not a register!\n", op);
		return 0;
	}
}

int operandOffset (char *op, struct Instruction *ins, struct Symbol *sym, int off_b, struct Alert *a)
{
	int s_cnt = sym[0].count;
	int off;
	int off_type = isValidOffset (op);
	int label_addr = labelAddress (sym, op);
	if (off_type > 0){
		off = offset (off_type, op, off_b);
	} else if (label_addr >= 0) {
		off = label_addr - ins->addr -1;
	} else {
		a->err++;
		printf ("Error: (%s line %d) ", ins->fname, ins->ln);
		printf ("invalid operand for '%s': %s!\n", ins->opcode, op);
		return 0;
	}

	if (off > (1 << off_b - 1) - 1 || off < -(1 << off_b - 1)) {
		a->err++;
		printf ("Error: (%s line %d) ", ins->fname, ins->ln);
		printf ("%d will not fit in %d offset bits!\n", off, off_b);
		return 0;
	} else {
		ins->instr += off & offsetMask (off_b);
		return 1;
	}
}

int operandString (char *str, struct Instruction *ins, struct File file, struct Alert *a)
{
	if (str[0] == '\"') {
		int i = 1;
		while (str[i] != '\0') {
			int escChar = escapeValue (str[i+1]);
			if (str[i] == '\"'){
				ins->instr = 0;
				fprintAsm (file, *ins);
				ins->addr++;
				break;
			} else if (str[i] == '\\' && escChar) {
				ins->instr = escChar;
				fprintAsm (file, *ins);
				if (ins->src) ins->src = false;
				ins->addr++;
				i += 2;
			} else if (str[i] == '\\' && !escChar) {
				a->err++;
				printf ("Error: (%s line %d) ", ins->fname, ins->ln);
				printf ("invalid escape sequence in string %s\n", str);
			} else {
				ins->instr = str[i];
				fprintAsm (file, *ins);
				if (ins->src) ins->src = false;
				ins->addr++;
				i++;
			}
		}
	} else {
		a->err++;
		printf ("Error: (%s line %d) ", ins->fname, ins->ln);
		printf ("invalid operand for '%s': %s\n", ins->opcode, str);
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

void fprintAsm (struct File file, struct Instruction ins)
{
	char hex[4];
	int bin[16];
	decToTwoComp (ins.instr, bin, 16);
	if (ins.type) {
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

	if (ins.type) {
		fprintf(file.lst, " ");
		decToAddr (hex, ins.addr);
		fprintCharArr (file.lst, hex, 4);
		fprintf (file.lst, " | x");
		binToHex (bin, 16, hex, 4);
		fprintCharArr (file.lst, hex, 4);
		fprintf (file.lst, " | ");
		if (PRINT_BIN_IN_LST) {
			fprintIntArr (file.lst, bin, 16);
			fprintf (file.lst, " | ");
		}
	} else if (ins.src && !PRINT_BIN_IN_LST) {
		fprintf (file.lst, "      |       | ");
	} else if (ins.src && PRINT_BIN_IN_LST) {
		fprintf (file.lst, "      |       |                  | ");
	}

	if (ins.src) {
		fprintf (file.lst, "%d", ins.ln);
		int n = ins.ln;
		while (n < 10000){
			fprintf (file.lst, " ");
			n *= 10;
		}
		fprintf (file.lst, "| %s", ins.line_buf);
	} else if (ins.type) {
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