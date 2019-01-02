#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

// macros
#define MAX_WORD_SIZE 20	// max # of chars for a label or a filename

// function declarations
int isHexChar(char c);

void notArr(int bin[], int size);

void addArr(int bin1[], int s1, int bin2[], int s2, int bin3[], int s3);

// hex conversion array
const char hex_chars[16]={'0','1','2','3',
						  '4','5','6','7',
						  '8','9','A','B',
						  'C','D','E','F'};

// keyword array used to parse text file
const char *keyword[][16]={
	{"BR", "br", "BRnzp", "brnzp", "BRnz", "brnz", "BRn", "brn",
	"BRnp", "brnp", "BRzp", "brzp", "BRz", "brz", "BRp", "brp"},
	{"ADD", "add"},
	{"LD", "ld"},
	{"ST", "st"},
	{"JSR", "jsr", "JSRR", "jsrr"},
	{"AND", "and"},
	{"LDR", "ldr"},
	{"STR", "str"},
	{"RTI", "rti"},
	{"NOT", "not"},
	{"LDI", "ldi"},
	{"STI", "sti"},
	{"JMP", "jmp", "RET", "ret"},
	{"INVALID_OPCODE_EXCEPTION"},
	{"LEA", "lea"},
	{"TRAP", "trap", "GETC", "getc", "OUT", "out", "PUTS", "puts",
	"IN", "in", "PUTSP", "putsp", "HALT", "halt"}
};

const char *regs[][2]={
	{"R0", "r0"},
	{"R1", "r1"},
	{"R2", "r2"},
	{"R3", "r3"},
	{"R4", "r4"},
	{"R5", "r5"},
	{"R6", "r6"},
	{"R7", "r7"}
};

const char *pseudoop[][2]={
	".ORIG", ".orig",
	".END", ".end",
	".STRINGZ", ".stringz",
	".BLKW", ".blkw",
	".FILL", ".fill"
};

// 16 bit one in binary (used by adder)
int one_16b[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};