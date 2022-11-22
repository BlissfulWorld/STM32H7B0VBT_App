#include "test_4.h"
#include "config.h"
#include "cmsis_os.h"
#include "config.h"
#include "string.h"
#include "stdio.h"
#include "cJSON.h"
#include "json_obj.h"
#include "pca9535.h"
#include "mic.h"
#include "wav.h"
#include "resource.h"
#include "hw_i2c.h"

static const osThreadAttr_t test_tid_attr = {
  .name = "test 4",
  .stack_size = 1024 * 8,
  .priority = (osPriority_t)osPriorityNormal,
};

const test_module_id_ut ipx_j6400_id =
{
  .device = IPHONE,
  .model_h = IPHONE_X >> 8,
  .model_l = IPHONE_X,
  .number = 1,
};

static const test_4_config_st config_default =
{
#define VI_CONST_INIT(idx,enable) \
  /*.power[idx-1].json_obj = (const char *[]){json_obj_test,json_obj_power,json_obj_##idx,0},*/\
  .power[idx-1].i_max = 5000,\
  .power[idx-1].i_min = 0,\
  .power[idx-1].v_max = 5000,\
  .power[idx-1].v_min = 0,\
  .power[idx-1].color_in = 0xFFFFFF,\
  .power[idx-1].color_out = 0xFF0000,\
  .power[idx-1].en = enable

#define OST_CONST_INIT(idx,val) \
  .ost[idx-1].i_max = val + val * 0.05,\
  .ost[idx-1].i_min = val - val * 0.05,\
  .ost[idx-1].color_in = 0xFFFFFF,\
  .ost[idx-1].color_out = 0xFF0000

  VI_CONST_INIT(1,1),
  VI_CONST_INIT(2,1),
  VI_CONST_INIT(3,1),
  VI_CONST_INIT(4,1),
  VI_CONST_INIT(5,1),
  VI_CONST_INIT(6,1),
  VI_CONST_INIT(7,0),
  VI_CONST_INIT(8,0),
  VI_CONST_INIT(9,0),
  VI_CONST_INIT(10,0),
  VI_CONST_INIT(11,1),
  VI_CONST_INIT(12,0),

  .mvi.i_max = 5000,
  .mvi.i_min = 0,
  .mvi.v_max = 5000,
  .mvi.v_min = 0,
  .mvi.color_in = 0xFFFFFF,
  .mvi.color_out = 0xFF0000,
};

static int is_test = 0;

static const test_4_io_st gnd[] =
{
  {2,MP_PA3},{7,MP_PA9},{8,MP_PE15},{14,MP_PE0},{18,MP_PE5},{25,MP_PA10},{31,MP_PE1},{39,MP_PE3}
};

static const test_4_io_st io[] =
{
  {1,MP_PC7},{3,MP_PD6},{4,MP_PE7},{5,MP_PC6},{6,MP_PE9},{10,MP_PC1},{11,MP_PA5},{12,MP_PD9},{13,MP_PB0},{15,MP_PC3},{16,MP_PC4},{20,MP_PD11},{24,MP_PE4},
  {26,MP_PB7},{28,MP_PB8},{30,MP_PB4},{32,MP_PD7},{36,MP_PC5},{41,MP_PD4},{43,MP_PD3},/*{40,MP_PB1},{44,MP_PA2},*/{45,MP_PC8},
};

//power
//PP_CODEC_TO_LOWERMIC4_BIAS_CON PB5 2.9V
//PP1V8_SAKONNET_CONN PD8 1.8V
//PP_VBUS1_E75_CON PD2

static struct
{
  uint8_t stage;
  test_4_st _4;
  int io_test_error;
  uint8_t mic1_rdy;
  uint8_t io_open_circuit_data[sizeof(io)];
  uint8_t io_open_circuit_count;
  uint16_t io_short_circuit_data[sizeof(io) * sizeof(io) - sizeof(io)];
  uint32_t io_short_circuit_count;
}*test;

static void test_io_init(const test_4_io_st* io_list, uint8_t length)
{
  GPIO_InitTypeDef gpio_init;
  gpio_init.Alternate = 0;
  gpio_init.Pull = GPIO_PULLDOWN;
  gpio_init.Speed = GPIO_SPEED_LOW;
  gpio_init.Mode = GPIO_MODE_INPUT;
  for(uint8_t i = 0;i < length;i++)
  {
    gpio_init.Pin = test_pin[io_list[i].mcu_pin].pin;
    HAL_GPIO_Init(test_pin[io_list[i].mcu_pin].gpio, &gpio_init);
  }
}

