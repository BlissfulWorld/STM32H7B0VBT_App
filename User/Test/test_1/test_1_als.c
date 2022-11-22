#include "test_1.h"
#include "stdio.h"
#include "math.h"
#include "cmsis_os.h"

#define ALS_I2C_ID (0x52)

static uint8_t als_data = 0;
static uint8_t als_rdy = 0;
static const test_1_interface_st* intf;

static const uint8_t config_data[] =
{
	0x86,0x04,
	0x89,0x00,
	0x88,0xA7,
	0x87,0x08,
	0x89,0x23,
	0x89,0x00,
	0x88,0xDD,
	0x87,0x09,
	0x89,0x23,
};

static uint8_t i2c_write_8a_8d(uint8_t addr, uint8_t data)
{
	uint8_t buffer[2];
	buffer[0] = addr;
	buffer[1] = data;

	return intf->i2c_write_read(ALS_I2C_ID, buffer, 2, 0, 0, 100);
}

static uint8_t i2c_write_8a_16d(uint8_t addr, uint16_t data)
{
	uint8_t buffer[3];
	buffer[0] = addr;
	buffer[1] = data >> 8;
	buffer[2] = data;

	return intf->i2c_write_read(ALS_I2C_ID, buffer, 3, 0, 0, 100);
}

static uint8_t i2c_read(uint8_t addr, uint8_t* data, uint32_t size)
{
	return intf->i2c_write_read(ALS_I2C_ID, &addr, 1, data, size, size);
}

void test_1_als_set_intf(const test_1_interface_st* i)
{
	intf = i;
}

/**
	*	@描述 获得环境光传感器百分比值，非线性，y = sin(sprt(x) * Pi * (1/2)) 曲线
	*	@参数	空
	* @返回	百分比
	*/
uint8_t test_1_als_get_percentage(void)
{
	return als_data;
}

/**
	*	@描述 ALS初始化
	*	@参数	空
	* @返回	空
	*/
void test_1_als_init(void)
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
uint8_t test_1_als_config(void)
{
	uint32_t i;
	for(i = 0;i < sizeof(config_data);i += 2)
	{
		if(i2c_write_8a_8d(config_data[i], config_data[i + 1]) != HAL_OK)
			return 1;
	}

	als_rdy = 1;

	return 0;
}

/**
	*	@描述 检测ALS
	*	@参数	空
	* @返回	0:检测到ALS		!0:未检测到ALS
	*/
uint8_t test_1_als_detect(void)
{
	uint8_t data;
	return i2c_read(0x00, &data, 1);
}

/**
	*	@描述	接收ALS数据
	*	@参数	data:存储接收到的数据的变量
	* @返回	0:成功	!0:失败
	*/
static uint8_t test_1_als_receive_data(uint8_t* data)
{
	uint8_t data0[13];

	if(HAL_GPIO_ReadPin(test_pin[intf->als_inti_pin].gpio, test_pin[intf->als_inti_pin].pin) != GPIO_PIN_RESET)
		return 1;

	if(i2c_read(0x8A, data0, 13) != 0)
		return -1;

	*data = data0[2];

	if(data0[0] != 0x07)
	{
		if(*data > 100)
			*data = 100;
	}
	else
	{
		*data = 100;
	}

	return 0;
}

/**
	*	@描述	判断ALS是否就绪
	* @返回	0:未就绪	1:已就绪 RDY_ING:就绪中
	*/
uint8_t test_1_als_is_rdy(void)
{
	return als_rdy;
}

/**
	*	@描述	更新ALS数据
	*/
void test_1_als_update_data(void)
{
	static uint32_t tick;
	if(als_rdy != 0)
	{
		uint8_t data;
		uint8_t ret = test_1_als_receive_data(&data);
		if(ret > 1)
		{
			als_data = 0;
			als_rdy = 0;
		}
		else if(ret == 0)
		{
			als_data = data;
		}
		else if(HAL_GetTick() - tick > 100)
		{
			als_rdy = 0;
			return;
		}

		tick = HAL_GetTick();
	}
	else
	{
		if(test_1_als_config() == 0)
		{
			tick = HAL_GetTick();
		}
	}
}
