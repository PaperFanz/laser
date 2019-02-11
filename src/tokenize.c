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

void putstr (char *line, char *token, int32_t *lineptr)
{
	int32_t i = *lineptr, j = 0;

	while (line[i] != '\0') {
		if (line[i] == '\"') {
			break;
		} else if (line[i] == '\\') {
			i++;
			token[j] = escval(line[i]);
			j++;
			i++;
		} else {
			token[j] = line[i];
			j++;
			i++;
		}
	}

	*lineptr = i;
}

void putwrd (char *line, char *token, int32_t *lineptr)
{
	int32_t i = *lineptr, j = 0;

	while (line[i] != '\0') {						// end of file
		if (line[i] == ' ' || line[i] == ',' ||
			line[i] == '\t' || line[i] == '\n') {	// end of word
			break;
		} else {
			token[j] = line[i];						// fill in word
			j++;
			i++;
		}
	}

	*lineptr = i;									// modify line pointer
}

void tokenize (char *line, char token_buf[MAX_WORD_NUM][MAX_WORD_SIZE])
{
	int32_t word = 0;
	bool delim = 0, quote = 0;
	bool wasdelim = 1;

	for(int32_t i = 0; line[i] && line[i] != ';'; i++) {
		delim = (line[i] == ' ' || line[i] == '\t' || line[i] == ',');
		quote = (line[i] == '\"');
		
		if (quote) {
			int32_t k = 0;
			i++;
			putstr (line, token_buf[word], &i);
			word++;
		} else if (wasdelim && !delim) {
			int32_t k = 0;
			putwrd (line, token_buf[word], &i);
			word++;
			delim = 1;
		}

		wasdelim = delim;
	}
}