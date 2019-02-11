#ifndef MAX_LEN

// set to 1 to enable logging in <file>.log file
#define ENABLE_LOGGING 0

// set to 1 to use spaces, 0 to use tabs
#define USE_SPACES_IN_SYM 1

// set to 1 to print binary in lst file alongside hex
#define PRINT_BIN_IN_LST 0

/*
	fill in your tabsize to properly format symbol file if using tabs
	0 is not a valid tabsize
*/
#define TABSIZE 4

/*
	if you are getting read errors, increase these numbers
	(you really shouldn't have to)
*/
#define MAX_WORD_NUM 6		// max # of words per line
#define MAX_WORD_SIZE 1024	// max # of chars for a label, filename, or .STRINGZ
#define MAX_LEN 6144		// MAX_WORD_NUM * MAX_WORD_SIZE

/*
	reasonable defaults for number of declarables (saves some reallocs)
	these numbers should never be less than 1
*/
#define DEFAULT_ALIAS_NUM 8
#define DEFAULT_MACRO_NUM 8
#define DEFAULT_LABEL_NUM 8

#endif