#include "laser.h"

// takes a file pointer and parses each line one by one
void parse_file(FILE *fp){
	char file_buf[max_len+1];
	fpos_t pos;
	while(fgets(file_buf, max_len+1, fp)!=NULL){
		printf("%s", file_buf);
		printf("%c\n", file_buf[0]);
	}
	
}