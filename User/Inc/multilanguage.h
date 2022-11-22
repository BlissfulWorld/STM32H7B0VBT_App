#ifndef _MULTILANGUAGE_H
#define _MULTILANGUAGE_H

#include "stdint.h"

typedef enum ml_id_e //: uint16_t
{
  ML_NAME = 0x0000,

  ML_UI_LAUNCH_TITIL = 0x0100,
  ML_UI_LAUNCH_UPDATE_RESOURCE = 0x0110,

  ML_UI_CHARGE_TITIL = 0x0200,
  ML_UI_CHARGE_CHARGING = 0x0210,
  ML_UI_CHARGE_FULL = 0x0211,
  ML_UI_CHARGE_DISCHARGING = 0x0212,

  ML_UI_HOME_TITIL = 0x0300,
  ML_UI_HOME_BTN_UPDATE = 0x0301,
  ML_UI_HOME_BTN_START = 0x0302,
  ML_UI_HOME_BTN_MENU = 0x0303,
  ML_UI_HOME_NAME = 0x0310,
  ML_UI_HOME_VERSION = 0x0311,
  ML_UI_HOME_DESC = 0x0312,
  ML_UI_HOME_WAITING = 0x0313,

  ML_UI_CONFIG_UPDATE_TITLE = 0x0400,
  ML_UI_CONFIG_UPDATE_BTN_BACK = 0x0401,
  ML_UI_CONFIG_UPDATE_BTN_UPDATE = 0x0402,
  ML_UI_CONFIG_UPDATE_BTN_NONE3 = 0x0403,
  ML_UI_CONFIG_UPDATE_NAME = 0x0410,
  ML_UI_CONFIG_UPDATE_VERSION = 0x0411,
  ML_UI_CONFIG_UPDATE_DESC = 0x0412,
  ML_UI_CONFIG_UPDATE_NOW_CONFIG = 0x0413,
  ML_UI_CONFIG_UPDATE_NEW_CONFIG = 0x0414,
  ML_UI_CONFIG_UPDATE_UPDATING = 0x0415,
  ML_UI_CONFIG_UPDATE_UPDATE_FINISHED = 0x0416,
  ML_UI_CONFIG_UPDATE_UPDATE_FAILED = 0x0417,

  ML_UI_OSTEST_RESULT_TITLE = 0x0500,
  ML_UI_OSTEST_RESULT_BTN_BACK = 0x0501,
  ML_UI_OSTEST_RESULT_BTN_EXPORT_DATA = 0x0502,
  ML_UI_OSTEST_RESULT_BTN_LEARN = 0x0503,
  ML_UI_OSTEST_RESULT_BTN_NO = 0x0504,
  ML_UI_OSTEST_RESULT_BTN_YES = 0x0505,
  ML_UI_OSTEST_RESULT_EXPORTING = 0x0510,
  ML_UI_OSTEST_RESULT_EXPORT_FINISHED = 0x0511,
  ML_UI_OSTEST_RESULT_EXPORT_FAILED = 0x0512,
  ML_UI_OSTEST_RESULT_EXPORT_DATA_TO_STORAGE = 0x0513,
  ML_UI_OSTEST_RESULT_LEARNING = 0x0514,
  ML_UI_OSTEST_RESULT_LEARN_FINISHED = 0x0515,
  ML_UI_OSTEST_RESULT_LEARN_FAILED = 0x0516,
  ML_UI_OSTEST_RESULT_LEARN_AND_WRITE_FLASH = 0x0517,

  ML_UI_NONSUPPORT_TITLE = 0x0600,
  ML_UI_NONSUPPORT_INFO = 0x0610,

  ML_UI_MENU_TITLE = 0x2000,
  ML_UI_MENU_BTN_BACK = 0x2001,
  ML_UI_MENU_BTN_CONFIRM = 0x2002,
  ML_UI_MENU_BTN_NONE3 = 0x2003,
  ML_UI_MENU_BRIGHTNESS = 0x2010,
  ML_UI_MENU_VOLUME = 0x2011,
  ML_UI_MENU_STANDBY = 0x2012,
  ML_UI_MENU_U_DISK_MODE = 0x2013,
  ML_UI_MENU_OSTEST = 0x2014,
  ML_UI_MENU_CLONE_APP = 0x2015,
  ML_UI_MENU_LANGUAGE = 0x2016,
  ML_UI_MENU_ABOUT = 0x2017,

  ML_UI_M_BRIGHTNESS_TITLE = 0x2100,
  ML_UI_M_BRIGHTNESS_BTN_BACK = 0x2101,
  ML_UI_M_BRIGHTNESS_BTN_NONE2 = 0x2102,
  ML_UI_M_BRIGHTNESS_BTN_SAVE = 0x2103,

  ML_UI_M_VOLUME_TITLE = 0x2200,
  ML_UI_M_VOLUME_BTN_BACK = 0x2201,
  ML_UI_M_VOLUME_BTN_PLAY = 0x2202,
  ML_UI_M_VOLUME_BTN_SAVE = 0x2203,

  ML_UI_M_STANDBY_TITLE = 0x2300,
  ML_UI_M_STANDBY_BTN_BACK = 0x2301,
  ML_UI_M_STANDBY_BTN_NONE2 = 0x2302,
  ML_UI_M_STANDBY_BTN_SAVE = 0x2303,

  ML_UI_M_U_DISK_MODE_TITLE = 0x2400,
  ML_UI_M_U_DISK_MODE_BTN_BACK = 0x2401,
  ML_UI_M_U_DISK_MODE_BTN_NONE2 = 0x2402,
  ML_UI_M_U_DISK_MODE_BTN_NONE3 = 0x2403,
  ML_UI_M_U_DISK_MODE_INFO = 0x2410,

  ML_UI_M_OSTEST_TITLE = 0x2500,
  ML_UI_M_OSTEST_BTN_BACK = 0x2501,
  ML_UI_M_OSTEST_BTN_EDIT = 0x2502,
  ML_UI_M_OSTEST_BTN_NONE3 = 0x2503,
  ML_UI_M_OSTEST_FUNCTION = 0x2510,
  ML_UI_M_OSTEST_FUNCTION_ENABLE = 0x2511,
  ML_UI_M_OSTEST_FUNCTION_DISABLE = 0x2512,
  ML_UI_M_OSTEST_MAX_ERROR = 0x2520,
  ML_UI_M_OSTEST_EDIT_CANCEL = 0x25A0,
  ML_UI_M_OSTEST_EDIT_CONFIRM = 0x25A1,

  ML_UI_M_CLONE_APP_TITLE = 0x2600,
  ML_UI_M_CLONE_BTN_BACK = 0x2601,
  ML_UI_M_CLONE_BTN_START = 0x2602,
  ML_UI_M_CLONE_BTN_NONE3 = 0x2603,
  ML_UI_M_CLONE_SOFTWARE_VERSION = 0x2610,
  ML_UI_M_CLONE_DEVICE_DISCONNECT = 0x2611,
  ML_UI_M_CLONE_CONNECTING = 0x2612,
  ML_UI_M_CLONE_CONNECTED = 0x2613,
  ML_UI_M_CLONE_ERROR = 0x2614,
  ML_UI_M_CLONE_CLONE_FAILED = 0x2615,
  ML_UI_M_CLONE_CLONE_FINISHED = 0x2616,
  ML_UI_M_CLONE_START_CLONE = 0x2617,

  ML_UI_M_LANGUAGE_TITLE = 0x2700,
  ML_UI_M_LANGUAGE_BTN_BACK = 0x2701,
  ML_UI_M_LANGUAGE_BTN_SELECT = 0x2702,
  ML_UI_M_LANGUAGE_BTN_SAVE = 0x2703,

  ML_UI_M_ABOUT_TITLE = 0x2800,
  ML_UI_M_ABOUT_BTN_BACK = 0x2801,
  ML_UI_M_ABOUT_APP_VERSION = 0x2810,
  ML_UI_M_ABOUT_BOOTLOADER_VERSION = 0x2811,

  ML_UI_TEST_0_TITLE = 0x8300,
  ML_UI_TEST_0_BTN_NONE1 = 0x8301,
  ML_UI_TEST_0_BTN_MENU = 0x8302,
  ML_UI_TEST_0_BTN_RECORD_PLAY = 0x8303,
  ML_UI_TEST_0_INFO_NO_FOUND_MODULE = 0x8310,
  ML_UI_TEST_0_INFO_DOING_OSTEST = 0x8311,
  ML_UI_TEST_0_PROX = 0x8312,
  ML_UI_TEST_0_ALS = 0x8313,
  ML_UI_TEST_0_MIC = 0x8314,
  ML_UI_TEST_0_OSTEST = 0x8315,
  ML_UI_TEST_0_CAMERA = 0x8316,
  ML_UI_TEST_0_CAMERA_FOUND = 0x8317,
  ML_UI_TEST_0_CAMERA_NO_FOUND = 0x8318,
  ML_UI_TEST_0_SPK_P = 0x8319,
  ML_UI_TEST_0_SPK_N = 0x831A,
  ML_UI_TEST_0_OSTEST_RESULT_NORMAL = 0x831B,
  ML_UI_TEST_0_OSTEST_RESULT_EXCEPTION = 0x831C,
  ML_UI_TEST_0_KEY_UP_CHECK = 0x831D,
  ML_UI_TEST_0_NO_OSTEST = 0x831E,
  ML_UI_TEST_0_INFO_DEVICE_EXCEPTION = 0x831F,

  ML_UI_TEST_1_TITLE = 0x8400,
  ML_UI_TEST_1_BTN_SPEAKER_PLAY = 0x8401,
  ML_UI_TEST_1_BTN_MENU = 0x8402,
  ML_UI_TEST_1_BTN_RECORD_PLAY = 0x8403,
  ML_UI_TEST_1_INFO_NO_FOUND_MODULE = 0x8410,
  ML_UI_TEST_1_INFO_DOING_OSTEST = 0x8411,
  ML_UI_TEST_1_PROX = 0x8412,
  ML_UI_TEST_1_ALS = 0x8413,
  ML_UI_TEST_1_MIC = 0x8414,
  ML_UI_TEST_1_OSTEST = 0x8415,
  ML_UI_TEST_1_SPK_P = 0x8416,
  ML_UI_TEST_1_SPK_N = 0x8417,
  ML_UI_TEST_1_CATHC = 0x8418,
  ML_UI_TEST_1_YOGI = 0x8419,
  ML_UI_TEST_1_HALL = 0x841A,
  ML_UI_TEST_1_OSTEST_RESULT_NORMAL = 0x841B,
  ML_UI_TEST_1_OSTEST_RESULT_EXCEPTION = 0x841C,
  ML_UI_TEST_1_KEY_UP_CHECK = 0x841D,
  ML_UI_TEST_1_NO_OSTEST = 0x841E,
  ML_UI_TEST_1_INFO_DEVICE_EXCEPTION = 0x841F,

  ML_UI_TEST_2_TITLE = 0x8500,
  ML_UI_TEST_2_BTN_SPEAKER_PLAY = 0x8501,
  ML_UI_TEST_2_BTN_MENU = 0x8502,
  ML_UI_TEST_2_BTN_RECORD_PLAY = 0x8503,
  ML_UI_TEST_2_INFO_NO_FOUND_MODULE = 0x8510,
  ML_UI_TEST_2_INFO_DOING_OSTEST = 0x8511,
  ML_UI_TEST_2_PROX = 0x8512,
  ML_UI_TEST_2_ALS = 0x8513,
  ML_UI_TEST_2_MIC = 0x8514,
  ML_UI_TEST_2_OSTEST = 0x8515,
  ML_UI_TEST_2_SPK_P = 0x8516,
  ML_UI_TEST_2_SPK_N = 0x8517,
  ML_UI_TEST_2_CATHC = 0x8518,
  ML_UI_TEST_2_YOGI = 0x8519,
  ML_UI_TEST_2_OSTEST_RESULT_NORMAL = 0x851A,
  ML_UI_TEST_2_OSTEST_RESULT_EXCEPTION = 0x851B,
  ML_UI_TEST_2_KEY_UP_CHECK = 0x851C,
  ML_UI_TEST_2_NO_OSTEST = 0x851D,
  ML_UI_TEST_2_INFO_DEVICE_EXCEPTION = 0x851E,

  ML_UI_TEST_3_TITLE = 0x8600,
  ML_UI_TEST_3_BTN_SPEAKER_PLAY = 0x8601,
  ML_UI_TEST_3_BTN_MENU = 0x8602,
  ML_UI_TEST_3_BTN_RECORD_PLAY = 0x8603,
  ML_UI_TEST_3_INFO_NO_FOUND_MODULE = 0x8610,
  ML_UI_TEST_3_INFO_DOING_OSTEST = 0x8611,
  ML_UI_TEST_3_PROX = 0x8612,
  ML_UI_TEST_3_ALS = 0x8613,
  ML_UI_TEST_3_MIC = 0x8614,
  ML_UI_TEST_3_OSTEST = 0x8615,
  ML_UI_TEST_3_SPK_P = 0x8616,
  ML_UI_TEST_3_SPK_N = 0x8617,
  ML_UI_TEST_3_CATHC = 0x8618,
  ML_UI_TEST_3_YOGI = 0x8619,
  ML_UI_TEST_3_COMPASS = 0x861A,
  ML_UI_TEST_3_OSTEST_RESULT_NORMAL = 0x861B,
  ML_UI_TEST_3_OSTEST_RESULT_EXCEPTION = 0x861C,
  ML_UI_TEST_3_KEY_UP_CHECK = 0x861D,
  ML_UI_TEST_3_NO_OSTEST = 0x861E,
  ML_UI_TEST_3_INFO_DEVICE_EXCEPTION = 0x861F,

  ML_UI_TEST_4_TITLE = 0x8700,
  ML_UI_TEST_4_BTN_NONE1 = 0x8701,
  ML_UI_TEST_4_BTN_MENU = 0x8702,
  ML_UI_TEST_4_BTN_RECORD_PLAY = 0x8703,
  ML_UI_TEST_4_INFO_NO_FOUND_MODULE = 0x8710,
  ML_UI_TEST_4_INFO_DOING_OSTEST = 0x8711,
  ML_UI_TEST_4_MIC1 = 0x8712,
  ML_UI_TEST_4_MIC1_FOUND = 0x8713,
  ML_UI_TEST_4_MIC1_NO_FOUND = 0x8714,
  ML_UI_TEST_4_MIC = 0x8715,
  ML_UI_TEST_4_OSTEST = 0x8716,
  ML_UI_TEST_4_OSTEST_RESULT_NORMAL = 0x8717,
  ML_UI_TEST_4_OSTEST_RESULT_OC = 0x8718,
  ML_UI_TEST_4_OSTEST_RESULT_SC = 0x8719,
  ML_UI_TEST_4_OSTEST_RESULT_TOTAL = 0x871A,
  ML_UI_TEST_4_INFO_DEVICE_EXCEPTION = 0x871B,

}ml_id_et;

