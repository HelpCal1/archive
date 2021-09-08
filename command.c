#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "archive.h"
#include "file.h"

int append(PARCHIVE archive, char *filename){
	int ret = 0;
	
	FILE *fp = fopen(filename,"rb");
	if(fp == NULL){
		printf("%s 파일 열기 실패\n",filename);
		return -1;
	}
	
	uint8_t *buffer;
	uint32_t size;
	
	size = getFileSize(fp);
	buffer = malloc(size);
	
	if(fread(buffer,size,1,fp) < 1){
		printf("%s 파일 읽기 실패\n",filename);
		ret = -1;
		goto Error1;
	}
	
	PFILE_DESC desc = malloc(sizeof(FILE_DESC));
	memset(desc,0,sizeof(FILE_DESC));
	strcpy(desc->name,filename);
	desc->size = size;
	
	PFILE_NODE node = archive->fileList.next;
	if(node == NULL){
		fseek(archive->fp,sizeof(ARCHIVE_HEADER),SEEK_SET);
		
		desc->dataOffset = ftell(archive->fp) + sizeof(FILE_DESC);
	}
	else{
		fseek(archive->fp,node->desc.dataOffset + node->desc.size,SEEK_SET);
		
		desc->dataOffset = node->desc.dataOffset + node->desc.size + sizeof(FILE_DESC);
	}
	
	if(fwrite(desc,sizeof(FILE_DESC),1,archive->fp) < 1){
		printf("파일 정보 쓰기 실패\n");
		ret = -1;
		goto Error2;
	}
	
	if(fwrite(buffer,size,1,archive->fp) < 1){
		printf("파일 데이터 쓰기 실패\n");
		ret = -1;
		goto Error2;
	}
	
	printf("%s 파일 추가 성공\n 크기: %d\n",filename,size);
	
Error2:
	free(desc);
	
Error1:
	free(buffer);
	fclose(fp);
	
	return ret;
}

void list(PARCHIVE archive){
	printf("파일 목록:\n");
	
	PFILE_NODE curr = archive->fileList.next;
	while(curr != NULL){
		printf("     %s\n",curr->desc.name);
		
		curr = curr->next;
	}
}

int extract(PARCHIVE archive, char *filename){
	PFILE_NODE curr = archive->fileList.next;
	
	while(curr != NULL){
		if(strcmp(curr->desc.name,filename) == 0){
			int ret = 0;
			uint32_t size = curr->desc.size;
			uint8_t *buffer = malloc(size);
			
			fseek(archive->fp,curr->desc.dataOffset,SEEK_SET);
			
			if(fread(buffer,size,1,archive->fp) < 1){
				printf("아카이브 파일 읽기 실패\n");
				ret = -1;
				goto Error1;
			}
			
			FILE *fp = fopen(filename,"wb");
			if(fp == NULL){
				printf("%s 파일 열기 실패 \n",filename);
				ret = -1;
				goto Error1;
			}
			
			if(fwrite(buffer,size,1,fp) < 1){
				printf("%s 파일 쓰기 실패\n",filename);
				ret = -1;
				goto Error2;
			}
			
			printf("%s 파일 추출 성공\n크기: %d\n",filename,size);
			
		Error2:
			fclose(fp);
			
		Error1:
			free(buffer);
			
			return ret;
		}
		
		curr = curr->next;
	}
	
	return -1;
}