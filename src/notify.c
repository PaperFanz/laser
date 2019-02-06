#define USES_NOTIFY
#define USES_FLAG
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