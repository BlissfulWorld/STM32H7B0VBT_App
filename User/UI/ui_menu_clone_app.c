#include "ui.h"
#include "DIALOG.h"
#include "stdint.h"
#include "battery.h"
#include "stdio.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "fatfs.h"
#include "usb_host.h"
#include "disk.h"
#include "user_settings.h"
#include "pwdn_watchdog.h"

#define FONT_DEF      (font_ext)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)
#define ID_TEXT_BOTTOM      (GUI_ID_USER + 0x04)

#define ID_TEXT_INFO        (GUI_ID_USER + 0x40)
#define ID_MULTIEDIT_INFO   (GUI_ID_USER + 0x50)

#define ID_TEXT_BACK        (GUI_ID_USER + 0x30)
#define ID_TEXT_OK          (GUI_ID_USER + 0x31)
#define ID_TEXT_NONE        (GUI_ID_USER + 0x32)


static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,   0, ID_WINDOW_0,              0,   0, 240, 320,  0,  0x0, 0},
  { BUTTON_CreateIndirect,   0, ID_BUTTON_0,            240, 320,   1,   1,  0,  0x0, 0},

  { TEXT_CreateIndirect,     0, ID_TEXT_TITLE,            0,   0, 240,  44,  0,  0x0, 0},

  { IMAGE_CreateIndirect,    0, ID_IMAGE_LEVEL,         180,  24,  20,  20,  0,  0x0, 0},

  { TEXT_CreateIndirect,     0, ID_TEXT_BOTTOM,           0, 320 - 30, 240,  30,  0,  0x0, 0},

  { TEXT_CreateIndirect,     0, ID_TEXT_INFO,             0,  44, 240,  20,  0,  0x0, 0},

  { MULTIEDIT_CreateIndirect,0, ID_MULTIEDIT_INFO,       10,  64, 220, 320 - 30 - 64,  0,  0x00, 0},

  { TEXT_CreateIndirect,     0, ID_TEXT_BACK,             0, 320 - 30, 80,  30,  0,  0x0, 0},
  { TEXT_CreateIndirect,     0, ID_TEXT_OK,              80, 320 - 30, 80,  30,  0,  0x0, 0},
  { TEXT_CreateIndirect,     0, ID_TEXT_NONE,           160, 320 - 30, 80,  30,  0,  0x0, 0},
};

#define STAGE_IDLE       (0)
#define STAGE_CONNECTING (1)
#define STAGE_READY      (2)
#define STAGE_UPDATE     (3)
#define STAGE_FINISHED   (4)

extern USBH_HandleTypeDef hUsbHostHS;

static WM_HWIN back;
static long timer;
static uint32_t stage;
static FRESULT f_resut;

static int app_version_get(char* buffer, uint32_t size)
{
  int err;
  FRESULT res;
  FIL fil_src;
  char path[128];
  char version[16 + 1];
  UINT br;

  err = 0;

  snprintf(path, sizeof(path), "%sapp/XPTest.app", USERPath);
  res = f_open(&fil_src, path, FA_READ);
  if(res != FR_OK)
  {
    err = -1;
    goto __exit;
  }

  res = f_lseek(&fil_src, 16);
  if(res != FR_OK)
  {
    err = -2;
    goto __close_fil_src;
  }

  res = f_read(&fil_src, version, 16, &br);
  if(res != FR_OK)
  {
    err = -3;
    goto __close_fil_src;
  }

  version[16] = '\0';

  snprintf(buffer, size, "%s", version);

__close_fil_src:
  f_close(&fil_src);

__exit:
  return err;
}

static void file_copy_stat_cb(uint32_t total, uint32_t count, void* user)
{
  IWDG1->KR = IWDG_KEY_RELOAD;
  pwdn_watchdog_reload();
}

static int export_app(WM_HWIN hwin)
{
  int err;
  FRESULT res;
  DIR dir;
  UINT br, bw;
  char path_src[128], path_dst[128];
  uint32_t size, count;

  err = 0;

  snprintf(path_src, sizeof(path_src), "%sapp/XPTest.app", USERPath);

  snprintf(path_dst, sizeof(path_dst), "%sXPTest.app", USBHPath);

  if(disk_copy_file(path_dst, path_src, file_copy_stat_cb, 0) != FR_OK)
    err = -1;

__exit:
  f_resut = res;
  return err;
}

