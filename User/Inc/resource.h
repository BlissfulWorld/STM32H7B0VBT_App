#ifndef _RESOURCE_H
#define _RESOURCE_H

#include "stdint.h"

typedef struct resource_icon_s
{
  uint32_t size;
  uint8_t data[0];
}resource_icon_st;

resource_icon_st* resource_icon_load(const char *name);
int resource_icon_unload(resource_icon_st *icon);

#endif
