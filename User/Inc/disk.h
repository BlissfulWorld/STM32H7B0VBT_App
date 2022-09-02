#ifndef _DISK_H
#define _DISK_H

#include "fatfs.h"

typedef void (*file_copy_stat_cb_func_t)(uint32_t total, uint32_t count, void* user);

FRESULT mount(uint8_t disk);
FRESULT umount(uint8_t disk);
FRESULT format(uint8_t disk);
FRESULT disk_get_size(const char* disk, uint32_t* total, uint32_t* free);
FRESULT disk_copy_file(const char* file_dst, const char* file_src, file_copy_stat_cb_func_t stat_cb, void* user);

FRESULT external_storage_mount(void);

#endif
