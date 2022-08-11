#ifndef _IP5306_H
#define _IP5306_H

#include "stdint.h"

typedef enum ip5306_chg_sta_e
{
  IP5306_CHG_STA_DISCHARGE,
  IP5306_CHG_STA_CHARGE,
  IP5306_CHG_STA_FULL,
}ip5306_chg_sta_et;

int ip5306_init(void);
uint8_t ip5306_key_get(void);

uint8_t ip5306_lvl_get(void);
int ip5306_charge_sta_get(ip5306_chg_sta_et* state);

int ip5306_boost_disable(void);
#endif
