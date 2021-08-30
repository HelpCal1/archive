#define _CRE_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#pragma pack(push,1)
typedef struct _ARCHIVE_HEADER{
	uint16_t magic;
	uint16_t version;
} ARCHIVE_HEADER, *PARCHIVE_HEADER;

typedef struct _FILE_DESC {
	char		name[256];
	uint32_t	size;
	uint32_t	dataOffset;
}FILE_DESC, *PFILE_DESC;

#pragma pack(pop)

typedef struct _FILE_NODE{
	struct _FILE_NODE *next;
	FILE_DESC desc;
}FILE_NODE, *PFILE_NODE;

typedef struct _ARCHIVE{
	ARCHIVE_HEADER header;
	FILE *fp;
	FILE_NODE fileList;
}ARCHIVE, *PARCHIVE;

#define ARCHIVE_NAME "archive.bin"

uint32_t getFileSize(FILE *fp){
	uint32_t size;
	uint32_t currPos = ftell(fp);
	
	fseek(fp,0,SEEK_END);
	size = ftell(fp);
	
	fseek(fp,currPos,SEEK_SET);
	
	return size;
}

int append(PARCHIVE archive, char *filename){
	int ret = 0;
	
	FILE *fp = fopen(filename,"rb");
	if(fp == NULL){
		printf("%s 파일이 없습니다.\n",filename);
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
	
	fseek(archive->fp,sizeof(ARCHIVE_HEADER),SEEK_SET);
	
	desc->dataOffset = ftell(archive->fp) + sizeof(FILE_DESC);
	
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
	
	printf("%s 파일 추가 성공\n크기: %d\n",filename,size);
	
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

int main(){
	PARCHIVE archive = malloc(sizeof(ARCHIVE));
	memset(archive,0,sizeof(ARCHIVE));
	
	unsigned char firstchar = 'F';
	unsigned char secondchar = 'A';
	unsigned short multichar = (secondchar << 8) | firstchar;
	
	PFILE_NODE curr;
	
	FILE *fp = fopen(ARCHIVE_NAME,"r+b");
	
	if(fp == NULL){
		fp = fopen(ARCHIVE_NAME,"w+b");
		if(fp == NULL){
			return -1;
		}
		
		archive->header.magic = multichar;
		archive->header.version = 1;
		
		if(fwrite(&archive->header,sizeof(ARCHIVE_HEADER),1,fp) < 1){
			printf("아카이브 헤더 쓰기 실패\n");
			fclose(fp);
			return -1;
		}
	}
	else{
		if(fread(&archive->header,sizeof(ARCHIVE_HEADER),1,fp) < 1){
			printf("아카이브 헤더읽기 실패\n");
			fclose(fp);
			return -1;
		}
	}
	
	if(archive->header.magic != multichar){
		printf("아카이브 파일이 아닙니다.\n");
		fclose(fp);
		return -1;
	}
	
	if(archive->header.version != 1){
		printf("버전이 맞지 않습니다.\n");
		fclose(fp);
		return -1;
	}
	
	archive->fp = fp;
	
	int ret = 0;
	uint32_t size = getFileSize(fp);
	uint32_t currPos = ftell(fp);
	
	while(size > currPos){
		PFILE_NODE node = malloc(sizeof(FILE_NODE));
		memset(node,0,sizeof(FILE_NODE));
		
		if(fread(&node->desc,sizeof(FILE_DESC),1,fp) < 1){
			printf("아카이브 파일 읽기 실패\n");
			free(node);
			ret = -1;
			goto FINALIZE;
		}
		
		node->next = archive->fileList.next;
		archive->fileList.next = node;
		currPos = ftell(fp) + node->desc.size;
		fseek(fp,currPos,SEEK_SET);
	}
	
	extract(archive,"hello.txt");

FINALIZE:
	curr = archive->fileList.next;
	while(curr != NULL){
		PFILE_NODE next = curr->next;
		free(curr);
		curr = next;
	}
	
	fclose(archive->fp);
	
	free(archive);
	
	return ret;
}