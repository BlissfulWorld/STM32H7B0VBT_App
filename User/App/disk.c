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
    res = f_mount(&USERFatFS, USERPath, 1);
  else
    res = f_mount(&USERFatFS2, USERPath2, 1);

  return res;
}

/**
  * @brief 卸载外部flash
  */
FRESULT umount(uint8_t disk)
{
  if(disk > 1) return FR_INVALID_PARAMETER;

  if(disk == 0)
    return f_mount(0, USERPath, 1);
  else
    return f_mount(0, USERPath2, 1);
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
    res = f_mkfs(USERPath, FM_ANY, 0, work, _MAX_SS);
  else
    res = f_mkfs(USERPath2, FM_ANY, 0, work, _MAX_SS);

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
