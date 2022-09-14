#include "es7243.h"
#include "hw_i2c.h"
#include "tim.h"
#include "sai.h"
#include "spi.h"
#include "cmsis_os.h"

#define ES7243_I2C_ID (0x26)

#define CHIP_ID (0x40)

#define DATA_BUFFER_MAX (1024)

static const uint8_t reg_list_init[];
static const uint8_t reg_list_start[];
static const uint8_t reg_list_stop[];

ALIGN_32BYTES(static es7243_data_st es7243_data_pbuffer[DATA_BUFFER_MAX] __attribute__((section(".AHB_SRAM1"))));
static void (*cb)(es7243_data_st* pdata, uint32_t len);

static int es7243_reg_list_exec(const uint8_t* list)
{
  for(uint32_t i = 0;list[i] != 0xFF;i += 2)
  {
    if(hw_i2c_write_1a_xd(hw_i2c2, ES7243_I2C_ID, list[i], list + i + 1, 1, 10))
    {
      return -1;
    }
  }

  return 0;
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef* hsai)
{
  if(cb)
  {
    cb(&es7243_data_pbuffer[DATA_BUFFER_MAX / 2], DATA_BUFFER_MAX / 2);
  }
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef* hsai)
{
  if(cb)
  {
    cb(&es7243_data_pbuffer[0], DATA_BUFFER_MAX / 2);
  }
}

/**
 * @brief 开始接收音频数据
 *
 * @param rate 采样率，须大于0
 * @param data_isr_cb 数据接收回调
 * @return int 错误码
 */
int es7243_start(uint32_t rate, void (*data_isr_cb)(es7243_data_st* pdata, uint32_t len))
{
  int err;
  uint8_t chip_id;

  err = 0;

  if(rate == 0)
  {
    err = -1;
    goto __exit;
  }

  GPIO_InitTypeDef GPIO_InitStruct = { 0 };

  HAL_GPIO_WritePin(I2S_MCLK_GPIO_Port, I2S_MCLK_Pin, GPIO_PIN_SET);

  GPIO_InitStruct.Pin = I2S_MCLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(I2S_MCLK_GPIO_Port, &GPIO_InitStruct);

  //MCLK引脚需发送至少20周期的脉冲使ES7243的I2C初始化
  for(uint32_t i = 0;i < 30;i++)
  {
    HAL_GPIO_TogglePin(I2S_MCLK_GPIO_Port, I2S_MCLK_Pin);
    HAL_Delay(1);
  }
  HAL_GPIO_WritePin(I2S_MCLK_GPIO_Port, I2S_MCLK_Pin, GPIO_PIN_SET);

  //检验ID
  if(hw_i2c_read_1a_1d(hw_i2c2, ES7243_I2C_ID, 0x0E, &chip_id, 10))
  {
    err = -3;
    goto __exit;
  }

  if(chip_id != CHIP_ID)
  {
    err = -4;
    goto __exit;
  }

  //初始化ES7243
  if(es7243_reg_list_exec(reg_list_init))
  {
    err = -5;
    goto __exit;
  }

  //配置S7243开始工作
  if(es7243_reg_list_exec(reg_list_start))
  {
    err = -6;
    goto __exit;
  }


  //配置MCU的SAI接口
  hsai_BlockB2.Instance = SAI2_Block_B;
  hsai_BlockB2.Init.AudioMode = SAI_MODEMASTER_RX;
  hsai_BlockB2.Init.Synchro = SAI_ASYNCHRONOUS;
  hsai_BlockB2.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
  hsai_BlockB2.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
  hsai_BlockB2.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai_BlockB2.Init.AudioFrequency = rate;
  hsai_BlockB2.Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai_BlockB2.Init.MonoStereoMode = SAI_STEREOMODE;
  hsai_BlockB2.Init.CompandingMode = SAI_NOCOMPANDING;
  HAL_SAI_InitProtocol(&hsai_BlockB2, SAI_I2S_STANDARD, SAI_PROTOCOL_DATASIZE_16BITEXTENDED, 2);

  vPortEnterCritical();
  HAL_SAI_Receive_DMA(&hsai_BlockB2, (uint8_t*)es7243_data_pbuffer, DATA_BUFFER_MAX * 2);
  vPortExitCritical();

  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  for(uint32_t i = 0;i < 20;i++)
  {
    osDelay(5);
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6 | GPIO_PIN_7);
  }
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  cb = data_isr_cb;

__exit:
  return err;
}

/**
 * @brief 停止音频数据
 *
 * @return int 错误码
 */
int es7243_stop(void)
{
  int err;

  err = 0;

  //停止MUC的SAI接口
  HAL_SAI_Abort(&hsai_BlockB2);
  HAL_SAI_DeInit(&hsai_BlockB2);
  __SAI1_FORCE_RESET();

  //
  if(es7243_reg_list_exec(reg_list_stop))
  {
    err = -2;
    goto __exit;
  }

__exit:
  return err;
}

/**
 * @brief 获取最新的一组音频数据
 *
 * @return es7243_data_st* 音频数据指针，等于0则无效
 */
es7243_data_st* es7243_get_current_value(void)
{
  uint32_t pos;

  pos = __HAL_DMA_GET_COUNTER(hsai_BlockB2.hdmarx);
  pos >>= 1;
  pos = DATA_BUFFER_MAX - pos;
  if(pos == 0)
    pos = DATA_BUFFER_MAX - 1;
  return es7243_data_pbuffer + pos;
}

static const uint8_t reg_list_init[] =
{
  0xFF //end
};

static const uint8_t reg_list_start[] =
{
  0x07,0x80,
  0x09,0x60,
  0x00,0x01,
  0x06,0x00,
  0x05,0x1B,
  0x01,0x0C,
  0x08,0x4B,
  0x05,0x30,
  0xFF//end
};

static const uint8_t reg_list_stop[] =
{
  0x06,0x05,
  0x05,0x1B,
  0x06,0x5C,
  0x07,0x3F,
  0x08,0x4B,
  0x09,0x9F,
  0xFF//end
};
