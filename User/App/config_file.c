#include "config_file.h"
#include "cmsis_os.h"
#include "spiflash.h"
#include "hash.h"
#include "string.h"
#include "ff.h"
#include "cjson.h"
#include "json_obj.h"
#include "stdio.h"

const char config_file_update_path[] = "2:/config.json";

/**
 * @brief 从小板中加载配置文件
 *
 * @param header 小板信息头
 * @return const config_file_st* 配置文件指针
 */
const config_file_st* config_file_load_from_board(board_header_st* header)
{
  config_file_st* config_file, * tmp;
  uint8_t md5[16];
  uint32_t size;
  uint32_t addr;

  //只分配配置文件信息大小的内存
  //加载配置文件信息部分
  config_file = pvPortMalloc(sizeof(config_file_st));
  if(config_file == 0)
  {
    goto __exit;
  }

  //得到配置文件地址
  addr = (uint32_t)header->info.config_file_sector_ofs * (uint32_t)header->info.sector_size;

  if(spiflash_read(addr, config_file, sizeof(config_file_st)) != 0)
  {
    goto __free_config_file;
  }

  //检查配置文件大小是否异常
  if(config_file->file.size > (header->info.sector_num - header->info.config_file_sector_ofs) * (uint32_t)header->info.sector_size)
  {
    goto __free_config_file;
  }

  addr += sizeof(config_file_st);

  //重新分配完整的配置文件大小的内存
  //加载完整的配置文件
  tmp = pvPortMalloc(sizeof(config_file_st) + config_file->file.size + 1);
  if(tmp == 0)
  {
    goto __free_config_file;
  }

  memcpy(tmp, config_file, sizeof(config_file_st));

  vPortFree(config_file);

  config_file = tmp;

  if(spiflash_read(addr, config_file->file.data, config_file->file.size) != 0)
  {
    goto __free_config_file;
  }

  //检验配置文件MD5
  if(HAL_HASH_MD5_Start(&hhash, (uint8_t*)&(config_file->file), sizeof(config_file->file) + config_file->file.size, md5, 1000) != HAL_OK)
  {
    goto __free_config_file;
  }

  if(memcmp(md5, config_file->md5, 16) != 0)
  {
    goto __free_config_file;
  }

  //文件末尾附加‘\0’结束字符串，以防文件字符串内容访问出现越界
  config_file->file.data[config_file->file.size] = 0;

  goto __exit;

__free_config_file:
  vPortFree(config_file);
  config_file = 0;

__exit:
  return config_file;
}

/**
 * @brief 从磁盘(内部flash)中加载配置文件，磁盘中的文件只有配置文件的内容部分，此函数会补上其他信息
 *
 * @param path 配置文件路径
 * @return const config_file_st* 配置文件指针
 */
const config_file_st* config_file_load_from_disk(const char* path)
{
  FRESULT res;
  FIL fil;
  config_file_st* config_file;
  UINT br;

  config_file = 0;

  res = f_open(&fil, path, FA_READ);
  if(res != FR_OK)
  {
    goto __exit;
  }

  config_file = pvPortMalloc(sizeof(config_file_st) + f_size(&fil) + 1);
  if(config_file == 0)
  {
    goto __close_fil;
  }

  memset(config_file, 0, sizeof(config_file_st));

  //磁盘中的配置文件只有内容，没有信息记录
  //添加文件类型信息
  config_file->file.size = f_size(&fil);
  memcpy(config_file->file.type, "json", 4);

  res = f_read(&fil, config_file->file.data, config_file->file.size, &br);
  if(res != FR_OK)
  {
    goto __free_config_file;
  }

  //添加MD5信息
  HAL_HASH_MD5_Start(&hhash, (uint8_t*)&(config_file->file), sizeof(config_file->file) + config_file->file.size, config_file->md5, 1000);

  config_file->file.data[config_file->file.size] = 0;

  goto __close_fil;

__free_config_file:
  vPortFree(config_file);
  config_file = 0;

__close_fil:
  f_close(&fil);

__exit:
  return config_file;
}

/**
 * @brief 卸载配置文件
 *
 * @param config_file 配置文件指针
 * @return int 错误码
 */
int config_file_unload(const config_file_st* config_file)
{
  if(config_file)
  {
    vPortFree((void*)config_file);
    return 0;
  }
  else
  {
    return 1;
  }
}

/**
 * @brief 更新小板的配置文件
 *
 * @param header 小板头部信息指针
 * @param path 配置文件路径
 * @return int 错误码
 */