struct ui_lanuch_s
{
  const char* update_resource;
};

struct ui_charge_s
{
  const char* full;
  const char* charging;
  const char* discharging;
};

struct ui_home_s
{
  const char* title;
  const char* btn_update;
  const char* btn_start;
  const char* btn_menu;

  const char* name;
  const char* version;
  const char* description;
  const char* waiting;
};

struct ui_config_update_s
{
  const char* title;
  const char* btn_back;
  const char* btn_update;

  const char* name;
  const char* version;
  const char* description;
  const char* now_config;
  const char* new_config;
  const char* updating;
  const char* update_finished;
  const char* update_failed;
};

struct ui_ostest_result_s
{
  const char* title;
  const char* btn_back;
  const char* btn_exp_data;
  const char* btn_learn;
  const char* btn_no;
  const char* btn_yes;

  const char* exporting;
  const char* export_finished;
  const char* export_failed;
  const char* export_data_to_storage;
  const char* learning;
  const char* learn_finished;
  const char* learn_failed;
  const char* learn_and_write_to_flash;
};

struct ui_nonsupport_s
{
  const char* title;

  const char* info;
};

struct ui_menu_s
{
  const char* title;
  const char* btn_back;
  const char* btn_confirm;

  const char* brightness;
  const char* volume;
  const char* standby;
  const char* u_disk_mode;
  const char* os_test;
  const char* clone_app;
  const char* language;
  const char* about;
};

