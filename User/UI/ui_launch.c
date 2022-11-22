#include "ui.h"
#include "DIALOG.h"
#include "stdio.h"
#include "disk.h"
#include "cmsis_os.h"
#include "sw_info.h"
#include "pwdn_watchdog.h"
#include "resource.h"

#define FONT_DEF      (font_ext)

#define TIMEOUT_MS (10000)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_LOGO        (GUI_ID_USER + 0x03)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320,   0,  0x0, 0},
  { BUTTON_CreateIndirect,  0, ID_BUTTON_0,            240, 320,   1,   1,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_LOGO,             0,   0, 240, 320,   0,  0x0, 0},
};

static long timer;
static const char* str = 0;

static void wm_cb(WM_MESSAGE* pMsg)
{
  switch(pMsg->MsgId)
  {
    case WM_POST_PAINT:
    {
      resource_st* rsc = resource_logo_load();
      if(rsc)
      {
        GUI_BMP_Draw(rsc->data, 0, 0);
        resource_unload(rsc);
      }
      if(str)
      {
        GUI_UC_SetEncodeUTF8();
        GUI_SetColor(GUI_MAKE_COLOR(0xFFFFFF));
        GUI_SetFont(FONT_DEF);
        GUI_SetTextMode(GUI_TM_TRANS);
        GUI_DispStringInRect(str, &(GUI_RECT){.x0 = 0, .y0 = 320 - 28, .x1 = 240, .y1 = 320}, GUI_TA_HCENTER | GUI_TA_TOP);
      }
      else
      {
#if BOOT_SHOW_VERSION
        GUI_UC_SetEncodeUTF8();
        GUI_SetColor(GUI_MAKE_COLOR(0xFFFFFF));
        GUI_SetFont(FONT_DEF);
        GUI_SetTextMode(GUI_TM_TRANS);
        char ver_str[128];
        snprintf(ver_str, sizeof(ver_str), "V%s", sw_info.version);
        GUI_DispStringInRect(ver_str, &(GUI_RECT){.x0 = 0, .y0 = 320 - 28, .x1 = 240, .y1 = 320}, GUI_TA_HCENTER | GUI_TA_TOP);
#endif
      }
      break;
    }
    default:
    {
      WM_DefaultProc(pMsg);
      break;
    }
  }
}

static void _cbDialog(WM_MESSAGE* pMsg)
{
  WM_HWIN hItem;

  switch(pMsg->MsgId)
  {
    case WM_INIT_DIALOG:
    {
      hItem = pMsg->hWin;
      WINDOW_SetBkColor(hItem, GUI_MAKE_COLOR(0));

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_LOGO);
      WM_SetCallback(hItem, wm_cb);

      timer = WM_CreateTimer(pMsg->hWin, 0, BOOT_LOGO_TIME_MIN, 0);

      WM_SetFocus(pMsg->hWin);
      break;
    }
    case WM_TIMER:
    {
      //检查是否需要更新资源
      char path[64];
      sprintf(path, "%sresource", DISK_USER_PATH);
      if(disk_dir_is_exist(path) == 1)
      {
        //提示正在更新资源
        str = ui_str.ui_lanuch.update_resource;
        WM_InvalidateRect(pMsg->hWin, &(GUI_RECT){.x0 = 0, .y0 = 320 - 28, .x1 = 240, .y1 = 320});
        GUI_Exec();
        resource_update();
        osKernelLock();
        //复位
        while(1);
      }
      else
      {
        WM_DeleteTimer(timer);
        GUI_EndDialog(pMsg->hWin, 0);
      }
      break;
    }
    case WM_DELETE:
    {
      uint32_t standby = 5 * 60 * 1000;
      US_GET(standbytime, &standby);
      pwdn_watchdog_start(standby);

      ui_home_open();
      break;
    }
    default:
    {
      WM_DefaultProc(pMsg);
      break;
    }
  }
}

void ui_launch_open(void)
{
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
