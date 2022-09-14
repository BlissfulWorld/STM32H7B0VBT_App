#ifndef _DISK_H
#define _DISK_H

#include "fatfs.h"

typedef enum dir_copy_event_e
{
  DIR_CPY_EVT_DIR_CPY_STA,
  DIR_CPY_EVT_DIR_CPY_END,
  DIR_CPY_EVT_DIR_CPY_ERR,
  DIR_CPY_EVT_FILE_CPY_STA,
  DIR_CPY_EVT_FILE_CPY_END,
  DIR_CPY_EVT_FILE_CPY_ERR,
}dir_copy_event_et;

typedef void (*file_copy_stat_cb_func_t)(uint32_t total, uint32_t count, void* user);
typedef int (*dir_copy_event_cb_func_t)(const char *dst,const char *src,dir_copy_event_et evt,void *user);

FRESULT mount(uint8_t disk);
FRESULT umount(uint8_t disk);
FRESULT format(uint8_t disk);
int disk_dir_is_exist(const char* dir_path);
FRESULT disk_dir_delete(const char* dir_path);
FRESULT disk_get_size(const char* disk, uint32_t* total, uint32_t* free);
FRESULT disk_copy_file(const char* file_dst, const char* file_src, file_copy_stat_cb_func_t stat_cb, void* user);
FRESULT disk_copy_dir(const char *dir_dst,const char *dir_src,dir_copy_event_cb_func_t event_cb,file_copy_stat_cb_func_t stat_cb,void *user);

FRESULT external_storage_mount(void);

#endif
