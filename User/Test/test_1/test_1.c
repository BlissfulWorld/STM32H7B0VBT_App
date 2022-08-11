#include "test_1.h"
#include "conFig.h"
#include "main.h"
#include "gpio.h"
#include "i2c.h"
#include "adc.h"
#include "pca9535.h"
#include "hw.h"
#include "cmsis_os.h"
#include "ui.h"
#include "string.h"
#include "stdio.h"
#include "cJSON.h"
#include "json_obj.h"
#include "ui.h"
#include "wav.h"
#include "tim.h"

const test_1_config_st test_1_config_default =
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


  VI_CONST_INIT(1,1),
  VI_CONST_INIT(2,1),
  VI_CONST_INIT(3,1),
  VI_CONST_INIT(4,1),
  VI_CONST_INIT(5,1),
  VI_CONST_INIT(6,1),
  VI_CONST_INIT(7,0),
  VI_CONST_INIT(8,0),
  //VI_CONST_INIT(9,0),
  //VI_CONST_INIT(10,0),
  //VI_CONST_INIT(11,0),
  //VI_CONST_INIT(12,0),

  .mvi.i_max = 5000,
  .mvi.i_min = 0,
  .mvi.v_max = 5000,
  .mvi.v_min = 0,
  .mvi.color_in = 0xFFFFFF,
  .mvi.color_out = 0xFF0000,
};

static test_1_st test_1;

extern void test_1_als_init(void);
extern void test_1_prox_init(void);
extern int test_1_mic_init(void);
extern void test_1_spkramp_init(void);
extern void test_1_als_set_intf(const test_1_interface_st* i);
extern void test_1_prox_set_intf(const test_1_interface_st* i);
extern void test_1_spkramp_set_intf(const test_1_interface_st* i);

void test_1_init(const test_1_interface_st* intf)
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


  test_1_als_set_intf(intf);
  test_1_prox_set_intf(intf);
  test_1_spkramp_set_intf(intf);

  power_en = 0;
  for(uint32_t i = 0;i < sizeof(test_1.config->power) / sizeof(power_test_st);i++)
  {
    if(test_1.config->power[i].en)
      power_en |= (1 << i);
  }

  pca9535_set_output(1, power_en | 0x1000);
  pca9535_config_port(1, 0x0000);

  intf->als_i2c_init();
  intf->prox_i2c_init();

  test_1_als_init();
  test_1_prox_init();
  test_1_mic_init();
  test_1_spkramp_init();
}

static int start = 0;
static uint8_t stage;

static osThreadId_t test_1_tid;
static const osThreadAttr_t test_1_tid_attr = {
  .name = "test 2",
  .stack_size = 1024 * 8,
  .priority = (osPriority_t)osPriorityNormal,
};

extern uint8_t test_1_prox_detect(void);
extern void test_1_prox_update_data(void);
extern uint8_t test_1_prox_get_percentage(void);
extern uint8_t test_1_prox_is_rdy(void);

extern uint8_t test_1_als_detect(void);
extern void test_1_als_update_data(void);
extern uint8_t test_1_als_get_percentage(void);
extern uint8_t test_1_als_is_rdy(void);

extern uint8_t test_1_mic_get_percentage(void);
extern void test_1_mic_update_data(void);
extern int test_1_mic_record_start(void);
extern int test_1_mic_record_stop(void);
extern int test_1_mic_record_play(void);
extern uint8_t test_1_mic_record_get_progress(void);

extern void test_1_spkramp_test_poll(void);
extern void test_1_spkramp_test_poll_end(void);
extern int test_1_spkramp_play(void);
extern uint8_t test_1_spkramp_res_get(void);

static uint8_t codec_aout_to_hac_test_res;

static uint8_t codec_aout_to_hac_test(const test_1_interface_st* intf)
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

