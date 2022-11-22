#include "ui.h"
#include "DIALOG.h"
#include "stdio.h"
#include "test_4.h"
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

#define ID_TEXT_MIC1        (GUI_ID_USER + 0x20)
#define ID_TEXT_MIC1_RES    (GUI_ID_USER + 0x21)

#define ID_TEXT_MIC         (GUI_ID_USER + 0x30)
#define ID_PROGBAR_MIC_VAL  (GUI_ID_USER + 0x32)

#define ID_TEXT_OS          (GUI_ID_USER + 0x40)
#define ID_TEXT_OS_RES      (GUI_ID_USER + 0x41)

#define ID_LISTBOX_OS_RES   (GUI_ID_USER + 0x42)

#define ID_LISTVIEW_CURRENT (GUI_ID_USER + 0x60)


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

  { TEXT_CreateIndirect,    0, ID_TEXT_WAIT,             0,  42, 240, ITEM7_OFFSET - 42,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_MIC1,              0, ITEM1_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_MIC1_RES,     TEXT_WIDTH, ITEM1_OFFSET, 220 - TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_MIC,              0, ITEM2_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { PROGBAR_CreateIndirect, 0, ID_PROGBAR_MIC_VAL,    TEXT_WIDTH, ITEM2_OFFSET, 220 - TEXT_WIDTH,  PROG_HEIGHT,   0,  0x0, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_OS,               0, ITEM3_OFFSET, TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_OS_RES,      TEXT_WIDTH, ITEM3_OFFSET, 220 - TEXT_WIDTH,  TEXT_HEIGHT,   0,  0x0, 0},

  { LISTBOX_CreateIndirect, 0, ID_LISTBOX_OS_RES,   TEXT_WIDTH, ITEM4_OFFSET, 220 - TEXT_WIDTH,  320 - ITEM4_OFFSET - 88 - 30,   0, 0x00, 0},

  { LISTVIEW_CreateIndirect,0, ID_LISTVIEW_CURRENT,      0, ITEM7_OFFSET, 240,  88,   0, 0x00, 0},

  { TEXT_CreateIndirect,    0, ID_TEXT_BTN_NONE,        20, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BTN_MENU,        90, 320 - 28,  60,  28,   0,  0x0, 0},
  { TEXT_CreateIndirect,    0, ID_TEXT_BTN_MIC,        160, 320 - 28,  60,  28,   0,  0x0, 0},
};

