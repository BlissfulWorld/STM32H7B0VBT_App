#include "stdio.h"
#include "math.h"
#include "config.h"
#include "i2c.h"
#include "cmsis_os.h"
#include "test.h"

#define ALS_I2C_ID 0x52

static uint32_t als_data = 0;
static uint8_t als_rdy = 0;
static const test_0_interface_st *intf;

static uint8_t i2c_write_8a_8d(uint8_t id, uint8_t addr, uint8_t data)
{
  uint8_t buffer[2];
  buffer[0] = addr;
  buffer[1] = data;

  return intf->als_i2c_write_read(buffer,2,0,0,100);
}

static uint8_t i2c_write_8a_16d(uint8_t id, uint8_t addr, uint16_t data)
{
  uint8_t buffer[3];
  buffer[0] = addr;
  buffer[1] = data >> 8;
  buffer[2] = data;

  return intf->als_i2c_write_read(buffer,3,0,0,100);
}

static uint8_t i2c_read(uint8_t id, uint8_t addr, uint8_t* data, uint32_t size)
{
  return intf->als_i2c_write_read(&addr,1,data,size,size);
}

void test_0_als_set_intf(const test_0_interface_st *i)
{
  intf = i;
}

/**
	*	@描述 获得环境光传感器百分比值，非线性，y = sin(sprt(x) * Pi * (1/2)) 曲线
	*	@参数	空
	* @返回	百分比
	*/
uint8_t test_0_als_get_percentage(void)
{
	return ((als_data == 0) ? (0) : ((8 + 91) * (sin((sqrt((als_data / 100) / (0x4400FFFF / 1000000))) / 200 * 3.14) + 0.0000001) + 1));
}

/**
	*	@描述 检测ALS
	*	@参数	空
	* @返回	0:检测到ALS		!0:未检测到ALS
	*/
uint8_t test_0_als_detect(void)
{
	uint8_t ReadVal;
	return i2c_read(ALS_I2C_ID, 0x92, &ReadVal, 1);
}

/**
	*	@描述 ALS初始化
	*	@参数	空
	* @返回	空
	*/
void test_0_als_init(void)
{
	als_data = 0;

	GPIO_InitTypeDef gpio_init;

	gpio_init.Alternate = 0;
	gpio_init.Pull = GPIO_PULLUP;
	gpio_init.Speed = GPIO_SPEED_LOW;
	gpio_init.Mode = GPIO_MODE_INPUT;
	gpio_init.Pin = test_pin[intf->als_inti_pin].pin;
	HAL_GPIO_Init(test_pin[intf->als_inti_pin].gpio, &gpio_init);
}

/**
	*	@描述 ALS配置
	*	@参数	空
	* @返回	0:成功	!0:失败
	*/
uint8_t test_0_als_config(void)
{
	uint8_t ReadVal;
	uint8_t ret;

	ret = i2c_read(ALS_I2C_ID, 0x92, &ReadVal, 1);
	if(ret != 0) return 0xf1;

	osDelay(1);

	//0x8F:IPHONE8P
	//0x9B:IPHONE7_7P
	if((ReadVal != 0x8F) && (ReadVal != 0x9B)) return 0xff;

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0x80, 0x03);
	if(ret != 0) return 1;

	osDelay(1);

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0x80, 0x01);
	if(ret != 0) return 2;

	osDelay(1);

	ret = i2c_write_8a_16d(ALS_I2C_ID, 0x84, 0x0000);
	if(ret != 0) return 3;

	osDelay(1);

	ret = i2c_write_8a_16d(ALS_I2C_ID, 0x86, 0x0000);
	if(ret != 0) return 4;

	osDelay(1);

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0x8C, 0x40);
	if(ret != 0) return 5;

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0x8C, 0x40);
	if(ret != 0) return 6;

	osDelay(1);

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0x81, 0xEF);
	if(ret != 0) return 7;

	osDelay(1);

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0x8F, 0x07);
	if(ret != 0) return 8;

	osDelay(1);

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0x80, 0x13);
	if(ret != 0) return 9;

	//ALS_INT_L();

	osDelay(1);

	ret = i2c_read(ALS_I2C_ID, 0x94, &ReadVal, 1);
	if(ret != 0) return ret;

	osDelay(1);

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0x81, 0xef);
	if(ret != 0) return 10;

	osDelay(1);

	ret = i2c_read(ALS_I2C_ID, 0x95, &ReadVal, 1);
	if(ret != 0) return 11;

	osDelay(10);

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0x8c, 0x40);
	if(ret != 0) return 12;

	osDelay(1);

	ret = i2c_read(ALS_I2C_ID, 0x96, &ReadVal, 1);
	if(ret != 0) return 13;

	osDelay(1);

	ret = i2c_write_8a_16d(ALS_I2C_ID, 0x84, 0x0000);
	if(ret != 0) return 14;

	osDelay(3);

	ret = i2c_read(ALS_I2C_ID, 0x97, &ReadVal, 1);
	if(ret != 0) return 15;

	osDelay(10);

	ret = i2c_write_8a_16d(ALS_I2C_ID, 0x86, 0x0000);
	if(ret != 0) return 16;

	osDelay(18);

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0xc1, 0x00);
	if(ret != 0) return 17;

	//ALS_INT_H();
	osDelay(10);

	//ret = i2c_read(0x42,0x14,&ReadVal,1);
	//if(ret != 0) return 15;

	als_rdy = 1;

	return 0;
}

