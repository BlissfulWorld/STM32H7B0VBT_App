#include "os_test.h"
#include "cmsis_os.h"
#include "hw_i2c.h"
#include "driver_ina219_basic.h"
#include "json_obj.h"

void gpio_recover(void);

/**
 * @brief 开短路测试，获取结果
 *
 * @param gnd_list 地线引脚列表
 * @param func_list 功能引脚列表
 * @param result 结果
 * @return int 错误码
 */
int os_test_get_volt(const uint8_t* gnd_list, const uint8_t* func_list, float* result)
{
  int err = 0;
  osPriority priority;
  float mv, ma, mw;
  float* out = result;
  ina219_handle_t handle;

  GPIO_InitTypeDef GPIO_InitStruct = { 0 };

  //提升任务级别，防止被打断
  priority = osThreadGetPriority(osThreadGetId());
  osThreadSetPriority(osThreadGetId(), osPriorityRealtime7);

  //等待I2C2空闲
  hw_i2c_fix(hw_i2c2, osWaitForever);

  //初始化电压、电流信息采集
  while(ina219_basic_init(&handle, INA219_ADDRESS_4, 0.1))
  {
    osDelay(100);
  }

  //设置引脚为模拟模式
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;

  GPIO_InitStruct.Pin = /*GPIO_PIN_0 |*/ GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
    GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
    GPIO_PIN_8 | GPIO_PIN_9 /*| GPIO_PIN_10 | GPIO_PIN_11 */ | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = /*GPIO_PIN_0 |*/ GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
    GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
    GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
    GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
    GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 |
    GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  //置低地引脚
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  for(const uint8_t* p = gnd_list;*p;p++)
  {
    GPIO_InitStruct.Pin = test_pin[*p].pin;

    HAL_GPIO_WritePin(test_pin[*p].gpio, test_pin[*p].pin, GPIO_PIN_RESET);
    HAL_GPIO_Init(test_pin[*p].gpio, &GPIO_InitStruct);
  }

  IWDG1->KR = IWDG_KEY_RELOAD;

  //遍历功能引脚的电压电流信息
  for(const uint8_t* p = func_list;*p;p++)
  {
    //置低放电
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pin = test_pin[*p].pin;
    HAL_GPIO_WritePin(test_pin[*p].gpio, test_pin[*p].pin, GPIO_PIN_RESET);
    HAL_GPIO_Init(test_pin[*p].gpio, &GPIO_InitStruct);

    //置高，开始测量
    HAL_Delay(10);
    HAL_GPIO_WritePin(test_pin[*p].gpio, test_pin[*p].pin, GPIO_PIN_SET);
    HAL_Delay(90);

    ina219_basic_read(&handle, &mv, &ma, &mw);

    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pin = test_pin[*p].pin;
    HAL_GPIO_Init(test_pin[*p].gpio, &GPIO_InitStruct);

    *out = ma;
    out++;

    IWDG1->KR = IWDG_KEY_RELOAD;
  }

  //恢复引脚功能
  gpio_recover();

  //hw_i2c_fix(hw_i2c2, osWaitForever);
  ina219_basic_deinit(&handle);

  IWDG1->KR = IWDG_KEY_RELOAD;

  //恢复任务优先级
  osThreadSetPriority(osThreadGetId(), priority);

  return err;
}

/**
 * @brief 恢复引脚的功能
 *
 */
