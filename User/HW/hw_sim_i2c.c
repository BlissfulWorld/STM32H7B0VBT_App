#include "hw_sim_i2c.h"
#include "hw.h"
#include "string.h"
#include "cmsis_os.h"

typedef struct hw_sim_i2c_handler_s
{
  hw_pin_st scl;
  hw_pin_st sda;
  osMutexId mutex;
}hw_sim_i2c_handler_st;

void* hw_sim_i2c1;

static hw_sim_i2c_handler_st _hw_sim_i2c1;

/**
 * @brief 模拟i2c初始化
 *
 */
void hw_sim_i2c_init(void)
{
  memset(&_hw_sim_i2c1, 0, sizeof(_hw_sim_i2c1));
  _hw_sim_i2c1.mutex = osMutexNew(NULL);

  hw_sim_i2c1 = &_hw_sim_i2c1;
}

/**
 * @brief 模拟i2c设置引脚
 *
 * @param handler 句柄
 * @param scl scl引脚
 * @param sda sda引脚
 * @param timeout 超时，单位系统滴答
 * @return int 错误码
 */
int hw_sim_i2c_set_pin(void* handler, const hw_pin_st* scl, const hw_pin_st* sda, uint32_t timeout)
{
  int res = 0;
  hw_sim_i2c_handler_st* hw_sim_i2c;
  GPIO_InitTypeDef gpio_init;

  if(handler == 0)
  {
    res = -1;
    goto __exit;
  }

  hw_sim_i2c = handler;

  hw_sim_i2c->scl = *scl;
  hw_sim_i2c->sda = *sda;

  if(osMutexWait(hw_sim_i2c->mutex, timeout) == osOK)
  {
    if((hw_sim_i2c->scl.gpio == 0) || (hw_sim_i2c->scl.pin == 0) || (hw_sim_i2c->sda.gpio == 0) || (hw_sim_i2c->sda.pin == 0))
    {
      res = -3;
      goto __release;
    }

    HW_PIN_SET(&hw_sim_i2c->scl);
    HW_PIN_SET(&hw_sim_i2c->sda);

    gpio_init.Alternate = 0;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pin = hw_sim_i2c->scl.pin;
    HAL_GPIO_Init(hw_sim_i2c->scl.gpio, &gpio_init);

    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
    gpio_init.Pin = hw_sim_i2c->sda.pin;
    HAL_GPIO_Init(hw_sim_i2c->sda.gpio, &gpio_init);
  }
  else
  {
    res = -2;
    goto __exit;
  }

__release:
  osMutexRelease(hw_sim_i2c->mutex);
__exit:
  return res;
}

/**
 * @brief 模拟i2c时序中延时
 *
 * @param n_unit 延时周期
 */
static void hw_sim_i2c_delay(uint32_t n_unit)
{
  hw_delay_100ns(n_unit * 30);
}

/**
 * @brief 发送起始信号
 *
 * @param hw_sim_i2c i2c句柄
 */
static void hw_sim_i2c_send_start(const hw_sim_i2c_handler_st* hw_sim_i2c)
{
  HW_PIN_SET(&hw_sim_i2c->sda);
  hw_sim_i2c_delay(1);
  HW_PIN_SET(&hw_sim_i2c->scl);
  hw_sim_i2c_delay(1);

  HW_PIN_CLR(&hw_sim_i2c->sda);
  hw_sim_i2c_delay(2);
  HW_PIN_CLR(&hw_sim_i2c->scl);
  hw_sim_i2c_delay(2);
}

/**
 * @brief 发送停止信号
 *
 * @param hw_sim_i2c i2c句柄
 */
static void hw_sim_i2c_send_stop(const hw_sim_i2c_handler_st* hw_sim_i2c)
{
  HW_PIN_CLR(&hw_sim_i2c->sda);
  hw_sim_i2c_delay(1);
  HW_PIN_SET(&hw_sim_i2c->scl);
  hw_sim_i2c_delay(2);
  HW_PIN_SET(&hw_sim_i2c->sda);
  hw_sim_i2c_delay(2);
}

/**
 * @brief 发送ack信号
 *
 * @param hw_sim_i2c i2c句柄
 */
static void hw_sim_i2c_send_ack(const hw_sim_i2c_handler_st* hw_sim_i2c)
{
  hw_sim_i2c_delay(1);
  HW_PIN_CLR(&hw_sim_i2c->sda);
  hw_sim_i2c_delay(1);
  HW_PIN_SET(&hw_sim_i2c->scl);
  hw_sim_i2c_delay(2);
  HW_PIN_CLR(&hw_sim_i2c->scl);
}

/**
 * @brief 发送nack信号
 *
 * @param hw_sim_i2c i2c句柄
 */
static void hw_sim_i2c_send_nack(const hw_sim_i2c_handler_st* hw_sim_i2c)
{
  hw_sim_i2c_delay(1);
  HW_PIN_SET(&hw_sim_i2c->sda);
  hw_sim_i2c_delay(1);
  HW_PIN_SET(&hw_sim_i2c->scl);
  hw_sim_i2c_delay(2);
  HW_PIN_CLR(&hw_sim_i2c->scl);
}

