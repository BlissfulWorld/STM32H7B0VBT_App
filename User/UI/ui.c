#include "ui.h"
#include "DIALOG.h"
#include "cmsis_os.h"
#include "battery.h"
#include "key.h"
#include "ip5306.h"
#include "ff.h"
#include "iwdg.h"
#include "stdio.h"
#include "config.h"

static uint32_t battery_value;
static WM_MESSAGE msg = {
  .MsgId = UI_MSG_BATTERY
};

static osThreadId_t ui_tid;
static const osThreadAttr_t ui_t_attr =
{
  .name = "ui",
  .stack_size = 1024 * 20,
  .priority = (osPriority_t)osPriorityNormal,
};

/**
  * @brief 初始化界面控件的皮肤
  * @retval 空
  */
void skin_init(void)
{
  {
    PROGBAR_SKINFLEX_PROPS prog;
    PROGBAR_GetSkinFlexProps(&prog, 0);

    prog.aColorUpperL[0] = GUI_MAKE_COLOR(0x7bbfea);
    prog.aColorUpperL[1] = GUI_MAKE_COLOR(0x7bbfea);
    prog.aColorLowerL[0] = GUI_MAKE_COLOR(0x7bbfea);
    prog.aColorLowerL[1] = GUI_MAKE_COLOR(0x7bbfea);

    prog.aColorUpperR[0] = GUI_MAKE_COLOR(0x999d9c);
    prog.aColorUpperR[1] = GUI_MAKE_COLOR(0x999d9c);
    prog.aColorLowerR[0] = GUI_MAKE_COLOR(0x999d9c);
    prog.aColorLowerR[1] = GUI_MAKE_COLOR(0x999d9c);

    prog.ColorFrame = GUI_MAKE_COLOR(BACKGROUND_COLOR);

    PROGBAR_SetSkinFlexProps(&prog, 0);
  }

  {
    BUTTON_SKINFLEX_PROPS prog;
    BUTTON_GetSkinFlexProps(&prog, BUTTON_SKINFLEX_PI_FOCUSED);

    prog.aColorLower[0] = GUI_MAKE_COLOR(0xabc88b);
    prog.aColorLower[1] = GUI_MAKE_COLOR(0xabc88b);
    prog.aColorUpper[0] = GUI_MAKE_COLOR(0xabc88b);
    prog.aColorUpper[1] = GUI_MAKE_COLOR(0xabc88b);

    prog.aColorFrame[0] = GUI_MAKE_COLOR(0xabc88b);
    prog.aColorFrame[1] = GUI_MAKE_COLOR(0xabc88b);
    prog.aColorFrame[2] = GUI_MAKE_COLOR(0xabc88b);

    prog.Radius = 0;

    BUTTON_SetSkinFlexProps(&prog, BUTTON_SKINFLEX_PI_PRESSED);

    prog.aColorFrame[0] = GUI_MAKE_COLOR(0xabc88b);
    prog.aColorFrame[1] = GUI_MAKE_COLOR(0xabc88b);
    BUTTON_SetSkinFlexProps(&prog, BUTTON_SKINFLEX_PI_FOCUSED);
    BUTTON_SetSkinFlexProps(&prog, BUTTON_SKINFLEX_PI_DISABLED);
    BUTTON_SetSkinFlexProps(&prog, BUTTON_SKINFLEX_PI_ENABLED);
  }

  {
    HEADER_SKINFLEX_PROPS prog;

    HEADER_GetSkinFlexProps(&prog, 0);

    prog.aColorLower[0] = GUI_MAKE_COLOR(0xabc88b);
    prog.aColorLower[1] = GUI_MAKE_COLOR(0xabc88b);
    prog.aColorUpper[0] = GUI_MAKE_COLOR(0xabc88b);
    prog.aColorUpper[1] = GUI_MAKE_COLOR(0xabc88b);

    prog.aColorFrame[0] = GUI_MAKE_COLOR(0xabc88b);
    prog.aColorFrame[1] = GUI_MAKE_COLOR(0xabc88b);

    prog.ColorArrow = GUI_MAKE_COLOR(0xabc88b);

    HEADER_SetSkinFlexProps(&prog, 0);
  }
}

/**
 * @brief 更新电池状态的图标显示
 *
 * @param image 图片控件句柄
 */
void ui_battery_level_icon_updata(WM_HWIN image, uint8_t show_charge_lvl)
{
  static resource_st* icon = 0;
  const char* name;

  if(icon)
    resource_unload(icon);

  if(!show_charge_lvl && battery_get_status() == BAT_STA_CHARGE)
  {
    name = "battery_charging";
  }
  else
  {
    uint8_t lvl = battery_get_level();

    if(lvl < 1)
      name = "battery_l0";//0
    else if(lvl < 25)
      name = "battery_l1";//1~24
    else if(lvl < 50)
      name = "battery_l2";//25~49
    else if(lvl < 75)
      name = "battery_l3";//50~74
    else if(lvl <= 100)
      name = "battery_l4";//75~100
  }

  icon = resource_icon_load(name);

  if(icon)
    IMAGE_SetDTA(image, icon->data, icon->size);
}

