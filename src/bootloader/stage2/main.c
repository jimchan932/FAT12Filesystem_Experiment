#include "stdint.h"
#include "stdio.h"
#include "memory.h"
#include "disk.h"
#include "fat.h"
#include "x86.h"
#include "string.h"
#include "shellCmd.h"
#include "fileConversion.h"
char currentWorkingDirectoryPath[256] = "/";

void printLocation()
{
    printf("%s>", currentWorkingDirectoryPath);
}

void test1(DISK *disk)
{
    
    const char *testStr = "new data from test file \0";
    printLocation();
    printf("cat test.txt\r\n");
    command_cat(disk,  currentWorkingDirectoryPath, "test.txt");
    printLocation();
    printf("echo \"%s\" > test.txt\r\n", testStr);    
    command_echo(disk, currentWorkingDirectoryPath, "test.txt", testStr);
    printLocation();
    printf("cat test.txt\r\n");
    command_cat(disk,  currentWorkingDirectoryPath, "test.txt");
}

void test2(DISK *disk)
{
    printLocation();
    printf("cd mydir\r\n");    
    command_cd(disk, currentWorkingDirectoryPath, "mydir");
    printLocation();
    printf("ls\r\n");
    command_ls(disk, currentWorkingDirectoryPath);
    printLocation();
    printf("cat bigtext.txt\r\n");
    command_cat(disk, currentWorkingDirectoryPath, "bigtext.txt");
}

void _cdecl cstart_(uint16_t bootDrive)
{

    DISK disk;
    if(!DISK_Initialize(&disk, bootDrive))
    {
		printf("Disk init error\r\n");
		goto end;
    }
    
    if(!FAT_Initialize(&disk))
    {	
		printf("FAT init error\r\n");
		goto end;
    }    

    //test1(&disk);
    test2(&disk);

end:
    for(;;);
}
