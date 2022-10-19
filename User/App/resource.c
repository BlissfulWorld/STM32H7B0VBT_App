#include "resource.h"
#include "fatfs.h"
#include "stdio.h"
#include "config.h"
#include "disk.h"

const char* audio_file_board_loaded = WAV_FILE_BOARD_LOADED;
const char* audio_file_warning = WAV_FILE_WARNING;

const char* resource_dir_list[] =
{
  "audio",
  "font",
  "icon",
  "language"
};

/**
 * @brief 更新资源 (从用户存储复制到系统存储）
 *
 */
void resource_update(void)
{
  char* dir_src;
  char* dir_dst;
  uint32_t dir_src_len;

  dir_src = ff_malloc(128 * 2);
  if(dir_src == 0)
  {
    goto __exit;
  }

  dir_dst = dir_src + 128;

  //检查用户区存储是否存在resource目录
  sprintf(dir_src, "%sresource", DISK_USER_PATH);
  if(disk_dir_is_exist(dir_src) == 0)
  {
    goto __free_dir_src;
  }

  dir_src_len = strlen(dir_src);
  dir_src[dir_src_len] = '/';
  dir_src_len++;
  dir_src[dir_src_len] = '\0';

  //遍历每种资源并更新到系统存储区
  for(uint32_t i = 0;i < sizeof(resource_dir_list) / sizeof(const char*);i++)
  {
    strcpy(dir_src + dir_src_len, resource_dir_list[i]);
    if(disk_dir_is_exist(dir_src))
    {
      //删除旧资源
      sprintf(dir_dst, "%s%s", DISK_SYS_PATH, resource_dir_list[i]);
      disk_dir_delete(dir_dst);

      if(f_mkdir(dir_dst) != FR_OK)
      {
        break;
      }

      //复制新资源
      if(disk_copy_dir(dir_dst, dir_src, 0, 0, 0) != FR_OK)
      {
        break;
      }
    }
  }

  //删除用户存储区资源
  dir_src[dir_src_len - 1] = '\0';
  disk_dir_delete(dir_src);

__free_dir_src:
  ff_free(dir_src);

__exit:
  return;
}

/**
 * @brief 加载资源
 *
 * @param path 路径
 * @return resource_st* 资源指针
 */
resource_st* resource_load(const char* path)
{
  FRESULT res;
  FIL* fp;
  UINT br;
  resource_st* src_p;

  src_p = 0;

  fp = (FIL*)ff_malloc(sizeof(FIL));
  if(fp == 0)
  {
    goto __exit;
  }

  res = f_open(fp, path, FA_READ);
  if(res != FR_OK)
  {
    goto __free_fp;
  }

  src_p = (resource_st*)pvPortMalloc(sizeof(resource_st) + f_size(fp));
  if(src_p == 0)
  {
    goto __close_fp;
  }

  src_p->size = f_size(fp);

  res = f_read(fp, src_p->data, src_p->size, &br);
  if((res != FR_OK) || (br != src_p->size))
  {
    goto __free_src_p;
  }

  goto __close_fp;

__free_src_p:
  vPortFree(src_p);
  src_p = 0;

__close_fp:
  f_close(fp);

__free_fp:
  ff_free(fp);

__exit:
  return src_p;
}

/**
 * @brief 卸载资源，释放内存
 *
 * @param res 资源指针
 * @return int 错误码
 */
int resource_unload(resource_st* res)
{
  if(res)
  {
    vPortFree(res);
    return 0;
  }
  else
  {
    return -1;
  }
}

/**
 * @brief 从磁盘(内部flash)中加载图标资源
 *
 * @param name 图标资源名称
 * @return resource_st* 图标资源指针
 */
resource_st* resource_icon_load(const char* name)
{
  char path[128];

  snprintf(path, sizeof(path), "%sicon/%s.dta", DISK_SYS_PATH, name);

  return resource_load(path);
}

/**
 * @brief 从磁盘(内部flash)中加载logo资源
 *
 * @return resource_st* logo资源指针
 */
resource_st* resource_logo_load(void)
{
  char path[128];

  snprintf(path, sizeof(path), "%slogo/logo.bmp", DISK_SYS_PATH);

  return resource_load(path);
}

/**
 * @brief dta格式图片资源：speaker
 *
 */
const uint8_t res_image_dta_speaker[84] =
{
  /*00000000*/0x42,0x4d,0x64,0x00,0x14,0x00,0x14,0x00,0x03,0x00,0x01,0x00,0x02,0x00,0x01,0x00,
  /*00000010*/0x00,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  /*00000020*/0x00,0x00,0x30,0x00,0x00,0x50,0x40,0x00,0x90,0x80,0x1f,0x11,0x00,0x10,0x12,0x00,
  /*00000030*/0x10,0x10,0x00,0x10,0x10,0x00,0x10,0x13,0xc0,0x10,0x10,0x00,0x10,0x10,0x00,0x10,
  /*00000040*/0x12,0x00,0x1f,0x11,0x00,0x00,0x90,0x80,0x00,0x50,0x40,0x00,0x30,0x00,0x00,0x00,
  /*00000050*/0x00,0x00,0x00,0x00,
};
