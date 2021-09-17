#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "archive.h"
#include "file.h"

#define ARCHIVE_NAME "archive.bin"

PARCHIVE initialize(){
	
	unsigned char firstchar = 'F';
	unsigned char secondchar = 'A';
	unsigned short multichar = (secondchar << 8) | firstchar;
	
	PARCHIVE archive = malloc(sizeof(ARCHIVE));
	memset(archive,0,sizeof(ARCHIVE));
	
	FILE *fp = fopen(ARCHIVE_NAME,"r+b");
	if(fp == NULL){
		fp = fopen(ARCHIVE_NAME,"w+b");
		if(fp == NULL){
			return NULL;
		}
		
		archive->header.magic = multichar;
		archive->header.version = 1;
		
		if(fwrite(&archive->header,sizeof(ARCHIVE_HEADER),1,fp) < 1){
			printf("아카이브 헤더 쓰기 실패\n");
			fclose(fp);
			return NULL;
		}
	}
	else{
		if(fread(&archive->header,sizeof(ARCHIVE_HEADER),1,fp) < 1){
			printf("아카이브 헤더 읽기 실패\n");
			fclose(fp);
			return NULL;
		}
	}
	
	if(archive->header.magic != multichar){
		printf("아카이브 파일이 아닙니다.\n");
		fclose(fp);
		return NULL;
	}
	
	if(archive->header.version != 1){
		printf("버전이 맞지 않습니다.\n");
		fclose(fp);
		return NULL;
	}
	
	archive->fp = fp;
	
	uint32_t size = getFileSize(fp);
	uint32_t currPos = ftell(fp);
	
	while(size > currPos){
		PFILE_NODE curnode = malloc(sizeof(FILE_NODE));
		memset(curnode,0,sizeof(FILE_NODE));
		
		if(fread(&curnode->desc,sizeof(FILE_DESC),1,fp) < 1){
			printf("아카이브 파일 읽기 실패\n");
			free(curnode);
			finalize(archive);
			return NULL;
		}
		
		curnode->next = archive->fileList.next;
		archive->fileList.next = curnode;
		
		currPos = ftell(fp) + curnode->desc.size;
		fseek(fp,currPos,SEEK_SET);
	}
	
	return archive;
}

void finalize(PARCHIVE archive){
	PFILE_NODE curr = archive->fileList.next;
	while(curr != NULL){
		PFILE_NODE next = curr->next;
		free(curr);
		curr = next;
	}
	
	fclose(archive->fp);
	
	free(archive);
}

bool isExist(PARCHIVE archive, char *filename){
	PFILE_NODE curr = archive->fileList.next;
	while(curr != NULL){
		if(strcmp(curr->desc.name,filename) == 0){
			return true;
		}
		
		curr = curr->next;
	}
	
	return false;
}