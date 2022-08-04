#include "adc.h"

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(hadc == &hadc2)
  {
    extern void adc2_conv_cplt_cb(void);
    adc2_conv_cplt_cb();
  }
  else if(hadc == &hadc1)
  {
    extern void adc1_conv_cplt_cb(void);
    adc1_conv_cplt_cb();
  }
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(hadc == &hadc1)
  {
    extern void adc1_conv_half_cplt_cb(void);
    adc1_conv_half_cplt_cb();
  }
}