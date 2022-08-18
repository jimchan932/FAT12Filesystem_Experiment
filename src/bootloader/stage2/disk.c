#include "disk.h"
#include "x86.h"
#include "stdio.h"

bool DISK_Initialize(DISK *disk, uint8_t driveNumber)
{
    uint8_t driveType;
    uint16_t cylinders, sectors, heads;
    if(!x86_Disk_GetDriveParams(disk->id, &driveType, &cylinders, &sectors, &heads))
    {
		return false;	
    }
    disk->id = driveNumber;
    disk->cylinders = cylinders+1;
    disk->heads = heads+1;
    disk->sectors = sectors;
    
    return true;
}

void DISK_LBA2CHS(DISK *disk, uint32_t lba, uint16_t *cylinderOut, uint16_t *headOut, uint16_t *sectorOut)
{
    // sector = (LBA $ sectors per track + 1)
    *sectorOut = lba % disk->sectors + 1;
    // cylinders = (LBA / sectors per track) /heads
    *cylinderOut = (lba/disk->sectors) / disk->heads;

    // head = (LBA / sectors per track) % heads
    *headOut = (lba / disk->sectors) % disk->heads;
}


 //printf("Read sectors: lba=%lu, cyl = %u, sec = %u, head = %u, disk.heads =%u, disk.sectors=%u, disk.cyls=%u\r\n",lba, cylinder, sector, head, disk->heads, disk->sectors, disk->cylinders);

bool DISK_ReadSectors(DISK *disk, uint32_t lba, uint8_t sectors, void far *dataOut)
{
    uint16_t cylinder, sector, head;
	// 从 lba 转换成 chs， 以调用 x86_Disk_Read
    DISK_LBA2CHS(disk, lba, &cylinder, &head, &sector);
	
	//根据 Ralf Brown的interrupt list 网站所说，我们需要读取三次，
	// 每一次之间重置disk, 才能够保证正确读取扇区
    for(int i = 0; i < 3; i++)
    {
		if(x86_Disk_Read(disk->id, cylinder, sector, head, sectors, dataOut))
		{
			return true;
		}
		x86_Disk_Reset(disk->id);
    }
    return false;
}

bool DISK_WriteSectors(DISK *disk, uint32_t lba, uint8_t sectors, void far *dataIn)
{
    uint16_t cylinder, sector, head;
	// 从 lba 转换成 chs， 以调用 x86_Disk_Write
    DISK_LBA2CHS(disk, lba, &cylinder, &head, &sector);
	
	//根据 Ralf Brown的interrupt list 网站所说，我们需要写入三次，
	// 每一次之间重置disk, 才能够保证正确写入扇区	
	for(int i = 0; i < 3; i++)
    {
		if(x86_Disk_Write(disk->id, cylinder, sector, head, sectors, dataIn))
		{
			return true;
		}
		x86_Disk_Reset(disk->id);
    }
    return false;
}
