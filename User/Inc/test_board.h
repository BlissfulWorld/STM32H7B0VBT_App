#ifndef _TEST_BOARD_H
#define _TEST_BOARD_H

#include "stdint.h"

typedef struct board_info_s
{
  //此结构体总大小必须16字节对齐
  uint32_t module_id_code;
  uint8_t flash_manufacturer_id;
  uint8_t flash_device_id;
  uint16_t sector_num;
  uint16_t sector_size;
  uint16_t config_file_sector_ofs;
  uint8_t config_file_encrypt;
  uint8_t version_major;
  uint8_t version_minor;
  uint8_t reserve;
}board_info_st;

typedef struct board_header_s
{
  uint8_t random[16];
  board_info_st info;
  uint8_t uid[16];
}board_header_st;

typedef enum device_e
{
  IPHONE = 0x10,

  DEVICE_END = 0xFE,
}device_et;

typedef enum ip_model_e
{
  IPHONE_1          = 0x0000,
  IPHONE_2G         = 0x0001,
  IPHONE_3G         = 0x0002,
  IPHONE_3GS        = 0x0003,
  IPHONE_4          = 0x0004,
  IPHONE_4S         = 0x0005,
  IPHONE_5          = 0x0006,
  IPHONE_5C         = 0x0007,
  IPHONE_5S         = 0x0008,
  IPHONE_6          = 0x0009,
  IPHONE_6P         = 0x000A,
  IPHONE_SE         = 0x000B,
  IPHONE_6S         = 0x000C,
  IPHONE_6SP        = 0x000D,
  IPHONE_7          = 0x000E,
  IPHONE_7P         = 0x000F,
  IPHONE_8          = 0x0010,
  IPHONE_8P         = 0x0011,
  IPHONE_X          = 0x0012,
  IPHONE_XS         = 0x0013,
  IPHONE_XS_MAX     = 0x0014,
  IPHONE_XR         = 0x0015,
  IPHONE_11         = 0x0016,
  IPHONE_11_PRO     = 0x0017,
  IPHONE_11_PRO_MAX = 0x0018,
  /*
        .
        .
        .
  */
  IPHONE_END = 0xFFFE,
}ip_model_t;

typedef union test_module_id_u
{
  uint32_t code;
  struct
  {
    uint8_t device;
    uint8_t model_h;
    uint8_t model_l;
    uint8_t number;
  };
}test_module_id_ut;

#include "config_file.h"

typedef struct test_module_s
{
  test_module_id_ut id;
  board_header_st board_header;
  const config_file_st* config_file;
}test_module_st;

test_module_st* test_module_load(void);
int test_module_unload(test_module_st* module);

#endif
