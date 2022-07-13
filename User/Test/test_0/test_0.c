#include "test_0.h"
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

const test_0_config_st test_0_config_default = 
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
  VI_CONST_INIT(9,0),
  VI_CONST_INIT(10,0),
  VI_CONST_INIT(11,0),
  VI_CONST_INIT(12,0),

  .mvi.i_max = 5000,
  .mvi.i_min = 0,
  .mvi.v_max = 5000,
  .mvi.v_min = 0,
  .mvi.color_in = 0xFFFFFF,
  .mvi.color_out = 0xFF0000,
};

static test_0_st test_0;

extern void test_0_als_init(void);
extern void test_0_prox_init(void);
extern int test_0_mic_init(void);
extern void test_0_als_set_intf(const test_0_interface_st *i);
extern void test_0_prox_set_intf(const test_0_interface_st *i);

void test_0_init(const test_0_interface_st *intf)
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
  

  test_0_als_set_intf(intf);
  test_0_prox_set_intf(intf);

  intf->cam_i2c_init();
  
  
  power_en = 0;
  for(uint32_t i = 0;i < sizeof(test_0.config->power) / sizeof(power_test_st);i++)
  {
    if(test_0.config->power[i].en)
      power_en |= (1 << i);
  }

  pca9535_config_output(1, power_en);
  pca9535_config_port(1, 0x0000);

  intf->als_i2c_init();
  intf->prox_i2c_init();

  test_0_als_init();
  test_0_prox_init();
  test_0_mic_init();
}

static int start = 0;
static uint8_t stage;

static osThreadId_t test_0_tid;
static const osThreadAttr_t test_0_tid_attr = {
  .name = "test 0",
  .stack_size = 1024 * 8,
  .priority = (osPriority_t)osPriorityNormal,
};

extern uint8_t test_0_prox_detect(void);
extern void test_0_prox_update_data(void);
extern uint8_t test_0_prox_get_percentage(void);
extern uint8_t test_0_prox_is_rdy(void);

extern uint8_t test_0_als_detect(void);
extern void test_0_als_update_data(void);
extern uint8_t test_0_als_get_percentage(void);
extern uint8_t test_0_als_is_rdy(void);

extern uint8_t test_0_mic_get_percentage(void);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        
extern void test_0_mic_update_data(void);

static uint8_t spk_pn;

