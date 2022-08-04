#include "user_settings.h"
#include "string.h"
#include "24c64.h"
#include "cmsis_os.h"

user_settings_item_st* user_settings_item;

/**
  * @brief 累加和
  */
static uint8_t crc(uint8_t* pdata, uint32_t len)
{
  uint8_t crc = 0xFF;
  for(uint32_t i = 0;i < len;i++)
  {
    crc += pdata[i];
  }

  return crc;
}

/**
  * @brief 加载用户设置到内存
  * @retval 0:成功 1:失败
  */
int us_load(void)
{
  user_settings_item = pvPortMalloc(sizeof(user_settings_item_st));
  if(user_settings_item == 0)
    return -1;

  if(_24c64_read(0, user_settings_item, sizeof(user_settings_item_st)))
  {
    memset(user_settings_item, 0, sizeof(user_settings_item_st));
    return -2;
  }
  else
  {
    return 0;
  }
}

/**
  * @brief 用户设置修改
  * @param ofs 此项设置的偏移位置
  * @param pdata 数据指针
  * @param data_len 数据长度
  * @retval 0:成功 1:失败
  */
int us_set(uint16_t ofs, void* pdata, uint16_t data_len)
{
  if(user_settings_item == 0)
    return -1;

  item_head_st* phead = (item_head_st*)(((uint8_t*)user_settings_item) + ofs);
  memcpy(phead->data, pdata, data_len);
  phead->count++;
  phead->crc = crc(phead->data, data_len);

  if(_24c64_write(ofs + sizeof(item_head_st), phead->data, data_len) || _24c64_write(ofs, phead, sizeof(item_head_st)))
  {
    return -2;
  }
  else
  {
    return 0;
  }
}

/**
  * @brief 用户设置获取
  * @param ofs 此项设置的偏移位置
  * @param pbuffer 内存指针
  * @param data_len 数据长度
  * @retval 0:成功 1:失败
  */
int us_get(uint16_t ofs, void* pbuffer, uint16_t data_len)
{
  if(user_settings_item == 0)
    return -1;

  item_head_st* phead = (item_head_st*)(((uint8_t*)user_settings_item) + ofs);

  if(crc(phead->data, data_len) == phead->crc)
  {
    memcpy(pbuffer, phead->data, data_len);
    return 0;
  }
  else
  {
    return -2;
  }
}
