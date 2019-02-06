#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>

#include "config.h"

#ifdef USES_ASSEMBLE
	struct Symbol {
		uint32_t ln;
		char label[MAX_WORD_SIZE+2];
		uint16_t addr;
		uint16_t count;
	};

	int8_t clean (char *file);

	int8_t assemble (char *file);
#endif

#ifdef USES_ALIAS
	struct Alias {
		uint32_t ln;
		char word[MAX_WORD_SIZE + 2];
		char replace[MAX_WORD_SIZE + 2];
		uint32_t count;
	};
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

	enum verbosity {
		NORMAL_V,
		QUIET_V,
		SILENT_V
	};

	int8_t checkFlags (char *f);

	int8_t parseFlag (int8_t flag);
#endif

#ifdef USES_FILE
	int8_t checkExtension (char *file, char *extension);

	char* replaceExtension (char *file, const char *extension);

	int8_t parseFile (char *file, int8_t last_flag);

	void fprintIntArr (FILE *fp, int num[], int size);

	void fprintCharArr (FILE *fp, char hex[], int size);

	void printSymbol (FILE *fp, char symbol[], char addr[]);
#endif

#ifdef USES_LABEL
	struct Label {
		uint32_t ln;
		char label[MAX_WORD_SIZE + 2];
		uint16_t address;
		uint32_t count;
	}
#endif

#ifdef USES_NOTIFY
	enum notify_t {
		WARN = 1,
		ERR = 2
	};

	int8_t quiet;

	void notify (const char *format, ...);
#endif

#ifdef USES_OPERAND
	
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
#endif

#ifdef USES_TOKENIZE
	void tokenize (char *line, char token_buf[MAX_WORD_NUM][MAX_WORD_SIZE]);
#endif
