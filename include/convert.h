#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "config.h"

#ifndef CONVERT_H
#define CONVERT_H
//==============================================================================
// Type Functions
//==============================================================================

int isKeyword(char c[]);

int isPseuodoOp(char c[]);

int isRegister(char c[]);

// extends isxdigit functionality by returning decimal value (0-16)
int isHexChar(char c);

int isValidOffset(char *c);

int isLabel(char *c);

int isTrap(char c[]);

int isBranch(char *c);

int isOrig (char word_buf[][MAX_WORD_SIZE+2]);

int isEnd (char word_buf[][MAX_WORD_SIZE+2]);

int isAlias (char word_buf[][MAX_WORD_SIZE+2]);

int escapeValue (char c);

//==============================================================================
// Convert Functions
//==============================================================================

// binary conversions
int binToDec(int bin[], int size);

void zext(int n, int bin[], int size);

// binary operations
void notArr(int bin[], int size);

// bin3 is result array, bin1 and bin2 are operands
void addArr(int bin1[], int s1, int bin2[], int s2, int bin3[], int s3);

// two's complement conversions
void decToTwoComp(int n, int bin[], int size);

int twoCompToDec(int bin[], int size);

// hex conversions
// note: hex_size should always be at 1/4 of bin_size
void binToHex(int bin[], int bin_size, char hex[], int hex_size);

int hexToDec(char hex[]);

unsigned char byteValue (char hex[2]);

// note: only handles positive numbers, intended for use with addresses
// DO NOT use to calculate offsets
int addrToDec(char hex[]);

// note: only handles positive numbers, intended for use with addresses
// DO NOT use to calculate offsets
char* decToAddr(char *hex, int dec_num);

#endif