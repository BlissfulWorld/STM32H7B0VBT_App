#ifndef _MIC_H
#define _MIC_H

#include "stdint.h"

int mic_init(void);
int mic_deinit(void);
uint8_t mic_get_percentage(void);
void mic_update_data(void);
int mic_record_start(void);
int mic_record_stop(void);
uint8_t mic_record_get_progress(void);
int mic_record_play(void);

#endif