/**
 * @brief 发送1字节数据
 *
 * @param hw_sim_i2c i2c句柄
 * @param byte 数据
 * @return int 错误码
 */
static int hw_sim_i2c_send_byte(const hw_sim_i2c_handler_st* hw_sim_i2c, uint8_t byte)
{
  int res = 0;

  for(uint8_t i = 0x80;i;i >>= 1)
  {
    HW_PIN_CLR(&hw_sim_i2c->scl);
    hw_sim_i2c_delay(1);
    if(byte & i)
    {
      HW_PIN_SET(&hw_sim_i2c->sda);
    }
    else
    {
      HW_PIN_CLR(&hw_sim_i2c->sda);
    }

    hw_sim_i2c_delay(1);
    HW_PIN_SET(&hw_sim_i2c->scl);
    hw_sim_i2c_delay(2);
  }

  HW_PIN_CLR(&hw_sim_i2c->scl);
  hw_sim_i2c_delay(1);
  HW_PIN_SET(&hw_sim_i2c->sda);
  hw_sim_i2c_delay(1);
  HW_PIN_SET(&hw_sim_i2c->scl);

  if(HW_PIN_GET(&hw_sim_i2c->sda) != 0)
    res = 1;

  hw_sim_i2c_delay(2);
  HW_PIN_CLR(&hw_sim_i2c->scl);
  hw_sim_i2c_delay(1);

__exit:
  return res;
}

/**
 * @brief 接收1字节数据
 *
 * @param hw_sim_i2c i2c句柄
 * @return uint8_t 字节数据
 */
static uint8_t hw_sim_i2c_recv_byte(const hw_sim_i2c_handler_st* hw_sim_i2c)
{
  uint8_t byte = 0;

  HW_PIN_SET(&hw_sim_i2c->sda);

  for(uint8_t i = 0x80;i;i >>= 1)
  {
    HW_PIN_CLR(&hw_sim_i2c->scl);
    hw_sim_i2c_delay(2);
    HW_PIN_SET(&hw_sim_i2c->scl);

    hw_sim_i2c_delay(1);
    if(HW_PIN_GET(&hw_sim_i2c->sda))
    {
      byte |= i;
    }
    hw_sim_i2c_delay(1);
  }

  HW_PIN_CLR(&hw_sim_i2c->scl);
  hw_sim_i2c_delay(2);

  return byte;
}

/**
 * @brief 模拟i2c读写数据
 *
 * @param handler i2c句柄
 * @param addr i2c地址
 * @param pw 待写入数据的指针
 * @param wlen 写入长度
 * @param pr 读出数据的缓存指针
 * @param rlen 读出长度
 * @param timeout 超时，单位系统滴答
 * @return int 错误码
 */
int hw_sim_i2c_write_read(const void* handler, uint8_t addr, const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout)
{
  int res = 0;
  const hw_sim_i2c_handler_st* hw_sim_i2c;

  if(handler == 0)
  {
    res = -1;
    goto __exit;
  }

  hw_sim_i2c = handler;

  if(osMutexWait(hw_sim_i2c->mutex, timeout) == osOK)
  {
    if((hw_sim_i2c->scl.gpio == 0) || (hw_sim_i2c->scl.pin == 0) || (hw_sim_i2c->sda.gpio == 0) || (hw_sim_i2c->sda.pin == 0))
    {
      res = -3;
      goto __release;
    }

    HW_PIN_SET(&hw_sim_i2c->scl);
    HW_PIN_SET(&hw_sim_i2c->sda);

    if((HW_PIN_GET(&hw_sim_i2c->scl) == 0) || (HW_PIN_GET(&hw_sim_i2c->sda) == 0))
    {
      res = -4;
      goto __release;
    }

    portENTER_CRITICAL();

    hw_sim_i2c_send_start(hw_sim_i2c);

    if(wlen)
    {
      if(hw_sim_i2c_send_byte(hw_sim_i2c, addr))
      {
        res = -5;
        goto __stop;
      }

      for(uint32_t i = 0;i < wlen;i++)
      {
        if(hw_sim_i2c_send_byte(hw_sim_i2c, pw[i]))
        {
          res = -6;
          goto __stop;
        }
      }
    }

    if(wlen && pr && rlen)
      hw_sim_i2c_send_start(hw_sim_i2c);

    if(pr && rlen)
    {
      if(hw_sim_i2c_send_byte(hw_sim_i2c, addr | 0x01))
      {
        res = -6;
        goto __stop;
      }

      for(uint32_t i = 0;i < rlen;i++)
      {
        pr[i] = hw_sim_i2c_recv_byte(hw_sim_i2c);
        if(i < rlen - 1)
          hw_sim_i2c_send_ack(hw_sim_i2c);
        else
          hw_sim_i2c_send_nack(hw_sim_i2c);
      }
    }
  }
  else
  {
    res = -2;
    goto __exit;
  }

__stop:
  hw_sim_i2c_send_stop(hw_sim_i2c);
  portEXIT_CRITICAL();
__release:
  osMutexRelease(hw_sim_i2c->mutex);
__exit:
  return res;
}
