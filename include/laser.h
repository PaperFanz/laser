// include statements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>

// macros
#define size(arr) ((&arr)[1]-arr)
#define MAX_WORD_NUM 6		// max # of words per line (5+1 ICOI)
#define MAX_WORD_SIZE 20	// max # of chars for a label or a filename
#define TABSIZE 4

// function declarations
int isKeyword(char c[]);

int isPseuodoOp(char c[]);

int hexToDec(char hex[]);

void decToHex(char hex[], int dec_num);

void parse_file(FILE *fp, char *fname);

void putSymbol(FILE *fp, char symbol[], char addr[]);

void notArr(int bin[], int size);

void printIntArr(int num[], int size);

void printCharArr(char hex[], int size);

int binToDec(int bin[], int size);

int twoCompToDec(int bin[], int size);

void decToTwoComp(int n, int bin[], int size);

void zext(unsigned int n, int bin[], int size);

void binToHex(int bin[], int bin_size, char hex[], int hex_size);

void addArr(int bin1[], int s1, int bin2[], int s2, int bin3[], int s3);