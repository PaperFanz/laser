#define USES_FILE
#define USES_ALIAS
#define USES_MACRO
#define USES_LABEL
#define USES_NOTIFY
#define USES_OFFSET
#define USES_OPERAND
#define USES_PSEUDOOP
#include "laser.h"

const char *extensions[] = {
	".sym",
	".bin",
	".hex",
	".lst",
	".obj",
	".log"
};

int8_t clean (char *file)
{
	bool err = 0;
	notify ("Cleaning up...");
	for (int i = 0; i < (ENABLE_LOGGING ? 6 : 5); i++) {
		file = replaceextension (file, extensions[i]);
		notify ("  Deleting %s...", file);
		if (remove (file)) {
			notify ("    Unable to delete %s!", file);
			err = 1;
		}
	}
	notify ("Finished!\n");

	file = replaceextension (file, ".sym");

	if (err) return -2;
	else return 0;
}

typedef struct Alert {
	int32_t warn;
	int32_t err;
	int32_t exceptions;
} Alert;

typedef struct Arrays {
	struct Alias *alias;
	struct Macro *macro;
	struct Label *label;
} Arrays;

uint16_t preorig (struct Files f, uint32_t *ln, Alert *alt, Arrays *a)
{
	uint16_t orig_addr = 0;
	int32_t i = 0;
	char line[MAX_LEN + 1];

	for (i = *ln; fgets (line, MAX_LEN + 1, f.asm_) != NULL; i++) {

		TokenBuffer *buf = tokenize (line);
		Token **token = buf->token;
		if (buf->token[0] == NULL){												// skip empty lines
			freetokenarr (buf);
			continue;
		}

		switch (ispseudoop (token[0])) {
		case ALIAS:
			addalias (a->alias, *ln, token[1], token[2]);
			break;
		case MACRO:
			a->macro = addmacro (a->macro, *ln, token[1], token[2]);
			break;
		case ORIG:
			orig_addr = offset(1, token[1]);
			break;
		default:
			error (WARN, f.log_, i,
				   "Ignoring invalid token '%s' before '.ORIG'", token[0]);
			break;
		}

		freetokenarr (buf);
		if (orig_addr) break;
	}

	if (!orig_addr) error (ERR, f.log_, i, "No origin detected!");

	*ln = i;
	return orig_addr;
}

const uint8_t opnumarr[] = {
	1,	// BR
	3,	// ADD
	2,	// LD
	2,	// ST
	1,	// JSR
	3,	// AND
	3,	// LDR
	3,	// STR
	0,	// RTI
	2,	// NOT
	2,	// LDI
	2,	// STI
	1,	// JMP
	0,	// INV
	2,	// LEA
	1,	// TRAP
	0,	// TRAPS
	1,	// JSRR
	0 	// RET
};

const uint8_t popnumarr[] = {
	2,	// ALIAS
	2,	// MACRO
	1,	// ORIG
	0,	// END
	1,	// STRINGZ
	1,	// BLKW
	1	// FILL
};

void passone (struct Files f, uint32_t ln, uint16_t addr,
			  Alert *alt, Arrays *arrs)
{
	char line[MAX_LEN + 1];
	int8_t end = 0;
	uint32_t i;

	for (i = ln; fgets (line, MAX_LEN + 1, f.asm_) != NULL; i++) {
		TokenBuffer *buf = tokenize (line);
		if (buf->toknum == 0){													// skip empty lines
			freetokenarr (buf);
			continue;
		}

		uint32_t macro = findmacro (arrs->macro, buf->token[0]);
		if (macro) {
			freetokenarr (buf);
			buf = tokenize (arrs->macro[macro].replace->str);
		}
		
		for (uint8_t j = 0; j < buf->toknum; j++) {
			uint32_t alias = findalias (arrs->alias, buf->token[j]);
			if (alias) {
				freetoken (buf->token[j]);
				buf->token[j] = (Token*) malloc (sizeof (Token));
				copytoken (buf->token[j], arrs->alias[alias].reg);
			}
		}

		Token **token = buf->token;
		instruction_t ins = {0, i + 1, NULL};
		int8_t opcode = -1, pseudoop = -1, opnum = -1, opcount = 0;

		for (uint8_t j = 0; j < buf->toknum; j++) {
			char *tok = token[j]->str;
			if (isvalidlabel (token[j]) && j == 0) {
				arrs->label = addlabel (arrs->label, ln, token[j], addr);
				printsymbol (f.sym_, token[j], addr);
				opnum = 0;
			} else if ((opcode = isoperand (token[j])) >= 0) {					// is an opcode
				opnum = opnumarr[opcode];
				addr++;
			} else if ((pseudoop = ispseudoop (token[j])) >= 0) {				// is pseudoop
				opnum = popnumarr [pseudoop];
				if (j + 1 < buf->toknum)
					addr += addrnum (pseudoop, token[j + 1]);
				if (pseudoop == END)
					end = 1;
			} else if (isregister (token[j]) >= 0) {
				opcount++;
			} else if (offtype (token[j]) >= 0) {
				opcount++;
			} else {															// unrecognized token
				alt->err++;
				error (ERR, f.log_, i,
					"Unrecognized token '%s'", token[j]->str);
			}
		}

		if (opcount > opnum) {
			alt->warn++;
			error (WARN, f.log_, i,
				"'%s' expects %d operands", ins.op, opnum);
		} else if (opcount < opnum) {
			alt->err++;
			error (ERR, f.log_, i,
				"'%s' expects %d operands", ins.op, opnum);
		}

		freetokenarr (buf);
		if (end) break;
	}

	if (!end) error (ERR, f.log_, i, "No end detected!");
}

