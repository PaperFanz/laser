// include statements
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "config.h"

// macros
#define size(arr) ((&arr)[1]-arr)

// types
struct Symbol {
	char label[MAX_WORD_SIZE+2];
	int addr;
};

struct Alert {
	int warn;
	int err;
	int exception;
};

struct File {
	FILE *sym;
	FILE *bin;
	FILE *hex;
	FILE *lst;
	FILE *obj;
};

// function declarations
void lineToWords (char *line_buf, char word_buf[][MAX_WORD_SIZE + 2]);

int countWords (int offset, char word_buf[][MAX_WORD_SIZE + 2]);

void printAlertSummary (struct Alert alert);

int isOrig (char word_buf[][MAX_WORD_SIZE + 2]);

int isEnd (char word_buf[][MAX_WORD_SIZE + 2]);

char *replaceExt (char *filename, const char *ext);

void fprintAsm (struct File file, int *bin, int addr, int ln, char *line_buf, bool op, bool src);

unsigned char byteValue (char hex[2]);

int isKeyword(char c[]);

int isPseuodoOp(char c[]);

int isRegister(char c[]);

int isLabel(char *c);

int isValidOffset(char c[]);

int isTrap(char c[]);

int isBranch(char c[]);

int isQuote(char c);

int escapeValue (char c);

int fillRegister(int r, int bin[], int n);

int offset (int type, char c[], int bits);

int fillDecOffset(int off, int bits, int ln, int put_bin[]);

int addrToDec(char hex[]);

char* decToAddr(char hex[], int dec_num);

int labelAddress (struct Symbol *symbols, int s_cnt, char *label);

void putSymbol(FILE *fp, char symbol[], char addr[]);

void addSymbol (struct Symbol *symbols, int s_cnt, char *c, int addr, struct File file);

void fprintIntArr(FILE *fp, int num[], int size);

void fprintCharArr(FILE *fp, char hex[], int size);

void decToTwoComp(int n, int bin[], int size);

void binToHex(int bin[], int bin_size, char hex[], int hex_size);

void op_reg_imm(char *keyword, char *op, int *bin, int loc, int offset_bits, int ln, struct Alert *alert, char *fname);

void op_register(char *keyword, char *op, int loc, int *bin, int ln, struct Alert *alert, char *fname);

void op_offset(char *keyword, char *op, int offset_bits, int ln, int *bin, int s_cnt, struct Symbol *symbols, int addr, struct Alert *alert, char *fname);