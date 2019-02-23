#include "laser.h"

int8_t arrcmp (char *str, const char *arr[][2], uint8_t size)
{
	for (uint8_t i = 0; i < size; i++) {
		if (strcmp (str, arr[i][0]) == 0 ||
			strcmp (str, arr[i][1]) == 0) return i;
	}
	return -1;
}

int8_t isregister (char *token)
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

	return arrcmp (token, regs, 8);
}

int8_t isbranch (char *token)
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
	
	return arrcmp (token, brs, 8);
}

int8_t istrap (char *token)
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

	return arrcmp (token, traps, 7);
}

enum opcodes {BR, ADD, LD, ST,
			   JSR, AND, LDR, STR,
			   RTI, NOT, LDI, STI,
			   JMP, INV, LEA, TRAP,
			   TRAPS, JSRR, RET};

int8_t isoperand (char *token)
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

	int8_t op = arrcmp (token, ops, 16);
	if (op < 0) {	// could be branch, trap, or assembler shortcut
		if (isbranch (token) >= 0) {
			op = BR;
		} else if (istrap (token) >= 0) {
			op = TRAPS;
		} else if (strcmp (token, "JSRR") == 0 ||
				   strcmp (token, "jsrr") == 0) {
			op = JSRR;
		} else if (strcmp (token, "RET") == 0 ||
				   strcmp (token, "ret") == 0) {
			op = RET;
		}
	} else if (op == 13) {		// invalid op
		op = -1;
	}
	return op;
}

uint8_t operandnum (int8_t operand)
{
	switch (operand) {
		case BR: return 1;
		case ADD: return 3;
		case LD: return 2;
		case ST: return 2;
		case JSR: return 1;
		case AND: return 3;
		case LDR: return 3;
		case STR: return 3;
		case RTI: return 0;
		case NOT: return 2;
		case LDI: return 2;
		case STI: return 2;
		case JMP: return 1;
		case INV: return 0;
		case LEA: return 2;
		case TRAP: return 1;
		case TRAPS: return 0;
		case JSRR: return 1;
		case RET: return 0;
		default: return 0;
	}
}