void gpio_recover(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = { 0 };


  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = LCD_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = PWM2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
  HAL_GPIO_Init(PWM2_GPIO_Port, &GPIO_InitStruct);


  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = MUTE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MUTE_GPIO_Port, &GPIO_InitStruct);


  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = PWM5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM4;
  HAL_GPIO_Init(PWM5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PD15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PAPin PAPin */
  GPIO_InitStruct.Pin = DBG_PIN0_Pin | DBG_PIN1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = SPI_FLASH_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(SPI_FLASH_CS_GPIO_Port, &GPIO_InitStruct);

  /**I2C4 GPIO Configuration
    PD12     ------> I2C4_SCL
    PD13     ------> I2C4_SDA
    */
  GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C4;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /**I2C1 GPIO Configuration
    PB7     ------> I2C1_SDA
    PB8     ------> I2C1_SCL
    */
  GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**OCTOSPI1 GPIO Configuration
    PE2     ------> OCTOSPIM_P1_IO2
    PA1     ------> OCTOSPIM_P1_IO3
    PB2     ------> OCTOSPIM_P1_CLK
    PC9     ------> OCTOSPIM_P1_IO0
    PC10     ------> OCTOSPIM_P1_IO1
    PB6     ------> OCTOSPIM_P1_NCS
    */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_OCTOSPIM_P1;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OCTOSPIM_P1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**SAI2_B_Block_B GPIO Configuration
  PE6     ------> SAI2_MCLK_B
  PE11     ------> SAI2_SD_B
  PE12     ------> SAI2_SCK_B
  PE13     ------> SAI2_FS_B
  */
  GPIO_InitStruct.Pin = I2S_MCLK_Pin | I2S_SDO_Pin | I2S_SCK_Pin | I2S_WS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /**SPI2 GPIO Configuration
  PB12     ------> SPI2_NSS
  PB13     ------> SPI2_SCK
  PB14     ------> SPI2_MISO
  PB15     ------> SPI2_MOSI
  */
  GPIO_InitStruct.Pin = LCD_CS_Pin | LCD_SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LCD_WR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(LCD_WR_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LCD_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(LCD_SDA_GPIO_Port, &GPIO_InitStruct);


  /**SPI3 GPIO Configuration
  PC11     ------> SPI3_MISO
  PC12     ------> SPI3_MOSI
  PB3     ------> SPI3_SCK
  */
  GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /**TIM2 GPIO Configuration
  PA0     ------> TIM2_CH1
  */
  GPIO_InitStruct.Pin = LCD_BACKLIGHT_PWM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
  HAL_GPIO_Init(LCD_BACKLIGHT_PWM_GPIO_Port, &GPIO_InitStruct);

  /**TIM3 GPIO Configuration
  PC7     ------> TIM3_CH2
  */
  GPIO_InitStruct.Pin = SPKRAMP_PWM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(SPKRAMP_PWM_GPIO_Port, &GPIO_InitStruct);
}

/**
 * @brief 解析json文件中的开短路配置
 *
 * @param j_test json中ostest节点
 * @param ost 配置
 * @param num 配置中的引脚数目
 * @return int 错误码
 */
int ostest_json_config_phase(cJSON* j_test, os_test_st* ost, uint8_t num)
{
  if(j_test == 0 || ost == 0 | num == 0)
    return 1;

  cJSON* j_os = cJSON_GetObjectItem(j_test, json_obj_os);
  if(j_os)
  {
    char os_n[3];
    for(uint8_t i = 0;i < num;i++)
    {
      snprintf(os_n, sizeof(os_n), "%d", i + 1);
      cJSON* j_os_n = cJSON_GetObjectItem(j_os, os_n);
      if(j_os_n != NULL)
      {
        cJSON* j_os_max = cJSON_GetObjectItem(j_os_n, json_obj_max);
        if(j_os_max != NULL)
        {
          ost[i].i_max = j_os_max->valuedouble;
        }

        cJSON* j_os_min = cJSON_GetObjectItem(j_os_n, json_obj_min);
        if(j_os_min != NULL)
        {
          ost[i].i_min = j_os_min->valuedouble;
        }

        cJSON* j_os_color_in = cJSON_GetObjectItem(j_os_n, json_obj_color_in);
        if(j_os_color_in != NULL)
        {
          sscanf(j_os_color_in->valuestring, "%x", &ost[i].color_in);
        }

        cJSON* j_os_color_out = cJSON_GetObjectItem(j_os_n, json_obj_color_out);
        if(j_os_color_out != NULL)
        {
          sscanf(j_os_color_out->valuestring, "%x", &ost[i].color_out);
        }
      }
    }
  }

  return 0;
}
