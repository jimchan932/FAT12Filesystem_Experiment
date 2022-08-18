#pragma once

#include "stdint.h"

typedef struct
{
    uint8_t id;
    uint16_t cylinders;
    uint16_t sectors;
    uint16_t heads;
} DISK;

bool DISK_Initialize(DISK *disk, uint8_t driveNumber);
void DISK_LBA2CHS(DISK *disk, uint32_t lba, uint16_t *cylinderOut, uint16_t *headOut, uint16_t *sectorOut);

bool DISK_ReadSectors(DISK *disk, uint32_t lba, uint8_t sectors, void far *dataOut); // read sectors using logical block addressing 
bool DISK_WriteSectors(DISK *disk, uint32_t lba, uint8_t sectors, void far *dataIn);
