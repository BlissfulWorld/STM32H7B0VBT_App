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

void hw_i2c_init(void)
{
  osMutexDef(hw_i2c1);
  osMutexDef(hw_i2c2);
  osMutexDef(hw_i2c4);

  _hw_i2c1.mutex = osMutexNew(osMutex(hw_i2c1));
  _hw_i2c1.hi2c = &hi2c1;
  _hw_i2c1.init = MX_I2C1_Init;

  _hw_i2c2.mutex = osMutexNew(osMutex(hw_i2c2));
  _hw_i2c2.hi2c = &hi2c2;
  _hw_i2c2.init = MX_I2C2_Init;

  _hw_i2c4.mutex = osMutexNew(osMutex(hw_i2c4));
  _hw_i2c4.hi2c = &hi2c4;
  _hw_i2c4.init = MX_I2C4_Init;

  hw_i2c1 = &_hw_i2c1;
  hw_i2c2 = &_hw_i2c2;
  hw_i2c4 = &_hw_i2c4;
}

int hw_i2c_fix(const void* handler, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __hw_i2c_fix_exit;
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
    goto __hw_i2c_fix_exit;
  }

__hw_i2c_fix_exit:
  return res;
}

int hw_i2c_write_read(const void* handler, uint8_t addr, const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __hw_i2c_write_read_exit;
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
    goto __hw_i2c_write_read_exit;
  }

__hw_i2c_write_read_exit:

  return res;
}

int hw_i2c_write_8a_xd(const void* handler, uint8_t id, uint8_t addr, const uint8_t* pw, uint32_t wlen, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __hw_i2c_write_8a_xd_exit;
  }

  if(!pw || !wlen)
  {
    res = -2;
    goto __hw_i2c_write_8a_xd_exit;
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
    goto __hw_i2c_write_8a_xd_exit;
  }

__hw_i2c_write_8a_xd_exit:

  return res;
}

int hw_i2c_write_16a_xd(const void* handler, uint8_t id, uint16_t addr, const uint8_t* pw, uint32_t wlen, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __hw_i2c_write_16a_xd_exit;
  }

  if(!pw || !wlen)
  {
    res = -2;
    goto __hw_i2c_write_16a_xd_exit;
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
    goto __hw_i2c_write_16a_xd_exit;
  }

__hw_i2c_write_16a_xd_exit:

  return res;
}

int hw_i2c_read_16a_xd(const void* handler, uint8_t id, uint16_t addr, uint8_t* pr, uint32_t rlen, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __hw_i2c_read_16a_xd_exit;
  }

  if(!pr || !rlen)
  {
    res = -2;
    goto __hw_i2c_read_16a_xd_exit;
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
    goto __hw_i2c_read_16a_xd_exit;
  }

__hw_i2c_read_16a_xd_exit:

  return res;
}

int hw_i2c_read_8a_8d(const void* handler, uint8_t id, uint8_t addr, uint8_t* pr, uint32_t timeout)
{
  const hw_i2c_handler_st* hw_i2c;
  int res = 0;

  if(handler == 0)
  {
    res = -1;
    goto __hw_i2c_read_8a_8d_exit;
  }

  if(pr == 0)
  {
    res = -2;
    goto __hw_i2c_read_8a_8d_exit;
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
    goto __hw_i2c_read_8a_8d_exit;
  }

__hw_i2c_read_8a_8d_exit:

  return res;
}