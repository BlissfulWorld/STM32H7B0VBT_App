#ifndef _HW_PIN_H
#define _HW_PIN_H

#include "gpio.h"

typedef struct hw_pin_s
{
  GPIO_TypeDef *gpio;
  uint16_t pin;
}hw_pin_st;

#define HW_PIN_SET(hw_pin) do{(hw_pin)->gpio->BSRR = (hw_pin)->pin;}while(0)
#define HW_PIN_CLR(hw_pin) do{(hw_pin)->gpio->BSRR = (uint32_t)((hw_pin)->pin) << 16U;}while(0)
#define HW_PIN_GET(hw_pin) (((hw_pin)->gpio->IDR & ((hw_pin)->pin)) != 0)

#endif
