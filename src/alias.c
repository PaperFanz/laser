#define USES_ALIAS
#define USES_OPERAND
#define USES_TOKENIZE
#include "laser.h"

Alias* addalias (Alias *a, uint32_t ln, char *word, char *reg)
{
	a[0].count++;
	uint32_t aliasnum = a[0].count;

	if (aliasnum >= DEFAULT_ALIAS_NUM)
		a = (Alias*) realloc (a, (aliasnum + 1) * sizeof (Alias));

	a[aliasnum].count = 0;
	a[aliasnum].ln = ln;
	a[aliasnum].word = puttoken (word);
	a[aliasnum].reg = puttoken (reg);

	return a;
}

char* findalias (Alias *a, char *word)
{
	uint32_t aliasnum = a[0].count;
	char *ret = NULL;
	for (int i = 1; i < aliasnum; i++) {
		if (strcmp (word, a[i].word) == 0) {
			ret = a[i].reg;
			break;
		}
	}
	return ret;
}

Alias* freealiasarr (Alias *a)
{
	if (a == NULL) return a;

	uint32_t aliasnum = a[0].count;

	for (uint32_t i = 1; i <= aliasnum; i++) {
		if (a[i].word != NULL) {
			free (a[i].word);
			a[i].word = NULL;
		}
		if (a[i].reg != NULL) {
			free (a[i].reg);
			a[i].reg = NULL;
		}
	}

	free (a);
	a = NULL;
	return a;
}