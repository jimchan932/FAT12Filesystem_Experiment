#include "fat.h"
#include "stdint.h"
#include "disk.h"
#include "filenameConversion.h"
#include "string.h"
#include "ctype.h"
#include "stdio.h"
#include "memdefs.h"
#include "memory.h"


#pragma pack(push, 1)

// structure of boot sector
typedef struct
{
    
	uint8_t BootJumpInstruction[3];
	uint8_t OemIdentifer[8];
	uint16_t BytesPerSector;
	uint8_t SectorsPerCluster;
	uint16_t ReservedSectors;
	uint8_t FatCount;
	uint16_t DirEntryCount;
	uint16_t TotalSectors;
	uint8_t MediaDescriptorType;
	uint16_t SectorsPerFat;
	uint16_t SectorsPerTrack;
	uint16_t Heads;
	uint32_t HiddenSectors;
	uint32_t LargeSectorCount;

	// extended boot record
	uint8_t DriveNumber;
	uint8_t _Reserved;
	uint8_t Signature;
	uint32_t VolumeId; // serial number , value doesn't matter
	uint8_t VolumeLabel[11]; // 11 bytes, padded with spaces
	uint8_t SystemId[8];

	// 我们不理会代码部分
} FAT_BootSector;
#pragma pack(pop)

typedef struct
{
    uint8_t Buffer[SECTOR_SIZE];    
    FAT_File Public; // contain the data we will return to the user
    bool Opened;
    uint32_t FirstCluster;   // 文件第一个簇的位置
    uint32_t CurrentCluster; // 目前文件的簇位置
    uint32_t CurrentSectorInCluster; 
} FAT_FileData;
    
typedef struct
{
    union
    {
	FAT_BootSector BootSector;
	uint8_t BootSectorBytes[SECTOR_SIZE];
    } BS;
    FAT_FileData RootDirectory;
    FAT_FileData OpenedFiles[MAX_FILE_HANDLES]; // open files
} FAT_Data;

static FAT_Data far *g_Data;
static uint8_t far *g_Fat = NULL;
static uint32_t g_DataSectionLba;

// 读取引导扇区
bool FAT_ReadBootSector(DISK *disk)
{
    return DISK_ReadSectors(disk, 0, 1, &g_Data->BS.BootSectorBytes);
}
// 读取FAT表
bool FAT_ReadFat(DISK *disk)
{
    return DISK_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool FAT_Initialize(DISK *disk)
{    
    g_Data = (FAT_Data far*)MEMORY_FAT_ADDR;

    
    // 读取引导扇区
    if(!FAT_ReadBootSector(disk))
    {	
		printf("FAT: read boot sector error");
		return false;
    }
    // 读取FAT表

    uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;
    g_Fat = (uint8_t far *)g_Data + sizeof(FAT_Data);
  
    
    if(sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE)
    {	
        printf("FAT: not enough memory to read FAT! Required %lu, only have %lu\r\n", sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
		return false;
    }
    if(!FAT_ReadFat(disk))
    {
		printf("FAT: read FAT failed\r\n");
		return false;
    }
 
    // 开根文件

    uint32_t rootDirLba = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount;
    
    uint32_t rootDirSize =  sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;
          
    g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE;
    g_Data->RootDirectory.Public.IsDirectory = true;
    g_Data->RootDirectory.Public.Position = 0;
    g_Data->RootDirectory.Public.Size = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount;  // directory_entry_size * num of directory entry
    g_Data->RootDirectory.Opened = true;
    g_Data->RootDirectory.FirstCluster = rootDirLba; 
    g_Data->RootDirectory.CurrentCluster = rootDirLba;
    g_Data->RootDirectory.CurrentSectorInCluster = 0;
    
    if(!DISK_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer))
    {
		printf("FAT: read root directory failed\r\n");
		return false;
    }
        
    // 计算数据区开始第一个扇区的位置
    uint32_t rootDirSectors = (rootDirSize + g_Data->BS.BootSector.BytesPerSector - 1)/g_Data->BS.BootSector.BytesPerSector;
    g_DataSectionLba = rootDirLba + rootDirSectors;

    // 从新设定已开文件
    for(int i = 0; i < MAX_FILE_HANDLES; i++)
    {
		g_Data->OpenedFiles[i].Opened = false;
    }
    
    return true;    
}

uint32_t FAT_ClusterToLba( uint32_t cluster)
{
    return g_DataSectionLba + (cluster - 2)  * g_Data->BS.BootSector.SectorsPerCluster;
}

FAT_File far *FAT_OpenEntry(DISK * disk, FAT_DirectoryEntry * entry)
{
    // 找出空的文件处理
    int handle = -1;
    for(int i = 0; i = MAX_FILE_HANDLES && handle < 0; i++)
    {
		if(!g_Data->OpenedFiles[i].Opened)
			handle = i;
    }
    // 没有空的文件处理
    if(handle < 0)
    {
		printf("FAT: out of file handlers\r\n");
		return false;
    }
    // 设置
    FAT_FileData far * fd = &g_Data->OpenedFiles[handle];
    fd->Public.Handle = handle;
    fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->Public.Position = 0;
    fd->Public.Size = entry->Size;
    fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
    fd->CurrentCluster= fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    if(!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer))
    {
		printf("FAT: read error\r\n");
		return false;
    }

    fd->Opened = true;
    return &fd->Public;
}

