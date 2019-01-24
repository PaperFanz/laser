#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#ifndef CALC_H
#define CALC_H
// function declarations
int offsetMask (int offset_bits);

int checkExt (const char *filename, const char *ext);

char *replaceExt (char *filename, const char *ext);

int hexOffset (char *offset);

int binOffset (char *offset);

int decOffset (char *offset);

int offset (int off_type, char *op);

// print functions
void printIntArr (int num[], int size);

void fprintIntArr (FILE *fp, int num[], int size);

void printCharArr (char hex[], int size);

void fprintCharArr (FILE *fp, char hex[], int size);

void putSymbol (FILE *fp, char symbol[], char addr[]);

#endif