/**
  * @brief 显示logo
  * @retval 空
  */
void ui_show_logo(void)
{
  FRESULT res;
  FIL fp;
  UINT br;
  uint8_t* src_p;

  res = f_open(&fp, "0:/logo/logo.bmp", FA_READ);
  if(res != FR_OK)
  {
    goto __logo_show_exit;
  }

  res = 1;

  src_p = pvPortMalloc(f_size(&fp) + 2);
  if(src_p == 0)
  {
    goto __logo_show_exit_close;
  }

  res = f_read(&fp, src_p + 2, f_size(&fp), &br);
  if((res != FR_OK) || (br != f_size(&fp)))
  {
    goto __logo_show_exit_free;
  }

  GUI_BMP_Draw(src_p + 2, 0, 0);

__logo_show_exit_free:
  vPortFree(src_p);

__logo_show_exit_close:
  f_close(&fp);

__logo_show_exit:

  return;
}

/**
 * @brief 电池状态改变时的回调函数
 *
 * @param lvl 电量
 * @param status 充放电状态
 */
static void battery_changed_cb(uint8_t lvl, uint8_t status)
{
  battery_value = lvl | (status << 8);
}

/**
  * @brief 获取按键事件并发送按键消息
  * @retval 空
  */
static void ui_key_detect()
{
  uint8_t key, evt;

  while(key_get(&key, &evt))
  {
    switch(evt)
    {
      case KEY_RELEASE:   GUI_StoreKeyMsg(UI_KEY_EVT_RELEASE + key, 1);break;
      case KEY_PRESS:     GUI_StoreKeyMsg(UI_KEY_EVT_PRESS + key, 1);break;
      case KEY_LONG_PRESS:GUI_StoreKeyMsg(UI_KEY_EVT_LONGPRESS + key, 1);break;
      case KEY_ONCLICK:   GUI_StoreKeyMsg(UI_KEY_EVT_ONCLICK + key, 1);break;
        //case KEY_DBLCLICK:  GUI_StoreKeyMsg(UI_KEY_EVT_DBLCLICK  + key, 1);break;
      case KEY_REPET:     GUI_StoreKeyMsg(UI_KEY_EVT_REPET + key, 1);break;
      default:break;
    }
  }

  key = ip5306_key_get();
  if(key)
  {
    if(key & 0x01) GUI_StoreKeyMsg(UI_KEY_EVT_ONCLICK + UI_KEY_10, 1);
    if(key & 0x02) GUI_StoreKeyMsg(UI_KEY_EVT_LONGPRESS + UI_KEY_10, 1);
    if(key & 0x04) GUI_StoreKeyMsg(UI_KEY_EVT_DBLCLICK + UI_KEY_10, 1);
  }
}

/**
 * @brief UI初始化
 *
 * @return int 错误码
 */
int ui_init(void)
{
  GUI_Init();

  //加载字库
  font_load();

  //控件皮肤初始化
  skin_init();

  GUI_SetBkColor(GUI_MAKE_COLOR(0x000000));

  GUI_SetColor(GUI_MAKE_COLOR(0xFFFFFF));

  GUI_UC_SetEncodeUTF8();

  GUI_SetTextMode(GUI_TM_TRANS);

  GUI_SetFont(font_ext);

  //设置电池状态改变后的回调函数
  battery_set_changed_cb(battery_changed_cb);

  return 0;
}

/**
 * @brief UI处理
 *
 */
void ui_exec(void)
{
  uint32_t tick;

  tick = osKernelGetTickCount();
  while(1)
  {
    //HAL_IWDG_Refresh(&hiwdg1);
    IWDG1->KR = IWDG_KEY_RELOAD;
    GUI_Exec();

    tick += 40;
    osDelayUntil(tick);

    //广播电池信息
    if(msg.Data.v != battery_value)
    {
      msg.Data.v = battery_value;
      WM_BroadcastMessage(&msg);
    }

    ui_key_detect();
  }
}

/**
 * @brief UI任务
 *
 * @param arg 未使用
 */
static void ui_task(void* arg)
{
  for(;;)
  {
    ui_exec();
  }
}

/**
 * @brief 开始UI任务
 *
 */
void ui_start(void)
{
  osKernelLock();
  if(ui_tid == 0)
    ui_tid = osThreadNew(ui_task, NULL, &ui_t_attr);
  osKernelUnlock();
}
