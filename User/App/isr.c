#include "adc.h"

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if(hadc == &hadc2)
  {
    extern void adc2_conv_cplt_cb(void);
    adc2_conv_cplt_cb();
  }
}