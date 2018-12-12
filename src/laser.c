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
	int bin_test=101010001;
	int bin[16];
	char hex_test[4];
	zext(bin_test, bin, size(bin));

	printf("%i in decimal is: %i\n", bin_test, twoCompToDec(bin, size(bin)));

	int num[16];
	zext(bin_test, num, size(num));
	decToTwoComp(3146, bin, size(bin));
	printIntArr(bin, size(bin));
	binToHex(bin, size(bin), hex_test, size(hex_test));
	printIntArr(num, size(num));
	printCharArr(hex_test, size(hex_test));

//
//	END TESTING
//

	// cleanup
	if(fileOpened) fclose(fp);
	return 0;
}

void printIntArr(int num[], int size){
	int i=0;
	while(i<=size-1){
		printf("%i", num[i]);
		i++;
	}
	printf("\n");
}

void printCharArr(char hex[], int size){
	int i=0;
	while(i<=size-1){
		printf("%c", hex[i]);
		i++;
	}
	printf("\n");
}