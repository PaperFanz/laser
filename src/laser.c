// includes
#include "laser.h"
#include "str_const.h"

int main(int argc, char *argv[]){
	
//==============================================================================
//	Declarations
//==============================================================================

	FILE *fp;
	bool fileOpened=false;
	bool assemble=false;
	
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
			else if(strcmp(argv[i], "-c")==0){
				i++;
				while(i<argc){
					char fname[MAX_WORD_SIZE+5];
					strcpy(fname, argv[i]);
					char fname_buf[MAX_WORD_SIZE+5];
					for(int j=0; j<=MAX_WORD_SIZE; j++){
						if(fname[j]==0x2E){
							fname_buf[j]=0x2E;			// .
							fname_buf[j+1]=0x73;		// s
							fname_buf[j+2]=0x79;		// y
							fname_buf[j+3]=0x6D;		// m
							fname_buf[j+4]=0x00;
							printf("Deleting %s!\n", fname_buf);
							if(remove(fname_buf)!=0){
								printf("Unable to delete %s!\n", fname_buf);
							}		// remove filename.sym
							fname_buf[j+1]=0x6C;		// l
							fname_buf[j+2]=0x73;		// s
							fname_buf[j+3]=0x74;		// t
							printf("Deleting %s!\n", fname_buf);
							if(remove(fname_buf)!=0){
								printf("Unable to delete %s!\n", fname_buf);
							}		// remove filename.lst
							fname_buf[j+1]=0x62;		// b
							fname_buf[j+2]=0x69;		// i
							fname_buf[j+3]=0x6E;		// n
							printf("Deleting %s!\n", fname_buf);
							if(remove(fname_buf)!=0){
								printf("Unable to delete %s!\n", fname_buf);
							}		// remove filename.bin
							fname_buf[j+1]=0x68;		// h
							fname_buf[j+2]=0x65;		// e
							fname_buf[j+3]=0x78;		// x
							printf("Deleting %s!\n", fname_buf);
							if(remove(fname_buf)!=0){
								printf("Unable to delete %s!\n", fname_buf);
							}		// remove filename.hex
							fname_buf[j+1]=0x6F;		// o
							fname_buf[j+2]=0x62;		// b
							fname_buf[j+3]=0x6A;		// j
							printf("Deleting %s!\n", fname_buf);
							if(remove(fname_buf)!=0){
								printf("Unable to delete %s!\n", fname_buf);
							}		// remove filename.obj
						}
						else{
							fname_buf[j]=fname[j];
						}
					}
					i++;
				}
			}
			else if(strcmp(argv[i],"-a")==0){
				i++;
				while(i<argc){
					if(checkExt(argv[i], ".asm")){
						fp=fopen(argv[i], "r+");
						if(fp!=NULL){
							fileOpened=true;
							parse_file(fp, argv[i]);
						}
						else{
							printf("Unable to open file: %s\n", argv[i]);
						}
					}
					else{
						printf("Invalid file extension: %s\n", strrchr(argv[i], '.'));
						break;
					}
					i++;
				}
			}
			else{
				printf("Type 'laser -h' for help.\n");
				break;
			}
		}
	}

//
//	TESTING FUNCTIONS
//
	// printf("%d\n", hexToDec("xFA"));
	// int bin_test=101010001;
	// int bin[16];
	// char hex[4];
	// zext(bin_test, bin, size(bin));

	// printf("%i in decimal is: %i\n", bin_test, twoCompToDec(bin, size(bin)));

	// int num[16];
	// zext(bin_test, num, size(num));
	// decToTwoComp(-10, bin, size(bin));
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
