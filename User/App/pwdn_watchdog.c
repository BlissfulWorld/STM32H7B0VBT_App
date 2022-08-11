#include "pwdn_watchdog.h"
#include "cmsis_os.h"
#include "ip5306.h"
#include "timers.h"
#include "stm32h7xx.h"

static TimerHandle_t th;
static uint32_t timeout;

/**
 * @brief 超时回调函数，将执行掉电程序
 *
 * @param xTimer 未使用
 */
static void pwdn_watchdog_timer_cb(TimerHandle_t xTimer)
{
  ip5306_boost_disable();
  NVIC_SystemReset();
}

/**
 * @brief 开启掉电看门狗
 *
 * @param ms 超时时间，单位毫秒
 * @return int 错误码
 */
int pwdn_watchdog_start(uint32_t ms)
{
  osKernelLock();
  if(th)
  {
    osKernelUnlock();
    return 1;
  }

  timeout = ms;

  th = xTimerCreate("pwdn watchdog", pdMS_TO_TICKS(timeout), pdFALSE, 0, pwdn_watchdog_timer_cb);

  osKernelUnlock();

  xTimerStart(th, 0);

  return 0;
}

/**
 * @brief 重新设置超时
 *
 * @param ms 超时时间，单位毫秒
 * @return int
 */
int pwdn_watchdog_set_timeout(uint32_t ms)
{
  if(th)
  {
    timeout = ms;
    xTimerStop(th, 0);
    xTimerChangePeriod(th, pdMS_TO_TICKS(timeout), 0);
    xTimerReset(th, 0);
    return 0;
  }

  return -1;
}

/**
 * @brief 获取超时时间
 *
 * @param ms 存储超时时间变量的指针
 * @return int 错误码
 */
int pwdn_watchdog_get_timeout(uint32_t* ms)
{
  if(th)
  {
    *ms = timeout;
    return 0;
  }

  return -1;
}

/**
 * @brief 喂狗
 *
 * @return int 错误码
 */
int pwdn_watchdog_reload(void)
{
  if(th)
  {
    xTimerReset(th, 0);
    return 0;
  }

  return -1;
}
