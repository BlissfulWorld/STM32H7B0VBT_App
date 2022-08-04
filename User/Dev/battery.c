#include "battery.h"
#include "adc.h"
#include "ip5306.h"
#include "config.h"
#include "cmsis_os.h"

static uint8_t level, level_pre;
static battery_status_et status;
static void (*battery_changed_cb)(uint8_t, battery_status_et);

static void battery_level_proc(void);
static void charger_detect(void);

static osThreadId_t battery_tid;
static const osThreadAttr_t battery_tid_attr = {
  .name = "battery",
  .stack_size = 1024 * 1,
  .priority = (osPriority_t)osPriorityNormal,
};

/**
 * @brief 电池任务
 *
 * @param p 未使用
 */
static void battery_task(void* p)
{
  uint32_t tick, i = 0;

  tick = osKernelGetTickCount();
  while(1)
  {
    charger_detect();

    if(i == 10)
    {
      i++;
      battery_level_proc();
    }

    tick += 100;
    osDelayUntil(tick);
    i++;
  }
}

/**
  * @brief 获取电池电量
  * @retval 电量百分比
  */
uint8_t battery_get_level(void)
{
  return level;
}

/**
  * @brief 获取充放电状态
  * @retval 状态枚举
  */
battery_status_et battery_get_status(void)
{
  return status;
}

/**
  * @brief 电池初始化
  */
void battery_init(void)
{
  uint16_t adc;
  ip5306_chg_sta_et ip5306_chg_sta;

  level = ip5306_lvl_get();
  level_pre = level;

  status = 0;
  if(ip5306_charge_sta_get(&ip5306_chg_sta) == 0)
  {
    status = (battery_status_et)ip5306_chg_sta;
  }

  if(battery_tid == NULL)
    battery_tid = osThreadNew(battery_task, NULL, &battery_tid_attr);
}

/**
  * @brief 设置电池事件回调函数
  */
void battery_set_changed_cb(void (*cb)(uint8_t lvl, battery_status_et status))
{
  battery_changed_cb = cb;
}

static void charger_detect(void)
{
  ip5306_chg_sta_et ip5306_chg_sta;

  //获取充放电状态
  if(ip5306_charge_sta_get(&ip5306_chg_sta)) return;

  //充放电状态改变则执行回调
  if(status != (battery_status_et)ip5306_chg_sta)
  {
    status = (battery_status_et)ip5306_chg_sta;
    if(battery_changed_cb) battery_changed_cb(level, status);
  }
}

static void battery_level_proc(void)
{
  static uint32_t count;
  uint8_t _level;

  //获取电量
  _level = ip5306_lvl_get();
  level = _level;

  //电量发生变化则执行回调
  if(level_pre != level)
  {
    level_pre = level;
    if(battery_changed_cb) battery_changed_cb(level, status);
  }
}