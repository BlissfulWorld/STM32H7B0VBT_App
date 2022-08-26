#include "stdio.h"
#include "string.h"
#include "config.h"
#include "i2c.h"
#include "cmsis_os.h"
#include "test.h"

typedef struct
{
	uint8_t data1[1];
	uint8_t data2[30];
}prox_data_st;

#define PROX_I2C_ID (0xB0)

static uint8_t prox_rdy;
static prox_data_st prox_data;
static const uint8_t prox_init_data[];

static const test_1_interface_st* intf;

static uint8_t i2c_write_nd(uint8_t* pdata, uint32_t size)
{
	return intf->i2c_write_read(PROX_I2C_ID, pdata, size, 0, 0, size);
}

static uint8_t i2c_read(unsigned char addr, unsigned char* data, unsigned int size)
{
	return intf->i2c_write_read(PROX_I2C_ID, &addr, 1, data, size, size);
}

void test_1_prox_set_intf(const test_1_interface_st* i)
{
	intf = i;
}

/**
	*	@描述 Prox Sensor初始化
	*	@参数	空
	* @返回	空
	*/
void test_1_prox_init(void)
{
	prox_rdy = 0;

	memset(&prox_data, 0, sizeof(prox_data));

	GPIO_InitTypeDef gpio_init;

	gpio_init.Alternate = 0;
	gpio_init.Pull = GPIO_PULLUP;
	gpio_init.Speed = GPIO_SPEED_LOW;
	gpio_init.Mode = GPIO_MODE_INPUT;
	gpio_init.Pin = test_pin[intf->prox_inti_pin].pin;
	HAL_GPIO_Init(test_pin[intf->prox_inti_pin].gpio, &gpio_init);
}

/**
	*	@描述 检测Prox Sensor
	*	@参数	空
	* @返回	0:检测到Prox Sensor	!0:未检测到Prox Sensor
	*/
uint8_t test_1_prox_detect(void)
{
	uint8_t ReadVal;

	return i2c_read(0x00, &ReadVal, 1);
}

/**
	* @描述	距离传感器配置
	*/
uint8_t test_1_prox_config(void)
{
	uint8_t ret, ReadData[15], nData;
	uint8_t ReadWrite;
	uint16_t i, z;

	for(i = 0, z = 0;prox_init_data[i] != 0xff;z++)
	{
		/*-- 获取数据长度， --*/
		nData = prox_init_data[i] & 0x0f;

		/*-- 数据操作方式 --*/
		ReadWrite = prox_init_data[i] & 0xf0;

		switch(ReadWrite)
		{
			case 0x10:
				/*-- 写数据，数据长度加上地址字节 --*/
				ret = i2c_write_nd((uint8_t*)&prox_init_data[i + 1], nData + 1);
				if(ret != 0)
					return (z);

				i += (nData + 2);
				break;
			case 0x20:
				/*-- 读数据 --*/
				ret = i2c_read(prox_init_data[i + 1], ReadData, nData);
				if(ret != 0)
					return (0x80 | z);

				i += 2;
				break;
			default:
				return (z);
		}
		osDelay(1);
	}

	prox_rdy = 1;

	return 0;
}

/**
	*	@描述	接收距离传感器数据
	*	@参数	距离传感器数据数据类型指针
	*	@返回	!0:有错误	0:读取成功
	*/
static uint8_t prox_receive_data(prox_data_st* Data)
{
	uint8_t ret;
	uint32_t t;

	uint8_t reg70[3] = { 0x70,0xE0,0x16 };
	ret = i2c_write_nd(reg70, 3);
	if(ret != 0) return 1;

	t = HAL_GetTick();

	while((HAL_GPIO_ReadPin(test_pin[intf->prox_inti_pin].gpio, test_pin[intf->prox_inti_pin].pin) != GPIO_PIN_RESET) && (HAL_GetTick() - t <= 10));
	if(HAL_GetTick() - t > 50)
		return 1;
	osDelay(5);

	ret = i2c_read(0x94, Data->data1, 1);
	if(ret != 0) return 2;

	osDelay(1);

	ret = i2c_read(0x76, Data->data2, 30);
	if(ret != 0) return 3;

	return 0;
}

