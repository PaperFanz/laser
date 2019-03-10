#include "laser.h"

const char escapeChars[] = {'\'', '\"', '\?', '\\', 'a', 'b', 'f', 'n', 'r', 't', 'v'};

const char escapeVals[] = {0x27, 0x22, 0x3F, 0x5C, 0x07, 0x08, 0x0C, 0x0A, 0x0D, 0x09, 0x0B};

char escval (char c)
{
	for (int8_t i = 0; i < 11; i++) {
		if (c == escapeChars[i]) {
			return escapeVals[i];
		}
	}
	return c;
}

void copytoken (Token *dest, Token *src)
{
	dest->len = src->len;
	dest->str = malloc ((dest->len + 1) * sizeof (char));
	memcpy (dest->str, src->str, (dest->len + 1) * sizeof (char));
}

Token* putstr (char *line, uint16_t *lineptr)
{
	uint16_t i = *lineptr, j = 0;
	char tmp[MAX_WORD_SIZE];

	while (line[i] != '\0') {
		if (line[i] == '\"') {
			break;
		} else if (line[i] == '\\') {
			i++;
			tmp[j] = escval(line[i]);
			j++;
			i++;
		} else {
			tmp[j] = line[i];
			j++;
			i++;
		}
	}

	Token *t = malloc (sizeof (Token));
	t->len = j + 1;
	t->str = malloc ((t->len + 1) * sizeof (char));
	for (uint16_t k = 0; k < j; k++) {
		t->str[k] = tmp[k];
	}
	t->str[j] = '\0';

	*lineptr = i;
	return t;
}

int8_t iseow (char c)
{
	int8_t eow = 0;
	const char eowchars[] = {0, 32, 44, 9, 10, 13};

	for (uint8_t i = 0; i < 6; i++) {
		if (c == eowchars[i]) eow++;
	}

	return eow;
}

Token* putwrd (char *line, uint16_t *lineptr)
{
	uint8_t i = *lineptr, j = 0;
	char tmp[MAX_WORD_SIZE];

	while (!iseow (line[i])) {					// end of word
		tmp[j] = line[i];						// fill in word
		j++;
		i++;
	}

	Token *t = malloc (sizeof (Token));
	t->len = j + 1;
	t->str = malloc ((t->len + 1) * sizeof (char));
	for (uint16_t k = 0; k < j; k++) {
		t->str[k] = tmp[k];
	}
	t->str[j] = '\0';

	*lineptr = i - 1;								// modify line pointer
	return t;
}

int8_t iseeol (char c)
{
	int8_t eol = 0;
	const char eolchars[] = {0, 10, 13, 59};

	for (uint8_t i = 0; i < 4; i++) {
		if (c == eolchars[i]) eol++;
	}

	return eol;
}

TokenBuffer* tokenize (char *line)
{
	TokenBuffer *buf = malloc (sizeof (TokenBuffer));
	buf->toknum = 0;
	buf->token = (Token**) malloc (sizeof (Token*));

	uint8_t newtok = 0;
	bool delim = 0, quote = 0;
	bool wasdelim = 1;

	for(uint16_t i = 0; !iseeol (line[i]); i++) {
		delim = (line[i] == ' ' || line[i] == '\t' || line[i] == ',');
		quote = (line[i] == '\"');
		
		if (quote) {
			i++;
			buf->token[buf->toknum] = putstr (line, &i);
			buf->toknum++;
			newtok = 1;
		} else if (wasdelim && !delim) {
			buf->token[buf->toknum] = putwrd (line, &i);
			buf->toknum++;
			delim = 1;
			newtok = 1;
		}

		if (newtok) {
			buf->token = (Token**) realloc (buf->token, (buf->toknum + 1) * sizeof (Token*));
			newtok = 0;
		}

		wasdelim = delim;
	}

	return buf;
}

void freetoken (Token *t)
{
	if (t) {
		if (t->str) free (t->str);
		t->str = NULL;
		free (t);
	}
}

void freetokenarr (TokenBuffer *tokenbuffer)
{
	if (tokenbuffer == NULL) return;
	if (tokenbuffer->token == NULL) return;

	for (uint8_t i = 0; i < tokenbuffer->toknum; i++) {
		if(tokenbuffer->token[i]->len){
			freetoken (tokenbuffer->token[i]);
		}
	}

	free (tokenbuffer->token);
	free (tokenbuffer);
}