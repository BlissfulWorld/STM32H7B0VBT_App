#ifndef _BACKLIGHT_H
#define _BACKLIGHT_H

#include "stdint.h"

void backlight_set(uint32_t level);
uint32_t backlight_get(void);
void backlight_time_set(uint32_t t);
uint32_t backlight_time_get(void);

void backlight_open(void);
void backlight_close(void);

#endif
