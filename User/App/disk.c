#include "disk.h"
#include "fatfs.h"
#include "string.h"
#include "stdio.h"
#include "config.h"

/**
  * @brief 挂载外部flash进文件系统
  */
FRESULT mount(uint8_t disk)
{
  FRESULT res;

  if(disk > 1) return FR_INVALID_PARAMETER;

  if(disk == 0)
    res = f_mount(&DISK_SYS_FS, DISK_SYS_PATH, 1);
  else
    res = f_mount(&DISK_USER_FS, DISK_USER_PATH, 1);

  return res;
}

/**
  * @brief 卸载外部flash
  */
FRESULT umount(uint8_t disk)
{
  if(disk > 1) return FR_INVALID_PARAMETER;

  if(disk == 0)
    return f_mount(0, DISK_SYS_PATH, 1);
  else
    return f_mount(0, DISK_USER_PATH, 1);
}

/**
  * @brief 格式化外部flash
  */
FRESULT format(uint8_t disk)
{
  FRESULT res;
  uint8_t* work;

  if(disk > 1) return FR_INVALID_PARAMETER;

  work = ff_malloc(_MAX_SS);

  if(disk == 0)
    res = f_mkfs(DISK_SYS_PATH, FM_ANY, 0, work, _MAX_SS);
  else
    res = f_mkfs(DISK_USER_PATH, FM_ANY, 0, work, _MAX_SS);

  ff_free(work);
  return res;
}

/**
  * @brief 获取磁盘总大小和剩余空间，单位字节
  */
FRESULT disk_get_size(const char* disk, uint32_t* total, uint32_t* free)
{
  FATFS* fs;
  FRESULT res;
  uint32_t clust;

  res = f_getfree(disk, (DWORD*)&clust, &fs);

  if(res == FR_OK)
  {
    *total = ((fs->n_fatent - 2) * fs->csize) * 4096;
    *free = (clust * fs->csize) * 4096;
  }

  return res;
}

/**
 * @brief 判断目录是否存在
 *
 * @param dir_path 目录路径
 * @return int 1：存在 0：不存在
 */
int disk_dir_is_exist(const char* dir_path)
{
  DIR dir;

  if(f_opendir(&dir, dir_path) == FR_OK)
  {
    f_closedir(&dir);
    return 1;
  }

  return 0;
}

/**
 * @brief 删除目录
 *
 * @param dir_path 目录路径
 * @return FRESULT 错误码
 */
FRESULT disk_dir_delete(const char* dir_path)
{
  FRESULT res;
  DIR* dir;
  FILINFO* fno;
  char* path;

  if(dir_path == 0)
  {
    res = FR_INVALID_PARAMETER;
    goto __exit;
  }

  dir = ff_malloc(sizeof(DIR));
  if(dir == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __exit;
  }

  fno = ff_malloc(sizeof(FILINFO));
  if(fno == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __free_dir;
  }

  path = ff_malloc(512);
  if(path == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __free_fno;
  }

  memset(dir, 0, sizeof(DIR));
  memset(fno, 0, sizeof(FILINFO));

  res = f_opendir(dir, dir_path);
  if(res == FR_OK)
  {
    while(1)
    {
      res = f_readdir(dir, fno);
      if(res != FR_OK || fno->fname[0] == 0)
        break;

      snprintf(path, 512, "%s/%s", dir_path, fno->fname);
      if((fno->fattrib & AM_DIR) == 0)
      {
        res = f_unlink(path);
      }
      else
      {
        res = disk_dir_delete(path);
      }

      if(res != FR_OK)
        break;
    }

    f_closedir(dir);
    f_unlink(dir_path);
  }

__free_path:
  ff_free(path);

__free_fno:
  ff_free(fno);

__free_dir:
  ff_free(dir);

__exit:
  return res;
}

/**
 * @brief 复制文件，如果目标文件存在将覆盖
 *
 * @param file_dst 目标文件路径，例如 0:/dst.txt
 * @param file_src 源文件路径，例如 0:/src.txt
 * @param stat_cb 统计数据回调函数指针
 * @param user 用户指针
 * @return FRESULT 错误码
 */
