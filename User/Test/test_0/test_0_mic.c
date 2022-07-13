#include "test.h"
#include "adc.h"
#include "tim.h"
#include "cmsis_os.h"

static uint16_t* mic;

int test_0_mic_init(void)
{
  //HAL_ADC_Start_IT(&hadc1);
  if(mic == 0)
  {
    mic = pvPortMalloc(sizeof(uint16_t) * 2);
    if(mic)
    {
      HAL_TIM_Base_Start(&htim6);
      HAL_ADC_Start_DMA(&hadc1, (uint32_t*)mic, 2);
    }
  }
  return 0;
}

uint8_t test_0_mic_get_percentage(void)
{
  uint16_t value;
  SCB_InvalidateDCache_by_Addr(mic, 1);
  if(mic[0] >= mic[1])
    value = mic[0] - mic[1];
  else
    value = mic[1] - mic[0];

  value = (double)value * 200 / 65535.0;

  if(value > 100) value = 100;

  return value;
}

void test_0_mic_update_data(void)
{

}

