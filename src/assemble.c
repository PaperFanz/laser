/*
*   Laser- a command line utility to assemble LC3 assembly code
*
*   Copyright Notice:
*
*       Copyright 2018, 2019 Zhiyuan Fan
*
*   License Notice:
*
*       Laser is free software: you can redistribute it and/or modify
*       it under the terms of the GNU General Public License as published by
*       the Free Software Foundation, either version 3 of the License, or
*       (at your option) any later version.
*
*       Laser is distributed in the hope that it will be useful,
*       but WITHOUT ANY WARRANTY; without even the implied warranty of
*       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*       GNU General Public License for more details.
*
*       You should have received a copy of the GNU General Public License
*       along with Laser.  If not, see <https://www.gnu.org/licenses/>.
*/

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

/*
    arrs_t: this typedef'd struct is used to keep project assembly organized,
    since each '.asm' file requires a filebuffer, an array of relevant resources
    (aliases, macros, and labels), and an array of tokens from preorig and
    passone
*/

typedef struct Arrays {
    filebuf_t *filebuf;
    aliasarr_t *alias;
    macroarr_t *macro;
    labelarr_t *label;
    tokbufarr_t *tokbufarr;
} arrs_t;

/*
    preorig: travels through and assembly file until it encounters the '.ORIG'
    statement, then returns the specified origin address. Returns 0 if no
    '.ORIG' is encountered

    inputs: an array of files relevant to the currently assembled file
            a pointer to the line number for error reporting
            pointer to an arrs_t struct initialized in assemble()

    outputs: origin address or 0 (err)
*/

uint16_t preorig (filearr_t f, uint32_t *ln, arrs_t *arrs)
{
    uint16_t orig_addr = 0;
    uint32_t i;
    char line[MAX_LEN + 1];

    for (i = *ln; fgets (line, MAX_LEN + 1, f.fp) != NULL; i++) {

        TokenBuffer *buf = tokenize (line);
        Token **token = buf->token;
        if (buf->toknum == 0){  						// skip empty lines
            freetokenarr (buf);
            continue;
        }

        switch (ispseudoop (token[0])) {
        case ALIAS:
            addalias (arrs->alias, i, token[1], token[2]);
            break;
        case MACRO:
            addmacro (arrs->macro, i, token[1], token[2]);
            break;
        case ORIG:
            orig_addr = offset(1, token[1]);
            writefilebuf (arrs->filebuf, 0, orig_addr, i);
            break;
        case EXPORT:
        case IMPORT:
            if (!isproject ())
                warning (i, "'%s' is only used in project mode", token[0]->str);
            break;
        default:
            warning (i, "Ignoring invalid token '%s' before '.ORIG'",
                    token[0]->str);
            break;
        }

        abuttokenbufferarray (arrs->tokbufarr, buf, i);
        if (orig_addr) break;
    }
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
    1,	// FILL
    1,	// EXPORT
    1	// IMPORT
};