FRESULT disk_copy_file(const char* file_dst, const char* file_src, file_copy_stat_cb_func_t stat_cb, void* user)
{
  FRESULT res;
  FIL* fil_src, * fil_dst;
  UINT br, bw;
  uint32_t size, count;
  uint8_t* pbuffer;

  fil_src = ff_malloc(sizeof(FIL));
  if(fil_src == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __exit;
  }

  fil_dst = ff_malloc(sizeof(FIL));
  if(fil_dst == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __free_fil_src;
  }

  res = f_open(fil_src, file_src, FA_READ);
  if(res != FR_OK)
  {
    goto __free_fil_dst;
  }

  size = f_size(fil_src);

  if(stat_cb)
  {
    stat_cb(size, 0, user);
  }

  res = f_open(fil_dst, file_dst, FA_CREATE_ALWAYS | FA_WRITE);
  if(res != FR_OK)
  {
    goto __close_fil_src;
  }

  pbuffer = ff_malloc(4096);
  if(pbuffer == 0)
  {
    goto __close_fil_dst;
  }

  count = 0;

  while(1)
  {
    res = f_read(fil_src, pbuffer, 4096, &br);
    if(res != FR_OK)
    {
      break;
    }

    if(br == 0)
    {
      break;
    }

    res = f_write(fil_dst, pbuffer, br, &bw);
    if(res != FR_OK)
    {
      break;
    }

    count += br;

    if(stat_cb)
    {
      stat_cb(size, count, user);
    }
  }

  ff_free(pbuffer);

__close_fil_dst:
  f_close(fil_dst);

__close_fil_src:
  f_close(fil_src);

__free_fil_dst:
  ff_free(fil_dst);

__free_fil_src:
  ff_free(fil_src);

__exit:
  return res;
}

/**
 * @brief 计算目录的大小
 *
 * @param dir_path 路径
 * @param size 大小指针
 * @return FRESULT 错误码
 */
FRESULT disk_dir_calc_size(const char* dir_path, uint32_t* size)
{
  FRESULT res;
  FIL* fil;
  DIR* dir;
  FILINFO* fno;
  char* path;

  fil = 0;

  dir = ff_malloc(sizeof(DIR));
  if(dir == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __exit;
  }

  fno = ff_malloc(sizeof(FILINFO));
  if(fno == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __free_dir;
  }

  path = ff_malloc(512);
  if(path == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __free_fno;
  }

  memset(dir, 0, sizeof(DIR));
  memset(fno, 0, sizeof(FILINFO));

  res = f_opendir(dir, dir_path);
  if(res == FR_OK)
  {
    while(1)
    {
      res = f_readdir(dir, fno);
      if(res != FR_OK || fno->fname[0] == 0)
        break;

      if((fno->fattrib & AM_DIR) == 0)
      {
        if(fil == 0)
        {
          fil = ff_malloc(sizeof(FIL));
          if(fil == 0)
          {
            res = FR_NOT_ENOUGH_CORE;
            break;
          }
        }
        snprintf(path, 512, "%s/%s", dir_path, fno->fname);

        res = f_open(fil, path, FA_READ);
        if(res != FR_OK)
          break;

        *size += f_size(fil);

        f_close(fil);
      }
      else
      {
        snprintf(path, 512, "%s/%s", dir_path, fno->fname);
        disk_dir_calc_size(path, size);
      }
    }
    f_closedir(dir);
  }

  if(fil) ff_free(fil);

__free_path:
  ff_free(path);

__free_fno:
  ff_free(fno);

__free_dir:
  ff_free(dir);

__exit:
  return res;
}

/**
 * @brief 复制目录，将覆盖所有相同名称的目录或文件
 *
 * @param dir_dst 目标目录，例如 0:/dst
 * @param dir_src 源目录，例如 0:/src
 * @param event_cb 事件回调函数指针
 * @param stat_cb 统计数据回调函数指针
 * @param user 用户指针
 * @return FRESULT 错误码
 */
