#include "wav.h"
#include "ff.h"
#include "cmsis_os.h"
#include "dac.h"
#include "tim.h"
#include "string.h"

#define DAC_DMA_LEN_MAX (0xF000)

#define _4S2U32(id) (uint32_t)((id[0] << 0u) | (id[1] << 8u) | (id[2] << 16u) |( id[3] << 24u))

#define ID_RIFF  _4S2U32("RIFF")
#define ID_FMT   _4S2U32("fmt ")
#define ID_DATA  _4S2U32("data")

#define FMT_WAVE _4S2U32("WAVE")

static const osThreadAttr_t attr = {
  .name = "audio paly",
  .stack_size = 1024 * 2,
  .priority = (osPriority_t)osPriorityNormal1,
};

static osThreadId_t tid;
uint32_t count;
uint8_t* pbuffer;

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
  if(count != 0)
  {
    if(count > DAC_DMA_LEN_MAX)
    {
      HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)pbuffer, DAC_DMA_LEN_MAX, DAC_ALIGN_8B_R);
      count -= DAC_DMA_LEN_MAX;
    }
    else
    {
      HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)pbuffer, count, DAC_ALIGN_8B_R);
      count = 0;
    }
  }
  else
  {
    vTaskNotifyGiveFromISR((TaskHandle_t)tid, NULL);
  }
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
}

static void play_task(void* arg)
{
  UINT br;
  FIL* fil;
  
  uint8_t stage;
  
  chunk_st chunk;
  chunk_riff_st chunk_riff;
  chunk_fmt_st chunk_fmt;

  char* path = arg;
  stage = 0;

  fil = pvPortMalloc(sizeof(FIL));
  if(fil == 0)
  {
    goto __play_task_exit;
  }

  if(f_open(fil, path, FA_READ) != FR_OK)
  {
    goto __play_task_free_fil;
  }
  
  if(f_read(fil, &chunk_riff, sizeof(chunk_riff), &br))
  {
    goto __play_task_close;
  }

  if((chunk_riff.chunk.id != ID_RIFF) || (chunk_riff.format != FMT_WAVE))
  {
    goto __play_task_close;
  }

  while(1)
  {
    if(f_read(fil, &chunk, sizeof(chunk), &br))
    {
      goto __play_task_close;
    }
    
    if((br != sizeof(chunk)) || (chunk.size == 0))
      break;

    if(chunk.id == ID_FMT)
    {
      uint32_t size;
      chunk_fmt.chunk.id = chunk.id;
      chunk_fmt.chunk.size = chunk.size;

      if(chunk_fmt.chunk.size > sizeof(chunk_fmt_st) - sizeof(chunk_st))
        size = sizeof(chunk_fmt_st) - sizeof(chunk_st);
      else
        size = chunk_fmt.chunk.size;

      if(f_read(fil, chunk_fmt.chunk.data, size, &br))
      {
        break;
      }
      
      if(size < chunk_fmt.chunk.size)
      {
        f_lseek(fil,f_tell(fil) + (chunk_fmt.chunk.size - size));
      }

      stage |= 1;
    }
    else if(chunk.id == ID_DATA)
    {
      count = chunk.size + 1;
      pbuffer = pvPortMalloc(count);
      if(pbuffer == 0)
      {
        break;
      }

      if(f_read(fil, pbuffer, chunk.size, &br))
      {
        break;
      }

      stage |= 2;
    }
    else
    {
      f_lseek(fil,f_tell(fil) + chunk.size);
    }

    if(stage == 3)
    {
      break;
    }

    if(f_tell(fil) >= chunk_riff.chunk.size + 8)
      break;
  }

  if(stage != 3)
  {
    if(pbuffer)
      goto __play_task_free_pbuffer;
    else
      goto __play_task_close;
  }

  if((chunk_fmt.num_of_channels != 1) || (chunk_fmt.bits_per_sample != 8) || (chunk_fmt.sample_rate != 8000))
  {
    goto __play_task_free_pbuffer;
  }

  /*for(uint32_t i = 0;i < count;i++)
  {
    pbuffer[i] += 128;
  }*/

  HAL_GPIO_WritePin(MUTE_GPIO_Port, MUTE_Pin, GPIO_PIN_RESET);

  osDelay(200);

  pbuffer[count - 1] = 0x00;

  SCB_CleanDCache_by_Addr((uint32_t*)pbuffer, count);

  if(count > DAC_DMA_LEN_MAX)
  {
    HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)pbuffer, DAC_DMA_LEN_MAX, DAC_ALIGN_8B_R);
    count -= DAC_DMA_LEN_MAX;
  }
  else
  {
    HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)pbuffer, count, DAC_ALIGN_8B_R);
    count = 0;
  }

  __HAL_TIM_SetAutoreload(&htim5, (280000000 / 2 / chunk_fmt.sample_rate) - 1);

  HAL_TIM_Base_Start(&htim5);

  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

  HAL_GPIO_WritePin(MUTE_GPIO_Port, MUTE_Pin, GPIO_PIN_SET);
  HAL_TIM_Base_Stop(&htim5);
  HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);

__play_task_free_pbuffer:
  vPortFree(pbuffer);

__play_task_close:
  f_close(fil);

__play_task_free_fil:
  vPortFree(fil);

__play_task_exit:
  vPortFree(path);

  tid = 0;
  osThreadExit();
}

int wav_play(const char* path)
{
  int err;
  char* str;

  vTaskSuspendAll();

  err = 0;

  if(tid != 0)
  {
    err = -1;
    goto __wav_play_exit;
  }

  if(path == 0)
  {
    err = -2;
    goto __wav_play_exit;
  }

  str = pvPortMalloc(strlen(path));
  if(str == 0)
  {
    err = -3;
    goto __wav_play_exit;
  }
  strcpy(str, path);
  tid = osThreadNew(play_task, str, &attr);
  if(tid == 0)
  {
    err = -4;
    vPortFree(str);
  }

__wav_play_exit:
  xTaskResumeAll();
  return err;
}