#define USES_FILE
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
		file = replaceExtension (file, extensions[i]);
		notify ("Deleting %s...\n", file);
		if (remove (file)) {
			notify ("Unable to delete %s!\n", file);
			err = 1;
		}
	}

	file = replaceExtension (file, ".sym");

	if (err) return -2;
	else return 0;
}

struct Alert {
	int32_t warn;
	int32_t err;
	int32_t exceptions;
};

uint16_t origof (FILE *fp, uint32_t *ln, struct Alert *a)
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

		int8_t t = ispseudoop (token[0]);

		if (t == ALIAS) {

		} else if (t == MACRO) {

		} else if (t == ORIG) {
			
			orig = 1;
			break;
		} else {

		}

		if (orig) break;	// exit when orig is found
	}

	*ln = i;
	return orig_addr;
}

int8_t assemble (char *file)
{
	notify ("Assembling %s...\n", file);
	return 0;
}