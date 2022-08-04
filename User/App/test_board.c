#include "test_board.h"
#include "spiflash.h"
#include "cmsis_os.h"
#include "hash.h"
#include "cryp.h"
#include "string.h"
#include "pca9535.h"

/**
 * @brief 小板上电
 * 
 * @return int 错误码
 */
int board_power_on(void)
{
  uint16_t output;

  if(pca9535_get_output(1, &output) != 0)
  {
    return -1;
  }

  if(output & (1 << 8))
  {
    return 0;
  }

  if(pca9535_set_output(1, output | (1 << 8)))
  {
    return -2;
  }

  return 0;
}

/**
 * @brief 小板掉电
 * 
 * @return int 错误码
 */
int board_power_off(void)
{
  uint16_t output;

  if(pca9535_get_output(1, &output) != 0)
  {
    return -1;
  }

  if((output & (1 << 8)) == 0)
  {
    return 0;
  }

  if(pca9535_set_output(1, output & (~(1 << 8))))
  {
    return -2;
  }

  return 0;
}

/**
 * @brief 解密小板头部信息
 * 
 * @param header 头部信息指针
 * @param uid 小板id数据指针
 * @return int 错误码
 */
int board_header_decrypt(board_header_st* header, uint8_t* uid)
{
  uint8_t iv[16], in_tmp[16], out_tmp[16];
  uint8_t* p;

  p = (uint8_t*)header;

  memcpy(iv, uid, 16);

  for(uint32_t i = 0;i < sizeof(board_header_st);i += 16)
  {
    memcpy(in_tmp, p + i, 16);
    if(HAL_CRYP_Decrypt(&hcryp, (uint32_t*)in_tmp, 4, (uint32_t*)out_tmp, 1000))
    {
      return -1;
    }

    for(uint32_t j = 0;j < 16;j++)
    {
      p[i + j] = out_tmp[j] ^ iv[j];
    }

    memcpy(iv, in_tmp, 16);
  }

  return 0;
}

/**
 * @brief 
 * 
 * @return test_module_st* 
 */
test_module_st* test_module_load(void)
{
  test_module_st* module;
  uint8_t uid[16];
  uint8_t manufacturer, device;

  module = pvPortMalloc(sizeof(test_module_st));
  if(module == 0)
  {
    goto __test_module_load_exit;
  }

  //获取flash id
  if(spiflash_product_id_read(&manufacturer, &device))
  {
    goto __test_module_load_free_module;
  }

  if((manufacturer == 0x00 && device == 0x00) || (manufacturer == 0xFF && device == 0xFF))
  {
    goto __test_module_load_free_module;
  }

  //获取flash uid
  if(spiflash_uid_read(uid))
  {
    goto __test_module_load_free_module;
  }

  //获取板子头部段
  if(spiflash_read(0x00000000, &module->board_header, sizeof(board_header_st)))
  {
    goto __test_module_load_free_module;
  }

  //解密头部段
  if(board_header_decrypt(&module->board_header, uid))
  {
    goto __test_module_load_free_module;
  }

  //校验
  if((manufacturer != module->board_header.info.flash_manufacturer_id) ||
    (device != module->board_header.info.flash_device_id) ||
    (memcmp(uid, module->board_header.uid, 16) != 0)
    )
  {
    goto __test_module_load_free_module;
  }

  module->id.code = module->board_header.info.module_id_code;
  module->config_file = config_file_load_from_board(&module->board_header);
  //if(module->config_file == 0)
  //  module->config_file = "{\"name\":\"内置\",\"version\":\"1\",\"desc\":\"主机内置默认版\"}";

  goto __test_module_load_exit;

__test_module_load_free_module:
  vPortFree(module);
  module = 0;

__test_module_load_exit:
  return module;
}

test_module_st* test_module_load_dbg(void)
{
  test_module_st* module;
  board_info_st* info;

  module = pvPortMalloc(sizeof(test_module_st));
  if(module)
  {
    memset(module, 0, sizeof(test_module_st));

    info = &module->board_header.info;

    info->config_file_encrypt = 0;
    info->module_id_code = 0;
    info->flash_manufacturer_id = 0;
    info->flash_device_id = 0;
    info->sector_num = 0;
    info->sector_size = 0;
    info->config_file_sector_ofs = 0;
    info->config_file_encrypt = 0;
    info->version_major = 0;
    info->version_minor = 0;
    info->reserve = 0;

    extern const test_module_id_ut ip7_id;
    module->id.code = ip7_id.code;
  }

  return module;
}

int test_module_unload(test_module_st* module)
{
  if(module)
  {
    vPortFree((void*)module->config_file);
    vPortFree(module);
    return 0;
  }
  else
  {
    return -1;
  }
}

#if 0//小板证书创建
board_header_st* board_header_create(uint32_t module_id_code)
{
  board_header_st* header;

  header = pvPortMalloc(sizeof(board_header_st));
  if(header == 0)
  {
    goto __exit;
  }

  memset(header, 0x00, sizeof(board_header_st));

  //获取flash id
  if(spiflash_product_id_read(&header->info.flash_manufacturer_id, &header->info.flash_device_id))
  {
    goto __free_header;
  }

  if((header->info.flash_manufacturer_id == 0x00 && header->info.flash_device_id == 0x00) ||
    (header->info.flash_manufacturer_id == 0xFF && header->info.flash_device_id == 0xFF))
  {
    goto __free_header;
  }

  if(spiflash_sector_num_get(header->info.flash_device_id) == 0)
  {
    goto __free_header;
  }

  //获取flash uid
  if(spiflash_uid_read(header->uid))
  {
    goto __free_header;
  }

  header->info.module_id_code = module_id_code;
  header->info.sector_num = spiflash_sector_num_get(header->info.flash_device_id);
  header->info.sector_size = 4096;

  header->info.version_major = 0;
  header->info.version_minor = 0;
  header->info.config_file_encrypt = 0;
  header->info.config_file_sector_ofs = 1;

  goto __exit;

__free_header:
  vPortFree(header);
  header = 0;

__exit:
  return header;
}

int board_header_delete(board_header_st* header)
{
  if(header == 0)
  {
    return -1;
  }

  vPortFree(header);

  return 0;
}

int board_header_encrypt(board_header_st* header)
{
  uint8_t iv[16], in_tmp[16];
  uint8_t* p;


  if(header == 0)
  {
    return -1;
  }

  p = (uint8_t*)header;

  memcpy(iv, header->uid, 16);

  for(uint32_t i = 0;i < sizeof(board_header_st);i += 16)
  {
    for(uint32_t j = 0;j < 16;j++)
    {
      in_tmp[j] = iv[j] ^ p[i + j];
    }

    if(HAL_CRYP_Encrypt(&hcryp, (uint32_t*)in_tmp, 4, (uint32_t*)iv, 1000))
    {
      return -2;
    }

    memcpy(p + i, iv, 16);
  }

  return 0;
}

int board_header_write(board_header_st* header)
{
  if(header == 0)
  {
    return -1;
  }

  if(spiflash_erase_sector(0, 5000))
  {
    return -2;
  }

  if(spiflash_write(0x00000000, header, sizeof(board_header_st), 2000))
  {
    return -3;
  }

  return 0;
}
#endif
