#include "test_3.h"
#include "tim.h"
#include "cmsis_os.h"
#include "spk_pn_pin_test.h"

#define PLAY_COUNT_MAX (30)

static const test_3_interface_st* intf;
static uint8_t result;
static uint32_t play;

void test_3_spkramp_init(void)
{
  result = 0;
}

void test_3_spkramp_set_intf(const test_3_interface_st* i)
{
  intf = i;
}

void test_3_spkramp_test_poll(void)
{
  GPIO_InitTypeDef gpio_init = { 0 };
  if(play)
  {
    if(play == PLAY_COUNT_MAX)
    {
      gpio_init.Pin = SPKRAMP_PWM_Pin;
      gpio_init.Mode = GPIO_MODE_AF_PP;
      gpio_init.Pull = GPIO_NOPULL;
      gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      gpio_init.Alternate = GPIO_AF2_TIM3;
      HAL_GPIO_Init(SPKRAMP_PWM_GPIO_Port, &gpio_init);

      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
      gpio_init.Alternate = 0;
      gpio_init.Pin = GPIO_PIN_8;
      gpio_init.Pull = GPIO_NOPULL;
      gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
      gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
      HAL_GPIO_Init(GPIOC, &gpio_init);

      __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, htim3.Init.Period * 0.8);
      HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
    }
    else if(play == 1)
    {
      gpio_init.Mode = GPIO_MODE_INPUT;
      gpio_init.Pull = GPIO_NOPULL;
      gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
      gpio_init.Alternate = 0;

      gpio_init.Pin = SPKRAMP_PWM_Pin;
      HAL_GPIO_Init(SPKRAMP_PWM_GPIO_Port, &gpio_init);

      gpio_init.Pin = GPIO_PIN_8;
      HAL_GPIO_Init(GPIOC, &gpio_init);

      HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
    }
    play--;
  }
  else
  {
    result = spk_pn_pin_test(MP_PC7, MP_PC8, intf->coil_to_spkramp_top_vsense_p_pin, intf->coil_to_spkramp_top_vsense_n_pin);;
  }
}

void test_3_spkramp_test_poll_end(void)
{
  if(play)
  {
    GPIO_InitTypeDef gpio_init = { 0 };

    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = 0;

    gpio_init.Pin = SPKRAMP_PWM_Pin;
    HAL_GPIO_Init(SPKRAMP_PWM_GPIO_Port, &gpio_init);

    gpio_init.Pin = GPIO_PIN_8;
    HAL_GPIO_Init(GPIOC, &gpio_init);

    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2);
    play = 0;
  }
}

int test_3_spkramp_play(void)
{
  if(play)
  {
    return 1;
  }
  else
  {
    play = PLAY_COUNT_MAX;
    return 0;
  }
}

uint8_t test_3_spkramp_res_get(void)
{
  return result;
}
