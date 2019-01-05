// include statements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// macros
#define MAX_WORD_SIZE 40	// max # of chars for a label or a filename
#define TABSIZE 4

// function declarations
int hexToDec (char hex[]);

void decToTwoComp (int n, int bin[], int size);

void notArr(int bin[], int size);

void addArr(int bin1[], int s1, int bin2[], int s2, int bin3[], int s3);

// 16 bit one in binary (used by adder)
int addone[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};