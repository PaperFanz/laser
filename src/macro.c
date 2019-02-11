#define USES_MACRO
#include "laser.h"

struct Macro* addmacro (struct Macro *m, uint32_t ln, char *macro,
						char *replace)
{
	m[0].count++;
	uint32_t macronum = m[0].count;

	if (macronum >= DEFAULT_MACRO_NUM)
		m = realloc (m, (macronum + 1) * sizeof (struct Macro));

	m[macronum].count = 0;
	m[macronum].ln = ln;
	m[macronum].macro = macro;
	m[macronum].replace = replace;

	return m;
}

uint32_t findmacro (struct Macro *m, char *macro)
{
	uint32_t macronum = m[0].count, macrofound = 0;
	for (int i = 1; i < macronum; i++) {
		if (strcmp (macro, m[i].macro) == 0) {
			macrofound = i;
			break;
		}
	}
	return macrofound;
}