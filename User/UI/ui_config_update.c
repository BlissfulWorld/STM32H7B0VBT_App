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
#include "test.h"
#include "cJSON.h"
#include "string.h"
#include "test.h"
#include "pwdn_watchdog.h"

#define FONT_DEF      (font_ext)
#define PROG_FONT_DEF (GUI_FONT_20_ASCII)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)

#define ID_TEXT_NONE        (GUI_ID_USER + 0x10)
#define ID_TEXT_BACK        (GUI_ID_USER + 0x11)
#define ID_TEXT_UPDATE      (GUI_ID_USER + 0x12)

#define ID_TEXT_NOTICE      (GUI_ID_USER + 0x20)

#define ID_LISTVIEW_CONFIG  (GUI_ID_USER + 0x30)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320,   0,  0x0, 0},
  { BUTTON_CreateIndirect,  0, ID_BUTTON_0,            240, 320,   1,   1,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_TITLE,            0,   0, 240,  44,   0,  0x0, 0},
  { IMAGE_CreateIndirect,   0, ID_IMAGE_LEVEL,         180,  24,  20,  20,   0,  0x0, 0},

  { LISTVIEW_CreateIndirect,0, ID_LISTVIEW_CONFIG,       0,  50, 240, 200,   0, 0x00, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_NOTICE,           0,  200 + 50, 240, 38,   0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BACK,             0, 320 - 30,  80,  30,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_UPDATE,          80, 320 - 30,  80,  30,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_NONE,           160, 320 - 30,  80,  30,   0,  0x0, 0},
};

static uint32_t status;

static void _cbDialog(WM_MESSAGE* pMsg) {
  WM_HWIN                hItem;
  int                    NCode;
  int                    Id;
  char str[1024];

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
      TEXT_SetText(hItem, "更新配置文件");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_CONFIG);
      LISTVIEW_SetFont(hItem, FONT_DEF);
      HEADER_SetFont(LISTVIEW_GetHeader(hItem), FONT_DEF);
      LISTVIEW_AddColumn(hItem, 50, "", GUI_TA_HCENTER | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 95, "当前配置", GUI_TA_HCENTER | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 95, "新配置", GUI_TA_HCENTER | GUI_TA_VCENTER);
      LISTVIEW_SetHeaderHeight(hItem, 20);
      LISTVIEW_SetWrapMode(hItem, GUI_WRAPMODE_WORD);
      LISTVIEW_SetGridVis(hItem, 1);
      LISTVIEW_SetRowHeight(hItem, 60);

      common_config_info_st config_info_old, config_info_new;
      const config_file_st* config_file_new;

      config_file_new = config_file_load_from_disk(config_file_update_path);

      common_config_info_phase(module->config_file, &config_info_old);
      common_config_info_phase(config_file_new, &config_info_new);

      LISTVIEW_AddRow(hItem, 0);
      LISTVIEW_SetItemText(hItem, 0, 0, "名称");
      LISTVIEW_SetItemTextColor(hItem, 0, 0, LISTVIEW_CI_UNSEL, 0x0000FF);
      LISTVIEW_SetItemText(hItem, 1, 0, config_info_old.name);
      LISTVIEW_SetItemText(hItem, 2, 0, config_info_new.name);

      LISTVIEW_AddRow(hItem, 0);
      LISTVIEW_SetItemText(hItem, 0, 1, "版本");
      LISTVIEW_SetItemTextColor(hItem, 0, 1, LISTVIEW_CI_UNSEL, 0x0000FF);
      snprintf(str, sizeof(str), "%d", config_info_old.version);
      LISTVIEW_SetItemText(hItem, 1, 1, str);
      snprintf(str, sizeof(str), "%d", config_info_new.version);
      LISTVIEW_SetItemText(hItem, 2, 1, str);

      LISTVIEW_AddRow(hItem, 0);
      LISTVIEW_SetItemText(hItem, 0, 2, "描述");
      LISTVIEW_SetItemTextColor(hItem, 0, 2, LISTVIEW_CI_UNSEL, 0x0000FF);
      LISTVIEW_SetItemText(hItem, 1, 2, config_info_old.desc);
      LISTVIEW_SetItemText(hItem, 2, 2, config_info_new.desc);

      config_file_unload(config_file_new);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NOTICE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0000FF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);

      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "返回");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_UPDATE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "更新");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NONE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
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
        case UI_KEY(OK, ONCLICK):
        {
          if(status == 0)
          {
            status = 1;
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NOTICE);
            TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0000FF));
            TEXT_SetText(hItem, "更新中。。。");
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
            TEXT_SetText(hItem, "");
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_UPDATE);
            TEXT_SetText(hItem, "");
            GUI_Exec();

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NOTICE);
            int err = config_file_update_to_board_from_disk(&module->board_header, config_file_update_path);
            if(err == 0)
            {
              TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x00FF00));
              TEXT_SetText(hItem, "更新完成");
            }
            else
            {
              TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000));
              snprintf(str, sizeof(str), "更新失败%d", err);
              TEXT_SetText(hItem, str);
            }

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
            TEXT_SetText(hItem, "返回");
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_UPDATE);
            TEXT_SetText(hItem, "更新");
            status = 0;
          }

          break;
        }
        case UI_KEY(LEFT, ONCLICK):
        {
          if(status == 0)
            GUI_EndDialog(pMsg->hWin, 0);
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

void ui_config_update_open(void)
{
  status = 0;

  LISTVIEW_SetDefaultBkColor(LISTVIEW_CI_UNSEL, GUI_MAKE_COLOR(0x00000000));
  LISTVIEW_SetDefaultBkColor(LISTVIEW_CI_SEL, GUI_MAKE_COLOR(0x00FF0000));
  LISTVIEW_SetDefaultBkColor(LISTVIEW_CI_DISABLED, GUI_MAKE_COLOR(0x00000000));
  LISTVIEW_SetDefaultBkColor(LISTVIEW_CI_SELFOCUS, GUI_MAKE_COLOR(0x00FF0000));

  LISTVIEW_SetDefaultTextColor(LISTVIEW_CI_UNSEL, GUI_MAKE_COLOR(0x00FFFFFF));
  LISTVIEW_SetDefaultTextColor(LISTVIEW_CI_SEL, GUI_MAKE_COLOR(0x00FFFFFF));
  LISTVIEW_SetDefaultTextColor(LISTVIEW_CI_SELFOCUS, GUI_MAKE_COLOR(0x00FFFFFF));
  LISTVIEW_SetDefaultTextColor(LISTVIEW_CI_DISABLED, GUI_MAKE_COLOR(0x00FFFFFF));

  LISTVIEW_SetDefaultGridColor(GUI_MAKE_COLOR(0xabc88b));

  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
