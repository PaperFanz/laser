#define USES_FLAG
#define USES_FILE
#define USES_NOTIFY
#define USES_OPERAND
#include "laser.h"

int8_t main (int argc, char **argv)
{
	int8_t last_flag = -1;
	quiet = 0;

	for (int i = 1; i < argc; i++) {
		char *file = argv[i];
		int8_t flag = checkFlags (file);

		if (flag >= 0) {
			last_flag = parseFlag (flag);
		} else {
			last_flag = parsefile (file, last_flag);
		}

		if (last_flag == -1) {
			notify ("Invalid flags!\n");
		} else if (last_flag == -2) {
			notify ("Parse error!\n");
			return 1;
		}
	}

	return 0;
}