static void test_1_task(void* p)
{
  int i = 0;
  uint32_t tick;
  const test_1_interface_st* intf = p;
  uint32_t err_flg;
  uint32_t err_flg_new;

  test_1_init(intf);
  while(test_1_tid)
  {
    stage = TEST_STAGE_WAIT;

    i = 0;
    while(test_1_tid)
    {
      codec_aout_to_hac_test_res = codec_aout_to_hac_test(intf);
      test_1_spkramp_test_poll();
      if((test_1_prox_detect() == 0) || (test_1_als_detect() == 0) || codec_aout_to_hac_test_res || test_1_spkramp_res_get())
      {
        i++;
        if(i > 5) break;
      }
      else
      {
        i = 0;
        intf->prox_i2c_fix();
        intf->als_i2c_fix();
      }

      osDelay(50);
    };

    stage = TEST_STAGE_OS;

    osDelay(1000);

    stage = TEST_STAGE_FUNC;

    err_flg = 0;
    err_flg_new = 0x0000001F;
    i = 0;
    tick = HAL_GetTick();
    while(test_1_tid)
    {
      test_1_mic_update_data();
      test_1_als_update_data();

      if(HAL_GetTick() - tick > 50)
      {
        tick += 50;

        uint32_t flag = 0;

        test_1_prox_update_data();

        if(test_1_als_detect() == 0)
        {
          flag |= (1U << 0);
          intf->als_i2c_fix();
        }

        if(test_1_prox_is_rdy() != 0)
        {
          flag |= (1U << 1);
          intf->prox_i2c_fix();
        }

        test_1_spkramp_test_poll();

        if((test_1_spkramp_res_get() & 1) != 0)
        {
          flag |= (1U << 2);
        }

        if((test_1_spkramp_res_get() & 2) != 0)
        {
          flag |= (1U << 3);
        }

        codec_aout_to_hac_test_res = codec_aout_to_hac_test(intf);
        if(codec_aout_to_hac_test_res != 0)
        {
          flag |= (1U << 4);
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
            if(err_flg_new != 0x0000001F)
            {
              if(err_flg_new != err_flg)
              {
                for(uint32_t i = 1;i < (1 << 5);i <<= 1)
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
          err_flg_new = 0x0000001F;
        }
      }

      osDelay(10);
    }

    test_1_spkramp_test_poll_end();
  }

  start--;
  osThreadExit();
}

void test_1_stop(void)
{
  if(start == 0) return;

  test_1_tid = 0;
}

static uint8_t test_1_stage_get(void)
{
  return stage;
}

static uint32_t test_1_os_result_get(void)
{
  return 0;
}

static uint8_t test_1_prox_get(void)
{
  return test_1_prox_get_percentage();
}

static uint8_t _test_1_prox_is_rdy(void)
{
  return test_1_prox_is_rdy();
}

static uint8_t test_1_als_get(void)
{
  return test_1_als_get_percentage();
}

static uint8_t _test_1_als_is_rdy(void)
{
  return test_1_als_is_rdy();
}

static uint8_t codec_aout_to_hac_test_res_get(void)
{
  return codec_aout_to_hac_test_res;
}

static int spkramp_play(void)
{
  return test_1_spkramp_play();
}

static uint8_t spkramp_test_res_get(void)
{
  return test_1_spkramp_res_get();
}

static test_1_config_st* config_parse(const config_file_st* config_file)
{
  test_1_config_st* config;
  cJSON* json;
  uint32_t str_length;

  config = pvPortMalloc(sizeof(test_1_config_st));
  if(config == 0)
  {
    goto __exit;
  }

  memcpy(config, &test_1_config_default, sizeof(test_1_config_st));

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
          cJSON* j_power_enable = cJSON_GetObjectItem(j_power_n, json_obj_enable);
          if(j_power_enable != NULL)
          {
            config->power[i].en = j_power_enable->valueint;
          }

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

int test_1_start(test_module_st* module, const test_1_interface_st* intf, const char* title)
{
  int err;
  test_1_config_st* config;

  err = 0;

  config = config_parse(module->config_file);
  if(config == 0)
  {
    return -1;
  }

  test_1.config = config;
  test_1.stage_get = test_1_stage_get;
  test_1.prox_get = test_1_prox_get;
  test_1.prox_is_rdy = test_1_prox_is_rdy;
  test_1.als_get = test_1_als_get;
  test_1.als_is_rdy = test_1_als_is_rdy;
  test_1.mic_get = test_1_mic_get_percentage;
  test_1.mic_record_play = test_1_mic_record_play;
  test_1.mic_record_start = test_1_mic_record_start;
  test_1.mic_record_get_progress = test_1_mic_record_get_progress;
  test_1.mic_record_stop = test_1_mic_record_stop;
  test_1.ma_get = test_ma_get;
  test_1.mv_get = test_mv_get;
  test_1.ma_is_update = test_ma_is_update;
  test_1.codec_aout_to_hac_test_res = codec_aout_to_hac_test_res_get;
  test_1.spkramp_play = spkramp_play;
  test_1.spkramp_test_res = spkramp_test_res_get;

  ui_test_1_open(&test_1, title);

  if(start == 0)
  {
    start = 1;
    test_1_tid = osThreadNew(test_1_task, (void*)intf, &test_1_tid_attr);
    if(test_1_tid == NULL)
    {
      start = 0;
    }
  }

  return 0;
}
