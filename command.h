#ifndef COMMAMD_H
#define COMMAND_H

#include "archive.h"

int append(PARCHIVE archive, char *filename);
void list(PARCHIVE archive);
int extract(PARCHIVE archive, char *filename);

#endif