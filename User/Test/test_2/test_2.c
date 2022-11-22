#include "test_2.h"
#include "gpio.h"
#include "pca9535.h"
#include "cmsis_os.h"
#include "string.h"
#include "stdio.h"
#include "cJSON.h"
#include "json_obj.h"
#include "wav.h"
#include "resource.h"
#include "mic.h"

extern void test_2_als_init(void);
extern void test_2_prox_init(void);
extern void test_2_spkramp_init(void);
extern void test_2_als_set_intf(const test_2_interface_st* i);
extern void test_2_prox_set_intf(const test_2_interface_st* i);
extern void test_2_spkramp_set_intf(const test_2_interface_st* i);

extern uint8_t test_2_prox_detect(void);
extern void test_2_prox_update_data(void);
extern uint8_t test_2_prox_get_percentage(void);
extern uint8_t test_2_prox_is_rdy(void);

extern uint8_t test_2_als_detect(void);
extern void test_2_als_update_data(void);
extern uint8_t test_2_als_get_percentage(void);
extern uint8_t test_2_als_is_rdy(void);

extern void test_2_spkramp_test_poll(void);
extern void test_2_spkramp_test_poll_end(void);
extern int test_2_spkramp_play(void);
extern uint8_t test_2_spkramp_res_get(void);

static struct
{
  uint8_t stage;
  test_2_st _2;
  float os_test_data[50];
  uint8_t os_test_data_len;
  uint8_t os_test_result;

  uint8_t codec_aout_to_hac_test_res;
  uint8_t yogi_rdy;

  const test_2_interface_st* intf;
}*test;

static const osThreadAttr_t test_tid_attr = {
  .name = "test 2",
  .stack_size = 1024 * 8,
  .priority = (osPriority_t)osPriorityNormal,
};

static int is_test = 0;

void test_2_init(const test_2_interface_st* intf)
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

  //GND
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
  for(uint32_t i = 0;intf->gnd_pin_list[i];i++)
  {
    if(test_pin[intf->gnd_pin_list[i]].gpio)
    {
      HAL_GPIO_WritePin(test_pin[intf->gnd_pin_list[i]].gpio, test_pin[intf->gnd_pin_list[i]].pin, GPIO_PIN_RESET);
      gpio_init.Pin = test_pin[intf->gnd_pin_list[i]].pin;
      HAL_GPIO_Init(test_pin[intf->gnd_pin_list[i]].gpio, &gpio_init);
    }
  }

  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
  gpio_init.Pin = GPIO_PIN_15;
  HAL_GPIO_Init(GPIOD, &gpio_init);

  //adc1
  gpio_init.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  gpio_init.Mode = GPIO_MODE_ANALOG;
  gpio_init.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &gpio_init);

  //cam pwdn
  HAL_GPIO_WritePin(test_pin[33].gpio, test_pin[33].pin, GPIO_PIN_SET);
  gpio_init.Pull = GPIO_PULLUP;
  gpio_init.Mode = GPIO_MODE_OUTPUT_OD;
  gpio_init.Pin = test_pin[33].pin;
  HAL_GPIO_Init(test_pin[33].gpio, &gpio_init);

  //clk
  gpio_init.Pin = GPIO_PIN_8;
  gpio_init.Mode = GPIO_MODE_AF_PP;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gpio_init.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &gpio_init);

  test_2_als_set_intf(intf);
  test_2_prox_set_intf(intf);
  test_2_spkramp_set_intf(intf);

  power_en = 0;
  for(uint32_t i = 0;i < sizeof(test->_2.config->power) / sizeof(power_test_st);i++)
  {
    if(test->_2.config->power[i].en)
      power_en |= (1U << i);
  }

  for(uint32_t i = 8;(i < sizeof(test->_2.config->power) / sizeof(power_test_st)) && (i < 12);i++)
  {
    if(test->_2.config->power[i].en)
      power_en |= (1U << ((12 - 1 - i) + 8));
  }

  pca9535_set_output(1, power_en | 0x1000);
  pca9535_config_port(1, 0x0000);

  intf->i2c_init();

  test_2_als_init();
  test_2_prox_init();
  mic_init();
  test_2_spkramp_init();
}

static void test_2_deinit(const test_2_interface_st* intf)
{
  GPIO_InitTypeDef gpio_init;

  mic_deinit();

  pca9535_set_output(1, 0);

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
}

static uint8_t yogi_detect(const test_2_interface_st* intf)
{
  uint8_t addr[1] = { 0 };
  uint8_t data[2];

  if(intf->i2c_write_read(0x66, addr, sizeof(addr), data, sizeof(data), 100) == 0)
    return 1;

  return 0;
}

