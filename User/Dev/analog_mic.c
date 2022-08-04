#include "analog_mic.h"
#include "adc.h"
#include "tim.h"
#include "cmsis_os.h"

#define ADC_DMA_LEN_MAX (1024 * 4)

#define EVT_FLG_BIT_CONV_CPLT      (1 << 0)
#define EVT_FLG_BIT_HALF_CONV_CPLT (1 << 1)
//#define EVT_FLG_BIT_RECORD_STOP    (1 << 2)
//#define EVT_FLG_BIT_RECORD_START   (1 << 3)
#define EVT_FLG_BIT_ALL            (0x0000000F)

typedef uint32_t adc_value_st;

typedef struct mic_adc_value_s
{
  adc_value_st p;
  adc_value_st n;
}mic_adc_value_st;

typedef struct mic_adc_buffer_s
{
  mic_adc_value_st front[ADC_DMA_LEN_MAX / 4];
  mic_adc_value_st back[ADC_DMA_LEN_MAX / 4];
}mic_adc_buffer_st;

static const osThreadAttr_t t_attr = {
  .name = "mic",
  .stack_size = 2048,
  .priority = (osPriority_t)osPriorityAboveNormal,
};

static const osEventFlagsAttr_t ef_attr = {
  .name = "mic event",
};

static void (*cb)(void);
static uint8_t* out_buffer = 0;
static uint32_t count;

static mic_adc_buffer_st* mic_adc_buffer;

static osThreadId_t tid;
static osEventFlagsId_t efid;

void adc1_conv_cplt_cb(void)
{
  osEventFlagsSet(efid, EVT_FLG_BIT_CONV_CPLT);
}

void adc1_conv_half_cplt_cb(void)
{
  osEventFlagsSet(efid, EVT_FLG_BIT_HALF_CONV_CPLT);
}

#define MIC_ADC_VALUE_BOUND(val) do{\
  if(val < 0x00100000)\
  {\
    val = 0;\
  }\
  else\
  {\
    val -= 0x00100000;\
    if(val >= 0x00400000)\
      val = 0x00400000 - 1;\
  }\
}while(0)

static uint8_t adc2mic(adc_value_st p, adc_value_st n)
{
  uint8_t out;

  MIC_ADC_VALUE_BOUND(p);
  MIC_ADC_VALUE_BOUND(n);

  if(p >= n)
  {
    out = (p - n) >> 14;
  }
  else
  {
    out = (n - p) >> 14;
  }

  return out;
}

static void adc_convert_mic(mic_adc_value_st* adc, uint32_t len, uint8_t* out)
{
  for(uint32_t i = 0;i < len;i++)
  {
    out[i] = adc2mic(adc[i].p, adc[i].n);
  }
}

static void analog_mic_task(void* arg)
{
  uint32_t flags;
  void (*__IO cb_tmp)(void);

  HAL_ADCEx_Calibration_Start(&hadc1, ADC_CALIB_OFFSET_LINEARITY, ADC_SINGLE_ENDED);

  HAL_ADC_Start_DMA(&hadc1, (uint32_t*)mic_adc_buffer, sizeof(mic_adc_buffer_st) / sizeof(adc_value_st));
  HAL_TIM_Base_Start(&htim6);

  while(1)
  {
    flags = osEventFlagsWait(efid, EVT_FLG_BIT_CONV_CPLT | EVT_FLG_BIT_HALF_CONV_CPLT, osFlagsWaitAny, osWaitForever);
    if(out_buffer == 0)
      continue;

    if(flags & EVT_FLG_BIT_HALF_CONV_CPLT)
    {
      uint32_t length = count;

      if(length > sizeof(mic_adc_buffer->front) / sizeof(mic_adc_value_st))
      {
        length = sizeof(mic_adc_buffer->front) / sizeof(mic_adc_value_st);
      }

      SCB_InvalidateDCache_by_Addr(mic_adc_buffer->front, sizeof(mic_adc_buffer->front));
      adc_convert_mic(mic_adc_buffer->front, length, out_buffer);

      out_buffer += length;
      count -= length;
      if(count == 0)
      {
        out_buffer = 0;
        if(cb) cb();
      }
    }
    else if(flags & EVT_FLG_BIT_CONV_CPLT)
    {
      uint32_t length = count;

      if(length > sizeof(mic_adc_buffer->back) / sizeof(mic_adc_value_st))
      {
        length = sizeof(mic_adc_buffer->back) / sizeof(mic_adc_value_st);
      }

      SCB_InvalidateDCache_by_Addr(mic_adc_buffer->back, sizeof(mic_adc_buffer->back));
      adc_convert_mic(mic_adc_buffer->back, length, out_buffer);

      out_buffer += length;
      count -= length;
      if(count == 0)
      {
        out_buffer = 0;
        if(cb) cb();
      }
    }
  }
}

