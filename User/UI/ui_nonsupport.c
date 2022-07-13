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

#define FONT_DEF      (font_ext)
#define PROG_FONT_DEF (GUI_FONT_20_ASCII)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)

#define ID_TEXT_INFO    (GUI_ID_USER + 0x10)

#define ID_TEXT_BACK        (GUI_ID_USER + 0x30)
#define ID_TEXT_OK          (GUI_ID_USER + 0x31)
#define ID_TEXT_NONE        (GUI_ID_USER + 0x32)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320, 0,  0x0, 0},
  { BUTTON_CreateIndirect,  0, ID_BUTTON_0,            240, 320,   1,   1, 0,  0x0, 0},
  
  { TEXT_CreateIndirect,    0, ID_TEXT_TITLE,            0,   0, 240,  44, 0,  0x0, 0},
  
  { IMAGE_CreateIndirect,    0, ID_IMAGE_LEVEL,        180,  24,  20,  20, 0,  0x0, 0},
  
  
  { TEXT_CreateIndirect, 0, ID_TEXT_INFO, 0,  44, 240, 320-30-44, 0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BACK,             0, 320-30, 80,  30, 0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_OK,              80, 320-30, 80,  30, 0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_NONE,           160, 320-30, 80,  30, 0,  0x0, 0},
};

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

      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_LEVEL);
      ui_battery_level_icon_updata(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_TITLE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_BOTTOM);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "错误");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "不支持");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");
      
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");
      
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NONE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");
      
      WM_SetFocus(pMsg->hWin);
    }
    case WM_KEY:
    {
      break;
    }
    case UI_MSG_BATTERY:
    {
      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_LEVEL);
      ui_battery_level_icon_updata(hItem);
      break;
    }
    case WM_DELETE:
    {
      break;
    }
    default:
    {
      WM_DefaultProc(pMsg);
      break;
    }
  }
}

void ui_nonsupport_open(void)
{
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}