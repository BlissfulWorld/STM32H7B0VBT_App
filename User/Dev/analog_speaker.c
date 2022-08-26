#include "analog_speaker.h"
#include "dac.h"
#include "tim.h"
#include "cmsis_os.h"

#define DAC_DMA_LEN_MAX (8196)

#define HTIM (&htim8)

extern osThreadId_t speakerHandle;

uint32_t align, rate;

static analog_speaker_play_finished_cb_func_t f_cb;
static analog_speaker_get_data_func_t d_cb;
static void* user_p;
static float multiplier = 1;
static int8_t db;
static uint8_t volume_adjust;

static uint32_t get_data(void* buffer, uint32_t size)
{
  uint32_t count, i;

  count = d_cb(user_p, buffer, size);

  if(align == DAC_ALIGN_8B_R)
  {
    //音量调节
    for(i = 0;i < count;i++)
    {
      uint8_t dd;
      if(((uint8_t*)buffer)[i] >= 0x80)
      {
        dd = ((uint8_t*)buffer)[i] - 0x80;
        if(volume_adjust)
          dd *= multiplier;
        ((uint8_t*)buffer)[i] = dd + 0x80;
      }
      else
      {
        dd = 0x80 - ((uint8_t*)buffer)[i];
        if(volume_adjust)
          dd *= multiplier;
        ((uint8_t*)buffer)[i] = 0x80 - dd;
      }
    }

    //其余部分填充静音数据
    for(;i < size;i++)
    {
      ((uint8_t*)buffer)[i] = 0x80;
    }
  }
  else
  {
    //音量调节
    uint32_t halfword_cnt;
    halfword_cnt = count >> 1;
    for(i = 0;i < halfword_cnt;i++)
    {
      uint16_t dd;
      if(((uint16_t*)buffer)[i] >= 0x8000)
      {
        dd = ((uint16_t*)buffer)[i] - 0x8000;
        if(volume_adjust)
          dd *= multiplier;
        ((uint16_t*)buffer)[i] = dd + 0x8000;
      }
      else
      {
        dd = 0x8000 - ((uint16_t*)buffer)[i];
        if(volume_adjust)
          dd *= multiplier;
        ((uint16_t*)buffer)[i] = 0x8000 - dd;
      }
    }

    //其余部分填充静音数据
    halfword_cnt = size >> 1;
    for(;i < halfword_cnt;i++)
    {
      ((uint16_t*)buffer)[i] = 0x8000;
    }
  }

  SCB_CleanDCache_by_Addr(buffer, size);

  return count;
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
  vTaskNotifyGiveFromISR((TaskHandle_t)speakerHandle, NULL);
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
  vTaskNotifyGiveFromISR((TaskHandle_t)speakerHandle, NULL);
}

/**
 * @brief 是否正在播放
 *
 * @return int 0：否 1：是
 */
int analog_speaker_is_busy(void)
{
  return d_cb ? 1 : 0;
}