/**
	*	@描述	判断Prox Sensor是否就绪
	* @返回	NO_RDY:未就绪	AL_RDY:已就绪 RDY_ING:就绪中
	*/
uint8_t test_1_prox_is_rdy(void)
{
	return prox_rdy;
}

/**
	*	@描述 更新PROX数据
	*/
void test_1_prox_update_data(void)
{
	if(prox_rdy != 0)
	{
		if(prox_receive_data(&prox_data) != 0)
		{
			prox_data.data2[1] = 0;
			prox_data.data2[2] = 0;
			prox_rdy = 0;
		}
	}
	else
	{
		if((test_1_prox_detect() == 0) && (test_1_prox_config() == 0))
		{

		}
	}
}

uint16_t test_1_prox_get_percentage(void)
{
	int32_t val1 = (int16_t)(prox_data.data2[1] << 8) | prox_data.data2[2];
	int32_t val2 = (int16_t)(prox_data.data2[5] << 8) | prox_data.data2[6];

	int16_t val = ((val1 * val2) / (14000000 / 100));

	if(val < 0) val = 0;
	else if(val > 100) val = 100;

	return val;
}

/**
	* 高4位表示读写，2读 1写
	* 低4位表示读写数据长度，最高15个数据
	* 数据读写地址8位
	* 0xff表示结束
	*/
static const uint8_t prox_init_data[] = {
0x11,0x71,0x00,
0x11,0x11,0x03,
0x12,0x6D,0x00,0x0D,
0x12,0x10,0x00,0x00,
0x11,0x04,0x18,
0x21,0x00,
0x11,0xFE,0x01,
0x11,0xFE,0x00,
0x11,0x04,0x18,
0x11,0x0D,0x0C,
0x11,0x16,0x04,
0x11,0x03,0x06,
0x11,0x75,0x00,
0x11,0xC1,0x00,
0x11,0x71,0x00,
0x11,0x14,0x01,
0x11,0x0C,0x0E,
0x11,0xC5,0x01,
0x11,0x09,0x1A,
0x16,0x4E,0x00,0x00,0x00,0x00,0x00,0x00,
0x16,0x48,0x00,0x00,0x00,0x20,0xD5,0x01,
0x16,0x54,0x00,0x00,0x00,0x00,0x00,0x00,
0x19,0x36,0xFF,0xFD,0x7F,0xF7,0xFF,0xFF,0xFF,0x7F,0xFF,
0x19,0x3F,0x7B,0xDF,0x7E,0x7F,0xFF,0xFB,0xAF,0xFF,0xFF,
0x11,0x34,0x43,
0x11,0x0F,0x90,
0x11,0x35,0x07,
0x11,0x05,0x01,
0x22,0x94,
0x11,0x05,0x09,
0x11,0x09,0x18,
0x11,0xC5,0x00,
0x11,0x0C,0x00,
0x11,0x14,0x00,
0x11,0x70,0xFF,
0x11,0x71,0x04,
0x11,0x0C,0x0E,
0x11,0xC5,0x01,
0x11,0x07,0x01,
0x11,0x08,0x01,
0x11,0x5A,0x01,
0x11,0x02,0x02,
0x11,0x13,0x00,
0x11,0x0E,0x0B,
0x11,0x06,0x05,
0x11,0x24,0x03,
0x11,0x25,0x03,
0x11,0xC4,0x45,
0x11,0x18,0x03,
0x11,0x17,0x3E,
0x11,0x5D,0x02,
0x11,0x5E,0x05,
0x11,0x5F,0x01,
0x11,0x62,0x02,
0x11,0x63,0x02,
0x11,0x60,0x05,
0x11,0x61,0x05,
0x12,0x6B,0x09,0x82,
0x12,0x68,0x09,0x82,
0x12,0x6D,0x02,0x97,
0x11,0x64,0x0C,
0xff
};
