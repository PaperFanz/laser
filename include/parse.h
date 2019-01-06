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

enum ops {
	NO_OP = -1,
	ORIG = 0,
	END = 1,
	STRINGZ = 2,
	BLKW = 3,
	FILL = 4,
	ALIAS = 5,
	BR = 0,
	ADD = 1,
	LD = 2,
	ST = 3,
	JSR = 4,
	AND = 5,
	LDR = 6,
	STR = 7,
	RTI = 8,
	NOT = 9,
	LDI = 10,
	STI = 11,
	JMP = 12,
	INVALID_OPCODE_EXCEPTION = 13,
	LEA = 14,
	TRAP = 15
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