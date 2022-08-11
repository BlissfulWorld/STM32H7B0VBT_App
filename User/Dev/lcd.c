#include "lcd.h"
#include "main.h"
#include "spi.h"
#include "stdint.h"
#include "backlight.h"

#define LCD_IO_SIM (0)
#define LCD_CS_SOFT (0)

#define LCD_RST_SET() HAL_GPIO_WritePin(LCD_RESET_GPIO_Port,LCD_RESET_Pin,GPIO_PIN_SET)
#define LCD_RST_CLR() HAL_GPIO_WritePin(LCD_RESET_GPIO_Port,LCD_RESET_Pin,GPIO_PIN_RESET)

#define LCD_DC_SET() HAL_GPIO_WritePin(LCD_WR_GPIO_Port,LCD_WR_Pin,GPIO_PIN_SET)
#define LCD_DC_CLR() HAL_GPIO_WritePin(LCD_WR_GPIO_Port,LCD_WR_Pin,GPIO_PIN_RESET)

void lcd_write_cmd(uint8_t cmd)
{
	LCD_DC_CLR();
	HAL_SPI_Transmit(&hspi2, &cmd, 1, 100);
	LCD_DC_SET();
}

void lcd_write_data(uint8_t data)
{
	HAL_SPI_Transmit(&hspi2, &data, 1, 100);
}

void lcd_fill(uint16_t color)
{
	uint32_t i, j;

	lcd_write_cmd(0x2A);
	lcd_write_data(0);
	lcd_write_data(0);
	lcd_write_data(0);
	lcd_write_data(239);

	lcd_write_cmd(0x2B);
	lcd_write_data(0);
	lcd_write_data(0);
	lcd_write_data(319 >> 8);
	lcd_write_data(319 & 0xFF);

	lcd_write_cmd(0x2C);

	for(i = 0;i < 320;i++)
	{
		for(j = 0;j < 240;j++)
		{
			lcd_write_data(color >> 8);
			lcd_write_data(color);
		}
	}

}

void lcd_pixel_set(uint16_t x, uint16_t y, uint16_t color)
{
	lcd_write_cmd(0x2A);
	lcd_write_data(0);
	lcd_write_data(y);
	lcd_write_data(0);
	lcd_write_data(y);

	lcd_write_cmd(0x2B);
	lcd_write_data(x >> 8);
	lcd_write_data(x & 0xFF);
	lcd_write_data(x >> 8);
	lcd_write_data(x & 0xFF);

	lcd_write_cmd(0x2C);

	lcd_write_data(color >> 8);
	lcd_write_data(color);

}

void lcd_on(void)
{
	lcd_write_cmd(0x29);                                             //显示开，等待MCU数传送
}

int lcd_init(void)
{
	LCD_DC_SET();
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = LCD_WR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(LCD_WR_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LCD_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
	HAL_GPIO_Init(LCD_SDA_GPIO_Port, &GPIO_InitStruct);

	LCD_RST_CLR();
	HAL_Delay(10);
	LCD_RST_SET();
	HAL_Delay(100);

	lcd_write_cmd(0x11);			//Sleep out
	HAL_Delay(120);

	lcd_write_cmd(0x36);
	lcd_write_data(0x00);

	lcd_write_cmd(0x3A);                                             //数据格式，65K色,565
	lcd_write_data(0x05);

	lcd_write_cmd(0xB2);                                             //帧频设置
	lcd_write_data(0x0C);
	lcd_write_data(0x0C);
	lcd_write_data(0x00);
	lcd_write_data(0x33);
	lcd_write_data(0x33);

	lcd_write_cmd(0xB7);                                             //GATE 设置
	lcd_write_data(0x35);

	lcd_write_cmd(0xBB);                                             //VCOM设置 
	lcd_write_data(0x19);

	lcd_write_cmd(0xC0);                                             //LCM设置,默认0x2c
	lcd_write_data(0x2C);

	lcd_write_cmd(0xC2);                                             //VDV&VRH SET ,默认0x01
	lcd_write_data(0x01);

	lcd_write_cmd(0xC3);                                             //VRHS SET，默认0x0b
	lcd_write_data(0x12);                                           //此处根据实际情况修正

	lcd_write_cmd(0xC4);                                             //VDV SET，默认0x20
	lcd_write_data(0x20);

	lcd_write_cmd(0xC6);                                             // FR SET, 默认0x0F
	lcd_write_data(0x0F);

	lcd_write_cmd(0xD0);                                             //电源控制1
	lcd_write_data(0xA4);                                           //该参数不变  
	lcd_write_data(0xA1);                                           //此处根据实际情况修改

	lcd_write_cmd(0xE0);                                             //正极性GAMMA调整 
	lcd_write_data(0xD0);
	lcd_write_data(0x04);
	lcd_write_data(0x0D);
	lcd_write_data(0x11);
	lcd_write_data(0x13);
	lcd_write_data(0x2B);
	lcd_write_data(0x3F);
	lcd_write_data(0x54);
	lcd_write_data(0x4C);
	lcd_write_data(0x18);
	lcd_write_data(0x0D);
	lcd_write_data(0x0B);
	lcd_write_data(0x1F);
	lcd_write_data(0x23);

	lcd_write_cmd(0xE1);                                              //负极性GAMMA调整
	lcd_write_data(0xD0);
	lcd_write_data(0x04);
	lcd_write_data(0x0C);
	lcd_write_data(0x11);
	lcd_write_data(0x13);
	lcd_write_data(0x2C);
	lcd_write_data(0x3F);
	lcd_write_data(0x44);
	lcd_write_data(0x51);
	lcd_write_data(0x2F);
	lcd_write_data(0x1F);
	lcd_write_data(0x1F);
	lcd_write_data(0x20);
	lcd_write_data(0x23);

	lcd_write_cmd(0x21);                                             //反显开，默认是0X20，正常模式
	return 0;
}

void lcd_test(void)
{
	while(1)
	{
		lcd_fill(0xF800);
		HAL_Delay(1000);
		lcd_fill(0x07E0);
		HAL_Delay(1000);
		lcd_fill(0x001F);
		HAL_Delay(1000);
	}
}
