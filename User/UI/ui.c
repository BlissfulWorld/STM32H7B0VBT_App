#include "ui.h"
#include "DIALOG.h"
#include "cmsis_os.h"
#include "battery.h"
#include "key.h"
#include "ip5306.h"
#include "fatfs.h"
#include "iwdg.h"
#include "stdio.h"
#include "string.h"
#include "config.h"

static osThreadId_t ui_tid;
static const osThreadAttr_t ui_t_attr =
{
  .name = "ui",
  .stack_size = 1024 * 20,
  .priority = (osPriority_t)osPriorityNormal,
};

static osMessageQueueId_t ui_mq;

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

  {
    LISTVIEW_SetDefaultBkColor(LISTVIEW_CI_UNSEL, GUI_MAKE_COLOR(0x00000000));
    LISTVIEW_SetDefaultBkColor(LISTVIEW_CI_SEL, GUI_MAKE_COLOR(0x00FF0000));
    LISTVIEW_SetDefaultBkColor(LISTVIEW_CI_DISABLED, GUI_MAKE_COLOR(0x00000000));
    LISTVIEW_SetDefaultBkColor(LISTVIEW_CI_SELFOCUS, GUI_MAKE_COLOR(0x00FF0000));

    LISTVIEW_SetDefaultTextColor(LISTVIEW_CI_UNSEL, GUI_MAKE_COLOR(0x00FFFFFF));
    LISTVIEW_SetDefaultTextColor(LISTVIEW_CI_SEL, GUI_MAKE_COLOR(0x00FFFFFF));
    LISTVIEW_SetDefaultTextColor(LISTVIEW_CI_SELFOCUS, GUI_MAKE_COLOR(0x00FFFFFF));
    LISTVIEW_SetDefaultTextColor(LISTVIEW_CI_DISABLED, GUI_MAKE_COLOR(0x00FFFFFF));

    LISTVIEW_SetDefaultGridColor(GUI_MAKE_COLOR(0xabc88b));
  }
}

/**
 * @brief 更新电池状态的图标显示
 *
 * @param image 图片控件句柄
 * @param show_charge_lvl 充电状态是否显示电量
 */
void ui_battery_level_icon_updata(WM_HWIN image, uint8_t show_charge_lvl)
{
  static resource_st* icon = 0;
  const char* name;

  if(icon)
    resource_unload(icon);

  if((show_charge_lvl == 0) && (battery_get_status() == BAT_STA_CHARGE))
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
 * @brief 电池状态改变时的回调函数
 *
 * @param lvl 电量
 * @param status 充放电状态
 */
static void battery_changed_cb(uint8_t lvl, uint8_t status)
{
  ui_send_msg(UI_MSG_BATTERY);
}

/**
  * @brief 获取按键事件并发送按键消息
  * @retval 空
  */
static void ui_key_detect()
{
  uint8_t key, evt;
  static uint32_t ip5306_key_tick;
  static uint8_t ip5306_key_flag;

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
    if(key & 0x01)
    {
      if(ip5306_key_flag == 0)
      {
        ip5306_key_tick = HAL_GetTick();
      }
      else
      {
#if SCREENSHOT_ENABLE
        ui_screenshot();
#endif
        GUI_StoreKeyMsg(UI_KEY_EVT_DBLCLICK + UI_KEY_10, 1);
      }
      ip5306_key_flag = !ip5306_key_flag;
    }
    if(key & 0x02) GUI_StoreKeyMsg(UI_KEY_EVT_LONGPRESS + UI_KEY_10, 1);
    //if(key & 0x04) GUI_StoreKeyMsg(UI_KEY_EVT_DBLCLICK + UI_KEY_10, 1);
  }

  if(ip5306_key_flag && (HAL_GetTick() - ip5306_key_tick > 300))
  {
    ip5306_key_flag = 0;
    GUI_StoreKeyMsg(UI_KEY_EVT_ONCLICK + UI_KEY_10, 1);
  }
}

#if SCREENSHOT_ENABLE
/**
  * @brief 生成用于屏幕内容截取后文件保存路径名的序号
  * @param root 文件保存路径
  * @retval 序号
  */
static FRESULT make_file_num(const char* root, uint32_t* num)
{
  FRESULT res;
  DIR* dir;
  UINT i = 0;
  FILINFO* fno;
  uint32_t file_num = 0;

  dir = pvPortMalloc(sizeof(DIR));
  if(dir == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __list_scan_files_exit;
  }

  fno = pvPortMalloc(sizeof(FILINFO));
  if(fno == 0)
  {
    res = FR_NOT_ENOUGH_CORE;
    goto __list_scan_files_free_dir;
  }

  memset(dir, 0, sizeof(DIR));
  memset(fno, 0, sizeof(FILINFO));

  res = f_opendir(dir, root);
  if(res == FR_OK)
  {
    for(;;)
    {
      res = f_readdir(dir, fno);
      if(res != FR_OK || fno->fname[0] == 0)
        break;

      if((fno->fattrib & AM_DIR) == 0)
      {
        uint32_t num = atoi(fno->fname);
        if(num > file_num) file_num = num;
        i++;
      }
    }
  }
  else
  {
    goto __list_scan_files_free_fno;
  }

  f_closedir(dir);
__list_scan_files_free_fno:
  vPortFree(fno);
__list_scan_files_free_dir:
  vPortFree(dir);
__list_scan_files_exit:

  if(res == FR_OK)
  {
    *num = ++file_num;
  }

  return res;
}

