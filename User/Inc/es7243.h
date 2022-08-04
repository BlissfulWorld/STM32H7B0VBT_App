#ifndef _ES7243_H
#define _ES7243_H

#include "stdint.h"

typedef struct es7243_data_s
{
  int16_t l;
  int16_t r;
}es7243_data_st;

int es7243_start(uint32_t rate, void (*data_isr_cb)(es7243_data_st* pdata, uint32_t len));
int es7243_stop(void);
es7243_data_st* es7243_get_current_value(void);

#endif
