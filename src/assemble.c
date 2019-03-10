#define USES_FILE
#define USES_ALIAS
#define USES_MACRO
#define USES_LABEL
#define USES_NOTIFY
#define USES_OFFSET
#define USES_OPERAND
#define USES_PSEUDOOP
#include "laser.h"

const char *extensions[] = {
	".sym",
	".bin",
	".hex",
	".lst",
	".obj",
	".log"
};

int8_t clean (char *file)
{
	bool err = 0;
	notify ("Cleaning up...\n");
	for (int i = 0; i < (ENABLE_LOGGING ? 6 : 5); i++) {
		file = replaceextension (file, extensions[i]);
		notify ("  Deleting %s...\n", file);
		if (remove (file)) {
			notify ("    Unable to delete %s!\n", file);
			err = 1;
		}
	}
	notify ("Finished!\n");

	file = replaceextension (file, ".sym");

	if (err) return -2;
	else return 0;
}

typedef struct Alert {
	int32_t warn;
	int32_t err;
	int32_t exceptions;
} Alert;

typedef struct Arrays {
	struct Alias *alias;
	struct Macro *macro;
	struct Label *label;
} Arrays;

uint16_t origof (struct Files f, uint32_t *ln, Alert *alt, Arrays *a)
{
	uint16_t orig_addr = 0;
	int32_t i = 0;
	char line[MAX_LEN + 1];

	for (i = *ln; fgets (line, MAX_LEN + 1, f.asm_) != NULL; i++) {
		struct Instruction ins = {0, i, line};

		TokenBuffer *buf = tokenize (line);
		Token **token = buf->token;
		if (buf->token[0] == NULL){													// skip empty lines
			freetokenarr (buf);
			continue;
		}

		switch (ispseudoop (token[0]->str)) {
		case ALIAS:
			addalias (a->alias, *ln, token[1], token[2]);
			break;
		case MACRO:
			a->macro = addmacro (a->macro, *ln, token[1], token[2]);
			break;
		case ORIG:
			orig_addr = offset(1, token[1]->str);
			break;
		default:
			error (WARN, f.log_, ins,
				   "Ignoring invalid token '%s' before '.ORIG'", token[0]);
			break;
		}

		freetokenarr (buf);
		if (orig_addr) break;
	}

	struct Instruction ins = {0, 0, ""};
	if (!orig_addr) error (ERR, f.log_, ins, "No origin detected!\n");

	*ln = i;
	return orig_addr;
}

void passone (struct Files f, uint32_t ln, uint16_t addr,
			  Alert *alt, Arrays *arrs)
{
	char line[MAX_LEN + 1];
	int8_t end = 0;

	for (uint32_t i = ln; fgets (line, MAX_LEN + 1, f.asm_) != NULL; i++) {
		TokenBuffer *buf = tokenize (line);
		Token **token = buf->token;
		if (buf->toknum == 0){													// skip empty lines
			freetokenarr (buf);
			continue;
		}
		
		struct Instruction ins = {0, i + 1, line};
		int8_t opcode = -1, pseudoop = -1, opnum = -1, opcount = 0;
		char *op = "\0";

		for (uint8_t j = 0; j < buf->toknum; j++) {
			char *tok = token[j]->str;
			if (isvalidlabel (tok) && j == 0) {
				arrs->label = addlabel (arrs->label, ln, token[j], addr);
				printsymbol (f.sym_, token[j], addr);
				opnum = 0;
			} else if ((opcode = isoperand (tok)) >= 0) {						// is an opcode
				op = tok;
				opnum = operandnum (opcode);
				addr++;
			} else if ((pseudoop = ispseudoop (tok)) >= 0) {					// is pseudoop
				op = tok;
				opnum = poperandnum (pseudoop);
				if (j + 1 < buf->toknum) addr += addrnum (pseudoop, token[j + 1]->str);
				if (pseudoop == END) end = 1;
			} else if (isregister (tok) >= 0) {
				opcount++;
			} else if (offtype (tok) >= 0) {
				opcount++;
			} else {															// unrecognized token
				alt->err++;
				error (ERR, f.log_, ins, "Unrecognized token '%s'", tok);
			}
		}

		if (opcount > opnum) {
			alt->warn++;
			error (WARN, f.log_, ins, "'%s' expects %d operands", op, opnum);
		} else if (opcount < opnum) {
			alt->err++;
			error (ERR, f.log_, ins, "'%s' expects %d operands", op, opnum);
		}

		freetokenarr (buf);
		if (end) break;
	}

	struct Instruction ins = {0, 0, ""};
	if (!end) error (ERR, f.log_, ins, "No end detected!\n");
}

void passtwo (struct Files f, uint32_t ln, uint16_t addr,
			  Alert *alt, Arrays *arrs)
{

}

int8_t assemble (char *file)
{
	int8_t err = 0;
	Alert alt = {0, 0, 0};
	notify ("Assembling %s...\n", file);

	struct Files f = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	err += openasmfiles (&f, file);
	if (err) return err;	// exit with error if files unopenable

	Arrays arrs = {
		initaliasarr(),
		initmacroarr(),
		initlabelarr()
	};

	uint32_t ln = 1;
	uint16_t origaddr = origof (f, &ln, &alt, &arrs);
	uint32_t ln_st = ln;
	
	fpos_t origpos;
	fgetpos (f.asm_, &origpos);

	// Pass 1 TODO
	passone (f, ln, origaddr, &alt, &arrs);

	// Pass 2 TODO
	if (!alt.err || !alt.exceptions) {
		fsetpos (f.asm_, &origpos);
		passtwo (f, ln, origaddr, &alt, &arrs);
	}

	// cleanup
	if (alt.err > 0 || alt.exceptions > 0) clean (file);
	freealiasarr (arrs.alias);
	freemacroarr (arrs.macro);
	freelabelarr (arrs.label);
	closeasmfiles (&f);

	return err;
}