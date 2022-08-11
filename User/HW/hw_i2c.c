#include "hw_i2c.h"
#include "i2c.h"
#include "cmsis_os.h"

typedef struct hw_i2c_handler_s
{
  I2C_HandleTypeDef* hi2c;
  void (*init)(void);
  osMutexId mutex;
}hw_i2c_handler_st;

void* hw_i2c1, * hw_i2c2, * hw_i2c4;
static hw_i2c_handler_st _hw_i2c1, _hw_i2c2, _hw_i2c4;

/**
 * @brief i2c初始化
 *
 */
void hw_i2c_init(void)
{
  _hw_i2c1.mutex = osMutexNew(NULL);
  _hw_i2c1.hi2c = &hi2c1;
  _hw_i2c1.init = MX_I2C1_Init;

  _hw_i2c2.mutex = osMutexNew(NULL);
  _hw_i2c2.hi2c = &hi2c2;
  _hw_i2c2.init = MX_I2C2_Init;

  _hw_i2c4.mutex = osMutexNew(NULL);
  _hw_i2c4.hi2c = &hi2c4;
  _hw_i2c4.init = MX_I2C4_Init;

  hw_i2c1 = &_hw_i2c1;
  hw_i2c2 = &_hw_i2c2;
  hw_i2c4 = &_hw_i2c4;
}

/**
 * @brief i2c总线修复
 *
 * @param handler hw i2c句柄
 * @param timeout 超时，单位系统滴答
 * @return int 错误码
 */
int hw_i2c_fix(const void* handler, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __exit;
  }

  hw_i2c = handler;

  if(osMutexWait(hw_i2c->mutex, timeout) == osOK)
  {
    HAL_I2C_DeInit(hw_i2c->hi2c);
    hw_i2c->init();
    osMutexRelease(hw_i2c->mutex);
  }
  else
  {
    res = -2;
    goto __exit;
  }

__exit:
  return res;
}

/**
 * @brief i2c读写
 *
 * @param handler 句柄
 * @param addr i2c地址
 * @param pw 待写入数据的指针
 * @param wlen 写入长度
 * @param pr 读出数据的缓存指针
 * @param rlen 读出长度
 * @param timeout 超时，单位系统滴答
 * @return int 错误
 */
int hw_i2c_write_read(const void* handler, uint8_t addr, const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __exit;
  }

  hw_i2c = handler;

  if(osMutexWait(hw_i2c->mutex, timeout) == osOK)
  {
    if(pw && wlen)
    {
      if(HAL_I2C_Master_Transmit(hw_i2c->hi2c, addr, (uint8_t*)pw, wlen, timeout))
        res = -2;
    }

    if((res == 0) && pr && rlen)
    {
      if(HAL_I2C_Master_Receive(hw_i2c->hi2c, addr, pr, rlen, timeout))
        res = -3;
    }

    osMutexRelease(hw_i2c->mutex);
  }
  else
  {
    res = -2;
    goto __exit;
  }

__exit:

  return res;
}

/**
 * @brief i2c写1字节地址x字节数据
 *
 * @param handler i2c句柄
 * @param id i2c地址
 * @param addr 8位地址
 * @param pw 待写入数据的指针
 * @param wlen 写入长度
 * @param timeout 超时，单位系统滴答
 * @return int 错误码
 */
int hw_i2c_write_1a_xd(const void* handler, uint8_t id, uint8_t addr, const uint8_t* pw, uint32_t wlen, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __exit;
  }

  if(!pw || !wlen)
  {
    res = -2;
    goto __exit;
  }

  hw_i2c = handler;

  if(osMutexWait(hw_i2c->mutex, timeout) == osOK)
  {
    if(HAL_I2C_Mem_Write(hw_i2c->hi2c, id, addr, I2C_MEMADD_SIZE_8BIT, (uint8_t*)pw, wlen, timeout))
      res = -3;
    osMutexRelease(hw_i2c->mutex);
  }
  else
  {
    res = -4;
    goto __exit;
  }

__exit:

  return res;
}

/**
 * @brief i2c写2字节地址x字节数据
 *
 * @param handler i2c句柄
 * @param id i2c地址
 * @param addr 2字节地址
 * @param pw 待写入数据的指针
 * @param wlen 写入长度
 * @param timeout 超时，单位系统滴答
 * @return int 错误码
 */
int hw_i2c_write_2a_xd(const void* handler, uint8_t id, uint16_t addr, const uint8_t* pw, uint32_t wlen, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __exit;
  }

  if(!pw || !wlen)
  {
    res = -2;
    goto __exit;
  }

  hw_i2c = handler;

  if(osMutexWait(hw_i2c->mutex, timeout) == osOK)
  {
    if(HAL_I2C_Mem_Write(hw_i2c->hi2c, id, addr, I2C_MEMADD_SIZE_16BIT, (uint8_t*)pw, wlen, timeout))
      res = -3;
    osMutexRelease(hw_i2c->mutex);
  }
  else
  {
    res = -4;
    goto __exit;
  }

__exit:

  return res;
}

/**
 * @brief i2c写2字节地址读x字节数据
 *
 * @param handler i2c句柄
 * @param id i2c地址
 * @param addr 2字节地址
 * @param pr 读出数据的缓存指针
 * @param rlen 读取长度
 * @param timeout 超时，单位系统滴答
 * @return int 错误码
 */
int hw_i2c_read_2a_xd(const void* handler, uint8_t id, uint16_t addr, uint8_t* pr, uint32_t rlen, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __exit;
  }

  if(!pr || !rlen)
  {
    res = -2;
    goto __exit;
  }

  hw_i2c = handler;

  if(osMutexWait(hw_i2c->mutex, timeout) == osOK)
  {
    if(HAL_I2C_Mem_Read(hw_i2c->hi2c, id, addr, I2C_MEMADD_SIZE_16BIT, pr, rlen, timeout))
      res = -3;
    osMutexRelease(hw_i2c->mutex);
  }
  else
  {
    res = -4;
    goto __exit;
  }

__exit:

  return res;
}

/**
 * @brief i2c写1字节读1字节
 *
 * @param handler i2c句柄
 * @param id i2c地址
 * @param addr 1字节地址
 * @param pr 读出数据的缓存指针
 * @param timeout 超时，单位系统滴答
 * @return int 错误码
 */
int hw_i2c_read_1a_1d(const void* handler, uint8_t id, uint8_t addr, uint8_t* pr, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __exit;
  }

  if(pr == 0)
  {
    res = -2;
    goto __exit;
  }

  hw_i2c = handler;

  if(osMutexWait(hw_i2c->mutex, timeout) == osOK)
  {
    if(HAL_I2C_Mem_Read(hw_i2c->hi2c, id, addr, I2C_MEMADD_SIZE_8BIT, pr, 1, timeout))
      res = -3;

    osMutexRelease(hw_i2c->mutex);
  }
  else
  {
    res = -4;
    goto __exit;
  }

__exit:

  return res;
}
