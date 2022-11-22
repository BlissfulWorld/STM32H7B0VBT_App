#include "test.h"
#include "string.h"
#include "cmsis_os.h"
#include "pca9535.h"
#include "hw_i2c.h"
#include "driver_ina219_basic.h"

test_module_st* module;

extern const test_st test_ip7;
extern const test_st test_ip7plus;
extern const test_st test_ip8plus;
extern const test_st test_ipx;
extern const test_st test_ipxs;
extern const test_st test_ipxsmax;
extern const test_st test_ipxr;
extern const test_st test_ip11;
extern const test_st test_ip11pro;
extern const test_st test_ip11promax;
extern const test_st test_ip12mini;
extern const test_st test_ip12;
extern const test_st test_ip12pro;
extern const test_st test_ip12promax;
extern const test_st test_ipx_j6400;

static const test_st* test_list[] =
{
  &test_ip7,
  &test_ip7plus,
  &test_ip8plus,
  &test_ipx,
  &test_ipxs,
  &test_ipxsmax,
  &test_ipxr,
  &test_ip11,
  &test_ip11pro,
  &test_ip11promax,
  &test_ip12mini,
  &test_ip12,
  &test_ip12pro,
  &test_ip12promax,

  &test_ipx_j6400,


  0,//结尾
};

//引脚定义
const hw_pin_st test_pin[51] =
{
  {0,0},
  {GPIOA,GPIO_PIN_6}, //1
  {GPIOA,GPIO_PIN_7}, //2
  {GPIOC,GPIO_PIN_3}, //3
  {GPIOB,GPIO_PIN_0}, //4
  {GPIOC,GPIO_PIN_4}, //5
  {GPIOC,GPIO_PIN_5}, //6
  {GPIOC,GPIO_PIN_6}, //7
  {GPIOC,GPIO_PIN_7}, //8
  {GPIOC,GPIO_PIN_8}, //9
  {GPIOA,GPIO_PIN_2}, //10
  {GPIOE,GPIO_PIN_15},//11
  {GPIOE,GPIO_PIN_3}, //12
  {GPIOE,GPIO_PIN_7}, //13
  {GPIOE,GPIO_PIN_9}, //14
  {GPIOC,GPIO_PIN_1}, //15
  {GPIOD,GPIO_PIN_9}, //16
  {GPIOE,GPIO_PIN_0}, //17
  {GPIOD,GPIO_PIN_11},//18
  {GPIOD,GPIO_PIN_12},//19
  {GPIOD,GPIO_PIN_13},//20
  {GPIOA,GPIO_PIN_3}, //21
  {GPIOA,GPIO_PIN_9}, //22
  {GPIOA,GPIO_PIN_10},//23
  {GPIOB,GPIO_PIN_1}, //24
  {0,0},              //25
  {0,0},              //26
  {GPIOA,GPIO_PIN_8}, //27
  {0,0},//{GPIOE,GPIO_PIN_6}, //28
  {GPIOE,GPIO_PIN_5}, //29
  {0,0},//{GPIOE,GPIO_PIN_12},//30
  {GPIOE,GPIO_PIN_4}, //31
  {GPIOD,GPIO_PIN_7}, //32
  {GPIOB,GPIO_PIN_4}, //33
  {GPIOA,GPIO_PIN_4}, //34
  {GPIOA,GPIO_PIN_5}, //35
  {GPIOB,GPIO_PIN_8}, //36
  {GPIOB,GPIO_PIN_7}, //37
  {GPIOE,GPIO_PIN_1}, //38
  {GPIOD,GPIO_PIN_10},//39
  {GPIOD,GPIO_PIN_8}, //40
  {GPIOD,GPIO_PIN_6}, //41
  {GPIOD,GPIO_PIN_5}, //42
  {GPIOD,GPIO_PIN_4}, //43
  {GPIOD,GPIO_PIN_3}, //44
  {GPIOD,GPIO_PIN_2}, //45
  {GPIOD,GPIO_PIN_1}, //46
  {GPIOD,GPIO_PIN_0}, //47
  {GPIOC,GPIO_PIN_13},//48
  {GPIOB,GPIO_PIN_9}, //49
  {GPIOB,GPIO_PIN_5}, //50
};

void test_init(void)
{
  GPIO_InitTypeDef gpio_init;

  pca9535_set_output(0, 0xFFFF);
  pca9535_config_port(0, 0x0000);

  pca9535_set_output(1, 0x0000);
  pca9535_config_port(1, 0x0000);

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
}

#define MA_MUN (13)

