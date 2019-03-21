#define USES_OPERAND
#include "laser.h"

int8_t arrcmp (char *str, const char *arr[][2], uint8_t size)
{
	for (uint8_t i = 0; i < size; i++) {
		if (strcmp (str, arr[i][0]) == 0 ||
			strcmp (str, arr[i][1]) == 0) return i;
	}
	return -1;
}

int8_t isregister (Token *token)
{
	const char *regs[][2] = {
		{"R0", "r0"},
		{"R1", "r0"},
		{"R2", "r0"},
		{"R3", "r0"},
		{"R4", "r0"},
		{"R5", "r0"},
		{"R6", "r0"},
		{"R7", "r0"}
	};

	return arrcmp (token->str, regs, 8);
}

int8_t isbranch (Token *token)
{
	const char *brs[][2] = {
		{"BR", "br"},
		{"BRp", "brp"},
		{"BRz", "brz"},
		{"BRzp", "brzp"},
		{"BRn", "brn"},
		{"BRnp", "brnp"},
		{"BRnz", "brnz"},
		{"BRnzp", "brnzp"}
	};
	
	return arrcmp (token->str, brs, 8);
}

int8_t istrap (Token *token)
{
	const char *traps[][2] = {
		{"GETC", "getc"},
		{"OUT", "out"},
		{"PUTS", "puts"},
		{"IN", "in"},
		{"PUTSP", "putsp"},
		{"HALT", "halt"},
		{"TRAP", "trap"}
	};

	int8_t tmp = arrcmp (token->str, traps, 7);
	if (tmp == -1) return -1;
	else if (tmp == 7) return 0;
	else return 32 + tmp;
}

int8_t isoperand (Token *token)
{
	const char *ops[][2] = {
		{"BR", "br"},
		{"ADD", "add"},
		{"LD", "ld"},
		{"ST", "st"},
		{"JSR", "jsr"},
		{"AND", "and"},
		{"LDR", "ldr"},
		{"STR", "str"},
		{"RTI", "rti"},
		{"NOT", "not"},
		{"LDI", "ldi"},
		{"STI", "sti"},
		{"JMP", "jmp"},
		{"", ""},
		{"LEA", "lea"},
		{"TRAP", "trap"}
	};

	int8_t op = arrcmp (token->str, ops, 16);
	if (op < 0) {																// could be branch, trap, or assembler shortcut
		if (isbranch (token) >= 0) {
			op = BR;
		} else if (istrap (token) >= 0) {
			op = TRAPS;
		} else if (strcmp (token->str, "JSRR") == 0 ||
				   strcmp (token->str, "jsrr") == 0) {
			op = JSRR;
		} else if (strcmp (token->str, "RET") == 0 ||
				   strcmp (token->str, "ret") == 0) {
			op = RET;
		}
	} else if (op == 13) {														// invalid op
		op = -1;
	}
	return op;
}