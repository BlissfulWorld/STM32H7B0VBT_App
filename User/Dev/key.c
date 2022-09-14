#include "key.h"
#include "adc.h"
#include "main.h"
#include "tim.h"

#define KEY_DBLCLICK_ENANLE (0)

#define LONG_PRESS_TIME (1000)
#define REPET_TIME  (400)
#define DBLCLICK_TIME (200)

#define get_systick() HAL_GetTick()

static uint8_t key_sta[KEY_NUM], key_cnt[KEY_NUM];
static uint8_t key_event_flag[KEY_NUM];
static uint32_t key_time[KEY_NUM];
static key_event_cb_t key_event_cb = 0;
static void key_on_proc(uint8_t num);
static void key_off_proc(uint8_t num);

typedef struct key_queue_s
{
  struct key_state_s
  {
    key_num_et num;
    key_event_et evt;
  }state[3];

  uint8_t in, out;
}key_queue_st;

key_queue_st key_queue;

/**
  * @brief 按键事件入队，队列未满时才能入队
  * @param num 按键号
  * @param evt 按键事件
  */
static void key_put(key_num_et num, key_event_et evt)
{
  uint8_t in = key_queue.in;
  in++;
  if(in >= sizeof(key_queue.state) / sizeof(struct key_state_s))
  {
    in = 0;
  }

  if(in != key_queue.out)
  {
    key_queue.state[in].num = num;
    key_queue.state[in].evt = evt;

    key_queue.in = in;
  }
}

/**
  * @brief 从按键事件队列中获取事件
  * @param num 指向用于存储按键号的内存指针
  * @param evt 指向用于存储按键事件的内存指针
  * @retval 0:无按键事件 1:有按键事件
  */
uint8_t key_get(key_num_et* num, key_event_et* evt)
{
  uint8_t out = key_queue.out;

  if(out != key_queue.in)
  {
    out++;
    if(out >= sizeof(key_queue.state) / sizeof(struct key_state_s))
    {
      out = 0;
    }
    *num = key_queue.state[out].num;
    *evt = key_queue.state[out].evt;
    key_queue.out = out;
    return 1;
  }
  else
    return 0;
}
//UP SW1 21000 25000
//DOWN SW3 29000 34800   
//RIGHT SW5 34500 40500
//LEFT SW4 40100 48000

uint16_t key_adc_value;
void adc2_conv_cplt_cb(void)
{
  key_adc_value = HAL_ADC_GetValue(&hadc2);

  if((key_adc_value > 23000) && (key_adc_value < 27000))
  {
    key_on_proc(KEY_S1);
  }
  else
  {
    key_off_proc(KEY_S1);
  }

  if((key_adc_value > 32800) && (key_adc_value < 36800))
  {
    key_on_proc(KEY_S3);
  }
  else
  {
    key_off_proc(KEY_S3);
  }

  if((key_adc_value > 46000) && (key_adc_value < 50000))
  {
    key_on_proc(KEY_S4);
  }
  else
  {
    key_off_proc(KEY_S4);
  }

  if((key_adc_value > 38500) && (key_adc_value < 42500))
  {
    key_on_proc(KEY_S5);
  }
  else
  {
    key_off_proc(KEY_S5);
  }
}

void key_init(void)
{
  key_event_cb = 0;
  key_queue.in = 0;
  key_queue.out = 0;
  MX_ADC2_Init();
  HAL_ADC_Start_IT(&hadc2);
  HAL_TIM_Base_Start(&htim15);
}

void key_deinit(void)
{
  HAL_TIM_Base_Stop(&htim15);
  HAL_ADC_Stop_IT(&hadc2);
}

#if KEY_DBLCLICK_ENANLE
/**
  * @brief 按下处理
  * @param 按键号
  */
static void key_on_proc(uint8_t num)
{
  uint32_t tick;

  tick = get_systick();

  switch(key_sta[num])
  {
    case 0://按下检测
      if(++key_cnt[num] > 3)
      {
        key_time[num] = tick;
        key_put(num, KEY_PRESS);
        key_sta[num] = 1;
      }
      break;
    case 1://长按检测
    case 5:
      if(tick - key_time[num] > LONG_PRESS_TIME)
      {
        key_sta[num] = 2;
        key_put(num, KEY_LONG_PRESS);
        key_time[num] = tick;
      }
      break;
    case 2://延迟
      if(tick - key_time[num] > REPET_TIME)
      {
        key_time[num] = tick;
        key_sta[num] = 3;
      }
      break;
    case 3://一直按住
      if(tick - key_time[num] > REPET_TIME)
      {
        key_time[num] = tick;
        key_put(num, KEY_REPET);
      }
      break;
    case 4://双击检测
      if(++key_cnt[num] > 3)
      {
        key_time[num] = tick;
        key_put(num, KEY_PRESS);
        key_sta[num] = 5;
      }
      break;
  }

  if(key_sta[num] != 0)
  {
    key_cnt[num] = 0;
  }
}

/**
  * @brief 松开处理
  * @param 按键号
  */
static void key_off_proc(uint8_t num)
{
  if(key_sta[num])
  {
    if(key_sta[num] == 4)//单击检测，（第一次单击后未在DBLCLICK_TIME时间内再单击）
    {
      if(get_systick() - key_time[num] > DBLCLICK_TIME)
      {
        key_put(num, KEY_ONCLICK);
        key_sta[num] = 0;
      }
    }
    else if((++key_cnt[num] > 3))
    {
      key_put(num, KEY_RELEASE);
      if(key_sta[num] == 1)//第一次单击
      {
        key_sta[num] = 4;//开始双击检测
        key_time[num] = get_systick();//记录第一次单击时间
      }
      else
      {
        if(key_sta[num] == 5)//双击
        {
          key_put(num, KEY_DBLCLICK);
        }

        key_sta[num] = 0;
      }
    }
  }
  else
    key_cnt[num] = 0;
}

#else
/**
  * @brief 按下处理
  * @param num 按键号
  */
static void key_on_proc(uint8_t num)
{
  uint32_t tick;

  tick = get_systick();

  switch(key_sta[num])
  {
    case 0://按下检测
      if(++key_cnt[num] > 3)
      {
        key_time[num] = tick;
        key_put(num, KEY_PRESS);
        key_sta[num] = 1;
      }
      break;
    case 1://长按检测
      if(tick - key_time[num] > LONG_PRESS_TIME)
      {
        key_sta[num] = 2;
        key_put(num, KEY_LONG_PRESS);
        key_time[num] = tick;
      }
      break;
    case 2://延迟
      if(tick - key_time[num] > REPET_TIME)
      {
        key_time[num] = tick;
        key_sta[num] = 3;
      }
      break;
    case 3://一直按住
      if(tick - key_time[num] > REPET_TIME)
      {
        key_time[num] = tick;
        key_put(num, KEY_REPET);
      }
      break;
  }

  if(key_sta[num] != 0)
  {
    key_cnt[num] = 0;
  }
}

/**
  * @brief 松开处理
  * @param 按键号
  */
static void key_off_proc(uint8_t num)
{
  if(key_sta[num])
  {
    key_put(num, KEY_RELEASE);
    if(key_sta[num] == 1)//单击
    {
      key_put(num, KEY_ONCLICK);
    }
    key_sta[num] = 0;
  }
  else
    key_cnt[num] = 0;
}
#endif
