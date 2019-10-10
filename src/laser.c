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

#define USES_FLAG
#define USES_FILE
#define USES_NOTIFY
#define USES_ASSEMBLE
#include "laser.h"

int main (int argc, char **argv)
{
    int8_t err = 0;

    if (argc == 1) printf ("%s\n\n%s", usage, about);

    ++argv;	// skip past first argument 'laser'
    int8_t flag = 0;

    /*
        stop parsing flags once an assemble/project/clean flag is
        encountered or the end of the argument vector is reached and
        assume that all remaining elements are either null or files for
        assembly
    */
    for (; *argv &&
            flag != ASSEMBLE &&
            flag != PROJECT &&
            flag != CLEAN &&
            flag != -1;
            argv++)
    {
        flag = checkflags (*argv);

        switch (flag) {
        case VERSION:
        printf ("laser version %s\n", version_num);
        break;
        case HELP:
        printf ("%s", help);
        break;
        case QUIET:
        setVerbosity (noWarn);		// notify.c
        break;
        case SILENT:
        setVerbosity (noErrs);		// notify.c
        break;
        case ASSEMBLE:
        setassemble ();				// flag.c
        break;
        case CLEAN:
        setclean ();				// flag.c
        break;
        case LOG:
        if (*(argv + 1)) {
            if (checkflags (*(++argv)) == -1) {
                if (!setcurrentlog (*argv)) setlog ();
            }
            else notify ("Please pass a valid log file after '-l'\n");
        }
        break;
        case PROJECT:
        setproject ();				// flag.c
        break;
        default:
            break;
        }
    }

    /*
        keep track of how long files take to assemble, 
        no need to track how long it takes to parse flags	
    */
    clock_t start, end;
    start = clock ();
    uint32_t jobs = 0, failed = 0;

    if (isproject ()) {
        uint8_t validproject = 1;
        for (uint32_t i = 0; *(argv + i); i++) {
            if (!checkextension (*(argv + i), ".asm")) {
                notify ("'%s' is not an assembly file, exiting...\n",
                        *(argv + i));
                validproject = 0;
            }
        }

        if (validproject) {
            failed += project (argv);
            jobs++;
        } else {
            jobs = 0;
        }
    } else if (isassemble ()) {
        for (; *argv; argv++) {
            if (checkextension (*argv, ".asm")) {
                failed += assemble (*argv);
                jobs++;
            } else {
                notify ("'%s' is not an assembly file, continuing...\n",
                        *argv);
            }
        }
    } else if (isclean ()) {
        for (; *argv; argv++) {
            if (checkextension (*argv, ".asm")) {
                failed += clean (*argv);
                jobs++;
            } else {
                notify ("'%s' is not an assembly file, continuing...\n",
                        *argv);
            }
        }
    } else {
        for (; *argv; argv++) {
            notify ("'%s' is not a valid option!\n", *argv);
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
        // notify ("%d %s completed in %f\n", jobs, s, cpu_time);
        notify ("%d %s processed in %02.0f:%02.0f:%03.0f\n",
                jobs, s, mins, secs, msecs);
        notify ("%d successful, %d failed\n", jobs - failed, failed);
    }

    if (islogging ()) endlog ();

    if (failed) err = 1;
    return err;
}