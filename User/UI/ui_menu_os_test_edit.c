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
#include "user_settings.h"

#define FONT_DEF      (font_ext)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)

#define ID_TEXT_INC         (GUI_ID_USER + 0x20)
#define ID_TEXT_SUB         (GUI_ID_USER + 0x21)
#define ID_TEXT_VAL         (GUI_ID_USER + 0x22)

#define ID_TEXT_CANCEL      (GUI_ID_USER + 0x30)
#define ID_TEXT_NONE        (GUI_ID_USER + 0x31)
#define ID_TEXT_CONFIRM     (GUI_ID_USER + 0x32)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,             75, 120,  90,  80,   0,  0x0, 0},
  { BUTTON_CreateIndirect,  0, ID_BUTTON_0,            240, 320,   1,   1,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_INC,              0,   0,  90,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_VAL,              0,  20,  90,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_SUB,              0,  40,  90,  20,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_CANCEL,           0,  60,  30,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_NONE,            30,  60,  30,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_CONFIRM,         60,  60,  30,  20,   0,  0x0, 0},
};

static struct ui_menu_os_test_var_s
{
  void (*cb)(void* p);
  void* p;
  uint32_t value;
  char str_buf[2][64];
  void (*set) (uint32_t val);
  uint32_t(*get) (void);
  uint32_t(*convert) (uint32_t);
  void (*string) (char*, uint32_t, uint32_t);
}*var;

static void _cbDialog(WM_MESSAGE* pMsg) {
  WM_HWIN hItem;

  switch(pMsg->MsgId)
  {
    case WM_INIT_DIALOG:
    {
      hItem = pMsg->hWin;
      WINDOW_SetBkColor(hItem, GUI_MAKE_COLOR(0x333333));

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INC);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x333333));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "--");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_VAL);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x333333));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetFont(hItem, FONT_DEF);
      var->value = var->convert(var->get());
      var->string(var->str_buf[0], sizeof(var->str_buf[0]), var->value);
      TEXT_SetText(hItem, var->str_buf[0]);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SUB);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0x333333));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "--");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CANCEL);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_m_ostest_edit.btn_cancel);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NONE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CONFIRM);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_m_ostest_edit.btn_confirm);

      WM_SetFocus(pMsg->hWin);
      break;
    }
    case WM_KEY:
    {
      pwdn_watchdog_reload();
      switch(((WM_KEY_INFO*)(pMsg->Data.p))->Key)
      {
        case UI_KEY(UP, REPET):
          var->value += 9;
        case UI_KEY(UP, ONCLICK):
        {
          var->value++;
          var->value = var->convert(var->value);
          var->string(var->str_buf[0], sizeof(var->str_buf[0]), var->value);
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_VAL);
          TEXT_SetText(hItem, var->str_buf[0]);
          break;
        }
        case UI_KEY(DOWN, REPET):
          var->value -= 9;
        case UI_KEY(DOWN, ONCLICK):
        {
          var->value--;
          var->value = var->convert(var->value);
          var->string(var->str_buf[0], sizeof(var->str_buf[0]), var->value);
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_VAL);
          TEXT_SetText(hItem, var->str_buf[0]);
          break;
        }
        case UI_KEY(LEFT, ONCLICK):
        {
          GUI_EndDialog(pMsg->hWin, 0);
          break;
        }
        case UI_KEY(RIGHT, ONCLICK):
        {
          var->set(var->value);
          GUI_EndDialog(pMsg->hWin, 0);
          break;
        }
      }
      break;
    }
    case WM_DELETE:
    {
      vPortFree(var);
      var->cb(var->p);
      break;
    }
    default:
    {
      WM_DefaultProc(pMsg);
      break;
    }
  }
}

void ui_menu_os_test_edit_open(void cb(void* p), void* p, void (*set) (uint32_t val), uint32_t(*get) (void), uint32_t(*convert) (uint32_t), void (*string) (char*, uint32_t, uint32_t))
{
  var = pvPortMalloc(sizeof(struct ui_menu_os_test_var_s));
  if(var)
  {
    var->cb = cb;
    var->p = p;
    var->set = set;
    var->get = get;
    var->convert = convert;
    var->string = string;
    GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  }
}
