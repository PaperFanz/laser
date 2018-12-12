// include statements
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "math.h"

// macros
#define size_arr(arr) ((&arr)[1]-arr)

// version number
char version_num[]="0.0.1";

// help message
char help[]="laser usage:\n\t-v or --version to display version number\n\t-h or --help to display this message again\n\t-f or --file to specify file to assemble\n";

// function declarations
void parse_args(int argc, char *argv[], FILE *fp, bool fileOpened);

int twoCompToDec(int bin[], int size);

unsigned int decToTwoComp(int n);

void zext(unsigned int n, int bin[], int size);

char binToHex(int bin[], int bin_size, char hex[]);