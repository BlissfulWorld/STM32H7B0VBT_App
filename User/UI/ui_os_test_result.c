#include "ui.h"
#include "DIALOG.h"
#include "stdint.h"
#include "test.h"
#include "battery.h"
#include "stdio.h"
#include "fatfs.h"
#include "cmsis_os.h"
#include "wav.h"
#include "stdio.h"
#include "config.h"
#include "sw_info.h"
#include "pwdn_watchdog.h"
#include "cJSON.h"
#include "json_obj.h"

#define FONT_DEF      (font_ext)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)
#define ID_TEXT_BOTTOM      (GUI_ID_USER + 0x04)

#define ID_TEXT_HEADER      (GUI_ID_USER + 0x10)
#define ID_LISTVIEW_RESULT  (GUI_ID_USER + 0x11)
#define ID_TEXT_INFO        (GUI_ID_USER + 0x12)

#define ID_TEXT_BACK        (GUI_ID_USER + 0x30)
#define ID_TEXT_EXPORT_DATA (GUI_ID_USER + 0x31)
#define ID_TEXT_MAKE_CONF   (GUI_ID_USER + 0x32)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320,   0,  0x0, 0},
  { BUTTON_CreateIndirect,  0, ID_BUTTON_0,            240, 320,   1,   1,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_TITLE,            0,   0, 240,  44,   0,  0x0, 0},
  { IMAGE_CreateIndirect,   0, ID_IMAGE_LEVEL,         180,  24,  20,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BOTTOM,           0, 320 - 30, 240,  30,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_HEADER,           0,  48, 240,  20,   0,  0x0, 0},
  { LISTVIEW_CreateIndirect,0, ID_LISTVIEW_RESULT,       0,  68, 240, 320 - 68 - 60, 0,  0x00, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_INFO,             0,  320 - 70, 240,  36,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BACK,             20, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_EXPORT_DATA,      90, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_MAKE_CONF,       160, 320 - 28,  60,  28,   0,  0x0, 0},
};

static WM_HWIN back;
static const float* data;
static const os_test_st* os_test;
static uint8_t len;
static const char* name;

/**
 * @brief 导出开短路数据到用户存储区
 *
 * @return int 错误码
 */