uint32_t FAT_NextCluster(uint32_t currentCluster)
{

    uint32_t fatIndex = currentCluster * 3 / 2;
    if(currentCluster % 2 == 0) // n 为偶数
    {
		return (*(uint16_t far*)(g_Fat + fatIndex))& 0x0FFF;
    }
    else // n 为奇数
    {
		return (*(uint16_t far*)(g_Fat + fatIndex)) >> 4; 
    }
}

uint32_t FAT_Read(DISK *disk, FAT_File far *file, uint32_t byteCount, void *dataOut)
{
    FAT_FileData far *fd = (file->Handle == ROOT_DIRECTORY_HANDLE)
	? &g_Data->RootDirectory
	: &g_Data->OpenedFiles[file->Handle];
    
    uint8_t *u8DataOut = (uint8_t *) dataOut;

    // 获取文件数据
    if(!fd->Public.IsDirectory || (fd->Public.IsDirectory && fd->Public.Size != 0))
    {
		byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);
    }
    
    while(byteCount > 0)
    {
		uint32_t leftInBuffer = SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
		uint32_t take = min(byteCount, leftInBuffer);

		memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
		u8DataOut += take;
		fd->Public.Position += take;
		byteCount -= take;

		//  看我们需不需要读取更多数据
		if(leftInBuffer == take)
		{	    
			// 对根文件的特定处理
			if(fd->Public.Handle == ROOT_DIRECTORY_HANDLE)
			{
				++fd->CurrentCluster;
				// 读取下一个扇区
				if(!DISK_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer))
				{
					printf("FAT: read error!\r\n");
					break;		
				}				
				}	    
				else
				{		
				// 计算下一个要读的簇和扇区
				if(++fd->CurrentSectorInCluster >= g_Data->BS.BootSector.SectorsPerCluster)
				{
					fd->CurrentSectorInCluster = 0;
					fd->CurrentCluster = FAT_NextCluster(fd->CurrentCluster);
				}
				if(fd->CurrentCluster >= 0xFF8) // 文件结束
				{
					// 记录文件结束位置
					fd->Public.Size = fd->Public.Position;
					break;
				}

				// 读取下一个扇区
				if(!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster,1, fd->Buffer))
				{
					// 处理错误
					printf("FAT: read error!\r\n");
					break;		
				}	
			}
		}
    }
    return u8DataOut - (uint8_t*)dataOut;
}

bool FAT_Write(DISK *disk, FAT_File far *file, void *dataIn)
{    
    FAT_FileData far *fd = (file->Handle == ROOT_DIRECTORY_HANDLE)
	? &g_Data->RootDirectory
	: &g_Data->OpenedFiles[file->Handle]; 
	// 写数据进入对应文件的扇区
    return DISK_WriteSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1, dataIn);
}

