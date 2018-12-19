#include "laser.h"

// takes a file pointer and parses each line one by one
void parse_file(FILE *fp){
	char str_buf[max_len+1], word_buf[10][12];
	while(fgets(str_buf, max_len+1, fp)!=NULL){
		if(str_buf[0]!=0x3B){
			int i=0, j=0, k=0;
			bool prev=false;
			memset(word_buf, 0, sizeof(word_buf));
			while(i<=max_len&&str_buf[i]!=0x00&&str_buf[i]!=0x3B){
				if(!isspace(str_buf[i])){
					word_buf[j][k]=str_buf[i];
					k++;
					prev=true;
				}
				else if(isspace(str_buf[i])&&prev){
					word_buf[j][k]=0x00;
					j++;
					k=0;
					prev=false;
				}
				i++;
			}
			for(i=0; i<=9; i++){
				printf("%s ", word_buf[i]);
			}
			printf("\n");
		}
	}	
}