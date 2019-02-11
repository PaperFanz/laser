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
	for (int i = 0; i < (ENABLE_LOGGING ? 6 : 5); i++) {
		file = replaceextension (file, extensions[i]);
		notify ("Deleting %s...\n", file);
		if (remove (file)) {
			notify ("Unable to delete %s!\n", file);
			err = 1;
		}
	}

	file = replaceextension (file, ".sym");

	if (err) return -2;
	else return 0;
}

struct Alert {
	int32_t warn;
	int32_t err;
	int32_t exceptions;
};

struct Arrays {
	struct Alias *alias;
	struct Macro *macro;
	struct Label *label;
};

uint16_t origof (struct Files f, uint32_t *ln, struct Alert *alert, struct Arrays *a)
{
	uint16_t orig_addr = 0;
	int32_t i = 0;
	struct Instruction ins = {0, 0, ""};

	char line[MAX_LEN + 1];
	char **token_buf;

	for (i = *ln; fgets (line, MAX_LEN + 1, f.asm_) != NULL; i++) {
		char token[MAX_WORD_NUM][MAX_WORD_SIZE] = {0};
		ins.ln = i;
		ins.line = line;

		tokenize (line, token);
		if (!token[0][0]) continue;	// skip empty lines

		int8_t pseudoop = ispseudoop (token[0]);
		if (pseudoop == ALIAS) {
			a->alias = addalias (a->alias, *ln, token[1], token[2]);
		} else if (pseudoop == MACRO) {
			a->macro = addmacro (a->macro, *ln, token[1], token[2]);
		} else if (pseudoop == ORIG) {
			orig_addr = offset(1, token[1]);
			break;
		}
	}

	if (!orig_addr) error (ERR, f.log_, ins, "no origin detected!");

	*ln = i;
	return orig_addr;
}

void passone (struct Files f, uint32_t *ln, struct Alert *alert,
			  struct Alias *a, struct Macro *m, struct Label *l)
{

}

void passtwo (struct Files f, uint32_t *ln, struct Alert *alert,
			  struct Alias *a, struct Macro *m, struct Label *l)
{

}

int8_t assemble (char *file)
{
	int8_t err = 0;
	struct Alert a = {0, 0, 0};
	notify ("Assembling %s...\n", file);

	struct Files f;
	err += openasmfiles (&f, file);
	if (err) return err;	// exit with error if files unopenable

	struct Arrays arrs = {
		malloc (DEFAULT_ALIAS_NUM * sizeof (struct Alias)),
		malloc (DEFAULT_MACRO_NUM * sizeof (struct Macro)),
		malloc (DEFAULT_LABEL_NUM * sizeof (struct Label))
	};
	arrs.alias[0].count = 0;
	arrs.macro[0].count = 0;
	arrs.label[0].count = 0;

	uint32_t ln = 1;
	uint16_t origaddr = origof (f, &ln, &a, &arrs);
	fpos_t origpos;
	fgetpos (f.asm_, &origpos);

	// Pass 1 TODO

	fsetpos (f.asm_, &origpos);
	// Pass 2 TODO

	free (arrs.alias);
	free (arrs.macro);
	free (arrs.label);

	return err;
}