int config_file_update_to_board_from_disk(board_header_st* header, const char* path)
{
  int err;
  const config_file_st* config_file;
  uint32_t sector_num_occupy;

  err = 0;

  if(header == 0)
  {
    err = -1;
    goto __exit;
  }

  config_file = config_file_load_from_disk(path);
  if(config_file == 0)
  {
    err = -2;
    goto __exit;
  }

  //检查小板空间是否足够存储新配置文件
  sector_num_occupy = ((config_file->file.size + sizeof(config_file_st)) + header->info.sector_size) / header->info.sector_size;
  if(sector_num_occupy > header->info.sector_num - header->info.config_file_sector_ofs)
  {
    err = -3;
    goto __unload_config_file;
  }

  //擦除
  for(uint32_t i = 0;i < sector_num_occupy;i++)
  {
    if(spiflash_erase_sector((i + header->info.config_file_sector_ofs) * header->info.sector_size, 5000))
    {
      err = -4;
      goto __unload_config_file;
    }
  }

  //写入
  if(spiflash_write(header->info.config_file_sector_ofs * header->info.sector_size, config_file, config_file->file.size + sizeof(config_file_st), config_file->file.size << 4))
  {
    err = -5;
    goto __unload_config_file;
  }

__unload_config_file:
  config_file_unload(config_file);

__exit:
  return err;
}

/**
 * @brief 解析配置文件的共同部分的信息
 *
 * @param config_file 配置文件指针
 * @param info 共同部分的信息的指针
 * @return int 错误码
 */
int common_config_info_phase(const config_file_st* config_file, common_config_info_st* info)
{
  int err;
  cJSON* json;

  err = 0;

  if(info == 0)
  {
    err = -1;
    goto __exit;
  }

  memset(info, 0, sizeof(common_config_info_st));

  if(memcmp(config_file->file.type, "json", 4) != 0)
  {
    err = -2;
    goto __exit;
  }

  json = cJSON_Parse((char*)config_file->file.data);
  if(json == NULL)
  {
    goto __exit;
  }

  cJSON* j_name = cJSON_GetObjectItem(json, json_obj_name);
  if(j_name != NULL)
  {
    snprintf(info->name, sizeof(info->name), "%s", j_name->valuestring);
  }

  cJSON* j_desc = cJSON_GetObjectItem(json, json_obj_description);
  if(j_desc != NULL)
  {
    snprintf(info->desc, sizeof(info->desc), "%s", j_desc->valuestring);
  }

  cJSON* j_version = cJSON_GetObjectItem(json, json_obj_version);
  if(j_version != NULL)
  {
    info->version = j_version->valueint;
  }

  cJSON_Delete(json);

__exit:

  return err;
}

/**
 * @brief 将数据写入小板，此数据只是配置的内容部分，不是完整配置文件，写入前会补上配置文件的其他部分的信息
 *
 * @param header 小板头部信息指针
 * @param pdata 数据指针
 * @param size 数据大小
 * @return int 错误码
 */
int config_file_data_write_to_board(board_header_st* header, const void* pdata, uint32_t size)
{
  int err;
  config_file_st* config_file;
  uint32_t sector_num_occupy;

  err = 0;

  if(header == 0 || size == 0)
  {
    err = -1;
    goto __exit;
  }

  config_file = pvPortMalloc(sizeof(config_file_st) + size);
  if(config_file == 0)
  {
    err = -2;
    goto __exit;
  }

  memset(config_file, 0, sizeof(config_file_st));
  memcpy(config_file->file.data, pdata, size);

  //添加文件类型信息
  config_file->file.size = size;
  memcpy(config_file->file.type, "json", 4);

  //添加MD5信息
  HAL_HASH_MD5_Start(&hhash, (uint8_t*)&(config_file->file), sizeof(config_file->file) + config_file->file.size, config_file->md5, 1000);

  //检查小板空间是否足够存储新配置文件
  sector_num_occupy = ((config_file->file.size + sizeof(config_file_st)) + header->info.sector_size) / header->info.sector_size;
  if(sector_num_occupy > header->info.sector_num - header->info.config_file_sector_ofs)
  {
    err = -3;
    goto __free_config_file;
  }

  //擦除
  for(uint32_t i = 0;i < sector_num_occupy;i++)
  {
    if(spiflash_erase_sector((i + header->info.config_file_sector_ofs) * header->info.sector_size, 5000))
    {
      err = -4;
      goto __free_config_file;
    }
  }

  //写入
  if(spiflash_write(header->info.config_file_sector_ofs * header->info.sector_size, config_file, config_file->file.size + sizeof(config_file_st), config_file->file.size << 2))
  {
    err = -5;
    goto __free_config_file;
  }

__free_config_file:
  vPortFree(config_file);

__exit:
  return err;
}
