#ifndef _SW_INFO_H
#define _SW_INFO_H

#include "stdint.h"

typedef struct sw_info_s
{
  uint32_t date;
  uint32_t _date_;
  char name[16];
  char version[16];
  char bootloader_version[16];
  uint32_t boot_addr;
  struct sw_info_map_s
  {
    uint32_t addr;
    uint32_t size;
  }map[5];
}sw_info_st;

extern const sw_info_st sw_info;
#endif