void speaker_task(void* argument)
{
  uint32_t size, count;
  uint32_t half_size;
  void* buffer, * p;

  //得到缓存指针
  ALIGN_32BYTES(static uint16_t pbuffer[DAC_DMA_LEN_MAX] __attribute__((section(".AHB_SRAM1"))));
  buffer = pbuffer;

  while(1)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    //关闭静音
    HAL_GPIO_WritePin(MUTE_GPIO_Port, MUTE_Pin, GPIO_PIN_RESET);

    //计算缓存大小
    if(hdac1.DMA_Handle1->Init.MemDataAlignment == DMA_MDATAALIGN_BYTE)
    {
      size = DAC_DMA_LEN_MAX;
    }
    else if(hdac1.DMA_Handle1->Init.MemDataAlignment == DMA_PDATAALIGN_HALFWORD)
    {
      size = DAC_DMA_LEN_MAX * 2;
    }

    //首次完整填充音频数据
    count = get_data(buffer, size);
    if(count == 0)
      goto __exit;

    if(align == DAC_ALIGN_8B_R)
      HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, align, 0x80);
    else
      HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, align, 0x8000);

    HAL_DAC_Start(&hdac1, DAC_CHANNEL_1);

    HAL_TIM_Base_Start(HTIM);

    //关闭静音须等待一段时间
    osDelayUntil(osKernelGetTickCount() + 150);

    half_size = size >> 1;
    while(1)
    {
      vPortEnterCritical();
      //开始转换
      HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)buffer, DAC_DMA_LEN_MAX, align);
      vPortExitCritical();

      uint32_t delay_tick = pdMS_TO_TICKS(((double)half_size / (double)rate) * 1000.0);
      delay_tick += (delay_tick >> 1);
      uint32_t val = ulTaskNotifyTake(pdTRUE, delay_tick);
      if(val == 1)
        break;

      vPortEnterCritical();
      HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
      vPortExitCritical();
    }

    p = 0;

    while(1)
    {
      if(count == 0)
        break;

      if(p != buffer)
        p = buffer;
      else
        p = (uint8_t*)buffer + half_size;

      count = get_data(p, half_size);

      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }

    vPortEnterCritical();
    //结束转换
    HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
    HAL_TIM_Base_Stop(HTIM);
    vPortExitCritical();

  __exit:
    //开启静音
    HAL_GPIO_WritePin(MUTE_GPIO_Port, MUTE_Pin, GPIO_PIN_SET);

    ulTaskNotifyTake(pdTRUE, 0);

    d_cb = 0;

    if(f_cb) f_cb(user_p);
  }
}

/**
 * @brief 播放音频
 *
 * @param rate 采样率
 * @param bits 音频位数
 * @param data_cb 数据填充回调函数指针
 * @param finished_cb 播放完成回调函数指针
 * @param user 用户指针
 * @return int 错误码
 */
int analog_speaker_play(uint32_t rate_hz, analog_spk_bits_et bits, uint8_t is_volume_adjust, analog_speaker_get_data_func_t data_get, analog_speaker_play_finished_cb_func_t finished_cb, void* user)
{
  int err;

  if(rate_hz == 0 || data_get == 0)
  {
    err = -1;
    goto __exit;
  }

  osKernelLock();

  if(d_cb)
  {
    err = -1;
    goto __kernel_unlock;
  }

  if(bits == ANALOG_SPK_BITS_16)
  {
    align = DAC_ALIGN_12B_L;
    hdac1.DMA_Handle1->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdac1.DMA_Handle1->Init.MemDataAlignment = DMA_PDATAALIGN_HALFWORD;
  }
  else
  {
    align = DAC_ALIGN_8B_R;
    hdac1.DMA_Handle1->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdac1.DMA_Handle1->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
  }

  d_cb = data_get;
  f_cb = finished_cb;
  user_p = user;
  volume_adjust = is_volume_adjust;
  rate = rate_hz;

  uint32_t period = ((280000000 / (double)rate) + 0.5) - 1;
  __HAL_TIM_SetAutoreload(HTIM, period);

  HAL_DMA_Init(hdac1.DMA_Handle1);

  err = 0;

  vTaskNotifyGiveFromISR((TaskHandle_t)speakerHandle, NULL);

__kernel_unlock:
  osKernelUnlock();
__exit:
  return err;
}

/**
 * @brief 设置音量
 *
 * @param volume 音量 0~10
 */
void analog_speaker_volume_set(uint8_t volume)
{
  if(volume > 10)
    volume = 10;

  db = (volume - 10) * 4.5;
  if(volume)
  {
    multiplier = pow(10, (double)db / 20.0);
  }
  else
  {
    multiplier = 0;
  }
}

/**
 * @brief 获取当前音量
 *
 * @return uint8_t 音量值
 */
uint8_t analog_speaker_volume_get(void)
{
  return 10 + db / 4.5;
}
