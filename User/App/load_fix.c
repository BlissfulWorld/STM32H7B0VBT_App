#include "config.h"
#include "sw_info.h"
#include "GUI.h"

#include "main.h"
#include "string.h"
#include "hash.h"
#include "cryp.h"

#include "24c64.h"
#include "w25qxx.h"

typedef struct fw_header_s
{
  char version[12];
  uint32_t size;
  uint8_t digest[16];
}fw_header_st;

typedef struct fw_s
{
  fw_header_st header;
  struct
  {
    union
    {
      sw_info_st info;
      uint8_t data[0x00010000];
    }code_1;
    struct
    {
      uint8_t data[0];
    }code_2;
  }code;
}fw_st;

#define FW_FLASH_SECTOR_OFFSET (0) 

#if 0
#include "stdio.h"
#define LF_DBG(...) printf(__VA_ARGS__)
#else
#define LF_DBG(...) do{}while(0)
#endif

//计算固件MD5
static int fw_calc_md5(sw_info_st* sw_info, uint8_t* out)
{
  uint8_t tmp[16];

  for(uint32_t i = 0;i < sizeof(tmp);i++)
  {
    tmp[i] = ((__IO uint8_t*)0x00000000)[i];
  }

  HAL_HASH_MD5_Accmlt(&hhash, (uint8_t*)tmp, sizeof(tmp));
  HAL_HASH_MD5_Accmlt(&hhash, (uint8_t*)(0x00000000 + sizeof(tmp)), 0x10000 - sizeof(tmp));
  for(int i = 0;(i < sizeof(sw_info->map) / sizeof(struct sw_info_map_s)) && sw_info->map[i].addr;i++)
  {
    HAL_HASH_MD5_Accmlt(&hhash, (uint8_t*)sw_info->map[i].addr, sw_info->map[i].size);
  }
  HAL_HASH_MD5_Accmlt(&hhash, (uint8_t*)sw_info->name, sizeof(sw_info->name));

  return HAL_HASH_MD5_Accmlt_End(&hhash, (uint8_t*)sw_info->version, sizeof(sw_info->version), out, 1000);
}

__ALIGN_BEGIN static const uint32_t pKeyCRYP[4] __ALIGN_END = {
                            0xC281ABCD,0x20200326,0xD554C7B7,0x88FA1A2C};

int fw_fix(void)
{
  int err;
  uint32_t *key_old;
  sw_info_st* sw_info = (sw_info_st*) 0x00000000;
  uint32_t idx;
  char name[sizeof(sw_info->name)], version[sizeof(sw_info->version)];
  uint8_t md5[16], tmp[16], out_tmp[16], iv[16];

  err = 0;
  idx = 0;
  
  key_old = hcryp.Init.pKey;
  hcryp.Init.pKey = (uint32_t *)pKeyCRYP;

  //读取名称
  if(w25qxx->read(name, idx, sizeof(name)))
  {
    err = -1;
    goto __exit;
  }
  idx += sizeof(name);

  //读取版本
  if(w25qxx->read(version, idx, sizeof(version)))
  {
    err = -2;
    goto __exit;
  }
  idx += sizeof(version);

  //读取MD5
  if(w25qxx->read(md5, idx, sizeof(md5)))
  {
    err = -3;
    goto __exit;
  }
  idx += sizeof(md5);
  
  fw_calc_md5(sw_info,tmp);

  if(memcmp(tmp,md5,16) == 0)
  {
    err = 0;
    goto __exit;
  }
  
  //md5作为AES128-ECB算法的IV
  memcpy(iv, md5, 16);

  //解密0x00000000区的代码数据
  for(uint32_t i = 0;i < 0x10000;i += 16)
  {
    if(w25qxx->read(tmp, idx, 16))
    {
      err = -4;
      goto __exit;
    }
    idx += 16;

    HAL_CRYP_Decrypt(&hcryp, (uint32_t*)tmp, 4, (uint32_t*)out_tmp, 20000);

    for(uint32_t j = 0;j < 16;j++)
    {
      ((__IO uint8_t*)(0x00000000))[i + j] = out_tmp[j] ^ iv[j];
    }

    memcpy(iv, tmp, 16);
  }

  //校验名称和版本
  if(memcmp(sw_info->name, name, sizeof(sw_info->name)) || memcmp(sw_info->version, version, sizeof(sw_info->version)))
  {
    err = -5;
    goto __exit;
  }

  //解密其他区的代码数据
  for(int m = 0;(m < sizeof(sw_info->map) / sizeof(struct sw_info_map_s)) && sw_info->map[m].addr;m++)
  {
    for(uint32_t i = 0;i < sw_info->map[m].size;i += 16)
    {
      if(w25qxx->read(tmp, idx, 16))
      {
        err = -6;
        goto __exit;
      }
      idx += 16;

      HAL_CRYP_Decrypt(&hcryp, (uint32_t*)tmp, 4, (uint32_t*)out_tmp, 20000);

      for(int j = 0;j < 16;j++)
      {
        ((uint8_t*)sw_info->map[m].addr)[i + j] = out_tmp[j] ^ iv[j];
      }

      memcpy(iv, tmp, 16);
    }
  }
  
__exit:
  hcryp.Init.pKey =  key_old;
  return err;
}
