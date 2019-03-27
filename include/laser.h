#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

#include "config.h"
#include "token.h"

#ifdef USES_ASSEMBLE
	int8_t clean (char *file);

	int8_t assemble (char *file);
#endif

#ifdef USES_ALIAS
	typedef struct Alias {
		uint32_t count;
		uint32_t ln;
		Token *word;
		Token *reg;
	} Alias;

	void addalias (Alias *a, uint32_t ln, Token *word, Token *replace);

	uint32_t findalias (Alias *a, Token *word);

	Alias* initaliasarr (void);

	void freealiasarr (Alias *a);
#endif

#ifdef USES_MACRO
	typedef struct Macro {
		uint32_t ln;
		Token *macro;
		Token *replace;
		uint32_t count;
	} Macro;

	Macro* addmacro (Macro *m, uint32_t ln, Token *macro, Token *replace);

	uint32_t findmacro (Macro *m, Token *macro);

	Macro* initmacroarr (void);

	void freemacroarr (Macro *m);
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
		HIPERF,
	};

	const char *about;

	const char *version_num;

	const char *usage;

	const char *help;

	int8_t islogging (void);

	int8_t ishiperf (void);

	int8_t checkflags (char *f);

	int8_t parseflag (int8_t flag);
#endif

#ifdef USES_LABEL
	typedef struct Label {
		uint32_t ln;
		Token *label;
		uint16_t address;
		uint32_t count;
	} Label;

	uint8_t isvalidlabel (Token *token);

	Label* addlabel (Label *l, uint32_t ln, Token *label, uint16_t addr);

	uint16_t labeladdr (Label *l, Token *label);

	Label* initlabelarr (void);

	void freelabelarr (Label *l);
#endif

#ifdef USES_FILE
	typedef struct Files {
		FILE *fp;
		FILE *sym;
		FILE *bin;
		FILE *hex;
		FILE *obj;
		FILE *lst;
	} Files;

	typedef struct iframe {
		uint16_t ins;
		uint32_t ln;
	} iframe_t;

	int8_t checkextension (char *file, char *extension);

	char* replaceextension (char *file, const char *extension);

	uint8_t openasmfiles (struct Files *f, char *file);

	void closeasmfiles (struct Files *f);

	FILE* getlog (void);

	int8_t parsefile (char *file, int8_t last_flag);

	void printsymbol (FILE *fp, Token *symbol, uint16_t addr);

	void writefilebuf (uint16_t ins, uint32_t ln);

	void resetfilebuf ();

	void writeobj (FILE *obj);

	void writehex (FILE *hex);

	void writebin (FILE *bin);

	void writelst (FILE *fp, FILE *lst);
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

	void notify (const char *format, ...);

	void warning (uint32_t ln, const char *format, ...);

	void error (uint32_t ln, const char *format, ...);

	uint32_t getwarnings (void);

	uint32_t geterrors (void);
#endif

#ifdef USES_OFFSET
	uint8_t offtype (Token *token);

	uint16_t offset (int8_t off_type, Token *op);
#endif

#ifdef USES_PSEUDOOP
	enum pseudoops {
		ALIAS,
		MACRO,
		ORIG,
		END,
		STRINGZ,
		BLKW,
		FILL
	};

	int8_t ispseudoop (Token *token);

	uint16_t addrnum (uint8_t popcode, Token *token);
#endif
