#include "laser.h"

int main(int argc, char *argv[]){
	
	FILE *fp;
	bool fileOpened = false;
	
	// parse arguments
	parse_args(argc, argv, fp, fileOpened);
	
	// parse .asm file
	if(fileOpened){
		
	}

	// cleanup
	if(fileOpened) fclose(fp);
	return 0;
}