static void ui_screenshot_write(uint8_t data, void* p)
{
  UINT bw;
  FIL* fp = p;
  f_write(fp, &data, 1, &bw);
  IWDG1->KR = IWDG_KEY_RELOAD;
}

extern void lcd_conf_set_ptr(uint16_t* ptr);

static uint8_t ui_screenshot_busy;
static const osThreadAttr_t ui_ss_t_attr =
{
  .name = "ui ss",
  .stack_size = 1024 * 2,
  .priority = (osPriority_t)osPriorityBelowNormal,
};

static void ui_screenshot_task(void* arg)
{
  FRESULT res;
  FIL* fp;
  UINT br;
  uint32_t num;
  char path[64];
  GUI_MEMDEV_Handle handle;

  handle = (GUI_MEMDEV_Handle)arg;

  if(make_file_num(DISK_USER_PATH, &num))
  {
    goto __exit;
  }

  snprintf(path, sizeof(path), "%s%u.bmp", DISK_USER_PATH, num);

  fp = (FIL*)ff_malloc(sizeof(FIL));
  if(fp == 0)
  {
    goto __delele_memdev;
  }

  res = f_open(fp, path, FA_WRITE | FA_CREATE_NEW);
  if(res != FR_OK)
  {
    goto __free_fp;
  }

  GUI_MEMDEV_SerializeBMP(handle, ui_screenshot_write, fp);

  f_close(fp);

__free_fp:
  ff_free(fp);

__delele_memdev:
  GUI_MEMDEV_Delete(handle);

__exit:
  ui_screenshot_busy = 0;
  osThreadExit();
}

int ui_screenshot(void)
{
  int err = 0;

  osKernelLock();
  if(ui_screenshot_busy)
  {
    err = 1;
    osKernelUnlock();
    goto __exit;
  }
  ui_screenshot_busy = 1;

  osKernelUnlock();

  GUI_MEMDEV_Handle handle;
  handle = GUI_MEMDEV_Create(0, 0, GUI_GetScreenSizeX(), GUI_GetScreenSizeY());
  if(handle == 0)
  {
    err = -1;
    goto __reset_busy;
  }

  lcd_conf_set_ptr(GUI_MEMDEV_GetDataPtr(handle));
  GUI_RECT rect;
  rect.x0 = 0;
  rect.x1 = GUI_GetScreenSizeX();
  rect.y0 = 0;
  rect.y1 = GUI_GetScreenSizeY();
  WM_InvalidateArea(&rect);
  GUI_Exec();
  lcd_conf_set_ptr(0);

  if(osThreadNew(ui_screenshot_task, (void*)handle, &ui_ss_t_attr) == 0)
  {
    GUI_MEMDEV_Delete(handle);
    goto __reset_busy;
  }

  goto __exit;

__reset_busy:
  ui_screenshot_busy = 0;
__exit:
  return err;
}
#endif

/**
 * @brief 发送一个消息到UI消息队列
 *
 * @param id 消息ID
 * @return int 错误码
 */
int ui_send_msg(int id)
{
  if(ui_mq)
  {
    if(osMessageQueuePut(ui_mq, &id, 0, 0) == osOK)
      return 0;
    else
      return -1;
  }

  return -2;
}

/**
 * @brief UI初始化
 *
 * @return int 错误码
 */
int ui_init(void)
{
  GUI_Init();

  ui_mq = osMessageQueueNew(10, sizeof(int), 0);

  //加载字库
  font_load();

  //控件皮肤初始化
  skin_init();

  GUI_SetBkColor(GUI_MAKE_COLOR(0x000000));

  GUI_SetColor(GUI_MAKE_COLOR(0xFFFFFF));

  GUI_UC_SetEncodeUTF8();

  GUI_SetTextMode(GUI_TM_TRANS);

  ml_set(0);

  if(font_ext != GUI_DEFAULT_FONT)
  {
    uint32_t len = US_LEN(language) + sizeof(DISK_SYS_PATH) + 9;
    char* path = pvPortMalloc(len);
    if(path)
    {
      if(US_GET(language, snprintf(path, len, "%slanguage/", DISK_SYS_PATH) + path) == 0)
        ml_set(path);
      else
        ml_set(0);
      vPortFree(path);
    }
  }

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
  WM_MESSAGE msg = { 0 };

  tick = osKernelGetTickCount();
  while(1)
  {
    IWDG1->KR = IWDG_KEY_RELOAD;

    while(osMessageQueueGet(ui_mq, &msg.MsgId, 0, 0) == osOK)
    {
      WM_BroadcastMessage(&msg);
      IWDG1->KR = IWDG_KEY_RELOAD;
    }

    GUI_Exec();

    tick += 40;
    osDelayUntil(tick);

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
  {
    ui_tid = osThreadNew(ui_task, NULL, &ui_t_attr);
  }
  osKernelUnlock();
}
