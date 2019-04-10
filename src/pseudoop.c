#define USES_PSEUDOOP
#define USES_OFFSET
#include "laser.h"

int8_t arrcmp (char *str, const char *arr[][2], uint8_t size);

int8_t ispseudoop (Token *token)
{
    const char *pops[][2] = {
        ".ALIAS", ".alias",		// 0
        ".MACRO", ".macro",		// 1
        ".ORIG", ".orig",		// 2
        ".END", ".end",			// 3
        ".STRINGZ", ".stringz",	// 4
        ".BLKW", ".blkw",		// 5
        ".FILL", ".fill",		// 6
        ".EXPORT", ".export",	// 7
        ".IMPORT", ".import"	// 8
    };
    return arrcmp (token->str, pops, 9);
}

uint16_t addrnum (uint8_t popcode, Token *token)
{
    if (popcode == STRINGZ && token != NULL) {
        return token->len + 1;
    } else if (popcode == BLKW && token != NULL) {
        uint8_t offt = offtype (token);
        int16_t off = offset (offt, token);
        if (offt > 0 && off > 0) return off;
        else return 0;
    } else if (popcode == FILL) {
        return 1;
    } else {
        return 0;
    }
}