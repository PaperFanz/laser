#include "calc.h"
#include "convert.h"

int offsetMask (int offset_bits)
{
	int mask = 0;
	for (int i = 0; i < offset_bits; i++)
		mask += 1 << i;
	return mask;
}

int checkExt (const char *filename, const char *ext)
{
	char *dot=strrchr(filename, '.');
	if (dot == NULL)
		return 0;
	else if (strcmp (dot, ext) == 0)
		return 1;
	else
		return 0;
}

char *replaceExt (char *filename, const char *ext)
{
	char *dot=strrchr(filename, '.');
	int i=0;
	while (dot[i] != '\0') {
		dot[i] = ext [i];
		i++;
	}
	return filename;
}

int hexOffset (char *offset)
{
	char *hex = strchr (offset, 'x');
	hex++;
	if (hex[0] == '-') {
		hex++;
	}

	if (strchr(offset, '-') != NULL) {
		return -hexToDec (hex);
	} else {
		return hexToDec (hex);
	}
}

int binOffset (char *offset)
{
	// move pointer past leading characters
	char *bin = strchr (offset, 'b');
	bin++;
	if (bin[0] == '-')
		bin++;

	// get array length
	int i = 0, l = 0, j = 16;
	while (bin[i] != '\0') {
		l++;
		i++;
	}

	int bin16[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	for (i = l; i >= 0; i--) {
		bin16[j] = bin[i] - 0x30;
		j--;
	}

	// sign extend
	for (i = j; i >= 0; i--) {
		bin16[i] = bin16[j + 1];
	}

	if (strchr (offset, '-')) {
		return -twoCompToDec (bin16, 16);
	} else {
		return twoCompToDec (bin16, 16);
	}
}

int decOffset (char *offset)
{
	char *dec = strchr (offset, '#');
	if (dec == NULL) {
		dec = offset;
	} else {
		dec++;
	}

	if (dec[0] == '-') {
		dec++;
	}

	int off = 0, i = 0;
	while (dec[i] != '\0') {
		off = off * 10 + (dec[i] - 0x30);
		i++;
	}

	if (strchr (offset, '-')) {
		return -off;
	} else {
		return off;
	}
}

int offset (int off_type, char *op)
{
	int off;
	if (off_type == 1) {
		off = hexOffset (op);
	} else if (off_type == 2) {
		off = binOffset (op);
	} else if (off_type == 3) {
		off = decOffset (op);
	} else {
		off = 0;
	}

	return off;
}

// print functions
void printIntArr (int num[], int size)
{
	for (int i = 0; i <= size - 1; i++)
		printf("%d", num[i]);
	printf("\n");
}

void fprintIntArr (FILE *fp, int num[], int size)
{
	for (int i = 0; i <= size - 1; i++)
		fprintf(fp, "%d", num[i]);
}

void printCharArr (char hex[], int size)
{
	for (int i = 0; i <= size-1; i++)
		printf("%c", hex[i]);
	printf("\n");
}

void fprintCharArr (FILE *fp, char hex[], int size)
{
	for (int i = 0; i <= size - 1; i++)
		fprintf (fp, "%c", hex[i]);
}

void putSymbol (FILE *fp, char symbol[], char addr[])
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