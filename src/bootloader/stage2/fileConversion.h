#pragma once

void convertToFATFilename(char *FAT_Filename, const char *name);
void convertToFileDirectoryName(char *filename, const char *FAT_Filename);
void convertToFilename(char *filename, char *extension, const char *FAT_Filename);