static void test_io_deinit(const test_4_io_st* io_list, uint8_t length)
{
  GPIO_InitTypeDef gpio_init;

  gpio_init.Alternate = 0;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_LOW;
  gpio_init.Mode = GPIO_MODE_INPUT;
  for(uint8_t i = 0;i < length;i++)
  {
    gpio_init.Pin = test_pin[io_list[i].mcu_pin].pin;
    HAL_GPIO_Init(test_pin[io_list[i].mcu_pin].gpio, &gpio_init);
  }
}

/**
 * @brief 检测引脚是否被拉高
 *
 * @param io_list 引脚列表
 * @param length 列表长度
 * @return int 1：有被拉高 0：没有被拉高
 */
static int test_io_is_have_high_lvl(const test_4_io_st* io_list, uint8_t length)
{
  for(uint8_t i = 0;i < length;i++)
  {
    if(HAL_GPIO_ReadPin(test_pin[io_list[i].mcu_pin].gpio, test_pin[io_list[i].mcu_pin].pin) == GPIO_PIN_SET)
    {
      return 1;
    }
  }

  return 0;
}

/**
 * @brief 开路检测
 *
 * @param io_list 引脚列表
 * @param length 列表长度
 * @param pbuffer 用于存储检测结果的缓存
 * @return uint8_t 开路引脚数量
 */
static uint8_t test_io_check_open_circuit(const test_4_io_st* io_list, uint8_t length, uint8_t* pbuffer)
{
  uint8_t oc_cnt = 0;
  for(uint8_t i = 0;i < length;i++)
  {
    if(HAL_GPIO_ReadPin(test_pin[io_list[i].mcu_pin].gpio, test_pin[io_list[i].mcu_pin].pin) == GPIO_PIN_RESET)
    {
      pbuffer[oc_cnt] = io_list[i].num;
      oc_cnt++;
    }
  }

  return oc_cnt;
}

/**
 * @brief 短路检测
 *
 * @param io_list 引脚列表
 * @param length 列表长度
 * @param pbuffer 用于存储检测结果的缓存
 * @return uint32_t 开路引脚对数量
 */
static uint32_t test_io_check_short_circuit(const test_4_io_st* io_list, uint8_t length, uint16_t* pbuffer)
{
  GPIO_InitTypeDef gpio_init = { 0 };
  uint32_t sc_cnt = 0;

  gpio_init.Speed = GPIO_SPEED_LOW;

  for(uint8_t i = 0;i < length;i++)
  {
    //置低引脚
    HAL_GPIO_WritePin(test_pin[io_list[i].mcu_pin].gpio, test_pin[io_list[i].mcu_pin].pin, GPIO_PIN_RESET);

    gpio_init.Pin = test_pin[io_list[i].mcu_pin].pin;

    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
    HAL_GPIO_Init(test_pin[io_list[i].mcu_pin].gpio, &gpio_init);

    osDelay(10);

    //遍历其他引脚是否被拉低
    for(uint8_t j = i + 1;j < length;j++)
    {
      if(HAL_GPIO_ReadPin(test_pin[io_list[j].mcu_pin].gpio, test_pin[io_list[j].mcu_pin].pin) == GPIO_PIN_RESET)
      {
        pbuffer[sc_cnt] = ((uint16_t)io_list[i].num << 8) | ((uint16_t)io_list[j].num & 0x00FF);
        sc_cnt++;
      }
    }

    //恢复引脚
    gpio_init.Pull = GPIO_PULLDOWN;
    gpio_init.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(test_pin[io_list[i].mcu_pin].gpio, &gpio_init);
  }

  return sc_cnt;
}

static void gnd_enable(const test_4_io_st* io_list, uint8_t length)
{
  GPIO_InitTypeDef gpio_init = { 0 };

  gpio_init.Speed = GPIO_SPEED_LOW;

  for(uint8_t i = 0;i < length;i++)
  {
    //置低引脚
    HAL_GPIO_WritePin(test_pin[io_list[i].mcu_pin].gpio, test_pin[io_list[i].mcu_pin].pin, GPIO_PIN_RESET);

    gpio_init.Pin = test_pin[io_list[i].mcu_pin].pin;

    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
    HAL_GPIO_Init(test_pin[io_list[i].mcu_pin].gpio, &gpio_init);
  }
}

