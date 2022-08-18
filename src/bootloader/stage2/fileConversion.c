#include "fileConversion.h"
#include "ctype.h"
#include "string.h"
#include "memory.h"
#include "stdio.h"
void convertToFATFilename(char *FAT_Filename, const char *name)
{    
    //convert from name to fat name
    memset(FAT_Filename, ' ', 11);
    FAT_Filename[11] = '\0';
    const char * ext = strchr(name, '.');
    if(ext == NULL)
    {
		ext = name + 11;
    }
    for(int i = 0; i < 8 && name[i] && name + i < ext ; i++)
    {
		FAT_Filename[i] = toupper(name[i]);
    }

    if(ext != name + 11)
    {
		for(int i = 0; i < 3 && ext[i+1]; i++)
		{
			FAT_Filename[i+8] = toupper(ext[i+1]);	    
		}
    }
}

char *trimString(char *str)
{
    char *end;

    // trim leading Space
    /*
    while(isspace((unsigned char) *str)) str++;
    */

    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char) *end))
	end--;
    end[1] = '\0';

    return str;	  
}

void convertToFileDirectoryName(char *filename, const char *FAT_Filename)
{
    memset(filename, ' ', 11);
    strncpy(filename, FAT_Filename, 8);
    filename[7] = '\0';
    trimString(filename);
    for(int i = 0; i < strlen(filename); i++)
    {
		filename[i] = tolower(filename[i]);
    }
}
void convertToFilename(char *filename, char *extension, const char *FAT_Filename)
{
    memset(filename, ' ', 11);
    strncpy(filename, FAT_Filename, 8);
    filename[7] = '\0';
    trimString(filename);       
    strncpy(extension, &(FAT_Filename[8]), 3);    
    for(int i = 0; i < strlen(filename); i++)
    {
		filename[i] = tolower(filename[i]);
    }

    for(int i = 0; i < strlen(extension); i ++)
    {
		extension[i] = tolower(extension[i]);
    }
}
