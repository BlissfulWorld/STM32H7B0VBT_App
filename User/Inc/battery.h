#ifndef _BATTERY_H
#define _BATTERY_H

#include "stdint.h"

/**
 * @brief 电池状态枚举
 * 
 */
typedef enum battery_status_e
{
  BAT_STA_DISCHARGE,
  BAT_STA_CHARGE,
  BAT_STA_FULL,
}battery_status_et;

void battery_init(void);
void battery_set_changed_cb(void (*cb)(uint8_t lvl, battery_status_et status));
uint8_t battery_get_level(void);
battery_status_et battery_get_status(void);

#endif
