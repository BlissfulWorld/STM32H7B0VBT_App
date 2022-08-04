#ifndef _BACKLIGHT_H
#define _BACKLIGHT_H

#include "stdint.h"

void backlight_set(uint8_t level);
uint8_t backlight_get(void);

void backlight_open(void);
void backlight_close(void);

#endif
