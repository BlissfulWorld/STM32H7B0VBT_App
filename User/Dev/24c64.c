#include "24c64.h"
#include "hw_i2c.h"
#include "stm32h7xx_hal.h"

#define _24C64_I2C_ID (0xA0)
#define _24C64_PAGE_SIZE (32)

#define TIMEOUT_MS (20)

static uint32_t _24c64_i2c_write(uint16_t addr, void* pdata, uint16_t len)
{
  uint32_t tick;

  tick = HAL_GetTick();

  do {
    if(hw_i2c_write_2a_xd(hw_i2c2, _24C64_I2C_ID, addr, pdata, len, (len >> 3) + 1) == 0) return 0;
  } while(HAL_GetTick() - tick < TIMEOUT_MS);

  return 1;
}

static uint32_t _24c64_i2c_read(uint16_t addr, void* pbuffer, uint16_t len)
{
  uint32_t tick;
  uint8_t buffer[2];

  buffer[0] = addr >> 8;
  buffer[1] = addr;

  tick = HAL_GetTick();

  do {
    if(hw_i2c_write_read(hw_i2c2, _24C64_I2C_ID, buffer, 2, pbuffer, len, (len >> 3) + 1) == 0) return 0;
  } while(HAL_GetTick() - tick < TIMEOUT_MS);

  return 1;
}

/**
  * @brief 24c64 写
  * @param addr 写入地址
  * @param pdata 指向写入的数据的指针
  * @param len 写入长度
  * @retval 0:成功 1:失败
  */
uint32_t _24c64_write(uint16_t addr, void* pdata, uint16_t len)
{
  uint8_t* p = pdata;
  uint16_t page_num;
  uint16_t count;

  //前未对齐部分
  count = _24C64_PAGE_SIZE - (addr % _24C64_PAGE_SIZE);
  if(count != _24C64_PAGE_SIZE)
  {
    if(count > len) count = len;
    if(_24c64_i2c_write(addr, p, count)) return 1;

    len -= count;
    addr += count;
    p += count;
  }

  //中间对齐部分
  page_num = len / _24C64_PAGE_SIZE;
  for(uint16_t i = 0;i < page_num;i++)
  {
    if(_24c64_i2c_write(addr, p, _24C64_PAGE_SIZE)) return 1;
    addr += _24C64_PAGE_SIZE;
    p += _24C64_PAGE_SIZE;
    len -= _24C64_PAGE_SIZE;
  }

  //尾未对齐部分
  if(len)
  {
    if(_24c64_i2c_write(addr, p, len)) return 1;
  }

  return 0;
}

/**
  * @brief 24c64 读
  * @param addr 读取地址
  * @param pbuffer 指向接收数据的缓存的指针
  * @param len 读取长度
  * @retval 0:成功 1:失败
  */
uint32_t _24c64_read(uint16_t addr, void* pbuffer, uint16_t len)
{
  return _24c64_i2c_read(addr, pbuffer, len);
}
