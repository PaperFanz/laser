// include statements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// macros
#define MAX_WORD_SIZE 20	// max # of chars for a label or a filename
#define TABSIZE 4

// function declarations
int hexToDec (char hex[]);

void decToTwoComp (int n, int bin[], int size);