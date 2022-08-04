#include "backlight.h"
#include "tim.h"

//亮度级别
static uint8_t backlight = 0;

/**
 * @brief 打开背光
 *
 */
void backlight_open(void)
{
  TIM2->CCR1 = backlight;

  //开启背光(使能pwm)
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

/**
 * @brief 关闭背光
 *
 */
void backlight_close(void)
{
  TIM2->CCR1 = 0;
  HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
}

/**
 * @brief 设置背光亮度
 *
 * @param level 亮度级别：0~100
 */
void backlight_set(uint8_t level)
{
  uint8_t val;

  val = level;

  if(val > 100)
    val = 100;

  backlight = val;
  TIM2->CCR1 = val;
}

/**
 * @brief 获取背光亮度
 *
 * @return uint8_t 亮度级别
 */
uint8_t backlight_get(void)
{
  return (uint8_t)(TIM2->CCR1);
}
