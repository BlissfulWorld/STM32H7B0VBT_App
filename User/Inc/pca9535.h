#ifndef _PCA9535_H
#define _PCA9535_H

#include "stdint.h"

int pca9535_config_port(uint32_t chip,uint16_t config);
int pca9535_config_output(uint32_t chip,uint16_t state);

#endif