static uint8_t spk_pn_detect(void)
{
#define SPK_OUT_P_PIN (8)
#define SPK_OUT_N_PIN (9)
#define SPK_VSENSE_P_PIN (10)
#define SPK_VSENSE_N_PIN (7)
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
  uint8_t p = 0, n = 0;

  GPIO_InitTypeDef gpio_init;

  gpio_init.Alternate = 0;
  gpio_init.Pull = GPIO_NOPULL;
  gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
  gpio_init.Mode = GPIO_MODE_OUTPUT_PP;

  //SPK_OUT_P
  HAL_GPIO_WritePin(test_pin[SPK_OUT_P_PIN].gpio, test_pin[SPK_OUT_P_PIN].pin, GPIO_PIN_SET);
  gpio_init.Pin = test_pin[SPK_OUT_P_PIN].pin;
  HAL_GPIO_Init(test_pin[SPK_OUT_P_PIN].gpio, &gpio_init);

  //SPK_OUT_N
  HAL_GPIO_WritePin(test_pin[SPK_OUT_N_PIN].gpio, test_pin[SPK_OUT_N_PIN].pin, GPIO_PIN_SET);
  gpio_init.Pin = test_pin[SPK_OUT_N_PIN].pin;
  HAL_GPIO_Init(test_pin[SPK_OUT_N_PIN].gpio, &gpio_init);

  osDelay(1);

  //SPK_VSENSE_P
  if(HAL_GPIO_ReadPin(test_pin[SPK_VSENSE_P_PIN].gpio, test_pin[SPK_VSENSE_P_PIN].pin) == GPIO_PIN_SET)
  {
    p++;
  }

  //SPK_VSENSE_N
  if(HAL_GPIO_ReadPin(test_pin[SPK_VSENSE_N_PIN].gpio, test_pin[SPK_VSENSE_N_PIN].pin) == GPIO_PIN_SET)
  {
    n++;
  }

  //SPK_OUT_P
  HAL_GPIO_WritePin(test_pin[SPK_OUT_P_PIN].gpio, test_pin[SPK_OUT_P_PIN].pin, GPIO_PIN_RESET);

  //SPK_OUT_N
  HAL_GPIO_WritePin(test_pin[SPK_OUT_N_PIN].gpio, test_pin[SPK_OUT_N_PIN].pin, GPIO_PIN_RESET);


  osDelay(1);

  //SPK_VSENSE_P
  if(HAL_GPIO_ReadPin(test_pin[SPK_VSENSE_P_PIN].gpio, test_pin[SPK_VSENSE_P_PIN].pin) == GPIO_PIN_RESET)
  {
    p++;
  }

  //SPK_VSENSE_N
  if(HAL_GPIO_ReadPin(test_pin[SPK_VSENSE_N_PIN].gpio, test_pin[SPK_VSENSE_N_PIN].pin) == GPIO_PIN_RESET)
  {
    n++;
  }

  gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
  gpio_init.Mode = GPIO_MODE_INPUT;

  //SPK_OUT_P
  HAL_GPIO_WritePin(test_pin[SPK_OUT_P_PIN].gpio, test_pin[SPK_OUT_P_PIN].pin, GPIO_PIN_SET);
  gpio_init.Pin = test_pin[SPK_OUT_P_PIN].pin;
  HAL_GPIO_Init(test_pin[SPK_OUT_P_PIN].gpio, &gpio_init);

  //SPK_OUT_N
  HAL_GPIO_WritePin(test_pin[SPK_OUT_N_PIN].gpio, test_pin[SPK_OUT_N_PIN].pin, GPIO_PIN_SET);
  gpio_init.Pin = test_pin[SPK_OUT_N_PIN].pin;
  HAL_GPIO_Init(test_pin[SPK_OUT_N_PIN].gpio, &gpio_init);

  return (p == 2 ? (1) : 0) | (n == 2 ? (2) : 0);
}

static void test_0_task(void* p)
{
  int i = 0;
  uint32_t tick;
  const test_0_interface_st *intf = p;

  test_0_init(intf);
  while(test_0_tid)
  {
    pca9535_config_output(1, 0x103F);

    stage = TEST_STAGE_WAIT;


    while(test_0_tid)
    {
      if((test_0_prox_detect() == 0) || (test_0_als_detect() == 0))
      {
        i++;
        if(i > 5) break;
      }
      else
      {
        i = 0;
      }

      osDelay(50);
    };

    stage = TEST_STAGE_OS;

    osDelay(1000);

    stage = TEST_STAGE_FUNC;

    tick = HAL_GetTick();
    while(test_0_tid)
    {
      test_0_mic_update_data();
      test_0_als_update_data();

      if(HAL_GetTick() - tick > 50)
      {
        tick += 50;

        spk_pn = spk_pn_detect();
        test_0_prox_update_data();
        
        if(test_0_prox_is_rdy() == 0)
          intf->prox_i2c_fix();

        if(test_0_als_detect() != 0)
        {
          intf->als_i2c_fix();
          if(test_0_prox_is_rdy() == 0)
            break;
        }
      }

      osDelay(10);
    }
  }

  start--;
  osThreadExit();
}

void test_0_stop(void)
{
  if(start == 0) return;

  test_0_tid = 0;
}

uint8_t test_0_stage_get(void)
{
  return stage;
}

uint32_t test_0_os_result_get(void)
{
  return 0;
}

