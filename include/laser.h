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
		uint32_t ln;
		Token *word;
		Token *reg;
		uint32_t count;
	} Alias;

	void addalias (Alias **a, uint32_t ln, Token *word, Token *replace);

	Token* findalias (Alias *a, Token *word);

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

	Token* findmacro (Macro *m, Token *macro);

	void freemacroarr (Macro *m);
#endif

#ifdef USES_FLAG
	const char *version_num;

	const char *usage;

	const char *help;

	const char *flags[6];

	enum flags {
		VERSION,
		HELP,
		QUIET,
		SILENT,
		ASSEMBLE,
		CLEAN
	};

	int8_t checkFlags (char *f);

	int8_t parseFlag (int8_t flag);
#endif

#ifdef USES_FILE
	struct Files {
		FILE *asm_;
		FILE *sym_;
		FILE *bin_;
		FILE *hex_;
		FILE *obj_;
		FILE *lst_;
		FILE *log_;
	};

	uint8_t openasmfiles (struct Files *f, char *file);

	void closeasmfiles (struct Files *f);

	int8_t checkextension (char *file, char *extension);

	char* replaceextension (char *file, const char *extension);

	int8_t parsefile (char *file, int8_t last_flag);

	void fprintintarr (FILE *fp, int *num, int size);

	void fprintchararr (FILE *fp, char *hex, int size);

	void printsymbol (FILE *fp, Token *symbol, uint16_t addr);
#endif

#ifdef USES_LABEL
	typedef struct Label {
		uint32_t ln;
		Token *label;
		uint16_t address;
		uint32_t count;
	} Label;

	uint8_t isvalidlabel (char *token);

	Label* addlabel (Label *l, uint32_t ln, Token *label, uint16_t addr);

	uint16_t labeladdr (Label *l, Token *label);

	void freelabelarr (Label *l);
#endif

#ifdef USES_OPERAND
	typedef struct Instruction {
		uint16_t bin;
		uint32_t ln;
		char *line;
	} Instruction;

	void error (int8_t type, FILE *fp, Instruction ins, 
				const char *format, ...);

	int8_t isregister (char *token);

	int8_t isbranch (char *token);

	int8_t istrap (char *token);

	int8_t isoperand (char *token);

	uint8_t operandnum (int8_t operand);
#endif

#ifdef USES_NOTIFY
	enum notify_t {
		WARN = 1,
		ERR
	};

	enum verbosity_t {
		ALL,
		WARNS_ONLY,
		ERRS_ONLY
	};

	void setVerbosity (int8_t q);

	void notify (const char *format, ...);
#endif

#ifdef USES_OFFSET
	uint8_t offtype (char *token);

	uint16_t offset (int8_t off_type, char *op);
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

	int8_t ispseudoop (char *token);

	uint8_t poperandnum (uint8_t popcode);

	uint16_t addrnum (uint8_t popcode, char *token);
#endif
