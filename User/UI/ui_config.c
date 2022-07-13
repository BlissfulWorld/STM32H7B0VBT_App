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

typedef struct image_dta_s
{
  const void *p;
  unsigned long size;
}image_dta_st;

#define FONT_DEF      (font_ext)
#define PROG_FONT_DEF (GUI_FONT_20_ASCII)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)

#define ID_TEXT_NONE        (GUI_ID_USER + 0x10)
#define ID_TEXT_OK          (GUI_ID_USER + 0x11)
#define ID_TEXT_UPDATE        (GUI_ID_USER + 0x12)

#define ID_TEXT_T_NAME    (GUI_ID_USER + 0x20)
#define ID_TEXT_NAME    (GUI_ID_USER + 0x21)

#define ID_TEXT_T_VERSION    (GUI_ID_USER + 0x30)
#define ID_TEXT_VERSION    (GUI_ID_USER + 0x31)

#define ID_TEXT_T_DESC    (GUI_ID_USER + 0x40)
#define ID_TEXT_DESC    (GUI_ID_USER + 0x41)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320, 0,  0x0, 0},
  { BUTTON_CreateIndirect,  0, ID_BUTTON_0,            240, 320,   1,   1, 0,  0x0, 0},
  
  { TEXT_CreateIndirect,    0, ID_TEXT_TITLE,            0,   0, 240,  44, 0,  0x0, 0},
  
  { IMAGE_CreateIndirect,    0, ID_IMAGE_LEVEL,        180,  24,  20,  20, 0,  0x0, 0},
  
  
  { TEXT_CreateIndirect, 0, ID_TEXT_T_NAME, 0,  44, 240, 20, 0,  0x00, 0},
  { TEXT_CreateIndirect, 0, ID_TEXT_NAME,   0,  64, 240, 40, 0,  0x00, 0},
  
  { TEXT_CreateIndirect, 0, ID_TEXT_T_VERSION, 0, 104, 240, 20, 0,  0x00, 0},
  { TEXT_CreateIndirect, 0, ID_TEXT_VERSION,   0, 124, 240, 20, 0,  0x00, 0},
  
  { TEXT_CreateIndirect, 0, ID_TEXT_T_DESC, 0,  144, 240, 20, 0,  0x00, 0},
  { TEXT_CreateIndirect, 0, ID_TEXT_DESC,   0,  164, 240, 320-30-44-164, 0,  0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_NONE,             0, 320-30, 80,  30, 0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_OK,              80, 320-30, 80,  30, 0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_UPDATE,           160, 320-30, 80,  30, 0,  0x0, 0},
};

static image_dta_st image_dta_bat;
static uint32_t btn;

static uint8_t* load_src(const char* name,unsigned long *size)
{
  FRESULT res;
  FIL fp;
  UINT br;
  uint8_t* src_p = 0;
  char path[128];

  snprintf(path, sizeof(path), "0:/icon/%s.dta", name);

  res = f_open(&fp, path, FA_READ);
  if(res != FR_OK)
  {
    goto source_load_font_exit;
  }

  *size = f_size(&fp);

  src_p = (uint8_t*)pvPortMalloc(*size);

  if(src_p == 0)
    goto source_load_font_close;

  res = f_read(&fp, src_p, f_size(&fp), &br);
  if((res != FR_OK) || (br != f_size(&fp)))
  {
    goto source_load_font_free;
  }

  goto source_load_font_close;

source_load_font_free:
  vPortFree(src_p);
  src_p = 0;

source_load_font_close:
  f_close(&fp);

source_load_font_exit:

  return src_p;
}

static void load_image_dta_bat(void)
{
  if(image_dta_bat.p)
    vPortFree((void *)image_dta_bat.p);

  battery_status_et status = battery_get_status();
  if(status == BAT_STA_CHARGE)
  { 
    image_dta_bat.p = load_src("battery_charging",&image_dta_bat.size);
  }
  else
  {
    uint8_t lvl = battery_get_level();

    if(lvl < 1)
      image_dta_bat.p = load_src("battery_l0",&image_dta_bat.size);//0
    else if(lvl < 25)
      image_dta_bat.p = load_src("battery_l1",&image_dta_bat.size);//1~24
    else if(lvl < 50)
      image_dta_bat.p = load_src("battery_l2",&image_dta_bat.size);//25~49
    else if(lvl < 75)
      image_dta_bat.p = load_src("battery_l3",&image_dta_bat.size);//50~74
    else if(lvl <= 100)
      image_dta_bat.p = load_src("battery_l4",&image_dta_bat.size);//75~100
  }
}

