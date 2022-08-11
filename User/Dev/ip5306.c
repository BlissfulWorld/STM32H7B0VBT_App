#include "ip5306.h"
#include "hw_i2c.h"

#define I2C_ID (0xEA)

static int reg_write(uint8_t reg, uint8_t val)
{
  uint8_t buffer[2];
  buffer[0] = reg;
  buffer[1] = val;
  return hw_i2c_write_read(hw_i2c2, I2C_ID, buffer, 2, 0, 0, 10);
}

static int reg_read(uint8_t reg, uint8_t* val)
{
  return hw_i2c_read_1a_1d(hw_i2c2, I2C_ID, reg, val, 10);
  //hw_i2c_write_read(hw_i2c2,I2C_ID,&reg,1,val,1,10);
}

/**
  * @brief 充电状态获取
  * @param state 状态
  * @retval 0：成功 其他：失败
  */
int ip5306_charge_sta_get(ip5306_chg_sta_et* state)
{
  uint8_t val;

  if(reg_read(0x70, &val))
    return -1;

  if(val & (1 << 3))
  {
    if(reg_read(0x71, &val))
    {
      return -2;
    }

    if(val & (1 << 3))
    {
      *state = IP5306_CHG_STA_FULL;
      return 0;
    }
  }
  else
  {
    *state = IP5306_CHG_STA_DISCHARGE;
    return 0;
  }

  *state = IP5306_CHG_STA_CHARGE;
  return 0;
}

/**
  * @brief 按键状态获取
  * @retval bit0:短按 bit1:双击 bit3:长按
  */
uint8_t ip5306_key_get(void)
{
  uint8_t val;

  if(reg_read(0x77, &val))
    return 0;

  val &= 0x07;

  if(val)
    reg_write(0x77, val);

  return val;
}

/**
  * @brief 电量获取
  * @retval 0~100
  */
uint8_t ip5306_lvl_get(void)
{
  uint8_t val, lvl;

  if(reg_read(0x78, &val))
  {
    hw_i2c_fix(hw_i2c2, 10);
    return 0;
  }

  switch(val)
  {
    case 0x00:
      lvl = 99;break;
    case 0x80:
      lvl = 75;break;
    case 0xc0:
      lvl = 50;break;
    case 0xe0:
      lvl = 25;break;
    case 0xf0:
      lvl = 1;break;
    default:
      lvl = 0;break;
  }

  return lvl;
}

/**
 * @brief ip5306初始化
 * 
 * @return int 错误码
 */
int ip5306_init(void)
{
  uint8_t val;

  if(reg_read(0x00, &val))
    return -1;

  //关闭负载检测
  if(val & (1 << 2))
  {
    val &= ~(1 << 2);
    if(reg_write(0x00, val))
      return -2;
  }

  //chg_dig_ctl0 default:0xD6
  if(reg_read(0x01, &val))
    return -3;
  //设置长按关升压
  val |= 0x80;
  //VIN拔出后不开启升压
  val &= (~0x04);
  //VIN拔出后开启升压
  //val |= (0x04);
  //短按不能开关boost
  val &= (~(1 << 5));
  if(reg_write(0x01, val))
    return -4;

  //reg_read(0x77, &val);
  //soutf("IP5306_KEY=%02X.\n", val);

  //清除按键状态
  if(reg_write(0x77, 0x07))
    return -5;

  return 0;
}

/**
 * @brief ip5306关闭升压
 * 
 * @return int 错误码
 */
int ip5306_boost_disable(void)
{
  uint8_t val;

  //使能短按开关boost
  if(reg_read(0x01, &val))
    return -1;

  val |= (1 << 5);

  if(reg_write(0x01, val))
    return -2;

  //关闭boost
  if(reg_read(0x00, &val))
    return -1;

  val &= (~(1 << 5));

  if(reg_write(0x00, val))
    return -2;

  return 0;
}
