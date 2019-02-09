#define USES_FLAG
#define USES_NOTIFY
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

void warn (int8_t type, FILE *fp, const char *format, ...)
{
	va_list strs, strscpy;
	va_copy (strscpy, strs);

	va_start (strs, format);
	if (quiet != SILENT_V) {
		vprintf (format, strs);
	}
	va_end (strs);

	va_start (strscpy, format);
	if (ENABLE_LOGGING) {
		vfprintf (fp, format, strscpy);
	}
	va_end (strscpy);
}

void error (int8_t type, FILE *fp, const char *format, ...)
{
	va_list strs, strscpy;
	va_copy (strscpy, strs);

	va_start (strs, format);
	if (quiet != SILENT_V) {
		vprintf (format, strs);
	}
	va_end (strs);

	va_start (strscpy, format);
	if (ENABLE_LOGGING) {
		vfprintf (fp, format, strscpy);
	}
	va_end (strscpy);
}