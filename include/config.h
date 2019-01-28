#ifndef MAX_LEN

// set to 1 to enable logging in <file>.log file
#define ENABLE_LOGGING 0

// set to 1 to use spaces , 0 to use tabs
#define USE_SPACES_IN_SYM 1

// set to 1 to print binary in list file alongside hex
#define PRINT_BIN_IN_LST 0

// set to 1 to print header comments in lsit file
#define PRESERVE_HEADER 1

// fill in your tabsize to properly format symbol file if using tabs
// 0 is not a valid tabsize
#define TABSIZE 4

// if you are getting read errors, increase these numbers
// (you really shouldn't have to)
#define MAX_WORD_NUM 8		// max # of words per line
#define MAX_WORD_SIZE 512	// max # of chars for a label, filename, or .STRINGZ
#define MAX_LEN 4096			// MAX_WORD_NUM * MAX_WORD_SIZE

#endif