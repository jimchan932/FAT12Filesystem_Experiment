#pragma once 

#include "stdint.h"
#include "disk.h"

#define SECTOR_SIZE          512
#define MAX_PATH_SIZE        256
#define MAX_FILE_HANDLES      10 
#define ROOT_DIRECTORY_HANDLE -1 



#pragma pack(push, 1)

typedef struct {
    uint8_t Name[11]; // "8:3" filename (8: filename, 3 extension) 
    uint8_t Attributes; //
    uint8_t _Reserved;
    uint8_t CreatedTimeTenths;
    uint16_t CreatedTime;
    uint16_t CreatedDate;
    uint16_t AccessedDate;
    uint16_t FirstClusterHigh; // high two bytes of first cluster number
    uint16_t ModifiedTime;
    uint16_t ModifiedDate; 
    uint16_t FirstClusterLow; // start of a file in clusters
    uint32_t Size; // file size in bytes,  directories should be 0
} FAT_DirectoryEntry;
#pragma pack(pop)

typedef struct
{
    int Handle;
    bool IsDirectory;
    uint32_t Position;
    uint32_t Size;
} FAT_File;


enum FAT_Attributes
{
    FAT_ATTRIBUTE_READ_ONLY = 0x01,
    FAT_ATTRIBUTE_HIDDEN= 0x02,
    FAT_ATTRIBUTE_SYSTEM = 0x04,
    FAT_ATTRIBUTE_VOLUME_ID = 0x08,
    FAT_ATTRIBUTE_DIRECTORY = 0x10,
    FAT_ATTRIBUTE_ARCHIVE = 0x20,
    FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID
};
bool FAT_Initialize(DISK *disk);
FAT_File far *FAT_Open(DISK *disk, const char *path);
uint32_t FAT_Read(DISK *disk, FAT_File far *file, uint32_t byteCount, void *dataOut);
bool FAT_ReadEntry(DISK *disk, FAT_File far * file, FAT_DirectoryEntry *dirEntry);
void FAT_Close(FAT_File far* file);
bool FAT_FindFile(DISK *disk, FAT_File far* file, const char *name, FAT_DirectoryEntry *entryOut);
bool FAT_Write(DISK *disk, FAT_File far *file, void *dataIn);


void FAT_updateDirEntry(DISK *disk, FAT_File far* file, uint32_t bytesToWrite, char *filename);