static long htimer;
static const char* title;
static test_4_st* test;
static uint8_t stage, mic_record;
static uint8_t error_vi[9];
static uint8_t vdd_is_exception;

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
  test_4_config_st* config;
  uint32_t column, row;

  config = test->config;
  for(int i = 0;i < 6;i++)
  {
    if(test->config->power[i].en == 0) continue;

    if(test->ma_is_update(i))
    {
      column = i & 0x01 ? 3 : 1;
      row = i / 2;

      snprintf(str, sizeof(str), "%.2f", test->ma_get(i));
      LISTVIEW_SetItemText(listview, column, row, str);

      if(test->ma_get(i) > config->power[i].i_max || test->ma_get(i) < config->power[i].i_min)
      {
        LISTVIEW_SetItemTextColor(listview, column, row, LISTVIEW_CI_UNSEL, config->power[i].color_out);
        LISTVIEW_SetItemTextColor(listview, column, row, LISTVIEW_CI_SEL, config->power[i].color_out);
        error_vi[i] |= 1;
      }
      else
      {
        LISTVIEW_SetItemTextColor(listview, column, row, LISTVIEW_CI_UNSEL, config->power[i].color_in);
        LISTVIEW_SetItemTextColor(listview, column, row, LISTVIEW_CI_SEL, config->power[i].color_in);
        error_vi[i] = 0;
      }

      column--;

      snprintf(str, sizeof(str), "%.0f", test->mv_get(i));
      LISTVIEW_SetItemText(listview, column, row, str);

      if((i < 4 && test->mv_get(i) <= 500) || (i >= 4 && test->mv_get(i) <= 1000))
      {
        vdd_is_exception = 1;
        LISTVIEW_SetItemTextColor(listview, column, row, LISTVIEW_CI_UNSEL, config->power[i].color_out);
        LISTVIEW_SetItemTextColor(listview, column, row, LISTVIEW_CI_SEL, config->power[i].color_out);
      }
      else
      {
        LISTVIEW_SetItemTextColor(listview, column, row, LISTVIEW_CI_UNSEL, config->power[i].color_in);
        LISTVIEW_SetItemTextColor(listview, column, row, LISTVIEW_CI_SEL, config->power[i].color_in);
      }

      if((stage == TEST_STAGE_FUNC) && error_vi[i] == 1)
      {
        play_warning();
        error_vi[i] = 3;
      }
    }
  }

  if(test->ma_is_update(10))
  {
    snprintf(str, sizeof(str), "%.0f", test->mv_get(10));
    LISTVIEW_SetItemText(listview, 0, 3, str);

    snprintf(str, sizeof(str), "%.2f", test->ma_get(10));
    LISTVIEW_SetItemText(listview, 1, 3, str);

    if(test->ma_get(10) > config->mvi.i_max || test->ma_get(10) < config->mvi.i_min)
    {
      LISTVIEW_SetItemTextColor(listview, 1, 3, LISTVIEW_CI_UNSEL, config->mvi.color_out);
      LISTVIEW_SetItemTextColor(listview, 1, 3, LISTVIEW_CI_SEL, config->mvi.color_out);
      error_vi[6] |= 1;
      if((stage == TEST_STAGE_FUNC) && error_vi[6] == 1)
      {
        play_warning();
        error_vi[6] = 3;
      }
    }
    else
    {
      LISTVIEW_SetItemTextColor(listview, 1, 3, LISTVIEW_CI_UNSEL, config->mvi.color_in);
      LISTVIEW_SetItemTextColor(listview, 1, 3, LISTVIEW_CI_SEL, config->mvi.color_in);
      error_vi[6] = 0;
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
      error_vi[7] |= 1;
      if(error_vi[7] == 1)
      {
        play_warning();
        error_vi[7] = 3;
      }
    }
    else
    {
      LISTVIEW_SetItemTextColor(listview, 1, 4, LISTVIEW_CI_UNSEL, config->mvi.color_in);
      LISTVIEW_SetItemTextColor(listview, 1, 4, LISTVIEW_CI_SEL, config->mvi.color_in);
      error_vi[7] = 0;
    }
  }
}

