#include "filenameConversion.h"
#include "string.h"
#include "memory.h"
#include "ctype.h"
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

    while(isspace((unsigned char) *str)) str++;

    if(*str == 0)
    {
	return str;
    }
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char) *end))
	end--;
    end[1] = '\0';

    return str;	  
}

void convertToFileDirectoryName(char *filename, const char *FAT_Filename)
{
    strncpy(filename, FAT_Filename, 8);
    filename[8] = '\0';
    trimString(filename);
    for(int i = 0; i < strlen(filename); i++)
    {
		filename[i] = tolower(filename[i]);
    }
}

