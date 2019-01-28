#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include "config.h"

#ifndef PARSE_H
#define PARSE_H

struct Instruction {
	char *fname;
	int ln;
	char *line_buf;
	char *opcode;
	uint16_t instr;
	int addr;
	int type;
	bool src;
};

enum instype {
	N_OP = 0,
	P_OP = 1,
	OP = 2
};

struct Symbol {
	int ln;
	char label[MAX_WORD_SIZE+2];
	int addr;
	int count;
};

struct Alias {
	int ln;
	char word[MAX_WORD_SIZE+2];
	char replace[MAX_WORD_SIZE+2];
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
	FILE *log;
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

void parseFile (FILE *fp, char *fname, int q);

int labelAddress (struct Symbol *sym, char *label);

int existLabel (struct Symbol *sym, char *label);

int unusedSymbol (struct Symbol *sym, struct Alert *a, FILE *fp_log);

void aliasWord (struct Alias *al, char c[MAX_WORD_SIZE + 2]);

int unusedAlias (struct Alias *al, struct Alert *a, FILE *fp_log);

int existAlias (char *op, struct Alias *al);

int operandImmediate (char *op, struct Instruction *ins, int off_b, struct Alert *a, FILE *fp_log);

int operandRegister (char *op, int loc, struct Instruction *ins, struct Alert *a, FILE *fp_log);

int operandOffset (char *op, struct Instruction *ins, struct Symbol *sym, int off_b, struct Alert *a, FILE *fp_log);

int operandString (char *str, struct Instruction *ins, struct File file, struct Alert *a);

void lineToWords (char *line_buf, char word_buf[][MAX_WORD_SIZE + 2]);

int countWords (int offset, char word_buf[][MAX_WORD_SIZE + 2]);

void fprintAsm (struct File file, struct Instruction ins);

void printAlertSummary (struct Alert alert);

void fprintAlertSummary (struct Alert alert, FILE *fp_log);

void trapShortcut (char *op, char *trapvect);

int warnOpOvf (struct Instruction ins, struct Alert *a, FILE *fp_log);

int errNoOp (struct Instruction ins, struct Alert *a, FILE *fp_log, char *c);

int errOpDef (struct Instruction ins, struct Alert *a, FILE *fp_log);

int errInvalidOp (struct Instruction ins, struct Alert *a, char *op, FILE *fp_log);

#endif