// includes
#include "laser.h"

int main (int argc, char *argv[]) {
	
//==============================================================================
//	Declarations
//==============================================================================

	FILE *fp;
	
//==============================================================================
//	Parse Arguments
//==============================================================================

	if (argc == 1)
		printf("%s: type 'laser -h' for help\n", argv[0]);

	for (int i = 1; i <= argc - 1; i++) {
		if (strcmp (argv[i], "-v") == 0) {
			printf ("laser is at version %s\n", version_num);
		} else if (strcmp (argv[i], "-h") == 0) {
			printf ("%s", help);
		} else if (strcmp (argv[i], "-c") == 0) {
			i++;
			while (i < argc) {
				char fname[MAX_WORD_SIZE+5];
				strcpy (fname, argv[i]);
				char fname_buf[MAX_WORD_SIZE+5];
				for (int j = 0; j <= MAX_WORD_SIZE; j++) {
					if (fname[j] == '.') {
						fname_buf[j] = '.';			// .
						fname_buf[j + 1] = 's';		// s
						fname_buf[j + 2] = 'y';		// y
						fname_buf[j + 3] = 'm';		// m
						fname_buf[j + 4] = '\0';
						printf("Deleting %s!\n", fname_buf);
						if(remove (fname_buf))
							printf ("Unable to delete %s!\n", fname_buf);
						fname_buf[j + 1] = 'l';		// l
						fname_buf[j + 2] = 's';		// s
						fname_buf[j + 3] = 't';		// t
						printf("Deleting %s!\n", fname_buf);
						if(remove (fname_buf))
							printf ("Unable to delete %s!\n", fname_buf);
						fname_buf[j + 1] = 'b';		// b
						fname_buf[j + 2] = 'i';		// i
						fname_buf[j + 3] = 'n';		// n
						printf("Deleting %s!\n", fname_buf);
						if(remove (fname_buf))
							printf ("Unable to delete %s!\n", fname_buf);
						fname_buf[j + 1] = 'h';		// h
						fname_buf[j + 2] = 'e';		// e
						fname_buf[j + 3] = 'x';		// x
						printf("Deleting %s!\n", fname_buf);
						if(remove (fname_buf))
							printf ("Unable to delete %s!\n", fname_buf);
						fname_buf[j + 1] = 'o';		// o
						fname_buf[j + 2] = 'b';		// b
						fname_buf[j + 3] = 'j';		// j
						printf("Deleting %s!\n", fname_buf);
						if(remove (fname_buf))
							printf ("Unable to delete %s!\n", fname_buf);
					} else {
						fname_buf[j] = fname[j];
					}
				}
				i++;
			}
		} else if (strcmp (argv[i], "-a") == 0) {
			i++;
			while (i < argc) {
				if (checkExt (argv[i], ".asm")) {
					fp = fopen (argv[i], "r+");
					if (fp != NULL)
						parse_file (fp, argv[i]);
					else
						printf ("Unable to open file: %s\n", argv[i]);
				} else {
					printf ("Invalid file extension: %s\n", strrchr (argv[i], '.'));
					break;
				}
				i++;
			}
		} else {
			printf ("Type 'laser -h' for help.\n");
			break;
		}
	}

	if (fp != NULL)
		fclose(fp);
	return 0;
}
