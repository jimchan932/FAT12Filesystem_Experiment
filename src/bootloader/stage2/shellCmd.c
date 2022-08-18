#include "shellCmd.h"
#include "fileConversion.h"
#include "fat.h"
#include "stdio.h"
#include "string.h"
#include "memory.h"

void command_ls(DISK *disk, const char *currentWorkingDirectory)
{
    FAT_File far *fd = FAT_Open(disk, currentWorkingDirectory);

    FAT_DirectoryEntry entry;
    int i = 0;
    while(FAT_ReadEntry(disk, fd, &entry) && i++ < 5)
    {	
		char filename[11];
		char extension[3];
	
		if(entry.Name[0] == 'N' && entry.Name[1] == 'B' && entry.Name[2] == 'O' && entry.Name[3] == 'S')
		{	    
			strcpy(filename, entry.Name);
			printf("%s", filename);
		}		  
		else if((entry.Attributes & 0x10))
		{
			convertToFileDirectoryName(filename, entry.Name);
			printf("%s", filename);
		}
		else
		{ 
			convertToFilename(filename, extension, entry.Name);
			printf("%s.%s", filename, extension);	    
		}    
    	printf("    ");	
    }

    FAT_Close(fd);
    printf("\r\n");
}

bool command_cd(DISK *disk, char *currentDirectoryPath, char *directoryName)
{
    FAT_File far *fd = FAT_Open(disk, currentDirectoryPath);
    FAT_DirectoryEntry entry;
    if(FAT_FindFile(disk, fd, directoryName, &entry))
    {
		// not a directory
		if((entry.Attributes & 0x10) == 0)
		{
			printf("Error: The directory name is invalid\n");
			FAT_Close(fd);
			printf("\r\n");
			return false;
		}
		// is a directory

		if(!(strcmp(currentDirectoryPath, "/")== 0)) // not root directory
		{
			strcat(currentDirectoryPath, "/");
		}
		strcat(currentDirectoryPath, directoryName);	
		FAT_Close(fd);
		printf("\r\n");
		return true;
	}
	else
	{
		printf("The system cannot find the path specified\n");
		FAT_Close(fd);
		printf("\r\n");
		return false;
	}
} 

void command_cat(DISK *disk, char *currentDirectoryPath, char *directoryName)
{
    
    char fullFilePath[256];
    strcpy(fullFilePath, currentDirectoryPath);

    if(strcmp(currentDirectoryPath, "/") != 0) // not root directory
    {
		strcat(fullFilePath, "/");
    }
    
    strcat(fullFilePath, directoryName);
        
    char buffer[100];
    uint32_t read;
    FAT_File far *fd = FAT_Open(disk, fullFilePath);
    while((read = FAT_Read(disk, fd, sizeof(buffer), buffer)))
    {
		for(uint32_t i = 0; i < read; i++)
		{	    
			if(buffer[i] == '\n')
			putc('\r');
			putc(buffer[i]);	    
		}
    }
    FAT_Close(fd);
    printf("\r\n");
}

void command_echo(DISK *disk, char *currentDirectoryPath, char *filename, char *buffer)
{
    /*
    FAT_File far * fd = FAT_Open(disk, currentDirectoryPath);    
    FAT_updateDirEntry(disk, fd, strlen(buffer) + 1, filename);
    FAT_Close(fd);
    */
    
    char fullFilePath[256];
    strcpy(fullFilePath, currentDirectoryPath);
    if(!(strcmp(currentDirectoryPath, "/") == 0)) // not root directory
    {
		strcat(currentDirectoryPath, "/");
    }
    strcat(fullFilePath, filename);
 
    FAT_File far * fd = FAT_Open(disk, fullFilePath);
    FAT_Write(disk, fd, buffer);
    FAT_Close(fd);
    printf("\r\n");
}

