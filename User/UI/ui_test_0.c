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
#include "analog_speaker.h"

#define FONT_DEF      (font_ext)
#define PROG_FONT_DEF (GUI_FONT_20_ASCII)

#define ID_WINDOW_0         (GUI_ID_USER + 0x00)
#define ID_BUTTON_0         (GUI_ID_USER + 0x01)
#define ID_TEXT_TITLE       (GUI_ID_USER + 0x02)
#define ID_IMAGE_LEVEL      (GUI_ID_USER + 0x03)
#define ID_IMAGE_SPEAKER    (GUI_ID_USER + 0x04)
#define ID_TEXT_BOTTOM      (GUI_ID_USER + 0x05)

#define ID_TEXT_WAIT        (GUI_ID_USER + 0x0A)

#define ID_TEXT_PROX        (GUI_ID_USER + 0x10)
#define ID_PROGBAR_PROX_VAL (GUI_ID_USER + 0x12)

#define ID_TEXT_ALS         (GUI_ID_USER + 0x20)
#define ID_PROGBAR_ALS_VAL  (GUI_ID_USER + 0x22)

#define ID_TEXT_MIC         (GUI_ID_USER + 0x30)
#define ID_PROGBAR_MIC_VAL  (GUI_ID_USER + 0x32)

#define ID_TEXT_OS          (GUI_ID_USER + 0x40)
#define ID_TEXT_OS_RES      (GUI_ID_USER + 0x41)

#define ID_TEXT_CAM         (GUI_ID_USER + 0x50)
#define ID_TEXT_CAM_RES     (GUI_ID_USER + 0x51)

#define ID_LISTVIEW_CURRENT (GUI_ID_USER + 0x60)

#define ID_TEXT_SPK_P       (GUI_ID_USER + 0x70)
#define ID_TEXT_SPK_N       (GUI_ID_USER + 0x71)

#define ID_TEXT_BTN_NONE    (GUI_ID_USER + 0xA0)
#define ID_TEXT_BTN_MENU    (GUI_ID_USER + 0xA1)
#define ID_TEXT_BTN_MIC     (GUI_ID_USER + 0xA2)

#define ITEM_SPACE   (5)
#define ITEM1_OFFSET (44 + ITEM_SPACE)
#define TEXT_HEIGHT  (20)
#define TEXT_WIDTH   (60)
#define PROG_HEIGHT  (20)

#define ITEM2_OFFSET (ITEM1_OFFSET + TEXT_HEIGHT + ITEM_SPACE)
#define ITEM3_OFFSET (ITEM2_OFFSET + TEXT_HEIGHT + ITEM_SPACE)
#define ITEM4_OFFSET (ITEM3_OFFSET + TEXT_HEIGHT + ITEM_SPACE)
#define ITEM5_OFFSET (ITEM4_OFFSET + TEXT_HEIGHT + ITEM_SPACE)
#define ITEM6_OFFSET (ITEM5_OFFSET + TEXT_HEIGHT + ITEM_SPACE)
#define ITEM7_OFFSET (ITEM6_OFFSET + TEXT_HEIGHT + ITEM_SPACE)
#define ITEM8_OFFSET (ITEM7_OFFSET + TEXT_HEIGHT + ITEM_SPACE)

