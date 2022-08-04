#ifndef _DISK_H
#define _DISK_H

#include "ff.h"

FRESULT mount(uint8_t disk);
FRESULT umount(uint8_t disk);
FRESULT format(uint8_t disk);
FRESULT disk_get_size(uint8_t disk, uint32_t* total, uint32_t* free);

FRESULT external_storage_mount(void);
#endif