static void wm_init_dialog(WM_HWIN hwin)
{
  WM_HWIN hitem;

  WINDOW_SetBkColor(hwin, GUI_MAKE_COLOR(0));

  hitem = WM_GetDialogItem(hwin, ID_IMAGE_LEVEL);
  ui_battery_level_icon_updata(hitem, 0);

  hitem = WM_GetDialogItem(hwin, ID_IMAGE_SPEAKER);
  IMAGE_SetDTA(hitem, res_image_dta_speaker, sizeof(res_image_dta_speaker));
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_TITLE);
  TEXT_SetTextAlign(hitem, GUI_TA_HCENTER | GUI_TA_BOTTOM);
  TEXT_SetBkColor(hitem, GUI_MAKE_COLOR(0xabc88b));
  TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0x0));
  TEXT_SetFont(hitem, FONT_DEF);
  if(title)
    TEXT_SetText(hitem, title);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_BOTTOM);
  TEXT_SetTextAlign(hitem, GUI_TA_HCENTER | GUI_TA_VCENTER);
  TEXT_SetBkColor(hitem, GUI_MAKE_COLOR(0xabc88b));
  TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0x0));
  TEXT_SetFont(hitem, FONT_DEF);
  TEXT_SetText(hitem, "");

  hitem = WM_GetDialogItem(hwin, ID_TEXT_WAIT);
  TEXT_SetTextAlign(hitem, GUI_TA_HCENTER | GUI_TA_VCENTER);
  TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0xFFFFFF));
  TEXT_SetFont(hitem, FONT_DEF);
  TEXT_SetText(hitem, ui_str.ui_test_4.info_no_found_module);
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_MIC1);
  TEXT_SetTextAlign(hitem, GUI_TA_LEFT | GUI_TA_BOTTOM);
  TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0xFF0000FF));
  TEXT_SetFont(hitem, FONT_DEF);
  TEXT_SetText(hitem, ui_str.ui_test_4.mic1);
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_MIC1_RES);
  TEXT_SetTextAlign(hitem, GUI_TA_LEFT | GUI_TA_BOTTOM);
  TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0xFFFFFFFF));
  TEXT_SetFont(hitem, FONT_DEF);
  TEXT_SetText(hitem, "");
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_MIC);
  TEXT_SetTextAlign(hitem, GUI_TA_LEFT | GUI_TA_BOTTOM);
  TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0xFF0000FF));
  TEXT_SetFont(hitem, FONT_DEF);
  TEXT_SetText(hitem, ui_str.ui_test_4.mic);
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_PROGBAR_MIC_VAL);
  PROGBAR_SetFont(hitem, PROG_FONT_DEF);
  PROGBAR_SetMinMax(hitem, 0, 100);
  PROGBAR_SetValue(hitem, 0);

  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_OS);
  TEXT_SetTextAlign(hitem, GUI_TA_LEFT | GUI_TA_BOTTOM);
  TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0xFF0000FF));
  TEXT_SetFont(hitem, FONT_DEF);
  TEXT_SetText(hitem, ui_str.ui_test_4.ostest);
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_OS_RES);
  TEXT_SetTextAlign(hitem, GUI_TA_LEFT | GUI_TA_BOTTOM);
  TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0xFFFFFFFF));
  TEXT_SetFont(hitem, FONT_DEF);
  TEXT_SetText(hitem, "");
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_LISTBOX_OS_RES);
  LISTBOX_SetFont(hitem, FONT_DEF);
  WM_HideWindow(hitem);
  WM_EnableMemdev(hitem);

  hitem = WM_GetDialogItem(hwin, ID_LISTVIEW_CURRENT);
  LISTVIEW_SetFont(hitem, GUI_FONT_13_ASCII);
  HEADER_SetFont(LISTVIEW_GetHeader(hitem), GUI_FONT_13B_ASCII);
  LISTVIEW_AddColumn(hitem, 60, "V (mV)", GUI_TA_HCENTER | GUI_TA_VCENTER);
  LISTVIEW_AddColumn(hitem, 60, "I (mA)", GUI_TA_HCENTER | GUI_TA_VCENTER);
  LISTVIEW_AddColumn(hitem, 60, "V (mV)", GUI_TA_HCENTER | GUI_TA_VCENTER);
  LISTVIEW_AddColumn(hitem, 60, "I (mA)", GUI_TA_HCENTER | GUI_TA_VCENTER);
  LISTVIEW_SetColumnWidth(hitem, LISTVIEW_ALL_ITEMS, 30);
  LISTVIEW_SetHeaderHeight(hitem, 16);
  LISTVIEW_SetGridVis(hitem, 1);

  for(int i = 0;i < 4;i++)
  {
    LISTVIEW_AddRow(hitem, 0);

    for(int j = 0;j < 4;j++)
      LISTVIEW_SetItemText(hitem, j, i, "0");
  }
  LISTVIEW_AddRow(hitem, 0);
  LISTVIEW_SetItemText(hitem, 0, 6, "0");
  LISTVIEW_SetItemText(hitem, 1, 6, "0");

  WM_EnableMemdev(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_BTN_MENU);
  TEXT_SetTextAlign(hitem, GUI_TA_HCENTER | GUI_TA_TOP);
  TEXT_SetBkColor(hitem, GUI_MAKE_COLOR(0xabc88b));
  TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0x0));
  TEXT_SetFont(hitem, FONT_DEF);
  TEXT_SetText(hitem, ui_str.ui_test_4.btn_menu);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_BTN_MIC);
  TEXT_SetTextAlign(hitem, GUI_TA_LEFT | GUI_TA_TOP);
  TEXT_SetBkColor(hitem, GUI_MAKE_COLOR(0xabc88b));
  TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0x0));
  TEXT_SetFont(hitem, FONT_DEF);
  TEXT_SetText(hitem, ui_str.ui_test_4.btn_record_play);

  htimer = WM_CreateTimer(hwin, 0, 100, 0);

  WM_SetFocus(hwin);
}

