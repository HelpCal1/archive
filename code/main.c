#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "archive.h"
#include "command.h"

int main(int argc, char *argv[]){
	if(argc == 1){
		printf("filearchive <명령> <파일명>\n\n");
		printf("명령:\n");
		printf("	append 파일 추가\n");
		printf("	list 파일 목록 출력\n");
		printf("	extract 파일 추출\n");
		return 0;
	}
	
	PARCHIVE archive = initialize();
	if(archive == NULL){
		return -1;
	}
	
	char *command = argv[1];
	char *filename = argv[2];
	
	if(strcmp(command, "append") == 0){
		if(!isExist(archive,filename)){
			if(append(archive,filename) == -1){
				printf("%s 파일 추가 실패\n",filename);
			}
		}
		else{
			printf("이미 %s 파일이 추가되어 있습니다.\n",filename);
		}
	}
	else if(strcmp(command,"list") == 0){
		list(archive);
	}
	else if(strcmp(command, "extract") == 0){
		if(isExist(archive,filename)){
			if(extract(archive,filename) == -1){
				printf("%s 파일 추출 실패\n",filename);
			}
		}
		else{
			printf("%s 파일이 없습니다.\n",filename);
		}
	}
	
	finalize(archive);
	
	return 0;

}