static int export_data(void)
{
  int err;
  char* str_buf;
  FIL* fil;
  UINT bw;

  err = 0;

  str_buf = pvPortMalloc(128);
  if(str_buf == 0)
  {
    err = -1;
    goto __exit;
  }

  fil = ff_malloc(sizeof(FIL));
  if(fil == 0)
  {
    err = -2;
    goto __free_str_buf;
  }

  //生成完整文件路径
  snprintf(str_buf, 128, "%sOS_TEST_RESULT_%s.txt", DISK_USER_PATH, name);
  for(int i = 0;str_buf[i];i++)
  {
    if(str_buf[i] == ' ') str_buf[i] = '_';
  }

  if(f_open(fil, str_buf, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
  {
    err = -3;
    goto __free_fil;
  }

  //写入每个引脚开短路信息到文件中
  for(uint8_t i = 0;i < len;i++)
  {
    if(f_write(fil,
      str_buf,
      snprintf(str_buf,
        128,
        "%02d: %.1f %s\n",
        i + 1,
        data[i],
        os_test[i].i_max >= data[i] && os_test[i].i_min <= data[i] ? "OK" : "ERR"
      ),
      &bw) != FR_OK)
    {
      err = -4;
      break;
    }
  }

  f_close(fil);

__free_fil:
  ff_free(fil);

__free_str_buf:
  vPortFree(str_buf);

__exit:
  return err;
}

/**
 * @brief 生成配置并写入小板中
 *
 * @return int 错误码
 */
static int make_config_to_board(void)
{
  int err;
  uint16_t data_error_pct;
  float data_error;
  cJSON* json;
  char str_buf[4];
  char* json_str;
  test_module_st* _module;

  err = 0;

  if(module == 0)
  {
    err = -1;
    goto __exit;
  }

  board_power_on();
  osDelay(100);

  _module = test_module_load();
  if(_module == 0)
  {
    err = -2;
    goto __exit;
  }

  if(module->id.code != _module->id.code)
  {
    err = -3;
    goto __module_unload;
  }

  json = 0;

  if(_module->config_file)
  {
    if(memcmp(_module->config_file->file.type, "json", 4))
    {
      err = -4;
      goto __module_unload;
    }

    json = cJSON_Parse((char*)(_module->config_file->file.data));
  }

  if(json == 0)
  {
    json = cJSON_CreateObject();
    if(json == 0)
    {
      err = -5;
      goto __module_unload;
    }
  }

  cJSON* j_test = cJSON_GetObjectItem(json, json_obj_test);
  if(j_test == 0)
  {
    j_test = cJSON_AddObjectToObject(json, json_obj_test);
    if(j_test == 0)
    {
      err = -6;
      goto __delele_json;
    }
  }

  cJSON_DeleteItemFromObject(j_test, json_obj_os);

  cJSON* j_os = cJSON_AddObjectToObject(j_test, json_obj_os);
  if(j_os == 0)
  {
    err = -7;
    goto __delele_json;
  }

  IWDG1->KR = IWDG_KEY_RELOAD;

  data_error_pct = 5;
  US_GET(ostest_mk_conf_err_pct, &data_error_pct);
  if(data_error_pct > 1000) data_error_pct = 1000;
  data_error = data_error_pct / 100.0;

  for(uint8_t i = 0;i < len;i++)
  {
    float d_e, min, max;
    sprintf(str_buf, "%u", i + 1);
    cJSON* js_item = cJSON_AddObjectToObject(j_os, str_buf);
    if(js_item == 0)
    {
      err = -8;
      goto __delele_json;
    }

    d_e = data[i] * data_error;
    max = data[i] + d_e;
    min = data[i] > d_e ? data[i] - d_e : 0;

    if(cJSON_AddNumberToObject(js_item, json_obj_max, max) == 0)
    {
      err = -9;
      goto __delele_json;
    }

    if(cJSON_AddNumberToObject(js_item, json_obj_min, min) == 0)
    {
      err = -10;
      goto __delele_json;
    }

    if(cJSON_AddStringToObject(js_item, json_obj_color_in, "0xFFFFFF") == 0)
    {
      err = -11;
      goto __delele_json;
    }

    if(cJSON_AddStringToObject(js_item, json_obj_color_out, "0xFF0000") == 0)
    {
      err = -12;
      goto __delele_json;
    }
  }

  json_str = cJSON_Print(json);
  if(json_str == 0)
  {
    err = -13;
    goto __delele_json;
  }
  cJSON_Delete(json);

  if(config_file_data_write_to_board(&_module->board_header, json_str, strlen(json_str)))
  {
    err = -14;
  }

  cJSON_free(json_str);

  goto __module_unload;

__delele_json:
  cJSON_Delete(json);

__module_unload:
  test_module_unload(_module);
  board_power_off();

__exit:
  return err;
}

static void _cbDialog(WM_MESSAGE* pMsg)
{
  WM_HWIN hItem;
  char    str_buf[128];

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
      TEXT_SetText(hItem, name);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BOTTOM);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_HEADER);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_BOTTOM);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_ostest_result.title);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_RESULT);
      LISTVIEW_SetFont(hItem, GUI_FONT_13_ASCII);
      HEADER_SetFont(LISTVIEW_GetHeader(hItem), GUI_FONT_13B_ASCII);
      LISTVIEW_AddColumn(hItem, 60, "", GUI_TA_HCENTER | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 60, "", GUI_TA_HCENTER | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 60, "", GUI_TA_HCENTER | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 60, "", GUI_TA_HCENTER | GUI_TA_VCENTER);
      //LISTVIEW_SetColumnWidth(hItem, LISTVIEW_ALL_ITEMS, 0);
      LISTVIEW_SetHeaderHeight(hItem, 0);
      LISTVIEW_SetGridVis(hItem, 1);

      for(uint8_t i = 0;i < len;i++)
      {
        uint32_t row, col, color;
        row = i % 4;
        col = i / 4;
        if(row == 0)
          LISTVIEW_AddRow(hItem, 0);

        snprintf(str_buf, sizeof(str_buf), "%.1f", data[i]);
        LISTVIEW_SetItemText(hItem, row, col, str_buf);
        if(os_test[i].i_max >= data[i] && os_test[i].i_min <= data[i])
        {
          color = os_test->color_in;
        }
        else
        {
          color = os_test->color_out;
        }

        LISTVIEW_SetItemTextColor(hItem, row, col, LISTVIEW_CI_UNSEL, color);
        LISTVIEW_SetItemTextColor(hItem, row, col, LISTVIEW_CI_SEL, color);
      }

      WM_EnableMemdev(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x000000FF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
      TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_back);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_EXPORT_DATA);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_exp_data);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAKE_CONF);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_learn);

      WM_SetFocus(pMsg->hWin);
      break;
    }
    case WM_KEY:
    {
      pwdn_watchdog_reload();
      switch(((WM_KEY_INFO*)(pMsg->Data.p))->Key)
      {
        case UI_KEY(LEFT, ONCLICK):
        {
          if(!WM_IsVisible(WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK)))
          {
            break;
          }

          if(WM_IsVisible(WM_GetDialogItem(pMsg->hWin, ID_TEXT_EXPORT_DATA)) && WM_IsVisible(WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAKE_CONF)))
          {
            GUI_EndDialog(pMsg->hWin, 0);
          }
          else
          {
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
            TEXT_SetText(hItem, "");
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
            TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_back);
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_EXPORT_DATA);
            TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_exp_data);
            WM_ShowWindow(hItem);
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAKE_CONF);
            TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_learn);
            WM_ShowWindow(hItem);
          }
          break;
        }
        case UI_KEY(OK, ONCLICK):
        {
          if(!WM_IsVisible(WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK)) || !WM_IsVisible(WM_GetDialogItem(pMsg->hWin, ID_TEXT_EXPORT_DATA)))
          {
            break;
          }

          if(!WM_IsVisible(WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAKE_CONF)))
          {
            WM_HideWindow(WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK));
            WM_HideWindow(WM_GetDialogItem(pMsg->hWin, ID_TEXT_EXPORT_DATA));

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
            TEXT_SetText(hItem, ui_str.ui_ostest_result.exporting);
            GUI_Exec();

            //开始导出数据
            int err = export_data();
            if(err == 0)
            {
              TEXT_SetText(hItem, ui_str.ui_ostest_result.export_finished);
            }
            else
            {
              snprintf(str_buf, sizeof(str_buf), "%s (%d)", ui_str.ui_ostest_result.export_failed, err);
              TEXT_SetText(hItem, str_buf);
            }

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
            TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_back);
            WM_ShowWindow(hItem);
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_EXPORT_DATA);
            TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_exp_data);
            WM_ShowWindow(hItem);
            WM_ShowWindow(WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAKE_CONF));
            GUI_Exec();
            break;
          }

          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
          TEXT_SetText(hItem, ui_str.ui_ostest_result.export_data_to_storage);
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
          TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_no);
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_EXPORT_DATA);
          TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_yes);
          WM_HideWindow(WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAKE_CONF));
          break;
        }
        case UI_KEY(RIGHT, ONCLICK):
        {
          if(!WM_IsVisible(WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK)) || !WM_IsVisible(WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAKE_CONF)))
          {
            break;
          }

          if(!WM_IsVisible(WM_GetDialogItem(pMsg->hWin, ID_TEXT_EXPORT_DATA)))
          {
            WM_HideWindow(WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK));
            WM_HideWindow(WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAKE_CONF));

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
            TEXT_SetText(hItem, ui_str.ui_ostest_result.learning);
            GUI_Exec();

            //开始生成配置
            int err = make_config_to_board();
            if(err == 0)
            {
              TEXT_SetText(hItem, ui_str.ui_ostest_result.learn_finished);
            }
            else
            {
              snprintf(str_buf, sizeof(str_buf), "%s (%d)", ui_str.ui_ostest_result.learn_failed, err);
              TEXT_SetText(hItem, str_buf);
            }

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
            TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_back);
            WM_ShowWindow(hItem);
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAKE_CONF);
            TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_learn);
            WM_ShowWindow(hItem);
            WM_ShowWindow(WM_GetDialogItem(pMsg->hWin, ID_TEXT_EXPORT_DATA));
            GUI_Exec();
            break;
          }

          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_INFO);
          TEXT_SetText(hItem, ui_str.ui_ostest_result.learn_and_write_to_flash);
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BACK);
          TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_no);
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_MAKE_CONF);
          TEXT_SetText(hItem, ui_str.ui_ostest_result.btn_yes);
          WM_HideWindow(WM_GetDialogItem(pMsg->hWin, ID_TEXT_EXPORT_DATA));
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

void ui_os_test_result_open(const char* _name_, WM_HWIN _back_, const os_test_st* _os_test_, const float* _data_, uint8_t _len_)
{
  name = _name_;
  back = _back_;
  data = _data_;
  len = _len_;
  os_test = _os_test_;
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}
