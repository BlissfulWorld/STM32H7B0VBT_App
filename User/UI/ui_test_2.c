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
#define PROG_FONT_DEF (GUI_DEFAULT_FONT)

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

#define ID_LISTVIEW_CURRENT (GUI_ID_USER + 0x60)

#define ID_TEXT_SPK_P       (GUI_ID_USER + 0x70)
#define ID_TEXT_SPK_N       (GUI_ID_USER + 0x71)
#define ID_TEXT_CATHC       (GUI_ID_USER + 0x72)
#define ID_TEXT_YOGI        (GUI_ID_USER + 0x73)

#define ID_TEXT_BTN_SPKRAMP (GUI_ID_USER + 0xA0)
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

  { TEXT_CreateIndirect,    0, ID_TEXT_WAIT,             0,  42, 240, ITEM7_OFFSET - 42,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_PROX,             0,  ITEM1_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { PROGBAR_CreateIndirect, 0, ID_PROGBAR_PROX_VAL,  TEXT_WIDTH,  ITEM1_OFFSET, 220 - TEXT_WIDTH,  PROG_HEIGHT,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_ALS,              0,  ITEM2_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { PROGBAR_CreateIndirect, 0, ID_PROGBAR_ALS_VAL,    TEXT_WIDTH, ITEM2_OFFSET, 220 - TEXT_WIDTH,  PROG_HEIGHT,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_MIC,              0, ITEM3_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { PROGBAR_CreateIndirect, 0, ID_PROGBAR_MIC_VAL,    TEXT_WIDTH, ITEM3_OFFSET, 220 - TEXT_WIDTH,  PROG_HEIGHT,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_OS,               0, ITEM4_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_OS_RES,      TEXT_WIDTH, ITEM4_OFFSET, 220 - TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_SPK_P,       0, ITEM5_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_SPK_N,       240 / 3, ITEM5_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_CATHC,       240 / 3 * 2, ITEM5_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT, 0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_YOGI,        0, ITEM6_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},

  { LISTVIEW_CreateIndirect,0, ID_LISTVIEW_CURRENT,      0, ITEM7_OFFSET, 240,  88,   0, 0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BTN_SPKRAMP,     20, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BTN_MENU,        90, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BTN_MIC,        160, 320 - 28,  60,  28,   0,  0x0, 0},
};

static long htimer;
static const char* title;
static test_2_st* test;
static uint8_t stage, mic_record;
static uint8_t error_vi[9];

static void play_warning(void)
{
  wav_play(audio_file_warning);
}

static void menu_close_cb(void* p)
{
  test_start(module);
}

static void vi_updata(WM_HWIN listview)
{
  char str[32];
  test_2_config_st* config;

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
    PROGBAR_SetValue(hItem, 1);
    PROGBAR_SetText(hItem, "X");
  }
  else
  {
    uint8_t prox = test->prox_get();
    hItem = WM_GetDialogItem(win, ID_PROGBAR_PROX_VAL);
    PROGBAR_SetText(hItem, 0);
    PROGBAR_SetValue(hItem, prox);
  }

  if(test->als_is_rdy() == 0)
  {
    hItem = WM_GetDialogItem(win, ID_PROGBAR_ALS_VAL);
    PROGBAR_SetValue(hItem, 1);
    PROGBAR_SetText(hItem, "X");
  }
  else
  {
    uint8_t als = test->als_get();
    hItem = WM_GetDialogItem(win, ID_PROGBAR_ALS_VAL);
    PROGBAR_SetText(hItem, 0);
    PROGBAR_SetValue(hItem, als);
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

  uint8_t spk_pn = test->spkramp_test_res();

  hItem = WM_GetDialogItem(win, ID_TEXT_SPK_P);
  TEXT_SetBkColor(hItem, (spk_pn & 1) == 0 ? GUI_MAKE_COLOR(0xFF0000) : GUI_MAKE_COLOR(0x00A000));

  hItem = WM_GetDialogItem(win, ID_TEXT_SPK_N);
  TEXT_SetBkColor(hItem, (spk_pn & 2) == 0 ? GUI_MAKE_COLOR(0xFF0000) : GUI_MAKE_COLOR(0x00A000));

  uint8_t res = test->codec_aout_to_hac_test_res();

  hItem = WM_GetDialogItem(win, ID_TEXT_CATHC);
  TEXT_SetBkColor(hItem, res ? GUI_MAKE_COLOR(0x00A000) : GUI_MAKE_COLOR(0xFF0000));

  hItem = WM_GetDialogItem(win, ID_TEXT_YOGI);
  TEXT_SetBkColor(hItem, test->yogi_is_rdy() ? GUI_MAKE_COLOR(0x00A000) : GUI_MAKE_COLOR(0xFF0000));
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
      TEXT_SetText(hItem, ui_str.ui_test_2.info_no_found_module);
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_PROX);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_BOTTOM);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000FF));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_test_2.prox);
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
      TEXT_SetText(hItem, ui_str.ui_test_2.als);
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
      TEXT_SetText(hItem, ui_str.ui_test_2.mic);
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
      TEXT_SetText(hItem, ui_str.ui_test_2.ostest);
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_OS_RES);
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
      TEXT_SetText(hItem, ui_str.ui_test_2.spk_p);
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_N);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFFFF));
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xFF0000));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_test_2.spk_n);
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CATHC);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFFFF));
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xFF0000));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_test_2.cathc);
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_YOGI);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_VCENTER);
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFFFFFFFF));
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xFF0000));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_test_2.yogi);
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

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BTN_SPKRAMP);
      TEXT_SetTextAlign(hItem, GUI_TA_RIGHT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_test_2.btn_speaker_play);
      WM_HideWindow(hItem);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BTN_MENU);
      TEXT_SetTextAlign(hItem, GUI_TA_HCENTER | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_test_2.btn_menu);

      hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BTN_MIC);
      TEXT_SetTextAlign(hItem, GUI_TA_LEFT | GUI_TA_TOP);
      TEXT_SetBkColor(hItem, GUI_MAKE_COLOR(0xabc88b));
      TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x0));
      TEXT_SetFont(hItem, FONT_DEF);
      TEXT_SetText(hItem, ui_str.ui_test_2.btn_record_play);

      htimer = WM_CreateTimer(pMsg->hWin, 0, 100, 0);

      WM_SetFocus(pMsg->hWin);
      break;
    }
    case WM_KEY:
    {
      pwdn_watchdog_reload();
      switch(((WM_KEY_INFO*)(pMsg->Data.p))->Key)
      {
        case UI_KEY(UP, ONCLICK):
        {
          if(stage == TEST_STAGE_FUNC)
          {
            const float* data;
            uint8_t len;
            data = test->os_test_data_get(&len);
            if(len)
              ui_os_test_result_open(title, pMsg->hWin, test->config->ost, data, len);
          }
          break;
        }
        /*case UI_KEY(DOWN, ONCLICK):
        {
          break;
        }*/
        case UI_KEY(LEFT, ONCLICK):
        {
          hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BTN_SPKRAMP);
          if(WM_IsVisible(hItem))
          {
            test->spkramp_play();
          }
          break;
        }
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
          if(!analog_speaker_is_busy())
            GUI_EndDialog(pMsg->hWin, 0);
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

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_P);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_N);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CATHC);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_YOGI);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BTN_SPKRAMP);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_WAIT);
            TEXT_SetText(hItem, ui_str.ui_test_2.info_no_found_module);
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

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_P);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_N);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CATHC);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_YOGI);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BTN_SPKRAMP);
            WM_HideWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_WAIT);
            TEXT_SetText(hItem, ui_str.ui_test_2.info_doing_ostest);
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
            uint8_t len;
            test->os_test_data_get(&len);
            if(len)
            {
              if(test->os_test_result_get() == 0)
              {
                TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x00FF00));
                sprintf(str_buf, " %s %s", ui_str.ui_test_2.ostest_result_normal, ui_str.ui_test_2.key_up_check);
                TEXT_SetText(hItem, str_buf);
              }
              else
              {
                char str_buf[128];
                TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0xFF0000));
                sprintf(str_buf, " %u %s", test->os_test_result_get(), ui_str.ui_test_2.key_up_check);
                TEXT_SetText(hItem, str_buf);
              }
            }
            else
            {
              TEXT_SetTextColor(hItem, GUI_MAKE_COLOR(0x333333));
              TEXT_SetText(hItem, ui_str.ui_test_2.no_ostest);
            }
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_P);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_SPK_N);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_CATHC);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_YOGI);
            WM_ShowWindow(hItem);

            hItem = WM_GetDialogItem(pMsg->hWin, ID_TEXT_BTN_SPKRAMP);
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
      test_stop(module);
      ui_menu_open(menu_close_cb, 0);
      break;
    }
    default:
    {
      WM_DefaultProc(pMsg);
      break;
    }
  }
}

void ui_test_2_open(test_2_st* _test_, const char* _title_)
{
  if(_test_)
  {
    test = _test_;
    title = _title_;
    stage = 0xff;

    GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  }
}
