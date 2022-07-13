#ifndef _HW_SIM_I2C_H
#define _HW_SIM_I2C_H

#include "hw_pin.h"

void hw_sim_i2c_init(void);
int hw_sim_i2c_set_pin(void *handler,const hw_pin_st *scl,const hw_pin_st *sda,uint32_t timeout);
int hw_sim_i2c_write_read(const void *handler,uint8_t addr,const uint8_t* pw,uint32_t wlen,uint8_t* pr,uint32_t rlen,uint32_t timeout);

extern void *hw_sim_i2c1;

#endif
