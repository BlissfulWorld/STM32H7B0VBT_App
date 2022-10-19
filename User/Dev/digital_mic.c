#include "digital_mic.h"
#include "tim.h"
#include "cmsis_os.h"
#include "es7243.h"

static const osThreadAttr_t t_attr = {
  .name = "d-mic",
  .stack_size = 2048,
  .priority = (osPriority_t)osPriorityAboveNormal,
};


static void (*cb)(void);
static uint16_t* out_buffer = 0;
static uint32_t count;

static es7243_data_st* pdata;
static uint32_t data_len;

static osThreadId_t tid;
static int is_run;

/**
 * @brief 麦克风数据接收回调函数
 *
 * @param _pdata 麦克风数据
 * @param _len 数据长度
 */
static void data_cb(es7243_data_st* _pdata, uint32_t _len)
{
  pdata = _pdata;
  data_len = _len;

  //通知任务数据已就绪
  vTaskNotifyGiveFromISR((TaskHandle_t)tid, NULL);
}

/**
 * @brief 麦克风任务
 *
 * @param arg 未使用
 */
static void digital_mic_task(void* arg)
{
  //开始ES7243音频数据录制
  if(es7243_start((uint32_t)arg, data_cb) == 0)
  {
    while(is_run)
    {
      ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
      if(out_buffer == 0)
        continue;

      uint32_t length = count;
      if(length > data_len)
        length = data_len;

      SCB_InvalidateDCache_by_Addr(pdata, (length * sizeof(es7243_data_st) + 32) & (~0x0000001FU));
      for(uint32_t i = 0;i < length;i++)
      {
        //提取右通道并转换后写入输出缓存
        uint16_t val = (uint16_t)pdata[i].r;
        out_buffer[i] = val + 0x8000;
      }

      out_buffer += length;
      count -= length;
      if(count == 0)
      {
        out_buffer = 0;
        if(cb) cb();
      }
    }
    es7243_stop();
  }

  tid = 0;
  osThreadExit();
}

/**
 * @brief 启动麦克风
 *
 * @param rate 采样率
 * @return int 错误码
 */
int digital_mic_start(uint32_t rate)
{
  int err;

  err = 0;

  osKernelLock();

  if(tid)
  {
    err = 1;
    goto __exit;
  }

  //创建线程处理麦克风数据
  tid = osThreadNew(digital_mic_task, (void*)rate, &t_attr);
  if(tid == 0)
  {
    err = -2;
    goto __exit;
  }

  is_run = 1;

__exit:
  osKernelUnlock();
  return err;
}

/**
 * @brief 停止麦克风
 *
 * @return int
 */
int digital_mic_stop(void)
{
  is_run = 0;
  while(tid) osDelay(10);
  return 0;
}

/**
 * @brief 录制音频数据
 *
 * @param buffer
 * @param size
 * @param finished_cb
 * @return int
 */
int digital_mic_record_start(void* buffer, uint32_t size, void (*finished_cb)(void))
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

/**
 * @brief 停止录制
 *
 * @return int 错误码
 */
int digital_mic_record_stop(void)
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

/**
 * @brief 获取计数器
 *
 * @return uint32_t 计算值
 */
uint32_t digital_mic_record_get_count(void)
{
  return count;
}

/**
 * @brief 获取当前的mic值，8bit
 *
 * @return uint8_t mic值
 */
uint8_t digital_mic_get_current_value(void)
{
  es7243_data_st* data = es7243_get_current_value();

  int16_t val = data->r;

  if(val > 0)
    return val >> 7;
  else
    return (-val) >> 7;
}