static void _cbDialog(WM_MESSAGE* pMsg) {
  WM_HWIN                hItem;
  int                    NCode;
  int                    Id;
  char str[128];

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
      TEXT_SetText(hItem, ui_str.ui_m_clone.title);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BOTTOM);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x333333));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0000FF));
      TEXT_SetFont(hItem, FONT_DEF);

      char version[17];
      if(app_version_get(version, sizeof(version)) == 0)
      {
        snprintf(str, sizeof(str), "%s : V%s", ui_str.ui_m_clone.software_version, version);
      }
      else
      {
        snprintf(str, sizeof(str), "%s : xx", ui_str.ui_m_clone.software_version);
      }
      TEXT_SetText(hItem, str);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_INFO);
      MULTIEDIT_SetFont(hItem, FONT_DEF);
      MULTIEDIT_SetTextAlign(hItem, GUI_TA_LEFT);
      MULTIEDIT_SetWrapWord(hItem);
      MULTIEDIT_SetBufferSize(hItem, 256);

      MULTIEDIT_SetTextColor(hItem, MULTIEDIT_CI_EDIT, GUI_MAKE_COLOR(0x00FFFFFF));
      MULTIEDIT_SetTextColor(hItem, MULTIEDIT_CI_READONLY, GUI_MAKE_COLOR(0x00FFFFFF));

      MULTIEDIT_SetBkColor(hItem, MULTIEDIT_CI_EDIT, GUI_MAKE_COLOR(0x00000000));
      MULTIEDIT_SetBkColor(hItem, MULTIEDIT_CI_READONLY, GUI_MAKE_COLOR(0x00000000));

      MULTIEDIT_ShowCursor(hItem, 0);

      snprintf(str, sizeof(str), "%s\n", ui_str.ui_m_clone.device_disconnect);
      MULTIEDIT_SetText(hItem, str);
      WM_EnableMemdev(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_m_clone.btn_back);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NONE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      stage = STAGE_IDLE;

      timer = WM_CreateTimer(pMsg->hWin, 0, 50, 0);

      WM_SetFocus(pMsg->hWin);

      MX_USB_HOST_Init();
    }
    case WM_TIMER:
    {
      //      WM_RestartTimer(timer,50);
      break;
    }
    case UI_USBH_APP_DISCONNECT:
    {
      hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_INFO);

      snprintf(str, sizeof(str), "%s\n", ui_str.ui_m_clone.device_disconnect);
      MULTIEDIT_AddText(hItem, str);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
      TEXT_SetText(hItem, "");

      f_mount(0, USBHPath, 1);

      stage = STAGE_IDLE;
      break;
    }
    case UI_USBH_APP_START:
    {
      hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_INFO);

      snprintf(str, sizeof(str), "%s\n", ui_str.ui_m_clone.connecting);
      MULTIEDIT_AddText(hItem, str);

      stage = STAGE_CONNECTING;
      break;
    }
    case UI_USBH_APP_READY:
    {
      hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_INFO);

      FRESULT res = f_mount(&USBHFatFS, USBHPath, 1);
      if(res == FR_OK)
      {
        MULTIEDIT_SetText(hItem, "");

        snprintf(str, sizeof(str), "%s\n", ui_str.ui_m_clone.connected);
        MULTIEDIT_AddText(hItem, str);

        hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
        TEXT_SetText(hItem, ui_str.ui_m_clone.btn_start);

        stage = STAGE_READY;
      }
      else
      {
        snprintf(str, sizeof(str), "%s (%d)\n", ui_str.ui_m_clone.error, res);
        MULTIEDIT_AddText(hItem, str);

        stage = STAGE_IDLE;
      }
      break;
    }
    case WM_KEY:
    {
      switch(((WM_KEY_INFO*)(pMsg->Data.p))->Key)
      {
        case UI_KEY(OK, ONCLICK):
        {
          if(stage == STAGE_READY)
          {
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
            WM_HideWindow(hItem);
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
            WM_HideWindow(hItem);
            hItem = WM_GetDialogItem(pMsg->hWin, ID_MULTIEDIT_INFO);
            stage = STAGE_UPDATE;

            snprintf(str, sizeof(str), "%s\n", ui_str.ui_m_clone.start_clone);
            MULTIEDIT_AddText(hItem, str);
            GUI_Exec();
            int err = export_app(pMsg->hWin);
            if(err == 0)
            {
              snprintf(str, sizeof(str), "%s\n", ui_str.ui_m_clone.clone_finished);
              MULTIEDIT_AddText(hItem, str);
            }
            else
            {
              snprintf(str, sizeof(str), "%s (%d,%u)\n", ui_str.ui_m_clone.clone_failed, err, f_resut);
              MULTIEDIT_AddText(hItem, str);
            }
            GUI_Exec();
            if(stage == STAGE_UPDATE)
            {
              stage = STAGE_READY;
              hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
              WM_ShowWindow(hItem);
              hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
              WM_ShowWindow(hItem);
            }
          }
          break;
        }
        case UI_KEY(RIGHT, ONCLICK):
        {
          break;
        }
        case UI_KEY(LEFT, ONCLICK):
        {
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
          if(WM_IsVisible(hItem))
          {
            GUI_EndDialog(pMsg->hWin, 0);
          }
          break;
        }
        case UI_KEY(DOWN, ONCLICK):
        {
          break;
        }
        case UI_KEY(UP, ONCLICK):
        {
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
      USBH_DeInit(&hUsbHostHS);

      WM_DeleteTimer(timer);

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

void ui_menu_clone_app(WM_HWIN _back_)
{
  back = _back_;
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
