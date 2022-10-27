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

typedef struct os_test_menu_item_s
{
  const char** name;
  void (*set) (uint32_t val);
  uint32_t(*get) (void);
  uint32_t(*convert) (uint32_t);
  void (*string) (char*, uint32_t, uint32_t);
}os_test_menu_item_st;

#define FONT_DEF      (font_ext)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)
#define ID_TEXT_BOTTOM      (GUI_ID_USER + 0x04)

#define ID_LISTVIEW_MENU    (GUI_ID_USER + 0x10)

#define ID_TEXT_BACK        (GUI_ID_USER + 0x30)
#define ID_TEXT_EDIT        (GUI_ID_USER + 0x31)
#define ID_TEXT_NONE        (GUI_ID_USER + 0x32)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_TITLE,            0,   0, 240,  44,   0,  0x0, 0},
  { IMAGE_CreateIndirect,   0, ID_IMAGE_LEVEL,         180,  24,  20,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BOTTOM,           0, 320 - 30, 240,  30,   0,  0x0, 0},

  { LISTVIEW_CreateIndirect,0, ID_LISTVIEW_MENU,         0,  44, 240, 320 - 30 - 44,   0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BACK,            20, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_EDIT,            90, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_NONE,           160, 320 - 28,  60,  28,   0,  0x0, 0},
};

static void item_func_set(uint32_t val)
{
  uint8_t tmp = val ? 1 : 0;
  US_SET(ostest, &tmp);
}

static uint32_t item_func_get(void)
{
  uint8_t tmp = 1;
  US_GET(ostest, &tmp);
  return (uint32_t)(tmp & 0x000000FF);
}

static uint32_t item_func_convert(uint32_t val)
{
  return val != 0 && val <= 0x80000000 ? 1 : 0;
}

static void item_func_string(char* str_buf, uint32_t size_max, uint32_t val)
{
  snprintf(str_buf, size_max, "%s", val ? ui_str.ui_m_ostest.function_enable : ui_str.ui_m_ostest.function_disable);
}

static void item_mk_conf_err_set(uint32_t val)
{
  uint16_t tmp = val <= 1000 ? val : 1000;
  US_SET(ostest_mk_conf_err_pct, &tmp);
}

static uint32_t item_mk_conf_err_get(void)
{
  uint16_t tmp = 5;
  US_GET(ostest_mk_conf_err_pct, &tmp);
  if(tmp > 1000) tmp = 1000;
  return (uint32_t)(tmp & 0x0000FFFF);
}

static uint32_t item_mk_conf_err_convert(uint32_t val)
{
  if(val >= 0x80000000)
    val = 0;
  else if(val > 1000)
    val = 1000;

  return val;
}

static void item_mk_conf_err_string(char* str_buf, uint32_t size_max, uint32_t val)
{
  snprintf(str_buf, size_max, "%u%% ", val);
}

static const os_test_menu_item_st menu_item[] =
{
  {&ui_str.ui_m_ostest.function,item_func_set,item_func_get,item_func_convert,item_func_string},
  {&ui_str.ui_m_ostest.max_error,item_mk_conf_err_set,item_mk_conf_err_get,item_mk_conf_err_convert,item_mk_conf_err_string},
};

static struct ui_menu_os_test_var_s
{
  WM_HWIN back;
  int select;
  char str_buf[2][64];
}*var;

static void edit_cb(void* p)
{
  WM_HWIN hitem;
  WM_HWIN hwin;

  hwin = (WM_HWIN)p;

  hitem = WM_GetDialogItem(hwin, ID_LISTVIEW_MENU);
  menu_item[var->select].string(var->str_buf[0],
    sizeof(var->str_buf[0]),
    menu_item[var->select].convert(
      menu_item[var->select].get())
  );
  LISTVIEW_SetItemText(hitem, 1, var->select, var->str_buf[0]);

  WM_SetFocus(hwin);
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
      TEXT_SetText(hItem, ui_str.ui_m_ostest.title);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BOTTOM);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_MENU);
      LISTVIEW_SetFont(hItem, FONT_DEF);
      HEADER_SetFont(LISTVIEW_GetHeader(hItem), FONT_DEF);
      LISTVIEW_AddColumn(hItem, 160, "", GUI_TA_LEFT | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 80, "", GUI_TA_RIGHT | GUI_TA_VCENTER);
      LISTVIEW_SetColumnWidth(hItem, LISTVIEW_ALL_ITEMS, 40);
      LISTVIEW_SetHeaderHeight(hItem, 0);

      for(uint32_t i = 0;i < sizeof(menu_item) / sizeof(os_test_menu_item_st);i++)
      {
        LISTVIEW_AddRow(hItem, 0);
        LISTVIEW_SetItemText(hItem, 0, i, *menu_item[i].name);
        menu_item[i].string(var->str_buf[0], sizeof(var->str_buf[0]), menu_item[i].convert(menu_item[i].get()));
        LISTVIEW_SetItemText(hItem, 1, i, var->str_buf[0]);
      }

      LISTVIEW_SetSel(hItem, var->select);

      WM_EnableMemdev(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
      TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_m_ostest.btn_back);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_EDIT);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_m_ostest.btn_edit);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NONE);
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
            ui_menu_os_test_edit_open(edit_cb,
              (void*)pMsg->hWin,
              menu_item[var->select].set,
              menu_item[var->select].get,
              menu_item[var->select].convert,
              menu_item[var->select].string);
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

void ui_menu_os_test_open(WM_HWIN back)
{
  var = pvPortMalloc(sizeof(struct ui_menu_os_test_var_s));
  if(var)
  {
    var->select = 0;
    var->back = back;
    GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  }
}