static void gnd_disable(const test_4_io_st* io_list, uint8_t length)
{
  GPIO_InitTypeDef gpio_init = { 0 };

  gpio_init.Speed = GPIO_SPEED_LOW;

  for(uint8_t i = 0;i < length;i++)
  {
    gpio_init.Pin = test_pin[io_list[i].mcu_pin].pin;

    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Mode = GPIO_MODE_INPUT;

    HAL_GPIO_Init(test_pin[io_list[i].mcu_pin].gpio, &gpio_init);
  }
}

void test_4_init(void)
{
  uint16_t power_en;
  GPIO_InitTypeDef gpio_init;

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  gpio_init.Alternate = 0;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_LOW;

  //Init All IO
  gpio_init.Mode = GPIO_MODE_INPUT;
  for(uint32_t i = 0;i < sizeof(test_pin) / sizeof(hw_pin_st);i++)
  {
    if(test_pin[i].gpio)
    {
      gpio_init.Pin = test_pin[i].pin;
      HAL_GPIO_Init(test_pin[i].gpio, &gpio_init);
    }
  }

  /*HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
  gpio_init.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &gpio_init);*/

  power_en = 0;
  for(uint32_t i = 0;(i < sizeof(test->_4.config->power) / sizeof(power_test_st)) && (i < 8);i++)
  {
    if(test->_4.config->power[i].en)
      power_en |= (1U << i);
  }

  for(uint32_t i = 8;(i < sizeof(test->_4.config->power) / sizeof(power_test_st)) && (i < 12);i++)
  {
    if(test->_4.config->power[i].en)
      power_en |= (1U << ((12 - 1 - i) + 8));
  }

  pca9535_set_output(1, power_en | 0x0000);
  pca9535_config_port(1, 0x0000);
}

static void test_4_deinit(void)
{
  pca9535_set_output(1, 0);
}

static void test_task(void* p)
{
  uint32_t i;
  while(is_test)
  {
    test_4_init();
    test->stage = TEST_STAGE_WAIT;

    test_io_init(io, sizeof(io) / sizeof(io[0]));

    //等待引脚被拉高
    i = 0;
    while(is_test)
    {
      gnd_enable(gnd, sizeof(gnd) / sizeof(gnd[0]));
      hw_i2c_fix(hw_i2c4, 10);
      osDelay(10);
      uint8_t tmp;
      test->mic1_rdy = hw_i2c_read_1a_1d(hw_i2c4, 0xA8, 0x00, &tmp, 10) == 0 ? 1 : 0;

      gnd_disable(gnd, sizeof(gnd) / sizeof(gnd[0]));

      osDelay(10);

      if(test->mic1_rdy || test_io_is_have_high_lvl(io, sizeof(io) / sizeof(io[0])))
      {
        i++;
        if(i > 5) break;
      }
      else
      {
        i = 0;
      }

      osDelay(30);
    };

    if(!is_test)
    {
      test_4_deinit();
      break;
    }

    //开始开短路测试
    test->stage = TEST_STAGE_OS;
    osDelay(1000);
    test->io_test_error = 0;

    //开路测试
    test->io_open_circuit_count = test_io_check_open_circuit(io, sizeof(io) / sizeof(io[0]), test->io_open_circuit_data);
    if(test->io_open_circuit_count == 0)
    {
      //短路测试
      test->io_short_circuit_count = test_io_check_short_circuit(io, sizeof(io) / sizeof(io[0]), test->io_short_circuit_data);

      if(test->io_short_circuit_count)
        test->io_test_error = -2;
    }
    else
    {
      test->io_test_error = -1;
    }

    if(test->io_test_error != 0)
    {
      wav_play(audio_file_warning);
    }

    //功能测试
    test->stage = TEST_STAGE_FUNC;
    gnd_enable(gnd, sizeof(gnd) / sizeof(gnd[0]));
    mic_init();
    hw_i2c_fix(hw_i2c4, 10);
    while(is_test)
    {
      //更新麦克风数据，用于进度条显示
      mic_update_data();

      uint8_t tmp;
      if(hw_i2c_read_1a_1d(hw_i2c4, 0xA8, 0x00, &tmp, 10) == 0)
      {
        test->mic1_rdy = 1;
      }
      else
      {
        if(test->mic1_rdy == 1)
          wav_play(audio_file_warning);

        test->mic1_rdy = 0;
      }

      //检测模组是否已取出
      if((test_io_is_have_high_lvl(io, sizeof(io) / sizeof(io[0])) == 0) && (test->mic1_rdy == 0))
      {
        break;
      }

      osDelay(50);
    }

    mic_deinit();
    gnd_disable(gnd, sizeof(gnd) / sizeof(gnd[0]));
    test_4_deinit();
  }

  osKernelLock();
  vPortFree(test);
  test = 0;
  osKernelUnlock();

  osThreadExit();
}

