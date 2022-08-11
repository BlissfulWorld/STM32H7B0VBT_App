#ifndef _W25QXX_H
#define _W25QXX_H

#include "stdint.h"

typedef enum w25qxx_e
{
  W25Q08 = 0xEF13,
  W25Q16,
  W25Q32,
  W25Q64,
  W25Q128,
  W25Q256,
  W25Q512,
}w25qxx_et;

typedef struct w25qxx_t
{
  //型号
  w25qxx_et id;

  //扇区数量
  uint32_t sector_num;

  //扇区大小
  uint32_t sector_size;

  /**
    * @brief 读
    * @param pbuffer 指向用于存储读取数据的内存的指针
    * @param address 地址
    * @param size 读取大小
    * @retval 0:成功 !0:失败
    */
  int (*read)(void* pbuffer, uint32_t address, uint32_t size);

  /**
    * @brief 写
    * @param pbuffer 指向待写入的数据的内存指针
    * @param address 地址
    * @param size 写入大小
    * @retval 0:成功 !0:失败
    */
  int (*write)(void* pdata, uint32_t address, uint32_t size);

  /**
    * @brief 扇区擦除 每扇区4KByte
    * @param address: 地址 (对应扇区号sector=address/(4*1024))
    * @retval 0:成功 !0:失败
    */
  int (*sector_erase)(uint32_t address);

  /**
    * @brief 芯片全擦除
    * @retval 0:成功 !0:失败
    */
  int (*chip_erase)(void);
}w25qxx_st;

extern w25qxx_st* w25qxx;

int w25qxx_init(void);

#endif