bool FAT_ReadEntry(DISK *disk, FAT_File far * file, FAT_DirectoryEntry *dirEntry)
{
    return FAT_Read(disk, file, sizeof(FAT_DirectoryEntry), dirEntry) == sizeof(FAT_DirectoryEntry);
}
void FAT_Close(FAT_File far* file)
{
    if(file->Handle == ROOT_DIRECTORY_HANDLE)
    {
		file->Position = 0;
		g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    }
    else
    {
		g_Data->OpenedFiles[file->Handle].Opened = false;
    }
}

// 在根文件夹找出文件
bool FAT_FindFile(DISK *disk, FAT_File far* file, const char *name, FAT_DirectoryEntry *entryOut)
{    
    
    char fatName[12];
	//转换文件名成 FAT格式
    convertToFATFilename(fatName, name);
    FAT_DirectoryEntry entry; 
    while(FAT_ReadEntry(disk, file, &entry))
    {	
		if(memcmp(fatName, entry.Name, 11) == 0)
		{
			*entryOut = entry;
			return true;
		}
    }
    return false;
}

FAT_File far *FAT_Open(DISK * disk, const char * path)
{
    char name[MAX_PATH_SIZE];
    // 略到斜线
    if(path[0] == '/')
		path++;
    FAT_File far *current  = &g_Data->RootDirectory.Public;

    while(*path)
    {
		// 获取路径的下一个文件名
		bool isLast = false;
		const char * delim = strchr(path, '/');
		if(delim != NULL)
		{
			memcpy(name, path, delim-path);
			name[delim-path + 1] = '\0';
			path = delim + 1;	    
		}
		else
		{
			unsigned len = strlen(path);
			memcpy(name, path, len);
			name[len+1] = '\0';
			path += len;
			isLast = true;
		}

		// 在目前文件家里找出对应文件名的目录项
		FAT_DirectoryEntry entry;
		if(FAT_FindFile(disk, current, name, &entry))
		{
			FAT_Close(current);
			// 判断是否文件夹
			if(!isLast && (entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) == 0)
			{
			printf("FAT: %s not a directory\r\n", name);
			return NULL;
			}

			// open new directory entry	
			current = FAT_OpenEntry(disk, &entry);
		}
		else
		{
			FAT_Close(current);
			printf("FAT %s not found\r\n", name);
			return NULL;
		}
    }
    return current;
}


    
/*
void FAT_updateDirEntry(DISK *disk, FAT_File far* file, uint32_t bytesToWrite, char *filename)
{
    
    FAT_FileData far *fd = (file->Handle == ROOT_DIRECTORY_HANDLE)
	? &g_Data->RootDirectory
	: &g_Data->OpenedFiles[file->Handle]; // forget about root directory
    
    char fatName[12];

    //convert from name to fat name
    convertToFATFilename(fatName, filename);
    FAT_DirectoryEntry entry;      
    int entryCount = 0;
    
    while(FAT_ReadEntry(disk, file, &entry) && entryCount++ < 5)
    {	
	if(memcmp(fatName, entry.Name, 11) == 0)
	{
	    break;
	}
    }
    printf("rest %d", fd->Buffer[entryCount* 4]);
    printf("rest %d", fd->Buffer[entryCount* 4 + 1]);
    printf("rest %d", fd->Buffer[entryCount* 4 + 2]);
    printf("rest %d", g_Data->RootDirectory.Buffer[entryCount* 4+ 3]);
    
    
  
    printf("asdf %d", fd->Buffer[entryCount*4 + 4]);
    fd->Buffer[entryCount*4+3] = 200;
	
    uint32_t beginClusterToWrite =
	(file->Handle == ROOT_DIRECTORY_HANDLE)
	? fd->FirstCluster
	: FAT_ClusterToLba(fd->FirstCluster);
    DISK_WriteSectors(disk, beginClusterToWrite, 1, fd->Buffer);   
	 
    printf("\r\n");

}
*/

