#define USES_ALIAS
#include "laser.h"

struct Alias* addalias (struct Alias *a, uint32_t ln, char *word, char *replace)
{
	a[0].count++;
	uint32_t aliasnum = a[0].count;

	if (aliasnum >= DEFAULT_ALIAS_NUM)
		a = realloc (a, (aliasnum + 1) * sizeof (struct Alias));

	a[aliasnum].count = 0;
	a[aliasnum].ln = ln;
	a[aliasnum].word = word;
	a[aliasnum].replace = replace;

	return a;
}

uint32_t findalias (struct Alias *a, char *word)
{
	uint32_t aliasnum = a[0].count, aliasfound = 0;
	for (int i = 1; i < aliasnum; i++) {
		if (strcmp (word, a[i].word) == 0) {
			aliasfound = i;
			break;
		}
	}
	return aliasfound;
}