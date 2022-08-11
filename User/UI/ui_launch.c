#include "ui.h"
#include "DIALOG.h"
#include "stdint.h"
#include "test.h"
#include "battery.h"
#include "stdio.h"
#include "ff.h"
#include "cmsis_os.h"
#include "wav.h"
#include "stdio.h"
#include "config.h"
#include "sw_info.h"
#include "backlight.h"
#include "lcd.h"
#include "user_settings.h"
#include "pwdn_watchdog.h"

#define FONT_DEF      (font_ext)
#define PROG_FONT_DEF (GUI_FONT_20_ASCII)

#define TIMEOUT_MS (10000)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_IMAGE_LOGO       (GUI_ID_USER + 0x03)

#define ID_TEXT_INFO        (GUI_ID_USER + 0x10)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320,   0,  0x0, 0},
  { BUTTON_CreateIndirect,  0, ID_BUTTON_0,            240, 320,   1,   1,   0,  0x0, 0},

  { IMAGE_CreateIndirect,   0, ID_IMAGE_LOGO,          0,  0,  240,  320,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_INFO,             0,  320 - 44, 240,  44, 0,  0x00, 0},
};

static long timer;
static resource_st* res;

static void _cbDialog(WM_MESSAGE* pMsg) {
  WM_HWIN                hItem;
  int                    NCode;
  int                    Id;

  switch(pMsg->MsgId)
  {
    case WM_INIT_DIALOG:
    {
      hItem = pMsg->hWin;
      WINDOW_SetBkColor(hItem, GUI_MAKE_COLOR(0));

      resource_st* res = resource_logo_load();
      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_LOGO);
      if(res)
        IMAGE_SetBMP(hItem, res->data, res->size);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);

      char str[128];
      snprintf(str, sizeof(str), "V%s", sw_info.version);
      TEXT_SetText(hItem, str);

      timer = WM_CreateTimer(pMsg->hWin, 0, BOOT_LOGO_TIME_MIN, 0);

      WM_SetFocus(pMsg->hWin);
      break;
    }
    case WM_TIMER:
    {
      WM_DeleteTimer(timer);
      GUI_EndDialog(pMsg->hWin, 0);
      break;
    }
    case WM_DELETE:
    {
      if(res)
        resource_unload(res);

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
