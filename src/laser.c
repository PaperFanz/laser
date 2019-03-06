#define USES_FLAG
#define USES_FILE
#define USES_NOTIFY
#define USES_OPERAND
#include "laser.h"

int8_t main (int argc, char **argv)
{
	int8_t last_flag = -1, err = -1;
	uint32_t jobs = 0;
	clock_t start, end;
	setVerbosity(ALL);

	start = clock ();
	for (uint32_t i = 1; i < argc; i++) {
		char *file = argv[i];
		int8_t flag = checkFlags (file);

		if (flag >= 0) {
			err = last_flag = parseFlag (flag);
		} else {
			err = parsefile (file, last_flag);
			jobs++;
		}

		if (err == -1) {
			notify ("Invalid flags!\n");
		} else if (err == -2) {
			notify ("Parse error!\n");
			return 1;
		}
	}
	end = clock ();
	if (jobs > 0) {
		double cpu_time = (double) (end - start) / CLOCKS_PER_SEC;
		double mins = cpu_time / 60;
		mins = (mins >= 1 ? mins : 0);
		double secs = cpu_time - ((uint64_t) mins * 60);
		secs = (secs >= 1 ? secs : 0);
		double msecs = (cpu_time - (uint64_t) secs - (60 * (uint64_t) mins)) * 1000;
		msecs = (msecs >= 1 ? msecs : 0);

		char *s = "jobs";
		if (jobs == 1) s = "job";
		notify ("%d %s completed in %02.0f:%02.0f:%03.0f\n", jobs, s, mins, secs, msecs);
	}


	return 0;
}