struct ui_m_brightness_s
{
  const char* title;
  const char* btn_back;
  const char* btn_save;
};

struct ui_m_volume_s
{
  const char* title;
  const char* btn_back;
  const char* btn_play;
  const char* btn_save;
};

struct ui_m_standby_s
{
  const char* title;
  const char* btn_back;
  const char* btn_save;
};

struct ui_m_u_disk_mode_s
{
  const char* title;
  const char* btn_back;

  const char* info;
};

struct ui_m_ostest_s
{
  const char* title;
  const char* btn_back;
  const char* btn_edit;

  const char* function;
  const char* function_enable;
  const char* function_disable;

  const char* max_error;
};

struct ui_m_clone_s
{
  const char* title;
  const char* btn_back;
  const char* btn_start;

  const char* software_version;
  const char* device_disconnect;
  const char* connecting;
  const char* connected;
  const char* error;
  const char* clone_failed;
  const char* clone_finished;
  const char* start_clone;
};

struct ui_m_language_s
{
  const char* title;
  const char* hint;
  const char* btn_select;
  const char* btn_back;
  const char* btn_save;
};

struct ui_m_about_s
{
  const char* title;
  const char* btn_back;

  const char* app_version;
  const char* bootloader_version;
};

struct ui_m_ostest_edit_s
{
  const char* btn_cancel;
  const char* btn_confirm;
};

