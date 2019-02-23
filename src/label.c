#define USES_LABEL
#define USES_OPERAND
#define USES_TOKENIZE
#include "laser.h"

Label* addlabel (Label *l, uint32_t ln, char *label, uint16_t addr)
{
	l[0].count++;
	uint32_t labelnum = l[0].count;

	if (labelnum >= DEFAULT_LABEL_NUM)
		l = (Label*) realloc (l, (labelnum + 1) * sizeof (Label));

	l[labelnum].count = 0;
	l[labelnum].ln = ln;
	l[labelnum].label = puttoken (label);
	l[labelnum].address = addr;

	return l;
}

uint8_t isvalidlabel (char *token)
{
	if (!(isalpha (token[0]) || token[0] == '_')) return 0;

	for (uint16_t i = 0; token[i]; i++) {
		if (!(isalnum (token[i]) || token[i] == '_')) return 0;
	}

	if (isoperand (token) + isregister (token) + 2) return 0;

	return 1;
}

uint16_t labeladdr (Label *l, char *label)
{
	uint32_t labelnum = l[0].count;
	uint16_t labeladdr = 0;
	for (int i = 1; i < labelnum; i++) {
		if (strcmp (label, l[i].label) == 0) {
			labeladdr = l[i].address;
			break;
		}
	}
	return labeladdr;
}

Label* freelabelarr (Label *l)
{
	if (l == NULL) return l;

	uint32_t labelnum = l[0].count;

	for (uint32_t i = 1; i <= labelnum; i++) {
		if (l[i].label != NULL) {
			free (l[i].label);
			l[i].label = NULL;
		}
	}

	free (l);
	l = NULL;
	return l;
}