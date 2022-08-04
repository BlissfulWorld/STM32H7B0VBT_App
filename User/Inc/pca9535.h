#ifndef _PCA9535_H
#define _PCA9535_H

#include "stdint.h"

int pca9535_config_port(uint32_t chip, uint16_t config);
int pca9535_set_output(uint32_t chip, uint16_t state);
int pca9535_get_output(uint32_t chip, uint16_t* output);

#endif
