#define USES_FLAG
#define USES_NOTIFY
#define USES_OPERAND
#include "laser.h"

void notify (const char *format, ...)
{
	va_list strs;

	va_start (strs, format);
	if (quiet != SILENT_V) {
		vprintf (format, strs);
	}
	va_end (strs);
}

void error (int8_t type, FILE *fp, struct Instruction ins, 
			const char *format, ...)
{
	va_list strs, strscpy;
	va_copy (strscpy, strs);
	char *typestr = "\033[01;33mWarning";
	if (type) typestr = "\033[1;31mError";

	va_start (strs, format);
	if (quiet != SILENT_V) {
		printf ("%s (line %d):\033[0m ", typestr, ins.ln);
		vprintf (format, strs);
		printf("\n");
	}
	va_end (strs);

	va_start (strscpy, format);
	if (ENABLE_LOGGING) {
		fprintf (fp, "%s (line %d):\033[0m ", typestr, ins.ln);
		vfprintf (fp, format, strscpy);
		fprintf(fp, "\n");
	}
	va_end (strscpy);
}