#ifndef _RESOURCE_H
#define _RESOURCE_H

#include "stdint.h"

typedef struct resource_s
{
  uint32_t size;
  uint8_t data[0];
}resource_st;

extern const char* audio_file_board_loaded;
extern const char* audio_file_warning;

resource_st* resource_icon_load(const char* name);
resource_st* resource_logo_load(void);

int resource_unload(resource_st* res);

extern const uint8_t res_image_dta_speaker[84];

#endif