static void hide_func_test_ui(WM_HWIN hwin)
{
  WM_HWIN hitem;

  hitem = WM_GetDialogItem(hwin, ID_TEXT_MIC1);
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_MIC1_RES);
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_MIC);
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_PROGBAR_MIC_VAL);
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_OS);
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_TEXT_OS_RES);
  WM_HideWindow(hitem);

  hitem = WM_GetDialogItem(hwin, ID_LISTBOX_OS_RES);
  WM_HideWindow(hitem);
}

static void update_func_test_result(WM_HWIN win)
{
  WM_HWIN hitem;
  char str_buf[32];

  uint8_t mic1_rdy = test->mic1_is_rdy();
  hitem = WM_GetDialogItem(win, ID_TEXT_MIC1_RES);
  TEXT_SetText(hitem, mic1_rdy ? ui_str.ui_test_4.mic1_found : ui_str.ui_test_4.mic1_no_found);
  if(mic1_rdy == 0)
  {
    TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0xFFFF0000));
  }
  else
  {
    TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0xFF00FF00));
  }

  hitem = WM_GetDialogItem(win, ID_PROGBAR_MIC_VAL);
  if(mic_record == 0)
  {
    PROGBAR_SetValue(hitem, test->mic_get());
  }
  else
  {
    uint8_t prog = test->mic_record_get_progress();
    PROGBAR_SetValue(hitem, prog);
    snprintf(str_buf, sizeof(str_buf), "REC %u%%", prog);
    PROGBAR_SetText(hitem, str_buf);
  }
}

