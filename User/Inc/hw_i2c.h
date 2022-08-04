#ifndef _HW_I2C_H
#define _HW_I2C_H

#include "stdint.h"

void hw_i2c_init(void);
int hw_i2c_fix(const void* handler, uint32_t timeout);
int hw_i2c_write_read(const void* handler, uint8_t addr, const uint8_t* pw, uint32_t wlen, uint8_t* pr, uint32_t rlen, uint32_t timeout);
int hw_i2c_write_1a_xd(const void* handler, uint8_t id, uint8_t addr, const uint8_t* pw, uint32_t wlen, uint32_t timeout);
int hw_i2c_write_2a_xd(const void* handler, uint8_t id, uint16_t addr, const uint8_t* pw, uint32_t wlen, uint32_t timeout);
int hw_i2c_read_2a_xd(const void* handler, uint8_t id, uint16_t addr, uint8_t* pr, uint32_t rlen, uint32_t timeout);
int hw_i2c_read_1a_1d(const void* handler, uint8_t id, uint8_t addr, uint8_t* pr, uint32_t timeout);

extern void* hw_i2c1, * hw_i2c2, * hw_i2c4;

#endif