void test_4_stop(void)
{
  is_test = 0;
}

int test_4_is_running(void)
{
  return test ? 1 : 0;
}

static uint8_t stage_get(void)
{
  return test->stage;
}

static uint8_t mic1_is_rdy(void)
{
  return test->mic1_rdy;
}

static int io_test_error_get(void)
{
  return test->io_test_error;
}

static uint8_t* io_open_circuit_data_get(void)
{
  return test->io_open_circuit_data;
}

static uint8_t io_open_circuit_count_get(void)
{
  return test->io_open_circuit_count;
}

static uint32_t io_short_circuit_count_get(void)
{
  return test->io_short_circuit_count;
}

static uint16_t* io_short_circuit_data_get(void)
{
  return test->io_short_circuit_data;
}

static test_4_config_st* config_parse(const config_file_st* config_file, const test_4_config_st* config_default)
{
  test_4_config_st* config;
  cJSON* json;
  uint32_t str_length;

  config = pvPortMalloc(sizeof(test_4_config_st));
  if(config == 0)
  {
    goto __exit;
  }

  memcpy(config, config_default, sizeof(test_4_config_st));

  if(config_file == 0)
  {
    goto __exit;
  }

  if(memcmp(config_file->file.type, "json", 4) != 0)
  {
    goto __exit;
  }

  json = cJSON_Parse((char*)config_file->file.data);
  if(json == NULL)
  {
    goto __exit;
  }

  cJSON* j_name = cJSON_GetObjectItem(json, json_obj_name);
  if(j_name != NULL)
  {
    snprintf(config->info.name, sizeof(config->info.name), "%s", j_name->valuestring);
  }

  cJSON* j_desc = cJSON_GetObjectItem(json, json_obj_description);
  if(j_desc != NULL)
  {
    snprintf(config->info.desc, sizeof(config->info.desc), "%s", j_desc->valuestring);
  }

  cJSON* j_version = cJSON_GetObjectItem(json, json_obj_version);
  if(j_version != NULL)
  {
    config->info.version = j_version->valueint;
  }

  cJSON* j_test = cJSON_GetObjectItem(json, json_obj_test);
  if(j_test != NULL)
  {
    cJSON* j_power = cJSON_GetObjectItem(j_test, json_obj_power);
    if(j_power != NULL)
    {
      char power_n[3];
      for(uint32_t i = 0;i < sizeof(config->power) / sizeof(power_test_st);i++)
      {
        snprintf(power_n, sizeof(power_n), "%d", i + 1);
        cJSON* j_power_n = cJSON_GetObjectItem(j_power, power_n);
        if(j_power_n != NULL)
        {
          //          cJSON* j_power_enable = cJSON_GetObjectItem(j_power_n, json_obj_enable);
          //          if(j_power_enable != NULL)
          //          {
          //            config->power[i].en = j_power_enable->valueint;
          //          }

          cJSON* j_power_vdd = cJSON_GetObjectItem(j_power_n, json_obj_vdd);
          if(j_power_vdd != NULL)
          {
            config->power[i].vdd = j_power_vdd->valuedouble;
          }

          cJSON* j_power_v_max = cJSON_GetObjectItem(j_power_n, json_obj_v_max);
          if(j_power_v_max != NULL)
          {
            config->power[i].v_max = j_power_v_max->valuedouble;
          }

          cJSON* j_power_v_min = cJSON_GetObjectItem(j_power_n, json_obj_v_min);
          if(j_power_v_min != NULL)
          {
            config->power[i].v_min = j_power_v_min->valuedouble;
          }

          cJSON* j_power_i_max = cJSON_GetObjectItem(j_power_n, json_obj_i_max);
          if(j_power_i_max != NULL)
          {
            config->power[i].i_max = j_power_i_max->valuedouble;
          }

          cJSON* j_power_i_min = cJSON_GetObjectItem(j_power_n, json_obj_i_min);
          if(j_power_i_min != NULL)
          {
            config->power[i].i_min = j_power_i_min->valuedouble;
          }

          cJSON* j_power_color_in = cJSON_GetObjectItem(j_power_n, json_obj_color_in);
          if(j_power_color_in != NULL)
          {
            sscanf(j_power_color_in->valuestring, "%x", &config->power[i].color_in);
          }

          cJSON* j_power_color_out = cJSON_GetObjectItem(j_power_n, json_obj_color_out);
          if(j_power_color_out != NULL)
          {
            sscanf(j_power_color_out->valuestring, "%x", &config->power[i].color_out);
          }
        }
      }

      cJSON* j_power_mvi = cJSON_GetObjectItem(j_power, json_obj_mvi);
      if(j_power_mvi != NULL)
      {
        cJSON* j_power_vdd = cJSON_GetObjectItem(j_power_mvi, json_obj_vdd);
        if(j_power_vdd != NULL)
        {
          config->mvi.vdd = j_power_vdd->valuedouble;
        }

        cJSON* j_power_v_max = cJSON_GetObjectItem(j_power_mvi, json_obj_v_max);
        if(j_power_v_max != NULL)
        {
          config->mvi.v_max = j_power_v_max->valuedouble;
        }

        cJSON* j_power_v_min = cJSON_GetObjectItem(j_power_mvi, json_obj_v_min);
        if(j_power_v_min != NULL)
        {
          config->mvi.v_min = j_power_v_min->valuedouble;
        }

        cJSON* j_power_i_max = cJSON_GetObjectItem(j_power_mvi, json_obj_i_max);
        if(j_power_i_max != NULL)
        {
          config->mvi.i_max = j_power_i_max->valuedouble;
        }

        cJSON* j_power_i_min = cJSON_GetObjectItem(j_power_mvi, json_obj_i_min);
        if(j_power_i_min != NULL)
        {
          config->mvi.i_min = j_power_i_min->valuedouble;
        }

        cJSON* j_power_color_in = cJSON_GetObjectItem(j_power_mvi, json_obj_color_in);
        if(j_power_color_in != NULL)
        {
          sscanf(j_power_color_in->valuestring, "%x", &config->mvi.color_in);
        }

        cJSON* j_power_color_out = cJSON_GetObjectItem(j_power_mvi, json_obj_color_out);
        if(j_power_color_out != NULL)
        {
          sscanf(j_power_color_out->valuestring, "%x", &config->mvi.color_out);
        }
      }
    }
  }

  cJSON_Delete(json);

__exit:
  return config;
}

