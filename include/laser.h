// include statements
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "math.h"

// macros
#define size_arr(arr) ((&arr)[1]-arr)

// function declarations
void parse_args(int argc, char *argv[], FILE *fp, bool fileOpened);

int twoCompToDec(int bin[], int size);

unsigned int decToTwoComp(int n);

void zext(unsigned int n, int bin[], int size);

char binToHex(int bin[], int bin_size, char hex[]);