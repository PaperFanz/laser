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

void inittokenbufferarray ();

void abuttokenbufferarray (TokenBuffer *buf, uint32_t ln);

TokenBuffer* fromtokenbufferarray (uint16_t index);

uint32_t linetokenbufferarray (uint16_t index);

uint16_t tokenbufferarrayend (void);

void freetokenbufferarray ();