int analog_mic_start(void)
{
  int err;
  __IO osThreadId_t t;

  err = 0;

  osKernelLock();
  if(tid)
  {
    osKernelUnlock();
    err = 1;
    goto __exit;
  }
  osKernelUnlock();

  mic_adc_buffer = pvPortMalloc(sizeof(mic_adc_buffer_st));
  if(mic_adc_buffer == 0)
  {
    err = -1;
    goto __exit;
  }

  efid = osEventFlagsNew(&ef_attr);
  if(efid == 0)
  {
    err = -2;
    goto __free_adc_buffer;
  }

  t = osThreadNew(analog_mic_task, mic_adc_buffer, &t_attr);
  if(t == 0)
  {
    err = -3;
    goto __event_flags_del;
  }

  tid = t;
  goto __exit;

__event_flags_del:
  osEventFlagsDelete(efid);
  efid = 0;

__free_adc_buffer:
  vPortFree(mic_adc_buffer);
  mic_adc_buffer = 0;

__exit:
  return err;
}

int analog_mic_stop(void)
{
  osKernelLock();
  if(tid == 0)
  {
    osKernelUnlock();
    return 1;
  }

  HAL_TIM_Base_Stop(&htim6);
  HAL_ADC_Stop_DMA(&hadc1);

  osThreadTerminate(tid);

  vPortFree(mic_adc_buffer);
  mic_adc_buffer = 0;

  osEventFlagsDelete(efid);
  efid = 0;

  out_buffer = 0;
  tid = 0;
  osKernelUnlock();

  return 0;
}

int analog_mic_record_start(void* buffer, uint32_t size, void (*finished_cb)(void))
{
  int err;

  if(buffer == 0 || size == 0)
  {
    err = -1;
    goto __exit;
  }

  osKernelLock();

  if(tid == 0)
  {
    err = -2;
    goto __unlock;
  }

  if(out_buffer)
  {
    err = -3;
    goto __unlock;
  }

  out_buffer = buffer;
  count = size;
  cb = finished_cb;

  err = 0;

__unlock:
  osKernelUnlock();

__exit:
  return err;
}

int analog_mic_record_stop(void)
{
  osKernelLock();
  if(out_buffer == 0)
  {
    osKernelUnlock();
    return -1;
  }

  out_buffer = 0;

  osKernelUnlock();

  return 0;
}

uint32_t analog_mic_record_get_count(void)
{
  return count;
}

uint8_t analog_mic_get_current_value(void)
{
  uint32_t i;
  adc_value_st* adc_value;

  if(tid && (out_buffer == 0))
  {
    i = sizeof(mic_adc_buffer_st) / sizeof(adc_value_st) - __HAL_DMA_GET_COUNTER(hadc1.DMA_Handle);
    i &= 0xFFFFFFFE;
    adc_value = (adc_value_st*)mic_adc_buffer;
    adc_value += i;
    SCB_InvalidateDCache_by_Addr(adc_value, sizeof(adc_value_st) * 2);
    return adc2mic(adc_value[0], adc_value[1]);
  }
  else
  {
    return 0;
  }
}