const int8_t opcodearr[] = {
	0,	// BR
	1,	// ADD
	2,	// LD
	3,	// ST
	4,	// JSR
	5,	// AND
	6,	// LDR
	7,	// STR
	8,	// RTI
	9,	// NOT
	10,	// LDI
	11,	// STI
	12,	// JMP
	-1,	// INVALID
	14,	// LEA
	15,	// TRAP
	15,	// TRAPS
	4,	// JSRR
	12	// RET
};

#define DRSR 0b00000001
#define SRC1 0b00000010
#define SRC2 0b00000100
#define COND 0b00001000
#define PC09 0b00010000
#define PC11 0b00100000
#define OFF6 0b01000000
#define TVEC 0b10000000

const uint8_t opmaskarr[] = {
	COND | PC09,			// BR
	DRSR | SRC1 | SRC2,		// ADD
	DRSR | PC09,			// LD
	DRSR | PC09,			// ST
	PC11,					// JSR
	DRSR | SRC1 | SRC2,		// AND
	DRSR | SRC1 | OFF6,		// LDR
	DRSR | SRC1 | OFF6,		// STR
	0,						// RTI
	DRSR | SRC1,			// NOT
	DRSR | PC09,			// LDI
	DRSR | PC09,			// STI
	SRC1,					// JMP
	0,						// INVALID
	DRSR | PC09,			// LEA
	TVEC,					// TRAP
	TVEC,					// TRAPS
	SRC1,					// JSRR
	SRC1,					// RET
};

