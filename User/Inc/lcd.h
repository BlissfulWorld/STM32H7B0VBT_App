#ifndef _LCD_H
#define _LCD_H

#include "stdint.h"

#define LCD_WIDTH  (320)
#define LCD_HEIGHT (240)

void lcd_write_cmd(uint8_t cmd);
void lcd_write_data(uint8_t data);

int lcd_init(void);
void lcd_test(void);

void lcd_pixel_set(uint16_t x,uint16_t y,uint16_t color);
void lcd_on(void);

#endif
