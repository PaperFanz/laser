#define USES_FILE
#define USES_ALIAS
#define USES_MACRO
#define USES_LABEL
#define USES_NOTIFY
#define USES_OPERAND
#define USES_PSEUDOOP
#define USES_TOKENIZE
#include "laser.h"

const char *extensions[] = {
	".sym",
	".bin",
	".hex",
	".lst",
	".obj"
};

int8_t clean (char *file)
{
	bool err = 0;
	for (int i = 0; i < 5; i++) {
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

uint16_t origof (FILE *fp, uint32_t *ln, struct Alert *alert,
				 struct Alias *a, struct Macro *m)
{
	uint16_t orig_addr = -1;
	int8_t orig = 0;
	int32_t i = 0;

	char line[MAX_LEN + 1];
	char **token_buf;

	for (i = *ln; fgets (line, MAX_LEN + 1, fp) != NULL; i++) {
		char token[MAX_WORD_NUM][MAX_WORD_SIZE] = {0};

		tokenize (line, token);
		if (!token[0][0] || token[0][0] == ';') continue;	//skip comments

		switch (ispseudoop (token[0])) {
		case ALIAS: {
			a = addalias (a, *ln, token[1], token[2]);
			break;
		}
		case MACRO: {
			m = addmacro (m, *ln, token[1], token[2]);
			break;
		}
		case ORIG: {
			printf ("%s\n", token[1]);
			orig = true;
			break;
		}
		default: {
			printf ("%s\n", "err");
			break;
		}
		}

		if (orig) break;	// exit when orig is found
	}

	*ln = i;
	return orig_addr;
}

int8_t assemble (char *file)
{
	notify ("Assembling %s...\n", file);
	FILE *fp = fopen (file, "r");

	struct Alias *aliases = malloc (sizeof (struct Alias));
	aliases[0].count = 0;
	struct Macro *macros = malloc (sizeof (struct Macro));
	macros[0].count = 0;
	struct Label *labels = malloc (sizeof (struct Label));
	labels[0].count = 0;

	struct Alert alerts;
	alerts.err = 0;
	alerts.exceptions = 0;
	alerts.warn = 0;
	

	uint32_t ln = 1;
	uint16_t origaddr = origof (fp, &ln, &alerts, aliases, macros);

	// Pass 1 TODO

	// Pass 2 TODO

	free (aliases);
	free (macros);
	free (labels);

	return 0;
}