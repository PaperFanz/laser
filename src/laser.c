// includes
#include "laser.h"
#include "str_const.h"

int main(int argc, char *argv[]){
	
//==============================================================================
//	Declarations
//==============================================================================

	FILE *fp;
	bool fileOpened = false;
	
//==============================================================================
//	Parse Arguments
//==============================================================================

	if(argc==1){
		printf("%s: type 'laser -h' for help\n", argv[0]);
	}
	else{
		int i=1;
		for(i=1; i<=argc-1; i++){
			if(strcmp(argv[i],"-v")==0){
				printf("laser is at version %s\n", version_num);
			}
			else if(strcmp(argv[i],"-h")==0){
				printf("%s", help);
			}
			else if((strcmp(argv[i],"-a")==0)&&(argc>=i+1)){
				fp=fopen(argv[i+1], "r+");
				if(fp!=NULL){
					fileOpened=true;
					parse_file(fp, argv[i+1]);
				}
				else{
					printf("Unable to open specified file: %s\n", argv[i+1]);
				}
				i++;	// skip over file name in argv
			}
			else{
				printf("Invalid flags!\n");
			}
		}
	}

//
//	TESTING FUNCTIONS
//
	// int bin_test=101010001;
	// int bin[16];
	// char hex[4];
	// zext(bin_test, bin, size(bin));

	// printf("%i in decimal is: %i\n", bin_test, twoCompToDec(bin, size(bin)));

	// int num[16];
	// zext(bin_test, num, size(num));
	// decToTwoComp(36417, bin, size(bin));
	// printIntArr(bin, size(bin));
	// binToHex(bin, size(bin), hex, size(hex));
	// printIntArr(num, size(num));
	// printCharArr(hex, size(hex));
	// printf("%d", isKeyword("gyu"));

//
//	END TESTING
//

	if(fileOpened) fclose(fp);		// clean up
	return 0;
}
