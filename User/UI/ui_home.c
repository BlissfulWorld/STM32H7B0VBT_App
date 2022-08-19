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
#include "ff.h"
#include "hash.h"
#include "spi.h"
#include "config_file.h"
#include "pwdn_watchdog.h"

#define FONT_DEF      (font_ext)
#define PROG_FONT_DEF (GUI_FONT_20_ASCII)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)
#define ID_TEXT_BOTTOM      (GUI_ID_USER + 0x04)

#define ID_TEXT_UPDATE      (GUI_ID_USER + 0x10)
#define ID_TEXT_OK          (GUI_ID_USER + 0x11)
#define ID_TEXT_FILE        (GUI_ID_USER + 0x12)

#define ID_TEXT_T_NAME      (GUI_ID_USER + 0x20)
#define ID_TEXT_NAME        (GUI_ID_USER + 0x21)

#define ID_TEXT_T_VERSION   (GUI_ID_USER + 0x30)
#define ID_TEXT_VERSION     (GUI_ID_USER + 0x31)

#define ID_TEXT_T_DESC      (GUI_ID_USER + 0x40)
#define ID_TEXT_DESC        (GUI_ID_USER + 0x41)

#define ID_TEXT_INFO        (GUI_ID_USER + 0x50)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320,   0,  0x0, 0},
  { BUTTON_CreateIndirect,  0, ID_BUTTON_0,            240, 320,   1,   1,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_TITLE,            0,   0, 240,  44,   0,  0x0, 0},
  { IMAGE_CreateIndirect,   0, ID_IMAGE_LEVEL,         180,  24,  20,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BOTTOM,           0, 320 - 30, 240,  30,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_T_NAME,           0,  44, 240,  20,   0,  0x00, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_NAME,             0,  64, 240,  40,   0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_T_VERSION,        0, 104, 240,  20,   0,  0x00, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_VERSION,          0, 124, 240,  20,   0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_T_DESC,           0, 144, 240,  20,   0,  0x00, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_DESC,             0, 164, 240, 320 - 30 - 44 - 164, 0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_INFO,            70, 144, 100,  40,   0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_UPDATE,          20, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_OK,              90, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_FILE,           160, 320 - 28,  60,  28,   0,  0x0, 0},
};

static uint32_t btn;
static long timer;
static uint8_t* new_config_file_md5;

static uint8_t* new_config_file_md5_get(void)
{
  uint8_t* md5;
  FRESULT res;
  FIL fil;
  const config_file_st* config_file;

  md5 = 0;

  config_file = config_file_load_from_disk(config_file_update_path);
  if(config_file == 0)
  {
    goto __exit;
  }

  md5 = pvPortMalloc(16);
  if(md5 == 0)
  {
    goto __config_file_unload;
  }

  memcpy(md5, config_file->md5, 16);

  goto __config_file_unload;

__free_md5:
  vPortFree(md5);
  md5 = 0;

__config_file_unload:
  config_file_unload(config_file);

__exit:
  return md5;
}

static void _cbDialog(WM_MESSAGE* pMsg) {
  WM_HWIN                hItem;
  int                    NCode;
  int                    Id;
  char str[1024];

  switch(pMsg->MsgId)
  {
    case WM_INIT_DIALOG:
    {
      if(module)
      {
        test_module_unload(module);
        module = 0;
      }

      hItem = pMsg->hWin;
      WINDOW_SetBkColor(hItem, GUI_MAKE_COLOR(0));

      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_LEVEL);
      ui_battery_level_icon_updata(hItem, 0);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_TITLE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_BOTTOM);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "配置");
      
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BOTTOM);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_T_NAME);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x8a8c8e));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0000FF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "名称");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_T_VERSION);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x8a8c8e));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0000FF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "版本");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_T_DESC);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x8a8c8e));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0000FF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "描述");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NAME);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_VERSION);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_DESC);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      WM_EnableMemdev(hItem);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_UPDATE);
      TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_FILE);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "文件");

      new_config_file_md5 = new_config_file_md5_get();

      WM_SetFocus(pMsg->hWin);

      timer = WM_CreateTimer(pMsg->hWin, 0, 500, 0);
      break;
    }
    case WM_TIMER:
    {
      test_module_st* __IO _module_;
      if(module)
      {
        break;
      }
      board_power_on();
      osDelay(200);
      _module_ = test_module_load();
      if(_module_)
      {
        pwdn_watchdog_reload();

        common_config_info_st config_info;
        common_config_info_phase(_module_->config_file, &config_info);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
        WM_HideWindow(hItem);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_T_NAME);
        WM_ShowWindow(hItem);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_T_VERSION);
        WM_ShowWindow(hItem);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_T_DESC);
        WM_ShowWindow(hItem);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NAME);
        WM_ShowWindow(hItem);
        snprintf(str, sizeof(str), "%s", config_info.name);
        TEXT_SetText(hItem, str);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_VERSION);
        WM_ShowWindow(hItem);
        snprintf(str, sizeof(str), "%u", config_info.version);
        TEXT_SetText(hItem, str);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_DESC);
        WM_ShowWindow(hItem);
        snprintf(str, sizeof(str), "%s", config_info.desc);
        TEXT_SetText(hItem, str);

        if(new_config_file_md5 && ((_module_->config_file == 0) || memcmp(_module_->config_file->md5, new_config_file_md5, 16)))
        {
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_UPDATE);
          TEXT_SetText(hItem, "更新");
        }

        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
        TEXT_SetText(hItem, "开始");

        wav_play(audio_file_board_loaded);

        module = _module_;
      }
      else
      {
        WM_RestartTimer(timer, 300);
        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
        snprintf(str, sizeof(str), "等待测试板\n%s", (osKernelGetTickCount() / 500) & 1 ? "---" : "+++");
        TEXT_SetText(hItem, str);

        board_power_off();
      }
      break;
    }
    case WM_KEY:
    {
      pwdn_watchdog_reload();
      switch(((WM_KEY_INFO*)(pMsg->Data.p))->Key)
      {
        case UI_KEY(OK, ONCLICK):
        {
          if(module)
          {
            btn = ID_TEXT_OK;
            GUI_EndDialog(pMsg->hWin, 0);
          }
          break;
        }
        case UI_KEY(RIGHT, ONCLICK):
        {
          btn = ID_TEXT_FILE;
          GUI_EndDialog(pMsg->hWin, 0);
          break;
        }
        case UI_KEY(LEFT, ONCLICK):
        {
          if(module && new_config_file_md5 && ((module->config_file == 0) || memcmp(module->config_file->md5, new_config_file_md5, 16)))
          {
            btn = ID_TEXT_UPDATE;
            GUI_EndDialog(pMsg->hWin, 0);
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
      if(new_config_file_md5)
      {
        vPortFree(new_config_file_md5);
        new_config_file_md5 = 0;
      }

      WM_DeleteTimer(timer);

      if(btn == ID_TEXT_OK)
      {
        if(test_start(module))
          ui_nonsupport_open();
      }
      else if(btn == ID_TEXT_UPDATE)
      {
        ui_config_update_open();
      }
      else if(btn == ID_TEXT_FILE)
      {
        ui_file_open();
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

void ui_home_open(void)
{
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
