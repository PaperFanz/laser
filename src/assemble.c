#define USES_FILE
#define USES_FLAG
#define USES_ALIAS
#define USES_MACRO
#define USES_LABEL
#define USES_NOTIFY
#define USES_OFFSET
#define USES_OPERAND
#define USES_PSEUDOOP
#include "laser.h"

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

uint16_t preorig (struct Files f, uint32_t *ln, Arrays *a)
{
	uint16_t orig_addr = 0;
	int32_t i = 0;
	char line[MAX_LEN + 1];

	for (i = *ln; fgets (line, MAX_LEN + 1, f.fp) != NULL; i++) {

		TokenBuffer *buf = tokenize (line);
		Token **token = buf->token;
		if (buf->toknum == 0){													// skip empty lines
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
			writefilebuf (orig_addr, i);
			break;
		default:
			warning (i, "Ignoring invalid token '%s' before '.ORIG'", token[0]);
			break;
		}

		freetokenarr (buf);
		if (orig_addr) break;
	}

	if (!orig_addr) error (i, "No origin detected!");

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

uint8_t passone (uint32_t ln, uint16_t *addr, TokenBuffer *buf, Arrays *arrs)
{
	uint8_t end = 0;
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
	int8_t op = -1, pop = -1, opnum = -1, opcount = 0;
	char *tok;

	for (uint8_t j = 0; j < buf->toknum; j++) {
		if (isvalidlabel (token[j]) && j == 0) {
			arrs->label = addlabel (arrs->label, ln, token[j], *addr);
			opnum = 0;
		} else if ((op = isoperand (token[j])) >= 0) {
			tok = token[j]->str;
			opnum = opnumarr[op];
			*addr += 1;
		} else if ((pop = ispseudoop (token[j])) >= 0) {
			tok = token[j]->str;
			opnum = popnumarr [pop];
			if (pop == END) {
				end = 1;
				break;
			}

			if (j + 1 >= buf->toknum) {
				// error will be handled later
				break;
			}

			if (pop == STRINGZ) {
				*addr += token[j + 1]->len;
			} else if (pop == BLKW) {
				uint8_t offt = offtype (token[j + 1]);
				if (offt > 0) *addr += offset (offt, token[j + 1]);
			} else if (pop == FILL) {
				*addr += 1;
			} else {
				warning (ln, "Ignoring unexpected use of '%s' in program body",
						token[j]);
			}
		} else if (isregister (token[j]) >= 0) {
			opcount++;
		} else if (offtype (token[j]) >= 0) {
			opcount++;
		} else {
			error (ln, "Unrecognized token '%s'", token[j]->str);
		}
	}

	if (opcount > opnum) {
		warning (ln, "'%s' expects %d operands", tok, opnum);
	} else if (opcount < opnum) {
		error (ln,	"'%s' expects %d operands", tok, opnum);
	}

	abuttokenbufferarray (buf, ln);
	if (end) return 1;
	else return 0;
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

#define INRANGE5(x) (-16 <= (x)) && ((x) <= 15)
#define INRANGE6(x) (-32 <= (x)) && ((x) <= 31)
#define INRANGE9(x) (-256 <= (x)) && ((x) <= 255)
#define INRANGEB(x) (-1024 <= (x)) && ((x) <= 1023)


uint8_t passtwo (uint32_t tbufind, uint16_t *addr, Arrays *arrs)
{
	TokenBuffer *buf = fromtokenbufferarray (tbufind);
	uint32_t ln = linetokenbufferarray (tbufind);
	uint8_t end = 0;
	
	Token **token = buf->token;
	int8_t opcode = -1, pseudoop = -1;

	uint32_t label = labeladdr (arrs->label, token[0]);
	if (label) {
		if (buf->toknum == 1) return 0;											// don't do anything if there's only a label
		token++;																// increment past labels
	}

	if ((opcode = isoperand (token[0])) >= 0) {
		uint16_t ins = opcodearr[opcode] << 12;
		uint8_t opmask = opmaskarr[opcode];										// essentially an array of 8 booleans
		if (opmask & DRSR) {
			int8_t reg = isregister (token[1]);
			if (reg >= 0) {
				ins += reg << 9;
			} else {
				error (ln, "'%s' is not a register", token[1]->str);
			}
		}
		if (opmask & SRC1) {
			int8_t reg = 0, tmp = 0;
			if (opcode == RET) {
				reg = 7;
			} else if (opcode == JMP || opcode == JSRR) {
				tmp = 1;
			}
			else {
				tmp = 2;
			}

			if (reg == 7 || (reg = isregister (token[tmp])) >= 0) {
				ins += reg << 6;
			} else {
				error (ln, "'%s' is not a register", token[tmp]->str);
			}
		}
		if (opmask & SRC2) {
			int8_t reg = isregister (token[3]);
			uint8_t offt;
			if (reg >= 0) {
				ins += reg;
			} else if ((offt = offtype (token[3])) > 0) {
				int16_t off = offset (offt, token[3]);
				if (!INRANGE5 (off)) {
					error (ln, "'%s' is not expressible in 5 bits",
							token[3]->str);
				}
				ins += 0x20;													// 1 in bit 5 indicates immediate value
				ins += off & 0x1F;
			} else {
				error (ln, "'%s' is not a valid argument for '%s'",
						token[3]->str, token[0]->str);
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
				int16_t off = (laddr - (*addr + 1));
				if (!INRANGE9 (off)) {
					error (ln, "'%s' is not expressible in 9 bits",
							token[tmp]->str);
				}
				ins += off & 0x1FF;
			} else if ((offt = offtype (token[tmp])) > 0) {
				int16_t off = offset (offt, token[tmp]);
				if (!INRANGE9 (off)) {
					error (ln, "'%s' is not expressible in 9 bits",
							token[tmp]->str);
				}
				ins += off & 0x1FF;
			} else {
				error (ln, "'%s' is not a valid argument for '%s'",
						token[tmp]->str, token[0]->str);
			}
		}
		if (opmask & PC11) {
			uint8_t offt;
			uint16_t laddr = labeladdr (arrs->label, token[1]);
			ins += 0x800;														// 1 in bit 11 indicates JSR (JSRR is 0)
			if (laddr) {
				int16_t off = (laddr - (*addr + 1));
				if (!INRANGEB (off)) {
					error (ln, "'%s' is not expressible in 11 bits",
							token[1]->str);
				}
				ins += off & 0x7FF;
			} else if ((offt = offtype (token[1])) > 0) {
				int16_t off = offset (offt, token[1]);
				if (!INRANGEB (off)) {
					error (ln, "'%s' is not expressible in 11 bits",
							token[1]->str);
				}
				ins += off & 0x7FF;
			} else {
				error (ln, "'%s' is not a valid argument for '%s'",
						token[1]->str, token[0]->str);
			}
		}
		if (opcode ==  NOT) {
			ins += 0x3F;														// this is in the spec for some reason
		}
		if (opmask & OFF6) {
			uint8_t offt = offtype (token[3]);
			if (offt > 0) {
				int16_t off = offset (offt, token[3]);
				if (!INRANGE6 (off)) {
					error (ln, "'%s' is not expressible in 6 bits",
							token[3]->str);
				}
				ins += off & 0x3F;
			} else {
				error (ln, "'%s' is not a valid argument for '%s'",
						token[3]->str, token[0]->str);
			}
		}
		if (opmask & TVEC) {
			int16_t trapvect8 = istrap (token[0]);
			
			if (trapvect8 == 0) {
				uint8_t offt = offtype (token[1]);

				if (offt > 0) {
					trapvect8 = offset (offt, token[1]);
					if (trapvect8 > 0x25 || trapvect8 < 0x20) {
						warning (ln, "'%s' is not a predefined trap routine",
								token[1]->str);
					}
					if (trapvect8 > 0xFF || trapvect8 < 0) {
						error (ln, "'%s' is not a valid trap vector",
								token[1]->str);
					}
				} else {
					error (ln, "'%s' is not a valid trap vector",
							token[1]->str);
				}
			}
			ins += trapvect8 & 0xFF;
		}
		writefilebuf (ins, ln);
		*addr += 1;
	} else if ((pseudoop = ispseudoop (token[0])) >= 0) {
		if (pseudoop == END) {
			end = 1;
		} else if (pseudoop == STRINGZ) {
			for (uint16_t k = 0; k < token[1]->len; k++)
				writefilebuf (token[1]->str[k], ln);
			*addr += token[1]->len;
		} else if (pseudoop == BLKW) {
			uint8_t offt = offtype (token[1]);
			uint16_t blknum = 0;

			if (offt > 0) {
				blknum = offset (offt, token[1]);
			} else {
				error (ln, "'%s' is not a valid argument for '%s'",
						token[1]->str, token[0]->str);
			}

			for (uint16_t k = 0; k < blknum; k++)
				writefilebuf (0, ln);

			*addr += blknum;
		} else if (pseudoop == FILL) {
			uint8_t offt = offtype (token[1]);
			uint16_t laddr;

			if (offt > 0) {
				writefilebuf (offset (offt, token[1]), ln);
				*addr += 1;
			} else if ((laddr = labeladdr (arrs->label, token[1])) > 0) {
				writefilebuf (laddr, ln);
			} else {
				error (ln, "'%s' is not a valid argument for '%s'",
						token[1]->str, token[0]->str);
			}
		} else {
			warning (ln, "Ignoring unexpected use of '%s' in program body",
					token[0]->str);
		}
	} else {
		error (ln, "Unrecognized token '%s'", token[0]);
	}

	if (end) return 1;
	else return 0;
}

void writesym (FILE *sym, Label *l)
{
	uint16_t labelnum = l[0].count;
	for (uint16_t i = 1; i < labelnum; i++) {
		Label tmp = l[i];
		printsymbol (sym, tmp.label, tmp.address);
	}
}

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
	for (int i = 0; i < (islogging () ? 6 : 5); i++) {
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

int8_t assemble (char *file)
{
	int8_t err = 0;
	notify ("Assembling %s...", file);

	Files f = {NULL, NULL, NULL, NULL, NULL, NULL};
	err += openasmfiles (&f, file);
	if (err) return err;														// exit with error if files unopenable

	Arrays arrs = {
		initaliasarr (),
		initmacroarr (),
		initlabelarr ()
	};

	resetfilebuf ();
	inittokenbufferarray ();

	uint32_t ln = 1;
	uint16_t origaddr = preorig (f, &ln, &arrs);
	++ln;

	fpos_t origpos;
	fgetpos (f.fp, &origpos);

	char line[MAX_LEN + 1];
	uint8_t end = 0;
	uint16_t addr = origaddr;

	// pass one
	for (uint32_t i = ln ; fgets (line, MAX_LEN + 1, f.fp); i++) {
		TokenBuffer *buf = tokenize (line);
		if (buf->toknum == 0) {													// skip empty lines
			freetokenarr (buf);
			continue;
		}
		end = passone (i, &addr, buf, &arrs);
		if (end) break;
	}
	if (!end) {
		error (ln, "No end detected!");
	}

	notify ("%d error(s) and %d warning(s) in pass one",
			geterrors(), getwarnings());
	// pass two does not execute if errors are encountered in pass one
	if (!geterrors()) {
		fsetpos (f.fp, &origpos);
		end = 0;
		addr = origaddr;

		uint16_t endofprogram = tokenbufferarrayend ();
		for (uint16_t i = 0; endofprogram; ++i, --endofprogram) {
			end = passtwo (i, &addr, &arrs);
			if (end) break;
		}
		notify ("%d error(s) and %d warning(s) in pass two",
				geterrors(), getwarnings());
	} else {
		notify ("Unresolved errors encountered in pass one, exiting...");
	}

	// write buffers to file

	writeobj (f.obj);
	writehex (f.hex);
	writebin (f.bin);
	writelst (f.fp, f.lst);
	writesym (f.sym, arrs.label);
	
	if (geterrors()) clean (file);

	notify ("Done!\n");

	freetokenbufferarray ();
	freealiasarr (arrs.alias);
	freemacroarr (arrs.macro);
	freelabelarr (arrs.label);
	closeasmfiles (&f);

	return err;
}