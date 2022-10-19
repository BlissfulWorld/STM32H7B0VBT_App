#ifndef _USER_SETTINGS_H
#define _USER_SETTINGS_H

#include "stdint.h"

typedef struct item_head_s
{
  uint8_t count;
  uint8_t crc;
  uint8_t data[0];
}item_head_st;

#define USI(name,len) \
struct item_##name##_s\
{\
  item_head_st head;\
  uint8_t data[len];\
}_##name##_

//用户设置项
typedef struct user_settings_item_s
{
  //语言文件
  USI(language, 128);

  //背光
  USI(backlight, 1);

  //音量
  USI(volume, 1);

  //待机多少分钟后关机
  USI(standbytime, 4);

  //开短路测试使能
  USI(ostest, 1);

  //开短路生成配置
  //误差类型
  USI(ostest_mk_conf_err_type, 1);
  //误差百分比
  USI(ostest_mk_conf_err_pct, 2);
}user_settings_item_st;

extern user_settings_item_st* user_settings_item;

int us_load(void);
int us_set(uint16_t ofs, void* pdata, uint16_t data_len);
int us_get(uint16_t ofs, void* pbuffer, uint16_t data_len);

#define US_OFS(item)         ((uint16_t)(uint32_t)&(((user_settings_item_st *)0)->_##item##_))
#define US_LEN(item)         (sizeof(user_settings_item->_##item##_.data))

#define US_GET(item,pbuffer) us_get(US_OFS(item),pbuffer,US_LEN(item))
#define US_SET(item,pdata)   us_set(US_OFS(item),pdata,US_LEN(item))

#endif