uint8_t test_0_cam_is_rdy(void)
{
  uint8_t cam_id_addr[2];
  uint8_t cam_id[2];
  cam_id_addr[0] = 0;
  cam_id_addr[1] = 0;

  if(hw_sim_i2c_write_read(hw_sim_i2c1, 0x20, cam_id_addr, 2, cam_id, 2, 100) == 0)
    return 1;
  return 0;
}

uint8_t test_0_prox_get(void)
{
  return test_0_prox_get_percentage();
}

uint8_t _test_0_prox_is_rdy(void)
{
  return test_0_prox_is_rdy();
}

uint8_t test_0_als_get(void)
{
  return test_0_als_get_percentage();
}

uint8_t _test_0_als_is_rdy(void)
{
  return test_0_als_is_rdy();
}

uint8_t test_0_mic_get(void)
{
  return test_0_mic_get_percentage();
}

uint8_t test_0_spk_pn_get(void)
{
  return spk_pn;
}

static test_0_config_st * config_parse(const config_file_st *config_file)
{
  test_0_config_st *config;
  cJSON *json;
  uint32_t str_length;
  
  config = pvPortMalloc(sizeof(test_0_config_st));
  if(config == 0)
  {
    goto __exit;
  }
  
  memcpy(config,&test_0_config_default,sizeof(test_0_config_st));
  
  if(config_file == 0)
  {
    goto __exit;
  }
  
  if(memcmp(config_file->file.type,"json",4) != 0)
  {
    goto __exit;
  }
  
  json = cJSON_Parse((char *)config_file->file.data);
  if(json == NULL)
  {
    goto __exit;
  }
  
  cJSON *j_name = cJSON_GetObjectItem(json,json_obj_name);
  if(j_name != NULL)
  {
    snprintf(config->info.name,sizeof(config->info.name),"%s",j_name->valuestring);
  }
  
  cJSON *j_desc = cJSON_GetObjectItem(json,json_obj_description);
  if(j_desc != NULL)
  {
    snprintf(config->info.desc,sizeof(config->info.desc),"%s",j_desc->valuestring);
  }
  
  cJSON *j_version = cJSON_GetObjectItem(json,json_obj_version);
  if(j_version != NULL)
  {
    config->info.version = j_version->valueint;
  }
  
  cJSON *j_test = cJSON_GetObjectItem(json,json_obj_test);
  if(j_test != NULL)
  {
    cJSON *j_power = cJSON_GetObjectItem(j_test,json_obj_power);
    if(j_power != NULL)
    {
      char power_n[3];
      for(uint32_t i = 0;i < sizeof(config->power) / sizeof(power_test_st);i++)
      {
        snprintf(power_n,sizeof(power_n),"%d",i+1);
        cJSON *j_power_n = cJSON_GetObjectItem(j_power,power_n);
        if(j_power_n != NULL)
        {
          cJSON *j_power_enable = cJSON_GetObjectItem(j_power_n,json_obj_enable);
          if(j_power_enable != NULL)
          {
            config->power[i].en = j_power_enable->valueint;
          }

          cJSON *j_power_vdd = cJSON_GetObjectItem(j_power_n,json_obj_vdd);
          if(j_power_vdd != NULL)
          {
            config->power[i].vdd = j_power_vdd->valuedouble;
          }

          cJSON *j_power_v_max = cJSON_GetObjectItem(j_power_n,json_obj_v_max);
          if(j_power_v_max != NULL)
          {
            config->power[i].v_max = j_power_v_max->valuedouble;
          }

          cJSON *j_power_v_min = cJSON_GetObjectItem(j_power_n,json_obj_v_min);
          if(j_power_v_min != NULL)
          {
            config->power[i].v_min = j_power_v_min->valuedouble;
          }

          cJSON *j_power_i_max = cJSON_GetObjectItem(j_power_n,json_obj_i_max);
          if(j_power_i_max != NULL)
          {
            config->power[i].i_max = j_power_i_max->valuedouble;
          }

          cJSON *j_power_i_min = cJSON_GetObjectItem(j_power_n,json_obj_i_min);
          if(j_power_i_min != NULL)
          {
            config->power[i].i_min = j_power_i_min->valuedouble;
          }

          cJSON *j_power_color_in = cJSON_GetObjectItem(j_power_n,json_obj_color_in);
          if(j_power_color_in != NULL)
          {
            sscanf(j_power_color_in->valuestring,"%x",&config->power[i].color_in);
          }

          cJSON *j_power_color_out = cJSON_GetObjectItem(j_power_n,json_obj_color_out);
          if(j_power_color_out != NULL)
          {
            sscanf(j_power_color_out->valuestring,"%x",&config->power[i].color_out);
          }
        }
      }
      
      cJSON *j_power_mvi = cJSON_GetObjectItem(j_power,json_obj_mvi);
      if(j_power_mvi != NULL)
      {
        cJSON *j_power_vdd = cJSON_GetObjectItem(j_power_mvi,json_obj_vdd);
        if(j_power_vdd != NULL)
        {
          config->mvi.vdd = j_power_vdd->valuedouble;
        }

        cJSON *j_power_v_max = cJSON_GetObjectItem(j_power_mvi,json_obj_v_max);
        if(j_power_v_max != NULL)
        {
          config->mvi.v_max = j_power_v_max->valuedouble;
        }

        cJSON *j_power_v_min = cJSON_GetObjectItem(j_power_mvi,json_obj_v_min);
        if(j_power_v_min != NULL)
        {
          config->mvi.v_min = j_power_v_min->valuedouble;
        }

        cJSON *j_power_i_max = cJSON_GetObjectItem(j_power_mvi,json_obj_i_max);
        if(j_power_i_max != NULL)
        {
          config->mvi.i_max = j_power_i_max->valuedouble;
        }

        cJSON *j_power_i_min = cJSON_GetObjectItem(j_power_mvi,json_obj_i_min);
        if(j_power_i_min != NULL)
        {
          config->mvi.i_min = j_power_i_min->valuedouble;
        }

        cJSON *j_power_color_in = cJSON_GetObjectItem(j_power_mvi,json_obj_color_in);
        if(j_power_color_in != NULL)
        {
          sscanf(j_power_color_in->valuestring,"%x",&config->mvi.color_in);
        }

        cJSON *j_power_color_out = cJSON_GetObjectItem(j_power_mvi,json_obj_color_out);
        if(j_power_color_out != NULL)
        {
          sscanf(j_power_color_out->valuestring,"%x",&config->mvi.color_out);
        }
      }
    }
  }
  
  cJSON_Delete(json);

__exit:
  return config;
}

int test_0_start(test_module_st *module,const test_0_interface_st *intf,const char *title)
{
  int err;
  test_0_config_st *config;

  err = 0;

  config = config_parse(module->config_file);
  if(config == 0)
  {
    return -1;
  }

  test_0.config = config;
  test_0.stage_get = test_0_stage_get;
  test_0.prox_get = test_0_prox_get;
  test_0.prox_is_rdy = test_0_prox_is_rdy;
  test_0.als_get = test_0_als_get;
  test_0.als_is_rdy = test_0_als_is_rdy;
  test_0.mic_get = test_0_mic_get;
  test_0.cam_is_rdy = test_0_cam_is_rdy;
  test_0.ma_get = test_ma_get;
  test_0.mv_get = test_mv_get;
  test_0.ma_is_update = test_ma_is_update;
  test_0.spk_pn_get = test_0_spk_pn_get;

  ui_test_0_open(&test_0,title);

  if(start == 0)
  {
    start = 1;
    test_0_tid = osThreadNew(test_0_task, (void *)intf, &test_0_tid_attr);
    if(test_0_tid == NULL)
    {
      start = 0;
    }
  }

  return 0;
}