static const GUI_WIDGET_CREATE_INFO _aDialogCreate[] = {
  { WINDOW_CreateIndirect,  0, ID_WINDOW_0,              0,   0, 240, 320,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_TITLE,            0,   0, 240,  44,   0,  0x0, 0},
  { IMAGE_CreateIndirect,   0, ID_IMAGE_LEVEL,         180,  24,  20,  20,   0,  0x0, 0},
  { IMAGE_CreateIndirect,   0, ID_IMAGE_SPEAKER,        40,  24,  20,  20,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BOTTOM,           0, 320 - 30, 240,  30,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_WAIT,             0,  42, 240, ITEM7_OFFSET - 42,  0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_PROX,             0,  ITEM1_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { PROGBAR_CreateIndirect, 0, ID_PROGBAR_PROX_VAL,  TEXT_WIDTH,  ITEM1_OFFSET, 220 - TEXT_WIDTH,  PROG_HEIGHT,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_ALS,              0,  ITEM2_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { PROGBAR_CreateIndirect, 0, ID_PROGBAR_ALS_VAL,    TEXT_WIDTH, ITEM2_OFFSET, 220 - TEXT_WIDTH,  PROG_HEIGHT, 0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_MIC,              0, ITEM3_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { PROGBAR_CreateIndirect, 0, ID_PROGBAR_MIC_VAL,    TEXT_WIDTH, ITEM3_OFFSET, 220 - TEXT_WIDTH,  PROG_HEIGHT, 0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_OS,               0, ITEM4_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_OS_RES,      TEXT_WIDTH, ITEM4_OFFSET, (220 - TEXT_WIDTH) / 2,  TEXT_HEIGHT, 0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_CAM,              0, ITEM5_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_CAM_RES,     TEXT_WIDTH, ITEM5_OFFSET, (220 - TEXT_WIDTH) / 2,  TEXT_HEIGHT,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_SPK_P,       TEXT_WIDTH + (220 - TEXT_WIDTH) / 2, ITEM4_OFFSET, (220 - TEXT_WIDTH) / 2,  TEXT_HEIGHT,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_SPK_N,       TEXT_WIDTH + (220 - TEXT_WIDTH) / 2, ITEM5_OFFSET, (220 - TEXT_WIDTH) / 2,  TEXT_HEIGHT,   0,  0x0, 0},

  { LISTVIEW_CreateIndirect,0, ID_LISTVIEW_CURRENT,      0, ITEM7_OFFSET, 240,  88, 0, 0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BTN_NONE,        20, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BTN_MENU,        90, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BTN_MIC,        160, 320 - 28,  60,  28,   0,  0x0, 0},
};

static long htimer;
static const char* title;
static test_0_st* test;
static uint8_t stage, mic_record;
static uint8_t error_vi[9];

static void play_warning(void)
{
  wav_play(audio_file_warning);
}

static void vi_updata(WM_HWIN listview)
{
  char str[32];
  test_0_config_st* config;

  config = test->config;
  for(int i = 0;i < 8;i++)
  {
    if(test->config->power[i].en == 0) continue;

    if(test->ma_is_update(i))
    {
      if(i & 0x01)
      {
        snprintf(str, sizeof(str), "%.0f", test->mv_get(i));
        LISTVIEW_SetItemText(listview, 2, i / 2, str);

        snprintf(str, sizeof(str), "%.2f", test->ma_get(i));
        LISTVIEW_SetItemText(listview, 3, i / 2, str);

        if(test->ma_get(i) > config->power[i].i_max || test->ma_get(i) < config->power[i].i_min)
        {
          LISTVIEW_SetItemTextColor(listview, 3, i / 2, LISTVIEW_CI_UNSEL, config->power[i].color_out);
          LISTVIEW_SetItemTextColor(listview, 3, i / 2, LISTVIEW_CI_SEL, config->power[i].color_out);
          error_vi[i] |= 1;
        }
        else
        {
          LISTVIEW_SetItemTextColor(listview, 3, i / 2, LISTVIEW_CI_UNSEL, config->power[i].color_in);
          LISTVIEW_SetItemTextColor(listview, 3, i / 2, LISTVIEW_CI_SEL, config->power[i].color_in);
          error_vi[i] = 0;
        }
      }
      else
      {
        snprintf(str, sizeof(str), "%.0f", test->mv_get(i));
        LISTVIEW_SetItemText(listview, 0, i / 2, str);

        snprintf(str, sizeof(str), "%.2f", test->ma_get(i));
        LISTVIEW_SetItemText(listview, 1, i / 2, str);

        if(test->ma_get(i) > config->power[i].i_max || test->ma_get(i) < config->power[i].i_min)
        {
          LISTVIEW_SetItemTextColor(listview, 1, i / 2, LISTVIEW_CI_UNSEL, config->power[i].color_out);
          LISTVIEW_SetItemTextColor(listview, 1, i / 2, LISTVIEW_CI_SEL, config->power[i].color_out);
          error_vi[i] |= 1;
        }
        else
        {
          LISTVIEW_SetItemTextColor(listview, 1, i / 2, LISTVIEW_CI_UNSEL, config->power[i].color_in);
          LISTVIEW_SetItemTextColor(listview, 1, i / 2, LISTVIEW_CI_SEL, config->power[i].color_in);
          error_vi[i] = 0;
        }
      }

      if((stage == TEST_STAGE_FUNC) && error_vi[i] == 1)
      {
        play_warning();
        error_vi[i] = 3;
      }
    }
  }

  if(test->ma_is_update(12))
  {
    snprintf(str, sizeof(str), "%.0f", test->mv_get(12));
    LISTVIEW_SetItemText(listview, 0, 4, str);

    snprintf(str, sizeof(str), "%.2f", test->ma_get(12));
    LISTVIEW_SetItemText(listview, 1, 4, str);

    if(test->ma_get(12) > config->mvi.i_max || test->ma_get(12) < config->mvi.i_min)
    {
      LISTVIEW_SetItemTextColor(listview, 1, 4, LISTVIEW_CI_UNSEL, config->mvi.color_out);
      LISTVIEW_SetItemTextColor(listview, 1, 4, LISTVIEW_CI_SEL, config->mvi.color_out);
      error_vi[8] |= 1;
      if(error_vi[8] == 1)
      {
        play_warning();
        error_vi[8] = 3;
      }
    }
    else
    {
      LISTVIEW_SetItemTextColor(listview, 1, 4, LISTVIEW_CI_UNSEL, config->mvi.color_in);
      LISTVIEW_SetItemTextColor(listview, 1, 4, LISTVIEW_CI_SEL, config->mvi.color_in);
      error_vi[8] = 0;
    }
  }
}

static void update_func_test_result(WM_HWIN win)
{
  WM_HWIN hItem;
  char str_buf[128];

  if(test->prox_is_rdy() == 0)
  {
    hItem = WM_GetDialogItem(win, ID_PROGBAR_PROX_VAL);
    PROGBAR_SetValue(hItem, 0);
    PROGBAR_SetText(hItem, "X");
    hItem = WM_GetDialogItem(win, ID_TEXT_PROX);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFF0000));
  }
  else
  {
    uint8_t prox = test->prox_get();
    hItem = WM_GetDialogItem(win, ID_PROGBAR_PROX_VAL);
    PROGBAR_SetText(hItem, 0);
    PROGBAR_SetValue(hItem, prox);
    hItem = WM_GetDialogItem(win, ID_TEXT_PROX);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000FF));
  }

  if(test->als_is_rdy() == 0)
  {
    hItem = WM_GetDialogItem(win, ID_PROGBAR_ALS_VAL);
    PROGBAR_SetValue(hItem, 0);
    PROGBAR_SetText(hItem, "X");
    hItem = WM_GetDialogItem(win, ID_TEXT_ALS);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFF0000));
  }
  else
  {
    uint8_t als = test->als_get();
    hItem = WM_GetDialogItem(win, ID_PROGBAR_ALS_VAL);
    PROGBAR_SetText(hItem, 0);
    PROGBAR_SetValue(hItem, als);
    hItem = WM_GetDialogItem(win, ID_TEXT_ALS);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000FF));
  }

  hItem = WM_GetDialogItem(win, ID_PROGBAR_MIC_VAL);
  if(mic_record == 0)
  {
    PROGBAR_SetValue(hItem, test->mic_get());
  }
  else
  {
    uint8_t prog = test->mic_record_get_progress();
    PROGBAR_SetValue(hItem, prog);
    snprintf(str_buf, sizeof(str_buf), "REC %u%%", prog);
    PROGBAR_SetText(hItem, str_buf);
  }

  uint8_t cam_rdy = test->cam_is_rdy();
  hItem = WM_GetDialogItem(win, ID_TEXT_CAM_RES);
  TEXT_SetText(hItem, cam_rdy ? " 有" : " 无");
  if(cam_rdy == 0)
  {
    hItem = WM_GetDialogItem(win, ID_TEXT_CAM);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFF0000));
  }
  else
  {
    hItem = WM_GetDialogItem(win, ID_TEXT_CAM);
    TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000FF));
  }

  uint8_t spk_pn = test->spk_pn_get();

  hItem = WM_GetDialogItem(win, ID_TEXT_SPK_P);
  TEXT_SetBkColor(hItem, (spk_pn & 1) == 0 ? GUI_MAKE_COLOR(0xFF0000) : GUI_MAKE_COLOR(0x00A000));

  hItem = WM_GetDialogItem(win, ID_TEXT_SPK_N);
  TEXT_SetBkColor(hItem, (spk_pn & 2) == 0 ? GUI_MAKE_COLOR(0xFF0000) : GUI_MAKE_COLOR(0x00A000));
}

