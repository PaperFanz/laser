/*
*   Laser- a command line utility to assemble LC3 assembly code
*
*   Copyright Notice:
*
*       Copyright 2018, 2019 Zhiyuan Fan
*
*   License Notice:
*
*       Laser is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation, either version 3 of the License, or
*       (at your option) any later version.
*
*       Laser is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License
*       along with Laser.  If not, see <https://www.gnu.org/licenses/>.
*/

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