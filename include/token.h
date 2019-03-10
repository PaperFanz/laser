#include <stdint.h>

typedef struct Token {
	uint16_t len;
	char *str;
} Token;

typedef struct TokenBuffer {
	uint16_t toknum;
	Token **token;
} TokenBuffer;

void copytoken (Token *dest, Token *src);

TokenBuffer* tokenize (char *line);

void freetoken (Token *t);

void freetokenarr (TokenBuffer *tokenbuffer);