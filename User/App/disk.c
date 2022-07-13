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
FRESULT disk_get_size(uint8_t disk,uint32_t* total, uint32_t* free)
{
  FATFS* fs;
  FRESULT res;
  uint32_t clust;

  if(disk > 1) return FR_INVALID_PARAMETER;

  if(disk == 0)
    res = f_getfree(USERPath, (DWORD*)&clust, &fs);
  else
    res = f_getfree(USERPath2, (DWORD*)&clust, &fs);

  if(res == FR_OK)
  {
    *total = ((fs->n_fatent - 2) * fs->csize) * 4096;
    *free = (clust * fs->csize) * 4096;
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
