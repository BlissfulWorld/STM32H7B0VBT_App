#include "sw_info.h"
#include "config.h"

#define YEAR ((((__DATE__[7] - '0') * 10 + (__DATE__[8] - '0')) * 10 + (__DATE__[9] - '0')) * 10 + (__DATE__[10] - '0'))

#define MONTH (\
 __DATE__[2] == 'n' ? 1\
:__DATE__[2] == 'b' ? 2\
:__DATE__[2] == 'r' ? (__DATE__[0] == 'M' ? 3 : 4)\
:__DATE__[2] == 'y' ? 5\
:__DATE__[2] == 'n' ? 6\
:__DATE__[2] == 'l' ? 7\
:__DATE__[2] == 'g' ? 8\
:__DATE__[2] == 'p' ? 9\
:__DATE__[2] == 't' ? 10\
:__DATE__[2] == 'v' ? 11 : 12)

#define DAY ((__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 + (__DATE__[5] - '0'))

#define HOUR   ((__TIME__[0] - '0') * 10 + (__TIME__[1] - '0'))
#define MINUTE ((__TIME__[3] - '0') * 10 + (__TIME__[4] - '0'))
#define SECOND ((__TIME__[6] - '0') * 10 + (__TIME__[7] - '0'))

#define DT_TO_UINT32_T() ((uint32_t)((((((YEAR - 2021) * 12 + MONTH) * 31 + DAY) * 24 + HOUR) * 60 + MINUTE) * 60 + SECOND))

const sw_info_st sw_info __attribute__((section(".ARM.__at_0x00000000"))) =
{
  .date = DT_TO_UINT32_T(),
  ._date_ = ~DT_TO_UINT32_T(),
  .name = "APP:XPTest",
  .version = SOFTWARE_VERSION,
  .boot_addr = 0x24000000,
  .map =
    {{0x24000000,0x00040000}},
};
