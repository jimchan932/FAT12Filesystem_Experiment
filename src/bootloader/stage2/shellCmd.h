#ifndef __SHELL_COMMANDS_
#define __SHELL_COMMANDS_

#include "disk.h"
#include "fat.h"
#include "stdint.h"


void command_ls(DISK *disk, const char *currentWorkingDirectory);
bool command_cd(DISK *disk, char *currentDirectoryPath, char *directoryName);
void command_cat(DISK *disk, char *currentDirectoryPath, char *directoryName);

void command_echo(DISK *disk, char *currentDirectoryPath, char *filename, char *buffer);
void test(DISK *disk, char *fullFilePath, char *buffer);
#endif