struct ui_test_0_s
{
  const char* title;
  const char* btn_menu;
  const char* btn_record_play;

  const char* info_no_found_module;
  const char* info_doing_ostest;
  const char* prox;
  const char* als;
  const char* mic;
  const char* ostest;
  const char* camera;
  const char* camera_found;
  const char* camera_no_found;
  const char* spk_p;
  const char* spk_n;
  const char* ostest_result_normal;
  const char* ostest_result_exception;
  const char* key_up_check;
  const char* no_ostest;
  const char* info_device_exception;
};

struct ui_test_1_s
{
  const char* title;
  const char* btn_speaker_play;
  const char* btn_menu;
  const char* btn_record_play;

  const char* info_no_found_module;
  const char* info_doing_ostest;
  const char* prox;
  const char* als;
  const char* mic;
  const char* ostest;
  const char* spk_p;
  const char* spk_n;
  const char* cathc;
  const char* yogi;
  const char* hall;
  const char* ostest_result_normal;
  const char* ostest_result_exception;
  const char* key_up_check;
  const char* no_ostest;
  const char* info_device_exception;
};

struct ui_test_2_s
{
  const char* title;
  const char* btn_speaker_play;
  const char* btn_menu;
  const char* btn_record_play;

  const char* info_no_found_module;
  const char* info_doing_ostest;
  const char* prox;
  const char* als;
  const char* mic;
  const char* ostest;
  const char* spk_p;
  const char* spk_n;
  const char* cathc;
  const char* yogi;
  const char* ostest_result_normal;
  const char* ostest_result_exception;
  const char* key_up_check;
  const char* no_ostest;
  const char* info_device_exception;
};

