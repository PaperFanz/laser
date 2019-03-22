#define USES_ASSEMBLE
#define USES_LABEL
#define USES_FLAG
#define USES_FILE
#define USES_NOTIFY
#include "laser.h"

int8_t checkextension (char *file, char *extension)
{
	char *dot=strrchr(file, '.');
	if (dot == NULL)
		return 0;
	else if (strcmp (dot, extension) == 0)
		return 1;
	else
		return 0;
}

char* replaceextension (char *file, const char *extension)
{
	char *dot=strrchr(file, '.');
	int i=0;
	while (dot[i] != '\0') {
		dot[i] = extension[i];
		i++;
	}
	return file;
}

int8_t parsefile (char *file, int8_t last_flag)
{
	if (checkextension (file, ".asm")) {
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

uint8_t openasmfiles (struct Files *f, char *file)
{
	uint8_t failed = 0;
	f->fp = fopen (file, "r");
	if (f->fp == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}
	replaceextension (file, ".sym");
	
	f->sym = fopen (file, "w");
	if (f->sym == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}
	replaceextension (file, ".bin");

	f->bin = fopen (file, "w");
	if (f->bin == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}
	replaceextension (file, ".hex");

	f->hex = fopen (file, "w");
	if (f->hex == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}
	replaceextension (file, ".obj");

	f->obj = fopen (file, "w");
	if (f->obj == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}
	replaceextension (file, ".lst");

	f->lst = fopen (file, "w");
	if (f->lst == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}

	if (ENABLE_LOGGING) {
		replaceextension (file, ".log");
		f->log = fopen (file, "w");
		if (f->log == NULL) {
			notify ("Unable to open %s!\n", file);
			failed++;
		}
	}
	replaceextension (file, ".sym");

	return failed;
}

void closeasmfiles (struct Files *f)
{
	if (f->fp != NULL) fclose (f->fp);
	if (f->sym != NULL) fclose (f->sym);
	if (f->bin != NULL) fclose (f->bin);
	if (f->hex != NULL) fclose (f->hex);
	if (f->obj != NULL) fclose (f->obj);
	if (f->lst != NULL) fclose (f->lst);
	if (f->log != NULL) fclose (f->log);
}

static uint16_t insfilebuffer[0xFFFF];
static uint32_t lnfilebuffer[0xFFFF];
static uint32_t filebufferindex = 0;

void writefilebuf (uint16_t ins, uint32_t ln)
{
	static uint8_t note = 0;
	if (filebufferindex < 0xFFFF) {
		insfilebuffer[filebufferindex] = ins;
		lnfilebuffer[filebufferindex] = ln;
		filebufferindex++;
	} else if (!note) {
		notify ("Congratulations, you've somehow managed to write a program long enough to saturate the LC3 memory...");
		note = 1;
	}
}

void resetfilebuf ()
{
	filebufferindex = 0;
}

void writeobj (FILE *obj)
{
	uint32_t bytenum = filebufferindex * 2;
	uint8_t tmp[bytenum];
	for (uint16_t i = 0; i < filebufferindex; i++) {
		tmp[i * 2 + 1] = insfilebuffer[i] & 0xFF;
		tmp[i * 2] = insfilebuffer[i] >> 8;
	}

	fwrite (tmp, sizeof (uint8_t), bytenum, obj);
}

void writehex (FILE *hex)
{
	for (uint32_t i = 0; i < filebufferindex; i++) {
		fprintf (hex, "%04X\n", insfilebuffer[i]);
	}
}

const char *XtoB[16] = {
	"0000", "0001", "0010", "0011",
	"0100", "0101", "0110", "0111",
	"1000",	"1001",	"1010",	"1011",
	"1100",	"1101",	"1110",	"1111",
};

#define instobin(ins) \
	XtoB[ins >> 12], \
	XtoB[(ins >> 8) & 0xF], \
	XtoB[(ins >> 4) & 0xF], \
	XtoB[ins & 0xF]

void writebin (FILE *bin)
{
	for (uint32_t i = 0; i < filebufferindex; i++) {
		uint16_t tmp = insfilebuffer[i];
		fprintf (bin, "%s%s%s%s\n", instobin(tmp));
	}
}

void printsymbol (FILE *fp, Token *symbol, uint16_t addr)
{
	uint16_t len = symbol->len;
	if (len >= 75) len = 75;
	else len = len;
	fprintf (fp, "%s", symbol->str);

	if (USE_SPACES_IN_SYM) {
		for (uint16_t i = (76 - len); i > 0; i--)
			fprintf (fp, " ");
	} else {
		for (uint16_t i = (72 - len) / TABSIZE; i > 0; i--)
			fprintf (fp, "\t");
	}

	fprintf (fp, "x%04X", addr);
	fprintf (fp, "\n");
}

const char* lstheader = "  HEX  |      BINARY      |  LN  |  ASSEMBLY\n";

void writelst (FILE *fp, FILE *lst)
{
	fseek (fp, 0, SEEK_SET);													// find beginning of .asm file
	fprintf (lst, "%s", lstheader);

	char line[MAX_LEN + 1];
	uint16_t lnindex = 0;
	for (uint32_t i = 1; fgets (line, MAX_LEN + 1, fp); i++) {
		if (lnfilebuffer[lnindex] == i) {
			uint16_t tmp = insfilebuffer[lnindex];
			fprintf (lst, " x%04X | %s%s%s%s ", tmp, instobin(tmp));
			lnindex++;
		} else {
			fprintf (lst, "       |                  ");
		}

		fprintf (lst, "| %4d | %s", i, line);

		while (lnfilebuffer[lnindex] == i) {									// necessary for STRINGZ and BLKW where
			uint16_t tmp = insfilebuffer[lnindex];								// a single line of assembly generates
			fprintf (lst, " x%04X | %s%s%s%s |      |\n", tmp, instobin(tmp));	// multiple lines of values
			lnindex++;
		}
	}
}