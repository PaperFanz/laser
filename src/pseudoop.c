#define USES_OFFSET
#include "laser.h"

int8_t arrcmp (char *str, const char *arr[][2], uint8_t size);

int8_t ispseudoop (char *token)
{
	const char *pops[][2] = {
		".ALIAS", ".alias",		// 0
		".MACRO", ".macro",		// 1
		".ORIG", ".orig",		// 2
		".END", ".end",			// 3
		".STRINGZ", ".stringz",	// 4
		".BLKW", ".blkw",		// 5
		".FILL", ".fill"		// 6
	};
	return arrcmp (token, pops, 7);
}

enum popcodes {ALIAS, MACRO, ORIG, END, STRINGZ, BLKW, FILL};

uint8_t poperandnum (uint8_t popcode)
{
	switch (popcode) {
	case ALIAS: return 2;
	case MACRO: return 2;
	case ORIG: return 1;
	case END: return 0;
	case STRINGZ: return 1;
	case BLKW: return 1;
	case FILL: return 1;
	default: return 0;
	}
}

uint16_t addrnum (uint8_t popcode, char *token)
{
	if (popcode == STRINGZ && token != NULL) {
		return strlen (token) + 1;
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

int16_t fillval ()
{

}

uint16_t fillstring ()
{

}

uint16_t blkwval ()
{
	
}