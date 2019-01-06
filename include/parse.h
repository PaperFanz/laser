#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ctype.h>
#include "config.h"

#ifndef PARSE_H
#define PARSE_H

struct Symbol {
	char label[MAX_WORD_SIZE+2];
	int addr;
	int count;
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

struct Alias {
	char word[MAX_WORD_SIZE+2];
	char replace[MAX_WORD_SIZE+2];
	int count;
};

void parseFile (FILE *fp, char *fname);

int labelAddress (struct Symbol *symbols, int s_cnt, char *label);

void aliasWord (struct Alias *aliases, int a_cnt, char c[MAX_WORD_SIZE + 2]);

void op_reg_imm (char *keyword, char *op, int *bin, int loc,
				int offset_bits, int ln, struct Alert *alert, char *fname);

void op_register (char *keyword, char *op, int loc, int *bin, int ln, struct Alert *alert, char *fname);

void op_offset (char *keyword, char *op, int offset_bits, int ln,
				int *bin, int s_cnt, struct Symbol *symbols, int addr, struct Alert *alert, char *fname);

void lineToWords (char *line_buf, char word_buf[][MAX_WORD_SIZE + 2]);

int countWords (int offset, char word_buf[][MAX_WORD_SIZE + 2]);

void fprintAsm (struct File file, int *bin, int addr, int ln, char *line_buf, bool op, bool src);

void printAlertSummary (struct Alert alert);

void branchCondition (char *c, int *bin);

void trapShortcut (char *op, char *trapvect);

#endif