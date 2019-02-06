#define USES_ASSEMBLE
#define USES_FLAG
#include "laser.h"

int8_t checkExtension (char *file, char *extension)
{
	char *dot=strrchr(file, '.');
	if (dot == NULL)
		return 0;
	else if (strcmp (dot, extension) == 0)
		return 1;
	else
		return 0;
}

char* replaceExtension (char *file, const char *extension)
{
	char *dot=strrchr(file, '.');
	int i=0;
	while (dot[i] != '\0') {
		dot[i] = extension[i];
		i++;
	}
	return file;
}

int8_t parseFile (char *file, int8_t last_flag)
{
	if (checkExtension (file, ".asm")) {
		if (last_flag == ASSEMBLE) {
			return assemble (file);
		} else if (last_flag == CLEAN) {
			return clean (file);
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

void fprintIntArr (FILE *fp, int num[], int size)
{
	for (int i = 0; i <= size - 1; i++)
		fprintf(fp, "%d", num[i]);
}

void fprintCharArr (FILE *fp, char hex[], int size)
{
	for (int i = 0; i <= size - 1; i++)
		fprintf (fp, "%c", hex[i]);
}

void printSymbol (FILE *fp, char symbol[], char addr[])
{
	int i = 0;
	fprintf (fp, "%s", symbol);
	if (USE_SPACES_IN_SYM) {
		for (i = 35 - symbol[MAX_WORD_SIZE + 1]; i >= 0; i--)
			fprintf (fp, " ");
	} else {
		for (i = 8 - (symbol[MAX_WORD_SIZE + 1] / TABSIZE); i >= 0; i--)
			fprintf (fp, "\t");
	}
	fprintf (fp, "x");
	fprintCharArr(fp, addr, 4);
	fprintf (fp, "\n");
}