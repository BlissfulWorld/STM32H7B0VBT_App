#ifndef _DIGITAL_H
#define _DIGITAL_H

#include "stdint.h"

int digital_mic_init(void);
int digital_mic_start(uint32_t rate);
int digital_mic_stop(void);
int digital_mic_record_start(void* buffer, uint32_t size, void (*finished_cb)(void));
int digital_mic_record_stop(void);
uint32_t digital_mic_record_get_count(void);
uint8_t digital_mic_get_current_value(void);

#endif
