#ifndef _PWDN_WATCHDOG_H
#define _PWDN_WATCHDOG_H

#include "stdint.h"

int pwdn_watchdog_start(uint32_t ms);
int pwdn_watchdog_set_timeout(uint32_t ms);
int pwdn_watchdog_get_timeout(uint32_t* ms);
int pwdn_watchdog_reload(void);

#endif