static void _cbDialog(WM_MESSAGE* pMsg) {
  WM_HWIN                hItem;
  int                    NCode;
  int                    Id;
  char                   str_buf[128];
  switch(pMsg->MsgId)
  {
    case WM_INIT_DIALOG:
    {
      hItem = pMsg->hWin;
      WINDOW_SetBkColor(hItem, GUI_MAKE_COLOR(0));

      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_LEVEL);
      ui_battery_level_icon_updata(hItem, 0);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_SPEAKER);
      IMAGE_SetDTA(hItem, res_image_dta_speaker, sizeof(res_image_dta_speaker));
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_TITLE);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_BOTTOM);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      if(title)
        TEXT_SetText(hItem, title);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BOTTOM);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_WAIT);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "未检测到模组");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PROX);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_BOTTOM);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000FF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "接近");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_PROX_VAL);
      PROGBAR_SetFont(hItem, PROG_FONT_DEF);
      PROGBAR_SetMinMax(hItem, 0, 100);
      PROGBAR_SetValue(hItem, 0);
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_ALS);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_BOTTOM);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000FF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "环境光");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_ALS_VAL);
      PROGBAR_SetFont(hItem, PROG_FONT_DEF);
      PROGBAR_SetMinMax(hItem, 0, 100);
      PROGBAR_SetValue(hItem, 0);
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_MIC);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_BOTTOM);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000FF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "麦克风");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_MIC_VAL);
      PROGBAR_SetFont(hItem, PROG_FONT_DEF);
      PROGBAR_SetMinMax(hItem, 0, 100);
      PROGBAR_SetValue(hItem, 0);

      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OS);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_BOTTOM);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000FF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "开短路：");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OS_RES);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_BOTTOM);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFFFF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CAM);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_BOTTOM);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000FF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "摄像头：");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CAM_RES);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_BOTTOM);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFFFF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_P);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFFFF));
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xFF0000));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "喇叭-P");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_N);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFFFF));
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xFF0000));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "喇叭-N");
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_CURRENT);
      LISTVIEW_SetFont(hItem, GUI_FONT_13_ASCII);
      HEADER_SetFont(LISTVIEW_GetHeader(hItem), GUI_FONT_13B_ASCII);
      LISTVIEW_AddColumn(hItem, 60, "V (mV)", GUI_TA_HCENTER | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 60, "I (mA)", GUI_TA_HCENTER | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 60, "V (mV)", GUI_TA_HCENTER | GUI_TA_VCENTER);
      LISTVIEW_AddColumn(hItem, 60, "I (mA)", GUI_TA_HCENTER | GUI_TA_VCENTER);
      LISTVIEW_SetColumnWidth(hItem, LISTVIEW_ALL_ITEMS, 30);
      LISTVIEW_SetHeaderHeight(hItem, 16);
      LISTVIEW_SetGridVis(hItem, 1);

      for(int i = 0;i < 4;i++)
      {
        LISTVIEW_AddRow(hItem, 0);

        snprintf(str_buf, sizeof(str_buf), "%d", i);
        LISTVIEW_SetItemText(hItem, 0, i, "0");
        LISTVIEW_SetItemText(hItem, 1, i, "0");

        snprintf(str_buf, sizeof(str_buf), "%d", i + 6);
        LISTVIEW_SetItemText(hItem, 2, i, "0");
        LISTVIEW_SetItemText(hItem, 3, i, "0");
      }
      LISTVIEW_AddRow(hItem, 0);
      LISTVIEW_SetItemText(hItem, 0, 6, "0");
      LISTVIEW_SetItemText(hItem, 1, 6, "0");

      WM_EnableMemdev(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BTN_NONE);
      TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BTN_MENU);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "菜单");

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BTN_MIC);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, "录/播");


      htimer = WM_CreateTimer(pMsg->hWin, 0, 100, 0);

      WM_SetFocus(pMsg->hWin);
      break;
    }
    case WM_KEY:
    {
      pwdn_watchdog_reload();
      switch(((WM_KEY_INFO*)(pMsg->Data.p))->Key)
      {
        /*case UI_KEY(UP, ONCLICK):
        {
          break;
        }
        case UI_KEY(DOWN, ONCLICK):
        {
          break;
        }
        case UI_KEY(LEFT,ONCLICK):
        {
          break;
        }*/
        case UI_KEY(RIGHT, ONCLICK):
        {
          if(stage == TEST_STAGE_FUNC)
            test->mic_record_play();
          break;
        }
        case UI_KEY(RIGHT, LONGPRESS):
        {
          if(stage == TEST_STAGE_FUNC)
          {
            if(test->mic_record_start() == 0)
              mic_record = 1;
          }
          break;
        }
        case UI_KEY(RIGHT, RELEASE):
        {
          if(stage == TEST_STAGE_FUNC)
          {
            if(mic_record)
            {
              test->mic_record_stop();
              hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_MIC_VAL);
              PROGBAR_SetText(hItem, 0);
              mic_record = 0;
              test->mic_record_play();
            }
          }
          break;
        }
        case UI_KEY(OK, ONCLICK):
        {
          ui_menu_open(pMsg->hWin);
          break;
        }
      }
      break;
    }
    case WM_TIMER:
    {
      WM_RestartTimer(htimer, 100);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_IMAGE_SPEAKER);
      if(analog_speaker_is_busy())
      {
        WM_ShowWindow(hItem);
      }
      else
      {
        WM_HideWindow(hItem);
      }

      if(stage == TEST_STAGE_FUNC)
      {
        update_func_test_result(pMsg->hWin);
      }
      else
      {
        if(mic_record)
        {
          mic_record = 0;
          test->mic_record_stop();
          hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_MIC_VAL);
          PROGBAR_SetText(hItem, 0);
        }
      }

      hItem = WM_GetDialogItem(pMsg->hWin, ID_LISTVIEW_CURRENT);
      vi_updata(hItem);

      if(stage != test->stage_get())
      {
        pwdn_watchdog_reload();
        stage = test->stage_get();
        switch(stage)
        {
          case TEST_STAGE_WAIT:
          {
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PROX);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_PROX_VAL);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_ALS);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_ALS_VAL);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_MIC);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_MIC_VAL);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OS);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OS_RES);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CAM);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CAM_RES);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_P);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_N);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_WAIT);
            TEXT_SetText(hItem, "未检测到模组");
            WM_ShowWindow(hItem);
            break;
          }
          case TEST_STAGE_OS:
          {
            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PROX);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_PROX_VAL);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_ALS);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_ALS_VAL);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_MIC);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_MIC_VAL);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OS);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OS_RES);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CAM);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CAM_RES);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_P);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_N);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_WAIT);
            TEXT_SetText(hItem, "开短路测试...");
            WM_ShowWindow(hItem);
            break;
          }
          case TEST_STAGE_FUNC:
          {
            update_func_test_result(pMsg->hWin);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_WAIT);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PROX);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_PROX_VAL);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_ALS);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_ALS_VAL);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_MIC);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_PROGBAR_MIC_VAL);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OS);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OS_RES);
            /*if(test_0_os_result_get() == 0)
            {
              TEXT_SetTextColor(hItem,GUI_MAKE_COLOR(0x00FF00));
              TEXT_SetText(hItem," 通过");
            }
            else
            {
              char str_buf[128];
              TEXT_SetTextColor(hItem,GUI_MAKE_COLOR(0xFF0000));
              sprintf(str_buf," %u个异常",test_0_os_result_get());
              TEXT_SetText(hItem,str_buf);
            }*/
            TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x333333));
            TEXT_SetText(hItem, " 未测试");
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CAM);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CAM_RES);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_P);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_N);
            WM_ShowWindow(hItem);
            break;
          }
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
      break;
    }
    default:
    {
      WM_DefaultProc(pMsg);
      break;
    }
  }
}

void ui_test_0_open(test_0_st* _test_, const char* _title_)
{
  if(_test_)
  {
    test = _test_;
    title = _title_;
    stage = 0xff;

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
}
