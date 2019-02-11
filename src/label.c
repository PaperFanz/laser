#define USES_LABEL
#include "laser.h"

struct Label* addlabel (struct Label *l, uint32_t ln, char *label, uint16_t addr)
{
	l[0].count++;
	uint32_t labelnum = l[0].count;

	if (labelnum >= DEFAULT_LABEL_NUM)
		l = realloc (l, (labelnum + 1) * sizeof (struct Label));

	l[labelnum].count = 0;
	l[labelnum].ln = ln;
	l[labelnum].label = label;
	l[labelnum].address = addr;

	return l;
}

uint16_t labeladdr (struct Label *l, char *label)
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