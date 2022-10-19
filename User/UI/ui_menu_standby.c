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
#include "user_settings.h"
#include "pwdn_watchdog.h"

#define FONT_DEF      (font_ext)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)
#define ID_TEXT_BOTTOM      (GUI_ID_USER + 0x04)

#define ID_TEXT_MAX_MIN     (GUI_ID_USER + 0x10)
#define ID_TEXT_STANDBY     (GUI_ID_USER + 0x11)

#define ID_TEXT_BACK        (GUI_ID_USER + 0x30)
#define ID_TEXT_OK          (GUI_ID_USER + 0x31)
#define ID_TEXT_SAVE        (GUI_ID_USER + 0x32)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320,   0,  0x0, 0},
  { BUTTON_CreateIndirect,  0, ID_BUTTON_0,            240, 320,   1,   1,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_TITLE,            0,   0, 240,  44,   0,  0x0, 0},
  { IMAGE_CreateIndirect,   0, ID_IMAGE_LEVEL,         180,  24,  20,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BOTTOM,           0, 320 - 30, 240,  30,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_MAX_MIN,         90,  99,  60, 130,   0,  0x00, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_STANDBY,         90, 144,  60,  40,   0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BACK,            20, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_OK,              90, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_SAVE,           160, 320 - 28,  60,  28,   0,  0x0, 0},
};

static WM_HWIN back;

static uint32_t standby_get_minute(void)
{
  uint32_t standby = 60 * 1000;
  pwdn_watchdog_get_timeout(&standby);
  standby /= (60 * 1000);

  return standby;
}

static void standby_set_minute(uint32_t standby)
{
  pwdn_watchdog_set_timeout(standby * 60 * 1000);
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

      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_LEVEL);
      ui_battery_level_icon_updata(hItem, 0);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_TITLE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_BOTTOM);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_m_standby.title);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BOTTOM);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAX_MIN);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "60\n--\n\n\n\n\n--\n1");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_STANDBY);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF8C00));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      char str[128];
      snprintf(str, sizeof(str), "%u", standby_get_minute());
      TEXT_SetText(hItem, str);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
      TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_m_standby.btn_back);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SAVE);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_m_standby.btn_save);

      uint32_t standby_saved;
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SAVE);
      if((US_GET(standbytime, &standby_saved) == 0) && (standby_get_minute() * 60 * 1000 == standby_saved))
      {
        WM_HideWindow(hItem);
      }
      else
      {
        WM_ShowWindow(hItem);
      }

      WM_SetFocus(pMsg->hWin);
      break;
    }
    case WM_KEY:
    {
      pwdn_watchdog_reload();

      uint32_t standby = standby_get_minute();

      switch(((WM_KEY_INFO*)(pMsg->Data.p))->Key)
      {
        case UI_KEY(UP, REPET):
          standby += 4;
        case UI_KEY(UP, ONCLICK):
        {
          standby++;
          if(standby > 60) standby = 60;
          standby_set_minute(standby);
          standby = standby_get_minute();
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_STANDBY);
          char str[128];
          snprintf(str, sizeof(str), "%u", standby);
          TEXT_SetText(hItem, str);
          break;
        }
        case UI_KEY(DOWN, REPET):
          if(standby > 4) standby -= 4;
          else standby = 1;
        case UI_KEY(DOWN, ONCLICK):
        {
          if(standby > 1) standby--;
          standby_set_minute(standby);
          standby = standby_get_minute();
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_STANDBY);
          char str[128];
          snprintf(str, sizeof(str), "%u", standby);
          TEXT_SetText(hItem, str);
          break;
        }
        case UI_KEY(RIGHT, ONCLICK):
        {
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SAVE);
          if(WM_IsVisible(hItem))
          {
            uint32_t tmp = standby * 60 * 1000;
            US_SET(standbytime, &tmp);
          }
          break;
        }
        case UI_KEY(LEFT, ONCLICK):
        {
          GUI_EndDialog(pMsg->hWin, 0);
          break;
        }
      }

      //如果与已存储的值不同则提示需要保存
      uint32_t standby_saved;
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SAVE);
      if((US_GET(standbytime, &standby_saved) == 0) && (standby * 60 * 1000 == standby_saved))
      {
        WM_HideWindow(hItem);
      }
      else
      {
        WM_ShowWindow(hItem);
      }
      break;
    }
    case UI_MSG_BATTERY:
    {
      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_LEVEL);
      ui_battery_level_icon_updata(hItem, 0);
      break;
    }
    case WM_DELETE:
    {
      WM_SetFocus(back);
      WM_ShowWindow(back);

      break;
    }
    default:
    {
      WM_DefaultProc(pMsg);
      break;
    }
  }
}

void ui_menu_standby_open(WM_HWIN _back_)
{
  back = _back_;
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
