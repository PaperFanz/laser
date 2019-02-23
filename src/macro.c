#define USES_MACRO
#define USES_OPERAND
#define USES_TOKENIZE
#include "laser.h"

Macro* addmacro (Macro *m, uint32_t ln, char *macro,
						char *replace)
{
	m[0].count++;
	uint32_t macronum = m[0].count;

	if (macronum >= DEFAULT_MACRO_NUM)
		m = (Macro*) realloc (m, (macronum + 1) * sizeof (Macro));

	m[macronum].count = 0;
	m[macronum].ln = ln;
	m[macronum].macro = puttoken (macro);
	m[macronum].replace = puttoken (replace);

	return m;
}

char* findmacro (Macro *m, char *macro)
{
	uint32_t macronum = m[0].count;
	char *ret = NULL;
	for (int i = 1; i < macronum; i++) {
		if (strcmp (macro, m[i].macro) == 0) {
			ret = m[i].replace;
			break;
		}
	}
	return ret;
}

Macro* freemacroarr (Macro *m)
{
	if (m == NULL) return m;

	uint32_t macronum = m[0].count;

	for (uint32_t i = 1; i <= macronum; i++) {
		if (m[i].macro != NULL) {
			free (m[i].macro);
			m[i].macro = NULL;
		}
		if (m[i].replace != NULL) {
			free (m[i].replace);
			m[i].replace = NULL;
		}
	}

	free (m);
	m = NULL;
	return m;
}