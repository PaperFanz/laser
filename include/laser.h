// include statements
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"
#include "math.h"

// macros
#define size_arr(arr) ((&arr)[1]-arr)

// function declarations
int binToDec(int bin[], int size);

int twoCompToDec(int bin[], int size);

void decToTwoComp(int n, int bin[], int size);

void zext(unsigned int n, int bin[], int size);

char binToHex(int bin[], int bin_size, char hex[]);