/*
    passone: parses a single line of assembly, checking for number of arguments
             based on opcode and generating the symbol table

    inputs: line number
            pointer to the address
            a token buffer with the tokenized line
            pointer to an arrs_t with the populated resources

    outputs: 0, 1 when '.END' is reached
*/
uint8_t passone (uint32_t ln, uint16_t *addr, TokenBuffer *buf, arrs_t *arrs)
{
    uint8_t end = 0;    // return value

    // if line begins with a macro, replace the token buffer with the declared
    // replacement and variables (if any)
    // uint32_t macro = findmacro (arrs->macro, buf->token[0]);
    // if (macro != -1) {
    //     buf = replacemacro (buf, arrs->macro->arr[macro].replace);
    //     if (buf == NULL) {
    //         error (ln, "macro '%s : %s' requires an argument!",
    //                 arrs->macro->arr[macro].macro->str,
    //                 arrs->macro->arr[macro].replace->str);
    //         return 1;
    //     }
    // }

    // some shortcuts
    Token **token = buf->token;
    int8_t op = -1, pop = -1, opnum = -1, opcount = 0;
    char *tok;  // used to store the operator for this tokenbuffer

    for (uint8_t j = 0; j < buf->toknum; ++j) {

        uint32_t alias = -1;    // value must be reset to -1 with every loop

        if (j == 0 && isvalidlabel (token[j])) {
            /*
                a label should only be added to the symbol table if it is the
                first token in a line
            */
            tok = token[j]->str;
            
            char *exists = existlabel (arrs->label, *addr);

            if (exists == NULL) {
                addlabel (arrs->label, ln, token[j], *addr);
            } else {
                error (ln, "Multiple labels pointing to same address: '%s' and '%s'",
                        exists, token[j]->str);    
            }
            
            opnum = 0;
        } else if ((op = isoperand (token[j])) >= 0) {
            tok = token[j]->str;
            opnum = opnumarr[op];
            *addr += 1;
        } else if ((pop = ispseudoop (token[j])) >= 0) {
            tok = token[j]->str;
            opnum = popnumarr[pop];
            if (pop == END) {
                end = 1;
                break;
            }

            // this error will be handled later, in the opnum checking
            if (j + 1 >= buf->toknum) break;

            if (pop == STRINGZ) {
                *addr += token[j + 1]->len;
                ++j;            // increment past string argument (unchecked)
                ++opcount;
            } else if (pop == BLKW) {
                uint8_t offt = offtype (token[j + 1]);
                if (offt > 0) *addr += offset (offt, token[j + 1]);
            } else if (pop == FILL) {
                *addr += 1;
            } else {
                warning (ln, "Ignoring unexpected use of '%s' in program body",
                        token[j]->str);
            }
        } else if (isregister (token[j]) >= 0) {
            opcount++;
        } else if (offtype (token[j]) > 0) {
            opcount++;
        } else if ((alias = findalias (arrs->alias, token[j])) != -1) {
            freetoken (buf->token[j]);
            buf->token[j] = (Token*) malloc (sizeof (Token));
            copytoken (buf->token[j], arrs->alias->arr[alias].reg);
            opcount++;
        } else if (isvalidlabel (token[j])) {
            opcount++;
        } else {
            error (ln, "Unrecognized token '%s'", token[j]->str);
        }
    }

    if (opnum == -1) {
        
    } else if (opcount > opnum) {
        warning (ln, "'%s' expects %d operands", tok, opnum);
    } else if (opcount < opnum) {
        error (ln,	"'%s' expects %d operands", tok, opnum);
    }

    abuttokenbufferarray (arrs->tokbufarr, buf, ln);
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

#define INRANGE5(x) ((-16 <= (x)) && ((x) <= 15))
#define INRANGE6(x) ((-32 <= (x)) && ((x) <= 31))
#define INRANGE9(x) ((-256 <= (x)) && ((x) <= 255))
#define INRANGEB(x) ((-1024 <= (x)) && ((x) <= 1023))

/*
    passtwo: converts a single line of assembly to an instruction, and writing
             the relevant information to a filebuffer (in arrs_t)

    inputs: an index for the populated array of token buffers that will
            be used to fetch the token buffer to parse,
            a pointer to the address,
            a pointer to an arrs_t generated in assemble() and populated in
            preorig() and passone()

    outputs: 0, 1 when '.END' is reached
*/

uint8_t passtwo (uint32_t tbufind, uint16_t *addr, arrs_t *arrs)
{
    lineinfo_t *line = fromtokenbufferarray (arrs->tokbufarr, tbufind);
    uint32_t ln = line->ln;
    uint8_t end = 0;
    
    Token **token = line->buf->token;
    int8_t opcode = -1, pseudoop = -1;

    uint32_t label = labeladdr (arrs->label, token[0]);
    if (label) {
        if (line->buf->toknum == 1) return 0;	// ignore label-only lines
        token++;								// increment past labels
    }

    if ((opcode = isoperand (token[0])) >= 0) {

        uint16_t ins = opcodearr[opcode] << 12;
        uint8_t opmask = opmaskarr[opcode];		// essentially an array of 8 bools
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
                ins += 0x20;			// 1 in bit 5 indicates immediate value
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
            ins += 0x800;				// 1 in bit 11 indicates JSR (JSRR is 0)
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
            ins += 0x3F;				// this is in the spec for some reason
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
        writefilebuf (arrs->filebuf, *addr, ins, ln);
        *addr += 1;

    } else if ((pseudoop = ispseudoop (token[0])) >= 0) {

        switch (pseudoop) {
        case END: {
            end = 1;
            break;
        }
        case STRINGZ: {
            for (uint16_t k = 0; k < token[1]->len; k++) {
                writefilebuf (arrs->filebuf, *addr, token[1]->str[k], ln);
                *addr += 1;
            }
            break;
        }
        case BLKW: {
            uint8_t offt = offtype (token[1]);
            uint16_t blknum = 0;

            if (offt > 0) {
                blknum = offset (offt, token[1]);
            } else {
                error (ln, "'%s' is not a valid argument for '%s'",
                        token[1]->str, token[0]->str);
            }

            for (uint16_t k = 0; k < blknum; k++) {
                writefilebuf (arrs->filebuf, *addr, 0, ln);
                *addr += 1;
            }
            break;
        }
        case FILL: {
            uint8_t offt = offtype (token[1]);
            uint16_t laddr;

            if (offt > 0) {
                writefilebuf (arrs->filebuf, *addr, offset (offt, token[1]), ln);
                *addr += 1;
            } else if ((laddr = labeladdr (arrs->label, token[1])) > 0) {
                writefilebuf (arrs->filebuf, *addr, laddr, ln);
                *addr += 1;
            } else {
                error (ln, "'%s' is not a valid argument for '%s'",
                        token[1]->str, token[0]->str);
            }
            break;
        }
        case EXPORT:
        case IMPORT:
            if (isproject ()) break;
        default:
            warning (ln, "Ignoring unexpected use of '%s' in program body",
                    token[0]->str);
            break;
        }

    } else {
        error (ln, "Unrecognized token '%s'", token[0]->str);
    }

    if (end) return 1;
    else return 0;
}

/*
    assemble: assembles a single file specified by the filename passed as
    a string, without checking file extension or format

    input: filename as string
    output: 0 if successful, 1 if unsuccessful
*/
int8_t assemble (char *file)
{
    initnotify ();
    setcurrentfile (file);
    notify ("Assembling \"%s\"...\n", file);

    filearr_t f = {NULL, NULL, NULL, NULL, NULL, NULL};
    if (openasmfiles (&f, file)) return 1;	     // exit if files unopenable

    arrs_t arrs = {
        initfilebuf (),
        initaliasarr (),
        initmacroarr (),
        initlabelarr (),
        inittokenbufferarray ()
    };

    uint32_t ln = 1;
    uint16_t addr = 0;
    uint32_t origind = 0;
    if (preorig (f, &ln, &arrs)) {
        addr = arrs.filebuf->insbuf[0];
        origind = arrs.tokbufarr->ind;
    } else {
        error (ln, "No origin address declared!\n");
    }
    ++ln;

    char line[MAX_LEN + 1];
    uint8_t end = 0;

    // pass one
    for (uint32_t i = ln ; fgets (line, MAX_LEN + 1, f.fp); i++) {
        TokenBuffer *buf = tokenize (line);
        if (buf->toknum == 0) {								// skip empty lines
            freetokenarr (buf);
            continue;
        }
        uint32_t macro = findmacro (arrs.macro, buf->token[0]);
        if (macro != -1) {
            buf = replacemacro (buf, arrs.macro->arr[macro].replace);
            if (buf == NULL) {
                error (ln, "macro '%s : %s' requires an argument!",
                        arrs.macro->arr[macro].macro->str,
                        arrs.macro->arr[macro].replace->str);
                continue;
            }
        }
        end = passone (i, &addr, buf, &arrs);
        if (end) break;
    }
    if (!end) {
        error (ln, "No end detected!");
    }

    uint32_t p1errs = geterrors ();
    uint32_t p1warns = getwarnings ();
    notify ("%d error(s) and %d warning(s) in pass one\n",
            p1errs, p1warns);

    if (p1errs == 0) {
        addr = arrs.filebuf->insbuf[0];

        uint16_t endofprogram = arrs.tokbufarr->ind - origind;
        for (uint16_t i = origind; endofprogram; ++i, --endofprogram) {
            passtwo (i, &addr, &arrs);
        }
        notify ("%d error(s) and %d warning(s) in pass two\n",
                geterrors() - p1errs, getwarnings() - p1warns);
    } else {
        notify ("Unresolved errors encountered in pass one.\n");
    }

    // write buffers to file
    writeobj (arrs.filebuf, f.obj);
    writehex (arrs.filebuf, f.hex);
    writebin (arrs.filebuf, f.bin);
    writesym (arrs.label, f.sym);
    writelst (arrs.filebuf, f.fp, f.lst);

    freefilebuf (arrs.filebuf);
    freealiasarr (arrs.alias);
    freemacroarr (arrs.macro);
    freelabelarr (arrs.label);
    freetokenbufferarray (arrs.tokbufarr);
    closeasmfiles (&f);
    
    if (geterrors()) clean (file);   

    notify ("Done!\n\n");

    if (geterrors ()) return 1;
    else return 0;
}

/*
    pnode_t: contains everything needed to assemble a single assembly file,
             and used in project-mode assembly to keep everything organized
*/
typedef struct ProjectNode {
    filearr_t out;
    arrs_t res;
    uint32_t origind;
    uint32_t ln;
} pnode_t;

/*
    generateshared: called by project() between pass one and pass two of ALL
            assemble jobs in order to generate a shared symbol table based on
            '.EXPORT' statements

    inputs: a pointer to a shared symbol table
            a pointer to a pnode_t initialized in project() and populated in
            pass one

    outputs: none
*/
void generateshared (labelarr_t* share, pnode_t *node)
{
    uint32_t end = node->res.tokbufarr->ind - 1;
    lineinfo_t **tmp = node->res.tokbufarr->arr;
    for (uint32_t i = 0; end; --end, ++i) {
        Token **tbuf = tmp[i]->buf->token;
        if (strcmp(tbuf[0]->str, ".EXPORT") == 0 ||
            strcmp(tbuf[0]->str, ".export") == 0)
        {
            uint32_t ln = tmp[i]->ln;
            if (tmp[i]->buf->toknum < 2) {
                error (ln, "'%s' requires an argument!", tbuf[0]->str);
                break;
            }

            uint16_t addr = labeladdr (node->res.label, tbuf[1]);

            if (addr) {
                addlabel (share, ln, tbuf[1], addr);
            }
        }
    }
}

/*
    extendprivate: called by project() after call to generate shared between
            pass one and pass two of ALL assemble jobs in order to populate
            each pnode's own resource arrays based on '.IMPORT' statements

    inputs: a pointer to a shared symbol table
            a pointer to a pnode_t initialized in project() and populated in
            pass one

    outputs: none
*/
void extendprivate (labelarr_t* share, pnode_t *node)
{
    uint16_t end = node->res.tokbufarr->ind - 1;
    lineinfo_t **tmp = node->res.tokbufarr->arr;
    for (uint32_t i = 0; end; --end, ++i) {
        Token **tbuf = tmp[i]->buf->token;
        if (strcmp(tbuf[0]->str, ".IMPORT") == 0 ||
            strcmp(tbuf[0]->str, ".import") == 0)
        {
            uint32_t ln = tmp[i]->ln;
            if (tmp[i]->buf->toknum < 2) {
                error (ln, "'%s' requires an argument!", tbuf[0]->str);
                break;
            }

            uint16_t addr = labeladdr (share, tbuf[1]);

            if (addr) {
                addlabel (node->res.label, ln, tbuf[1], addr);
            }
        }
    }
}

/*
    project: assembles a project comprised of multiple files specified in
    an array of strings

    input: filenames as array of strings
    output: 0 if successful, 1 if unsuccessful
*/
int8_t project (char **files)
{
    if (!*files) return 1;		// nothing to assemble

    notify ("Assembling ");
    uint32_t filecount = 0;
    for (; *(files + filecount); filecount++) {
        notify ("\"%s\" ", *(files + filecount));
    }
    notify ("in project mode...\n");

    notify ("Beginning pass one...\n");
    pnode_t *project = (pnode_t*) calloc (filecount, sizeof (pnode_t));
    for (uint32_t i = 0; i < filecount; i++) {
        setcurrentfile (*(files + i));

        if (openasmfiles (&project[i].out, *(files + i))) return 1;
        project[i].res.filebuf = initfilebuf ();
        project[i].res.alias = initaliasarr ();
        project[i].res.macro = initmacroarr ();
        project[i].res.label = initlabelarr ();
        project[i].res.tokbufarr = inittokenbufferarray ();

        project[i].ln = 1;
        uint16_t addr = 0;
        if (preorig (project[i].out, &project[i].ln, &project[i].res)) {
            addr = project[i].res.filebuf->insbuf[0];
            project[i].origind = project[i].res.tokbufarr->ind;
            project[i].ln++;
        } else {
            error (project[i].ln, "No origin address declared!\n");
        }

        char line[MAX_LEN + 1];
        uint8_t end = 0;
        macroarr_t *curmacro = project[i].res.macro;

        for (uint32_t ln = project[i].ln;
             fgets (line, MAX_LEN + 1, project[i].out.fp);
             ++ln)
        {
            TokenBuffer *buf = tokenize (line);
            if (buf->toknum == 0) {							// skip empty lines
                freetokenarr (buf);
                continue;
            }
            uint32_t macro = findmacro (curmacro, buf->token[0]);
            if (macro != -1) {
                buf = replacemacro (buf, curmacro->arr[macro].replace);
                if (buf == NULL) {
                    error (ln, "macro '%s : %s' requires an argument!",
                            curmacro->arr[macro].macro->str,
                            curmacro->arr[macro].replace->str);
                    continue;
                }
            }
            end = passone (ln, &addr, buf, &project[i].res);
            if (end) break;
        }
        if (!end) {
            error (project[i].ln, "no end detected!");
        }
    }

    uint32_t p1errs = geterrors ();
    uint32_t p1warns = getwarnings ();
    notify ("%d error(s) and %d warning(s) in pass one\n",
            p1errs, p1warns);

    if (p1errs == 0) {

        notify ("Generating shared symbol table...\n");
        labelarr_t *shared = initlabelarr ();
        for (uint32_t i = 0; i < filecount; i++) {
            generateshared (shared, &project[i]);
        }

        notify ("Extending private symbol tables...\n");
        for (uint32_t i = 0; i < filecount; i++) {
            extendprivate (shared, &project[i]);
        }
        freelabelarr (shared);

        notify ("Beginning pass two...\n");
        for (uint32_t i = 0; i < filecount; i++) {
            setcurrentfile (*(files + i));

            uint16_t addr = project[i].res.filebuf->insbuf[0];

            uint16_t end = project[i].res.tokbufarr->ind - project[i].origind;
            for (uint16_t j = project[i].origind;
                 end;
                 ++j, --end)
            {
                passtwo (j, &addr, &project[i].res);
            }
        }
        notify ("%d error(s) and %d warning(s) in pass two\n",
                geterrors() - p1errs, getwarnings() - p1warns);
    } else {
        notify ("Unresolved errors encountered in pass one.\n");
    }

    // check for collisions between project object files
    if (!geterrors ()) {
        for (uint32_t i = 0; i < filecount; ++i) {
            char *file1 = *(files + i);
            setcurrentfile (file1);
            filebuf_t *fbuf1 = project[i].res.filebuf;
            uint16_t loaddr1 = fbuf1->insbuf[0];
            uint16_t hiaddr1 = fbuf1->addrbuf[fbuf1->ind - 1];
            for (uint32_t j = i + 1; j < filecount; ++j) {
                char *file2 = *(files + j);
                filebuf_t *fbuf2 = project[j].res.filebuf;
                uint16_t loaddr2 = fbuf2->insbuf[0];
                uint16_t hiaddr2 = fbuf2->addrbuf[fbuf2->ind - 1];

                if ((loaddr1 < loaddr2 && loaddr2 < hiaddr1) ||
                    (loaddr1 < hiaddr2 && hiaddr2 < hiaddr1))
                {
                    error (fbuf1->lnbuf[0],
                            "Collision between \"%s\" and \"%s\"!",
                            file1, file2);
                }
            }
        }
    }
    
    // write buffers to file and clean up
    for (uint32_t i = 0; i < filecount; i++) {
        arrs_t res = project[i].res;
        writeobj (res.filebuf, project[i].out.obj);
        writehex (res.filebuf, project[i].out.hex);
        writebin (res.filebuf, project[i].out.bin);
        writesym (res.label, project[i].out.sym);
        writelst (res.filebuf, project[i].out.fp, project[i].out.lst);
        
        if (geterrors()) clean (*(files + i));

        freefilebuf (res.filebuf);
        freealiasarr (res.alias);
        freemacroarr (res.macro);
        freelabelarr (res.label);
        freetokenbufferarray (res.tokbufarr);
        closeasmfiles (&project[i].out);
    }
    free (project);

    notify ("Done!\n\n");

    if (geterrors ()) return 1;
    else return 0;
}
