#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

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

PARCHIVE initialize();
void finalize(PARCHIVE archive);
bool isExist(PARCHIVE archive, char *filename);

#endif