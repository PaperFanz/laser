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

// fill and error-check a regiser
#define OPERAND_REGISTER(operand, loc) {\
	if (!fillRegister (isRegister(operand), bin, loc))\
		printf ("Error: (line %d) invalid operand for '%s': %s\n", ln, word_buf[i], operand);\
}

// fill and error-check a register or immediate
#define OPERAND_REGISTER_IMMEDIATE(operand, loc) {\
	if (!fillRegister (isRegister(operand), bin, loc)){\
		if(fillOffset (isValidOffset(operand), operand, offset_bits, ln, bin))\
			bin[10]=1;\
		else\
			printf("Error: (line %d) invalid operand for '%s': %s\n", ln, word_buf[i], operand);\
	}\
}

// check for extra operands
#define CHECK_OP_OVERFLOW(max){\
	if(word_buf[i + max + 1][0] != '\0')\
		printf("Warning: (line %d) '%s' only takes %d operands!\n\t%s", ln, word_buf[i], max, line_buf);\
}

// fill and error-check an offset
#define OPERAND_OFFSET(operand){\
	if(!fillOffset(isValidOffset(operand), operand, offset_bits, ln, bin)){\
		for(j=0; j<s_cnt; j++){if(strcmp(operand, symbol[j])==0){match=true; break;}}\
		if(match){\
			if(!fillDecOffset((dec_addr[j]-(addr+1)), offset_bits, ln, bin))\
				printf("Error: (line %d) %d cannot be expressed in %d bits!\n", ln, (dec_addr[j]-(addr+1)), offset_bits);\
		}\
		else{\
			printf("Error: (line %d) Undeclared label '%s'!\n", ln, operand);\
		}\
	}\
}

// function declarations
int isKeyword(char c[]);

int isPseuodoOp(char c[]);

int isRegister(char c[]);

int isLabel(char c[]);

int isValidOffset(char c[]);

int fillRegister(int r, int bin[], int n);

int fillOffset(int type, char c[], int bits, int ln, int put_bin[]);

int fillDecOffset(int off, int bits, int ln, int put_bin[]);

int addrToDec(char hex[]);

char* decToAddr(char hex[], int dec_num);

void putSymbol(FILE *fp, char symbol[], char addr[]);

void fprintIntArr(FILE *fp, int num[], int size);

void fprintCharArr(FILE *fp, char hex[], int size);

void decToTwoComp(int n, int bin[], int size);

void binToHex(int bin[], int bin_size, char hex[], int hex_size);