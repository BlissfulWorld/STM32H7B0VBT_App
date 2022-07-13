#ifndef _HW_H
#define _HW_H

#include "hw_i2c.h"
#include "hw_sim_i2c.h"

void hw_init(void);
void hw_delay_100ns(uint32_t n_100ns);

#endif
