#include "laser.h"

int main(int argc, char *argv[]){
	
	FILE *fp;
	bool fileOpened = false;
	
	// parse arguments
	parse_args(argc, argv, fp, fileOpened);

	printf("64 in binary is: %u\n", decToTwoComp(-24));

	int bin[16];
	zext(101101, bin, size_arr(bin));

	printf("1000000 in decimal is: %i\n", twoCompToDec(bin, size_arr(bin)));

	int num[32];
	int size=size_arr(num);
	zext(10011, num, size);
	int i=0;
	while(i<=size-1){
		printf("%d", num[i]);
		i++;
	}
	printf("\n");
	
	
	// parse .asm file
	if(fileOpened){
		
	}

	// cleanup
	if(fileOpened) fclose(fp);
	return 0;
}