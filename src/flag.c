#define USES_FLAG
#define USES_FILE
#define USES_NOTIFY
#include "laser.h"

const char *version_num = "2.0.0";

const char *usage = "laser usage:\n"
					"\t'laser -v' to display version number\n"
					"\t'laser -h' to display a more detailed help message\n"
					"\t'laser -a <file>' to assemble a file\n"
					"\t'laser -c <file>' to remove all files generated by 'laser -a <file>'\n";

const char *help = "Description:\n"
					"\tAn assembler for the LC3 assembly language written entirely in C by Pete Fan.\n"
					"Usage:\n"
					"\t-v: 'laser -v' displays version number\n"
					"\t-h: 'laser -h' displays this message again\n"
					"\t-q: 'laser -q' suppresses all warning messages during assembly\n"
					"\t-s: 'laser -s' suppresses all warnings and errors during assembly\n"
					"\t-a: 'laser -a <file>' assembles the specified file\n"
					"\t    'laser -a *.asm' assembles all assembly files in current directory\n"
					"\t-c: 'laser -c <file>' removes all files generated by 'laser -a <file'\n"
					"\t    'laser -c *.asm' removes all files generated by 'laser -a *.asm'\n"
					"Configuration:\n"
					"\tConfiguration options can be found in the 'config.h' header file.\n"
					"\tSee 'https://github.com/PaperFanz/laser' for more details.\n";

const char *flags[] = { "-v", "-h", "-q", "-s", "-a", "-c" };

int8_t checkFlags (char *f)
{
	for (int8_t i = 0; i < 6; i++) {
		if (strcmp (f, flags[i]) == 0)
			return i;
	}

	return -1;
}

int8_t parseFlag (int8_t flag)
{
	switch (flag) {
		case VERSION: {
			printf ("laser version %s\n", version_num);
			break;
		}		
		case HELP: {
			printf ("%s", help);
			break;
		}
		case QUIET: {
			quiet = 1;
			break;
		}
		case SILENT: {
			quiet = 2;
			break;
		}
		case ASSEMBLE: {
			break;
		}
		case CLEAN: {
			break;
		}
		default: {
			return -1;
		}
	}

	return flag;
}