#ifndef _SPIFLASH_H
#define _SPIFLASH_H

#include "stdint.h"

int spiflash_product_id_read(uint8_t *manufacturer,uint8_t *device);
int spiflash_uid_read(uint8_t *uid);
int spiflash_write(uint32_t address,const void *pdata,uint32_t size,uint32_t timout);
int spiflash_read(uint32_t address,void *pdata,uint32_t size);
int spiflash_erase_sector(uint32_t address,uint32_t timout);
uint32_t spiflash_sector_num_get(uint8_t device);

#endif
