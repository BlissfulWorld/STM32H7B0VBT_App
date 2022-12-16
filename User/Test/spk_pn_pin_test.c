#include "spk_pn_pin_test.h"
#include "test.h"
#include "gpio.h"
#include "cmsis_os.h"

/**
 * @brief 喇叭引脚测试
 *
 * @param out_p 喇叭输出脚P
 * @param out_n 喇叭输出脚N
 * @param vsense_p 喇叭反馈脚P
 * @param vsense_n 喇叭反馈脚N
 * @return uint8_t 结果，对应位为1则通过，bit0:P bit1:N
 */
uint8_t spk_pn_pin_test(uint8_t out_p, uint8_t out_n, uint8_t vsense_p, uint8_t vsense_n)
{
  uint8_t p = 0, n = 0;
  GPIO_InitTypeDef gpio_init = { 0 };

  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;

  gpio_init.Pull = GPIO_PULLDOWN;
  gpio_init.Mode = GPIO_MODE_INPUT;

  //下拉SPK_VSENSE
  //SPK_VSENSE_P
  gpio_init.Pin = test_pin[vsense_p].pin;
  HAL_GPIO_Init(test_pin[vsense_p].gpio, &gpio_init);

  //SPK_VSENSE_N
  gpio_init.Pin = test_pin[vsense_n].pin;
  HAL_GPIO_Init(test_pin[vsense_n].gpio, &gpio_init);

  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;

  //置高SPK_OUT
  //SPK_OUT_P
  HAL_GPIO_WritePin(test_pin[out_p].gpio, test_pin[out_p].pin, GPIO_PIN_SET);
  gpio_init.Pin = test_pin[out_p].pin;
  HAL_GPIO_Init(test_pin[out_p].gpio, &gpio_init);

  //SPK_OUT_N
  HAL_GPIO_WritePin(test_pin[out_n].gpio, test_pin[out_n].pin, GPIO_PIN_SET);
  gpio_init.Pin = test_pin[out_n].pin;
  HAL_GPIO_Init(test_pin[out_n].gpio, &gpio_init);

  osDelay(2);

  //SPK_VSENSE_P
  if(HAL_GPIO_ReadPin(test_pin[vsense_p].gpio, test_pin[vsense_p].pin) == GPIO_PIN_SET)
  {
    p++;
  }

  //SPK_VSENSE_N
  if(HAL_GPIO_ReadPin(test_pin[vsense_n].gpio, test_pin[vsense_n].pin) == GPIO_PIN_SET)
  {
    n++;
  }

  gpio_init.Pull = GPIO_PULLUP;
  gpio_init.Mode = GPIO_MODE_INPUT;

  //上拉SPK_VSENSE
  //SPK_VSENSE_P
  gpio_init.Pin = test_pin[vsense_p].pin;
  HAL_GPIO_Init(test_pin[vsense_p].gpio, &gpio_init);

  //SPK_VSENSE_N
  gpio_init.Pin = test_pin[vsense_n].pin;
  HAL_GPIO_Init(test_pin[vsense_n].gpio, &gpio_init);

  //置低SPK_OUT
  //SPK_OUT_P
  HAL_GPIO_WritePin(test_pin[out_p].gpio, test_pin[out_p].pin, GPIO_PIN_RESET);

  //SPK_OUT_N
  HAL_GPIO_WritePin(test_pin[out_n].gpio, test_pin[out_n].pin, GPIO_PIN_RESET);

  osDelay(2);

  //SPK_VSENSE_P
  if(HAL_GPIO_ReadPin(test_pin[vsense_p].gpio, test_pin[vsense_p].pin) == GPIO_PIN_RESET)
  {
    p++;
  }

  //SPK_VSENSE_N
  if(HAL_GPIO_ReadPin(test_pin[vsense_n].gpio, test_pin[vsense_n].pin) == GPIO_PIN_RESET)
  {
    n++;
  }

  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Mode = GPIO_MODE_INPUT;

  //SPK_OUT_P
  gpio_init.Pin = test_pin[out_p].pin;
  HAL_GPIO_Init(test_pin[out_p].gpio, &gpio_init);

  //SPK_OUT_N
  gpio_init.Pin = test_pin[out_n].pin;
  HAL_GPIO_Init(test_pin[out_n].gpio, &gpio_init);

  //SPK_VSENSE_P
  gpio_init.Pin = test_pin[vsense_p].pin;
  HAL_GPIO_Init(test_pin[vsense_p].gpio, &gpio_init);

  //SPK_VSENSE_N
  gpio_init.Pin = test_pin[vsense_n].pin;
  HAL_GPIO_Init(test_pin[vsense_n].gpio, &gpio_init);

  return (p == 2 ? (1) : 0) | (n == 2 ? (2) : 0);
}
