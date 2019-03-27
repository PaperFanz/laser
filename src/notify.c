#define USES_FLAG
#define USES_FILE
#define USES_NOTIFY
#define USES_OPERAND
#include "laser.h"

static int8_t NO_WARNINGS = 0;
static int8_t NO_ERRORS = 0;

void setVerbosity (int8_t q)
{
	if (q == noErrs) {
		NO_ERRORS = 1;
		NO_WARNINGS = 1;
	} else if (q == noWarn) {
		NO_ERRORS = 0;
		NO_WARNINGS = 1;
	} else {
		NO_ERRORS = 0;
		NO_WARNINGS = 0;
	}
}

void notify (const char *format, ...)
{
	va_list strs;

	va_start (strs, format);
	if (!NO_ERRORS) {
		vprintf (format, strs);
		printf("\n");
	}
	va_end (strs);
}

static uint32_t ALERT_WARN;
static uint32_t ALERT_ERRS;

void resetalerts (void)
{
	ALERT_WARN = 0;
	ALERT_ERRS = 0;
}

uint32_t getwarnings (void)
{
	return ALERT_WARN;
}

uint32_t geterrors (void)
{
	return ALERT_ERRS;
}

void warning (uint32_t ln, const char *format, ...)
{
	va_list strs, strscpy;
	va_copy (strscpy, strs);

	ALERT_WARN++;

	if (!NO_WARNINGS) {
		va_start (strs, format);
		printf ("\033[01;33mWarning (line %d):\033[0m ", ln);
		vprintf (format, strs);
		printf("\n");
		va_end (strs);
	}

	if (islogging ()) {
		va_start (strscpy, format);
		FILE *fp = getlog ();
		fprintf (fp, "\033[01;33mWarning (line %d):\033[0m ", ln);
		vfprintf (fp, format, strscpy);
		fprintf(fp, "\n");
		va_end (strscpy);
	}
}

void error (uint32_t ln, const char *format, ...)
{
	va_list strs, strscpy;
	va_copy (strscpy, strs);

	ALERT_ERRS++;

	if (!NO_ERRORS) {
		va_start (strs, format);
		printf ("\033[1;31mError (line %d):\033[0m ", ln);
		vprintf (format, strs);
		printf("\n");
		va_end (strs);
	}

	if (islogging ()) {
		va_start (strscpy, format);
		FILE *fp = getlog ();
		fprintf (fp, "\033[1;31mError (line %d):\033[0m ", ln);
		vfprintf (fp, format, strscpy);
		fprintf(fp, "\n");
		va_end (strscpy);
	}
}