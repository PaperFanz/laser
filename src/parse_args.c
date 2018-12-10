#include "laser.h"
#include "parse_args.h"

void parse_args(int argc, char *argv[], FILE *fp, bool fileOpened){
	if(argc==1){
		printf("%s: type 'laser -h' for help\n", argv[0]);
	}
	else if((strcmp(argv[1],"-v")==0) || (strcmp(argv[1],"--version")==0)){
		printf("laser is at version %s\n", version_num);
	}
	else if((strcmp(argv[1],"-h")==0) || (strcmp(argv[1],"--help")==0)){
		printf("%s", help);
	}
	else if(((strcmp(argv[1],"-f")==0) || (strcmp(argv[1],"--file"))==0) && argc==3){
		printf("filename: %s\n", argv[2]);
		fp = fopen(argv[2],"r+");
		if(fp!=NULL){
			fputs("write successful\n", fp);
			fileOpened = true;
		}
		else{
			printf("unable to open %s\n", argv[2]);
		}
	}
	else{
		printf("invalid flags\n");
	}
}