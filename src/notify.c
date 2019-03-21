#define USES_FLAG
#define USES_NOTIFY
#define USES_OPERAND
#include "laser.h"


static int8_t quiet;

void setVerbosity (int8_t q)
{
	quiet = q;
}

void notify (const char *format, ...)
{
	va_list strs;

	va_start (strs, format);
	if (quiet != ERRS_ONLY) {
		vprintf (format, strs);
		printf("\n");
	}
	va_end (strs);
}

void error (int8_t type, FILE *fp, uint32_t ln, const char *format, ...)
{
	va_list strs, strscpy;
	va_copy (strscpy, strs);
	char *typestr;
	if (type == WARN) typestr = "\033[01;33mWarning";
	else typestr = "\033[1;31mError";

	va_start (strs, format);
	if (type >= quiet) {
		printf ("%s (line %d):\033[0m ", typestr, ln);
		vprintf (format, strs);
		printf("\n");
	}
	va_end (strs);

	va_start (strscpy, format);
	if (ENABLE_LOGGING) {
		fprintf (fp, "%s (line %d):\033[0m ", typestr, ln);
		vfprintf (fp, format, strscpy);
		fprintf(fp, "\n");
	}
	va_end (strscpy);
}