static FRESULT _disk_copy_dir(const char* dir_dst, const char* dir_src, dir_copy_event_cb_func_t event_cb, file_copy_stat_cb_func_t stat_cb, void* user)
{
  FRESULT res;
  DIR* dir;
  FILINFO* fno;
  char* path_dst, * path_src;

  if(dir_src == 0 || dir_dst == 0)
  {
    res = FR_INVALID_PARAMETER;
    goto __exit;
  }

  dir = ff_malloc(sizeof(DIR));
  if(dir == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __exit;
  }

  fno = ff_malloc(sizeof(FILINFO));
  if(fno == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __free_dir;
  }

  path_src = ff_malloc(512);
  if(path_src == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __free_fno;
  }

  path_dst = ff_malloc(512);
  if(path_dst == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __free_path_src;
  }

  memset(dir, 0, sizeof(DIR));
  memset(fno, 0, sizeof(FILINFO));

  res = f_opendir(dir, dir_src);
  if(res == FR_OK)
  {
    while(1)
    {
      res = f_readdir(dir, fno);
      if(res != FR_OK || fno->fname[0] == 0)
        break;

      if((fno->fattrib & AM_DIR) == 0)
      {
        snprintf(path_src, 512, "%s/%s", dir_src, fno->fname);
        snprintf(path_dst, 512, "%s/%s", dir_dst, fno->fname);

        if(event_cb)
        {
          if(event_cb(path_dst, path_src, DIR_CPY_EVT_FILE_CPY_STA, user))
          {
            break;
          }
        }
        int cp_err = disk_copy_file(path_dst, path_src, stat_cb, user);
        if(event_cb)
        {
          if(event_cb(path_dst, path_src, cp_err == 0 ? DIR_CPY_EVT_FILE_CPY_END : DIR_CPY_EVT_FILE_CPY_ERR, user) != 0)
            break;
        }
      }
      else
      {
        snprintf(path_dst, 512, "%s/%s", dir_dst, fno->fname);
        snprintf(path_src, 512, "%s/%s", dir_src, fno->fname);
        if(event_cb)
        {
          if(event_cb(path_dst, path_src, DIR_CPY_EVT_DIR_CPY_STA, user))
          {
            break;
          }
        }
        f_mkdir(path_dst);
        if(_disk_copy_dir(path_dst, path_src, event_cb, stat_cb, user) == FR_OK)
        {
          if(event_cb)
          {
            if(event_cb(path_dst, path_src, DIR_CPY_EVT_DIR_CPY_END, user))
            {
              break;
            }
          }
        }
        else
        {
          if(event_cb)
          {
            if(event_cb(path_dst, path_src, DIR_CPY_EVT_DIR_CPY_ERR, user))
            {
              break;
            }
          }
        }
      }
    }

    f_closedir(dir);
  }

__free_path_dst:
  ff_free(path_dst);

__free_path_src:
  ff_free(path_src);

__free_fno:
  ff_free(fno);

__free_dir:
  ff_free(dir);

__exit:
  return res;
}

/**
 * @brief 复制目录，将覆盖所有相同名称的目录或文件
 *
 * @param dir_dst 目标目录，例如 0:/dst
 * @param dir_src 源目录，例如 0:/src
 * @param event_cb 事件回调函数指针
 * @param stat_cb 统计数据回调函数指针
 * @param user 用户指针
 * @return FRESULT 错误码
 */
FRESULT disk_copy_dir(const char* dir_dst, const char* dir_src, dir_copy_event_cb_func_t event_cb, file_copy_stat_cb_func_t stat_cb, void* user)
{
  FRESULT res;
  if(event_cb)
  {
    if(event_cb(dir_dst, dir_src, DIR_CPY_EVT_DIR_CPY_STA, user))
    {
      return FR_OK;
    }
  }

  f_mkdir(dir_dst);

  res = _disk_copy_dir(dir_dst, dir_src, event_cb, stat_cb, user);
  if(res == FR_OK)
  {
    if(event_cb)
    {
      event_cb(dir_dst, dir_src, DIR_CPY_EVT_DIR_CPY_END, user);
    }
  }
  else
  {
    if(event_cb)
    {
      event_cb(dir_dst, dir_src, DIR_CPY_EVT_DIR_CPY_ERR, user);
    }
  }

  return res;
}

/**
  * @brief 挂载外部存储到文件系统
  * @retval 错误码
  */
FRESULT external_storage_mount(void)
{
  uint32_t total, free;
  FRESULT res;

  res = mount(0);
  if(res == FR_NO_FILESYSTEM)
  {
    res = format(0);
    if(res == 0)
    {
      res = mount(0);
    }
  }

  res = mount(1);
  if(res == FR_NO_FILESYSTEM)
  {
    res = format(1);
    if(res == 0)
    {
      res = mount(1);
    }
  }

  return res;
}
