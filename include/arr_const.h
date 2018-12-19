#include <string.h>

// hex conversion array
const char hex_chars[16]={'0','1','2','3',
						  '4','5','6','7',
						  '8','9','A','B',
						  'C','D','E','F'};

// keyword array used to parse text file
const char *keyword[]={
	"ADD", "add",
	"AND", "and",
	"NOT", "not",
	"JMP", "jmp", "RET", "ret",
	"JSR", "jsr", "JSRR", "jsrr",
	"BR", "br", "BRnzp", "brnzp", "BRn", "brn", "BRnz",
	"brnz", "BRz", "brz", "BRzp", "brzp", "BRp", "brp",
	"LD", "ld",
	"LDR", "ldr",
	"LDI", "ldi",
	"ST", "st",
	"STR", "str",
	"STI", "sti",
	"LEA", "lea",
	"RTI", "rti",
	"TRAP", "trap",
	"HALT", "halt",
	"GETC", "getc",
	"OUT", "out",
	"PUTS", "puts"
	"IN", "in",
	"PUTSP", "putsp"
};



// 16 bit one in binary (used by adder)
int one_16b[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1};