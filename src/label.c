#define USES_LABEL
#define USES_OPERAND
#include "laser.h"

Label* addlabel (Label *l, uint32_t ln, Token *label, uint16_t addr)
{
	l[0].count++;
	uint32_t labelnum = l[0].count;

	if (labelnum >= DEFAULT_LABEL_NUM)
		l = (Label*) realloc (l, (labelnum + 1) * sizeof (Label));

	l[labelnum].count = 0;
	l[labelnum].ln = ln;
	l[labelnum].label = (Token*) malloc (sizeof (Token));
	copytoken (l[labelnum].label, label);
	l[labelnum].address = addr;

	return l;
}

uint8_t isvalidlabel (Token *tok)
{
	char *str = tok->str;
	if (!(isalpha (str[0]) || str[0] == '_')) return 0;

	for (uint16_t i = 0; str[i]; i++) {
		if (!(isalnum (str[i]) || str[i] == '_')) return 0;
	}

	if (isoperand (tok) + isregister (tok) + 2) return 0;

	return 1;
}

uint16_t labeladdr (Label *l, Token *label)
{
	uint32_t labelnum = l[0].count;
	for (int i = 1; i <= labelnum; i++) {
		if (strcmp (label->str, l[i].label->str) == 0) {
			return l[i].address;
		}
	}
	return 0;
}

Label* initlabelarr (void)
{
	Label *l = (Label*) malloc (DEFAULT_LABEL_NUM * sizeof (struct Label));
	l[0].count = 0;																// tracks labelnum
	l[0].ln = DEFAULT_LABEL_NUM;												// tracks capacity
	l[0].label = NULL;
	l[0].address = 0;

	return l;
}

void freelabelarr (Label *l)
{
	if (l == NULL) return;

	uint32_t labelnum = l[0].count;

	for (uint32_t i = 1; i <= labelnum; i++) {
		freetoken (l[i].label);
	}

	free (l);
}