int test_4_start(test_module_st* module, const char* title)
{
  int err;
  test_4_config_st* config;

  osKernelLock();

  if(test)
  {
    err = 1;
    goto __exit;
  }

  test = pvPortMalloc(sizeof(*test));
  if(test == 0)
  {
    err = -1;
    goto __exit;
  }

  config = config_parse(module->config_file, &config_default);
  if(config == 0)
  {
    err = -2;
    goto __free_test;
  }

  if(osThreadNew(test_task, 0, &test_tid_attr) == 0)
  {
    err = -3;
    goto __free_config;
  }

  memset(test, 0, sizeof(*test));

  test->_4.config = config;

  test->_4.stage_get = stage_get;
  test->_4.mic1_is_rdy = mic1_is_rdy;
  test->_4.mic_get = mic_get_percentage;
  test->_4.mic_record_play = mic_record_play;
  test->_4.mic_record_start = mic_record_start;
  test->_4.mic_record_get_progress = mic_record_get_progress;
  test->_4.mic_record_stop = mic_record_stop;
  test->_4.ma_get = test_ma_get;
  test->_4.mv_get = test_mv_get;
  test->_4.ma_is_update = test_ma_is_update;
  test->_4.io_test_error_get = io_test_error_get;
  test->_4.io_open_circuit_data_get = io_open_circuit_data_get;
  test->_4.io_open_circuit_count_get = io_open_circuit_count_get;
  test->_4.io_short_circuit_count_get = io_short_circuit_count_get;
  test->_4.io_short_circuit_data_get = io_short_circuit_data_get;

  err = 0;
  is_test = 1;

  goto __exit;

__free_config:
  vPortFree(config);

__free_test:
  vPortFree(test);
  test = 0;

__exit:
  osKernelUnlock();

  if(err == 0)
    ui_test_4_open(&test->_4, title);

  return err;
}

const test_st test_ipx_j6400 =
{
  &ipx_j6400_id, test_4_start, test_4_stop, test_4_is_running, "XP X J6400"
};