static uint8_t codec_aout_to_hac_test(const test_2_interface_st* intf)
{
  uint8_t p_pin, n_pin;

  uint8_t cnt = 0;

  p_pin = intf->codec_aout_to_hac_conn_p_pin;
  n_pin = intf->codec_aout_to_hac_conn_n_pin;

  GPIO_InitTypeDef gpio_init;

  gpio_init.Alternate = 0;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;

  //SPK_OUT_P
  HAL_GPIO_WritePin(test_pin[intf->codec_aout_to_hac_conn_p_pin].gpio, test_pin[p_pin].pin, GPIO_PIN_SET);
  gpio_init.Pin = test_pin[p_pin].pin;
  HAL_GPIO_Init(test_pin[p_pin].gpio, &gpio_init);

  osDelay(1);

  //SPK_VSENSE_P
  if(HAL_GPIO_ReadPin(test_pin[n_pin].gpio, test_pin[n_pin].pin) == GPIO_PIN_SET)
  {
    cnt++;
  }

  //SPK_OUT_P
  HAL_GPIO_WritePin(test_pin[p_pin].gpio, test_pin[p_pin].pin, GPIO_PIN_RESET);

  osDelay(1);

  //SPK_VSENSE_P
  if(HAL_GPIO_ReadPin(test_pin[n_pin].gpio, test_pin[n_pin].pin) == GPIO_PIN_RESET)
  {
    cnt++;
  }

  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  gpio_init.Mode = GPIO_MODE_INPUT;

  //SPK_OUT_P
  HAL_GPIO_WritePin(test_pin[p_pin].gpio, test_pin[p_pin].pin, GPIO_PIN_SET);
  gpio_init.Pin = test_pin[p_pin].pin;
  HAL_GPIO_Init(test_pin[p_pin].gpio, &gpio_init);

  return cnt == 2 ? 1 : 0;
}

static void test_task(void* p)
{
  int i = 0;
  uint32_t tick;
  const test_2_interface_st* intf = test->intf;
  uint32_t err_flg;
  uint32_t err_flg_new;
  uint8_t ostest;

  while(is_test)
  {
    test_2_init(intf);
    test->stage = TEST_STAGE_WAIT;

    i = 0;
    while(is_test)
    {
      test->codec_aout_to_hac_test_res = codec_aout_to_hac_test(intf);
      test->yogi_rdy = yogi_detect(intf);
      test_2_spkramp_test_poll();
      if((test_2_prox_detect() == 0) || (test_2_als_detect() == 0) || test->codec_aout_to_hac_test_res || test_2_spkramp_res_get() || test->yogi_rdy)
      {
        i++;
        if(i > 5) break;
      }
      else
      {
        i = 0;
        intf->i2c_fix();
      }

      osDelay(50);
    };

    if(!is_test)
    {
      test_2_deinit(intf);
      break;
    }

    test->stage = TEST_STAGE_OS;
    osDelay(400);

    ostest = 1;
    US_GET(ostest, &ostest);
    test->os_test_data_len = 0;
    test->os_test_result = 0;
    if(ostest)
    {
      test_2_deinit(intf);

      test_current_stop();
      pca9535_set_output(1, 0);
      for(test->os_test_data_len = 0;intf->func_pin_list[test->os_test_data_len];test->os_test_data_len++);
      os_test_get_volt(intf->gnd_pin_list, intf->func_pin_list, test->os_test_data);

      test->os_test_result = 0;
      for(uint8_t i = 0;i < test->_2.config->os_test_num;i++)
      {
        if(test->_2.config->ost[i].i_max < test->os_test_data[i] || test->_2.config->ost[i].i_min > test->os_test_data[i])
        {
          test->os_test_result++;
        }
      }

      osDelay(100);
      test_current_start();

      if(test->os_test_result) wav_play(audio_file_warning);

      test_2_init(intf);
      osDelay(1000);
    }

    err_flg = 0;
    err_flg_new = 0x0000003F;
    i = 0;
    test_2_spkramp_test_poll();
    test->codec_aout_to_hac_test_res = codec_aout_to_hac_test(intf);
    test->yogi_rdy = yogi_detect(intf);
    mic_update_data();
    test_2_als_update_data();
    test_2_prox_update_data();
    test_2_spkramp_play();
    test->stage = TEST_STAGE_FUNC;
    tick = HAL_GetTick();
    while(is_test)
    {
      mic_update_data();
      test_2_als_update_data();

      if(HAL_GetTick() - tick > 50)
      {
        tick += 50;

        uint32_t flag = 0;

        test_2_prox_update_data();
        test_2_spkramp_test_poll();
        test->codec_aout_to_hac_test_res = codec_aout_to_hac_test(intf);
        test->yogi_rdy = yogi_detect(intf);

        if(test_2_prox_is_rdy() != 0)
        {
          flag |= (1U << 0);
        }
        else
        {
          intf->i2c_fix();
        }

        if(test_2_als_detect() == 0)
        {
          flag |= (1U << 1);
        }

        if((test_2_spkramp_res_get() & 1) != 0)
        {
          flag |= (1U << 2);
        }

        if((test_2_spkramp_res_get() & 2) != 0)
        {
          flag |= (1U << 3);
        }

        if(test->codec_aout_to_hac_test_res != 0)
        {
          flag |= (1U << 4);
        }

        if(test->yogi_rdy != 0)
        {
          flag |= (1U << 5);
        }

        if(flag == 0)
          break;

        err_flg_new &= (~flag);

        i++;
        if(i == 4)
        {
          i = 0;

          if(err_flg_new)
          {
            if(err_flg_new != 0x0000003F)
            {
              if(err_flg_new != err_flg)
              {
                for(uint32_t i = 1;i < (1 << 6);i <<= 1)
                {
                  if((err_flg_new & i) != 0 && (err_flg & i) == 0)
                  {
                    wav_play(audio_file_warning);
                    break;
                  }
                }
              }
            }
            else
            {
              break;
            }
          }

          err_flg = err_flg_new;
          err_flg_new = 0x0000003F;
        }
      }

      osDelay(10);
    }

    test_2_spkramp_test_poll_end();
    test_2_deinit(intf);
  }

  osKernelLock();
  vPortFree(test);
  test = 0;
  osKernelUnlock();

  osThreadExit();
}

