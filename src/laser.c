#include "laser.h"
#include "str_const.h"

int main(int argc, char *argv[]){
	
	FILE *fp;
	bool fileOpened = false;
	
	// parse arguments
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

//
//	TESTING FUNCTIONS
//

	int bin[24];
	zext(101101, bin, size_arr(bin));

	printf("1000000 in decimal is: %i\n", twoCompToDec(bin, size_arr(bin)));

	int num[16];
	int size=size_arr(bin);
	zext(10011, num, size);
	decToTwoComp(24, bin, size_arr(bin));
	int i=0;
	while(i<=size-1){
		//printf("%d", num[i]);
		printf("%d", bin[i]);
		i++;
	}
	printf("\n");

//
//	END TESTING
//

	// cleanup
	if(fileOpened) fclose(fp);
	return 0;
}