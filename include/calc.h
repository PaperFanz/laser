#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "config.h"

#ifndef CALC_H
#define CALC_H
// function declarations
int intPow (int a, int b);

int offsetMask (int offset_bits);

int checkExt (const char *filename, const char *ext);

char *replaceExt (char *filename, const char *ext);

// .bin generator functions
int fillRegister (int r, int bin[], int n);

int fillDecOffset (int off, int bits, int ln, int put_bin[]);

// read a fixed offset or immediate value and express it in n bits
int offset (int type, char c[], int bits);

// print functions
void printIntArr (int num[], int size);

void fprintIntArr (FILE *fp, int num[], int size);

void printCharArr (char hex[], int size);

void fprintCharArr (FILE *fp, char hex[], int size);

void putSymbol (FILE *fp, char symbol[], char addr[]);

#endif