struct ui_test_3_s
{
  const char* title;
  const char* btn_speaker_play;
  const char* btn_menu;
  const char* btn_record_play;

  const char* info_no_found_module;
  const char* info_doing_ostest;
  const char* prox;
  const char* als;
  const char* mic;
  const char* ostest;
  const char* spk_p;
  const char* spk_n;
  const char* cathc;
  const char* yogi;
  const char* compass;
  const char* ostest_result_normal;
  const char* ostest_result_exception;
  const char* key_up_check;
  const char* no_ostest;
  const char* info_device_exception;
};

struct ui_test_4_s
{
  const char* title;
  const char* btn_menu;
  const char* btn_record_play;

  const char* info_no_found_module;
  const char* info_doing_ostest;
  const char* mic1;
  const char* mic1_found;
  const char* mic1_no_found;
  const char* mic;
  const char* ostest;
  const char* ostest_result_normal;
  const char* ostest_result_oc;
  const char* ostest_result_sc;
  const char* ostest_result_total;
  const char* info_device_exception;
};

struct ui_str_s
{
  const char* name;

  struct ui_lanuch_s ui_lanuch;
  struct ui_charge_s ui_charge;

  struct ui_home_s ui_home;
  struct ui_config_update_s ui_config_update;
  struct ui_ostest_result_s ui_ostest_result;
  struct ui_nonsupport_s ui_nonsupport;
  struct ui_menu_s ui_menu;

  struct ui_m_brightness_s ui_m_brightness;
  struct ui_m_volume_s ui_m_volume;
  struct ui_m_standby_s ui_m_standby;
  struct ui_m_u_disk_mode_s ui_m_u_disk_mode;
  struct ui_m_ostest_s ui_m_ostest;
  struct ui_m_clone_s ui_m_clone;
  struct ui_m_language_s ui_m_language;
  struct ui_m_about_s ui_m_about;

  struct ui_m_ostest_edit_s ui_m_ostest_edit;

  struct ui_test_0_s ui_test_0;
  struct ui_test_1_s ui_test_1;
  struct ui_test_2_s ui_test_2;
  struct ui_test_3_s ui_test_3;
  struct ui_test_4_s ui_test_4;
};

extern struct ui_str_s ui_str;

void ml_set(const char* path);
int ml_get_name(const char* path, char* buf, uint32_t max_len);

#endif