/**
	*	@描述	接收ALS数据
	*	@参数	data:存储接收到的数据的变量
	* @返回	0:成功	!0:失败
	*/
static uint8_t test_0_als_receive_data(uint32_t* data)
{
	uint8_t Temp, ret = 0;

	*data = 0;

	//ALS_INT_L();

	osDelay(1);

	//if(GPIO_ReadInputDataBit(ALS_INT_PORT,ALS_INT_PINN) != Bit_RESET)
	//  return 1;
	if(HAL_GPIO_ReadPin(test_pin[intf->als_inti_pin].gpio, test_pin[intf->als_inti_pin].pin) != GPIO_PIN_RESET)
		return 1;

	ret = i2c_read(ALS_I2C_ID, 0x94, &Temp, 1);
	if(ret != 0) { ret = 2; goto __test_0_als_receive_data_exit; }

	*data |= Temp;

	osDelay(1);

	ret = i2c_read(ALS_I2C_ID, 0x95, &Temp, 1);
	if(ret != 0) { ret = 3; goto __test_0_als_receive_data_exit; }

	*data |= ((uint32_t)Temp << 8);

	osDelay(1);

	ret = i2c_read(ALS_I2C_ID, 0x96, &Temp, 1);
	if(ret != 0) { ret = 4; goto __test_0_als_receive_data_exit; }

	*data |= ((uint32_t)Temp << 16);

	osDelay(1);

	ret = i2c_read(ALS_I2C_ID, 0x97, &Temp, 1);
	if(ret != 0) { ret = 5; goto __test_0_als_receive_data_exit; }

	*data |= ((uint32_t)Temp << 24);

	osDelay(1);

	ret = i2c_write_8a_8d(ALS_I2C_ID, 0xc1, 0x00);
	if(ret != 0) { ret = 6; goto __test_0_als_receive_data_exit; }

__test_0_als_receive_data_exit:
	//ALS_INT_H();

	return ret;
}

/**
	*	@描述	判断ALS是否就绪
	* @返回	0:未就绪	1:已就绪 RDY_ING:就绪中
	*/
uint8_t test_0_als_is_rdy(void)
{
	return als_rdy;
}

/**
	*	@描述	更新ALS数据
	*/
void test_0_als_update_data(void)
{
	static uint32_t tick;
	if(als_rdy != 0)
	{
		uint32_t data;
		uint8_t ret = test_0_als_receive_data(&data);
		if(ret > 1)
		{
			als_data = 0;
			als_rdy = 0;
		}
		else if(ret == 0)
		{
			als_data = data;
		}
		else if(HAL_GetTick() - tick > 200)
		{
			als_rdy = 0;
			return;
		}

		tick = HAL_GetTick();
	}
	else
	{
		if((test_0_als_detect() == 0) && (test_0_als_config() == 0))
		{
			tick = HAL_GetTick();
		}
	}
}
