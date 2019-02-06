#include "parse.h"
#include "convert.h"
#include "calc.h"

int quiet;

void parseFile (FILE *fp, char *fname, int q) {

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

	quiet = q;

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

	file.sym = fopen (replaceExt (fname, ".sym"), "w");
	if (file.sym == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	file.bin = fopen (replaceExt (fname, ".bin"), "w");
	if (file.bin == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	file.hex = fopen (replaceExt (fname, ".hex"), "w");
	if (file.hex == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	file.lst = fopen (replaceExt (fname, ".lst"), "w");
	if (file.lst == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	file.obj = fopen (replaceExt (fname, ".obj"), "w");
	if (file.obj == NULL)
		printf ("Error: Unable to open %s!\n", fname);
	if (ENABLE_LOGGING) {
		file.log =  fopen (replaceExt (fname, ".log"), "w");
		if (file.log == NULL)
			printf ("Error: Unable to open %s!\n", fname);
	} else {
		file.log = NULL;
	}
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

	if (quiet < 1)
		printf ("Pass 1: \n");
	if (ENABLE_LOGGING)
		fprintf (file.log, "Pass 1: \n");

	// look for origin and set aliases
	while (fgets (line_buf, MAX_LEN + 1, fp) != NULL) {
		ln++;

		if (!PRESERVE_HEADER && (line_buf[0] == ';' || line_buf[0] == '\0'))
			continue;

		memset (word_buf, 0, sizeof(word_buf));
		lineToWords (line_buf, word_buf);

		instruction.type = N_OP;
		instruction.src = true;
		instruction.line_buf = line_buf;
		instruction.ln = ln;
		instruction.instr = 0;

		int o = isOrig (word_buf);
		int a = isAlias (word_buf);
		if (o >= 0) {
			instruction.type = P_OP;
			addr_st = instruction.addr = addrToDec (word_buf[o + 1]);
			instruction.instr = instruction.addr;
			ln_st = ln;
			fgetpos (fp, &pos);
			goto pass_1;
		} else if (a >= 0) {
			if (isRegister (word_buf[a + 2]) >= 0) {
				if (existAlias (word_buf[a + 1], aliases)) {
					alert.err++;
					if (quiet < 2) {
						printf ("Error: (%s line %d) ", fname, ln);
						printf ("'%s' has already been declared as an alias!\n", word_buf[a + 1]);
					}
					if (ENABLE_LOGGING) {
						fprintf (file.log, "Error: (%s line %d) ", fname, ln);
						fprintf (file.log, "'%s' has already been declared as an alias!\n", word_buf[a + 1]);
					}
				} else {
					aliases = addAlias (aliases, word_buf[a + 1], word_buf[a + 2], ln);
				}
			} else {
				alert.err++;
				if (quiet < 2) {
					printf ("Error: (%s line %d) ", fname, ln);
					printf ("'%s' is for renaming registers only!\n", word_buf[a]);
					printf ("\t%s", line_buf);
				}
				if (ENABLE_LOGGING) {
					fprintf (file.log, "Error: (%s line %d) ", fname, ln);
					fprintf (file.log, "'%s' is for renaming registers only!\n", word_buf[a]);
					fprintf (file.log, "\t%s", line_buf);
				}
			}
		}
		fprintAsm (file, instruction);
	}

pass_1:
	fprintAsm (file, instruction);	// fill in origin address

	while(fgets (line_buf, MAX_LEN + 1, fp) != NULL) {
		ln++;

		if (line_buf[0] == ';' || line_buf[0] == '\0')
			continue;

		memset (word_buf, 0, sizeof(word_buf));
		lineToWords (line_buf, word_buf);

		instruction.line_buf = line_buf;
		instruction.ln = ln;
		instruction.instr = 0;

		int i;
		int keyword, pseudoop, label;
		for (i = 0; i < MAX_WORD_NUM; i++) {
			char *c = word_buf[i];
			keyword = isKeyword (c);
			pseudoop = isPseuodoOp (c);
			label = isLabel (c);
			s_cnt = symbols[0].count;
			if (label && i == 0) {
				if (existLabel (symbols, c)) {
					alert.err++;
					if (quiet < 2) {
						printf ("Error: (%s line %d) ", fname, ln);
						printf ("'%s' has already been declared as an label!\n", c);
					}
					if (ENABLE_LOGGING) {
						fprintf (file.log, "Error: (%s line %d) ", fname, ln);
						fprintf (file.log, "'%s' has already been declared as an label!\n", c);
					}
				}
				symbols = realloc (symbols, (s_cnt + 1) * sizeof (struct Symbol));
				memcpy (symbols[s_cnt].label, c, sizeof(char) * (MAX_WORD_SIZE + 2));
				symbols[s_cnt].addr = instruction.addr;
				symbols[s_cnt].count = 0;
				symbols[s_cnt].ln = ln;
				symbols[0].count++;

				char addr_str[4];
				decToAddr (addr_str, instruction.addr);
				putSymbol (file.sym, c, addr_str);
			} else if (word_buf[i][0] == '\0') {
				break;
			} else if (!isValidOffset (word_buf[i])) {
				instruction.opcode = word_buf[i];
				instruction.addr++;
				break;
			} else {
				alert.err++;
				if (quiet < 2){
					printf ("Error: (%s line %d) ", fname, ln);
					printf ("Unrecognized token '%s'!\n", c);
				}
				if (ENABLE_LOGGING) {
					fprintf (file.log, "Error: (%s line %d) ", fname, ln);
					fprintf (file.log, "Unrecognized token '%s'!\n", c);
				}
				break;
			}
		}

		instruction.type = 0;
		int off = 0;

		switch (pseudoop) {
		case NO_OP:
		{
			break;
		}
		case ORIG:
		{
			instruction.type = 1;
			alert.err++;
			if (quiet < 2){
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("Multiple .ORIG declaration!\n%s", line_buf);
			}
			break;
		}
		case END:
		{
			goto pass_2;
		}
		case STRINGZ:
		{
			instruction.type = 1;
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
			instruction.type = 1;
			int off_type = isValidOffset (word_buf[i + 1]);
			
			off = offset (off_type, word_buf[i + 1]);

			for (int j = off; j > 1; j--) {
				instruction.addr++;
			}
			break;
		}
		case FILL:
		{
			instruction.type = 1;
			break;
		}
		case ALIAS:
		{
			instruction.type = 2;
			alert.err++;
			if (quiet < 2){
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("aliases must be declared before .ORIG!\n\t%s", line_buf);
			}
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
			if (word_buf[i][0] == '\0') {
				continue;
			} else if (pseudoop < 0) {
				errNoOp(instruction, &alert, file.log, word_buf[i]);
				instruction.type = -1;
			}
			break;
		}
		case BR:
		{
			instruction.type = 1;
			break;
		}
		case ADD:
		{
			instruction.type = 3;
			break;
		}
		case LD:
		{
			instruction.type = 2;
			break;
		}
		case ST:
		{
			instruction.type = 2;
			break;
		}
		case JSR:
		{
			instruction.type = 1;
			break;
		}
		case AND:
		{
			instruction.type = 3;
			break;
		}
		case LDR:
		{
			instruction.type = 3;
			break;
		}
		case STR:
		{
			instruction.type = 3;
			break;
		}
		case RTI:
		{
			instruction.type = 0;
			break;
		}
		case NOT:
		{
			instruction.type = 2;
			break;
		}
		case LDI:
		{
			instruction.type = 2;
			break;
		}
		case STI:
		{
			instruction.type = 2;
			break;
		}
		case JMP:
		{
			if(strcmp(word_buf[i], "RET")==0||strcmp(word_buf[i], "ret")==0)
				instruction.type = 0;
			else
				instruction.type = 1;
			break;
		}
		case LEA:
		{
			instruction.type = 2;
			break;
		}
		case TRAP:
		{
			if (isTrap (word_buf[i]) > 1)
				instruction.type = 0;
			else
				instruction.type = 1;
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
		if (instruction.type == 1)
			c = '\0';
		else
			c = 's';

		if (instruction.type >= 0) {
			if ((countWords (i, word_buf) - instruction.type) > 0) {
				warnOpOvf (instruction, &alert, file.log);
			} else if ((countWords (i, word_buf) - instruction.type) < 0) {
				errOpDef (instruction, &alert, file.log);
			}
		}
	}

pass_2:
	if (quiet < 1)
		printAlertSummary (alert);
	if (ENABLE_LOGGING)
		fprintAlertSummary (alert, file.log);

	if (file.sym != NULL)
		fclose(file.sym);

	//==========================================================================
	//	Pass 2 - Generate List, Binary, Hex, and Object files
	//==========================================================================
	if (quiet < 1)
		printf ("Pass 2:\n");
	if (ENABLE_LOGGING)
		fprintf (file.log, "Pass 2: \n");

	struct Alert alert_st;
	alert_st.warn = alert.warn;
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

		if (line_buf[0] == ';' || line_buf[0] == '\0'){
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
		while (existLabel (symbols, word_buf[i])) {
			i++;
		}

		char *op1 = word_buf[i + 1];
		char *op2 = word_buf[i + 2];
		char *op3 = word_buf[i + 3];

		int pseudoop = isPseuodoOp (word_buf[i]);
		switch (pseudoop) {
		case NO_OP:
		{
			break;
		}
		case ORIG:
		{
			alert.err++;
			if (quiet < 2) {
				printf ("Error: (%s line %d) ", fname, ln);
				printf ("Multiple .ORIG declaration\n\t%s", line_buf);
			}
			if (ENABLE_LOGGING) {
				fprintf (file.log, "Error: (%s line %d) ", fname, ln);
				fprintf (file.log, "Multiple .ORIG declaration\n\t%s", line_buf);
			}
			break;			
		}
		case END:		// clean up and return
		{
			unusedSymbol (symbols, &alert, file.log);
			unusedAlias (aliases, &alert, file.log);
			
			if (quiet < 1)
				printAlertSummary (alert);
			if (ENABLE_LOGGING)
				fprintAlertSummary (alert, file.log);
			alert.warn += alert_st.warn;
			alert.err += alert_st.err;
			alert.exception += alert_st.exception;

			if (quiet > 0 && alert.err > 0){
				printf ("%s: ", fname);
				printAlertSummary (alert);
			}

			free (symbols);
			free (aliases);

			if (file.bin != NULL)
				fclose (file.bin);
			if (file.hex != NULL)
				fclose (file.hex);
			if (file.lst != NULL)
				fclose (file.lst);
			if (file.obj != NULL)
				fclose (file.obj);
			if (file.log != NULL)
				fclose (file.log);

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
			if (!operandString (op1, &instruction, file, &alert))
				errInvalidOp (instruction, &alert, op1, file.log);
			instruction.type = N_OP;
			instruction.src = false;
			break;
		}
		case BLKW:
		{
			instruction.type = P_OP;
			instruction.src = true;
			off_type = isValidOffset (op1);

			off = offset (off_type, op1);

			if (off == 0)
				errInvalidOp (instruction, &alert, op1, file.log);

			for (int k = off; k > 0; k--){
				fprintAsm (file, instruction);
				if (instruction.src) instruction.src = false;
				instruction.addr++;
			}

			instruction.type = N_OP;
			instruction.src = false;
			break;
		}
		case FILL:
		{
			instruction.type = P_OP;
			instruction.src = true;
			int label_addr = labelAddress (symbols, op1);
			if (label_addr >= 0) {
				instruction.instr += label_addr;
			} else {
				off_type = isValidOffset (op1);
				instruction.instr += offset (off_type, op1);
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
			if (pseudoop < 0) {
				continue;
			} else {
				break;
			}
		}
		case BR:
		{
			offset_bits = 9;
			int branch = isBranch (word_buf[i]);
			if (branch >= 0) {
				instruction.instr += branch << 9;
			} else {
				alert.err++;
				if (quiet < 2){
					printf ("Error: (%s line %d) ", instruction.fname, instruction.ln);
					printf ("invalid branch conditions: %s\n", instruction.opcode);
				}
				if (ENABLE_LOGGING) {
					fprintf (file.log, "Error: (%s line %d) ", instruction.fname, instruction.ln);
					fprintf (file.log, "invalid branch conditions: %s\n", instruction.opcode);
				}
			}
			operandOffset (op1, &instruction, symbols, offset_bits, &alert, file.log);
			break;
		}
		case ADD:
		{
			offset_bits = 5;
			operandRegister (op1, 0, &instruction, &alert, file.log);
			operandRegister (op2, 1, &instruction, &alert, file.log);
			operandImmediate (op3, &instruction, offset_bits, &alert, file.log);
			break;
		}
		case LD:
		{
			offset_bits = 9;
			operandRegister (op1, 0, &instruction, &alert, file.log);
			operandOffset (op2, &instruction, symbols, offset_bits, &alert, file.log);
			break;
		}
		case ST:
		{
			offset_bits = 9;
			operandRegister (op1, 0, &instruction, &alert, file.log);
			operandOffset (op2, &instruction, symbols, offset_bits, &alert, file.log);
			break;
		}
		case JSR:
		{
			if (strcmp (word_buf[i], "JSR") == 0 || strcmp (word_buf[i], "jsr") == 0) {
				instruction.instr += 1 << 11;
				offset_bits = 11;
				operandOffset (op1, &instruction, symbols, offset_bits, &alert, file.log);
			} else {
				operandRegister (op1, 1, &instruction, &alert, file.log);
			}
			break;
		}
		case AND:
		{
			offset_bits = 5;
			operandRegister (op1, 0, &instruction, &alert, file.log);
			operandRegister (op2, 1, &instruction, &alert, file.log);
			operandImmediate (op3, &instruction, offset_bits, &alert, file.log);
			break;
		}
		case LDR:
		{
			offset_bits = 6;
			operandRegister (op1, 0, &instruction, &alert, file.log);
			operandRegister (op2, 1, &instruction, &alert, file.log);
			operandOffset (op3, &instruction, symbols, offset_bits, &alert, file.log);
			break;
		}
		case STR:
		{
			offset_bits = 6;
			operandRegister (op1, 0, &instruction, &alert, file.log);
			operandRegister (op2, 1, &instruction, &alert, file.log);
			operandOffset (op3, &instruction, symbols, offset_bits, &alert, file.log);
			break;
		}
		case RTI:
		{
			break;
		}
		case NOT:
		{
			instruction.instr += offsetMask (6);
			operandRegister (op1, 0, &instruction, &alert, file.log);
			operandRegister (op2, 1, &instruction, &alert, file.log);
			break;
		}
		case LDI:
		{
			offset_bits = 9;
			operandRegister (op1, 0, &instruction, &alert, file.log);
			operandOffset (op2, &instruction, symbols, offset_bits, &alert, file.log);
			break;
		}
		case STI:
		{
			offset_bits = 9;
			operandRegister (op1, 0, &instruction, &alert, file.log);
			operandOffset (op2, &instruction, symbols, offset_bits, &alert, file.log);
			break;
		}
		case JMP:
		{
			if(strcmp(word_buf[i], "RET")==0||strcmp(word_buf[i], "ret")==0)
				instruction.instr += offsetMask (3) << 6;
			else
				operandRegister (op1, 1, &instruction, &alert, file.log);
			break;
		}
		case LEA:
		{
			offset_bits = 9;
			operandRegister (op1, 0, &instruction, &alert, file.log);
			operandOffset (op2, &instruction, symbols, offset_bits, &alert, file.log);
			break;
		}
		case TRAP:
		{
			offset_bits = 8;
			trapShortcut (word_buf[i], op1);
			operandOffset (op1, &instruction, symbols, offset_bits, &alert, file.log);
			break;
		}
		default:
		{
			printf ("%d Unhandled keyword exception!\n", isKeyword (word_buf[i]));
			break;
		}
		}

		fprintAsm (file, instruction);
		if(instruction.type)
			instruction.addr++;
	}
}

int findOrig (struct Alias **als) {

}

struct Alias* addAlias (struct Alias *als, char *word, char *rep, int ln)
{
	int i = als[0].count;

	als = realloc (als, (i + 1) * sizeof (struct Alias));

	memcpy (als[i].word, word, sizeof (char) * (MAX_WORD_SIZE + 1));
	memcpy (als[i].replace, rep, sizeof (char) * (MAX_WORD_SIZE + 1));

	als[i].ln = ln;
	als[i].count = 0;
	als[0].count++;

	return als;
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

int unusedAlias (struct Alias *al, struct Alert *a, FILE *fp_log)
{
	for (int i = 1; i < al[0].count; i++) {
		if (al[i].count == 0) {
			a->warn++;
			if (quiet < 1) {
				printf ("Warning: (%s line %d) ", al[0].word, al[i].ln);
				printf ("unused alias: %s\n", al[i].word);
			}
			if (ENABLE_LOGGING) {
				fprintf (fp_log, "Warning: (%s line %d) ", al[0].word, al[i].ln);
				fprintf (fp_log, "unused alias: %s\n", al[i].word);
			}
		}
	}
	return 1;
}

int existAlias (char *op, struct Alias *al)
{
	for (int i = al[0].count; i >= 1; i--) {
		if (strcmp (op, al[i].word) == 0) {
			printf ("%s %s\n", op, al[i].word);
			return 1;
		}
	}
	return 0;
}

int unusedSymbol (struct Symbol *sym, struct Alert *a, FILE *fp_log)
{
	for (int i = 1; i < sym[0].count; i++) {
		if (sym[i].count == 0) {
			a->warn++;
			if (quiet < 1) {
				printf ("Warning: (%s line %d) ", sym[0].label, sym[i].ln);
				printf ("unused label: %s\n", sym[i].label);
			}
			if (ENABLE_LOGGING) {
				fprintf (fp_log, "Warning: (%s line %d) ", sym[0].label, sym[i].ln);
				fprintf (fp_log, "unused label: %s\n", sym[i].label);
			}
		}
	}
	return 1;
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

int existLabel (struct Symbol *sym, char *label)
{
	for (int i = 1; i < sym[0].count; i++) {
		if (strcmp (sym[i].label, label) == 0) {
			return 1;
		}
	}
	return 0;
}

int operandImmediate (char *op, struct Instruction *ins, int off_b, struct Alert *a, FILE *fp_log)
{
	int off;
	int off_type = isValidOffset (op);
	int r = isRegister (op);
	if (r >= 0) {
		ins->instr += r;
		return 1;
	} else if (off_type <= 3 && off_type >= 1) {
		off = offset (off_type, op);
	} else {
		a->err++;
		if (quiet < 2) {
			printf ("Error: (%s line %d) ", ins->fname, ins->ln);
			printf ("invalid operand for '%s': %s!\n", ins->opcode, op);
		}
		if (ENABLE_LOGGING) {
			fprintf (fp_log, "Error: (%s line %d) ", ins->fname, ins->ln);
			fprintf (fp_log, "invalid operand for '%s': %s!\n", ins->opcode, op);
		}
		return 0;
	}

	if (off > (1 << off_b - 1) - 1 || off < -(1 << off_b - 1)) {
		a->err++;
		if (quiet < 2){
			printf ("Error: (%s line %d) ", ins->fname, ins->ln);
			printf ("%d will not fit in %d offset bits!\n", off, off_b);
		}
		if (ENABLE_LOGGING) {
			fprintf (fp_log, "Error: (%s line %d) ", ins->fname, ins->ln);
			fprintf (fp_log, "%d will not fit in %d offset bits!\n", off, off_b);
		}
		return 0;
	} else {
		ins->instr += off & offsetMask (off_b);
		ins->instr += 1 << off_b;
		return 1;
	}
}

int operandRegister (char *op, int loc, struct Instruction *ins, struct Alert *a, FILE *fp_log)
{
	int r = isRegister (op);
	if (loc == 2) loc = 3;
	int shift = 9 - (3 * loc);
	if (r >= 0) {
		ins->instr += r << shift;
		return 1;
	} else {
		a->err++;
		if (quiet < 2) {
			printf ("Error: (%s line %d) ", ins->fname, ins->ln);
			printf ("'%s' expects operand type register; ", ins->opcode);
			printf ("'%s' is not a register!\n", op);
		}
		if (ENABLE_LOGGING) {
			fprintf (fp_log, "Error: (%s line %d) ", ins->fname, ins->ln);
			fprintf (fp_log, "'%s' expects operand type register; ", ins->opcode);
			fprintf (fp_log, "'%s' is not a register!\n", op);
		}
		return 0;
	}
}

int operandOffset (char *op, struct Instruction *ins, struct Symbol *sym, int off_b, struct Alert *a, FILE *fp_log)
{
	int s_cnt = sym[0].count;
	int off;
	int off_type = isValidOffset (op);
	int label_addr = labelAddress (sym, op);
	if (label_addr >= 0){
		off = label_addr - ins->addr -1;
	} else if (off_type >= 1 && off_type <= 3) {
		off = offset (off_type, op);
	} else {
		a->err++;
		if (quiet < 2) {
			printf ("Error: (%s line %d) ", ins->fname, ins->ln);
			printf ("invalid operand for '%s': %s!\n", ins->opcode, op);
		}
		if (ENABLE_LOGGING) {
			fprintf (fp_log, "Error: (%s line %d) ", ins->fname, ins->ln);
			fprintf (fp_log, "invalid operand for '%s': %s!\n", ins->opcode, op);
		}
		return 0;
	}

	if (off > (1 << off_b - 1) - 1 || off < -(1 << off_b - 1)) {
		a->err++;
		if (quiet < 2) {
			printf ("Error: (%s line %d) ", ins->fname, ins->ln);
			printf ("%d will not fit in %d offset bits!\n", off, off_b);
		}
		if (ENABLE_LOGGING) {
			fprintf (fp_log, "Error: (%s line %d) ", ins->fname, ins->ln);
			fprintf (fp_log, "%d will not fit in %d offset bits!\n", off, off_b);
		}
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
				return 1;
			} else if (str[i] == '\\' && escChar) {
				ins->instr = escChar;
				fprintAsm (file, *ins);
				if (ins->src) ins->src = false;
				ins->addr++;
				i += 2;
			} else if (str[i] == '\\' && !escChar) {
				a->err++;
				if (quiet < 2) {
					printf ("Error: (%s line %d) ", ins->fname, ins->ln);
					printf ("invalid escape sequence in string %s\n", str);
				}
				if (ENABLE_LOGGING) {
					fprintf (file.log, "Error: (%s line %d) ", ins->fname, ins->ln);
					fprintf (file.log, "invalid escape sequence in string %s\n", str);
				}
			} else {
				ins->instr = str[i];
				fprintAsm (file, *ins);
				if (ins->src) ins->src = false;
				ins->addr++;
				i++;
			}
		}
	} else {
		return 0;
	}
	return 1;
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
	char *warn, *err, *exception;
	if (alert.err == 1)
		err = "error";
	else
		err = "errors";
	if (alert.exception == 1)
		exception = "exception";
	else
		exception = "exceptions";
	if (alert.warn == 1)
		warn = "warning";
	else
		warn = "warnings";
	printf ("%d %s,", alert.err, err);
	printf (" %d %s,", alert.warn, warn);
	printf (" and %d %s!\n", alert.exception, exception);
}

void fprintAlertSummary (struct Alert alert, FILE *fp_log)
{
	char *warn, *err, *exception;
	if (alert.err == 1)
		err = "error";
	else
		err = "errors";
	if (alert.exception == 1)
		exception = "exception";
	else
		exception = "exceptions";
	if (alert.warn == 1)
		warn = "warning";
	else
		warn = "warnings";
	fprintf (fp_log, "%d %s,", alert.err, err);
	fprintf (fp_log, " %d %s,", alert.warn, warn);
	fprintf (fp_log, " and %d %s!\n", alert.exception, exception);
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

int warnOpOvf (struct Instruction ins, struct Alert *a, FILE *fp_log)
{
	if (fp_log == NULL && ENABLE_LOGGING)
		return 0;

	char c;
	if (ins.type == 1)
		c = '\0';
	else
		c = 's';

	a->warn++;
	if (quiet < 1) {
		printf ("Warning: (%s line %d) ", ins.fname, ins.ln);
		printf ("'%s' takes %d operand%c!\n", ins.opcode, ins.type, c);
		printf ("\t%s", ins.line_buf);
	}
	if (ENABLE_LOGGING) {
		fprintf (fp_log, "Warning: (%s line %d) ", ins.fname, ins.ln);
		fprintf (fp_log, "'%s' takes %d operand%c!\n", ins.opcode, ins.type, c);
		fprintf (fp_log, "\t%s", ins.line_buf);
	}
	return 1;
}

int errNoOp (struct Instruction ins, struct Alert *a, FILE *fp_log, char *c)
{
	if (fp_log == NULL && ENABLE_LOGGING)
		return 0;

	a->err++;
	if (quiet < 1) {
		printf ("Error: (%s line %d) ", ins.fname, ins.ln);
		printf ("newline cannot start with '%s'!\n", c);
		printf ("\t%s", ins.line_buf);
	}
	if (ENABLE_LOGGING) {
		fprintf (fp_log, "Error: (%s line %d) ", ins.fname, ins.ln);
		fprintf (fp_log, "newline cannot start with '%s'!\n", c);
		fprintf (fp_log, "\t%s", ins.line_buf);
	}
	return 1;
}

int errOpDef (struct Instruction ins, struct Alert *a, FILE *fp_log)
{
	if (fp_log == NULL && ENABLE_LOGGING)
		return 0;

	char c;
	if (ins.type == 1)
		c = '\0';
	else
		c = 's';

	a->err++;
	if (quiet < 2) {
		printf ("Error: (%s line %d) ", ins.fname, ins.ln);
		printf ("'%s' requires %d operand%c!\n", ins.opcode, ins.type, c);
		printf ("\t%s", ins.line_buf);
	}
	if (ENABLE_LOGGING) {
		fprintf (fp_log, "Error: (%s line %d) ", ins.fname, ins.ln);
		fprintf (fp_log, "'%s' requires %d operand%c!\n", ins.opcode, ins.type, c);
		fprintf (fp_log, "\t%s", ins.line_buf);
	}
	return 1;
}

int errInvalidOp (struct Instruction ins, struct Alert *a, char *op, FILE *fp_log)
{
	if (fp_log == NULL && ENABLE_LOGGING)
		return 0;
	a->err++;
	if (quiet < 2) {
		printf ("Error: (%s line %d) ", ins.fname, ins.ln);
		printf ("invalid operand for '%s': %s\n", ins.opcode, op);
	}
	if (ENABLE_LOGGING) {
		fprintf (fp_log, "Error: (%s line %d) ", ins.fname, ins.ln);
		fprintf (fp_log, "invalid operand for '%s': %s\n", ins.opcode, op);
	}
	return 1;
}