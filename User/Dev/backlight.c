#include "backlight.h"
#include "tim.h"

#define PWM_MAX (999)
#define BACKLIGHT_LEVEL (10)

//亮度级别
static uint8_t backlight_lvl = 0;

static uint32_t backlight_to_pwm(uint8_t level)
{
  double l;

  if(level > BACKLIGHT_LEVEL)
  {
    level = BACKLIGHT_LEVEL;
  }

  l = level;

  l = (l / BACKLIGHT_LEVEL) * 100;

  if(l <= 8)
  {
    l /= 902.3;
  }
  else
  {
    l = (l + 16) / 116;
    l = l * l * l;
  }

  l *= PWM_MAX;

  return (uint32_t)l;
}

/**
 * @brief 打开背光
 *
 */
void backlight_open(void)
{
  TIM2->CCR1 = backlight_to_pwm(backlight_lvl);

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

  if(val > BACKLIGHT_LEVEL)
    val = BACKLIGHT_LEVEL;

  backlight_lvl = val;
  TIM2->CCR1 = backlight_to_pwm(backlight_lvl);
}

/**
 * @brief 获取背光亮度
 *
 * @return uint8_t 亮度级别
 */
uint8_t backlight_get(void)
{
  return backlight_lvl;
}