void passtwo (struct Files f, uint32_t ln, uint16_t addr,
			  Alert *alt, Arrays *arrs)
{
	char line[MAX_LEN + 1];
	int8_t end = 0;
	uint32_t i;

	for (i = ln; fgets (line, MAX_LEN + 1, f.asm_) != NULL; i++) {
		TokenBuffer *buf = tokenize (line);
		if (buf->toknum == 0){													// skip empty lines
			freetokenarr (buf);
			continue;
		}

		uint32_t macro = findmacro (arrs->macro, buf->token[0]);
		if (macro) {
			freetokenarr (buf);
			buf = tokenize (arrs->macro[macro].replace->str);
		}

		for (uint8_t j = 0; j < buf->toknum; j++) {
			uint32_t alias = findalias (arrs->alias, buf->token[j]);
			if (alias) {
				freetoken (buf->token[j]);
				buf->token[j] = (Token*) malloc (sizeof (Token));
				copytoken (buf->token[j], arrs->alias[alias].reg);
			}
		}
		
		Token **token = buf->token;
		uint32_t curln = i + 1;
		int8_t opcode = -1, pseudoop = -1;

		uint32_t label = labeladdr (arrs->label, token[0]);
		if (label) {
			if (buf->toknum == 1) {												// continue if label is only token
				freetokenarr (buf);
				continue;
			}
			token++;															// increment past labels
		}

		if ((opcode = isoperand (token[0])) >= 0) {
			uint16_t ins = opcodearr[opcode] << 12;
			uint8_t opmask = opmaskarr[opcode];									// essentially an array of 8 booleans
			if (opmask & DRSR) {
				int8_t reg = isregister (token[1]);
				if (reg >= 0) {
					ins += reg << 9;
				} else {
					alt->err++;
					// error message TODO
				}
			}
			if (opmask & SRC1) {
				int8_t reg;
				if (opcode == JMP || opcode == JSRR || opcode == RET)
					reg = isregister (token[1]);
				else
					reg = isregister (token[2]);

				if (reg >= 0) {
					ins += reg << 6;
				} else {
					alt->err++;
					// error message TODO
				}
			}
			if (opmask & SRC2) {
				int8_t reg = isregister (token[3]);
				uint8_t offt;
				if (reg >= 0) {
					ins += reg;
				} else if ((offt = offtype (token[3])) > 0) {
					// check offset range TODO
					ins += 0x20;												// 1 in bit 5 indicates immediate value
					ins += offset (offt, token[3]) & 0x1F;
				} else {
					alt->err++;
					// error message TODO
				}
			}
			if (opmask & COND) {
				ins += isbranch (token[0]) << 9;
			}
			if (opmask & PC09) {
				uint8_t tmp, offt;
				if (opcode == BR) tmp = 1;
				else tmp = 2;

				uint16_t laddr = labeladdr (arrs->label, token[tmp]);
				if (laddr > 0) {
					// check offset range TODO
					ins += (laddr - (addr + 1)) & 0x1FF;
				} else if ((offt = offtype (token[tmp])) > 0) {
					// check offset range TODO
					ins += offset (offt, token[tmp]) & 0x1FF;
				} else {
					alt->err++;
					// error message TODO
				}
			}
			if (opmask & PC11) {
				uint8_t offt;
				uint16_t laddr = labeladdr (arrs->label, token[1]);

				ins += 0x800;													// 1 in bit 11 indicates JSR (JSRR is 0)
				if (laddr) {
					// check offset range TODO
					ins += (laddr - addr) & 0x7FF;
				} else if ((offt = offtype (token[1])) > 0) {
					// check offset range TODO
					ins += offset (offt, token[1]) & 0x7FF;
				} else {
					alt->err++;
					// error message TODO
				}
			}
			if (opcode ==  NOT) {
				ins += 0x3F;													// this is in the spec for some reason
			}
			if (opmask & OFF6) {
				uint8_t offt = offtype (token[3]);
				if (offt > 0) {
					// check offset range TODO
					ins += offset (offt, token[3]) & 0x3F;
				} else {
					alt->err++;
					// error message TODO
				}
			}
			if (opmask & TVEC) {
				int8_t trapvect8 = istrap (token[0]);
				if (trapvect8 > 0) {
					ins += trapvect8;
				} else if (trapvect8 == 0) {
					uint8_t offt = offtype (token[1]);
					if (offt > 0) {
						ins += offset (offt, token[1]) & 0xFF;
					} else {
						alt->err++;
						// error message TODO
					}
				} else {
					alt->err++;
					// error message TODO
				}
			}
			addr++;
		} else if ((pseudoop = ispseudoop (token[0])) >= 0) {

		} else {
			alt->err++;
			error (ERR, f.log_, curln, "Unrecognized token '%s'", token[0]);
		}

		freetokenarr (buf);
		if (end) break;
	}

	if (!end) error (ERR, f.log_, i, "No end detected!");
}

int8_t assemble (char *file)
{
	int8_t err = 0;
	Alert alt = {0, 0, 0};
	notify ("Assembling %s...\n", file);

	struct Files f = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
	err += openasmfiles (&f, file);
	if (err) return err;	// exit with error if files unopenable

	Arrays arrs = {
		initaliasarr(),
		initmacroarr(),
		initlabelarr()
	};

	uint32_t ln = 1;
	uint16_t origaddr = preorig (f, &ln, &alt, &arrs);
	uint32_t ln_st = ln;
	
	fpos_t origpos;
	fgetpos (f.asm_, &origpos);

	// Pass 1 TODO
	passone (f, ln, origaddr, &alt, &arrs);

	// Pass 2 TODO
	if (!alt.err || !alt.exceptions) {
		fsetpos (f.asm_, &origpos);
		passtwo (f, ln, origaddr, &alt, &arrs);
	}

	// cleanup
	if (alt.err > 0 || alt.exceptions > 0) clean (file);
	freealiasarr (arrs.alias);
	freemacroarr (arrs.macro);
	freelabelarr (arrs.label);
	closeasmfiles (&f);

	return err;
}