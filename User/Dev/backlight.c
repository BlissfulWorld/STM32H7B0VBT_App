#include "backlight.h"
#include "tim.h"

static uint32_t backlight = 0;

void backlight_open(void)
{
  TIM2->CCR1 = backlight;

  //开启背光(使能pwm)
  HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_1);
}

void backlight_close(void)
{
  TIM2->CCR1 = 0;
  HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_1);
}

void backlight_set(uint32_t level)
{
  uint32_t val;

  val = level;

  if(val > 100)
    val = 100;

  backlight = val;
  TIM2->CCR1 = val;
}

uint32_t backlight_get(void)
{
  return (uint8_t)(TIM2->CCR1);
}
