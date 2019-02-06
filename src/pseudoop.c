#include "laser.h"

const char *pseudoops[][2] = {
	".ALIAS", ".alias",		// 0
	".MACRO", ".macro",		// 1
	".ORIG", ".orig",		// 2
	".END", ".end",			// 3
	".STRINGZ", ".stringz",	// 4
	".BLKW", ".blkw",		// 5
	".FILL", ".fill"		// 6
};

int8_t ispseudoop (char *token)
{
	for (int8_t i = 0; i < 7; i++ ) {
		if (strcmp (token, pseudoops[i][0]) == 0 ||
			strcmp (token, pseudoops[i][1]) == 0) return i;
	} 
	return -1;
}

int8_t fillval ()
{

}

int8_t fillstring ()
{

}

int8_t blkwval ()
{
	
}