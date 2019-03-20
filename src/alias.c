#define USES_ALIAS
#define USES_OPERAND
#include "laser.h"

void addalias (Alias *a, uint32_t ln, Token *word, Token *reg)
{
	a[0].count++;
	uint32_t aliasnum = a[0].count;

	if (aliasnum >= DEFAULT_ALIAS_NUM)
		a = (Alias*) realloc (a, (aliasnum + 1) * sizeof (Alias));

	a[aliasnum].count = 0;
	a[aliasnum].ln = ln;
	a[aliasnum].word = (Token*) malloc (sizeof (Token));
	copytoken (a[aliasnum].word, word);
	a[aliasnum].reg = (Token*) malloc (sizeof (Token));
	copytoken (a[aliasnum].reg, reg);
}

uint32_t findalias (Alias *a, Token *word)
{
	uint32_t aliasnum = a[0].count;
	for (uint32_t i = 1; i <= aliasnum; i++) {
		if (strcmp (word->str, a[i].word->str) == 0) {
			return i;
		}
	}
	return 0;
}

Alias* initaliasarr (void)
{
	Alias *a = (Alias*) malloc (DEFAULT_ALIAS_NUM * sizeof (struct Alias));
	a[0].count = 0;																// tracks aliasnum
	a[0].ln = DEFAULT_ALIAS_NUM;												// tracks capacity
	a[0].word = NULL;
	a[0].reg = NULL;
	return a;
}

void freealiasarr (Alias *a)
{
	if (a == NULL) return;
	uint32_t aliasnum = a[0].count;

	for (uint32_t i = 1; i <= aliasnum; i++) {
		if (a[i].word != NULL) freetoken (a[i].word);
		if (a[i].reg != NULL) freetoken (a[i].reg);
	}

	free (a);
}