#ifndef _ANALOG_MIC_H
#define _ANALOG_MIC_H

#include "stdint.h"

int analog_mic_start(void);
int analog_mic_stop(void);
int analog_mic_record_start(void* buffer, uint32_t size, void (*finished_cb)(void));
int analog_mic_record_stop(void);
uint32_t analog_mic_record_get_count(void);
uint8_t analog_mic_get_current_value(void);

#endif
