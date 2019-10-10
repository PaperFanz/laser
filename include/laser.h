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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#include "token.h"

#ifdef USES_ASSEMBLE
    int8_t clean (char *file);

    int8_t assemble (char *file);

    int8_t project (char **files);
#endif

#ifdef USES_ALIAS
    typedef struct Alias {
        uint32_t count;
        uint32_t ln;
        Token *word;
        Token *reg;
    } alias_t;

    typedef struct AliasArr {
        alias_t *arr;
        uint16_t ind;
        uint16_t cap;
    } aliasarr_t;

    void addalias (aliasarr_t *a, uint32_t ln, Token *word, Token *replace);

    int32_t findalias (aliasarr_t *a, Token *word);

    aliasarr_t* initaliasarr (void);

    void freealiasarr (aliasarr_t *a);
#endif

#ifdef USES_MACRO
    typedef struct Macro {
        uint32_t ln;
        Token *macro;
        Token *replace;
        uint32_t count;
    } macro_t;

    typedef struct MacroArr {
        macro_t *arr;
        uint16_t ind;
        uint16_t cap;
    } macroarr_t;

    void addmacro (macroarr_t *m, uint32_t ln, Token *macro, Token *replace);

    int32_t findmacro (macroarr_t *m, Token *macro);

    TokenBuffer* replacemacro (TokenBuffer *buf, Token *replace);

    macroarr_t* initmacroarr (void);

    void freemacroarr (macroarr_t *m);
#endif

#ifdef USES_FLAG
    enum flags {
        VERSION,
        HELP,
        QUIET,
        SILENT,
        ASSEMBLE,
        CLEAN,
        LOG,
        PROJECT,
    };

    const char *about;

    const char *version_num;

    const char *usage;

    const char *help;

    int8_t isclean (void);

    int8_t islogging (void);

    int8_t isproject (void);

    int8_t isassemble (void);

    int8_t checkflags (char *f);

    void setassemble (void);

    void setclean (void);

    void setlog (void);

    void setproject (void);
#endif

#ifdef USES_LABEL
    typedef struct Label {
        uint32_t ln;
        Token *label;
        uint16_t address;
        uint32_t count;
    } label_t;

    typedef struct LabelArr {
        label_t *arr;
        uint16_t ind;
        uint16_t cap;
    } labelarr_t;

    uint8_t isvalidlabel (Token *token);

    void addlabel (labelarr_t *l, uint32_t ln, Token *label, uint16_t addr);

    uint16_t labeladdr (labelarr_t *l, Token *label);

    char* existlabel (labelarr_t *l, uint16_t addr);

    labelarr_t* initlabelarr (void);

    void freelabelarr (labelarr_t *l);

    void writesym (labelarr_t *symarr, FILE *sym);
#endif

#ifdef USES_FILE
    typedef struct Files {
        FILE *fp;
        FILE *sym;
        FILE *bin;
        FILE *hex;
        FILE *obj;
        FILE *lst;
    } filearr_t;

    typedef struct FileBuffer {
        uint16_t *addrbuf;
        uint16_t *insbuf;
        uint32_t *lnbuf;
        uint16_t ind;
        uint16_t cap;
    } filebuf_t;

    int8_t checkextension (char *file, const char *extension);

    void replaceextension (char *file, const char *extension);

    uint8_t openasmfiles (filearr_t *f, char *file);

    void closeasmfiles (filearr_t *f);

    int8_t clean (char *file);

    void writefilebuf (filebuf_t *buf, uint16_t addr, uint16_t ins, uint32_t ln);

    filebuf_t* initfilebuf(void);

    void freefilebuf(filebuf_t *buf);

    void writeobj (filebuf_t *buf, FILE *obj);

    void writehex (filebuf_t *buf, FILE *hex);

    void writebin (filebuf_t *buf, FILE *bin);

    void writelst (filebuf_t *buf, FILE *fp, FILE *lst);
#endif

#ifdef USES_OPERAND
    enum opcodes {BR, ADD, LD, ST,
               JSR, AND, LDR, STR,
               RTI, NOT, LDI, STI,
               JMP, INV, LEA, TRAP,
               TRAPS, JSRR, RET};

    int8_t isregister (Token *token);

    int8_t isbranch (Token *token);

    int8_t istrap (Token *token);

    int8_t isoperand (Token *token);
#endif

#ifdef USES_NOTIFY
    enum verbosity {
        all,
        noWarn,
        noErrs,
    };

    void setVerbosity (int8_t q);

    uint8_t setcurrentlog (char *file);

    void endlog (void);

    void setcurrentfile (char *file);

    void notify (const char *format, ...);

    void warning (uint32_t ln, const char *format, ...);

    void error (uint32_t ln, const char *format, ...);

    void initnotify (void);

    uint32_t getwarnings (void);

    uint32_t geterrors (void);
#endif

#ifdef USES_OFFSET
    uint8_t offtype (Token *token);

    uint16_t offset (int8_t off_type, Token *op);
#endif

#ifdef USES_PSEUDOOP
    enum popcodes {
        ALIAS,
        MACRO,
        ORIG,
        END,
        STRINGZ,
        BLKW,
        FILL,
        EXPORT,
        IMPORT
    };

    int8_t ispseudoop (Token *token);

    uint16_t addrnum (uint8_t popcode, Token *token);
#endif