static void _cbDialog(WM_MESSAGE* pmsg)
{
  WM_HWIN hwin;
  WM_HWIN hitem;
  char    str_buf[128];

  hwin = pmsg->hWin;
  switch(pmsg->MsgId)
  {
    case WM_INIT_DIALOG:
    {
      wm_init_dialog(pmsg->hWin);
      break;
    }
    case WM_KEY:
    {
      pwdn_watchdog_reload();
      switch(((WM_KEY_INFO*)(pmsg->Data.p))->Key)
      {
        case UI_KEY(OK, ONCLICK):
        {
          analog_speaker_stop();
          GUI_EndDialog(hwin, 0);
          break;
        }
        case UI_KEY(UP, REPET):
        case UI_KEY(UP, ONCLICK):
        {
          hitem = WM_GetDialogItem(hwin, ID_LISTBOX_OS_RES);
          LISTBOX_DecSel(hitem);
          break;
        }
        case UI_KEY(DOWN, REPET):
        case UI_KEY(DOWN, ONCLICK):
        {
          hitem = WM_GetDialogItem(hwin, ID_LISTBOX_OS_RES);
          LISTBOX_IncSel(hitem);
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
              hitem = WM_GetDialogItem(hwin, ID_PROGBAR_MIC_VAL);
              PROGBAR_SetText(hitem, 0);
              mic_record = 0;
              test->mic_record_play();
            }
          }
          break;
        }
      }
      break;
    }
    case WM_TIMER:
    {
      WM_RestartTimer(htimer, 100);

      hitem = WM_GetDialogItem(hwin, ID_IMAGE_SPEAKER);
      if(analog_speaker_is_busy())
      {
        WM_ShowWindow(hitem);
      }
      else
      {
        WM_HideWindow(hitem);
      }

      if(stage == TEST_STAGE_FUNC)
      {
        update_func_test_result(hwin);
      }
      else
      {
        if(mic_record)
        {
          mic_record = 0;
          test->mic_record_stop();
          hitem = WM_GetDialogItem(hwin, ID_PROGBAR_MIC_VAL);
          PROGBAR_SetText(hitem, 0);
        }
      }

      hitem = WM_GetDialogItem(hwin, ID_LISTVIEW_CURRENT);
      vi_updata(hitem);

      if(stage != test->stage_get())
      {
        pwdn_watchdog_reload();
        stage = test->stage_get();

        switch(stage)
        {
          case TEST_STAGE_WAIT:
          {
            hide_func_test_ui(hwin);

            hitem = WM_GetDialogItem(hwin, ID_TEXT_WAIT);
            TEXT_SetText(hitem, ui_str.ui_test_4.info_no_found_module);
            WM_ShowWindow(hitem);
            break;
          }
          case TEST_STAGE_OS:
          {
            hide_func_test_ui(hwin);

            hitem = WM_GetDialogItem(hwin, ID_TEXT_WAIT);
            TEXT_SetText(hitem, ui_str.ui_test_4.info_doing_ostest);
            WM_ShowWindow(hitem);
            break;
          }
          case TEST_STAGE_FUNC:
          {
            update_func_test_result(hwin);

            hitem = WM_GetDialogItem(hwin, ID_TEXT_WAIT);
            WM_HideWindow(hitem);

            hitem = WM_GetDialogItem(hwin, ID_TEXT_MIC1);
            WM_ShowWindow(hitem);

            hitem = WM_GetDialogItem(hwin, ID_TEXT_MIC1_RES);
            WM_ShowWindow(hitem);

            hitem = WM_GetDialogItem(hwin, ID_TEXT_MIC);
            WM_ShowWindow(hitem);

            hitem = WM_GetDialogItem(hwin, ID_PROGBAR_MIC_VAL);
            WM_ShowWindow(hitem);

            hitem = WM_GetDialogItem(hwin, ID_TEXT_OS);
            WM_ShowWindow(hitem);

            hitem = WM_GetDialogItem(hwin, ID_TEXT_OS_RES);
            WM_ShowWindow(hitem);

            if(test->io_test_error_get() == 0)
            {
              TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0x00FF00));
              TEXT_SetText(hitem, ui_str.ui_test_4.ostest_result_normal);
            }
            else
            {
              TEXT_SetTextColor(hitem, GUI_MAKE_COLOR(0xFF0000));

              if(test->io_test_error_get() == -1)
              {
                sprintf(str_buf, "%s [%s%u]", ui_str.ui_test_4.ostest_result_oc, ui_str.ui_test_4.ostest_result_total, test->io_open_circuit_count_get());
                TEXT_SetText(hitem, str_buf);
                hitem = WM_GetDialogItem(hwin, ID_LISTBOX_OS_RES);

                while(LISTBOX_GetNumItems(hitem))
                  LISTBOX_DeleteItem(hitem, 0);

                WM_ShowWindow(hitem);

                for(uint32_t i = 0;i < test->io_open_circuit_count_get();i++)
                {
                  uint8_t open_circuit = test->io_open_circuit_data_get()[i];
                  sprintf(str_buf, "%02d", open_circuit);
                  LISTBOX_InsertString(hitem, str_buf, LISTBOX_GetNumItems(hitem));
                }

                LISTBOX_SetSel(hitem, 0);
              }
              else if(test->io_test_error_get() == -2)
              {
                sprintf(str_buf, "%s [%s%u]", ui_str.ui_test_4.ostest_result_sc, ui_str.ui_test_4.ostest_result_total, test->io_short_circuit_count_get());
                TEXT_SetText(hitem, str_buf);
                hitem = WM_GetDialogItem(hwin, ID_LISTBOX_OS_RES);

                while(LISTBOX_GetNumItems(hitem))
                  LISTBOX_DeleteItem(hitem, 0);

                WM_ShowWindow(hitem);

                for(uint32_t i = 0;i < test->io_short_circuit_count_get();i++)
                {
                  uint16_t short_circuit = test->io_short_circuit_data_get()[i];
                  sprintf(str_buf, "%02d <-> %02d", short_circuit >> 8, short_circuit & 0x00FF);
                  LISTBOX_InsertString(hitem, str_buf, LISTBOX_GetNumItems(hitem));
                }

                LISTBOX_SetSel(hitem, 0);
              }
            }
            break;
          }
        }
      }

      break;
    }
    case UI_MSG_BATTERY:
    {
      hitem = WM_GetDialogItem(hwin, ID_IMAGE_LEVEL);
      ui_battery_level_icon_updata(hitem, 0);
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
      WM_DefaultProc(pmsg);
      break;
    }
  }
}

void ui_test_4_open(test_4_st* _test_, const char* _title_)
{
  if(_test_)
  {
    test = _test_;
    title = _title_;
    stage = 0xff;

    GUI_CreateDialogBox(_aDialogCreate, GUI_COUNTOF(_aDialogCreate), _cbDialog, WM_HBKWIN, 0, 0);
  }
}
