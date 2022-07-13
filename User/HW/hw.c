#include "hw.h"

#define DWT_CR     (*(__IO uint32_t *)0xE0001000)
#define DWT_CYCCNT (*(__IO uint32_t *)0xE0001004)
#define DEM_CR     (*(__IO uint32_t *)0xE000EDFC)

#define DWT_CR_CTCCNTENA (1U << 0U)
#define DEM_CR_TRCENA    (1U << 24U)

extern uint32_t SystemCoreClock;

void hw_init(void)
{
  DEM_CR |= DEM_CR_TRCENA;
  DWT_CYCCNT = 0;
  DWT_CR |= DWT_CR_CTCCNTENA;

  hw_i2c_init();
  hw_sim_i2c_init();
}

void hw_delay_100ns(uint32_t n_100ns)
{
  uint32_t tick, now;

  tick = n_100ns * (SystemCoreClock / 10000000);
  now = DWT_CYCCNT;

  while(DWT_CYCCNT - now < tick);
}