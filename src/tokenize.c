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

char* puttoken (char *token)
{
	uint16_t toklen = token[-1];
	char *ret = (char*) malloc (toklen * sizeof (char));
	for (uint16_t i = 0; i < toklen; i++) {
		ret[i] = token[i];
	}
	return ret;
}

char* putstr (char *line, uint16_t *lineptr)
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

	char *token = (char*) malloc ((j + 3) * sizeof (char));
	uint16_t *len = (uint16_t*) token;
	token += 2;
	*len = j + 1;
	for (uint16_t k = 0; k < j; k++) {
		token[k] = tmp[k];
	}
	token[j] = '\0';

	*lineptr = i;
	return token;
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

char* putwrd (char *line, uint16_t *lineptr)
{
	uint8_t i = *lineptr, j = 0;
	char tmp[MAX_WORD_SIZE];

	while (!iseow (line[i])) {					// end of word
		tmp[j] = line[i];						// fill in word
		j++;
		i++;
	}

	char *token = (char*) malloc ((j + 3) * sizeof (char));
	uint16_t *len = (uint16_t*) token;
	token += 2;
	*len = j + 1;
	for (uint16_t k = 0; k < j; k++) {
		token[k] = tmp[k];
	}
	token[j] = '\0';

	*lineptr = i - 1;								// modify line pointer
	return token;
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

char** tokenize (char *line)
{
	char *tmp[MAX_WORD_NUM];
	char **token = calloc (MAX_WORD_NUM + 1, sizeof (char*));
	token++;

	uint8_t word = 0;
	bool delim = 0, quote = 0;
	bool wasdelim = 1;

	for(uint16_t i = 0; !iseeol (line[i]); i++) {
		delim = (line[i] == ' ' || line[i] == '\t' || line[i] == ',');
		quote = (line[i] == '\"');
		
		if (quote) {
			i++;
			token[word] = putstr (line, &i);
			word++;
		} else if (wasdelim && !delim) {
			token[word] = putwrd (line, &i);
			word++;
			delim = 1;
		}

		wasdelim = delim;
	}
	uint64_t *wordnum = (uint64_t*) (token - 1);
	*wordnum = word;

	return token;
}

char** free_token (char **token)
{
	if (token == NULL) return token;

	for (uint8_t i = 0; i < MAX_WORD_NUM; i++) {
		if(token[i] != NULL){
			token[i] -= 2;
			free (token[i]);
			token[i] == NULL;
		}
	}
	
	token--;
	free (token);
	token == NULL;
}