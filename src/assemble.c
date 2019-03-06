#define USES_FILE
#define USES_ALIAS
#define USES_MACRO
#define USES_LABEL
#define USES_NOTIFY
#define USES_OFFSET
#define USES_OPERAND
#define USES_PSEUDOOP
#define USES_TOKENIZE
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

		char **token = tokenize (line);
		if (token[0] == NULL){													// skip empty lines
			free_token (token);
			continue;
		}

		switch (ispseudoop (token[0])) {
		case ALIAS:
			a->alias = addalias (a->alias, *ln, token[1], token[2]);
			break;
		case MACRO:
			a->macro = addmacro (a->macro, *ln, token[1], token[2]);
			break;
		case ORIG:
			orig_addr = offset(1, token[1]);
			break;
		default:
			error (WARN, f.log_, ins,
				   "Ignoring invalid token '%s' before '.ORIG'", token[0]);
			break;
		}

		token = free_token (token);
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
		char **token = tokenize (line);
		if (token[0] == NULL){													// skip empty lines
			token = free_token (token);
			continue;
		}
		
		struct Instruction ins = {0, i + 1, line};
		int8_t opcode = -1, pseudoop = -1, opnum = -1, opcount = 0;
		uint64_t toknum = (uint64_t) *(token - 1);
		char *op = "\0";

		for (uint8_t j = 0; j < toknum; j++) {
			char *tok = token[j];
			if (isvalidlabel (tok) && j == 0) {
				arrs->label = addlabel (arrs->label, ln, tok, addr);
				printsymbol (f.sym_ ,tok, addr);
				opnum = 0;
			} else if ((opcode = isoperand (tok)) >= 0) {						// is an opcode
				op = tok;
				opnum = operandnum (opcode);
				addr++;
			} else if ((pseudoop = ispseudoop (tok)) >= 0) {					// is pseudoop
				op = tok;
				opnum = poperandnum (pseudoop);
				if (token[j + 1]) addr += addrnum (pseudoop, token[j + 1]);
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

		token = free_token (token);
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
		(Alias*) malloc (DEFAULT_ALIAS_NUM * sizeof (struct Alias)),
		(Macro*) malloc (DEFAULT_MACRO_NUM * sizeof (struct Macro)),
		(Label*) malloc (DEFAULT_LABEL_NUM * sizeof (struct Label))
	};
	arrs.alias[0].count = 0;
	arrs.macro[0].count = 0;
	arrs.label[0].count = 0;

	uint32_t ln = 1;
	uint16_t origaddr = origof (f, &ln, &alt, &arrs);
	uint32_t ln_st = ln;
	
	fpos_t origpos;
	fgetpos (f.asm_, &origpos);

	// Pass 1 TODO
	passone (f, ln, origaddr, &alt, &arrs);

	fsetpos (f.asm_, &origpos);
	// Pass 2 TODO
	passtwo (f, ln, origaddr, &alt, &arrs);

	// cleanup
	if (alt.err > 0 || alt.exceptions > 0) clean (file);
	arrs.alias = freealiasarr (arrs.alias);
	arrs.macro = freemacroarr (arrs.macro);
	arrs.label = freelabelarr (arrs.label);
	closeasmfiles (&f);

	return err;
}