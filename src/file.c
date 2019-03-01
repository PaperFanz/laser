#define USES_ASSEMBLE
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

void fprintintarr (FILE *fp, int *num, int size)
{
	for (int i = 0; i <= size - 1; i++)
		fprintf(fp, "%d", num[i]);
}

void fprintchararr (FILE *fp, char *hex, int size)
{
	for (int i = 0; i <= size - 1; i++)
		fprintf (fp, "%c", hex[i]);
}

void printsymbol (FILE *fp, char *symbol, uint16_t addr)
{
	int i = 0, tmp = symbol[-1];
	if (tmp >= 74) tmp = 74;
	else tmp = tmp;
	fprintf (fp, "%s", symbol);

	if (USE_SPACES_IN_SYM) {
		for (i = 75 - tmp; i >= 0; i--)
			fprintf (fp, " ");
	} else {
		for (i = (72 - tmp) / TABSIZE; i >= 0; i--)
			fprintf (fp, "\t");
	}

	fprintf (fp, "x%X", addr);
	fprintf (fp, "\n");
}

uint8_t openasmfiles (struct Files *f, char *file)
{
	uint8_t failed = 0;
	f->asm_ = fopen (file, "r");
	if (f->asm_ == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}
	replaceextension (file, ".sym");
	
	f->sym_ = fopen (file, "w");
	if (f->sym_ == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}
	replaceextension (file, ".bin");

	f->bin_ = fopen (file, "w");
	if (f->bin_ == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}
	replaceextension (file, ".hex");

	f->hex_ = fopen (file, "w");
	if (f->hex_ == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}
	replaceextension (file, ".obj");

	f->obj_ = fopen (file, "w");
	if (f->obj_ == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}
	replaceextension (file, ".lst");

	f->lst_ = fopen (file, "w");
	if (f->lst_ == NULL) {
		notify ("Unable to open %s!\n", file);
		failed++;
	}

	if (ENABLE_LOGGING) {
		replaceextension (file, ".log");
		f->log_ = fopen (file, "w");
		if (f->log_ == NULL) {
			notify ("Unable to open %s!\n", file);
			failed++;
		}
	}
	replaceextension (file, ".sym");

	return failed;
}

void closeasmfiles (struct Files *f)
{
	if (f->asm_ != NULL) fclose (f->asm_);
	if (f->sym_ != NULL) fclose (f->sym_);
	if (f->bin_ != NULL) fclose (f->bin_);
	if (f->hex_ != NULL) fclose (f->hex_);
	if (f->obj_ != NULL) fclose (f->obj_);
	if (f->lst_ != NULL) fclose (f->lst_);
	if (f->log_ != NULL) fclose (f->log_);
}