void test_2_stop(void)
{
  is_test = 0;
}

int test_2_is_running(void)
{
  return test ? 1 : 0;
}

static uint8_t stage_get(void)
{
  return test->stage;
}

static const float* os_test_data_get(uint8_t* len)
{
  *len = test->os_test_data_len;
  return test->os_test_data;
}

static uint8_t os_test_result_get(void)
{
  return test->os_test_result;
}

static uint8_t prox_get(void)
{
  return test_2_prox_get_percentage();
}

static uint8_t prox_is_rdy(void)
{
  return test_2_prox_is_rdy();
}

static uint8_t als_get(void)
{
  return test_2_als_get_percentage();
}

static uint8_t als_is_rdy(void)
{
  return test_2_als_is_rdy();
}

static uint8_t codec_aout_to_hac_test_res_get(void)
{
  return test->codec_aout_to_hac_test_res;
}

static int spkramp_play(void)
{
  return test_2_spkramp_play();
}

static uint8_t spkramp_test_res_get(void)
{
  return test_2_spkramp_res_get();
}

static uint8_t yogi_is_rdy(void)
{
  return test->yogi_rdy;
}

static test_2_config_st* config_parse(const config_file_st* config_file, const test_2_config_st* config_default)
{
  test_2_config_st* config;
  cJSON* json;
  uint32_t str_length;

  config = pvPortMalloc(sizeof(test_2_config_st));
  if(config == 0)
  {
    goto __exit;
  }

  memcpy(config, config_default, sizeof(test_2_config_st));

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

    ostest_json_config_phase(j_test, config->ost, config->os_test_num);
  }

  cJSON_Delete(json);

__exit:
  return config;
}

int test_2_start(test_module_st* module, const test_2_interface_st* intf, const test_2_config_st* config_default, const char* title)
{
  int err;
  test_2_config_st* config;

  osKernelLock();

  if(test)
  {
    err = 1;
    goto __unlock;
  }

  test = pvPortMalloc(sizeof(*test));
  if(test == 0)
  {
    err = -1;
    goto __unlock;
  }

  config = config_parse(module->config_file, config_default);
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

  test->intf = intf;

  test->_2.config = config;
  test->_2.stage_get = stage_get;
  test->_2.os_test_result_get = os_test_result_get;
  test->_2.os_test_data_get = os_test_data_get;
  test->_2.prox_get = prox_get;
  test->_2.prox_is_rdy = prox_is_rdy;
  test->_2.als_get = als_get;
  test->_2.als_is_rdy = als_is_rdy;
  test->_2.mic_get = mic_get_percentage;
  test->_2.mic_record_play = mic_record_play;
  test->_2.mic_record_start = mic_record_start;
  test->_2.mic_record_get_progress = mic_record_get_progress;
  test->_2.mic_record_stop = mic_record_stop;
  test->_2.ma_get = test_ma_get;
  test->_2.mv_get = test_mv_get;
  test->_2.ma_is_update = test_ma_is_update;
  test->_2.codec_aout_to_hac_test_res = codec_aout_to_hac_test_res_get;
  test->_2.spkramp_play = spkramp_play;
  test->_2.spkramp_test_res = spkramp_test_res_get;
  test->_2.yogi_is_rdy = yogi_is_rdy;

  err = 0;
  is_test = 1;

  goto __unlock;

__free_config:
  vPortFree(config);

__free_test:
  vPortFree(test);
  test = 0;

__unlock:
  osKernelUnlock();

  if(err == 0)
    ui_test_2_open(&test->_2, title);

  return err;
}
