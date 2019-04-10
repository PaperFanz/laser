#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
    if you are getting read errors, increase these numbers
    (you really shouldn't have to)
*/
#define MAX_WORD_NUM 6		// max # of words per line
#define MAX_WORD_SIZE 1025	// max # of chars for a label, filename, or .STRINGZ
#define MAX_LEN 6150		// MAX_WORD_NUM * MAX_WORD_SIZE

/*
    Token: the basic building plock of my text parser in C contains a pointer to
            a null-terminated character string on the heap along with the length
            of the string not including the null terminator
*/
typedef struct Token {
    uint16_t len;
    char *str;
} Token;

/*
    TokenBuffer: a container for Tokens, used to hold the Tokens for an entire
            line, along with the number of tokens in the container
*/
typedef struct TokenBuffer {
    uint16_t toknum;
    Token **token;
} TokenBuffer;

void copytoken (Token *dest, Token *src);

TokenBuffer* tokenize (char *line);

void freetoken (Token *t);

void freetokenarr (TokenBuffer *tokenbuffer);

typedef struct LineInfo {
    TokenBuffer *buf;
    uint32_t ln;
} lineinfo_t;

typedef struct TokenBufferArray {
    lineinfo_t **arr;
    uint32_t cap;
    uint32_t ind;
} tokbufarr_t;

#define DEFAULT_ARR_SIZE 200

tokbufarr_t* inittokenbufferarray ();

void abuttokenbufferarray (tokbufarr_t *arr, TokenBuffer *buf, uint32_t ln);

lineinfo_t* fromtokenbufferarray (tokbufarr_t *arr, uint16_t index);

void freetokenbufferarray (tokbufarr_t *arr);