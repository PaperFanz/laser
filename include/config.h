#ifndef MAX_WORD_NUM

// set to 1 to use spaces , 0 to use tabs
#define USE_SPACES_IN_SYM 1

// set to 1 to print binary in list file alongside hex
#define PRINT_BIN_IN_LST 0

// fill in your tabsize to properly format symbol file if using tabs
// 0 is not a valid tabsize
#define TABSIZE 4

// if you are getting read errors, increase these numbers
// (you really shouldn't have to)
#define MAX_WORD_NUM 6		// max # of words per line
#define MAX_WORD_SIZE 60	// max # of chars for a label, filename, or .STRINGZ
#define MAX_LEN 360			// MAX_WORD_NUM * MAX_WORD_SIZE

#endif