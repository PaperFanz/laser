#define USES_MACRO
#define USES_OPERAND
#include "laser.h"

Macro* addmacro (Macro *m, uint32_t ln, Token *macro, Token *replace)
{
	m[0].count++;
	uint32_t macronum = m[0].count;

	if (macronum >= DEFAULT_MACRO_NUM)
		m = (Macro*) realloc (m, (macronum + 1) * sizeof (Macro));

	m[macronum].count = 0;
	m[macronum].ln = ln;
	m[macronum].macro = (Token*) malloc (sizeof (Token));
	copytoken (m[macronum].macro, macro);
	m[macronum].replace = (Token*) malloc (sizeof (Token));
	copytoken (m[macronum].replace, replace);

	return m;
}

Token* findmacro (Macro *m, Token *macro)
{
	uint32_t macronum = m[0].count;
	for (int i = 1; i <= macronum; i++) {
		if (strcmp (macro->str, m[i].macro->str) == 0) {
			Token *replace;
			copytoken (replace, m[i].replace);
			return replace;
		}
	}
	return NULL;
}

Macro* initmacroarr (void)
{
	Macro *m = (Macro*) malloc (DEFAULT_MACRO_NUM * sizeof (struct Macro));
	m[0].count = 0;																// tracks macronum
	m[0].ln = DEFAULT_MACRO_NUM;												// tracks capacity
	m[0].macro = NULL;
	m[0].replace = NULL;
	return m;
}

void freemacroarr (Macro *m)
{
	if (m == NULL) return;

	uint32_t macronum = m[0].count;

	for (uint32_t i = 1; i <= macronum; i++) {
		freetoken (m[i].macro);
		freetoken (m[i].replace);
	}

	free (m);
}