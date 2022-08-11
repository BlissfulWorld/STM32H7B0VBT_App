#include "ui.h"
#include "DIALOG.h"
#include "stdint.h"
#include "test.h"
#include "battery.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "config.h"
#include "analog_speaker.h"
#include "wav.h"
#include "user_settings.h"
#include "pwdn_watchdog.h"

#define FONT_DEF      (font_ext)
#define PROG_FONT_DEF (GUI_FONT_20_ASCII)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)
#define ID_TEXT_BOTTOM      (GUI_ID_USER + 0x04)

#define ID_TEXT_MAX_MIN     (GUI_ID_USER + 0x10)
#define ID_TEXT_VOLUME      (GUI_ID_USER + 0x11)

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
  { TEXT_CreateIndirect,    0, ID_TEXT_VOLUME,          90, 144,  60,  40,   0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BACK,             0, 320 - 30,  80,  30,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_OK,              80, 320 - 30,  80,  30,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_SAVE,           160, 320 - 30,  80,  30,   0,  0x0, 0},
};

static WM_HWIN back;

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
      TEXT_SetText(hItem, "系统音量");

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
      TEXT_SetText(hItem, "10\n▲\n\n\n\n\n▼\n0");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_VOLUME);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF8C00));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      char str[128];
      snprintf(str, sizeof(str), "%u", analog_speaker_volume_get());
      TEXT_SetText(hItem, str);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "返回");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "播放");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SAVE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "保存");

      uint8_t volume_saved;
      if((US_GET(volume, &volume_saved) == 0) && (analog_speaker_volume_get() == volume_saved))
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
      uint8_t volume = analog_speaker_volume_get();
      switch(((WM_KEY_INFO*)(pMsg->Data.p))->Key)
      {
        case UI_KEY(UP, REPET):
          volume += 2;
        case UI_KEY(UP, ONCLICK):
        {
          volume++;
          analog_speaker_volume_set(volume);
          volume = analog_speaker_volume_get();
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_VOLUME);
          char str[128];
          snprintf(str, sizeof(str), "%u", volume);
          TEXT_SetText(hItem, str);
          break;
        }
        case UI_KEY(DOWN, REPET):
          if(volume > 2) volume -= 2;
          else volume = 0;
        case UI_KEY(DOWN, ONCLICK):
        {
          if(volume > 0) volume--;
          analog_speaker_volume_set(volume);
          volume = analog_speaker_volume_get();
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_VOLUME);
          char str[128];
          snprintf(str, sizeof(str), "%u", volume);
          TEXT_SetText(hItem, str);
          break;
        }
        case UI_KEY(LEFT, ONCLICK):
        {
          GUI_EndDialog(pMsg->hWin, 0);
          break;
        }
        case UI_KEY(RIGHT, ONCLICK):
        {
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SAVE);
          if(WM_IsVisible(hItem))
          {
            US_SET(volume, &volume);
          }
          break;
        }
        case UI_KEY(OK, ONCLICK):
        {
          wav_play(audio_file_board_loaded);
          break;
        }
      }

      uint8_t volume_saved;
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SAVE);
      if((US_GET(volume, &volume_saved) == 0) && (volume == volume_saved))
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

void ui_menu_volume_open(WM_HWIN _back_)
{
  back = _back_;
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
