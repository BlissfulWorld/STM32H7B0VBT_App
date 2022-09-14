#include "ui.h"
#include "DIALOG.h"
#include "stdint.h"
#include "test.h"
#include "battery.h"
#include "stdio.h"
#include "fatfs.h"
#include "cmsis_os.h"
#include "wav.h"
#include "pwdn_watchdog.h"
#include "user_settings.h"

#define FONT_DEF      (font_ext)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)
#define ID_TEXT_BOTTOM      (GUI_ID_USER + 0x04)

#define ID_LISTVIEW_MENU    (GUI_ID_USER + 0x10)

#define ID_TEXT_BACK        (GUI_ID_USER + 0x30)
#define ID_TEXT_SELECT      (GUI_ID_USER + 0x31)
#define ID_TEXT_SAVE        (GUI_ID_USER + 0x32)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_TITLE,            0,   0, 240,  44,   0,  0x0, 0},
  { IMAGE_CreateIndirect,   0, ID_IMAGE_LEVEL,         180,  24,  20,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BOTTOM,           0, 320 - 30, 240,  30,   0,  0x0, 0},

  { LISTVIEW_CreateIndirect,0, ID_LISTVIEW_MENU,         0,  44, 240, 320 - 30 - 44,   0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BACK,            20, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_SELECT,          90, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_SAVE,           160, 320 - 28,  60,  28,   0,  0x0, 0},
};


static struct ui_menu_os_test_var_s
{
  WM_HWIN back;
  int select;
  char str_buf[2][64];
}*var;

static FRESULT listview_scan_files(WM_HWIN hlv, const char* root)
{
  FRESULT res;
  DIR* dir;
  UINT i = 0;
  FILINFO* fno;
  char str_buf[128];

  dir = pvPortMalloc(sizeof(DIR));
  if(dir == 0)
  {
    return FR_NO_FILE;
  }

  fno = pvPortMalloc(sizeof(FILINFO));
  if(fno == 0)
  {
    vPortFree(dir);
    return FR_NO_FILE;
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
        char* ptr = str_buf;
        snprintf(ptr, sizeof(str_buf), "%s/%s", root, fno->fname);
        if(ml_get_name(ptr, ptr, sizeof(str_buf)) == 0)
        {
          LISTVIEW_AddRow(hlv, 0);
          LISTVIEW_SetItemText(hlv, 0, i, ptr);
          snprintf(ptr, sizeof(str_buf), "%s/%s", root, fno->fname);
          LISTVIEW_SetItemText(hlv, 1, i, ptr);
          LISTVIEW_SetItemText(hlv, 2, i, fno->fname);
          i++;
        }
      }
    }
  }

  f_closedir(dir);
  vPortFree(dir);
  vPortFree(fno);
  return res;
}

static void _cbDialog(WM_MESSAGE* pMsg) {
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
      TEXT_SetText(hItem, ui_str.ui_m_language.title);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BOTTOM);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_MENU);
      LISTVIEW_SetFont(hItem, FONT_DEF);
      HEADER_SetFont(LISTVIEW_GetHeader(hItem), FONT_DEF);
      LISTVIEW_AddColumn(hItem, 240, "", GUI_TA_LEFT | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 0, "", GUI_TA_LEFT | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 0, "", GUI_TA_LEFT | GUI_TA_VCENTER);
      LISTVIEW_SetColumnWidth(hItem, LISTVIEW_ALL_ITEMS, 40);
      LISTVIEW_SetHeaderHeight(hItem, 0);
      char dir[128];
      snprintf(dir, sizeof(dir), "%slanguage", USERPath);
      listview_scan_files(hItem, dir);

      WM_EnableMemdev(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
      TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_m_language.btn_back);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SELECT);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_m_language.btn_select);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SAVE);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      WM_SetFocus(pMsg->hWin);
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
          var->select = -1;
          GUI_EndDialog(pMsg->hWin, 0);
          break;
        }
        case UI_KEY(OK, ONCLICK):
        {
          hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_MENU);
          var->select = LISTVIEW_GetSel(hItem);
          if(var->select >= 0)
          {
            char path[US_LEN(language)];
            LISTVIEW_GetItemText(hItem, 1, LISTVIEW_GetSel(hItem), path, sizeof(path));

            ml_set(path);

            LISTVIEW_GetItemText(hItem, 2, LISTVIEW_GetSel(hItem), path, sizeof(path));
            US_SET(language, path);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_TITLE);
            TEXT_SetText(hItem, ui_str.ui_m_language.title);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
            TEXT_SetText(hItem, ui_str.ui_m_language.btn_back);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SELECT);
            TEXT_SetText(hItem, ui_str.ui_m_language.btn_select);
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
      WM_SetFocus(var->back);
      WM_ShowWindow(var->back);
      WM_SendMessageNoPara(var->back,UI_MSG_ML);
      vPortFree(var);
      break;
    }
    default:
    {
      WM_DefaultProc(pMsg);
      break;
    }
  }
}

void ui_menu_language_open(WM_HWIN back)
{
  var = pvPortMalloc(sizeof(struct ui_menu_os_test_var_s));
  if(var)
  {
    var->select = 0;
    var->back = back;
    GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  }
}
