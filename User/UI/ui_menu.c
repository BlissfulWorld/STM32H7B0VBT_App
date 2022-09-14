#include "ui.h"
#include "DIALOG.h"
#include "stdint.h"
#include "test.h"
#include "battery.h"
#include "stdio.h"
#include "ff.h"
#include "cmsis_os.h"
#include "wav.h"
#include "pwdn_watchdog.h"

typedef struct menu_item_s
{
  void (*ui_open)(WM_HWIN back);
  const char** name;
}menu_item_st;

#define FONT_DEF      (font_ext)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)
#define ID_TEXT_BOTTOM      (GUI_ID_USER + 0x04)

#define ID_LISTVIEW_MENU    (GUI_ID_USER + 0x10)

#define ID_TEXT_BACK        (GUI_ID_USER + 0x30)
#define ID_TEXT_OK          (GUI_ID_USER + 0x31)
#define ID_TEXT_NONE        (GUI_ID_USER + 0x32)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_TITLE,            0,   0, 240,  44,   0,  0x0, 0},
  { IMAGE_CreateIndirect,   0, ID_IMAGE_LEVEL,         180,  24,  20,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BOTTOM,           0, 320 - 30, 240,  30,   0,  0x0, 0},

  { LISTVIEW_CreateIndirect,0, ID_LISTVIEW_MENU,         0,  44, 240, 320 - 30 - 44,   0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BACK,            20, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_OK,              90, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_NONE,           160, 320 - 28,  60,  28,   0,  0x0, 0},
};

static ui_close_cb_func_t close_cb;
static void* user_p;
static int select = 0;

const menu_item_st menu_item[] =
{
  {ui_menu_brightness_open,&ui_str.ui_menu.brightness},
  {ui_menu_volume_open,&ui_str.ui_menu.volume},
  {ui_menu_standby_open,&ui_str.ui_menu.standby},
  {ui_menu_u_disk_mode_open,&ui_str.ui_menu.u_disk_mode},
  {ui_menu_os_test_open,&ui_str.ui_menu.os_test},
  {ui_menu_clone_app,&ui_str.ui_menu.clone_app},
  {ui_menu_language_open,&ui_str.ui_menu.language},
  {ui_menu_about_open,&ui_str.ui_menu.about}
};

static void update_text(WM_HWIN win)
{
  WM_HWIN hItem;

  hItem = WM_GetDialogItem(win, ID_TEXT_TITLE);
  TEXT_SetText(hItem, ui_str.ui_menu.title);

  hItem = WM_GetDialogItem(win, ID_LISTVIEW_MENU);
  for(uint32_t i = 0;i < sizeof(menu_item) / sizeof(menu_item_st);i++)
  {
    LISTVIEW_SetItemText(hItem, 0, i, *menu_item[i].name);
    LISTVIEW_SetItemText(hItem, 1, i, ">>");
  }

  hItem = WM_GetDialogItem(win, ID_TEXT_BACK);
  TEXT_SetText(hItem, ui_str.ui_menu.btn_back);

  hItem = WM_GetDialogItem(win, ID_TEXT_OK);
  TEXT_SetText(hItem, ui_str.ui_menu.btn_confirm);
}

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
      ui_battery_level_icon_updata(hItem, 0);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_TITLE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_BOTTOM);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_menu.title);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BOTTOM);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_MENU);
      LISTVIEW_SetFont(hItem, FONT_DEF);
      HEADER_SetFont(LISTVIEW_GetHeader(hItem), FONT_DEF);
      LISTVIEW_AddColumn(hItem, 120, "", GUI_TA_LEFT | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 120, "", GUI_TA_RIGHT | GUI_TA_VCENTER);
      LISTVIEW_SetColumnWidth(hItem, LISTVIEW_ALL_ITEMS, 40);
      LISTVIEW_SetHeaderHeight(hItem, 0);

      for(uint32_t i = 0;i < sizeof(menu_item) / sizeof(menu_item_st);i++)
      {
        LISTVIEW_AddRow(hItem, 0);
        LISTVIEW_SetItemText(hItem, 0, i, *menu_item[i].name);
        LISTVIEW_SetItemText(hItem, 1, i, ">>");
      }

      LISTVIEW_SetSel(hItem, select);

      WM_EnableMemdev(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
      TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_menu.btn_back);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_menu.btn_confirm);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NONE);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      WM_SetFocus(pMsg->hWin);
      break;
    }
    case UI_MSG_ML:
    {
      if(WM_IsVisible(pMsg->hWin))
        update_text(pMsg->hWin);
      break;
    }
    case WM_KEY:
    {
      pwdn_watchdog_reload();
      switch(((WM_KEY_INFO*)(pMsg->Data.p))->Key)
      {
        case UI_KEY(UP, REPET):
        case UI_KEY(UP, ONCLICK):
        {
          GUI_SendKeyMsg(GUI_KEY_UP, 1);
          break;
        }
        case UI_KEY(DOWN, REPET):
        case UI_KEY(DOWN, ONCLICK):
        {
          GUI_SendKeyMsg(GUI_KEY_DOWN, 1);
          break;
        }
        case UI_KEY(LEFT, ONCLICK):
        {
          select = -1;
          GUI_EndDialog(pMsg->hWin, 0);
          break;
        }
        case UI_KEY(OK, ONCLICK):
        {
          hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_MENU);
          select = LISTVIEW_GetSel(hItem);
          if(select >= 0)
          {
            WM_HideWindow(pMsg->hWin);
            menu_item[select].ui_open(pMsg->hWin);
          }
          break;
        }
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
      if(select == -1)
      {
        close_cb(user_p);
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

void ui_menu_open(ui_close_cb_func_t cb, void* user)
{
  select = 0;
  close_cb = cb;
  user_p = user;
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
