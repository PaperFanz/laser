// include statements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

// macros
#define size(arr) ((&arr)[1]-arr)
#define max_len 40					// max # of chars for a line of assembly

// function declarations
int isKeyword(char c[]);

void parse_file(FILE *fp);

void notArr(int bin[], int size);

void printIntArr(int num[], int size);

void printCharArr(char hex[], int size);

int binToDec(int bin[], int size);

int twoCompToDec(int bin[], int size);

void decToTwoComp(int n, int bin[], int size);

void zext(unsigned int n, int bin[], int size);

void binToHex(int bin[], int bin_size, char hex[], int hex_size);

void addArr(int bin1[], int s1, int bin2[], int s2, int bin3[], int s3);