static test_config_info_st *config_info_phase(const config_file_st *config_file)
{
  cJSON *json;
  test_config_info_st *config_info;
  
  if(memcmp(config_file->file.type,"json",4) != 0)
  {
    config_info = 0;
    goto __exit;
  }

  config_info = pvPortMalloc(sizeof(test_config_info_st));
  if(config_info == 0)
  {
    goto __exit;
  }
  
  memset(config_info,0,sizeof(test_config_info_st));
  
  json = cJSON_Parse((char *)config_file->file.data);
  if(json == NULL)
  {
    goto __exit;
  }
  
  cJSON *j_name = cJSON_GetObjectItem(json,json_obj_name);
  if(j_name != NULL)
  {
    snprintf(config_info->name,sizeof(config_info->name),"%s",j_name->valuestring);
  }
  
  cJSON *j_desc = cJSON_GetObjectItem(json,json_obj_desc);
  if(j_desc != NULL)
  {
    snprintf(config_info->desc,sizeof(config_info->desc),"%s",j_desc->valuestring);
  }
  
  cJSON *j_version = cJSON_GetObjectItem(json,json_obj_version);
  if(j_version != NULL)
  {
    config_info->version = j_version->valueint;
  }

  cJSON_Delete(json);

__exit:

  return config_info;
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
      hItem = pMsg->hWin;
      WINDOW_SetBkColor(hItem, GUI_MAKE_COLOR(0));

      load_image_dta_bat();
      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_LEVEL);
      IMAGE_SetDTA(hItem,image_dta_bat.p,image_dta_bat.size);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_TITLE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_BOTTOM);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "配置文件");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_T_NAME);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0x8a8c8e));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0000FF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "名称");
      
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_T_VERSION);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0x8a8c8e));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0000FF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "版本");
      
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_T_DESC);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0x8a8c8e));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0000FF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "描述");
      
      test_config_info_st *config_info = config_info_phase(module->config_file);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_NAME);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      if(config_info)
      {
        snprintf(str,sizeof(str),"%s",config_info->name);
        TEXT_SetText(hItem, str);
      }
      else
      {
        TEXT_SetText(hItem, "");
      }
      
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_VERSION);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      if(config_info)
      {
        snprintf(str,sizeof(str),"%u",config_info->version);
        TEXT_SetText(hItem, str);
      }
      else
      {
        TEXT_SetText(hItem, "");
      }
      
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_DESC);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0x000000));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetWrapMode(hItem, GUI_WRAPMODE_CHAR);
      TEXT_SetFont(hItem, FONT_DEF);
      if(config_info)
      {
        snprintf(str,sizeof(str),"%s",config_info->desc);
        TEXT_SetText(hItem, str);
      }
      else
      {
        TEXT_SetText(hItem, "");
      }

      vPortFree(config_info);
      
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OK);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "开始");
      
      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_UPDATE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem,GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "更新");
      
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
      switch(((WM_KEY_INFO*)(pMsg->Data.p))->Key)
      {
        case UI_KEY(OK,ONCLICK):
        {
          btn = ID_TEXT_OK;
          GUI_EndDialog(pMsg->hWin, 0);
          break;
        }
        case UI_KEY(RIGHT,ONCLICK):
        {
          btn = ID_TEXT_UPDATE;
          GUI_EndDialog(pMsg->hWin, 0);
          break;
        }
      }
      break;
    }
    case UI_MSG_BATTERY:
    {
      load_image_dta_bat();
      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_LEVEL);
      IMAGE_SetDTA(hItem,image_dta_bat.p,image_dta_bat.size);
      break;
    }
    case WM_DELETE:
    {
      if(image_dta_bat.p)
        vPortFree((void *)image_dta_bat.p);
      image_dta_bat.p = 0;
      
      if(btn == ID_TEXT_OK)
        test_start(module);
      else
      {
        ui_config_update_open();
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

void ui_config_open(void)
{
  GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
}