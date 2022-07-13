#include "ina219.h"
#include "i2c.h"

#define INA219_I2C_ID (0x80)

static int ina219_reg_write(uint8_t reg,uint16_t val)
{
  uint8_t data[2];
  data[0] = val >> 8;
  data[1] = val;
  return HAL_I2C_Mem_Write(&hi2c2,INA219_I2C_ID,reg,I2C_MEMADD_SIZE_8BIT,data,2,10);
}

static int ina219_reg_read(uint8_t reg,uint16_t *val)
{
  uint8_t data[2];
  if(HAL_I2C_Mem_Read(&hi2c2,INA219_I2C_ID,reg,I2C_MEMADD_SIZE_8BIT,data,2,10))
    return 1;
  
  *val = ((uint16_t)data[0] << 8) | (uint16_t)data[1];
  
  return 0;
}

int ina219_init(void)
{
  uint16_t config = INA219_CONFIG_BVOLTAGERANGE_32V |
					INA219_CONFIG_GAIN_8_320MV |
					INA219_CONFIG_BADCRES_12BIT |
					INA219_CONFIG_SADCRES_12BIT_1S_532US |
					INA219_CONFIG_MODE_SANDBVOLT_CONTINUOUS;
  ina219_reg_write(0x05,4096);
  ina219_reg_write(0x00,config);
  return 0;
}

int ina219_convert_start(void)
{
  //ina219_reg_write(0x00,0x3C1F);
  return ina219_reg_write(0x05,10240);
}

int ina219_convert_is_ready(void)
{
  uint16_t val;
  if((ina219_reg_read(0x02,&val) == 0) && (val & 0x02))
    return 1;
  
  return 0;
}

int ina219_ma_get(uint16_t *ma)
{
  int ret;
  uint16_t val;
  
  //ret = ina219_reg_read(0x02,&val);
  ret = ina219_reg_read(0x04,&val);
  if(ret == 0)
  {
    *ma = val * 0.1;
  }
  //ina219_reg_read(0x03,&val);
  return 0;
}