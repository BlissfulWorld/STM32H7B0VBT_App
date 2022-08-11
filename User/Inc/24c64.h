#ifndef _24C64_H
#define _24C64_H

#include "stdint.h"

uint32_t _24c64_write(uint16_t addr, void* pdata, uint16_t len);
uint32_t _24c64_read(uint16_t addr, void* pbuffer, uint16_t len);

#endif
