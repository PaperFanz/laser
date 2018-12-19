#include <string.h>

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
	{"\0"},
	{"LEA", "lea"},
	{"TRAP", "trap", "GETC", "getc", "OUT", "out", "PUTS", "puts",
	"IN", "in", "PUTSP", "putsp"}
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