static const osThreadAttr_t test_current_tid_attr = {
  .name = "test current",
  .stack_size = 1024 * 2,
  .priority = (osPriority_t)osPriorityNormal,
};
static ina219_handle_t gs_handle[2];
static float test_ma[MA_MUN], test_mv[MA_MUN];
static int test_ma_flag[MA_MUN];
static uint8_t test_current_task_running;
static uint8_t test_current_task_stop;

static void test_current(void* arg)
{
  uint32_t i = 0;
  float mv, ma, mw;
  uint16_t calibration;

  memset(gs_handle, 0, sizeof(gs_handle));
  memset(test_ma, 0, sizeof(test_ma));
  memset(test_ma_flag, 0, sizeof(test_ma_flag));

  while(ina219_basic_init(&gs_handle[0], INA219_ADDRESS_0, 0.1))
  {
    osDelay(100);
  }

  while(ina219_basic_init(&gs_handle[1], INA219_ADDRESS_4, 0.1))
  {
    osDelay(100);
  }

  //pca9535_set_output(0,0xFFEF);
  pca9535_config_port(0, 0x0000);
  pca9535_set_output(0, ~(1U << i));
  osDelay(10);

  test_current_task_stop = 0;
  while(test_current_task_stop == 0)
  {
    if(ina219_basic_read(&gs_handle[0], &mv, &ma, &mw))
    {
      //osDelay(5);
      hw_i2c_fix(hw_i2c2, 10);
    }
    else if(((uint32_t)(test_ma[i] * 100) != (uint32_t)(ma * 100)) || ((uint32_t)(test_mv[i] * 100) != (uint32_t)(mv * 100)))
    {
      if(ma < 0) ma = 0;
      test_mv[i] = mv;
      test_ma[i] = ma;
      test_ma_flag[i] = 1;
    }

    if(i == 0)
    {
      if(ina219_basic_read(&gs_handle[1], &mv, &ma, &mw))
      {
        //osDelay(5);
        //hw_i2c_fix(hw_i2c2,10);
      }
      else if(((uint32_t)(test_ma[MA_MUN - 1] * 100) != (uint32_t)(ma * 100)) || ((uint32_t)(test_mv[MA_MUN - 1] * 100) != (uint32_t)(mv * 100)))
      {
        if(ma < 0) ma = 0;
        test_mv[MA_MUN - 1] = mv;
        test_ma[MA_MUN - 1] = ma;
        test_ma_flag[MA_MUN - 1] = 1;
      }
    }

    i++;
    if(i >= MA_MUN - 1)
    {
      i = 0;
    }
    //ina219_basic_deinit(&gs_handle[0]);
    pca9535_set_output(0, 0xFFFF);
    osDelay(4);
    pca9535_set_output(0, ~(1U << i));
    //osDelay(50);
    //ina219_basic_init(&gs_handle[0],INA219_ADDRESS_0,0.1);
    osDelay(96);
  }
  ina219_basic_deinit(&gs_handle[0]);
  ina219_basic_deinit(&gs_handle[1]);
  for(uint32_t i = 0;i < MA_MUN;i++)
    test_ma_flag[i] = 0;
  test_current_task_running = 0;
  osThreadExit();
}

void test_current_start(void)
{
  osKernelLock();
  if(!test_current_task_running)
    if(osThreadNew(test_current, NULL, &test_current_tid_attr))
      test_current_task_running = 1;
  osKernelUnlock();
}

void test_current_stop(void)
{
  test_current_task_stop = 1;
  while(test_current_task_running) osDelay(100);
}

double test_ma_get(uint8_t idx)
{
  if(idx > MA_MUN - 1)
    return 0;
  return test_ma[idx];
}

double test_mv_get(uint8_t idx)
{
  if(idx > MA_MUN - 1)
    return 0;
  return test_mv[idx];
}

uint8_t test_ma_is_update(uint8_t idx)
{
  uint8_t update = 0;

  if(idx < MA_MUN)
  {
    update = test_ma_flag[idx];
    test_ma_flag[idx] = 0;
  }

  return update;
}

int test_start(test_module_st* module)
{
  for(uint32_t i = 0;test_list[i]->id;i++)
  {
    if(module->id.code == test_list[i]->id->code)
    {
      test_current_start();
      test_list[i]->start(module, test_list[i]->title);
      return 0;
    }
  }

  return -1;
}

int test_stop(test_module_st* module)
{
  for(uint32_t i = 0;test_list[i];i++)
  {
    if(module->id.code == test_list[i]->id->code)
    {
      test_list[i]->stop();
      while(test_list[i]->is_running()) osDelay(50);
      test_current_stop();
      return 0;
    }
  }

  return -1;
}
